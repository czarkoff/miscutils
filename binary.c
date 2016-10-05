#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include "binary.h"


char *
binary(const uint8_t *src, size_t srclen, const char *sym, int flags)
{
	uint8_t n;
	char *dst, *p;
	int i;

	if (sym == NULL)
		sym = DEFAULT_SYM;
	if (flags & BSKIP)
		for (; srclen > 0 && *src == 0; srclen--, src++)
			;

	if (srclen > SIZE_MAX / (flags & BGROUP ? 9 : 8) || srclen == 0) {
		errno = ERANGE;
		return NULL;
	}
	dst = malloc(srclen * 8 + (flags & BGROUP ? srclen : 1));
	if (dst == NULL)
		return NULL;

	for (p = dst;; src++) {
		for (i = 0, n = *src; i < 8; i++, n >>= 1)
			p[7 - i] = sym[n & 01];
		p += 8;

		if (--srclen == 0)
			break;

		if (flags & BGROUP)
			*p++ = ' ';
	}
	*p = '\0';

	return dst;
}
