LIB=../../lib
BIN=../../bin
LIBS=$(LIB)/access $(LIB)/iutil $(LIB)/gutil -lm
CC=cc
CFLAGS=-O
LDFLAGS=-i
AR=ar
LIBMOD=644
MODE=4700
OBJS=	main.o \
	index.o \
	create.o \
	modify.o \
	modupdate.o \
	destroy.o \
	userdestroy.o \
	purgetup.o \
	print.o \
	help.o \
	copy.o \
	save.o \
	rupdate.o \
	update.o \
	convert.o \
	secupdate.o \
	rdwrbatch.o \
	resetrel.o \
	get_seq_atts.o \
	dbuproctab.o \
	display.o \
	pr_tree.o \
	readtree.o \
	pr_prot.o \
	rmqm.o \
	get_p_tid.o

dbu: dbulib overlayg.o
	$(CC) $(LDFLAGS) overlayg.o dbulib $(LIBS) -o overlayg

dbulib: $(OBJS)
	-$(AR) rvuc dbulib $(OBJS)
	cp dbulib $(LIB)/dbulib
	chmod $(LIBMOD) $(LIB)/dbulib

ksort: ksort.o
	$(CC) $(LDFLAGS) ksort.o $(LIBS) -o ksort

install: dbulib dbu ksort
	cp overlayg $(BIN)/overlayg
	cp ksort $(BIN)/ksort
	chmod 4711 ksort
	chmod $(MODE) $(BIN)/overlayg

clean:
	rm *.o
