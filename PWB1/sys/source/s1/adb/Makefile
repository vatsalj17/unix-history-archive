CC = cc
SYSINC = /sys/sys/hd

adb:	access.o command.o expr.o findrtn.o
adb:	format.o input.o instr.o main.o
adb:	message.o output.o pcs.o print.o
adb:	runpcs.o setup.o sym.o
adb:;	$(CC) -o adb -i *.o

access.o:	defs.h $(SYSINC)/proc.h
command.o:	defs.h
expr.o:		defs.h
findrtn.o:	defs.h
format.o:	defs.h
input.o:	defs.h
instr.o:	defs.h
main.o:		defs.h
message.o:	mac.h mode.h
output.o:	defs.h
pcs.o:		$(SYSINC)/proc.h defs.h
print.o:	$(SYSINC)/proc.h defs.h
runpcs.o:	$(SYSINC)/proc.h defs.h
setup.o:	defs.h
sym.o:		$(SYSINC)/proc.h defs.h

defs.h:		$(SYSINC)/param.h $(SYSINC)/user.h mac.h mode.h message.h

sdb:	dummy.o
sdb:	access.o command.o expr.o findrtn.o
sdb:	format.o input.o instr.o main.o
sdb:	message.o output.o print.o setup.o sym.o
sdb:;	$(CC) -o sdb -i *.o

install:;
	install adb
	for i in *.[ch] Makefile ; do cp $$i /sys/source/s1/adb/$$i; done

list:;
	list $(SYSINC)/param.h $(SYSINC)/user.h $(SYSINC)/proc.h *.h *.c

.c.o:; $(CC) -O -c -I$(SYSINC) $<
