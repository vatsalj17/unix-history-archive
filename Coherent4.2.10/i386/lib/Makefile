# Makefile for building a kernel support library.

DESTDIR=$(K386LIB)
OBJDIR=$(K386OBJ)
PREFIX=.
MANIFEST=$(PREFIX)/lib
MAKEFILE=Makefile
MAKEARGS="CC=$(CC)" "CFLAGS=$(CFLAGS)" \
	 "LDFLAGS=$(LDFLAGS)" "LIBFLAGS=$(LIBFLAGS)"
DESTLIB=$(DESTDIR)/k386.a

all depend:
	+@$(MAKE) $(MAKEARGS) -f ilib.make $*

gcc:
	+@exec $(MAKE)  "CC=$(GCC)" "CFLAGS=$(GCCFLAGS)" \
			"LDFLAGS=$(GLDFLAGS)" "LIBFLAGS=$(GLIBFLAGS)" \
			-f $(MAKEFILE)

setup distrib clean:

manifest:
	@for i in *; do \
		case $$i in \
		RCS)	;; \
		*)	echo $(MANIFEST)/$$i \
			;; \
		esac \
	done
