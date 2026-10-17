
#	%M%	%I%	%E%
#
DESTDIR=

all:	getsyspr.c
	cc -O -n -o getsyspr getsyspr.c

cp:	all
	cp getsyspr ${DESTDIR}/usr/new
	strip ${DESTDIR}/usr/new/getsyspr

clean:
	rm -f getsyspr
