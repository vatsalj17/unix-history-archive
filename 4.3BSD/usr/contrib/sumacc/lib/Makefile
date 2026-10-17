DL=/usr/sun/lib
.SUFFIXES: .b .sl
CFLAGS= -O -I../h
CFLAGS68= -I../h
LINTL=libqd.c libos.c libtool.c libpack.c libfast.c libpr.c
STUBS=mactrap.b mem.b misc.b hand.b event.b dlgcut.b pack.b sane.b sanescan.b insane.b elems.b getpargs.b strconv.b prlink.b getind.b

#	turns .s into .b and strips about 700 useless abs symbols
.s.b:
	cc68 -c $<
	ld68 -r -x -T 0 $*.b
	mv b.out $*.b

#	turns .c into .sl for libmake
.c.sl:
	cc68 $(CFLAGS68) -DCODE -S $<
	mv $*.s $*.sl

#	turns .c into .b
.c.b:
	cc68 $(CFLAGS68) -c $<


all:	libmac.a llib-lmac.ln crtmac.b

llib-lmac.ln: $(LINTL)
	lint $(CFLAGS68) -Cmac $(LINTL)
#	mk41lint $(LINTL)

libmact.a: libqd.sl libos.sl libtool.sl libpack.sl libpr.sl libmake
	rm -f libmact.a 
	cat libqd.sl libos.sl libtool.sl libpack.sl libpr.sl | \
		sed -f libmac.sed | libmake libmact.a

libmac.a: libmact.a $(STUBS)
	(cd io; make)
	cp libmact.a libmac.a
	ar rv libmac.a $(STUBS) io/*.b

libmake: libmake.o
	cc -o libmake libmake.o

clean:
	rm -f libmac.a libmact.a *.ln *.o *.b *.sl *.s.b *.s.list
	cd io; make clean

install: 
	-cp llib-lmac.ln /usr/lib/lint
	-cp crtmac.b $(DL)
	-cp libmac.a $(DL)
#	ranlib68 $(DL)/libmac.a	(not used on VAX, but would be nice)
