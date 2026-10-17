INGRES=	../..
CC=	cc
CFLAGS=	-O
AR=	-ar
ARFLAGS=rvu
LIB=	$(INGRES)/lib
ACCESS=	$(LIB)/access
GUTIL=	$(LIB)/gutil
IUTIL=	$(LIB)/iutil

LIBS=	$(ACCESS) $(IUTIL) $(GUTIL)

OBJS=	main.o \
	ovqp.o \
	startovqp.o \
	getqry.o \
	rdsym.o \
	strategy.o \
	findsimps.o \
	key.o \
	scan.o \
	interp.o \
	interp1.o \
	equelpr.o \
	string.o \
	typecheck.o \
	support.o \
	prsym.o \
	copyreturn.o

ovqplib: $(OBJS)
	-$(AR) $(ARFLAGS) ovqplib $(OBJS)

install: ovqplib
	cp ovqplib $(LIB)/ovqplib

clean:
	rm *.o
