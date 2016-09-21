#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <ctype.h>
#include <unistd.h>
#include "utf8.h"


#define NAMELIST "NamesList.txt"


int
main(void)
{
	size_t sz = 0;
	ssize_t l;
	FILE *in;

	char *s = NULL, *p;
	int64_t cp = -1;
	int noname = 0;
	char *descr = NULL;

	if ((in = fopen(NAMELIST, "r")) == NULL)
		err(1, NAMELIST);

	while ((l = getline(&s, &sz, in)) != -1) {
		if ((p = strchr(s, '\n')))
			*p = '\0';

		if (isxdigit(*s)) {
			if (descr != NULL) {
				if (printf("%lld %s\n", cp, descr) < 0)
					err(1, "stdout", NULL);
				free(descr);
				descr = NULL;
			}

			cp = (int64_t)strtoull(s, &p, 16);
			if (*p != '\t')
				errx(1, "%s: wrong format", s);
			*p++ = '\0';

			while (isspace(*p) && *p != '\0')
				p++;
			if (*p == '\0')
				continue;
			if (*p != '<') {
				descr = strndup(p, l -= s - p);
				noname = 0;
			} else {
				noname = 1;
			}
		} else if (noname && isspace(*s)) {
			for (p = s; isspace(*p); p++)
				if ((long)l <= p - s)
					continue;
			if (*p++ == '=') {
				while (isspace(*p))
					p++;
				if (descr != NULL)
					free(descr);
				descr = strndup(p, l -= s - p);
				noname = 0;
			}
		} else {
			continue;
		}
	}
	if (ferror(in))
		err(1, NAMELIST);
	if (descr != NULL) {
		if (printf("%lld %s\n", cp, descr) < 0)
			err(1, "stdout", NULL);
		free(descr);
	}

	fclose(in);
	return 0;
}
