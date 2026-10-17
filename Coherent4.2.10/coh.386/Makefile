#
#	Makefile for Coherent (TM) 386 base system
#

USRINC=/usr/include
PREFIX=.
MANIFEST=$(PREFIX)/coh.386
DESTLIB=$(K386LIB)/k386.a
MAKEFILE=Makefile
MAKEARGS="CC=$(CC)" "CFLAGS=$(CFLAGS)" \
	 "LDFLAGS=$(LDFLAGS)" "LIBFLAGS=$(LIBFLAGS)"
GCC=gcc
GCCFLAGS=-O2 $(CFLAGS) # -Wall -ansi -pedantic
GLDFLAGS=-nostdlib /lib/crts0.o
GLIBFLAGS=/lib/libc.a

all depend:
	+cd lib; $(MAKE) $(MAKEARGS) $*
	+$(MAKE) $(MAKEARGS) -f coh.make $*

gcc:
	+@exec $(MAKE)  "CC=$(GCC)" "CFLAGS=$(GCCFLAGS)" \
			"LDFLAGS=$(GLDFLAGS)" "LIBFLAGS=$(GLIBFLAGS)" \
			-f $(MAKEFILE)

clean:
	rm -f *.o
	@cd lib; $(MAKE) clean

manifest:
	@for i in *; do \
		case $$i in \
		RCS | lib) \
			;; \
		*)	echo $(MANIFEST)/$$i \
			;; \
		esac \
	done
	@cd lib; $(MAKE) "PREFIX=$(MANIFEST)" $@

setup:
