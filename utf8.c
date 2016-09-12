#include <err.h>
#include <locale.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "args.h"


#define LOCALE "en_US.UTF-8"
#define UTF8_MAX_BYTES 6
#define UNICODE_MAX_VALUE 0x10FFFF


int
main(int argc, char **argv)
{
	unsigned long n;
	int l;
	int base, offset, ret = 0;
	char *e, buf[UTF8_MAX_BYTES + 1];
	char *p;
	char ec;

	if (setlocale(LC_CTYPE, LOCALE) == NULL)
		err(1, "setlocale (" LOCALE ")");

	for (args_raw_init(&argc, &argv); argc--; argv++) {
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
		if (*e != ec || n > UNICODE_MAX_VALUE) {
			ret++;
			printf("%s  invalid argument\n", *argv);
			continue;
		}

		if ((l = wctomb(buf, n)) <= 0) {
			warnx("wctomb: %4lx", n);
			ret++;
			continue;
		}
		buf[l] = '\0';

		printf("U+%04lx  %s:  # %s\n", n, buf, *argv);

		printf("  oct ");
		for (p = buf; *p != '\0'; p++)
			printf(" %hho", *p);
		printf("\n");

		printf("  dec ");
		for (p = buf; *p != '\0'; p++)
			printf(" %hhu", *p);
		printf("\n");

		printf("  hex ");
		for (p = buf; *p != '\0'; p++)
			printf(" %#hhx", *p);
		printf("\n");
	}

	args_free();
	return 0;
}
