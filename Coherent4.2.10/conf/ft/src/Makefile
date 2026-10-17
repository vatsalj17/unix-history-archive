# Standard 'conf'-system makefile with productions for standardized targets;
# 'make install', 'make clean', 'make manifest', 'make setup', 'make distrib'
# See the top-level 'conf' system Makefile for more details.

INSTALL_DIR=..
PREFIX=.
MANIFEST=$(PREFIX)/src
MAKEFILE=Makefile
USRINC=/usr/include
GCC=gcc
#GCCFLAGS=-O2 $(CFLAGS) -Wall -ansi -pedantic
GCCFLAGS=-Wall -ansi -pedantic
GLDFLAGS=-nostdlib /lib/crts0.o
GLIBFLAGS=/lib/libc.a

DRIVER=$(INSTALL_DIR)/Driver.a

TARGETS=$(INSTALL_DIR)/Space.c $(INSTALL_DIR)/mkdev $(DRIVER) \
	$(INSTALL_DIR)/Stub.o

OBJS= $(DRIVER)(ft.o) $(DRIVER)(ftCvt.o) $(DRIVER)(ftMotion.o) \
	$(DRIVER)(ftGRB.o) $(DRIVER)(ecc.o) $(DRIVER)(eccas.o) \
	$(DRIVER)(ftx.o) $(DRIVER)(ftxqueue.o)

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

$(DRIVER): $(OBJS)

depend:
	echo > makedep
	grep "^\#include" * \
		| sed 's/\(.*\)\.c:.*"\(.*\)".*/$$(DRIVER)(\1.o): \2/' \
		| sed '/\#include/d' >> makedep
	grep "^\#include" * \
		| sed 's/\(.*\)\.c:.*<\(.*\)>.*/$$(DRIVER)(\1.o): $$(USRINC)\/\2/' \
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

$(DRIVER)(Stub.o): $(USRINC)/sys/con.h
$(DRIVER)(ft.o): $(USRINC)/sys/coherent.h
$(DRIVER)(ft.o): $(USRINC)/coh/defer.h
$(DRIVER)(ft.o): $(USRINC)/coh/i386lib.h
$(DRIVER)(ft.o): $(USRINC)/coh/misc.h
$(DRIVER)(ft.o): $(USRINC)/coh/proc.h
$(DRIVER)(ft.o): $(USRINC)/coh/timeout.h
$(DRIVER)(ft.o): $(USRINC)/kernel/v_types.h
$(DRIVER)(ft.o): $(USRINC)/errno.h
$(DRIVER)(ft.o): $(USRINC)/sys/buf.h
$(DRIVER)(ft.o): $(USRINC)/sys/cmn_err.h
$(DRIVER)(ft.o): $(USRINC)/sys/con.h
$(DRIVER)(ft.o): $(USRINC)/sys/devices.h
$(DRIVER)(ft.o): $(USRINC)/sys/fdc765.h
$(DRIVER)(ft.o): $(USRINC)/sys/fdioctl.h
$(DRIVER)(ft.o): $(USRINC)/sys/file.h
$(DRIVER)(ft.o): $(USRINC)/sys/mmu.h
$(DRIVER)(ft.o): $(USRINC)/sys/seg.h
$(DRIVER)(ft.o): $(USRINC)/sys/stat.h
$(DRIVER)(ft.o): $(USRINC)/sys/tape.h
$(DRIVER)(ft.o): $(USRINC)/sys/ft.h
$(DRIVER)(ft.o): $(USRINC)/sys/ftioctl.h
$(DRIVER)(ft.o): $(USRINC)/sys/ftx.h
$(DRIVER)(ftCvt.o): $(USRINC)/sys/coherent.h
$(DRIVER)(ftCvt.o): $(USRINC)/sys/cmn_err.h
$(DRIVER)(ftCvt.o): $(USRINC)/common/_tricks.h
$(DRIVER)(ftCvt.o): $(USRINC)/sys/ft.h
$(DRIVER)(ftGRB.o): $(USRINC)/sys/coherent.h
$(DRIVER)(ftGRB.o): $(USRINC)/sys/cmn_err.h
$(DRIVER)(ftGRB.o): $(USRINC)/common/ccompat.h
$(DRIVER)(ftGRB.o): $(USRINC)/common/xdebug.h
$(DRIVER)(ftGRB.o): $(USRINC)/sys/ft.h
$(DRIVER)(ftMotion.o): $(USRINC)/sys/coherent.h
$(DRIVER)(ftMotion.o): $(USRINC)/sys/cmn_err.h
$(DRIVER)(ftMotion.o): $(USRINC)/sys/param.h
$(DRIVER)(ftMotion.o): $(USRINC)/sys/ft.h
$(DRIVER)(ftx.o): $(USRINC)/sys/coherent.h
$(DRIVER)(ftx.o): $(USRINC)/sys/cmn_err.h
$(DRIVER)(ftx.o): $(USRINC)/sys/dmac.h
$(DRIVER)(ftx.o): $(USRINC)/sys/fdc765.h
$(DRIVER)(ftx.o): $(USRINC)/coh/misc.h
$(DRIVER)(ftx.o): $(USRINC)/sys/ft.h
$(DRIVER)(ftx.o): $(USRINC)/sys/ftx.h
$(DRIVER)(ftx.o): $(USRINC)/sys/ftxreq.h
$(DRIVER)(ftxqueue.o): $(USRINC)/sys/coherent.h
$(DRIVER)(ftxqueue.o): $(USRINC)/sys/cmn_err.h
$(DRIVER)(ftxqueue.o): $(USRINC)/sys/ft.h
$(DRIVER)(ftxqueue.o): $(USRINC)/sys/ftxreq.h
$(INSTALL_DIR)/Space.c: $(USRINC)/sys/con.h
$(INSTALL_DIR)/Space.c: $(USRINC)/common/ccompat.h
$(INSTALL_DIR)/Space.c: $(USRINC)/sys/ft.h

# DEPENDENCIES MUST END AT END OF FILE: IF YOU PUT STUFF HERE, IT WILL GO AWAY
# See make depend, above
