CFLAGS=		-O
FMT=

all:		sed

sed:		sed0.o sed1.o
		cc ${FMT} -o sed $?

sed0.o:		sed0.c sed.h

sed1.o:		sed1.c sed.h

clean:
		rm -f sed *.o

install:	sed
		cp sed ${DESTDIR}/usr/bin/sed
		-strip ${DESTDIR}/usr/bin/sed
