#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "roman.h"


char *
roman(int n)
{
	char *num, *p;
	int sz;
	int stop[] = { 1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1 };
	char *trans[] = {
		"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I" 
	};
	int i;

	if (n < 0 || n > ROMAN_MAX) {
		errno = EINVAL;
		return NULL;
	}

	/* 0 has special value */
	if (n == 0) {
		asprintf(&num, "nulla");
		return num;
	}

	if ((num = malloc(1)) == NULL)
		return NULL;
	*num = '\0';

	for (i = 0; n != 0; i++) {
		while (n >= stop[i]) {
			sz = asprintf(&p, "%s%s", num, trans[i]);
			free(num);
			num = p;

			n -= stop[i];

			if (sz == -1)
				return NULL;
		}
	}

	return num;
}


int
arabic(const char *s)
{
	int num=0;
	const char *p;
	int cur, last=0;

	if (s == NULL || *s == '\0') {
		errno = EINVAL;
		return -1;
	}

	/* "nulla" or "Nulla" means 0, no need to parse anything */
	if (strcasecmp(s, "nulla") == 0)
		return num;

	/* skip to the end */
	for (p = s; *p != '\0'; p++)
		;

	do {
		switch (toupper(*(--p))) {
			case 'I':
				cur = 1;
				break;
			case 'V':
				cur = 5;
				break;
			case 'X':
				cur = 10;
				break;
			case 'L':
				cur = 50;
				break;
			case 'C':
				cur = 100;
				break;
			case 'D':
				cur = 500;
				break;
			case 'M':
				cur = 1000;
				break;
			default:
				errno = EINVAL;
				return -1;
		}

		if (cur < last)
			num -= cur;
		else 
			num += cur;

		last = cur;
	} while (p != s);

	return num;
}
