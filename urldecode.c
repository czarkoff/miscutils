#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <err.h>


#define num(x) ((x > '9') ? (x - 'A' + 10) : (x - '0'))
int
main(void)
{
	char *line = NULL;
	size_t sz = 0;
	ssize_t len;
	int c;
	char *s;

	while ((len = getline(&line, &sz, stdin)) != -1) {
		s = line;
		while (*s != '\0') {
			if (*s == '%' &&
			    isxdigit(*(s + 1)) &&
			    isxdigit(*(s + 2))) {
				c = (num(*(s + 1)) << 4) + num(*(s + 2));
				putchar(c);
				s += 3;
			} else {
				putchar(*s);
				s++;
			}
		}
	}
	free(line);
	return 0;
}
