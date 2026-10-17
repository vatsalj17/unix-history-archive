all:	nroff troff

nroff:
	make -f nmake
	rm *.o

troff:
	make -f tmake
	rm *.o

cp:	all
	-strip nroff troff
	cp nroff troff ${DESTDIR}/usr/bin
	rm nroff troff

cmp:	all
	cmp nroff ${DESTDIR}/usr/bin/nroff
	cmp troff ${DESTDIR}/usr/bin/troff

clean:
	rm -f *.o
