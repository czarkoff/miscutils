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

#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <wchar.h>
#include "utf8.h"
#include "uniname.h"


#define HEX 0x1
#define DEC 0x2


void stou_out(const char *);


int verbose = 0;
int fmt_in = 0;
int fmt_out = 0;
int name = 0;


void
stou_out(const char *s)
{
	wchar_t wc;
	int l;
	int i;
	char buf[UTF8_MAX_BYTES + 1];

	for (l = 0; *s != '\0'; s += l) {
		l = mbtowc(&wc, s, UTF8_MAX_BYTES);
		
		if (l == 0)
			return;

		if (l == -1) {
			printf("0x%02hhX\t%s\n", *s, strerror(errno));
			l = 1;
			continue;
		}
		
		strlcpy(buf, s, l + 1);
		if (verbose) {
			if (fmt_in) {
				for (i = 0; i < l; i++)
					printf("%02hhX", buf[i]);
			} else {
				switch (wcwidth(wc)) {
					case -1:
						printf("*NP*");
						break;
					case 0:
						printf(" %s", buf);
						break;
					default:
						printf("%s", buf);
						break;
				}
			}
			printf("\t");
		}
		if (fmt_out & HEX)
			printf("U+%04X", wc);
		if (fmt_out & (HEX|DEC))
			printf("\t");
		if (fmt_out & DEC)
			printf("&#%d;", wc);
		if (name) {
			for (i = 0; unikey[i] < UINT32_MAX; i++) {
				if (unikey[i] == (uint32_t)wc) {
					printf("\t(%s) ", univalue[i]);
					break;
				}
			}
		}
		printf("\n");
	}
}


int
main(int argc, char **argv)
{
	size_t sz = 0;
	ssize_t len;
	char c, *s = NULL;
	int ret = 0;
	int separate = 0;

	if (setlocale(LC_CTYPE, LOCALE) == NULL)
		err(1, "setlocale (" LOCALE ")");

	while ((c = getopt(argc, argv, "bdhnsvx")) != -1) {
		switch (c) {
			case 'b':
				fmt_in = 1;
				break;
			case 'd':
				fmt_out |= DEC;
				break;
			case 'x':
				fmt_out |= HEX;
				break;
			case 'n':
				name = 1;
				break;
			case 's':
				separate = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			default:
				ret = 1;
			case 'h':
				dprintf(ret ? STDOUT_FILENO : STDERR_FILENO,
						"Usage: %s [-dsvx] [...]\n",
						getprogname());
				return ret;
		}
	}

	if (fmt_out == 0)
		fmt_out = HEX;

	if (argc -= optind) {
		for (argv += optind; argc--; argv++) {
			stou_out(*argv);
			if (argc && separate)
				printf("\n");
		}

	} else {
		while ((len = getline(&s, &sz, stdin)) != -1) {
			if (s[len - 1] != '\n') {
				if ((s = realloc(s, ++sz)) == NULL)
					err(1, NULL);
				s[len++] = '\n';
				s[len] = '\0';
			}
			stou_out(s);
		}
		if (ferror(stdin))
			err(1, "getline");
	}

	return ret;
}
