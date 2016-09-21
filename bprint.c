#include <stdio.h>
#include <errno.h>
#include <string.h>


char dsym[] = {'0', '1'};

char *
bits(char *s, uint8_t b, char sym[2])
{
	char *p;
	int i;

	if (s == NULL) {
		errno = EINVAL;
		return NULL;
	}

	if (sym == NULL)
		sym = dsym;

	p = s;
	for (i = 0; i < 8; i++) {
		*p++ = (b & 0x80) ? sym[1] : sym[0];
		b = b << 1;
	}
	*p = '\0';

	return s;
}


int
unbits(const char *s, char sym[2])
{
	int n = 0;

	if (s == NULL || strnlen(s, 9) != 8) {
		errno = EINVAL;
		return -1;
	}

	if (sym == NULL)
		sym = dsym;

	for (; *s != '\0'; s++) {
		n <<= 1;

		if (*s == sym[1]) {
			n++;
		} else if (*s == sym[0]) {
			;
		} else {
			errno = ERANGE;
			return -1;
		}
	}

	return n;
}
