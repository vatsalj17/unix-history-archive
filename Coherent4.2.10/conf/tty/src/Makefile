# Standard 'conf'-system makefile with productions for standardized targets;
# 'make install', 'make clean', 'make manifest', 'make setup', 'make distrib'
# See the top-level 'conf' system Makefile for more details.

INSTALL_DIR=..
PREFIX=.
MANIFEST=$(PREFIX)/src
MAKEFILE=Makefile
USRINC=/usr/include
GCC=gcc
#GCCFLAGS=-O2 $(CFLAGS) -Wall -ansi -pedantic
GCCFLAGS=$(CFLAGS) -Wall -ansi -pedantic
GLDFLAGS=-nostdlib /lib/crts0.o
GLIBFLAGS=/lib/libc.a

DRIVER=$(INSTALL_DIR)/Driver.a

TARGETS=$(DRIVER)

OBJS= $(DRIVER)(tty.o) $(DRIVER)(clist.o)

all: $(TARGETS)

gcc:
	+@exec make "CC=$(GCC)" "CFLAGS=$(GCCFLAGS)" \
		    "LDFLAGS=$(GLDFLAGS)" "LIBFLAGS=$(GLIBFLAGS)" \
		    -f $(MAKEFILE)

install setup:

clean:
	rm -f $(TARGETS)

manifest:
	@for i in *; do \
		case $$i in \
		RCS)	;; \
		*)	echo $(MANIFEST)/$$i \
			;; \
		esac \
	done

distrib:
	@for i in $(TARGETS); do echo $(PREFIX)/$${i\#$(INSTALL_DIR)/}; done

$(DRIVER): $(OBJS)

depend:
	echo > makedep
	grep "^\#include" * \
		| sed 's/\(.*\)\.c:.*"\(.*\)".*/$$(DRIVER)(\1.o): \2/' \
		| sed '/\#include/d' >> makedep
	grep "^\#include" * \
		| sed 's/\(.*\)\.c:.*<\(.*\)>.*/$$(DRIVER)(\1.o): $$(USRINC)\/\2/' \
		| sed '/\#include/d' >> makedep
	grep "^\#include" * \
		| sed 's/\(.*\)\.spc:.*<\(.*\)>.*/$$(INSTALL_DIR)\/\1.c: $$(USRINC)\/\2/' \
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

$(DRIVER)(clist.o): $(USRINC)/kernel/trace.h
$(DRIVER)(clist.o): $(USRINC)/kernel/param.h
$(DRIVER)(clist.o): $(USRINC)/sys/clist.h
$(DRIVER)(clist.o): $(USRINC)/sys/sched.h
$(DRIVER)(tty.o): $(USRINC)/sys/stat.h
$(DRIVER)(tty.o): $(USRINC)/sys/errno.h
$(DRIVER)(tty.o): $(USRINC)/sys/file.h
$(DRIVER)(tty.o): $(USRINC)/termio.h
$(DRIVER)(tty.o): $(USRINC)/kernel/_timers.h
$(DRIVER)(tty.o): $(USRINC)/kernel/_pl.h
$(DRIVER)(tty.o): $(USRINC)/sys/coherent.h
$(DRIVER)(tty.o): $(USRINC)/kernel/trace.h
$(DRIVER)(tty.o): $(USRINC)/sys/uproc.h
$(DRIVER)(tty.o): $(USRINC)/sys/clist.h
$(DRIVER)(tty.o): $(USRINC)/sys/con.h
$(DRIVER)(tty.o): $(USRINC)/sys/deftty.h
$(DRIVER)(tty.o): $(USRINC)/sys/io.h
$(DRIVER)(tty.o): $(USRINC)/sys/proc.h
$(DRIVER)(tty.o): $(USRINC)/sys/sched.h
$(DRIVER)(tty.o): $(USRINC)/sys/tty.h

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
