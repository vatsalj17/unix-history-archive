ranm:		ranm.o ovranm.o

ranm.o:		ranm.s f77.s
		cat f77.s ranm.s > temp.s
		as -o ranm.o temp.s
		rm temp.s

ovranm.o:	ranm.s f77.s
		cat f77.s ranm.s > temp.s
		as -V -o ovranm.o temp.s
		rm temp.s

cp:		ranm.o ovranm.s
		ar rv ${DESTDIR}/usr/lib/libm.a ranm.o
		ranlib ${DESTDIR}/usr/lib/libm.a
		ar rv ${DESTDIR}/usr/lib/libovm.a ovranm.o
		ranlib ${DESTDIR}/usr/lib/libovm.a

clean:
		rm -f *.o temp.s
