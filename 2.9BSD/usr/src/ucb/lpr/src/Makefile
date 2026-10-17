# makefile for line printer spooling system
#
# Ownerships
# ROOT		a user that can access any and all files
# DAEMON	someone special + the group id of the spooling programs
#
# Options:
#	add -DSLEEPER to CFLAGS if you want the printer daemon to sleep
#	awaiting work;  otherwise a new daemon is started with each job.
#
CFLAGS=		-O
LDFLAGS=	-s
LIBDIR=		/usr/lib
BINDIR=		/usr/bin
ETCDIR=		/etc
SPOOLDIR=	/usr/spool/lpd
LPDEV=		/dev/lp
ERRF=		/usr/adm/lp.err
ACCF=		/usr/adm/lpacct

ROOT=		root
DAEMON=		daemon
SYSGRP=		sys

all:		lpd lpr lpq lprm lpdrestart ulf

lpd:		lpd.o lpdchar.o printcap.o
		${CC} -o lpd ${LDFLAGS} lpd.o lpdchar.o printcap.o

lpr:		lpr.o printcap.o
		${CC} -o lpr ${LDFLAGS} lpr.o printcap.o

lpq:		lpq.o printcap.o
		${CC} -o lpq ${LDFLAGS} lpq.o printcap.o

lprm:		lprm.o printcap.o
		${CC} -o lprm ${LDFLAGS} lprm.o printcap.o

lpdrestart:	lpdrestart.o
		${CC} -o lpdrestart ${LDFLAGS} lpdrestart.o

ulf:		ulf.o printcap.o
		${CC} -o ulf ${CFLAGS} ${LDFLAGS} ulf.o printcap.o

ulf.o:		lp.local.h
lpr.o:		lp.local.h
lpd.o:		lp.local.h
lpq.o:		lp.local.h
lprm.o:		lp.local.h
lpdrestart.o:	lp.local.h

cp:		all
		-cp lpd ${LIBDIR}/lpd
		-cp lpr ${BINDIR}/lpr
		-cp ulf ${LIBDIR}/ulf
		-cp lpq  ${BINDIR}/lpq
		-cp lprm  ${BINDIR}/lprm
		-cp lpdrestart ${ETCDIR}
		cat /dev/null >> ${ERRF}
		cat /dev/null >> ${ACCF}
		-rm -f ${BINDIR}/opr ${BINDIR}/print ${LIBDIR}/lp ${LIBDIR}/slp
		ln ${BINDIR}/lpr ${BINDIR}/print
		ln ${BINDIR}/lpr ${BINDIR}/opr
		ln ${LIBDIR}/ulf ${LIBDIR}/lp
		chown ${ROOT} ${BINDIR}/lpr ${BINDIR}/lpq \
			${ETCDIR}/lpdrestart
		chown ${DAEMON} ${BINDIR}/lprm ${LIBDIR}/lpd ${SPOOLDIR} \
			${LPDEV} ${ERRF} ${ACCF}
		chgrp ${DAEMON} ${LIBDIR}/lpd ${BINDIR}/lpr ${BINDIR}/lprm \
			${BINDIR}/lpq ${SPOOLDIR} ${ERRF} ${ACCF}
		chgrp ${SYSGRP} ${ETCDIR}/lpdrestart
		chmod 6711 ${LIBDIR}/lpd ${BINDIR}/lpr ${BINDIR}/lprm
		chmod 4710 ${ETCDIR}/lpdrestart
		chmod 2711 ${BINDIR}/lpq
		chmod 775 ${SPOOLDIR}
		chmod 700 ${LPDEV}
		chmod 664 ${ERRF} ${ACCF}
		chmod 644 /etc/printcap

clean:
		rm -f lprm lpq lpd lpdrestart lpr ulf *.o
