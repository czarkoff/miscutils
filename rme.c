#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>


enum {
	NO = 0,
	IMPLICIT,
	YES
};

int dirs =	NO;
int files =	IMPLICIT;
int links =	NO;
int noop =	NO;
int recurse =	NO;
int verbose =	NO;


static void usage(int);
static ssize_t rm_empty(const char *, int);
static int rm(const char *path, int isdir);


int
main(int argc, char *argv[])
{
	int c;

	while ((c = getopt(argc, argv, "dfhlnRv")) != -1) {
		switch (c) {
			case 'd':
				dirs = YES;
				if (files == -1)
					files = 0;
				break;
			case 'f':
				files = YES;
				break;
			case 'l':
				links = YES;
				break;
			case 'R':
				recurse = YES;
				break;
			case 'n':
				noop = YES;
				/* FALLTHROUGH */
			case 'v':
				verbose = YES;
				break;
			case 'h':
				usage(0);
			default:
				return 1;
		}
	}

	if ((argc -= optind) == 0) {
		if (rm_empty(".", 0) == -1)
			err(1, NULL);
	} else {
		for (argv += optind; argc--; argv++)
			if (rm_empty(*argv, 0) == -1)
				err(1, "%s", *argv);
	}

	return 0;
}


static void
usage(int status)
{
	fprintf(stderr, "usage: %s [-dfnRsv | -h]\n", getprogname());
	exit(status);
}


static ssize_t 
rm_empty(const char *path, int level)
{
	DIR *dir;
	struct dirent *dent;
	struct stat sb;
	char buf[PATH_MAX];
	ssize_t len, dirsz, ret;

	if (fstatat(AT_FDCWD, path, &sb, AT_SYMLINK_NOFOLLOW) == -1)
		return -1;

	if (S_ISREG(sb.st_mode) && files) {
		if (sb.st_size == 0)
			return rm(path, 0);
		else
			return 1;
	} else if (S_ISDIR(sb.st_mode) && (dirs || recurse)) {
		dirsz = 0;
		dir = opendir(path);
		while ((dent = readdir(dir)) != NULL) {
			if (strncmp(dent->d_name, ".", MAXNAMLEN) == 0 ||
			    strncmp(dent->d_name, "..", MAXNAMLEN) == 0)
				continue;

			if (!recurse)
				return 1;

			snprintf(buf, PATH_MAX, "%s/%s", path, dent->d_name);
			if ((ret = rm_empty(buf, level + 1)) == -1)
				return -1;

			dirsz += ret;
		}
		closedir(dir);

		if (dirs && dirsz == 0)
			return rm(path, 1);

		return dirsz;
	} else if (S_ISLNK(sb.st_mode) && links) {
		if ((len = readlink(path, buf, sizeof(buf))) > 0) {
			buf[len] = '\0';
			if (fstatat(AT_FDCWD, path, &sb, 0) == -1)
				return rm(path, 0);
		}
	}

	return 1;
}

static int
rm(const char *path, int isdir)
{
	const char *p = path;
	if (verbose) {
		printf("rm");
		if (noop || isdir) {
			printf(" -");
			if (isdir)
				printf("d");
			if (noop)
				printf("i");
		}
		printf(" -- ");

		for (p = path; *p == '\''; p++)
			printf("\\\'");

		putchar('\'');
		while (*p != '\0')
			if (putchar(*p++) == '\'')
				printf("\\\'\'");
		putchar('\'');
		putchar('\n');
	}

	if (noop)
		return 0;

	return (isdir ? rmdir(path) : unlink(path));
}
