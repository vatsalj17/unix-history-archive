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

TARGETS=$(INSTALL_DIR)/Space.c \
	$(INSTALL_DIR)/Stub.o \
	$(INSTALL_DIR)/mkdev_hai \
	$(DRIVER)

OBJS= $(DRIVER)(hai154x.o) $(DRIVER)(haict.o) $(DRIVER)(haiscsi.o) \
	$(DRIVER)(haisd.o) 

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

$(DRIVER): hai154x.c
	$(CC) $(CFLAGS) -o $@ -c hai154x.c

depend:
	echo > makedep
	grep "^\#include" * \
		| sed 's/\(.*\)\.c:.*"\(.*\)".*/$$(DRIVER): \2/' \
		| sed '/\#include/d' >> makedep
	grep "^\#include" * \
		| sed 's/\(.*\)\.c:.*<\(.*\)>.*/$$(DRIVER): $$(USRINC)\/\2/' \
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

$(DRIVER): $(USRINC)/stddef.h
$(DRIVER): $(USRINC)/sys/coherent.h
$(DRIVER): $(USRINC)/sys/haiscsi.h
$(DRIVER): $(USRINC)/stddef.h
$(DRIVER): $(USRINC)/sys/coherent.h
$(DRIVER): $(USRINC)/sys/con.h
$(DRIVER): $(USRINC)/sys/cmn_err.h
$(DRIVER): $(USRINC)/sys/devices.h
$(DRIVER): $(USRINC)/sys/types.h
$(DRIVER): $(USRINC)/sys/hdioctl.h
$(DRIVER): $(USRINC)/sys/haiscsi.h
$(DRIVER): $(USRINC)/sys/stat.h
$(INSTALL_DIR)/Space.c: $(USRINC)/sys/haiscsi.h

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
