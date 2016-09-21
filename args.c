#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "args.h"


#define out() {*argc = -1; args_free(); return;}


char *buf = NULL;
char **pool = NULL;


void
args_raw_init(int *argc, char ***argv)
{
	(*argc)--;
	(*argv)++;

	args_init(argc, argv);
}


void
args_init(int *argc, char ***argv)
{
	char *bufp, **poolp;
	char *s;
	size_t sz = 0, l;
	int i = 0;

	if (*argc > 0)
		return;

	while ((s = fgetln(stdin, &l)) != NULL) {
		if (*(s + l - 1) != '\n')
			l++;

		if (sz + l < sz) /* overflow */
			break;

		if ((bufp = reallocarray(buf, sz + l, sizeof(*buf))) == NULL)
			out();
		buf = bufp;

		bufp = buf + sz;
		strlcpy(bufp, s, l);
		sz += l;

		i++;
	}

	if ((pool = calloc(i, sizeof(char *))) == NULL)
		out();

	poolp = pool;
	bufp = buf;

	do {
		*(poolp++) = bufp;
		bufp += strlen(bufp) + 1;
	} while (++(*argc) < i);

	*argv = pool;
}


void args_free(void)
{
	free(buf);
	free(pool);
}
