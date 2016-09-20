#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <wchar.h>


#define LOCALE "en_US.UTF-8"
#define UTF8_MAX_BYTES 6

#define HEX 0x1
#define DEC 0x2


void stou_out(const char *);


int verbose = 0;
int fmt = 0;


void
stou_out(const char *s)
{
	wchar_t wc;
	int l;
	char buf[UTF8_MAX_BYTES + 1];

	for (l = 0; *s != '\0'; s += l) {
		l = mbtowc(&wc, s, UTF8_MAX_BYTES);
		
		if (l == 0)
			return;

		if (l == -1)
			err(1, "mbtowc");
		
		strlcpy(buf, s, l + 1);
		if (verbose) {
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
			printf("\t");
		}
		if (fmt & HEX)
			printf("U+%04X", wc);
		if (fmt & (HEX|DEC))
			printf("\t");
		if (fmt & DEC)
			printf("&#%d;", wc);
		printf("\n");
	}
}


int
main(int argc, char **argv)
{
	size_t len;
	char c, *s;
	int ret = 0;
	int separate = 0;

	if (setlocale(LC_CTYPE, LOCALE) == NULL)
		err(1, "setlocale (" LOCALE ")");

	while ((c = getopt(argc, argv, "dhsvx")) != -1) {
		switch (c) {
			case 'd':
				fmt |= DEC;
				break;
			case 'x':
				fmt |= HEX;
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

	if (fmt == 0)
		fmt = HEX;

	if (argc -= optind) {
		for (argv += optind; argc--; argv++) {
			stou_out(*argv);
			if (argc && separate)
				printf("\n");
		}

	} else {
		while ((s = fgetln(stdin, &len)) != NULL) {
			s[len - 1] = '\0';
			stou_out(s);
		}
		if (ferror(stdin))
			err(1, "fgetln");
	}

	return ret;
}
