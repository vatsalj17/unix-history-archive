CFLAGS=	-n -O -I${INCLUDES} -s
INCLUDES=

all: msgs

cp: msgs
	cp msgs ${DESTDIR}/usr/ucb

cmp: msgs
	cmp msgs ${DESTDIR}/usr/ucb

msgs: msgs.c msgs.h
	cc $(CFLAGS) -o msgs msgs.c -ltermcap

clean:
	-rm -f *.o msgs
