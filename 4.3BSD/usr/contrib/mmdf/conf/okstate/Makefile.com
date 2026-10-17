#
#	Common Sub-Makefile for the MMDF System
#
HOST		= okstate
SYSTEM		= v7
MMPREF		=
LIBDIR		= /usr/mmdfii
CHANDIR		= /usr/mmdfii/chans
TBLDIR		= /usr/mmdfii/table
BINDIR		= /usr/local
RCVDIR		= /usr/mmdfii/rcvmail

#
#  Defines used during installation
#
CHOWN		= chown
ROOTLOGIN	= root
MMDFLOGIN	= mmdf
PGMPROT 	= 711

#
#  Configuration Defines
#
#  The paths below are relative from the directories below this one.
#  Define DOASSIGN to CONFIGDEFS if you want the d_assign code
#  Define SECURETTY if you run with terminals write protected (mode 711)
#  Define DEBUG to include extensive calls to the logging package
#  Define D_LOG if you want phone channel logging
#  Define D_DBGLOG if you want phone channel debugging output
#  Define RUNALONE for standalone testing (seldom used)
#  Define V4_2BSD if you are compiling for 4.2BSD
#  Define NODIAL if you do not want the dial package (no phone channel)
#  Define SYS5 if you have a SystemV Unix or something similar (Sys3)
#  Define ALTOS if you have an Altos, does some depraved things.
#  Define NODUP2 if you don't a dup2() syscall or subroutine.
#  Define NOFCNTL and NODUP2 if you don't have fcntl(x, F_DUPFD) or dup2()
#  Define -Drindex=strrchr -Dindex=strchr if you are on a SYS5 system
#  Define NAMESERVER if you want the nameserver lookup code
#  Define UUCPLOCK if you want UUCP/tip/cu-style locking for dial-out lines
#  Define NODOMLIT if you don't want literals like [10.0.0.52] in messages
#
# Lots of special crocks here for OKSTATE problems (this is not a good sample)
CONFIGDEFS	= -DDEBUG=1 -DD_LOG -DD_DBGLOG -DNODOMLIT
CFLAGS		= -m -O -I../../h $(CONFIGDEFS)
LDFLAGS 	= -k 8k
LDFLAGSB	= -k 16k
LDSHR		= -n
MMDFLIBS 	= ../../lib/dial/d_pglobals.o ../../lib/libmmdf.a
SYSLIBS		= -ldbm -lndir
LINT		= lint
LFLAGS		= -phbvxacL -I../../h $(CONFIGDEFS)
LLIBS		= ../../lib/llib-lmmdf.ln
AR		= ar


#  Specify either ch_tbdbm (for DBM based tables) or ch_tbseq for
#  sequential IO based tables.
CH_TB	= ch_tbdbm

#  Specify one of the nameserver modules or the fake module
#  if you do not intend to support nameservers (4.2, fake)
TB_NS	= fake

#  Specify tai_???.o and lk_lock???.o
LOCALUTIL = tai_file.o lk_lock.rand.o


default:	real-default

#
#  special case dependencies
#
../../h/mmdf.h:	../../h/conf.h
	-touch ../../h/mmdf.h
.PRECIOUS: ../../h/mmdf.h

#
#  #include dependencies
#
#  Two versions are supplied.  One for sites with cc -M (4.3BSD)
#  and one for those that don't have it.  Comment out the one
#  you do not want.

#  This one is for sites without cc -M
depend:
	cat </dev/null >x.c
	for i in $(MODULES); do \
		(echo $$i.o: $$i.c >>makedep; \
		/bin/grep '^#[ 	]*include' x.c $$i.c | sed \
			-e 's,c:[^"]*"\./\([^"]*\)".*,o: \1,' \
			-e 's,c:[^"]*"/\([^"]*\)".*,o: /\1,' \
			-e 's,c:[^"]*"\([^"]*\)".*,o: ../../h/\1,' \
			-e 's,c:[^<]*<\(.*\)>.*,o: /usr/include/\1,' \
			>>makedep); done
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile.real Makefile.bak
	ed - Makefile.real < eddep
	rm eddep makedep x.c
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile.real
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile.real
	echo '# see make depend above' >> Makefile.real

#  This one id for sites with cc -M
#depend:
#	( for i in ${MODULES} ; do \
#		${CC} -M ${CFLAGS} $$i.c ; done ) | \
#	awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
#		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
#		       else rec = rec " " $$2 } } \
#	      END { print rec } ' > makedep
#	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
#	echo '$$r makedep' >>eddep
#	echo 'w' >>eddep
#	cp Makefile.real Makefile.bak
#	ed - Makefile.real < eddep
#	rm eddep makedep
#	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile.real
#	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile.real
#	echo '# see make depend above' >> Makefile.real
