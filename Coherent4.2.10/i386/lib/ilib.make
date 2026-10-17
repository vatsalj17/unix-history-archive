# Sub-makefile to batch compilation and archiving into large passes

MAKEFILE=ilib.make
DESTLIB=$(K386LIB)/k386.a
USRINC=/usr/include
GCC=gcc
GCCFLAGS=-O2 -Wall -ansi -pedantic
GLDFLAGS=-nostdlib /lib/ndp/crts0.o
GLIBFLAGS=-lc

OBJS=	$(DESTLIB)(inb.o) $(DESTLIB)(inl.o) $(DESTLIB)(inw.o) \
	$(DESTLIB)(ldiv.o) \
	$(DESTLIB)(memcpy.o) $(DESTLIB)(memset.o) $(DESTLIB)(outb.o) \
	$(DESTLIB)(outl.o) $(DESTLIB)(outw.o) $(DESTLIB)(repinsb.o) \
	$(DESTLIB)(repinsd.o) $(DESTLIB)(repinsw.o) $(DESTLIB)(repoutsb.o) \
	$(DESTLIB)(repoutsd.o) $(DESTLIB)(repoutsw.o) $(DESTLIB)(strncmp.o) \
	$(DESTLIB)(strncpy.o) $(DESTLIB)(strlen.o) \
	$(DESTLIB)(disable.o) $(DESTLIB)(enable.o) $(DESTLIB)(setmask.o) \
	$(DESTLIB)(splcmp.o) $(DESTLIB)(splx.o) $(DESTLIB)(intr_disp.o) \
	$(DESTLIB)(trap_disp.o) $(DESTLIB)(trap_level.o) $(DESTLIB)(coh_lock.o)

.c.a:
	echo $< >>sources; echo $% >> members
.s.a:
	$(AS) $(ASFLAGS) $*.s
	echo $% >> members

all:	prelude $(OBJS) postlude

gcc:
	+@exec make "CC=$(GCC)" "CFLAGS=$(GCCFLAGS)" \
		    "LDFLAGS=$(GLDFLAGS)" "LIBFLAGS=$(GLIBFLAGS)" \
		    -f $(MAKEFILE)

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
		$(AR) $(ARFLAGS) $(DESTLIB) $$members; \
		rm -f $$members ;; \
	esac
	rm sources; rm members

depend:
	echo > makedep
	grep "^\#include" *.c \
		| sed 's/\(.*\)\.c:.*"\(.*\)".*/\1.o: \2/' \
		| sed '/\#include/d' >> makedep
	grep "^\#include" *.c \
		| sed 's/\(.*\)\.c:.*<\(.*\)>.*/$$(DESTLIB)(\1.o): $$(USRINC)\/\2/' \
		| sed '/\#include/d' >> makedep
	grep ".include" *.s \
		| sed 's/\(.*\)\.s:.*"\(.*\)".*/\1.o: \2/' \
		| sed '/\#include/d' >> makedep
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

$(DESTLIB)(spl.o): $(USRINC)/common/ccompat.h
$(DESTLIB)(spl.o): $(USRINC)/sys/inline.h
$(DESTLIB)(trap_level.o): $(USRINC)/sys/types.h
$(DESTLIB)(trap_level.o): $(USRINC)/common/_gregset.h
intr_disp.o: struct.inc
intr_disp.o: intr.inc
intr_disp.o: ddi.inc
intr_disp.o: select.inc
repinsb.o: repeat.inc
repinsd.o: repeat.inc
repinsw.o: repeat.inc
repoutsb.o: repeat.inc
repoutsd.o: repeat.inc
repoutsw.o: repeat.inc
setmask.o: struct.inc
setmask.o: pl.inc
setmask.o: ddi.inc
splx.o: struct.inc
splx.o: pl.inc
splx.o: ddi.inc
trap_disp.o: struct.inc
trap_disp.o: intr.inc
trap_disp.o: ddi.inc
trap_disp.o: select.inc

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
