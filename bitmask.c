#include <limits.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>


#define BUFMUL sizeof(uintmax_t)
#define BUFSZ BUFMUL * 9
#define STEP 16


char *binary(char *, uintmax_t);
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
	char buf[BUFSZ];
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
			if (verbose)
				printf("%s → ", expand(buf, arr[i]));
			printf(of, bintomask(arr[i]));
			putchar('\n');
		} else {
			if (verbose) {
				printf(of, strtomask(arr[i]));
				printf(" → ");
			}
			printf("%s", binary(buf, strtomask(arr[i])));
			putchar('\n');
		}
	}
	if (arr != argv)
		free(arr);

	return 0;
}


char *
binary(char *dst, uintmax_t src)
{
	char *p;
	uintmax_t n;
	size_t len;

	for (n = 1, p = dst; n < src; n <<= 8)
		p += (gaps ? 9 : 8);
	if (gaps)
		p--;
	for (*p-- = '\0', len = 0; src; src >>= 1, len++) {
		if (gaps && len % 8 == 0 && len != 0)
			*p-- = ' ';
		*p-- = sym[src & 1];
	}

	while (p >= dst)
		*p-- = sym[0];

	return dst;
}


char *
expand(char *dst, char *src)
{
	char *s = src, *d = dst;
	size_t len;

	for (s = src, len = 0; *s; s++) {
		if (strchr(sym, *s))
			len++;
		else if (gaps && isspace(*s))
			continue;
		else
			errx(1, "%s: %s", src, strerror(EINVAL));
	}
	if (len == 0)
		errx(1, "%s: %s", src, strerror(EINVAL));

	for (d = dst; len % 8; len++, *d++ = sym[0]);

	for (s = src; *s; s++) {
		if (strchr(sym, *s) == NULL)
			continue;
		*d++ = *s;
		if (--len == 0)
			break;
		else if (gaps && len % 8 == 0) {
			*d++ = ' ';
		}
	}
	*d = '\0';

	return dst;
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
