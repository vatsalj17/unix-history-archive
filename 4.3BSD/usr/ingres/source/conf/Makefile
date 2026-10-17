###################################################################
###################################################################
###								###
###								###
###	*****    *   *    *****    ****     *****     ****	###
###	  *      **  *    *        *   *    *        *    	###
###	  *      * * *    * ***    ****     ***       *** 	###
###	  *      *  **    *   *    *  *     *            *	###
###	*****    *   *    *****    *   *    *****    **** 	###
###								###
###								###
###	      A relational database management system		###
###								###
###			    RELEASE 8				###
###								###
###################################################################
###################################################################
#
#	For information contact:
#		Mike Stonebraker
#		Dept. of Electrical Engineering &
#		  Computer Science
#		University of California
#		Berkeley, California  94720
#

#
#	@(#)Makefile	8.3	1/31/86
#

#
#  This makefile compiles most of the modules for the bulk of
#  the INGRES system.  The modules affected are those used by
#  the "ingres" command itself.  Stand-alone utilities such as
#  "printr" and "restore" are in the 'support' directory.
#

#
#	Major recompilation will take place from this directory.
#	There are several interesting entry points:
#		allclean	cleans all cruft from all
#				directories.
#		allinstall	installs binaries in the INGRES
#				bin directory.
#			INSTALL=actual entry point called.
#		sysinstall	installs that subset of the INGRES
#				system in public directories.
#			SYSBIN=	bin directory that the binaries
#				should live in.
#			SYSLIB=	directory that libraries should live
#				in (typically /usr/lib).
#	for all of these:
#		CPUTYPE=type of cpu you are running on, either VAX
#			or SUN.
#
#	Examples of usage:
#		To recompile entire system from sources:
#			make allclean allinstall
#		To install INGRES for general users:
#			make allinstall sysinstall
#		To print the entire system:
#			make install INSTALL=print
#			{yes, this is cheating}
#

#
#  General system paths
#

# the ingres subtree
INGRES=	../..
# the location of the system header files
H=	../h
# the residence of the system (internal) libraries
LIB=	$(INGRES)/lib
# the residence of the system (internal) binaries
# this entry must be reflected in the process table
BIN=	$(INGRES)/bin
# the location of user binaries
SYSBIN=	/usr/local
# the location of user libraries (libq.a)
SYSLIB=	/usr/local/lib
# the cputype, VAX or SUN
CPUTYPE=VAX

ALL=	monitor univingres sysmonitor sysmodfunc printadmin sysdump

SRCS=	monitor.c univingres.c sysdump.c version.c sysmonitor.c sysmodfunc.c

#
#  Libraries
#
#	Libraries should be loaded in the order listed below.
#
CTLMOD=	$(LIB)/ctlmod
IUTIL=	$(LIB)/iutil
GUTIL=	$(LIB)/gutil
MATH=	-lm

ALLLIB=	$(CTLMOD) $(IUTIL) $(GUTIL)

#
#  Binaries
#
MONITOR=$(LIB)/monitor
SYSMONITOR=$(LIB)/sysmonlib
PARSER=	$(LIB)/parser
QRYMOD=	$(LIB)/qrymod
DECOMP=	$(LIB)/decomp
OVQP=	$(LIB)/ovqp
DBULIB=	$(LIB)/dbulib

#
#  Assorted commands & other garbage
#
CFLAGS=	-I$H -O
LDFLAGS=
CHMOD=	chmod
BINMODE=751
SUMODE=	4751
CP=	cp
GET=	sccs get
REL=
PR=	print
VGRIND=	csh /usr/ucb/vgrind
HEADER=	Configuration Tables
MAKE=	make CPUTYPE=$(CPUTYPE)
MFLAGS=	-k
INSTALL=install

#
#  And the processes....
#
#	Proper loading order is:
#		conf table
#		library(s) for modules
#		ctlmod library
#		iutil library
#		gutil library
#
#  Sysmodfunc

sysmodfunc: version.o sysmodfunc.o $(DBULIB) $(CTLMOD) $(ALLLIB)
	$(CC) $(LDFLAGS) version.o $@.o $(DBULIB) $(OTHERS) $(ALLLIB) -o $@
	$(CHMOD) $(SUMODE) $@
	ls -l $@; size $@

#  Sysdump

sysdump: version.o sysdump.o $(CTLMOD) $(ALLLIB)
	$(CC) $(LDFLAGS) version.o $@.o $(OTHERS) $(ALLLIB) -o $@
	$(CHMOD) $(SUMODE) $@
	ls -l $@; size $@

#  Monitor

monitor: version.o monitor.o $(MONITOR) $(CTLMOD) $(IUTIL) $(GUTIL)
	$(CC) $(LDFLAGS) version.o $@.o $(OTHERS) $(MONITOR) $(CTLMOD) $(IUTIL) $(GUTIL) -o $@
	$(CHMOD) $(BINMODE) $@
	ls -l $@; size $@

#  SysMonitor 

sysmonitor: version.o sysmonitor.o $(SYSMONITOR) $(DBULIB) $(IUTIL) $(GUTIL)
	$(CC) $(LDFLAGS) version.o $@.o $(OTHERS) $(SYSMONITOR) $(CTLMOD) $(IUTIL) $(GUTIL) -o $@
	$(CHMOD) $(BINMODE) $@
	ls -l $@; size $@

#  Full System

ALLLIBS=$(PARSER) $(QRYMOD) $(DECOMP) $(OVQP) $(DBULIB) $(ALLLIB)

univingres: version.o univingres.o $(ALLLIBS)
	$(CC) $(LDFLAGS) version.o $@.o $(OTHERS) $(ALLLIBS) $(MATH) -o $@
	$(CHMOD) $(SUMODE) $@
	ls -l $@; size $@

#################################################################
#
#	Install the world
#

install: $(BIN)/univingres $(BIN)/sysdump $(BIN)/monitor \
	 $(BIN)/sysmonitor $(BIN)/sysmodfunc

$(BIN)/monitor: monitor
	$(CP) monitor $(BIN)/monitor
$(BIN)/sysmonitor: sysmonitor
	$(CP) sysmonitor $(BIN)/sysmonitor
$(BIN)/parser: parser
	$(CP) parser $(BIN)/parser; $(CHMOD) $(SUMODE) $(BIN)/parser
$(BIN)/qrymod: qrymod
	$(CP) qrymod $(BIN)/qrymod; $(CHMOD) $(SUMODE) $(BIN)/qrymod
$(BIN)/decomp: decomp
	$(CP) decomp $(BIN)/decomp; $(CHMOD) $(SUMODE) $(BIN)/decomp
$(BIN)/alldbu: alldbu
	$(CP) alldbu $(BIN)/alldbu; $(CHMOD) $(SUMODE) $(BIN)/alldbu
$(BIN)/univingres: univingres
	$(CP) univingres $(BIN)/univingres; $(CHMOD) $(SUMODE) $(BIN)/univingres
$(BIN)/sysdump: sysdump
	$(CP) sysdump $(BIN)/sysdump; $(CHMOD) $(SUMODE) $(BIN)/sysdump
$(BIN)/sysmodfunc: sysmodfunc
	$(CP) sysmodfunc $(BIN)/sysmodfunc; $(CHMOD) $(SUMODE) $(BIN)/sysmodfunc

allinstall: version.o all1 install

all1:
	cd ../h; $(MAKE) $(MFLAGS) $(INSTALL)
	cd ../gutil; $(MAKE) $(MFLAGS) $(INSTALL)
	cd ../iutil; $(MAKE) $(MFLAGS) $(INSTALL)
	cd ../ctlmod; $(MAKE) $(MFLAGS) $(INSTALL)
	cd ../dbu; $(MAKE) $(MFLAGS) $(INSTALL)
	cd ../decomp; $(MAKE) $(MFLAGS) $(INSTALL)
	cd ../equel; $(MAKE) $(MFLAGS) $(INSTALL)
	cd ../libq; $(MAKE) $(MFLAGS) $(INSTALL)
	cd ../monitor; $(MAKE) $(MFLAGS) $(INSTALL)
	cd ../ovqp; $(MAKE) $(MFLAGS) $(INSTALL)
	cd ../parser; $(MAKE) $(MFLAGS) $(INSTALL)
	cd ../qrymod; $(MAKE) $(MFLAGS) $(INSTALL)
	cd ../support; $(MAKE) $(MFLAGS) $(INSTALL)

# install the user programs
sysinstall:
	@echo 'you may have to be root for this to work....'
	cd ../support; $(MAKE) $(MFLAGS) SYSBIN=$(SYSBIN) SYSLIB=$(SYSLIB) sysinstall
	cd ../equel; $(MAKE) $(MFLAGS) SYSBIN=$(SYSBIN) SYSLIB=$(SYSLIB) sysinstall
	cd ../libq; $(MAKE) $(MFLAGS) SYSBIN=$(SYSBIN) SYSLIB=$(SYSLIB) sysinstall

#################################################################
#
#	Miscellaneous entries
#

new:
	rm -f $(ALL)

print: sources
	$(PR) Makefile *.[csy]

vgrind: sources
	cp /dev/null index
	$(VGRIND) -h "$(HEADER) (Release 8)" -n Makefile
	$(VGRIND) -h "$(HEADER) (Release 8)" *.[csy]
	$(VGRIND) -h "$(HEADER) (Release 8)" -x index

clean: new
	rm -f *.o
	rm -f a.out core

allclean: clean
	cd ../h; $(MAKE) $(MFLAGS) clean
	cd ../gutil; $(MAKE) $(MFLAGS) clean
	cd ../iutil; $(MAKE) $(MFLAGS) clean
	cd ../ctlmod; $(MAKE) $(MFLAGS) clean
	cd ../dbu; $(MAKE) $(MFLAGS) clean
	cd ../decomp; $(MAKE) $(MFLAGS) clean
	cd ../equel; $(MAKE) $(MFLAGS) clean
	cd ../libq; $(MAKE) $(MFLAGS) clean
	cd ../monitor; $(MAKE) $(MFLAGS) clean
	cd ../ovqp; $(MAKE) $(MFLAGS) clean
	cd ../parser; $(MAKE) $(MFLAGS) clean
	cd ../qrymod; $(MAKE) $(MFLAGS) clean
	cd ../support; $(MAKE) $(MFLAGS) clean

tags: 
	cp /dev/null ../tags
	cd ../h; $(MAKE) $(MFLAGS) systags
	cd ../gutil; $(MAKE) $(MFLAGS) systags
	cd ../iutil; $(MAKE) $(MFLAGS) systags
	cd ../ctlmod; $(MAKE) $(MFLAGS) systags
	cd ../dbu; $(MAKE) $(MFLAGS) systags
	cd ../decomp; $(MAKE) $(MFLAGS) systags
	cd ../equel; $(MAKE) $(MFLAGS) systags
	cd ../libq; $(MAKE) $(MFLAGS) systags
	cd ../monitor; $(MAKE) $(MFLAGS) systags
	cd ../ovqp; $(MAKE) $(MFLAGS) systags
	cd ../parser; $(MAKE) $(MFLAGS) systags
	cd ../qrymod; $(MAKE) $(MFLAGS) systags
	cd ../support; $(MAKE) $(MFLAGS) systags
	sort -o ../tags ../tags
	cd ../h; $(MAKE) $(MFLAGS) installtags
	cd ../gutil; $(MAKE) $(MFLAGS) installtags
	cd ../iutil; $(MAKE) $(MFLAGS) installtags
	cd ../ctlmod; $(MAKE) $(MFLAGS) installtags
	cd ../dbu; $(MAKE) $(MFLAGS) installtags
	cd ../decomp; $(MAKE) $(MFLAGS) installtags
	cd ../equel; $(MAKE) $(MFLAGS) installtags
	cd ../libq; $(MAKE) $(MFLAGS) installtags
	cd ../monitor; $(MAKE) $(MFLAGS) installtags
	cd ../ovqp; $(MAKE) $(MFLAGS) installtags
	cd ../parser; $(MAKE) $(MFLAGS) installtags
	cd ../qrymod; $(MAKE) $(MFLAGS) installtags
	cd ../support; $(MAKE) $(MFLAGS) installtags


sources: $(SRCS)

$(SRCS):
	$(GET) $(REL) $@

depend:
	grep '^#[ 	]*include' $(SRCS) | sed -n -f ../h/depend.sed > makedep
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it

monitor.o:$H/func.h
parser.o:$H/ingres.h
parser.o:$H/aux.h
parser.o:$H/access.h
parser.o:$H/func.h
qrymod.o:$H/func.h
decomp.o:$H/func.h
alldbu.o:$H/func.h
vaxingres.o:$H/ingres.h
vaxingres.o:$H/aux.h
vaxingres.o:$H/access.h
vaxingres.o:$H/func.h
sysdump.o:$H/func.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
