INGRES=	../..
CC=	cc
CFLAGS=	-O
LDFLAGS= -i
AR=	-ar
ARFLAGS=rvu
LIB=	$(INGRES)/lib
BIN =	$(INGRES)/bin
ACCESS=	$(LIB)/access
GUTIL=	$(LIB)/gutil
IUTIL=	$(LIB)/iutil
OVQP=	$(LIB)/ovqplib
DBU=	$(LIB)/dbulib

LIBS=	$(OVQP) $(DBU) $(ACCESS) $(IUTIL) $(GUTIL) -lm

OBJS= ageval.o aggregate.o byeval.o call_dbu70.o \
	call_ovqp70.o decision.o decomp.o error.o exec_sq.o lockit.o \
	main.o makenode.o mapvar.o mklist.o openrs.o printree.o prlinks.o \
	pull_const.o pull_sq.o qryproc.o rangetable.o readq.o reduction.o \
	reformat.o reinit.o ret_unique.o rnum.o selectv.o setvar.o tempvar.o \
	usubr.o writeback.o writenod.o

decomp70: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o decomp70

install: decomp70
	cp decomp70 $(BIN)/decomp70
	chmod 4700 $(BIN)/decomp70

clean:
	rm *.o
