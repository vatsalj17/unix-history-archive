# Standard 'conf'-system makefile with productions for standardized targets;
# 'make install', 'make clean', 'make manifest', 'make setup', 'make distrib'
# See the top-level 'conf' system Makefile for more details.

INSTALL_DIR=..
TMPDIR=$(INSTALL_DIR)/obj
PREFIX=.
MANIFEST=$(PREFIX)/src
USRINC=/usr/include
GCC=gcc
GCCFLAGS=-O2 $(CFLAGS) -Wall -ansi -pedantic
GLDFLAGS=-nostdlib /lib/crts0.o
GLIBFLAGS=/lib/libc.a

TARGETS=$(INSTALL_DIR)/stune $(INSTALL_DIR)/mdevice $(INSTALL_DIR)/sdevice \
	$(INSTALL_DIR)/keeplist

all gcc install depend:

clean:
	rm -f $(INSTALL_DIR)/conf.[ch] $(INSTALL_DIR)/drvbld.mak $(TMPDIR)/*

setup: mkobj $(TARGETS)


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

mkobj:
	[ -d $(TMPDIR) ] || mkdir $(TMPDIR)

$(INSTALL_DIR)/mtune: mtune
	ln -f mtune $(INSTALL_DIR)

$(INSTALL_DIR)/stune: stune
	ln -f stune $(INSTALL_DIR)

$(INSTALL_DIR)/mdevice: mdevice
	ln -f mdevice $(INSTALL_DIR)

$(INSTALL_DIR)/sdevice: sdevice
	ln -f sdevice $(INSTALL_DIR)

$(INSTALL_DIR)/template.mak: template.mak
	ln -f template.mak $(INSTALL_DIR)
