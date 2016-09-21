/*
 * Copyright (c) 2016 Dmitrij D. Czarkoff <czarkoff@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/event.h>
#include <sys/time.h>
#include <errno.h>
#include <err.h>


#ifndef PATH_MAX
#define PATH_MAX _POSIX_PATH_MAX
#endif
#define LOCKDIR "/tmp/single"
#define MODE    S_IRWXU | S_IRWXG | S_IRWXO


extern char *__progname;


static void usage(int);
static int mklockdir(void);
static int mklockfile(const char *);
static int waitfile(const char *);


static char waitlock = 0;
static char verbose = 0;
static char *exe = NULL;


static void
usage(int ec)
{
  FILE *stream;
  stream = ec ? stderr : stdin;
  fprintf(stream, "usage: %s [-vw] [-d lockdir] [-f lockfile] [--] command\n", getprogname());
  exit(ec);
}


static int
mklockdir(void)
{
  mode_t mask;

  mask = umask(0);

  if (mkdir(LOCKDIR, MODE) && errno != EEXIST)
    return 1;

  (void)umask(mask);

  return 0;
}


static int
mklockfile(const char *fn)
{
  int fd;
  struct stat sb;

  if ((fd = open(fn, O_RDONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) == -1) {
    if (errno == EEXIST) {
      errno = 0;
      if (waitlock) {
        if (verbose)
          fprintf(stderr, "waiting for %s to unlock... ", exe);
        if (waitfile(fn))
          return EX_OSERR;
        if (verbose)
          fprintf(stderr, "done\n");
        return mklockfile(fn);
      } else {
        if (verbose) {
          if (stat(fn, &sb) != 0)
            return EX_OSERR;

          fprintf(stderr, "%s is locked since %s", exe,
              ctime(&(sb.st_mtim.tv_sec)));
        }
        return EX_UNAVAILABLE;
      }
    } else
      return EX_OSERR;
  }

  if (close(fd))
    return EX_IOERR;

  return EX_OK;
}


static int
waitfile(const char *fn)
{
  struct kevent ev, nev;
  int fd, kq;

  if ((fd = open(fn, O_RDONLY)) == -1) {
    if (errno == ENOENT)
      return EX_OK;
    else
      return EX_OSERR;
  }

  if ((kq = kqueue()) == -1)
    return EX_OSERR;

  EV_SET(&ev, fd, EVFILT_VNODE, EV_ADD | EV_ENABLE, NOTE_DELETE, 0, NULL);

  if (kevent(kq, &ev, 1, &nev, 1, NULL) <= 0 || nev.fflags != NOTE_DELETE)
      return EX_OSERR;

  if (close(fd))
    errno = 0;

  return EX_OK;
}


int
main(int argc, char **argv)
{
  extern char *optarg;
  extern int optind;

	pid_t pid;
  int ret = 0;
  int status;
  char dir[PATH_MAX] = LOCKDIR, fn[PATH_MAX] = "";
  char opt;

  while ((opt = getopt(argc, argv, "f:hvw")) != -1) {
    switch (opt) {
      case 'd':
        (void)strlcpy(dir, optarg, PATH_MAX);
        break;
      case 'f':
        (void)strlcpy(fn, optarg, PATH_MAX);
        break;
      case 'h':
        usage(EX_OK);
        break;
      case 'v':
        verbose = 1;
        break;
      case 'w':
        waitlock = 1;
        break;
      default:
        usage(EX_USAGE);
        break;
    }
  }

  argc -= optind;
  argv += optind;

  if (argc < 1)
    usage(EX_USAGE);

  exe = *argv;

  if (*fn == '\0') {
    if (snprintf(fn, PATH_MAX, "%s/%s.lock", LOCKDIR, basename(exe)) >= PATH_MAX)
      errx(EX_OSERR, "path to lock file too long");
  }

  if (mklockdir())
    err(EX_OSERR, "can't create subdirectory in /tmp");

  switch (ret = mklockfile(fn)) {
    case EX_OK:
      break;
    case EX_UNAVAILABLE:
      return EX_UNAVAILABLE;
    case EX_OSERR:
      err(ret, NULL);
      break;
    case EX_IOERR:
      warn(NULL);
      goto out;
    default:
      errx(EX_SOFTWARE, "this should not have happened");
  }

  switch(pid = fork()) {
  case -1:			/* error */
    warn(NULL);
    break;
  case 0:				/* child */
    execvp(exe, argv);
    ret = (errno == ENOENT) ? EX_USAGE : EX_OSERR;
    err(ret, "failed to run %s", exe);
  }

  /* parent */
  (void)signal(SIGINT, SIG_IGN);
  (void)signal(SIGQUIT, SIG_IGN);
  wait(&status);

out:
  if (unlink(fn)) {
    warn(NULL);
    ret = EX_OSERR;
  }

  return ret;
}


/* vim:set tw=78 sts=2 ts=2 sw=2 et ci: */
