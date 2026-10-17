# Simple makefile for 'devadm' under Coherent. This doesn't change very often,
# so keeping the .o's around doesn't seem like a win.

INSTALL_DIR=.
PREFIX=.
MANIFEST=$(PREFIX)/devadm
USRINC=/usr/include
MAKEFILE=Makefile
GCC=gcc
GCCFLAGS=-O2 -Wall -ansi -pedantic
GLDFLAGS=-nostdlib /lib/ndp/crts0.o
GLIBFLAGS=/lib/libc.a

SOURCES= assign.c buildobj.c devadm.c ehand.c input.c lex.c mdev.c mkconf.c \
		mklink.c mktune.c mtune.c read.c sdev.c stune.c symbol.c \
		val_str.c

TARGET= $(INSTALL_DIR)/devadm

all: $(TARGET)

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

$(INSTALL_DIR)/devadm:	$(SOURCES)
	$(CC) -o $@ $(SOURCES)
	strip $@
	rm -f $(SOURCES:.c=.o)

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

$(TARGET): $(USRINC)/sys/compat.h
$(TARGET): $(USRINC)/sys/types.h
$(TARGET): $(USRINC)/stddef.h
$(TARGET): $(USRINC)/stdlib.h
$(TARGET): $(USRINC)/stdio.h
$(TARGET): $(USRINC)/sys/compat.h
$(TARGET): $(USRINC)/stdlib.h
$(TARGET): $(USRINC)/string.h
$(TARGET): $(USRINC)/assert.h
$(TARGET): $(USRINC)/sys/compat.h
$(TARGET): $(USRINC)/stdlib.h
$(TARGET): $(USRINC)/unistd.h
$(TARGET): $(USRINC)/stdio.h
$(TARGET): $(USRINC)/string.h
$(TARGET): $(USRINC)/errno.h
$(TARGET): $(USRINC)/sys/compat.h
$(TARGET): $(USRINC)/setjmp.h
$(TARGET): $(USRINC)/stdarg.h
$(TARGET): $(USRINC)/stdio.h
$(TARGET): $(USRINC)/stdlib.h
$(TARGET): $(USRINC)/common/tricks.h
$(TARGET): $(USRINC)/sys/compat.h
$(TARGET): $(USRINC)/stddef.h
$(TARGET): $(USRINC)/stdio.h
$(TARGET): $(USRINC)/stdlib.h
$(TARGET): $(USRINC)/string.h
$(TARGET): $(USRINC)/stdarg.h
$(TARGET): $(USRINC)/sys/compat.h
$(TARGET): $(USRINC)/string.h
$(TARGET): $(USRINC)/sys/compat.h
$(TARGET): $(USRINC)/stdlib.h
$(TARGET): $(USRINC)/string.h
$(TARGET): $(USRINC)/sys/compat.h
$(TARGET): $(USRINC)/sys/types.h
$(TARGET): $(USRINC)/stddef.h
$(TARGET): $(USRINC)/stdio.h
$(TARGET): $(USRINC)/time.h
$(TARGET): $(USRINC)/string.h
$(TARGET): $(USRINC)/fcntl.h
$(TARGET): $(USRINC)/sys/compat.h
$(TARGET): $(USRINC)/stdio.h
$(TARGET): $(USRINC)/stdlib.h
$(TARGET): $(USRINC)/time.h
$(TARGET): $(USRINC)/string.h
$(TARGET): $(USRINC)/errno.h
$(TARGET): $(USRINC)/unistd.h
$(TARGET): $(USRINC)/sys/compat.h
$(TARGET): $(USRINC)/stdio.h
$(TARGET): $(USRINC)/time.h
$(TARGET): $(USRINC)/limits.h
$(TARGET): $(USRINC)/string.h
$(TARGET): $(USRINC)/sys/compat.h
$(TARGET): $(USRINC)/stdlib.h
$(TARGET): $(USRINC)/sys/compat.h
$(TARGET): $(USRINC)/limits.h
$(TARGET): $(USRINC)/string.h
$(TARGET): $(USRINC)/sys/compat.h
$(TARGET): $(USRINC)/stdlib.h
$(TARGET): $(USRINC)/string.h
$(TARGET): $(USRINC)/sys/compat.h
$(TARGET): $(USRINC)/stdlib.h
$(TARGET): $(USRINC)/sys/compat.h
$(TARGET): $(USRINC)/stdio.h
$(TARGET): $(USRINC)/stdlib.h
$(TARGET): $(USRINC)/string.h
$(TARGET): $(USRINC)/errno.h

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
