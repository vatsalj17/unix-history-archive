#	%M%	%I%	%E%
#
DESTDIR=

all:	
	cd src; make ${MFLAGS}

cp:
	cd src; make DESTDIR=${DESTDIR} cp
	cd man; make DESTDIR=${DESTDIR} cp

clean:
	cd src; make ${MFLAGS} clean
