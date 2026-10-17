DEFS=	-DFULLLOGIN -DUCB_NTTY
CFLAGS=	-O -n -s ${DEFS} ${INCLUDES}
INCLUDES=

all:	tset

cp:	tset
	cp tset ${DESTDIR}/usr/ucb
	rm -f ${DESTDIR}/usr/ucb/reset
	ln ${DESTDIR}/usr/ucb/tset ${DESTDIR}/usr/ucb/reset
	rm -f ${DESTDIR}/usr/ucb/RESET
	ln ${DESTDIR}/usr/ucb/tset ${DESTDIR}/usr/ucb/RESET

cmp:	tset
	cmp tset ${DESTDIR}/usr/ucb

tset:	tset.c
	${CC} ${CFLAGS} tset.c -o tset -ltermcap

clean:
	rm -f *.o tset
