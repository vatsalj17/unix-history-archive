#
#  Makefile for INGRES EQUEL precompiler
#
#	@(#)Makefile	8.2	12/18/85
#

INGRES=	../..
H=	../h
LIB=	$(INGRES)/lib
BIN=	$(INGRES)/bin
SYSBIN=	/usr/local

OBJS= 	main.o yylex.o grammar.o cvar.o include.o name.o getkey.o \
	number.o operator.o prtout.o string.o comment.o retrieve.o display.o \
	getch.o cmap.o yyerror.o nalloc.o 
HDRS=	constants.h globals.h
SRCS= 	main.c yylex.c grammar.y cvar.c include.c name.c getkey.c \
	number.c operator.c prtout.c string.c comment.c retrieve.c display.c \
	getch.c cmap.c yyerror.c nalloc.c tokens.y

GUTIL=	$(LIB)/gutil
# include -lm on the VAX
LIBS=	$(GUTIL) -lm

CFLAGS=	-I$H -O 
LINTFLAGS= -huc -I$H
LDFLAGS=
YFLAGS=	-d
GET=	sccs get
REL=
PR=	pr
LINT=	lint
VGRIND=	csh /usr/ucb/vgrind
HEADER=	Equel Precompiler
CP=	cp
CHMOD=	chmod
CHOWN=	-chown
BINMODE=755

equel: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o equel
	$(CHMOD) $(BINMODE) equel
	ls -l equel; size equel

new:
	rm -f equel

clean:
	rm -f *.o
	rm -f a.out y.tab.c y.tab.h core equel grammar.c

install: $(BIN)/equel

$(BIN)/equel: equel
	$(CP) equel $(BIN)/equel

sysinstall: $(SYSBIN)/equel
	$(CP) equel $(SYSBIN)/equel
	$(CHOWN) ingres $(SYSBIN)/equel

lint:
	$(LINT) $(LINTFLAGS) $(SRCS)

print: sources
	$(PR) Makefile TraceFlags *.[hm] *.[csy]

vgrind: sources
	cp /dev/null index
	$(VGRIND) -h "$(HEADER) (Release 8)" -n Makefile 
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

#
# We assume that systags has already been done.
#
installtags:
	sed "/	equel/d" ../tags | sed "s/	/	..\//" > othertags
	sort -o tags tags othertags
	-rm -f othertags systags

sources: $(SRCS) $(HDRS)

$(HDRS):
	$(GET) $(REL) $@
$(SRCS):
	$(GET) $(REL) $@

y.tab.h: grammar.c

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

main.o:constants.h
main.o:globals.h
yylex.o:constants.h
yylex.o:globals.h
yylex.o:y.tab.h
grammar.o:constants.h
grammar.o:globals.h
grammar.o:tokens.y
cvar.o:constants.h
cvar.o:globals.h
include.o:constants.h
include.o:globals.h
name.o:constants.h
name.o:globals.h
name.o:y.tab.h
getkey.o:constants.h
getkey.o:globals.h
number.o:constants.h
number.o:globals.h
number.o:y.tab.h
operator.o:constants.h
operator.o:globals.h
operator.o:y.tab.h
prtout.o:constants.h
prtout.o:globals.h
string.o:constants.h
string.o:globals.h
string.o:y.tab.h
comment.o:constants.h
comment.o:globals.h
retrieve.o:constants.h
retrieve.o:globals.h
display.o:constants.h
display.o:globals.h
getch.o:constants.h
getch.o:globals.h
cmap.o:constants.h
yyerror.o:constants.h
yyerror.o:globals.h
yyerror.o:y.tab.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
