#
# Makefile for ibm specific coherent sources and coherent images.
#

USRINC=/usr/include
DESTLIB=$(K386LIB)/k386.a
MAKEFILE=io.make

OBJS =  $(DESTLIB)(albaud.o) $(DESTLIB)(alx.o) $(DESTLIB)(bufq.o) \
	$(DESTLIB)(com1.o) $(DESTLIB)(com2.o) \
	$(DESTLIB)(fdisk.o) $(DESTLIB)(hs.o) \
	$(DESTLIB)(putchar.o)

#	$(DESTLIB)(xla.o) $(DESTLIB)(xl_dec.o) $(DESTLIB)(xlft.o) \
#	$(DESTLIB)(ft.o) 

# $(DESTLIB)(msg386.o) $(DESTLIB)(sem386.o) $(DESTLIB)(shm1.o)

.c.a:
	echo $< >>sources; echo $% >> members

all:	prelude $(OBJS) postlude

prelude:
	echo > sources; echo > members

postlude:
	@case "$${members=$$(cat members)}" in \
	"")	;; \
	*)	case "$${sources=$$(cat sources)}" in \
		"")	;; \
		*)	echo Compiling $$sources; \
			$(CC) $(CFLAGS) -c $$sources || exit 1 ;; \
		esac; \
		echo Archiving $$members ; \
		$(AR) $(ARFLAGS) $(DESTLIB) $$members || exit 1; \
		rm -f $$members ;; \
	esac
	rm sources; rm members

$(DESTLIB)(bufq.o): bufq.c
	$(CC) $(CFLAGS) -DALCOM1=1 -c $*.c
	echo $% >> members

$(DESTLIB)(com1.o): al.c
	$(CC) $(CFLAGS) -DALCOM1=1 -o $% -c al.c
	echo $% >> members

$(DESTLIB)(com2.o): al.c
	$(CC) $(CFLAGS) -DALCOM2=1 -o $% -c al.c
	echo $% >> members

depend:
	echo > makedep
	grep "^\#include" *.c | \
		sed 's/\(.*\)\.c:.*"\(.*\)".*/$$(DESTLIB)(\1.o): \2/' | \
		sed '/\#include/d' >> makedep
	grep "^\#include" *.c | \
		sed 's/\(.*\)\.c:.*<\(.*\)>.*/$$(DESTLIB)(\1.o): $$(USRINC)\/\2/' | \
		sed '/\#include/d' >> makedep
	echo '/^\# DO NOT DELETE THIS LINE/+1,$$d' > eddep
	echo '$$r makedep' >> eddep
	echo 'w' >> eddep
	ed $(MAKEFILE) < eddep
	rm makedep eddep
	echo >> $(MAKEFILE)
	echo "\# DEPENDENCIES MUST END AT END OF FILE:" \
	     "IF YOU PUT STUFF HERE, IT WILL GO AWAY" >> $(MAKEFILE)
	echo "\# See make depend, above" >> $(MAKEFILE)

# DO NOT DELETE THIS LINE - make depend needs it

$(DESTLIB)(al.o): $(USRINC)/sys/coherent.h
$(DESTLIB)(al.o): $(USRINC)/sys/con.h
$(DESTLIB)(al.o): $(USRINC)/sys/errno.h
$(DESTLIB)(al.o): $(USRINC)/sys/stat.h
$(DESTLIB)(al.o): $(USRINC)/sys/tty.h
$(DESTLIB)(al.o): $(USRINC)/sys/clist.h
$(DESTLIB)(al.o): $(USRINC)/sys/ins8250.h
$(DESTLIB)(al.o): $(USRINC)/sys/sched.h
$(DESTLIB)(al.o): $(USRINC)/sys/al.h
$(DESTLIB)(al.o): $(USRINC)/sys/devices.h
$(DESTLIB)(albaud.o): $(USRINC)/kernel/param.h
$(DESTLIB)(albaud.o): $(USRINC)/sys/ins8250.h
$(DESTLIB)(alx.o): $(USRINC)/kernel/timeout.h
$(DESTLIB)(alx.o): $(USRINC)/sys/coherent.h
$(DESTLIB)(alx.o): $(USRINC)/sys/al.h
$(DESTLIB)(alx.o): $(USRINC)/sys/con.h
$(DESTLIB)(alx.o): $(USRINC)/sys/errno.h
$(DESTLIB)(alx.o): $(USRINC)/sys/stat.h
$(DESTLIB)(alx.o): $(USRINC)/sys/tty.h
$(DESTLIB)(alx.o): $(USRINC)/sys/clist.h
$(DESTLIB)(alx.o): $(USRINC)/sys/ins8250.h
$(DESTLIB)(alx.o): $(USRINC)/sys/sched.h
$(DESTLIB)(alx.o): $(USRINC)/sys/silo.h
$(DESTLIB)(alx.o): $(USRINC)/sys/signal.h
$(DESTLIB)(alx.o): $(USRINC)/sys/proc.h
$(DESTLIB)(bufq.o): $(USRINC)/sys/coherent.h
$(DESTLIB)(bufq.o): $(USRINC)/sys/buf.h
$(DESTLIB)(fdisk.o): $(USRINC)/sys/coherent.h
$(DESTLIB)(fdisk.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(fdisk.o): $(USRINC)/sys/errno.h
$(DESTLIB)(fdisk.o): $(USRINC)/sys/fdisk.h
$(DESTLIB)(fdisk.o): $(USRINC)/sys/buf.h
$(DESTLIB)(fdisk.o): $(USRINC)/sys/con.h
$(DESTLIB)(fdisk.o): $(USRINC)/sys/file.h
$(DESTLIB)(hs.o): $(USRINC)/sys/coherent.h
$(DESTLIB)(hs.o): $(USRINC)/sys/ins8250.h
$(DESTLIB)(hs.o): $(USRINC)/sys/stat.h
$(DESTLIB)(hs.o): $(USRINC)/sys/proc.h
$(DESTLIB)(hs.o): $(USRINC)/sys/tty.h
$(DESTLIB)(hs.o): $(USRINC)/sys/con.h
$(DESTLIB)(hs.o): $(USRINC)/sys/devices.h
$(DESTLIB)(hs.o): $(USRINC)/sys/errno.h
$(DESTLIB)(hs.o): $(USRINC)/sys/sched.h
$(DESTLIB)(hs.o): $(USRINC)/sys/poll_clk.h
$(DESTLIB)(putchar.o): $(USRINC)/sys/coherent.h
$(DESTLIB)(putchar.o): $(USRINC)/sys/uproc.h
$(DESTLIB)(putchar.o): $(USRINC)/sys/stat.h
$(DESTLIB)(putchar.o): $(USRINC)/sys/con.h
$(DESTLIB)(putchar.o): $(USRINC)/sys/io.h
$(DESTLIB)(putchar.o): $(USRINC)/sys/devices.h
$(DESTLIB)(putchar.o): $(USRINC)/sys/file.h

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
