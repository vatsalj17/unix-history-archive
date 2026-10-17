CFLAGS=	-O -DCM_N -DCM_GT -DCM_B -DCM_D -I${INCLUDES}
INCLUDES=

all: termcap.a ovtermcap.a

cp: termcap.a ovtermcap.a
	cp termcap.a ${DESTDIR}/usr/lib/libtermcap.a
	-rm -f ${DESTDIR}/usr/lib/libtermlib.a
	ln ${DESTDIR}/usr/lib/libtermcap.a ${DESTDIR}/usr/lib/libtermlib.a
	ranlib ${DESTDIR}/usr/lib/libtermcap.a
	cp ovtermcap.a ${DESTDIR}/usr/lib/libovtermcap.a
	-rm -f ${DESTDIR}/usr/lib/libovtermlib.a
	ln ${DESTDIR}/usr/lib/libovtermcap.a ${DESTDIR}/usr/lib/libovtermlib.a
	ranlib ${DESTDIR}/usr/lib/libovtermcap.a

termcap.a: termcap.o tgoto.o tputs.o
	ar cr termcap.a termcap.o tgoto.o tputs.o
	rm termcap.o tgoto.o tputs.o

ovtermcap.a:
	make -f Ovmakefile ovtermcap.a

clean:
	-rm -f *.o

VGRIND=	csh /usr/ucb/vgrind
vgrind:
	cp /dev/null index
	${VGRIND} -h "Termcap library" termcap.c tputs.c tgoto.c
	${VGRIND} -h "Termcap library" -x index
