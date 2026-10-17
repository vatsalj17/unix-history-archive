# Makefile for building a kernel support library.

DESTDIR=$(K386LIB)
OBJDIR=$(K386OBJ)
PREFIX=.
MANIFEST=$(PREFIX)/lib
DESTLIB=$(DESTDIR)/k386.a
MAKEFILE=Makefile
MAKEARGS="CC=$(CC)" "CFLAGS=$(CFLAGS)" \
	 "LDFLAGS=$(LDFLAGS)" "LIBFLAGS=$(LIBFLAGS)"
USRINC=/usr/include
GCC=gcc
GCCFLAGS=-O2 $(CFLAGS) -Wall -ansi -pedantic
GLDFLAGS=-nostdlib /lib/crts0.o
GLIBFLAGS=/lib/libc.a

all depend:
	+@$(MAKE) $(MAKEARGS) -f clib.make $@

gcc:
	+@exec $(MAKE)  "CC=$(GCC)" "CFLAGS=$(GCCFLAGS)" \
			"LDFLAGS=$(GLDFLAGS)" "LIBFLAGS=$(GLIBFLAGS)" \
			-f $(MAKEFILE)

manifest:
	@for i in *; do \
		case $$i in \
		RCS)	;; \
		*)	echo $(MANIFEST)/$$i \
			;; \
		esac \
	done

clean distrib setup:
