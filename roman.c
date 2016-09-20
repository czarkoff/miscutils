#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "roman.h"


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
