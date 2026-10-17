#
#  Makefile for INGRES One Variable Query Processor (OVQP)
#
#	@(#)Makefile	8.3	12/18/85
#

INGRES=	../..
H=	../h
LIB=	$(INGRES)/lib

CFLAGS=	-O -I$H
LINTFLAGS= -huc -I$H
AR=	-ar
ARFLAGS=rvu
RANLIB=	ranlib
LDFLAGS=
CP=	cp
CHMOD=	chmod
LIBMODE=664
BINMODE=4740
GET=	sccs get
REL=
LINT=	lint
PR=	print
VGRIND=	csh /usr/ucb/vgrind
HEADER=	One Variable Query Processor

CTLMOD=	$(LIB)/ctlmod
IUTIL=	$(LIB)/iutil
GUTIL=	$(LIB)/gutil

OBJS=	scan.o interp.o startovqp.o\
	strategy.o findsimps.o key.o string.o typecheck.o interp1.o \
	equelpr.o prsym.o tidtest.o
SRCS1=	scan.c interp.c startovqp.c\
	strategy.c findsimps.c key.c string.c typecheck.c interp1.c \
	equelpr.c prsym.c tidtest.c
HDRS=	ovqp.h strategy.h
SRCS=	$(SRCS1) 

LIBS=	$(CTLMOD) $(IUTIL) $(GUTIL)

ovqplib: $(OBJS)
	$(AR) rvu ovqplib $?
	$(CHMOD) $(LIBMODE) ovqplib
	$(RANLIB) ovqplib

install: $(LIB)/ovqp

$(LIB)/ovqp: ovqplib
	$(CP) ovqplib $(LIB)/ovqp
	$(RANLIB) $(LIB)/ovqp

sources: $(SRCS) $(HDRS)

$(SRCS):
	$(GET) $(REL) $@

new:
	rm -f ovqplib

clean:
	rm -f *.o
	rm -f ovqplib a.out core

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
	sed "s/	/	ovqp\//" tags > systags
	cat systags >> ../tags

#
# We assume that systags has already been done.
#
installtags:
	sed "/	ovqp/d" ../tags | sed "s/	/	..\//" > othertags
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

scan.o:$H/ingres.h
scan.o:$H/symbol.h
scan.o:$H/tree.h
scan.o:../decomp/globs.h
interp.o:$H/ingres.h
interp.o:$H/aux.h
interp.o:$H/symbol.h
interp.o:$H/tree.h
interp.o:$H/access.h
interp.o:../decomp/globs.h
interp.o:$H/errors.h
startovqp.o:$H/ingres.h
startovqp.o:$H/symbol.h
startovqp.o:$H/tree.h
startovqp.o:$H/aux.h
startovqp.o:../decomp/globs.h
startovqp.o:../ctlmod/pipes.h
startovqp.o:$H/errors.h
strategy.o:$H/ingres.h
strategy.o:$H/aux.h
strategy.o:$H/catalog.h
strategy.o:$H/symbol.h
strategy.o:$H/tree.h
strategy.o:../decomp/globs.h
strategy.o:strategy.h
strategy.o:$H/btree.h
strategy.o:$H/errors.h
findsimps.o:$H/ingres.h
findsimps.o:$H/symbol.h
findsimps.o:$H/tree.h
findsimps.o:../decomp/globs.h
findsimps.o:strategy.h
key.o:$H/ingres.h
key.o:$H/aux.h
key.o:$H/symbol.h
key.o:$H/tree.h
key.o:$H/catalog.h
key.o:../decomp/globs.h
key.o:strategy.h
key.o:$H/btree.h
string.o:$H/ingres.h
string.o:$H/aux.h
string.o:$H/symbol.h
string.o:$H/tree.h
string.o:../decomp/globs.h
string.o:$H/errors.h
typecheck.o:$H/ingres.h
typecheck.o:$H/aux.h
typecheck.o:$H/tree.h
typecheck.o:$H/symbol.h
typecheck.o:../decomp/globs.h
typecheck.o:$H/errors.h
interp1.o:$H/ingres.h
interp1.o:$H/aux.h
interp1.o:$H/symbol.h
interp1.o:$H/tree.h
interp1.o:../decomp/globs.h
interp1.o:$H/errors.h
equelpr.o:$H/ingres.h
equelpr.o:$H/aux.h
equelpr.o:$H/symbol.h
equelpr.o:$H/tree.h
equelpr.o:$H/batch.h
equelpr.o:../ctlmod/pipes.h
equelpr.o:../decomp/globs.h
prsym.o:$H/ingres.h
prsym.o:$H/aux.h
prsym.o:$H/symbol.h
prsym.o:$H/tree.h
prsym.o:../decomp/globs.h
tidtest.o:$H/ingres.h
tidtest.o:$H/symbol.h
tidtest.o:$H/tree.h
tidtest.o:../decomp/globs.h
tidtest.o:strategy.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
