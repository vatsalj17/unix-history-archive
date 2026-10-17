D = /usr/bin
L = /usr/lib/pascal

CFLAGS = -O

all: pix px_header pc pc2 pc3 pmerge

install: all pxref.p
	./install.sh $D $L

pix: pix.c
	cc -O -o pix pix.c

pmerge: pmerge.c
	cc -O -o pmerge pmerge.c

px_header: px_header.c objfmt.h
	cc -O -d2 -o px_header px_header.c -N
	strip px_header

pc2: pc2.c
	cc pc2.c -lpc -o tmp
	cc -S pc2.c
	./tmp pc2.s | as -d2 -o pc2.o
	cc pc2.o -o pc2 -s
	rm -f pc2.o pc2.s tmp

pc3: pc3.c pc3.h pstab.h
	cc -O -d2 pc3.c -o pc3 -s

pxref: pxref.p
	pc -w pxref.p -o pxref

pc: pc.c
	cc -O -d2 -o pc pc.c

clean:
	rm -f pix px_header pxref pmerge pc pc2 pc3
