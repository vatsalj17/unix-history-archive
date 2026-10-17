#
#	Ls is the old Bell version.
#	Ucbls is a fancy version designed for crts.
#

G=	32	# max. number of gids recognized (must not be a multiple of 5)
U=	512	# max. number of uids recognized (must not be a multiple of 5)
F=	1024	# max. number of files recognized
CFLAGS=	-O -n
DEFS=	-DNGID=$G -DNUID=$U -DNFILES=$F			# no password hashing
#DEFS=	-DNGID=$G -DNUID=$U -DNFILES=$F -DUCB_PWHASH	# password hashing
DESTDIR=/

all:	ls ucbls

cp:	all
	-strip ls ucbls
	cp ls ${DESTDIR}/bin/ls
	cp ucbls ${DESTDIR}/usr/ucb/ls
	rm -f ${DESTDIR}/usr/ucb/l
	ln ${DESTDIR}/usr/ucb/ls ${DESTDIR}/usr/ucb/l

ls:	ls.c
	cc ${CFLAGS} ls.c -o ls

ucbls:	ucbls.c
	cc ${CFLAGS} ${DEFS} ucbls.c -o ucbls

clean:
	rm -f *.o
