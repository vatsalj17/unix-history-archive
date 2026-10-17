CFLAGS=-O -I${INCLUDES}
INCLUDES=
SOURCES=vfontedpr.c vgrindefs.c regexp.c retest.c
OBJS=vfontedpr retest

all:	$(OBJS)

vfontedpr: vfontedpr.o vgrindefs.o regexp.o
	cc -f -O -o vfontedpr vfontedpr.o vgrindefs.o regexp.o
retest: retest.o regexp.o
	cc -o retest retest.c regexp.o

cp: all
	cp vfontedpr ${DESTDIR}/usr/lib/vfontedpr
	cp vgrind.sh ${DESTDIR}/usr/ucb/vgrind
	chmod +x ${DESTDIR}/usr/ucb/vgrind
	cp vgrindefs.src ${DESTDIR}/usr/lib/vgrindefs
	cp tmac.vgrind ${DESTDIR}/usr/lib/tmac/tmac.vgrind

clean:
	rm -f ${OBJS}

vprintall:
	csh /usr/ucb/vprint ${SOURCES}
