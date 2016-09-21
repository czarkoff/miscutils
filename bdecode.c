#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <libgen.h>
#include <locale.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <err.h>


#define LOCALE "en_US.UTF-8"

#define MAXDEPTH 20

#define INDENT "  "


void decodef(char *);
void decodein(void);
char *bdecode(char *);
void newout(char *);
void pindent(void);
void pstring(char *, uintmax_t);
void usage(FILE *);


int dict = 0;
int join = 0;
int local = 0;
FILE *out = NULL;
int samename = 0;


int indent = 0;


int
main(int argc, char *argv[])
{
	int was_stdin = 0;
	char c;

	while ((c = getopt(argc, argv, "adhlOo:")) != -1) {
		switch (c) {
		case 'a':
			dict = 0;
			join = 1;
			break;
		case 'd':
			dict = 1;
			join = 1;
			break;
		case 'l':
			local = 1;
			break;
		case 'O':
			samename = 1;
			break;
		case 'o':
			join = 1;
			if (strncmp(optarg, "-", 2) == 0) {
				out = stdout;
				break;
			}
			if ((out = fopen(optarg, "w")) == NULL)
				err(1, "%s", optarg);
			break;
		case 'h':
			usage(stdout);
			return 0;
		default:
			usage(stderr);
			return 1;
		}
	}

	argc -= optind;
	argv += optind;

	if (samename && (out != NULL || join)) {
		usage(stderr);
		return 1;
	}
	if (out == NULL)
		out = stdout;

	if (setlocale(LC_CTYPE, LOCALE) == NULL)
		warn("setlocale(" LOCALE ")");

	if (argc == 0) {
		decodein();
		return 0;
	}

	if (join && argc > 1) {
		if (dict)
			fprintf(out, "{\n");
		else
			fprintf(out, "[\n");
		indent++;
	}

	for (; argc--; argv++) {
		if (dict)
			fprintf(out, "\"%s\": ", *argv);

		if (samename)
			newout(*argv);

		if (strncmp(*argv, "-", 2) == 0) {
			if (was_stdin++ != 0)
				errx(1, "standard input already consumed");
			decodein();
		} else {
			decodef(*argv);
		}
		
		if (join) {
			if (argc) {
				pindent();
				fprintf(out, ",\n");
			} else  {
				if (dict)
					fprintf(out, "}\n");
				else
					fprintf(out, "]\n");
			}
		}
		if (samename)
			fclose(out);
	}
	return 0;
}


void
usage(FILE *stream)
{
	fprintf(stream, "Usage: %s [-ad [-o file] | -Ol] [file ...]\n",
			getprogname());
}


void
decodein(void)
{
	char *buf = NULL, *bufp, *p;
	size_t bufsz = 0;
	ssize_t l;
	static int decoded = 0;

	if (decoded++)
		err(1, "standard input can't be decoded twice");

	for (;;) {
		if (bufsz > SSIZE_MAX - MAX_INPUT)
			errx(1, "reading from stdin: input too big");
		if ((buf = realloc(buf, bufsz + MAX_INPUT)) == NULL)
			err(1, "reading from stdin");
		bufp = buf + bufsz;
		if ((l = read(STDIN_FILENO, bufp, MAX_INPUT)) == -1)
			err(1, "reading from stdin");
		bufsz += l;
		if (l == 0 || l < MAX_INPUT)
			break;
	}
	p = buf;
	do {
		if ((p = bdecode(p)) == NULL)
			err(1, "reading from stdin");
		putc('\n', out);
		while (*p == '\n')
			p++;
	} while (*p != '\0');
	free(buf);
}


void
decodef(char *fname)
{
	int fd;
	void *addr;
	char *p;
	size_t len;
	struct stat sb;

	if ((fd = open(fname, O_RDONLY)) < 0)
		err(1, "%s", fname);

	memset(&sb, 0, sizeof(struct stat));
	if (fstat(fd, &sb) == -1)
		err(1, "%s", fname);
	len = (size_t)sb.st_size;

	addr = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED) {
		warn("%s", fname);
		errno = 0;
		close(fd);
		return;
	}

	p = (char *)addr;
	do {
		if ((p = bdecode(p)) == NULL)
			warn("%s", fname);
		putc('\n', out);
		while (*p == '\n')
			p++;
	} while (*p != '\0');

	if (munmap(addr, len) < 0) {
		warn("%s", fname);
		errno = 0;
	}

	if (close(fd) < 0) {
		warn("%s", fname);
		errno = 0;
	}
}


char *
bdecode(char *p)
{
	char *np;
	uintmax_t len;

	while (*p == '\n')
		p++;

	switch (*p) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		len = strtoumax(p, &np, 10);
		if (*np != ':')
			return NULL;

		pstring(++np, len);

		return np + len;
	case 'i':
		np = p;
		while (isdigit(*(++np)))
			putc(*np, out);

		if (*np == 'e')
			return ++np;

		errno = EINVAL;
		return NULL;
	case 'l':
		putc('[', out);
		np = p + 1;
		while ((np = bdecode(np)) && *np != 'e')
			fprintf(out, ", ");
		putc(']', out);
		return ++np;
	case 'd':
		pindent();
		indent++;
		fprintf(out, "{\n");
		for (np = p + 1; *np != 'e'; fprintf(out, "\n")) {
			pindent();
			np = bdecode(np);
			if (np == NULL || *np == 'e' || *np == '\0')
				return NULL;
			fprintf(out, ": ");
			np = bdecode(np);
			if (np == NULL || *np == '\0')
				return NULL;
			if (*np != 'e')
				putc(',', out);
		}
		np++;
		indent--;
		pindent();
		putc('}', out);
		return np;
	}

	errno = EINVAL;
	return NULL;
}


void
newout(char *inname)
{
	char outfile[PATH_MAX];
	char *p = inname;

	if (local)
		p = basename(inname);

	if (snprintf(outfile, PATH_MAX, "%s.json", p) > PATH_MAX)
		errx(1, "%s: filename too long", p);

	if ((out = fopen(outfile, "w")) == NULL)
		err(1, "%s", outfile);
}


void
pindent(void)
{
	int i = indent;

	while (i--)
		fprintf(out, INDENT);
}


void
pstring(char *s, uintmax_t len)
{
	char *p = s;
	int ulen;
	wchar_t wc;
	uintmax_t tlen = len;

	if (len == 0) {
		fprintf(out, "\"\"");
		return;
	}
	putc('\"', out);
	while (tlen)
		if ((ulen = mbtowc(&wc, p, tlen--)) > 0)
			p += ulen;
		else
			goto invalid;

	for (; len--; s++)
		if (iscntrl(*s))
			fprintf(out, "\\%0hho", *s);
		else
			putc(*s, out);
	putc('\"', out);
	return;

invalid:
	fprintf(out, "0x");
	while (len--)
		fprintf(out, "%0hhX", *s++);
	putc('\"', out);
}
