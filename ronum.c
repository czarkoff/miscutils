#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>


/* Roman nerals only cover [0...3999] range */
#define ROMAN_MAX 3999
#define ROMAN_BUF 19 


int roman(char *s, size_t l, unsigned int n, int classic);
int arabic(const char *s);

void readin(void);
void wprintw(char *);
void printw(char *);
void usage(void);


static unsigned int stop[] = {
	1000,
	900, 500, 400, 100,
	90, 50, 40, 10,
	9, 5, 4, 1
};
static char *trans[] = {
	"M",
	"CM", "D", "CD", "C",
	"XC", "L", "XL", "X",
	"IX", "V", "IV", "I" , NULL
};

int ignore = 0;
int classic = 0;
int verbose = 0;
char charset[] = " \t";


int
roman(char *s, size_t l, unsigned int n, int classic)
{
	int sz = 0;
	int i, step;

	if (n > ROMAN_MAX) {
		errno = EINVAL;
		return -1;
	}

	/* 0 has special value */
	if (n == 0)
		return snprintf(s, l, "nulla");

	(void)memset(s, '\0', l);

	step = (classic) ? 2 : 1;
	for (i = 0; n != 0; i += step) {
		while (n >= stop[i]) {
			if ((size_t)sz < l)
				(void)strlcat(s, trans[i], l);
			sz += strnlen(trans[i], 3) - 1;
			n -= stop[i];
		}
	}

	return sz;
}


int
arabic(const char *s)
{
	int num=0;
	int i;
	const char *ps, *pt;

	if (s == NULL || *s == '\0') {
		errno = EINVAL;
		return -1;
	}

	/* "nulla" or "Nulla" means 0, no need to parse anything */
	if (strcasecmp(s, "nulla") == 0)
		return num;

	for (i = 0; trans[i] != NULL; i++) {
		do {
			for (ps = s, pt = trans[i]; *ps == *pt; ps++, pt++)
				if (*ps == '\0')
					break;

			if (*pt == '\0') {
				s = ps;
				num += stop[i];
			}

			if (*s == '\0')
				return num;

		} while (*pt == '\0');
	}
	errno = EINVAL;
	return -1;
}


void
readin(void)
{
	char *line = NULL;
	size_t linesize = 0;
	ssize_t linelen;
	char *p;

	while ((linelen = getline(&line, &linesize, stdin)) != -1)
		for (p = line; p != NULL; printw(strsep(&p, " \t\n")))
			;

	free(line);
	if (ferror(stdin))
		err(1, "standard input");
	fclose(stdin);
}


void
printw(char *word)
{
	char buf[ROMAN_BUF];
	const char *e = NULL;
	int n;
	int l;

	if (word == NULL || *word == '\0')
		return;

	n = (int)strtonum(word, 0, ROMAN_MAX, &e);
	if (n == 0 && e != NULL) {
		if ((n = arabic(word)) < 0) {
			if (ignore)
				return;
			errx(1, "%s: %s", word, e);
		}

		if (verbose)
			printf("%s > ", word);
		printf("%d\n", n);

		return;
	}

	l = roman(buf, ROMAN_BUF, n, classic);
	if (l == -1 || l > ROMAN_BUF)
		err(1, "%s", word);

	if (verbose)
		printf("%s > ", word);
	printf("%s\n", buf);

	return;
}


void
usage(void)
{
	dprintf(STDERR_FILENO, "usage: %s [-c | -m] [-i] [-v] ...\n",
			getprogname());
}


int
main(int argc, char **argv)
{
	char c;
	int printed = 0;

	while ((c = getopt(argc, argv, "chimv")) != -1) {
		switch (c) {
			case 'c':
				classic = 1;
				break;
			case 'h':
				usage();
				return 0;
			case 'i':
				ignore = 1;
				break;
			case 'm':
				classic = 0;
				break;
			case 'v':
				verbose = 1;
				break;
			default:
				usage();
				return 1;
		}
	}

	for (argc -= optind, argv += optind; argc--; argv++) {
		printed = 1;
		if (strncmp(*argv, "-", 2) == 0)
			readin();
		else
			printw(*argv);
	}

	if (!printed)
		readin();

	return 0;
}
