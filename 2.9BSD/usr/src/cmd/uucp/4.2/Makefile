# @(#)Makefile	4.5  6/29/83  uucp-4.2BSD
# On PDPs and some other systems, '-s -n' is better.
LDFLAGS=-s
#
OWNER=uucp
GROUP=daemon
#
# This version uses the 4.2 bsd directory reading routines.
# Set the LIBNDIR below to reference the "new directory" object library,
# or set it to nothing if your libc.a includes those routines.
#
LIBNDIR=
#
# Set LIBUUCICO to reference object modules and libraries needed by uucico.
# For example, if you define UNET, append -lunet to link to tcpopen.
#
LIBUUCICO=
########################################################################
# Wed Dec 30 21:24:31 EST 1981
# ittvax!swatt: many changes to make this easier to manage
#
# Common object files assembled into a library (uulib.a)
#
# Targets defined are:
#	all(default)	Make all UUCP commands
#	install		Install all executables
#	new		Install executables and make all necessary
#			directories
#	cp		Make and install executables
#	cpdirs:
#	cpfiles:
#	cpuucp		Install uucp program
#	cpuux		Install uux program
#	cpuuxqt		Install uuxqt program
#	cpuucico	Install uucico program
#	cpuulog		Install uulog program
#	cpuuclean	Install uuclean program
#	cpuuname	Install uuname program
#	cpuupoll	Install uupoll program
#	cpuusnap	Install uusnap program
#	cpuuencode	Install uuencode program
#	cpuudecode	Install uudecode program
#	cpuusend	Install uusend/ruusend program
#	clean		Remove all .o files
#	clobber		Remove .o files and executables
#	get:
#	cmp:		all
#	uucp		Make uucp program
#	uux		Make uux program
#	uuxqt		Make uuxqt program
#	uucico		Make uucico program
#	uulog		Make uulog program
#	uuclean		Make uuclean program
#	uuname		Make uuname program
#	uupoll		Make uupoll program
#	uusnap		Make uusnap program
#	uuencode	Make uuencode program
#	uudecode	Make uudecode program
#	uusend		Make uusend program
#	mkdirs		Create necessary directories
#	lint		Run lint on everything
#	lintuucp	Run lint on uucp sources
#	lintuucico	Run lint on uucico sources
#	lintuux		Run lint on uux sources
#	lintuuxqt	Run lint on uuxqt sources
#	lintuulog	Run lint on uulog sources
#	lintuuclean	Run lint on uuclean sources
#	lintuuname	Run lint on uuname sources
#	lintuupoll	Run lint on uupoll sources
#	lintuusnap	Run lint on uusnap sources
#	lintuuencode	Run lint on uuencode sources
#	lintuudecode	Run lint on uudecode sources
#	lintuusend	Run lint on uusend sources
#	tar		Make exportable 'tar' archive of all
#			necessary sources
########################################################################
CC=cc
CFLAGS=-O

#
# Files to export with tar
TAROUT= uucp.tar
HEADRS=	*.h pk.p
SOURCES=*.c
METOO=	[mM]akefile *INSTALL* *CHANGE* *README* *TODO*
MISCDIR= UUAIDS UUDIFF LIBNDIR MAN
TFILES= $(METOO) $(HEADRS) $(SOURCES) $(MISCDIR)

INSDIR=/usr/lib/uucp
BIN=/usr/bin
UPATH=.
APATH=../uuaids
PUBDIR=/usr/spool/uucppublic
SPOOL=/usr/spool/uucp
XQTDIR=/usr/spool/uucp/XTMP
OLDLOG=/usr/lib/uucp/.OLD
# The six subdirs (not counting XTMP).  The order of the D. files is crucial!
SUBDIRS=C. D.`uuname -l`X D.`uuname -l` D. X. TM.
PKON=pkon.o
IOCTL=
CLEAN=
LINTOP=-hbau
COMMANDS=uucp uux uuxqt uucico uulog uuclean uuname \
	 uupoll uusnap uuencode uudecode uusend
COMMLIB=uulib.a
# Common object files
COMMON =anlwrk.o anyread.o assert.o cfgets.o chkpth.o \
	cpmv.o expfile.o gename.o getargs.o getopt.o getprm.o \
	getpw.o getpwinfo.o gio.o gnamef.o gnxseq.o gwd.o imsg.o \
	index.o lastpart.o logent.o mailst.o pk0.o pk1.o prefix.o \
	sdmail.o setline.o shio.o subdir.o sysacct.o systat.o ulockf.o \
	uucpname.o versys.o xqt.o uucpdefs.o
#
# Should steal "make depend" from kernel makefile
#
# Object files which depend on "pk.h"
#
PKDEP=	gio.o pk0.o pk1.o
#
# Object files which depend on "uucp.h"
#
UUCPDEP=anlwrk.o anyread.o assert.o chkpth.o cico.o cntrl.o conn.o condevs.o \
	cpmv.o expfile.o gename.o getpwinfo.o gio.o gnamef.o \
	gnsys.o gnxseq.o gwd.o imsg.o ioctl.o logent.o mailst.o \
	sdmail.o setline.o shio.o subdir.o systat.o ulockf.o \
	uuclean.o uucp.o uucpdefs.o \
	uucpname.o uulog.o uuname.o uupoll.o uux.o uuxqt.o \
	versys.o xqt.o
#
#
LFILES=assert.c cpmv.c expfile.c gename.c getpwinfo.c index.c lastpart.c \
	prefix.c shio.c ulockf.c xqt.c
OUUCP=uucp.o $(COMMLIB)
LUUCP=uucpdefs.c uucp.c gwd.c chkpth.c getargs.c logent.c uucpname.c\
	versys.c 
OUUX=uux.o $(COMMLIB)
LUUX=uucpdefs.c uux.c gwd.c anyread.c chkpth.c getargs.c getprm.c\
	logent.c uucpname.c versys.c
OUUXQT=uuxqt.o $(COMMLIB)
LUUXQT=uucpdefs.c uuxqt.c mailst.c getprm.c gnamef.c logent.c uucpname.c \
	chkpth.c getargs.c anyread.c
OUUCICO=cico.o cntrl.o conn.o condevs.o gnsys.o $(COMMLIB)
LUUCICO=uucpdefs.c cico.c cntrl.c conn.c condevs.c pk0.c pk1.c gio.c anyread.c \
	anlwrk.c chkpth.c getargs.c gnamef.c gnsys.c gnxseq.c \
	imsg.c logent.c sysacct.c systat.c \
	mailst.c uucpname.c setline.c
OUULOG=uulog.o $(COMMLIB)
LUULOG=uucpdefs.c uulog.c prefix.c xqt.c ulockf.c gnamef.c assert.c
OUUCLEAN=uuclean.o $(COMMLIB)
LUUCLEAN=uucpdefs.c uuclean.c gnamef.c prefix.c mailst.c getpwinfo.c\
	 getargs.c
OUUNAME=uuname.o $(COMMLIB)
LUUNAME=uuname.c uucpname.c uucpdefs.c
OUUPOLL=uupoll.o $(COMMLIB)
LUUPOLL=uucpdefs.c uupoll.c systat.c versys.c
OUUSNAP=uusnap.o
LUUSNAP=uusnap.c
OUUENCODE=uuencode.o
LUUENCODE=uuencode.c
OUUDECODE=uudecode.o
LUUDECODE=uudecode.c
OUUSEND=uusend.o
LUUSEND=uusend.c

all:	$(COMMANDS) 

$(COMMANDS):	$(COMMLIB)

$(COMMLIB):	$(COMMON)
	rm -f $@
	: The lorder...tsort below is for systems w/o ranlib
	: But it doesnt hurt to use it. Ignore the spurious "cycle" messages
	ar cr $@ `lorder $(COMMON) | tsort`
	ranlib $@

install:	all cp $(CLEAN)

new:		mkdirs cpfiles

cp:	all cpdirs cpuucp cpuux cpuuxqt cpuucico cpuulog cpuuclean cpuuname \
	cpuupoll cpuusnap cpuuencode cpuudecode cpuusend

cpdirs:
	chown $(OWNER) $(INSDIR)
	chgrp $(GROUP) $(INSDIR)
	chmod 755 $(INSDIR)

cpuucp: uucp
	cp uucp $(BIN)
	chown $(OWNER) $(BIN)/uucp
	chgrp $(GROUP) $(BIN)/uucp
	chmod 6111 $(BIN)/uucp

cpuux: uux
	cp uux $(BIN)
	chown $(OWNER) $(BIN)/uux
	chgrp $(GROUP) $(BIN)/uux
	chmod 6111 $(BIN)/uux
	: chmod u+t $(BIN)/uux; : this helps PDPs

cpuuxqt: uuxqt
	cp uuxqt $(INSDIR)
	chown $(OWNER) $(INSDIR)/uuxqt
	chgrp $(GROUP) $(INSDIR)/uuxqt
	chmod 6110 $(INSDIR)/uuxqt
	: chmod u+t $(INSDIR)/uuxqt; : this helps PDPs

cpuucico: uucico
	cp uucico $(INSDIR)
	chown $(OWNER) $(INSDIR)/uucico
	chgrp $(GROUP) $(INSDIR)/uucico
	: was 6110, but then only root or uucp could exec uucico
	chmod 6111 $(INSDIR)/uucico
	: chmod u+t $(INSDIR)/uucico; : this helps PDPs if uucico is pure

cpuulog: uulog
	cp uulog $(BIN)
	chown $(OWNER) $(BIN)/uulog
	chgrp $(GROUP) $(BIN)/uulog
	chmod 6111 $(BIN)/uulog

cpuuclean: uuclean
	cp uuclean $(INSDIR)
	chown $(OWNER) $(INSDIR)/uuclean
	chgrp $(GROUP) $(INSDIR)/uuclean
	chmod 6110 $(INSDIR)/uuclean

cpuuname: uuname
	cp uuname $(BIN)
	chown $(OWNER) $(BIN)/uuname
	chgrp $(GROUP) $(BIN)/uuname
	chmod 6111 $(BIN)/uuname

cpuupoll: uupoll
	cp uupoll $(BIN)
	chown $(OWNER) $(BIN)/uupoll
	chgrp $(GROUP) $(BIN)/uupoll
	chmod 6111 $(BIN)/uupoll

cpuusnap: uusnap
	cp uusnap $(BIN)
	chown $(OWNER) $(BIN)/uusnap
	chgrp $(GROUP) $(BIN)/uusnap
	chmod 6111 $(BIN)/uusnap

cpuuencode: uuencode
	cp uuencode $(BIN)
	chown $(OWNER) $(BIN)/uuencode
	chgrp $(GROUP) $(BIN)/uuencode
	chmod 6111 $(BIN)/uuencode

cpuudecode: uudecode
	cp uudecode $(BIN)
	chown $(OWNER) $(BIN)/uudecode
	chgrp $(GROUP) $(BIN)/uudecode
	chmod 6111 $(BIN)/uudecode

cpuusend: uusend
	cp uusend $(BIN)
	chown $(OWNER) $(BIN)/uusend
	chgrp $(GROUP) $(BIN)/uusend
	chmod 6111 $(BIN)/uusend
	rm -f $(BIN)/ruusend
	ln $(BIN)/uusend $(BIN)/ruusend

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(COMMLIB)
	-rm -f $(COMMANDS)

get:
	cp $(UPATH)/* .

cmp:	all
	-cmp uucp $(BIN)/uucp
	-cmp uux $(BIN)/uux
	-cmp uuxqt $(INSDIR)/uuxqt
	-cmp uucico $(INSDIR)/uucico
	-cmp uulog $(BIN)/uulog
	-cmp uuclean $(INSDIR)/uuclean
	-cmp uuname $(BIN)/uuname
	-cmp uupoll $(BIN)/uupoll
	-cmp uusnap $(BIN)/uusnap
	-cmp uuencode $(BIN)/uuencode
	-cmp uudecode $(BIN)/uudecode
	-cmp uusend $(BIN)/uusend
	-cmp uusend $(BIN)/ruusend; : In case the link got broken

uucp:	$(OUUCP) 
	$(CC) $(LDFLAGS) $(OUUCP) -o uucp

uux:	$(OUUX) 
	$(CC) $(LDFLAGS) $(OUUX) -o uux

uuxqt:	$(OUUXQT) 
	$(CC) $(LDFLAGS) $(OUUXQT) $(LIBNDIR) -o uuxqt

uucico:	$(OUUCICO) $(IOCTL) $(PKON)
	$(CC) $(LDFLAGS) $(OUUCICO) $(IOCTL) $(PKON) $(LIBNDIR) $(LIBUUCICO) -o uucico

uulog:	$(OUULOG)
	$(CC) $(LDFLAGS) $(OUULOG) -o uulog

uuclean: $(OUUCLEAN)
	$(CC) $(LDFLAGS) $(OUUCLEAN) $(LIBNDIR) -o uuclean

uuname:	$(OUUNAME)
	$(CC) $(LDFLAGS) $(OUUNAME) -o uuname
 
uupoll:	$(OUUPOLL)
	$(CC) $(LDFLAGS) $(OUUPOLL) -o uupoll

uusnap:	$(OUUSNAP)
	$(CC) $(LDFLAGS) $(OUUSNAP) $(LIBNDIR) -o uusnap

uuencode: $(OUUENCODE)
	$(CC) $(LDFLAGS) $(OUUENCODE) -o uuencode

uudecode: $(OUUDECODE)
	$(CC) $(LDFLAGS) $(OUUDECODE) -o uudecode

uusend: $(OUUSEND)
	$(CC) $(LDFLAGS) $(OUUSEND) -o uusend

#
# Header file dependencies
#
$(PKDEP):	pk.h
$(UUCPDEP):	uucp.h

mkdirs:
	-mkdir $(INSDIR)
	: Note: mode 700 would be safer. Also,  L.sys etc should be 600.
	chmod 755 $(INSDIR)
	chown $(OWNER) $(INSDIR)
	chgrp $(GROUP) $(INSDIR)
	-mkdir $(SPOOL)
	chmod 755 $(SPOOL)
	chown $(OWNER) $(SPOOL)
	chgrp $(GROUP) $(SPOOL)
	-mkdir $(PUBDIR)
	chmod 777 $(PUBDIR)
	chown $(OWNER) $(PUBDIR)
	chgrp $(GROUP) $(PUBDIR)
	-mkdir $(XQTDIR)
	chmod 755 $(XQTDIR)
	chown $(OWNER) $(XQTDIR)
	chgrp $(GROUP) $(XQTDIR)
	: -mkdir $(OLDLOG)
	: chmod 755 $(OLDLOG)
	: chown $(OWNER) $(OLDLOG)
	: chgrp $(GROUP) $(OLDLOG)
	:
	: make subdirectories of /usr/spool/uucp.
	: The XQTDIR directory is used by uuxqt and should always be made.
	: These are needed when using subdirectories
	cd $(SPOOL); for i in $(SUBDIRS); do mkdir $$i; chown $(UUCP) $$i; chgrp $(GROUP) $$i; chmod 755 $$i; done

mvspoolfiles:
	: Alas, this is slow on large spool directories.
	# $(SPOOL); for i in $(SUBDIRS); do find $$i* ! -type d -a -exec mv {} $$i ';' -a -exec echo {} put in $$i ';' ; done

cpfiles:
	cp $(APATH)/L* $(APATH))/USERFILE ${DESTDIR}$(INSDIR)
#	cp $(APATH)/uudemon* ${DESTDIR}$(INSDIR)
#	chmod 755 ${DESTDIR}$(INSDIR)/uudemon*
	chmod 600 $(DESTDIR)$(INSDIR)/L* ${DESTDIR}$(INSDIR)/USERFILE
	-chown $(OWNER) ${DESTDIR}$(INSDIR)/*
	-chgrp $(GROUP) ${DESTDIR}$(INSDIR)/*

lint:	lintuucp lintuucico lintuux lintuuxqt lintuulog lintuuclean\
	lintuuname lintuupoll lintuusnap lintuuencode lintuudecode lintuusend

lintuucp:
	lint $(LINTOP) $(LUUCP) $(LFILES)

lintuucico:
	lint $(LINTOP) $(LUUCICO) $(LFILES)

lintuux:
	lint $(LINTOP) $(LUUX) $(LFILES)

lintuuxqt:
	lint $(LINTOP) $(LUUXQT) $(LFILES)

lintuulog:
	lint $(LINTOP) $(LUULOG)

lintuuclean:
	lint $(LINTOP) $(LUUCLEAN)

lintuuname:
	lint $(LINTOP) $(LUUNAME)

lintuupoll:
	lint $(LINTOP) $(LUUPOLL)

lintuusnap:
	lint $(LINTOP) $(LUUSNAP)

# Make exportable 'tar' archive package
#
tar:
	tar cbf 1 $(TAROUT) $(TFILES)
