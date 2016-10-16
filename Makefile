sinclude config.mk

CC                   ?= cc
LD                   := ${CC}
CFLAGS               ?= -O2 -pipe -Wall -Wextra
CFLAGS               += -std=c99 -pedantic
INSTALL_MAN          ?= install -c -m 644
INSTALL_PROGRAM      ?= install -c -m 755
INSTALL_MAN_DIR      ?= install -d -m 755
INSTALL_PROGRAM_DIR  ?= install -d -m 755
PREFIX               ?= /usr/local
BINDIR               ?= ${PREFIX}/bin
MANDIR               ?= ${PREFIX}/man
FETCH_CMD            ?= curl -OsS
UCD_URL              ?= http://unicode.org/Public/UNIDATA/NamesList.txt
EXTRA                := uniname uniname.h uniname.tmp
APPS                 ?= bdecode bitmask jcuken rangecomp rme ronum ptc single \
			unutf8 urldecode utf8


NAMED_APPS =
.for t in ${.TARGETS}
. if ${APPS:M$t}
NAMED_APPS += $t
. endif
.endfor
.if !empty(NAMED_APPS)
APPS := ${NAMED_APPS}
.endif


.MAIN: ${APPS}

.PHONY: clean install uninstall info

.SUFFIXES: .c .o .h

.c.o:
	${CC} -c ${CFLAGS} ${CPPFLAGS} $<

.o:
	${LD} -o $@ ${LDFLAGS} ${.ALLSRC} ${LIBS}

bdecode: bdecode.o
bitmask: binary.o bitmask.o
jcuken: jcuken.o
rangecomp: rangecomp.o
rme: rme.o
ronum: ronum.o
ptc: ptc.o
single: single.o
uniname: uniname.o
unutf8: unutf8.o
urldecode: urldecode.o
utf8: binary.o utf8.o

binary.c bitmask.c utf8.c: binary.h
uniname.c unutf8.c utf8.c: utf8.h
unutf8.c utf8.c: uniname.h

uniname.tmp: NamesList.txt uniname
	./uniname > uniname.tmp
	
uniname.h: NamesList.txt uniname.tmp
	echo   '/*'                                                 > uniname.h
	echo   ' * This file is generated automatically.'          >> uniname.h
	echo   ' *'                                                >> uniname.h
	echo   ' * Do NOT edit.'                                   >> uniname.h
	echo   ' */'                                               >> uniname.h
	echo   '#include <limits.h>'                               >> uniname.h
	echo                                                       >> uniname.h
	echo   'uint32_t unikey[] = {'                             >> uniname.h
	sed -r 's/^([0-9]+) .+/	\1,/' uniname.tmp                  >> uniname.h
	echo   '	UINT32_MAX'                                >> uniname.h
	echo   '};'                                                >> uniname.h
	echo                                                       >> uniname.h
	echo   'char *univalue[] = {'                              >> uniname.h
	sed -r 's/^[0-9]+ (.+)/	\"\1\",/' uniname.tmp              >> uniname.h
	echo   '	"error"'                                   >> uniname.h
	echo   '};'                                                >> uniname.h

NamesList.txt:
	${FETCH_CMD} ${UCD_URL}

${BINDIR}:
	${INSTALL_PROGRAM_DIR} ${BINDIR}

${MANDIR}:
	${INSTALL_MAN_DIR} ${MANDIR:=/man1}

install: $(BINDIR) $(MANDIR) $(APPS)
	${INSTALL_PROGRAM} ${APPS} ${BINDIR}
	${INSTALL_MAN} ${APPS:=.1} ${MANDIR:=/man1}

uninstall:
	rm -f ${APPS:%=${BINDIR}/%} ${APPS:%=${MANDIR}/man1/%.1}

clean:
	rm -Rf *.o *.core ${APPS} ${EXTRA}

info:
	@echo "APPS:                 ${APPS}"
	@echo "CC:                   ${CC}"
	@echo "CFLAGS:               ${CFLAGS}"
	@echo "CPPFLAGS:             ${CPPFLAGS}"
	@echo "LD:                   ${LD}"
	@echo "LDFLAGS:              ${LDFLAGS}"
	@echo "LIBS:                 ${LIBS}"
	@echo "INSTALL_MAN:          ${INSTALL_MAN}"
	@echo "INSTALL_PROGRAM:      ${INSTALL_PROGRAM}"
	@echo "INSTALL_MAN_DIR:      ${INSTALL_MAN_DIR}"
	@echo "INSTALL_PROGRAM_DIR:  ${INSTALL_PROGRAM_DIR}"
	@echo "PREFIX:               ${PREFIX}"
	@echo "BINDIR:               ${BINDIR}"
	@echo "MANDIR:               ${MANDIR}"
	@for app in ${APPS}; do \
		[ -r $${app}.1 ] || echo "  Manual for $$app missing."; \
	done
