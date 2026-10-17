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

DRIVER=$(INSTALL_DIR)/Driver.a

OBJS= $(DRIVER)(haiscsi.o) $(DRIVER)(haisg.o) $(DRIVER)(biosdisk.o)

TARGETS=$(INSTALL_DIR)/Space.c \
	$(INSTALL_DIR)/mkdev \
	$(DRIVER)

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

# The following is temporary, to be replaced when gcc timing issues are fixed.

$(DRIVER): $(OBJS)

$(DRIVER)(haiscsi.o):	haiscsi.c
	$(CC) $(CFLAGS) -o $% -c haiscsi.c
	$(AR) $(ARFLAGS) $@ $%
	rm -f $%

$(DRIVER)(haisg.o):	haisg.c
	$(CC) $(CFLAGS) -o $% -c haisg.c
	$(AR) $(ARFLAGS) $@ $%
	rm -f $%

$(DRIVER)(biosdisk.o):	biosdisk.c
	$(CC) $(CFLAGS) -o $% -c biosdisk.c
	$(AR) $(ARFLAGS) $@ $%
	rm -f $%

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

$(DRIVER): $(USRINC)/kernel/typed.h
$(DRIVER): $(USRINC)/sys/hdioctl.h
$(DRIVER): $(USRINC)/sys/haiscsi.h
$(DRIVER): $(USRINC)/sys/coherent.h
$(DRIVER): $(USRINC)/stddef.h
$(DRIVER): $(USRINC)/errno.h
$(DRIVER): $(USRINC)/sys/buf.h
$(DRIVER): $(USRINC)/sys/cmn_err.h
$(DRIVER): $(USRINC)/sys/con.h
$(DRIVER): $(USRINC)/sys/haiioctl.h
$(DRIVER): $(USRINC)/sys/haiscsi.h
$(DRIVER): $(USRINC)/sys/io.h
$(DRIVER): $(USRINC)/sys/sched.h
$(DRIVER): $(USRINC)/sys/stat.h
$(DRIVER): $(USRINC)/sys/uproc.h
$(DRIVER): $(USRINC)/coh/proc.h
$(DRIVER): $(USRINC)/sys/coherent.h
$(DRIVER): $(USRINC)/sys/cmn_err.h
$(DRIVER): $(USRINC)/sys/buf.h
$(DRIVER): $(USRINC)/sys/mmu.h
$(DRIVER): $(USRINC)/sys/haiscsi.h
$(INSTALL_DIR)/Space.c: $(USRINC)/sys/haiscsi.h
$(INSTALL_DIR)/Space.c: $(USRINC)/sys/scsiwork.h

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
