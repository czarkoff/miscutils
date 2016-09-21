/*
 * Copyright (c) 2016 Dmitrij D. Czarkoff <czarkoff@gmail.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <locale.h>
#include <wchar.h>
#include <unistd.h>
#include <errno.h>


wchar_t jcuken(wchar_t c);
int convert_stream(FILE *in);
void usage(int ret);


static wchar_t ru[] = L"ёЁ\"№;:?₽йцукенгшщзхъЙЦУКЕНГШЩЗХЪ/фывапролджэФЫВАПРОЛДЖЭячсмитьбю.ЯЧСМИТЬБЮ,";
static wchar_t us[] = L"`~@#$^&*qwertyuiop[]QWERTYUIOP{}|asdfghjkl;'ASDFGHJKL:\"zxcvbnm,./ZXCVBNM<>?";

int dir = 0;


wchar_t
jcuken(wchar_t c)
{
	int i;
	wchar_t *key;
	wchar_t *value;

	if (dir) {
		key = us;
		value = ru;
	} else {
		key = ru;
		value = us;
	}

	for (i = 0; key[i] != L'\0'; i++)
		if (key[i] == c)
			return value[i];

	return c;
}


int
convert_stream(FILE *in)
{
	wint_t w, last;

	while ((w = getwc(in)) != WEOF)
		if (putwc(jcuken(last = w), stdout) == WEOF)
			break;

	if (last != L'\n')
		putwc(L'\n', stdout);

	if (ferror(stdout)) {
		fwprintf(stderr, L"%s: output error: %s", getprogname(), strerror(errno));
		return 1;
	}

	if (ferror(in)) {
		fwprintf(stderr, L"%s: input error: %s", getprogname(), strerror(errno));
		return 1;
	} else if (in != stdin) {
		fclose(in);
	}

	return 0;
}


void
usage(int ret)
{
	FILE *out;
	if (ret == 0)
		out = stdout;
	else
		out = stderr;
	fwprintf(out, L"Usage: %s [-r] [-f file | -s string]\n", getprogname());
	exit(ret);
}


int
main(int argc, char **argv)
{
	wint_t w, last;
	char c, *path = NULL, *s = NULL;
	FILE *in = stdin;
	int fflag, sflag;
	size_t len;
	int n;
	int printed = 0;

	setlocale(LC_CTYPE, "");

	while ((c = getopt(argc, argv, "f:hrs:")) != -1) {
		fflag = 0;
		sflag = 0;
		switch(c) {
		case 'f':
			if (strncmp(optarg, "-", 2) == 0) {
				if (printed == -1) {
					fwprintf(stderr, L"%s: standard input already consumed\n", getprogname());
					return 1;
				}
				printed = -1;
				in = stdin;
			} else {
				fflag = 1;
				path = optarg;
			}
			break;
		case 'h':
			usage(0);
			break;
		case 's':
			sflag = 1;
			s = optarg;
			break;
		case 'r':
			dir = !dir;
			break;
		default:
			usage(1);
			break;
		}

		if (printed == 0)
			printed = 1;

		if (sflag) {
			in = NULL;
			len = strnlen(s, ARG_MAX);
			while (len > 0 && (n = mbtowc(&w, s, len)) > 0) {
				if (putwc(jcuken(last = w), stdout) == WEOF)
					break;
				len -= n;
				s += n;
			}

			if (last != L'\n')
				putwc(L'\n', stdout);

			if (ferror(stdout)) {
				fwprintf(stderr, L"%s: output error: %s", getprogname(), strerror(errno));
				return 1;
			}

			continue;
		}
		
		if (fflag && (in = fopen(path, "r")) == NULL) {
			fwprintf(stderr, L"%s: can't open \"%s\": %s", getprogname(), path, strerror(errno));
			return 1;
		}

		if (in != NULL)
			if (convert_stream(in) != 0)
				return 1;
	}
	if (argc - optind) {
		fwprintf(stderr, L"%s: unexpected argument: %s\n", getprogname(), *(argv + optind));
		usage(1);
	}

	if (printed == 0)
		if (convert_stream(in) != 0)
			return 1;

	return 0;
}
