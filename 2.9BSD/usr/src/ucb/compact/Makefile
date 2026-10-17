CFLAGS=-O -I${INCLUDES}
INCLUDES=

all: compact uncompact

cp: compact uncompact
	cp compact ${DESTDIR}/usr/ucb
	cp uncompact ${DESTDIR}/usr/ucb
	cp ccat.sh ${DESTDIR}/usr/ucb/ccat
	chmod 755 ${DESTDIR}/usr/ucb/ccat
	rm compact uncompact *.o

cmp: compact uncompact
	cmp compact ${DESTDIR}/usr/ucb
	cmp uncompact ${DESTDIR}/usr/ucb
	rm *.o

compact: compact.o tree.o
	cc -s -n -O compact.o tree.o -o compact

uncompact: uncompact.o tree.o
	cc -s -n -O uncompact.o tree.o -o uncompact

install:
	install -s compact ${DESTDIR}/usr/ucb
	install -s uncompact ${DESTDIR}/usr/ucb
	cp ccat.sh ${DESTDIR}/usr/ucb/ccat
	chmod 755 ${DESTDIR}/usr/ucb/ccat

clean:
	rm -f compact.o uncompact.o tree.o
