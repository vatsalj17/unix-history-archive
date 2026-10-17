LIB=../../lib
BIN=../../bin
CC=cc
CFLAGS=-O
LDFLAGS= -n
LIBS=$(LIB)/qmlib \
	$(LIB)/parlib \
	$(LIB)/access \
	$(LIB)/iutil\
	$(LIB)/gutil
OBJS=	main.o \
	readtree.o \
	qrymod.o \
	issue.o \
	define.o \
	d_view.o \
	d_integ.o \
	d_prot.o \
	fillprotup.o \
	view.o \
	integrity.o \
	protect.o \
	writetree.o \
	range.o \
	tree.o \
	trscan.o \
	treepr.o \
	util.o \
	catalog_desc.o
AR=ar
LIBMOD= 644
MODE=4700

qrymod: qmlib
	$(CC) $(LDFLAGS) $(LIBS) -o qrymod

qmlib: $(OBJS)
	-$(AR) cruv qmlib $(OBJS)
	cp qmlib $(LIB)/qmlib
	chmod $(LIBMOD) $(LIB)/qmlib

install: qrymod
	size		qrymod
	ls -l	qrymod
	cp qrymod $(BIN)/qrymod
	chmod $(MODE) $(BIN)/qrymod

clean: 
	rm *.o
