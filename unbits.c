#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <sysexits.h>
#include "args.h"
#include "bprint.h"


char *ascii[] = {
	"nul", "soh", "stx", "etx", "eot", "enq", "ack", "bel",
	"bs",  "ht",  "nl",  "vt",  "np",  "cr",  "so",  "si",
	"dle", "dc1", "dc2", "dc3", "dc4", "nak", "syn", "etb",
	"can", "em",  "sub", "esc", "fs",  "gs",  "rs",  "us",
	"sp",  "!",   "\"",  "#",   "$",   "%",   "&",   "\'",
	"(",   ")",   "*",   "+",   ",",   "-",   ".",   "/",
	"0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
	"8",   "9",   ":",   ";",   "<",   "=",   ">",   "?",
	"@",   "A",   "B",   "C",   "D",   "E",   "F",   "G",
	"H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
	"P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
	"X",   "Y",   "Z",   "[",   "\\",  "]",   "^",   "_",
	"`",   "a",   "b",   "c",   "d",   "e",   "f",   "g",
	"h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
	"p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
	"x",   "y",   "z",   "{",   "|",   "}",   "~",   "del"
};


int
main(int argc, char **argv)
{
	int n;

	argc--; argv++;

	args_init(&argc, &argv);

	if (argc == -1) {
		args_free();
		err(EX_OSERR, "Failed to get arguments from standard input");
	}

	errno = EINVAL;
	for (; argc > 0; argc--, argv++)
		if (strlen(*argv) == 8 && (n = unbits(*argv, NULL)) != -1) {
				printf("%s: %3o %3u %#4x", *argv, n, n, n);
				if (n < 0200)
					printf(" %s\n", ascii[n]);
				else
					printf(" not in ASCII range\n");
		} else {
			err(EX_USAGE, "\"%s\"", *argv);
		}

	args_free();

	return EX_OK;
}
