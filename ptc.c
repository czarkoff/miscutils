/*
 * Copyright (c) 2014, Dmitrij D. Czarkoff
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <ctype.h>
#include <errno.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define LEN 40


typedef struct Token Token;
struct Token {
  long value;
  enum {
    NONE,
    VALUE,
    OP
  } type;
  struct Token *prev;
  struct Token *next;
};


Token *act(Token *tok);
Token *gettok(void);

long pop(Token *tok);
void push(Token *pos, long value);

void print(long ms);


Token *
act(Token *tok)
{
  extern int errno;
  long arg;

  if (tok->type != OP) {
    errno = EPERM;
    err(1, "operation %ld", tok->value);
  }

  switch (tok->value) {
    case '+':
      push(tok, pop(tok->prev) + pop(tok->prev));
      break;
    case '-':
      arg = pop(tok->prev);
      push(tok, pop(tok->prev) - arg);
      break;
    case '*':
      push(tok, pop(tok->prev) * pop(tok->prev));
      break;
    case '/':
      arg = pop(tok->prev);
      push(tok, pop(tok->prev) / arg);
      break;
    case '%':
      arg = pop(tok->prev);
      push(tok, pop(tok->prev) % arg);
      break;
    case 'p':
      print(tok->prev->value);
      tok = tok->prev;
      pop(tok->next);
      break;
    case 't':
      arg = 0;
      while (tok->prev != NULL)
        arg += pop(tok->prev);
      push(tok, arg);
      break;
    default:
      errno = EINVAL;
      err(1, "unknown operation \'%c\'", (char)tok->value);
  }

  return tok;
}


long
pop(Token *tok)
{
  long ret;

  ret = tok->value;

  if (tok->prev != NULL)
    tok->prev->next = tok->next;
  if (tok->next != NULL)
    tok->next->prev = tok->prev;

  free(tok);

  return ret;
}


void
push(Token *pos, long value)
{
  pos->type = VALUE;
  pos->value = value;
}

void
print(long ms)
{
  long d=0, h=0, m=0, s;

  if (ms < 0) {
    ms *= -1;
    fprintf(stdout, "-");
  }

  if ((s = ms/1000)) {
    ms %= 1000;
    if ((m = s/60)) {
      s %= 60;
      if ((h = m/60)) {
        m %= 60;
        if ((d = h/24)) {
          h %= 24;
          fprintf(stdout, "%ldd ", d);
        }
      }
    }
  }

  fprintf(stdout, "%02ld:%02ld:%02ld.%03ld\n", h, m, s, ms);
}


int
main(void)
{
  Token *tok, *last=NULL;

  while ((tok = gettok()) != NULL) {
    tok->prev = last;
    if (last != NULL)
      last->next = tok;
    if (tok->type == OP)
      last = act(tok);
    else
      last = tok;
  };

  return 0;
}


Token *
gettok(void)
{
  char *s, *p, sign=1;
  Token *tok;
  char fields=0, digits;

  extern int errno;

  if ((s = malloc(LEN)) == NULL)
    err(1, NULL);

  if ((tok = malloc(sizeof(Token))) == NULL)
    err(1, NULL);
  tok->prev = NULL;
  tok->next = NULL;

  while (isspace(*s = getc(stdin)) || *s == '\n')
    ;

  if (*s == EOF)
    return NULL;

  if (!isdigit(*s) && *s != '_') {
    tok->value = *s;
    tok->type = OP;
    return tok;
  }

  tok->type = VALUE;

  for (p=s+1; !isspace(*p = getc(stdin)) && *p != EOF; p++) {
    if (p-s-1 == LEN) {
      *p = '\0';
      errno = EINVAL;
      err(1, "\"%s\"", s);
    }
  }
  *p = '\0';

  if (!strchr(s, '.') && !strchr(s, ':')) {
    tok->value = strtol(s, &p, 0);
    switch (*p) {
      case '\0':
        break;
      case 'd':
        tok->value *= 24;
      case 'h':
        tok->value *= 60;
      case 'm':
        tok->value *= 60;
      case 's':
        tok->value *= 1000;
        break;
      default:
        errno = EINVAL;
        err(1, "\"%s\"", s);
    }
    goto out;
  }

  tok->value = 0;

  if (*(p = s) == '_') {
    sign = -1;
    p++;
  }

  do {
    tok->value *= 60;
    tok->value += strtol(p, &p, 10);
    if (fields++ > 2) {
      errno = EINVAL;
      err(1, "\"%s\"", s);
    }
  } while (*p++ == ':');

  /* If strings ends without dot, assume %H:%M[:%S] format */
  if (!strchr(s, '.') && *(p-1) == '\0') {
    tok->value *= 1000;
    if (fields < 3)
      tok->value *= 60;

  } else if (*(p-1) != '.') {
    errno = EINVAL;
    err(1, "\"%s\"", s);

  } else {
    for (digits = 0; digits < 3; digits++) {
      tok->value *= 10;
      if (*p != '\0')
        tok->value += *p++ - '0';
    }
  }

  tok->value *= sign;
  
out:
  free(s);
  return tok;
}
