#
#  Makefile for INGRES database utilities
#
#	@(#)Makefile	8.4 	12/18/85
#

INGRES=	../..
H=	../h
LIB=	$(INGRES)/lib
BIN=	$(INGRES)/bin

CTLMOD=	$(LIB)/ctlmod
IUTIL=	$(LIB)/iutil
GUTIL=	$(LIB)/gutil
DBULIB=	$(LIB)/dbulib

LIBS=	$(CTLMOD) $(IUTIL) $(GUTIL)
LIBS1=	$(CTLMOD) $(DBULIB) $(IUTIL) $(GUTIL)
OBJS=	index.o create.o modify.o modupdate.o destroy.o \
	udestroy.o copy.o purgetup.o print.o rupdate.o \
	help.o save.o update.o convert.o secupdate.o \
	rdwrbatch.o resetrel.o seq_atts.o sysfunc.o \
	get_p_tid.o display.o pr_prot.o pr_tree.o rmqm.o ksort.o \
	btreeupdate.o
HDRS=
SRCS=	index.c create.c modify.c modupdate.c destroy.c udestroy.c \
	purgetup.c print.c rupdate.c help.c save.c update.c convert.c \
	secupdate.c rdwrbatch.c resetrel.c seq_atts.c \
	get_p_tid.c copy.c ksort.c \
	display.c rmqm.c pr_prot.c pr_tree.c  sysfunc.c btreeupdate.c

SOBJS=  sysmon.o

SSRCS=  sysmon.c

AR=	-ar
ARFLAGS=rvu
RANLIB=	ranlib
CFLAGS=	-I$H -O -I../ctlmod
LDFLAGS=
CP=	cp
CHMOD=	chmod
LIBMODE=664
BINMODE=4740
ROMODE=	444
GET=	sccs get
REL=
PR=	print
VGRIND=	csh /usr/ucb/vgrind
HEADER=	Database Utilities

dbulib: $(OBJS)
	$(AR) $(ARFLAGS) dbulib $?
	$(CHMOD) $(LIBMODE) dbulib
	$(RANLIB) dbulib

$(LIB)/dbulib: dbulib
	$(CP) dbulib $(LIB)/dbulib
	$(RANLIB) $(LIB)/dbulib

install: $(LIB)/dbulib $(LIB)/sysmonlib

sysmonlib: $(SOBJS)
	$(AR) $(ARFLAGS) sysmonlib $?
	$(CHMOD) $(LIBMODE) sysmonlib
	$(RANLIB) sysmonlib

$(LIB)/sysmonlib: sysmonlib
	$(CP) sysmonlib $(LIB)/sysmonlib
	$(RANLIB) $(LIB)/sysmonlib

sysmonitor: $(LIB)/sysmonlib
	
alldbu: $(OBJS) config.o
	$(CC) config.o $(OBJS) $(LDFLAGS) $(LIBS) -o alldbu
	$(CHMOD) $(BINMODE) alldbu

new:
	rm -f dbulib alldbu

clean:
	rm -f *.o
	rm -f dbulib alldbu a.out core sysmonlib

sources: $(SRCS)

$(SRCS):
	$(GET) $(REL) $@

print: sources
	$(PR) Makefile TraceFlags *.[hm] *.[csy]

vgrind: sources
	cp /dev/null index
	$(VGRIND) -h "$(HEADER) (Release 8)" -n Makefile TraceFlags
	$(VGRIND) -h "$(HEADER) (Release 8)" *.[csy]
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
	sed "s/	/	dbu\//" tags > systags
	cat systags >> ../tags

#
# We assume that systags has already been done.
#
installtags:
	sed "/	dbu/d" ../tags | sed "s/	/	..\//" > othertags
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

index.o:$H/pv.h
index.o:$H/func.h
index.o:$H/symbol.h
index.o:$H/ingres.h
index.o:$H/aux.h
index.o:$H/catalog.h
index.o:$H/access.h
index.o:$H/lock.h
index.o:$H/errors.h
create.o:$H/pv.h
create.o:$H/ingres.h
create.o:$H/access.h
create.o:$H/aux.h
create.o:$H/catalog.h
create.o:$H/symbol.h
create.o:$H/lock.h
create.o:$H/func.h
create.o:$H/errors.h
modify.o:$H/ingres.h
modify.o:$H/pv.h
modify.o:$H/aux.h
modify.o:$H/access.h
modify.o:$H/batch.h
modify.o:$H/lock.h
modify.o:$H/opsys.h
modify.o:$H/func.h
modify.o:$H/version.h
modify.o:$H/symbol.h
modify.o:$H/catalog.h
modify.o:$H/btree.h
modify.o:$H/errors.h
modupdate.o:$H/pv.h
modupdate.o:$H/ingres.h
modupdate.o:$H/aux.h
modupdate.o:$H/catalog.h
modupdate.o:$H/access.h
modupdate.o:$H/batch.h
modupdate.o:$H/opsys.h
modupdate.o:$H/btree.h
modupdate.o:$H/symbol.h
destroy.o:$H/pv.h
destroy.o:$H/ingres.h
destroy.o:$H/aux.h
destroy.o:$H/catalog.h
destroy.o:$H/func.h
destroy.o:$H/errors.h
udestroy.o:$H/ingres.h
udestroy.o:$H/aux.h
udestroy.o:$H/catalog.h
udestroy.o:$H/btree.h
udestroy.o:$H/pv.h
purgetup.o:$H/ingres.h
purgetup.o:$H/access.h
print.o:$H/pv.h
print.o:$H/ingres.h
print.o:$H/access.h
print.o:$H/aux.h
print.o:$H/lock.h
print.o:$H/func.h
print.o:$H/catalog.h
print.o:$H/btree.h
print.o:$H/errors.h
rupdate.o:$H/pv.h
rupdate.o:$H/ingres.h
rupdate.o:$H/func.h
help.o:$H/pv.h
help.o:$H/ingres.h
help.o:$H/aux.h
help.o:$H/catalog.h
help.o:$H/access.h
help.o:$H/func.h
help.o:$H/errors.h
save.o:$H/pv.h
save.o:$H/ingres.h
save.o:$H/aux.h
save.o:$H/func.h
save.o:$H/opsys.h
save.o:$H/errors.h
update.o:$H/ingres.h
update.o:$H/resp.h
update.o:$H/aux.h
update.o:$H/symbol.h
update.o:$H/access.h
update.o:$H/batch.h
update.o:$H/opsys.h
update.o:$H/catalog.h
update.o:$H/btree.h
update.o:$H/version.h
update.o:$H/errors.h
convert.o:$H/ingres.h
convert.o:$H/symbol.h
secupdate.o:$H/ingres.h
secupdate.o:$H/aux.h
secupdate.o:$H/catalog.h
secupdate.o:$H/symbol.h
secupdate.o:$H/access.h
secupdate.o:$H/batch.h
rdwrbatch.o:$H/ingres.h
rdwrbatch.o:$H/aux.h
rdwrbatch.o:$H/symbol.h
rdwrbatch.o:$H/access.h
rdwrbatch.o:$H/batch.h
resetrel.o:$H/pv.h
resetrel.o:$H/ingres.h
resetrel.o:$H/aux.h
resetrel.o:$H/batch.h
resetrel.o:$H/access.h
resetrel.o:$H/func.h
seq_atts.o:$H/ingres.h
seq_atts.o:$H/access.h
get_p_tid.o:$H/ingres.h
get_p_tid.o:$H/catalog.h
copy.o:$H/func.h
copy.o:$H/pv.h
copy.o:$H/ingres.h
copy.o:$H/aux.h
copy.o:$H/access.h
copy.o:$H/symbol.h
copy.o:$H/lock.h
copy.o:$H/errors.h
modify.o:$H/ingres.h
modify.o:$H/pv.h
modify.o:$H/aux.h
modify.o:$H/access.h
modify.o:$H/batch.h
modify.o:$H/lock.h
modify.o:$H/opsys.h
modify.o:$H/func.h
modify.o:$H/version.h
modify.o:$H/symbol.h
modify.o:$H/catalog.h
modify.o:$H/btree.h
modify.o:$H/errors.h
ksort.o:$H/ingres.h
ksort.o:$H/aux.h
ksort.o:$H/symbol.h
ksort.o:$H/access.h
ksort.o:$H/func.h
ksort.o:$H/batch.h
ksort.o:$H/catalog.h
ksort.o:$H/pv.h
display.o:$H/ingres.h
display.o:$H/aux.h
display.o:$H/catalog.h
display.o:$H/tree.h
display.o:$H/symbol.h
display.o:$H/access.h
display.o:$H/func.h
display.o:$H/pv.h
display.o:$H/errors.h
rmqm.o:$H/pv.h
rmqm.o:$H/ingres.h
rmqm.o:$H/access.h
rmqm.o:$H/aux.h
rmqm.o:$H/catalog.h
rmqm.o:$H/symbol.h
rmqm.o:$H/func.h
rmqm.o:$H/errors.h
pr_prot.o:$H/ingres.h
pr_prot.o:$H/tree.h
pr_prot.o:$H/aux.h
pr_prot.o:$H/catalog.h
pr_prot.o:$H/symbol.h
pr_tree.o:$H/symbol.h
pr_tree.o:$H/ingres.h
pr_tree.o:$H/tree.h
pr_tree.o:$H/aux.h
sysfunc.o:$H/ingres.h
sysfunc.o:$H/aux.h
sysfunc.o:$H/pv.h
sysfunc.o:$H/symbol.h
sysfunc.o:$H/func.h
btreeupdate.o:$H/ingres.h
btreeupdate.o:$H/aux.h
btreeupdate.o:$H/catalog.h
btreeupdate.o:$H/symbol.h
btreeupdate.o:$H/access.h
btreeupdate.o:$H/batch.h
btreeupdate.o:$H/btree.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
