all:	roff

cp:	roff
	cp roff ${DESTDIR}/bin
	rm roff roff.o

cmp:	roff
	cmp roff ${DESTDIR}/bin/roff
	rm roff.o

roff:	*.s
	as -o roff.o /usr/include/sys.s *.s
	ld -s roff.o -lc -o roff

clean:
	rm -f *.o
