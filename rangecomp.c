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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>


#define STEP 100

void addnum(const char *);
void grow(void);
void prange(void);

typedef struct range Range;
struct range {
	unsigned int start;
	unsigned int end;
};


Range **buf = NULL;
size_t nnums = 0, bufsz = 0;
int ignore = 0;


void
grow(void)
{
	Range **p;

	bufsz += STEP;
	p = reallocarray(buf, bufsz, sizeof(Range *));

	if (p == NULL) {
		if (buf)
			free(buf);
		err(1, "%d", __LINE__);
	}
	buf = p;
}


void
addnum(const char *s)
{
	const char *e;
	unsigned int n;
	size_t i, j;

	n = (unsigned int)strtonum(s, 0, UINT_MAX, &e);
	if (e != NULL) {
		if (!ignore)
			errx(1, "%s: %s", s, e);
		warnx("%s: %s", s, e);
		return;
	}

	for (i = 0; i < nnums; i++) {
		/* Check if n borders current range */
		if (buf[i]->start == n + 1) {
			buf[i]->start = n;
			return;
		} else if (buf[i]->end == n - 1) {
			buf[i]->end = n;
			if (buf[i + 1] == NULL || buf[i + 1]->start > n + 1)
				return;
			buf[i]->end = buf[i + 1]->end;
			free(buf[++i]);
			nnums--;
			if (i + 1 < nnums)
				for (j = i; j < nnums; j++)
					buf[j + 1] = buf[j];
			return;
		}

		/* If n is greater then current range, continue */
		if (buf[i]->end < n)
			continue;

		/* If n is within range, continue */
		if (buf[i]->start <= n && buf[i]->end >= n)
			continue;

		for (j = i; j < nnums; j++)
			buf[j + 1] = buf[j];

		if ((buf[i] = malloc(sizeof(Range))) == NULL)
			err(1, "%d", __LINE__);

		buf[i]->start = n;
		buf[i]->end = n;
		nnums++;
		return;
	}
	if ((buf[i] = malloc(sizeof(Range))) == NULL)
		err(1, "%d", __LINE__);

	buf[i]->start = n;
	buf[i]->end = n;
	nnums++;
	return;
}


void
prange(void)
{
	size_t i;

	for (i = 0; i < nnums; i++) {
		if (i != 0)
			putchar(',');
		printf("%u", buf[i]->start);
		if (buf[i]->start != buf[i]->end) {
			if (buf[i]->end - buf[i]->start == 1)
				putchar(',');
			else
				putchar('-');
			printf("%u", buf[i]->end);
		}
	}
	printf("\n");
}


int
main(int argc, char **argv)
{
	char c;
	char *s, *p;
	size_t l;
	int ret = 0;

	while ((c = getopt(argc, argv, "ih")) != -1) {
		switch (c) {
			case 'i':
				ignore = 1;
				break;
			default:
				ret++;
			case 'h':
				dprintf(ret ? STDERR_FILENO : STDOUT_FILENO,
						"Usage: %s [-i] [...]\n",
						getprogname());
				return ret;
		}
	}

	if (argc -= optind) {
		bufsz = argc;
		buf = reallocarray(NULL, bufsz, sizeof(Range *));
		if (buf == NULL)
			err(1, "%d", __LINE__);

		for (argv += optind; argc--; argv++)
			addnum(*argv);
	} else {
		while (getline(&s, &l, stdin) != -1) {
			if (nnums == bufsz)
				grow();
			if ((p = strchr(s, '\n')) != NULL)
				*p = '\0';
			addnum(s);
		}
		if (ferror(stdin))
			err(1, NULL);
	}

	prange();
	return 0;
}
