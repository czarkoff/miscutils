#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>


#define MAXLEN 1048576
#define MAXDEPTH 20

#define INDENT "  "


char *bdecode(char *p);
void pindent(void);
void pstring(char *s, uintmax_t len);


extern char *__progname;

int indent = 0;


int
main(int argc, char *argv[])
{
	int fd;
	void *addr;
	char *p;

	while (--argc) {
		if ((fd = open(*(++argv), O_RDONLY)) < 0) {
			fprintf(stderr, "%s: %s: %s\n", __progname, *argv, strerror(errno));
			errno = 0;
			continue;
		}
		
		addr = mmap(NULL, MAXLEN, PROT_READ, MAP_PRIVATE, fd, 0);
		if (addr == MAP_FAILED) {
			fprintf(stderr, "%s: %s: %s\n", __progname, *argv, strerror(errno));
			errno = 0;
			close(fd);
			continue;
		}

		p = (char *)addr;
		do {
			if ((p = bdecode(p)) == NULL)
				fprintf(stderr, "%s: %s: %s\n", __progname, *argv, strerror(errno));
			printf("\n");
			while (*p == '\n')
				p++;
		} while (*p != '\0');

		if (munmap(addr, MAXLEN) < 0) {
			fprintf(stderr, "%s: %s: %s\n", __progname, *argv, strerror(errno));
			errno = 0;
		}

		if (close(fd) < 0) {
			fprintf(stderr, "%s: %s: %s\n", __progname, *argv, strerror(errno));
			errno = 0;
		}
	}

	return 0;
}


char *
bdecode(char *p)
{
	char *np;
	uintmax_t len;

	while (*p == '\n')
		p++;

	if (isdigit(*p)) {
		if ((len = strtoumax(p, &np, 10)) == 0 || *np != ':')
			return NULL;

		pstring(++np, len);

		return np + len;
	} else if (*p == 'i') {
		np = p;
		while (isdigit(*(++np)))
			putchar(*np);

		if (*np == 'e')
			return ++np;

		errno = EINVAL;
		return NULL;
	} else if (*p == 'l') {
		printf("[");
		np = p + 1;
		while ((np = bdecode(np)) && *np != 'e')
			printf(", ");
		printf("]");
		return ++np;
	} else if (*p == 'd') {
		pindent();
		indent++;
		printf("{\n");
		for (np = p + 1; *np != 'e'; printf("\n")) {
			pindent();
			if ((np = bdecode(np)) == NULL || *np == 'e' || *np == '\0')
				return NULL;
			printf(": ");
			if ((np = bdecode(np)) == NULL || *np == '\0')
				return NULL;
			if (*np != 'e')
				printf(",");
		}
		np++;
		indent--;
		pindent();
		printf("}");
		return np;
	}

	errno = EINVAL;
	return NULL;
}


void
pindent(void)
{
	int i = indent;

	while (i--)
		printf(INDENT);
}


void
pstring(char *s, uintmax_t len)
{
	char *p;
	char ulen = 0;
	uintmax_t tlen = len;

	printf("\"");
	for (p=s; tlen--; p++)
		if (isascii(*p) && !ulen) {
			;
		} else if (!ulen) {
			if ((*p & 0xf0) == 0xf0)
				ulen = 4;
			else if ((*p & 0xf0) == 0xe0)
				ulen = 3;
			else if ((*p & 0xe0) == 0xc0)
				ulen = 2;
			else
				goto invalid;
			ulen--;
		} else if ((*p & 0xc0) == 0x80 && ulen--) {
			;
		} else {
			goto invalid;
		}

	if (ulen)
		goto invalid;

	for (; len--; s++)
		if (iscntrl(*s))
			printf("\\%0hho", *s);
		else
			printf("%c", *s);
	printf("\"");
	return;

invalid:
	printf("0x");
	while (len--)
		printf("%0hhX", *s++);
	printf("\"");
}
