# Simple makefile for 'devadm' under Coherent. This doesn't change very often,
# so keeping the .o's around doesn't seem like a win.

INSTALL_DIR=.
PREFIX=.
MANIFEST=$(PREFIX)/bports
MAKEFILE=Makefile
USRINC=/usr/include
GCC=gcc
GCCFLAGS=-O2 -Wall -ansi -pedantic
GLDFLAGS=-nostdlib /lib/ndp/crts0.o
GLIBFLAGS=/lib/libc.a

TARGET=$(INSTALL_DIR)/bports

all:	$(TARGET)

gcc:
	+@exec make "CC=$(GCC)" "CFLAGS=$(GCCFLAGS)" \
		    "LDFLAGS=$(GLDFLAGS)" "LIBFLAGS=$(GLIBFLAGS)" \
		    -f $(MAKEFILE)

distrib setup install clean:

manifest:
	@for i in *; do \
		case $$i in \
		RCS | lib) \
			;; \
		*)	echo $(MANIFEST)/$$i \
			;; \
		esac \
	done

depend:
	echo > makedep
	grep "^\#include" * \
		| sed 's/\(.*\.c\):.*"\(.*\)".*/$$(TARGET): \2/' \
		| sed '/\#include/d' >> makedep
	grep "^\#include" * \
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

$(TARGET): $(USRINC)/sys/compat.h
$(TARGET): $(USRINC)/stdio.h
$(TARGET): $(USRINC)/stdlib.h
$(TARGET): $(USRINC)/fcntl.h
$(TARGET): $(USRINC)/unistd.h

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
