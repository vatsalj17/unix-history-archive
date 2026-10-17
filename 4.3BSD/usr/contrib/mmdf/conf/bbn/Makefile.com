#
#	Common Sub-Makefile for the MMDF System
#
HOST		= bbn
SYSTEM		= 4.2
MMPREF		=
LIBDIR		= /usr/mmdf/lib
CHANDIR		= /usr/mmdf/chans
TBLDIR		= /usr/mmdf/table
BINDIR		= /usr/local/mmdf
RCVDIR		= /usr/local/mmdf

#
#  Defines used during installation
#
CHOWN		= /etc/chown
MMDFLOGIN	= mmdf
ROOTLOGIN	= root
PGMPROT 	= 775

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
#
CONFIGDEFS	= -DDEBUG=1 -DV4_2BSD -DNAMESERVER -DD_LOG -DD_DBGLOG -DUUCPLOCK
CFLAGS		= -g -I../../h $(CONFIGDEFS)
LDFLAGS 	= 
MMDFLIBS 	= ../../lib/libmmdf.a
SYSLIBS		= -ldbm
LINT		= lint
LFLAGS		= -phbvxacL -I../../h $(CONFIGDEFS)
LLIBS		= ../../lib/llib-lmmdf.ln
AR		= ar


#  Specify either ch_tbdbm (for DBM based tables) or ch_tbseq for
#  sequential IO based tables.
CH_TB	= ch_tbdbm

#  Specify one of the nameserver modules or the fake module
#  if you do not intend to support nameservers (4.2, fake)
TB_NS	= 4.2

#  Specify tai_???.o and lk_lock???.o
LOCALUTIL = tai_file.o lk_lock.4.2.o


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
