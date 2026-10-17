CFLAGS=-n -s -O

all:	tp
	rm *.o

cp:	tp
	cp tp ${DESTDIR}/usr/bin/tp
	rm *.o tp

cmp:	tp
	cmp tp ${DESTDIR}/usr/bin/tp
	rm *.o

tp:	tp0.o tp1.o tp2.o tp3.o
	cc $(CFLAGS) tp0.o tp1.o tp2.o tp3.o -o tp

tp0.c tp1.c tp2.c tp3.c: tp.h

clean:
	rm -f *.o
