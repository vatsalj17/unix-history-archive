#
#  Makefile for tty monitor
#
#	@(#)Makefile	8.3	4/22/86
#

INGRES=	../..
LIB=	$(INGRES)/lib
H=	../h

CTLMOD=	$(LIB)/ctlmod
IUTIL=	$(LIB)/iutil
GUTIL=	$(LIB)/gutil
LIBS=	$(CTLMOD) $(IUTIL) $(GUTIL)

OBJS=	ttymon.o monitor.o clear.o edit.o go.o trapquery.o \
	print.o quit.o shell.o eval.o writeout.o xwait.o \
	clrline.o prompt.o getch.o branch.o newdirec.o \
	include.o getfilenm.o expr.o mcall.o mac.o buf.o \
	syscntrl.o
SRCS1=	ttymon.c monitor.c clear.c edit.c go.c trapquery.c \
	print.c quit.c shell.c eval.c writeout.c xwait.c \
	clrline.c prompt.c getch.c branch.c newdirec.c \
	include.c getfilenm.c expr.c mcall.c mac.c buf.c \
	syscntrl.c 
HDRS=	buf.h monitor.h
SRCS=	$(SRCS1) $(HDRS)

CFLAGS=	-I$H -O -I../ctlmod
LINTFLAGS= -huc -I$H -I../ctlmod
LDFLAGS=
BIN=	$(INGRES)/bin
GET=	sccs get
REL=
LINT=	lint
AR=	-ar
ARFLAGS=rvu
RANLIB=	ranlib
PR=	print
VGRIND=	csh /usr/ucb/vgrind
HEADER=	Terminal Monitor
CP=	cp
CHMOD=	chmod
LIBMODE=664
BINMODE=751
ROMODE=	444

monlib: $(OBJS)
	$(AR) $(ARFLAGS) monlib $?
	$(CHMOD) $(LIBMODE) monlib
	$(RANLIB) monlib

install: $(LIB)/monitor

$(LIB)/monitor: monlib
	$(CP) monlib $(LIB)/monitor
	$(RANLIB) $(LIB)/monitor

new:
	rm -f monlib

clean:
	rm -f a.out monlib core
	rm -f *.o

sources: $(SRCS)

$(SRCS):
	$(GET) $(REL) $@

lint:
	$(LINT) $(LINTFLAGS) $(SRCS1)

print: sources
	$(PR) Makefile TraceFlags *.[hm] *.[csy]

vgrind: sources
	cp /dev/null index
	$(VGRIND) -h "$(HEADER) (Release 8)" -n Makefile TraceFlags
	$(VGRIND) -h "$(HEADER) (Release 8)" *.[hm] *.[csy]
	sed /SCCSID/d < index > tmp
	mv tmp index
	$(VGRIND) -h "$(HEADER) (Release 8)" -x index

config.c: ../conf/SCCS/s.monitor.c
	rm -f config.c
	$(GET) $(REL) -p ../conf/SCCS/s.monitor.c > config.c
	$(CHMOD) $(ROMODE) config.c

tags: sources
	-fgrep ../ tags > othertags
	ctags  $(SRCS) $(HDRS)
	sort -o tags tags othertags
	-rm -f othertags

#
# DANGER	DANGER	DANGER	DANGER	DANGER	DANGER
# The following two entries should NOT be executed by the user.
# These entries are only for the Makefile in conf to construct
# the system tag files.
#
systags: 
	ctags $(SRCS) $(HDRS)
	sed "s/	/	monitor\//" tags > systags
	cat systags >> ../tags

#
# We assume that systags has already been done.
#
installtags:
	sed "/	monitor/d" ../tags | sed "s/	/	..\//" > othertags
	sort -o tags tags othertags
	-rm -f othertags systags

depend:
	grep '^#[ 	]*include' $(SRCS1) | sed -n -f ../h/depend.sed >makedep
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

ttymon.o:monitor.h
ttymon.o:$H/ingres.h
ttymon.o:$H/version.h
ttymon.o:$H/opsys.h
ttymon.o:$H/pv.h
ttymon.o:$H/func.h
ttymon.o:$H/sccs.h
monitor.o:monitor.h
monitor.o:$H/ingres.h
monitor.o:$H/aux.h
monitor.o:$H/sccs.h
clear.o:monitor.h
clear.o:$H/ingres.h
clear.o:$H/aux.h
clear.o:$H/sccs.h
edit.o:monitor.h
edit.o:$H/ingres.h
edit.o:$H/aux.h
edit.o:$H/opsys.h
edit.o:$H/sccs.h
go.o:monitor.h
go.o:$H/ingres.h
go.o:$H/aux.h
go.o:$H/resp.h
go.o:$H/symbol.h
go.o:$H/pv.h
go.o:$H/sccs.h
trapquery.o:monitor.h
trapquery.o:$H/trace.h
trapquery.o:$H/resp.h
trapquery.o:$H/sccs.h
print.o:monitor.h
print.o:$H/ingres.h
print.o:$H/aux.h
print.o:$H/sccs.h
quit.o:monitor.h
quit.o:$H/ingres.h
quit.o:$H/aux.h
quit.o:$H/sccs.h
shell.o:monitor.h
shell.o:$H/ingres.h
shell.o:$H/aux.h
shell.o:$H/opsys.h
shell.o:$H/sccs.h
eval.o:monitor.h
eval.o:$H/ingres.h
eval.o:$H/aux.h
eval.o:$H/sccs.h
writeout.o:monitor.h
writeout.o:$H/ingres.h
writeout.o:$H/aux.h
writeout.o:$H/sccs.h
xwait.o:monitor.h
xwait.o:$H/ingres.h
xwait.o:$H/aux.h
xwait.o:$H/sccs.h
clrline.o:monitor.h
clrline.o:$H/ingres.h
clrline.o:$H/aux.h
clrline.o:$H/sccs.h
prompt.o:monitor.h
prompt.o:$H/ingres.h
prompt.o:$H/aux.h
prompt.o:$H/sccs.h
getch.o:monitor.h
getch.o:$H/ingres.h
getch.o:$H/aux.h
getch.o:$H/sccs.h
branch.o:monitor.h
branch.o:$H/ingres.h
branch.o:$H/aux.h
branch.o:$H/sccs.h
newdirec.o:monitor.h
newdirec.o:$H/ingres.h
newdirec.o:$H/aux.h
newdirec.o:$H/sccs.h
include.o:monitor.h
include.o:$H/ingres.h
include.o:$H/aux.h
include.o:$H/sccs.h
getfilenm.o:monitor.h
getfilenm.o:$H/ingres.h
getfilenm.o:$H/aux.h
getfilenm.o:$H/sccs.h
expr.o:$H/ingres.h
expr.o:$H/sccs.h
mcall.o:$H/ingres.h
mcall.o:$H/sccs.h
mac.o:$H/useful.h
mac.o:$H/sccs.h
mac.o:buf.h
buf.o:buf.h
buf.o:$H/sccs.h
syscntrl.o:monitor.h
syscntrl.o:$H/func.h
syscntrl.o:$H/sccs.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
