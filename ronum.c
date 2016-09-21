#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include "args.h"
#include "roman.h"


void usage(void);


int
main(int argc, char **argv)
{
	int verbose = 0;
	char *e, *p;
	int n;

	argc--; argv++;
	if (argc) {
		if (strcmp("-h", *argv) == 0) {
			usage();
			return EX_OK;
		} else if (strcmp("-v", *argv) == 0) {
			verbose = 1;
			argc--; argv++;
		}
	}

	args_init(&argc, &argv);

	for (; argc--; argv++) {
		if ((n = strtol(*argv, &e, 0)) == 0 && *argv == e) {
			if ((n = arabic(*argv)) < 0) {
				args_free();
				err(EX_USAGE, "%s", *argv);
			}
			asprintf(&p, "%d", n);
		} else {
			if ((p = roman(n)) == NULL) {
				args_free();
				err(EX_USAGE, "%s", *argv);
			}
		}
		if (verbose)
			printf("%s > ", *argv);
		printf("%s\n", p);
		free(p);
	}

	args_free();
	return EX_OK;
}


void
usage(void)
{
	printf("%s [-v] ...\n", getprogname());
}
