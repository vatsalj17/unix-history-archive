#	Makefile	4.2	83/07/27
#
DESTDIR=

all:
	cd src; make ${MFLAGS}

cp: all
	cd src; make ${MFLAGS} DESTDIR=${DESTDIR} cp
	cd lib; make ${MFLAGS} DESTDIR=${DESTDIR} cp
	cd man; make ${MFLAGS} DESTDIR=${DESTDIR} cp

clean:
	cd src; make ${MFLAGS} clean
