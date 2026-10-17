#
#	makefile for Coherent 386 - overall system build
#
#	Do not use this makefile directly!  Use a build script.
#	(Unless you know what you are doing.)
#

PREFIX=.
MANIFEST=$(PREFIX)/i386
MAKEFILE=Makefile
MAKEARGS="CC=$(CC)" "CFLAGS=$(CFLAGS)" \
	 "LDFLAGS=$(LDFLAGS)" "LIBFLAGS=$(LIBFLAGS)"
DESTLIB=$(K386LIB)/k386.a
USRINC=/usr/include
GCC=gcc
GCCFLAGS=-O2 $(CFLAGS) -Wall -ansi -pedantic -Wno-implicit
GLDFLAGS=-nostdlib /lib/crts0.o
GLIBFLAGS=/lib/libc.a

all: coh io
all depend: 
	+cd lib; $(MAKE) $(MAKEARGS) $*
	+$(MAKE) $(MAKEARGS) -f i386.make $*
	@/bin/echo "Installation libraries up to date."

gcc:
	+@exec $(MAKE)	"CC=$(GCC)" "CFLAGS=$(GCCFLAGS)" \
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
	[ ! -d ../lib ] && mkdir ../lib

coh:
	+cd ../coh.386; $(MAKE) $(MAKEARGS)

io:
	+cd ../io.386; $(MAKE) $(MAKEARGS)
