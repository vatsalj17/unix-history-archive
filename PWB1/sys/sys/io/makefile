LIB = /sys/sys
DD = tmp
LNAME = lib2

CC = cc
CCFLAGS = -O
HD = /sys/sys/hd

LD = -ld
LDFLAGS = -x

.c.o:
	$(CC) -c $(CCFLAGS) $<
	$(LD) -r $(LDFLAGS) $*.o
	mv a.out $*.o

# install: Create and install /sys/sys/lib2 - io

install:	lib2
	-mv $(LIB)/$(LNAME) $(LIB)/$(DD)$(LNAME)
	mv lib2 $(LIB)/$(LNAME)
	-chown bin $(LIB)/$(LNAME)
	-rm -f $(LIB)/$(DD)$(LNAME)
	ls -l $(LIB)/$(LNAME)

# lib2: Create lib2 in local directory.

lib2:	bio.o cat.o dh.o dn.o dqs.o du.o hp.o hs.o ht.o kl.o \
	lp.o mem.o partab.o pio.o rp.o sys.o tm.o tty.o vp.o
	-rm -f lib2 tmp.a
	ar r tmp.a bio.o pio.o tty.o kl.o dh.o hp.o hs.o rp.o ht.o tm.o
	ar r tmp.a mem.o partab.o sys.o cat.o dqs.o dn.o du.o lp.o vp.o
	mv tmp.a lib2

bio.o:	bio.c $(HD)/param.h $(HD)/user.h $(HD)/buf.h $(HD)/conf.h $(HD)/systm.h $(HD)/var.h
cat.o:	cat.c $(HD)/param.h $(HD)/user.h $(HD)/tty.h
dh.o:	dh.c $(HD)/param.h $(HD)/user.h $(HD)/tty.h
dn.o:	dn.c $(HD)/param.h $(HD)/user.h
dqs.o:	dqs.c $(HD)/param.h $(HD)/buf.h $(HD)/systm.h $(HD)/user.h $(HD)/peri.h
du.o:	du.c $(HD)/param.h $(HD)/user.h $(HD)/buf.h $(HD)/peri.h
hp.o:	hp.c $(HD)/param.h $(HD)/buf.h $(HD)/user.h
hs.o:	hs.c $(HD)/param.h $(HD)/buf.h $(HD)/user.h
ht.o:	ht.c $(HD)/param.h $(HD)/buf.h $(HD)/user.h
kl.o:	kl.c $(HD)/param.h $(HD)/user.h $(HD)/tty.h
lp.o:	lp.c $(HD)/param.h $(HD)/user.h $(HD)/tty.h
mem.o:	mem.c $(HD)/param.h $(HD)/user.h $(HD)/seg.h
pio.o:	pio.c $(HD)/param.h $(HD)/user.h $(HD)/buf.h $(HD)/systm.h \
	$(HD)/proc.h $(HD)/conf.h $(HD)/seg.h
rp.o:	rp.c $(HD)/param.h $(HD)/buf.h $(HD)/user.h
sys.o:	sys.c $(HD)/param.h $(HD)/user.h $(HD)/conf.h $(HD)/tty.h
tm.o:	tm.c $(HD)/param.h $(HD)/buf.h $(HD)/user.h
tty.o:	tty.c $(HD)/param.h $(HD)/systm.h $(HD)/user.h $(HD)/tty.h \
	$(HD)/proc.h $(HD)/conf.h $(HD)/inode.h $(HD)/file.h $(HD)/reg.h $(HD)/var.h
vp.o:	vp.c $(HD)/param.h $(HD)/user.h $(HD)/buf.h $(HD)/peri.h
