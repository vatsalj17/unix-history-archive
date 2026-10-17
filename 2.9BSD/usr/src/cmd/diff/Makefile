DIFF=	/bin/diff
DIFFH=	/usr/lib/diffh
PR=	/bin/pr
CFLAGS=	-O -DDIFF='"${DIFF}"' -DDIFFH='"${DIFFH}"' -DPR='"${PR}"'
LDFLAGS= -n

SRCS=	diff.c diffdir.c diffreg.c
HDRS=	diff.h
OBJS=	diff.o diffdir.o diffreg.o

all: diff diffh

cp: diff diffh
	-strip diff diffh
	cp diff ${DESTDIR}/bin/diff
	cp diffh ${DESTDIR}/usr/lib/diffh
	rm diff diffh *.o

cmp: diff diffh
	cmp diff ${DESTDIR}/bin/diff
	cmp diffh ${DESTDIR}/usr/lib/diffh
	rm *.o

diff: ${OBJS}
	cc ${CFLAGS} -o diff ${OBJS} ${LDFLAGS}

diffh: diffh.o
	cc -o diffh diffh.o ${LDFLAGS}

clean:
	rm -f ${OBJS} diffh.o
