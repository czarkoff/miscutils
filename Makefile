CC                   ?= clang
LD                   := ${CC}
CFLAGS               ?= -Wall -Wextra
CFLAGS               += -std=c99 -pedantic
INSTALL_MAN          ?= install -c -m 644
INSTALL_PROGRAM      ?= install -c -m 755
INSTALL_MAN_DIR      ?= install -d -m 755
INSTALL_PROGRAM_DIR  ?= install -d -m 755
PREFIX               ?= /usr/local
BINDIR               ?= ${PREFIX}/bin
MANDIR               ?= ${PREFIX}/man

sinclude config.mk

APPS ?=	bdecode jcuken rangecomp rme ronum ptc single unutf8 utf8

.MAIN: ${APPS}

.PHONY: clean install uninstall info

.SUFFIXES: .c .o .h

.c.o:
	${CC} -c ${CFLAGS} ${CPPFLAGS} $<

.o:
	${LD} -o $@ ${LDFLAGS} ${.ALLSRC} ${LIBS}

bdecode: bdecode.o
jcuken: jcuken.o
rangecomp: rangecomp.o
rme: rme.o
ronum: ronum.o
ptc: ptc.o
single: single.o
unutf8: unutf8.o
utf8: utf8.o

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
	rm -Rf *.o *.core ${APPS}

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
