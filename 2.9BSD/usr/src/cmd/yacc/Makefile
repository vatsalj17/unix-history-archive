CFLAGS=-O

all:	yacc
	rm *.o

cmp:	all
	cmp yacc ${DESTDIR}/usr/bin/yacc

cp:	all
	-strip yacc
	cp yacc ${DESTDIR}/usr/bin/yacc
	rm yacc *.o

yacc: y1.o y2.o y3.o y4.o
	cc -i -s -o yacc y1.o y2.o y3.o y4.o
	size yacc

y1.o y2.o y3.o y4.o: dextern files

clean:
	rm -f *.o
