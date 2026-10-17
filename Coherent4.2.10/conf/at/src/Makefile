# (-lgl
#	Coherent 386 release 4.2
#	Copyright (c) 1982, 1994 by Mark Williams Company.
#	All rights reserved. May not be copied without permission.
#	For copying permission and licensing info, write licensing@mwc.com
# -lgl)
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

TARGETS=$(DRIVER) $(INSTALL_DIR)/Space.c $(INSTALL_DIR)/mkdev

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

$(DRIVER): at.c
	/bin/cc -o $@ -c at.c

#	$(CC) $(CFLAGS) -o $@ -c at.c

depend:
	echo > makedep
	grep "^\#include" * \
		| sed 's/\(.*\)\.c:.*"\(.*\)".*/$$(TARGETS): \2/' \
		| sed '/\#include/d' >> makedep
	grep "^\#include" * \
		| sed 's/\(.*\)\.c:.*<\(.*\)>.*/$$(TARGETS): $$(USRINC)\/\2/' \
		| sed '/\#include/d' >> makedep
	grep "^\#include" * \
		| sed 's/\(.*\)\.spc:.*<\(.*\)>.*/$$(TARGETS): $$(USRINC)\/\2/' \
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

$(TARGETS): $(USRINC)/sys/cmn_err.h
$(TARGETS): $(USRINC)/sys/inline.h
$(TARGETS): $(USRINC)/sys/stat.h
$(TARGETS): $(USRINC)/sys/errno.h
$(TARGETS): $(USRINC)/stdlib.h
$(TARGETS): $(USRINC)/kernel/typed.h
$(TARGETS): $(USRINC)/sys/coherent.h
$(TARGETS): $(USRINC)/sys/uproc.h
$(TARGETS): $(USRINC)/sys/fdisk.h
$(TARGETS): $(USRINC)/sys/hdioctl.h
$(TARGETS): $(USRINC)/sys/buf.h
$(TARGETS): $(USRINC)/sys/con.h
$(TARGETS): $(USRINC)/sys/devices.h
$(TARGETS): $(USRINC)/sys/hdioctl.h

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
