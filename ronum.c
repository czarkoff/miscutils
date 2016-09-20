#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <err.h>
#include "roman.h"


void readin(void);
void wprintw(char *);
void printw(char *);
void usage(void);


int ignore = 0;
int classic = 0;
int verbose = 0;
char charset[] = " \t";


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
