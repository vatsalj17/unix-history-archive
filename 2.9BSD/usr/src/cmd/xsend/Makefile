CFLAGS = -n -O -s

all:	enroll xsend xget

cp:	all
	cp xsend xget enroll ${DESTDIR}/usr/bin
	rm *.o xsend xget enroll

cmp:	all
	cmp xsend ${DESTDIR}/usr/bin/xsend
	cmp xget ${DESTDIR}/usr/bin/xget
	cmp enroll ${DESTDIR}/usr/bin/enroll
	rm *.o

enroll:	enroll.o lib.o
	cc -n -s enroll.o lib.o -lmp -o enroll
xget:	xget.o lib.o
	cc -n -s xget.o lib.o -lmp -o xget
xsend:	xsend.o lib.o
	cc -n -s xsend.o lib.o -lmp -o xsend
enroll.o xget.o xsend.o lib.o:	xmail.h
list:
	@pr -2 -w124 m.h xmail.h enroll.c xsend.c xget.c lib.c makefile

clean:
	rm -f *.o
