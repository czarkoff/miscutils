#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <sysexits.h>
#include "args.h"

#define SIZE	100
#define NMAX	100


int getnums(int n, char **pp);
int numcmp(const void *n1, const void *n2);
void reprint(unsigned int *nums, size_t nnums);


int
main(int argc, char **argv)
{
	const char *e;
	unsigned int *pnums;
	unsigned int *nums;
	size_t nnums=0;

	argc--; argv++;
	args_init(&argc, &argv);
	if (argc == -1) {
		args_free();
		err(EX_OSERR, "Failed to get arguments from standard input");
	} else if (argc == 0) {
		args_free();
		errx(EX_USAGE, "No arguments");
	}

	if ((nums = calloc(argc, sizeof(unsigned int))) == NULL) {
		args_free();
		err(EX_OSERR, "Failed to allocate memory");
	}

	pnums = nums;
	while (argc--) {
		*pnums++ = strtonum(*argv++, 0, UINT_MAX, &e);
		nnums++;
		if (e != NULL) {
			args_free();
			err(EX_USAGE, "%s", *(argv - 1));
		}
	}
	reprint(nums, nnums);

	free(nums);
	args_free();
	return EX_OK;
}


void
reprint(unsigned int *nums, size_t nnums)
{
	int seq = 0;

	qsort(nums, nnums, sizeof(unsigned int), numcmp);

	printf("%u", *nums++);

	while (--nnums) {
		for (; *nums == *(nums - 1) + 1; nums++) {
			seq = 1;
			if (--nnums == 0) {
				printf("-%u\n", *nums);
				return;
			}
		}
		if (seq) {
			seq = 0;
			printf("-%u", *(nums - 1));
		}
		printf(",%u", *nums++);
	}
	printf("\n");
}

int
numcmp(const void *n1, const void *n2)
{
	return *(unsigned int *)n1 - *(unsigned int *)n2;
}
