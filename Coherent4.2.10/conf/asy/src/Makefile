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

DRIVER=$(INSTALL_DIR)/Driver.o

TARGETS=$(INSTALL_DIR)/after $(INSTALL_DIR)/mkdev $(DRIVER)

OBJS=

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

$(DRIVER): asy.c
	$(CC) $(CFLAGS) -o $@ -c asy.c

depend:
	echo > makedep
	grep "^\#include" * \
		| sed 's/\(.*\)\.c:.*"\(.*\)".*/$$(TARGETS): \2/' \
		| sed '/\#include/d' >> makedep
	grep "^\#include" * \
		| sed 's/\(.*\)\.c:.*<\(.*\)>.*/$$(TARGETS): $$(USRINC)\/\2/' \
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

$(TARGETS): $(USRINC)/sys/coherent.h
$(TARGETS): $(USRINC)/sys/errno.h
$(TARGETS): $(USRINC)/sys/stat.h
$(TARGETS): $(USRINC)/kernel/v_types.h
$(TARGETS): $(USRINC)/termio.h
$(TARGETS): $(USRINC)/poll.h
$(TARGETS): $(USRINC)/kernel/trace.h
$(TARGETS): $(USRINC)/sys/uproc.h
$(TARGETS): $(USRINC)/sys/proc.h
$(TARGETS): $(USRINC)/sys/tty.h
$(TARGETS): $(USRINC)/sys/con.h
$(TARGETS): $(USRINC)/sys/devices.h
$(TARGETS): $(USRINC)/sys/sched.h
$(TARGETS): $(USRINC)/sys/asy.h
$(TARGETS): $(USRINC)/sys/ins8250.h
$(TARGETS): $(USRINC)/sys/poll_clk.h

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
