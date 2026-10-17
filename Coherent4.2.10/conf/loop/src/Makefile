# Standard 'conf'-system makefile with productions for standardized targets;
# 'make install', 'make clean', 'make manifest', 'make setup', 'make distrib'
# See the top-level 'conf' system Makefile for more details.

INSTALL_DIR=..
PREFIX=.
MANIFEST=$(PREFIX)/src
MAKEFILE=Makefile
USRINC=/usr/include
GCC=gcc
GCCFLAGS=-O2 $(CFLAGS) -Wall -ansi -pedantic
GLDFLAGS=-nostdlib /lib/crts0.o
GLIBFLAGS=/lib/libc.a

DRIVER=$(INSTALL_DIR)/Driver.o

TARGETS=$(INSTALL_DIR)/Space.c $(DRIVER)

OBJS=

all: $(TARGETS) $(INSTALL_DIR)/loop.h

gcc:
	+@exec make "CC=$(GCC)" "CFLAGS=$(GCCFLAGS)" \
		    "LDFLAGS=$(GLDFLAGS)" "LIBFLAGS=$(GLIBFLAGS)" \
		    -f $(MAKEFILE)

install:
	cp loop.h /usr/include
	chmog =r bin bin /usr/include/loop.h

clean:
	rm -f $(TARGETS)

setup:

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

$(INSTALL_DIR)/loop.h: loop.h
	ln -f loop.h $@

$(DRIVER): loop.c
	$(CC) $(CFLAGS) -o $@ -c loop.c

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

$(TARGETS): loop.h
$(TARGETS): $(USRINC)/common/ccompat.h
$(TARGETS): $(USRINC)/sys/types.h
$(TARGETS): $(USRINC)/sys/cred.h
$(TARGETS): $(USRINC)/sys/kmem.h
$(TARGETS): $(USRINC)/sys/ksynch.h
$(TARGETS): $(USRINC)/sys/cmn_err.h
$(TARGETS): $(USRINC)/sys/errno.h
$(TARGETS): $(USRINC)/sys/cred.h
$(TARGETS): $(USRINC)/stddef.h
$(TARGETS): $(USRINC)/sys/stream.h
$(TARGETS): $(USRINC)/sys/stropts.h
$(TARGETS): $(USRINC)/sys/ddi.h

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
