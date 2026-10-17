CFLAGS = -O -n -s -I${INCLUDES}
FMT = -i		# -n for machines without floating point; c2 only
FP =			# -f for machines without floating point

all:	c0 c1 c2 fc1

cmp:	all
	cmp c0 ${DESTDIR}/lib/c0
	cmp c1 ${DESTDIR}/lib/c1
	cmp c2 ${DESTDIR}/lib/c2
	[ x${FP} = x-f ] && cmp c1 ${DESTDIR}/lib/c1
	rm *.o

cp:	all
	-strip c0 c1 c2 fc1
	cp c0 ${DESTDIR}/lib/c0
	[ x${FP} != x-f ] && cp c1 ${DESTDIR}/lib/c1
	[ x${FP} = x-f ] && cp fc1 ${DESTDIR}/lib/fc1 \
		&& ln ${DESTDIR}/lib/fc1 ${DESTDIR}/lib/c1 
	cp c2 ${DESTDIR}/lib/c2
	rm c0 c1 c2 *.o

fcp:	fc1
	cp fc1 /lib/fc1
	rm fc1 c1*.o

c0: c00.o c01.o c02.o c03.o c04.o c05.o
	cc $(CFLAGS) -o c0 c00.o c01.o c02.o c03.o c04.o c05.o

c00.o c01.o c02.o c03.o c04.o c05.o: c0.h

c1: c10.o c11.o c12.o c13.o table.o
	cc $(CFLAGS) -o c1 c10.o c11.o c12.o c13.o table.o

fc1: c10.o c11.o c12.o c13.o table.o
	cc $(CFLAGS) -f -o fc1 c10.o c11.o c12.o c13.o table.o

c10.o c11.o c12.o c13.o: c1.h

table.o: table.s cvopt
	./cvopt <table.s >table.i
	as -o table.o table.i
	rm table.i

c2: c20.o c21.o
	cc ${FMT} -O -I{INCLUDES} -s -o c2 c20.o c21.o

c20.o c21.o: c2.h

cvopt:	cvopt.c
	cc $(CFLAGS) -o cvopt cvopt.c

clean:
	rm -f *.o cvopt
