LIB = /sys/sys
DD = tmp
LNAME = lib1

CC = cc
CCFLAGS = -O
HD = /sys/sys/hd

LD = -ld
LDFLAGS = -x

.c.o:
	$(CC) -c $(CCFLAGS) $<
	$(LD) -r $(LDFLAGS) $*.o
	mv a.out $*.o

# install: Create and install /sys/sys/lib1 - os

install:	lib1
	-mv $(LIB)/$(LNAME) $(LIB)/$(DD)$(LNAME)
	mv lib1 $(LIB)/$(LNAME)
	-chown bin $(LIB)/$(LNAME)
	-rm -f $(LIB)/$(DD)$(LNAME)
	ls -l $(LIB)/$(LNAME)

# lib1: Create lib1 in local directory.

lib1:	alloc.o clock.o fio.o iget.o main.o malloc.o nami.o pipe.o \
	prf.o pwbsys.o rdwri.o sig.o slp.o subr.o sys1.o sys2.o \
	sys3.o sys4.o sysent.o text.o trap.o
	-rm -f lib1 tmp.a
	ar r tmp.a main.o alloc.o iget.o prf.o rdwri.o slp.o subr.o text.o trap.o sig.o sysent.o
	ar r tmp.a clock.o fio.o malloc.o nami.o pipe.o sys1.o sys2.o sys3.o sys4.o pwbsys.o
	mv tmp.a lib1

alloc.o:	alloc.c $(HD)/param.h $(HD)/systm.h $(HD)/filsys.h \
	$(HD)/conf.h $(HD)/buf.h $(HD)/inode.h $(HD)/user.h $(HD)/var.h
clock.o:	clock.c $(HD)/param.h $(HD)/systm.h $(HD)/user.h $(HD)/proc.h $(HD)/var.h
fio.o:	fio.c $(HD)/param.h $(HD)/user.h $(HD)/filsys.h $(HD)/file.h \
	$(HD)/conf.h $(HD)/inode.h $(HD)/reg.h $(HD)/systm.h $(HD)/var.h
iget.o:	iget.c $(HD)/param.h $(HD)/systm.h $(HD)/user.h $(HD)/inode.h \
	$(HD)/filsys.h $(HD)/buf.h $(HD)/var.h
main.o:	main.c $(HD)/param.h $(HD)/user.h $(HD)/proc.h $(HD)/text.h $(HD)/inode.h $(HD)/seg.h
malloc.o:	malloc.c $(HD)/param.h $(HD)/systm.h
nami.o:	nami.c $(HD)/param.h $(HD)/inode.h $(HD)/user.h $(HD)/systm.h $(HD)/buf.h
pipe.o:	pipe.c $(HD)/param.h $(HD)/systm.h $(HD)/user.h $(HD)/inode.h $(HD)/file.h $(HD)/reg.h
prf.o:	prf.c $(HD)/param.h $(HD)/seg.h $(HD)/buf.h $(HD)/systm.h
pwbsys.o:	pwbsys.c $(HD)/param.h $(HD)/buf.h $(HD)/filsys.h $(HD)/inode.h \
	$(HD)/systm.h $(HD)/user.h $(HD)/reg.h $(HD)/var.h
rdwri.o:	rdwri.c $(HD)/param.h $(HD)/inode.h $(HD)/user.h $(HD)/buf.h \
	$(HD)/conf.h $(HD)/systm.h
sig.o:	sig.c $(HD)/param.h $(HD)/systm.h $(HD)/user.h $(HD)/proc.h \
	$(HD)/inode.h $(HD)/reg.h $(HD)/text.h $(HD)/seg.h $(HD)/var.h
slp.o:	slp.c $(HD)/param.h $(HD)/user.h $(HD)/proc.h $(HD)/text.h $(HD)/systm.h \
	$(HD)/file.h $(HD)/inode.h $(HD)/buf.h $(HD)/var.h
subr.o:	subr.c $(HD)/param.h $(HD)/inode.h $(HD)/user.h $(HD)/buf.h $(HD)/systm.h
sys1.o:	sys1.c $(HD)/param.h $(HD)/systm.h $(HD)/user.h $(HD)/proc.h $(HD)/buf.h $(HD)/reg.h \
	$(HD)/inode.h $(HD)/seg.h $(HD)/var.h
sys2.o:	sys2.c $(HD)/param.h $(HD)/systm.h $(HD)/user.h $(HD)/reg.h $(HD)/file.h $(HD)/inode.h
sys3.o:	sys3.c $(HD)/param.h $(HD)/systm.h $(HD)/reg.h $(HD)/buf.h $(HD)/filsys.h \
	$(HD)/user.h $(HD)/inode.h $(HD)/file.h $(HD)/conf.h $(HD)/var.h
sys4.o:	sys4.c $(HD)/param.h $(HD)/user.h $(HD)/reg.h $(HD)/inode.h $(HD)/systm.h \
	$(HD)/proc.h $(HD)/var.h
text.o:	text.c $(HD)/param.h $(HD)/systm.h $(HD)/user.h $(HD)/proc.h $(HD)/text.h \
	$(HD)/inode.h $(HD)/buf.h $(HD)/seg.h $(HD)/var.h
trap.o:	trap.c $(HD)/param.h $(HD)/systm.h $(HD)/user.h $(HD)/proc.h $(HD)/reg.h $(HD)/seg.h
