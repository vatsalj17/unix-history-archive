#
#  Makefile for INGRES support routines
#
#	@(#)Makefile	8.10	2/10/86
#

INGRES=	../..
H=	../h
LIB=	$(INGRES)/lib
BIN=	$(INGRES)/bin
SYSBIN=	/usr/local

ALL=	creatdb destroydb helpr printr purge \
	restore usersetup ingres printadmin \
	ingreslock copydb ingconv sysmod
SRCS=	creatdb.c destroydb.c error.c helpr.c ingres.y \
	printr.c purge.c subs.c usersetup.c \
	indconv.q restore.c copydb.q prvect.c printadmin.c \
	demodb.sh initsocket.c ildr.c ingconv.q 
HDRS=	ildr.h

CTLMOD=	$(LIB)/ctlmod
IUTIL=	$(LIB)/iutil
GUTIL=	$(LIB)/gutil
DBULIB=	$(LIB)/dbulib
LIBS1=	$(GUTIL)
LIBS2=	$(IUTIL) $(GUTIL)
LIBS4=	$(DBULIB) $(IUTIL) $(GUTIL) $(CTLMOD)
LIBS4=  externs.o $(DBULIB) $(IUTIL) $(CTLMOD) $(GUTIL) $(IUTIL)

CFLAGS=	-O -I$H -I../ctlmod
EQUEL=	$(BIN)/equel
LDFLAGS=
CP=	cp
CHMOD=	chmod
BINMODE=751
SUMODE=	4751
ROMODE=	444
SHMODE=	755
CHOWN=	-chown
GET=	sccs get
REL=
PR=	print
VGRIND=	csh /usr/ucb/vgrind
HEADER=	Support Programs

.SUFFIXES: .q

.q.c:
	$(EQUEL) $<

.q.o:
	$(EQUEL) $<
	$(CC) $(CFLAGS) -c $*.c

all: $(ALL) demodb.sh

copydb:	copydb.o $(LIBS1)
	$(CC) $(LDFLAGS) copydb.o $(LIBS1) $(LIB)/libq.a -o copydb
	$(CHMOD) $(BINMODE) copydb
	ls -l copydb; size copydb

ingconv:  ingconv.o $(LIBS1)
	$(CC) $(LDFLAGS) ingconv.o $(LIBS1) $(LIB)/libq.a -o ingconv
	$(CHMOD) $(SUMODE) ingconv
	ls -l ingconv; size ingconv

creatdb: creatdb.o error.o prvect.o $(LIBS4)
	$(CC) $(LDFLAGS) creatdb.o error.o prvect.o $(LIBS4) -lm -o creatdb
	$(CHMOD) $(SUMODE) creatdb
	ls -l creatdb; size creatdb

destroydb: destroydb.o $(LIBS4)
	$(CC) $(LDFLAGS) destroydb.o $(LIBS4) -lm -o destroydb
	$(CHMOD) $(SUMODE) destroydb
	ls -l destroydb; size destroydb

helpr:	helpr.o externs.o error.o prvect.o $(LIBS4) 
	$(CC) $(LDFLAGS) helpr.o externs.o error.o prvect.o $(LIBS4) -lm -o helpr
	$(CHMOD) $(SUMODE) helpr
	ls -l helpr; size helpr

indconv: indconv.o $(LIBS2)
	$(CC) indconv.o $(LIB)/equel $(LIBS2) -o indconv
	$(CHMOD) $(BINMODE) indconv
	ls -l indconv; size indconv

ingres: ingres.o $(LIBS2)
	$(CC) ingres.o $(LDFLAGS) $(LIBS2) -o ingres
	$(CHMOD) $(SUMODE) ingres
	ls -l ingres; size ingres
	rm -f sysmod
	$(CP) ingres sysmod
	$(CHMOD) $(SUMODE) sysmod

sysmod:	ingres

ingres.o: ingres.c
	$(CC) $(CFLAGS) -I../ctlmod -c ingres.c

ingreslock: ildr.o initsocket.o
	$(CC) $(CFLAGS) -o ingreslock ildr.o initsocket.o 
	$(CHMOD) $(BINMODE) ingreslock
	ls -l ingreslock ; size ingreslock

printadmin: printadmin.o $(LIBS2)
	$(CC) $(LDFLAGS) printadmin.o $(LIBS2) -o printadmin
	$(CHMOD) $(SUMODE) printadmin
	ls -l printadmin; size printadmin

printr: printr.o error.o prvect.o $(LIBS4)
	$(CC) $(LDFLAGS) printr.o error.o prvect.o $(LIBS4) -lm -o printr
	$(CHMOD) $(SUMODE) printr
	ls -l printr; size printr

purge: purge.o externs.o subs.o error.o $(LIBS4)
	$(CC) $(LDFLAGS) externs.o purge.o subs.o error.o $(LIBS4) -lm  -o purge
	$(CHMOD) $(SUMODE) purge
	ls -l purge; size purge

restore: restore.o externs.o subs.o error.o $(LIBS4)
	$(CC) $(LDFLAGS) restore.o externs.o subs.o error.o $(LIBS4) -lm -o restore
	$(CHMOD) $(SUMODE) restore
	ls -l restore; size restore

usersetup: usersetup.o $(LIBS1)
	$(CC) $(LDFLAGS) usersetup.o $(LIBS1) -o usersetup
	$(CHMOD) 700 usersetup

new:
	rm -f $(ALL)

clean:
	rm -f *.o
	rm -f $(ALL) ingconv.c copydb.c copydb5.c indconv.c ingres.c a.out core y.tab.c

install: $(BIN)/creatdb $(BIN)/destroydb $(BIN)/helpr
install: $(BIN)/printr $(BIN)/purge $(BIN)/restore 
install: $(BIN)/usersetup $(BIN)/ingres $(BIN)/printadmin
install: $(BIN)/demodb $(BIN)/ingreslock $(BIN)/copydb
install: $(BIN)/ingconv

$(BIN)/creatdb: creatdb
	$(CP) creatdb $(BIN)/creatdb; $(CHMOD) $(SUMODE) $(BIN)/creatdb
$(BIN)/destroydb: destroydb
	$(CP) destroydb $(BIN)/destroydb; $(CHMOD) $(SUMODE) $(BIN)/destroydb
$(BIN)/helpr: helpr
	$(CP) helpr $(BIN)/helpr; $(CHMOD) $(SUMODE) $(BIN)/helpr
$(BIN)/indconv: indconv
	$(CP) indconv $(BIN)/indconv; $(CHMOD) $(BINMODE) $(BIN)/indconv
$(BIN)/printr: printr
	$(CP) printr $(BIN)/printr; $(CHMOD) $(SUMODE) $(BIN)/printr
$(BIN)/purge: purge
	$(CP) purge $(BIN)/purge; $(CHMOD) $(SUMODE) $(BIN)/purge
$(BIN)/restore: restore
	$(CP) restore $(BIN)/restore; $(CHMOD) $(SUMODE) $(BIN)/restore
$(BIN)/usersetup: usersetup
	$(CP) usersetup $(BIN)/usersetup; $(CHMOD) 700 $(BIN)/usersetup
$(BIN)/ingres: ingres
	$(CP) ingres $(BIN)/ingres; $(CHMOD) $(SUMODE) $(BIN)/ingres
	$(CP) $(BIN)/ingres $(BIN)/sysmod ; $(CHMOD) $(SUMODE) $(BIN)/sysmod
$(BIN)/printadmin: printadmin
	$(CP) printadmin $(BIN)/printadmin; $(CHMOD) $(SUMODE) $(BIN)/printadmin
$(BIN)/demodb: demodb.sh
	$(CP) demodb.sh $(BIN)/demodb; $(CHMOD) $(SHMODE) $(BIN)/demodb

$(BIN)/ingreslock: ingreslock
	$(CP) ingreslock $(BIN)/ingreslock ; $(CHMOD) $(BINMODE) $(BIN)/ingreslock
$(BIN)/copydb: copydb
	$(CP) copydb $(BIN)/copydb ; $(CHMOD) $(BINMODE) $(BIN)/copydb

$(BIN)/ingconv: ingconv
	$(CP) ingconv $(BIN)/ingconv ; $(CHMOD) $(SUMODE) $(BIN)/ingconv

sysinstall:
sysinstall: $(SYSBIN)/ingres $(SYSBIN)/creatdb $(SYSBIN)/destroydb $(SYSBIN)/demodb
sysinstall: $(SYSBIN)/purge $(SYSBIN)/restore $(SYSBIN)/copydb
sysinstall: $(SYSBIN)/printr $(SYSBIN)/helpr $(SYSBIN)/ingconv

$(SYSBIN)/ingres: ingres
	$(CP) ingres $(SYSBIN)/ingres
	$(CHOWN) ingres $(SYSBIN)/ingres; $(CHMOD) $(SUMODE) $(SYSBIN)/ingres
	$(CP) sysmod $(SYSBIN)/sysmod
	$(CHOWN) ingres $(SYSBIN)/sysmod; $(CHMOD) $(SUMODE) $(SYSBIN)/sysmod
$(SYSBIN)/creatdb: creatdb
	$(CP) creatdb $(SYSBIN)/creatdb
	$(CHOWN) ingres $(SYSBIN)/creatdb; $(CHMOD) $(SUMODE) $(SYSBIN)/creatdb
$(SYSBIN)/destroydb: destroydb
	$(CP) destroydb $(SYSBIN)/destroydb
	$(CHOWN) ingres $(SYSBIN)/destroydb; $(CHMOD) $(SUMODE) $(SYSBIN)/destroydb
$(SYSBIN)/demodb: demodb.sh
	$(CP) demodb.sh $(SYSBIN)/demodb
	$(CHOWN) ingres $(SYSBIN)/demodb; $(CHMOD) $(SHMODE) $(SYSBIN)/demodb
$(SYSBIN)/purge: purge
	$(CP) purge $(SYSBIN)/purge
	$(CHOWN) ingres $(SYSBIN)/purge; $(CHMOD) $(SUMODE) $(SYSBIN)/purge
$(SYSBIN)/restore: restore
	$(CP) restore $(SYSBIN)/restore
	$(CHOWN) ingres $(SYSBIN)/restore; $(CHMOD) $(SUMODE) $(SYSBIN)/restore
$(SYSBIN)/printr: printr
	$(CP) printr $(SYSBIN)/printr
	$(CHOWN) ingres $(SYSBIN)/printr; $(CHMOD) $(SUMODE) $(SYSBIN)/printr
$(SYSBIN)/helpr: helpr
	$(CP) helpr $(SYSBIN)/helpr
	$(CHOWN) ingres $(SYSBIN)/helpr; $(CHMOD) $(SUMODE) $(SYSBIN)/helpr
$(SYSBIN)/copydb: copydb
	$(CP) copydb $(SYSBIN)/copydb ; $(CHMOD) $(BINMODE) $(SYSBIN)/copydb
$(SYSBIN)/ingconv: ingconv
	$(CP) ingconv $(SYSBIN)/ingconv ; $(CHMOD) $(SUMODE) $(SYSBIN)/ingconv

sources: $(SRCS)

$(SRCS):
	$(GET) $(REL) $@

print: sources
	$(PR) Makefile TraceFlags *.[hm] *.[csy]

vgrind: sources
	cp /dev/null index
	$(VGRIND) -h "$(HEADER) (Release 8)" -n Makefile 
	$(VGRIND) -h "$(HEADER) (Release 8)" $(SRCS) $(HDRS)
	sed /SCCSID/d < index > tmp
	mv tmp index
	$(VGRIND) -h "$(HEADER) (Release 8)" -x index

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
	sed "s/	/	support\//" tags > systags
	cat systags >> ../tags

#
# We assume that systags has already been done.
#
installtags:
	sed "/	support/d" ../tags | sed "s/	/	..\//" > othertags
	sort -o tags tags othertags
	-rm -f othertags systags
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

creatdb.o:$H/ingres.h
creatdb.o:$H/aux.h
creatdb.o:$H/version.h
creatdb.o:$H/access.h
creatdb.o:$H/symbol.h
creatdb.o:$H/opsys.h
creatdb.o:$H/pv.h
creatdb.o:$H/sccs.h
destroydb.o:$H/ingres.h
destroydb.o:$H/aux.h
destroydb.o:$H/access.h
destroydb.o:$H/sccs.h
error.o:$H/ingres.h
error.o:$H/aux.h
error.o:$H/sccs.h
helpr.o:$H/ingres.h
helpr.o:$H/aux.h
helpr.o:$H/access.h
helpr.o:$H/lock.h
helpr.o:$H/pv.h
helpr.o:$H/sccs.h
ingres.o:$H/ingres.h
ingres.o:$H/aux.h
ingres.o:$H/version.h
ingres.o:$H/access.h
ingres.o:$H/lock.h
ingres.o:$H/opsys.h
ingres.o:$H/sccs.h
printr.o:$H/ingres.h
printr.o:$H/aux.h
printr.o:$H/access.h
printr.o:$H/lock.h
printr.o:$H/pv.h
printr.o:$H/sccs.h
purge.o:$H/ingres.h
purge.o:$H/aux.h
purge.o:$H/lock.h
purge.o:$H/pv.h
purge.o:$H/sccs.h
purge.o:$H/opsys.h
subs.o:$H/ingres.h
subs.o:$H/aux.h
subs.o:$H/access.h
subs.o:$H/lock.h
subs.o:$H/sccs.h
subs.o:$H/opsys.h
usersetup.o:$H/ingres.h
usersetup.o:$H/aux.h
usersetup.o:$H/opsys.h
usersetup.o:$H/sccs.h
indconv.o:$H/sccs.h
restore.o:$H/ingres.h
restore.o:$H/aux.h
restore.o:$H/catalog.h
restore.o:$H/access.h
restore.o:$H/batch.h
restore.o:$H/opsys.h
restore.o:$H/lock.h
restore.o:$H/symbol.h
restore.o:$H/resp.h
restore.o:$H/sccs.h
copydb.o:$H/useful.h
copydb.o:$H/sccs.h
prvect.o:$H/tree.h
prvect.o:$H/pv.h
prvect.o:$H/sccs.h
printadmin.o:$H/ingres.h
printadmin.o:$H/access.h
printadmin.o:$H/sccs.h
initsocket.o:$H/sccs.h
ildr.o:$H/ildr.h
ildr.o:$H/sccs.h
ingconv.o:$H/ingres.h
ingconv.o:$H/access.h
ingconv.o:$H/sccs.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
