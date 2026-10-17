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

TARGETS=$(DRIVER)

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

$(DRIVER): echo.c
	$(CC) $(CFLAGS) -o $@ -c echo.c

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

$(TARGETS): $(USRINC)/common/ccompat.h
$(TARGETS): $(USRINC)/sys/types.h
$(TARGETS): $(USRINC)/sys/stream.h
$(TARGETS): $(USRINC)/sys/stropts.h
$(TARGETS): $(USRINC)/sys/cred.h
$(TARGETS): $(USRINC)/stddef.h
$(TARGETS): $(USRINC)/common/tricks.h
$(TARGETS): $(USRINC)/sys/compat.h
$(TARGETS): $(USRINC)/stdio.h
$(TARGETS): $(USRINC)/unistd.h
$(TARGETS): $(USRINC)/poll.h
$(TARGETS): $(USRINC)/fcntl.h
$(TARGETS): $(USRINC)/stdarg.h
$(TARGETS): $(USRINC)/time.h
$(TARGETS): $(USRINC)/signal.h
$(TARGETS): $(USRINC)/stropts.h
$(TARGETS): $(USRINC)/fcntl.h
$(TARGETS): $(USRINC)/stdio.h
$(TARGETS): $(USRINC)/poll.h
$(TARGETS): $(USRINC)/errno.h

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
