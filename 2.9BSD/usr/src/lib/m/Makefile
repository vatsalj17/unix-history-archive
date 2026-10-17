FP =			# use -f if no floating point
CFLAGS = -O ${FP}

OBJS =	asin.o atan.o hypot.o jn.o j0.o j1.o pow.o fabs.o gamma.o \
	log.o sin.o sqrt.o tan.o tanh.o sinh.o exp.o floor.o

libm.a:	${OBJS}
	-rm -f libm.a
	ar rc libm.a ${OBJS}

cp: libm.a
	cp libm.a ${DESTDIR}/usr/lib/libm.a
	ranlib ${DESTDIR}/usr/lib/libm.a

clean:
	-rm -f *.o libm.a
