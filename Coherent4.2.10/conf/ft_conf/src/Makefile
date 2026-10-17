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

TARGETS=$(INSTALL_DIR)/stune $(INSTALL_DIR)/sdevice $(INSTALL_DIR)/keeplist

all gcc install depend distrib:

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

mkobj:
	[ -d $(TMPDIR) ] || mkdir $(TMPDIR)

$(INSTALL_DIR)/mtune: mtune
	cp mtune $(INSTALL_DIR)

$(INSTALL_DIR)/stune: stune
	cp stune $(INSTALL_DIR)

$(INSTALL_DIR)/mdevice: mdevice
	cp mdevice $(INSTALL_DIR)

$(INSTALL_DIR)/sdevice: sdevice
	cp sdevice $(INSTALL_DIR)

$(INSTALL_DIR)/keeplist: keeplist.sh

$(INSTALL_DIR)/template.mak: template.mak
	cp template.mak $(INSTALL_DIR)
