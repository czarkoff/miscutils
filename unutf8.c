#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <wchar.h>
#include "args.h"


#define LOCALE "en_US.UTF-8"
#define UTF8_MAX_BYTES 6


int
main(int argc, char **argv)
{
	wchar_t wc;
	int l;
	char buf[UTF8_MAX_BYTES + 1];

	if (setlocale(LC_CTYPE, LOCALE) == NULL)
		err(1, "setlocale (" LOCALE ")");

	for (args_raw_init(&argc, &argv); argc--; argv++) {
		do {
			l = mbtowc(&wc, *argv, UTF8_MAX_BYTES);
			
			if (l == 0)
				break;
			
			if (l == -1) {
				warn("%#02hhx", **argv);
				*argv += 1;
				continue;
			}
			
			strlcpy(buf, *argv, l + 1);
			printf("%s\tU+%04x\n", buf, wc);
		} while (*(*argv += l) != '\0');
	}

	return 0;
}
