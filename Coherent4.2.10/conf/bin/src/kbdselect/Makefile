# Simple makefile for 'devadm' under Coherent. This doesn't change very often,
# so keeping the .o's around doesn't seem like a win.

INSTALL_DIR=..
PREFIX=.
MANIFEST=$(PREFIX)/kbdselect
MAKEFILE=Makefile
USRINC=/usr/include
GCC=gcc
GCCFLAGS=-O2 -Wall -ansi -pedantic
GLDFLAGS=-nostdlib /lib/ndp/crts0.o
GLIBFLAGS=/lib/libc.a

SOURCES= kbdselect.c build0.c

TARGET=$(INSTALL_DIR)/kbdselect

all: $(TARGET)

gcc:
	+@exec make "CC=$(GCC)" "CFLAGS=$(GCCFLAGS)" \
		    "LDFLAGS=$(GLDFLAGS)" "LIBFLAGS=$(GLIBFLAGS)" \
		    -f $(MAKEFILE)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -so $@ $(SOURCES) -lcurses
	rm -f *.o

clean distrib setup install:

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

$(TARGET): build0.h
$(TARGET): build0.h
$(TARGET): $(USRINC)/stdio.h
$(TARGET): $(USRINC)/sys/stat.h
$(TARGET): $(USRINC)/stdio.h
$(TARGET): $(USRINC)/string.h
$(TARGET): $(USRINC)/ctype.h
$(TARGET): $(USRINC)/curses.h
$(TARGET): $(USRINC)/sys/kb.h

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
