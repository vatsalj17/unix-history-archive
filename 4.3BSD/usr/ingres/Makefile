DESTDIR=/
DESTBIN=$(DESTDIR)/usr/bin
# Psuedo makefile for Ingres.
#
# NOTE: This is NOT the "real" makefile, the true makefile
# can be found in source/conf/Makefile. This just allows the
# "make clean", "make DESTDIR=/whatever install" to work.
# However, note that "make binaries" properly installs all the
# binaries in /usr/ingres.  You only need to run the "make install"
# here if you want the binaries some place else.

binaries:
	cd source/conf ; make allinstall

clean:
	cd source/conf ; make allclean

install: binaries
	chdir source/support ; make SYSBIN=$(DESTBIN) sysinstall

