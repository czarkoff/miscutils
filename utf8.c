#include <err.h>
#include <locale.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include "utf8.h"
#include "uniname.h"


#define USAGE "Usage: %s [-dho] ...\n", getprogname()

#define FHEX 0x1
#define FDEC 0x2
#define FOCT 0x4


int
main(int argc, char **argv)
{
	unsigned long n;
	int l;
	int base, offset, ret = 0;
	char *e, buf[UTF8_MAX_BYTES + 1];
	char *p;
	char ec;
	int fmt = 0, quiet = 0, name = 0;
	char c;
	int i;

	if (setlocale(LC_CTYPE, LOCALE) == NULL)
		err(1, "setlocale (" LOCALE ")");

	while ((c = getopt(argc, argv, "dhnoq")) != -1) {
		switch (c) {
			case 'd':
				fmt |= FDEC;
				break;
			case 'h':
				fmt |= FHEX;
				break;
			case 'n':
				name = 1;
				break;
			case 'o':
				fmt |= FOCT;
				break;
			case 'q':
				quiet = 1;
				break;
			default:
				dprintf(STDERR_FILENO, USAGE);
				return 1;
		}
	}
	
	if (fmt == 0)
		fmt = FDEC | FHEX | FOCT;

	if (fmt == FDEC || fmt == FHEX || fmt == FOCT)
		quiet = 1;

	for (argv += optind, argc -= optind; argc--; argv++) {
		if (strncmp(*argv, "U+", 2) == 0) {
			offset = 2;
			base = 16;
			ec = '\0';
		} else if (strncmp(*argv, "&#", 2) == 0) {
			offset = 2;
			base = 10;
			ec = ';';
		} else {
			offset = 0;
			base = 10;
			ec = '\0';
		}

		n = strtoul(*argv + offset, &e, base);
		if (*e != ec || n > UNICODE_MAX_VALUE)
			err(++ret, "invalid argument: %s", *argv);

		if ((l = wctomb(buf, n)) <= 0) {
			warnx("wctomb: %4lx", n);
			ret++;
			continue;
		}
		buf[l] = '\0';

		printf("U+%04lx ", n);

		if (name) {
			for (i = 0; unikey[i] < UINT32_MAX; i++) {
				if (unikey[i] == n) {
					printf("(%s) ", univalue[i]);
					break;
				}
			}
		}
		if (!quiet)
			printf(" %s:  # %s\n", buf, *argv);
		else
			printf("→");

		if (fmt & FOCT) {
			if (!quiet)
				printf("  oct ");
			else
				printf(" ");
			for (p = buf; *p != '\0'; p++)
				printf(" %hho", *p);
			if (!quiet)
				printf("\n");
		}

		if (fmt & FDEC) {
			if (!quiet)
				printf("  dec ");
			else
				printf(" ");
			for (p = buf; *p != '\0'; p++)
				printf(" %hhu", *p);
			if (!quiet)
				printf("\n");
		}

		if (fmt & FHEX) {
			if (!quiet)
				printf("  hex ");
			else
				printf(" ");
			for (p = buf; *p != '\0'; p++)
				printf(" %hhx", *p);
			if (!quiet)
				printf("\n");
		}

		if (quiet)
			printf("\n");
	}

	return 0;
}
