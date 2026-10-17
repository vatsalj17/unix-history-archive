#  Makefile for C libraries
#
#  This makefile does not check dependencies, but only runs the correct
#  scripts.  For machines with floating point hardware, do a "make fplib",
#  which generates libc.a and libovc.a, then "make fpinstall".
#  (libnonfpc.a is still used by the bootstrap and standalones.)
#  Without floating point, "make nonfplib" will make libnonfpc.a and
#  libovnonfpc.a, and "make nonfpinstall" will install them by the more
#  normal names.


fplib:		libc.a libovc.a
nonfplib:	libnonfpc.a libovnonfpc.a

libc.a:
	compall
	mklib
	rm *.o

libnonfpc.a:
	fcompall
	fmklib
	rm *.o

libovc.a:
	ovcompall
	ovmklib
	rm *.o

libovnonfpc.a:
	fovcompall
	fovmklib
	rm *.o

fpinstall: libc.a libovc.a
	cp libc.a ${DESTDIR}/lib/libc.a
	chown bin.bin ${DESTDIR}/lib/libc.a
	ranlib ${DESTDIR}/lib/libc.a
	cp libovc.a ${DESTDIR}/usr/lib/libovc.a
	chown bin.bin ${DESTDIR}/usr/lib/libovc.a
	ranlib ${DESTDIR}/usr/lib/libovc.a
	rm libc.a libovc.a

nonfpinstall: libnonfpc.a libovnonfpc.a
	cp libnonfpc.a ${DESTDIR}/lib/libc.a
	-rm -f ${DESTDIR}/lib/libnonfpc.a
	ln ${DESTDIR}/lib/libc.a ${DESTDIR}/lib/libnonfpc.a
	chown bin.bin ${DESTDIR}/lib/libc.a
	ranlib ${DESTDIR}/lib/libc.a
	cp libovnonfpc.a ${DESTDIR}/usr/lib/libovc.a
	chown bin.bin ${DESTDIR}/usr/lib/libovc.a
	ranlib ${DESTDIR}/usr/lib/libovc.a
	rm libnonfpc.a libovnonfpc.a
