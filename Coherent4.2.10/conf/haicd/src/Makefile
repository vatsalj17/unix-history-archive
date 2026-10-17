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

OBJS=

all: $(TARGETS)

# No real gcc till timing issues are sorted out.
#
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

$(DRIVER): haicd.c
	$(CC) $(CFLAGS) -o $@ -c haicd.c

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

$(DRIVER): $(USRINC)/sys/haiscsi.h
$(DRIVER): $(USRINC)/errno.h
$(DRIVER): $(USRINC)/stddef.h
$(DRIVER): $(USRINC)/sys/cdrom.h
$(DRIVER): $(USRINC)/sys/cmn_err.h
$(DRIVER): $(USRINC)/sys/coherent.h
$(DRIVER): $(USRINC)/sys/file.h
$(DRIVER): $(USRINC)/sys/haiscsi.h
$(DRIVER): $(USRINC)/sys/inode.h
$(DRIVER): $(USRINC)/sys/sched.h
$(DRIVER): $(USRINC)/sys/stat.h
$(INSTALL_DIR)/Space.c: $(USRINC)/sys/haiscsi.h

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
