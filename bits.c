#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <sysexits.h>
#include "args.h"
#include "bprint.h"


int
main(int argc, char **argv)
{
	char s[9], *p;

	argc--; argv++;

	args_init(&argc, &argv);

	if (argc == -1) {
		args_free();
		err(EX_OSERR, "Failed to get arguments from standard input");
	}

	for (; argc > 0; argc--, argv++)
		for (; **argv != '\0'; (*argv)++)
			if ((p = bits(s, (uint8_t) **argv, NULL)) != NULL)
				printf("%s\n", p);
			else
				warn("\"%hhd\"", **argv);

	args_free();

	return EX_OK;
}
