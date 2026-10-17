# @(#)Makefile	4.2 (Berkeley) 8/2/82

DESTDIR	=
CFLAGS=	-O 

OBJS=	closedir.o opendir.o readdir.o seekdir.o telldir.o

libndir.a: ${OBJS}
	ar rv libndir.a ${OBJS}
	ranlib libndir.a

${DESTDIR}/usr/include/dir.h: dir.h
	cp dir.h ${DESTDIR}/usr/include/dir.h

install: libndir.a
	cp libndir.a ${DESTDIR}/usr/lib/libndir.a
	cp directory.3 /usr/man/man3/directory.3
	cp dir.h ${DESTDIR}/usr/include/dir.h

clean:
	rm -f libndir.a ${OBJS}

.c.o:
	${CC} ${CFLAGS} -c $*.c

closedir.o:	closedir.c dir.h
opendir.o:	opendir.c dir.h
readdir.o:	readdir.c dir.h
seekdir.o:	seekdir.c dir.h
telldir.o:	telldir.c dir.h
