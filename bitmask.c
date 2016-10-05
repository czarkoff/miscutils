#include <limits.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include "binary.h"


#define BUFMUL sizeof(uintmax_t)
#define BUFSZ BUFMUL * 9
#define STEP 16


char *masktobin(uintmax_t);
uintmax_t bintomask(char *);
char *expand(char *, char *);
uintmax_t strtomask(char *);
void usage(FILE *);


const char *sym = "01";
int gaps = 0;


int
main(int argc, char **argv)
{
	char **arr;
	char *bin;
	uintmax_t mask;
	size_t arrsz = 0, i, num;

	int c;
	const char *of = "%#jx";
	int reverse = 0;
	int verbose = 0;

	char *s = NULL;
	size_t sz = 0;
	ssize_t l;

	while ((c = getopt(argc, argv, "ghors:vx")) != -1) {
		switch (c) {
		case 'o':
			of = "%#jo";
			break;
		case 'x':
			of = "%#jx";
			break;
		case 'r':
			reverse = 1;
			break;
		case 's':
			if (strnlen(optarg, 3) != 2)
				errx(1, "sym must be exactly 2 characters");
			sym = optarg;
			break;
		case 'g':
			gaps = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'h':
			usage(stdout);
			return 0;
		default:
			usage(stderr);
			return 1;
		}
	}

	if ((num = (argc -= optind))) {
		arr = (argv += optind);
	} else {
		for (num = 0, sz = 0; (l = getline(&s, &sz, stdin)) != -1; num++, s = NULL, sz = 0) {
			if (s[l - 1] == '\n')
				s[--l] = '\0';

			if (num == arrsz && (arr = reallocarray(arr, arrsz += STEP, sizeof(char *))) == NULL)
				err(1, NULL);

			arr[num] = s;
		}
	}
	for (i = 0; i < num; i++) {
		if (reverse) {
			mask = bintomask(arr[i]);
			if ((bin = masktobin(mask)) == NULL)
				err(1, "%s", arr[i]);
			if (verbose)
				printf("%s → ", bin);
			printf(of, bintomask(arr[i]));
			putchar('\n');
		} else {
			mask = strtomask(arr[i]);
			if ((bin = masktobin(mask)) == NULL)
				err(1, "%s", arr[i]);
			if (verbose) {
				printf(of, mask);
				printf(" → ");
			}
			printf("%s\n", bin);
		}
		free(bin);
	}
	if (arr != argv)
		free(arr);

	return 0;
}


char *
masktobin(uintmax_t mask)
{
	uint8_t bytes[sizeof(uintmax_t)];
	int i = sizeof(uintmax_t);
	int flags = BSKIP;

	while (i--) {
		bytes[i] = mask & 0xFF;
		mask >>= 8;
	}
	if (gaps)
		flags |= BGROUP;
	return binary(bytes, sizeof(uintmax_t), sym, flags);
}


uintmax_t
bintomask(char *str)
{
	uintmax_t n = 0;
	char *s;

	for (s = str; *s; s++) {
		if (strchr(sym, *s))
			n = (n << 1) | (*s == sym[1]);
		else if (gaps && isspace(*s))
			continue;
		else
			errx(1, "%s: %s", str, strerror(EINVAL));
	}

	return n;
}


uintmax_t
strtomask(char *s)
{
	char *e;
	uintmax_t n;

	n = strtoumax(s, &e, 0);

	if (e == NULL)
		err(1, "%s", s);
	else if (*e != '\0')
		errx(1, "%s: %s", s, strerror(EINVAL));

	return n;
}


void
usage(FILE *f)
{
	(void)fprintf(f, "Usage: %s [-gorvx] [-s sym] ...\n", getprogname());
}
