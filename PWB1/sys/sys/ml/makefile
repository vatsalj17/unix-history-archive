LIB = /sys/sys
DD = tmp
LNAME = lib0

AS = as
CC = cc
CCFLAGS = -O
HD = /sys/sys/hd

LD = -ld
LDFLAGS = -x

.s.o:
	$(AS) $<
	$(LD) -r $(LDFLAGS) a.out
	mv a.out $*.o

.c.o:
	$(CC) -c $(CCFLAGS) $<
	$(LD) -r $(LDFLAGS) $*.o
	mv a.out $*.o

# install: Create and install /sys/sys/lib0 - ml

install:	lib0
	-mv $(LIB)/$(LNAME) $(LIB)/$(DD)$(LNAME)
	mv lib0 $(LIB)/$(LNAME)
	-chown bin $(LIB)/$(LNAME)
	-rm -f $(LIB)/$(DD)$(LNAME)
	ls -l $(LIB)/$(LNAME)

# lib0: Create lib0 in local directory.

lib0:	mch.o dump_ht.o dump_tm.o
	-rm -f lib0 tmp.a
	ar r tmp.a mch.o dump_ht.o dump_tm.o
	mv tmp.a lib0

mch.o:	mch.s
dump_ht.o:	dump_ht.s
dump_tm.o:	dump_tm.s
