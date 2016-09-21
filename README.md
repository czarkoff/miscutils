This is a set of miscellaneous tools, each of which is too tiny to deserve a
separate repository.  They are also too tiny to deserve a version number,
although that may change over time.

# Tools

* `bdecode` reads benc-encoded input and outputs converted into JSON. If no
  arguments provided on the command line, or special argument ‘-’ is provided,
  `bdecode` reads input from standard input.
* `jcuken` changes input so as if instead of “ru” keyboard layout it was typed
  with “us” keyboard layout. By default, `jcuken` reads standard input.
* `rangecomp` detect and compress ranges of numbers. If no arguments provided
  on the command line `rangecomp` reads numbers from standard input.
* `rme` removes empty files. If no files are specified on command line, all
  files in local directory are verified. If no arguments provided on the command
  line.
* `ronum` converts roman numerals into arabic and vice versa. If no arguments
  provided on the command line, or if one the arguments is ‘-’, `ronum` reads
  numerals from standard input.
* `single` runs a comman and creates a lock, so that consequent attempts to
  run the same program using `single` will fail until command exits.
* `unutf8` decodes UTF-8-encoded text and prints Unicode codepoints, one per
  line. If no arguments provided on the command line, `unutf8` reads text from
  standard input.
* `utf8` displays octal, decimal and/or hexadecimal representation of Unicode
  code point as encoded in UTF-8 encoding. The choice of representation depends
  on flags with which the program is invoced.  If no flags were provided, all
  three representations are shown.

# Building and installing

This projects uses plain Makefiles for all configuration, build and installation
tasks.

By default all of these utilities are build, and all of them and their manuals
are installed to `/usr/local/bin/` and `/usr/local/man/man1/`.  If these options
are unacceptable, the following variable may be used to configure the build:

* `FETCH_CMD` (`curl -OsS` by default) is used to fetch
  "*[NamesList.txt](http://unicode.org/Public/UNIDATA/NamesList.txt)*" file
  from the internet.  If this file is already in place, build system will skip
  downloading it.
* Variable `APPS` controls which applications will be built by default.
  Building particular set of applications may be achieved by either setting this
  variable or listing them as arguments to `make`.
* Build process my be tuned by setting `CC`, `CFLAGS`, `CPPFLAGS`, `LD`,
  `LDFLAGS` and `LIBS`.  All of these variables mean exactly what they mean
  elsewhere around Unixes.
* Locations may be tuned by setting `PREFIX` (`/usr/local` by default), `BINDIR`
  (`${PREFIX}/bin` by default) and `MANDIR` (`${PREFIX}/man` by default).
  `INSTALL_MAN`, `INSTALL_PROGRAM`, `INSTALL_MAN_DIR`, `INSTALL_PROGRAM_DIR`
  are programs used to install manual pages, programs, and directories.

All of these options may be set in file `config.mk`.  This file will not be
tracked by revision control software, so it may be used without worrying about
exposing aspect of your platform when committing.  Using this file instead of
editing `Makefile` is encouraged.

When all things are configured, the tools may be installed by issuing the
following commands:

```
$ make
# make install
```

(Depending on choice of installation location `root` access may be unnecessary.

## Using GNU make

Unfortunately, GNU Project's `make` software derived in its syntax from classic
BSD `make` implementations.  So far, users of GNU `make` have two options:

1. Switch to BSD `make`.
2. Put `.ALLSRC = $^` in `config.mk`.
