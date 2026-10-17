#
# Makefile for ibm specific coherent sources and coherent images.
#

USRINC=/usr/include
PREFIX=.
MANIFEST=$(PREFIX)/io.386
DESTLIB=$(K386LIB)/k386.a
MAKEFILE=Makefile
MAKEARGS="CC=$(CC)" "CFLAGS=$(CFLAGS)" \
	 "LDFLAGS=$(LDFLAGS)" "LIBFLAGS=$(LIBFLAGS)"
DESTLIB=$(K386LIB)/k386.a
USRINC=/usr/include
GCC=gcc
GCCFLAGS=-O2 $(CFLAGS) # -Wall -ansi -pedantic
GLDFLAGS=-nostdlib /lib/crts0.o
GLIBFLAGS=/lib/libc.a

all depend:
	+@$(MAKE) $(MAKEARGS) -f io.make $@

gcc:
	+@exec $(MAKE)	"CC=$(GCC)" "CFLAGS=$(GCCFLAGS)" \
			"LDFLAGS=$(GLDFLAGS)" "LIBFLAGS=$(GLIBFLAGS)" \
			-f $(MAKEFILE)

setup distrib:

clean:
	rm -f *.o

manifest:
	@for i in *; do \
		case $$i in \
		RCS | lib) \
			;; \
		*)	echo $(MANIFEST)/$$i \
			;; \
		esac \
	done

