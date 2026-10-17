# (-lgl
#	Coherent 386 release 4.2-Beta
#	Copyright (c) 1982, 1993 by Mark Williams Company.
#	All rights reserved. May not be copied without permission.
#	For copying permission and licensing info, write licensing@mwc.com
# -lgl)
# Simple makefile for 'devadm' under Coherent. This doesn't change very often,
# so keeping the .o's around doesn't seem like a win.

INSTALL_DIR=.
PREFIX=.
MANIFEST=$(PREFIX)/shell
USRINC=/usr/include
MAKEFILE=Makefile
GCC=gcc
GCCFLAGS=-O2 -Wall -ansi -pedantic
GLDFLAGS=-nostdlib /lib/ndp/crts0.o
GLIBFLAGS=/lib/libc.a

TARGETS=$(INSTALL_DIR)/idenable $(INSTALL_DIR)/idtune $(INSTALL_DIR)/idcheck \
	$(INSTALL_DIR)/idsetirq $(INSTALL_DIR)/idmkcoh $(INSTALL_DIR)/cohtune \
	$(INSTALL_DIR)/idbld $(INSTALL_DIR)/conf_lib.sh

all: $(TARGETS)

gcc:
	+@exec make "CC=$(GCC)" "CFLAGS=$(GCCFLAGS)" \
		    "LDFLAGS=$(GLDFLAGS)" "LIBFLAGS=$(GLIBFLAGS)" \
		    -f $(MAKEFILE)

install clean setup distrib:

manifest:
	@for i in *; do \
		case $$i in \
		RCS | lib) \
			;; \
		*)	echo $(MANIFEST)/$$i \
			;; \
		esac \
	done

$(INSTALL_DIR)/conf_lib.sh: conf_lib.sh
	ln -f conf_lib.sh $(INSTALL_DIR)

depend:
	echo > makedep
	grep "^\#include" * \
		| sed 's/\(*\.c\):.*"\(.*\)".*/$$(TARGET): \2/' \
		| sed '/\#include/d' >> makedep
	grep "^\#include" *.c \
		| sed 's/\(.*\.c\):.*<\(.*\)>.*/$$(TARGET): $$(USRINC)\/\2/' \
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




# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
