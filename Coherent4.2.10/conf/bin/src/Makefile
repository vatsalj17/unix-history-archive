# Standard 'conf'-system makefile with productions for standardized targets;
# 'make install', 'make clean', 'make manifest', 'make setup', 'make distrib'
# See the top-level 'conf' system Makefile for more details.

INSTALL_DIR=..
PREFIX=.
MANIFEST=$(PREFIX)/src
MAKEARGS= CC="$(CC)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" \
	   LIBFLAGS="$(LIBFLAGS)"

all gcc distrib setup install clean depend:
	+@j=$$(pwd); for i in */Makefile; do \
		cd $${i%/*}; \
		$(MAKE) $(MAKEFLAGS) INSTALL_DIR=../$(INSTALL_DIR) $@ || exit 1; \
		cd $$j; \
	done

manifest:
	+@echo $(MANIFEST)/Makefile
	+@j=$$(pwd); for i in */Makefile; do \
		cd $${i%/*}; \
		$(MAKE) $(MAKEARGS) PREFIX=$(MANIFEST) $@ || exit 1; \
		cd $$j; \
	done
	
