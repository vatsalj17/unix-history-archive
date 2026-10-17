#
#  Makefile for INGRES gutil library
#
#	@(#)Makefile	8.3	12/18/85
#

INGRES=	../..
H=	../h
LIB=	$(INGRES)/lib
CPUTYPE=VAX

OBJS=	append.o atof.o bequal.o bitcnt.o bitpos.o cat.o \
	concat.o freebuf.o ftoa.o fullwait.o \
	getufield.o iocv.o itoa.o length.o \
	xalloc.o need.o oatoi.o pad.o pmove.o \
	prargs.o scompare.o sequal.o set_si_buf.o set_so_buf.o \
	setflag.o trace.o atol.o smove.o \
	lprintf.o clrmem.o xputchar.o ztack.o syserr.o \
	min.o xfree.o bmove.$(CPUTYPE).o perror.o locv.o capital.o

SRCS1=	append.c atof.c bequal.c bitcnt.c bitpos.c cat.c \
	concat.c freebuf.c ftoa.c fullwait.c \
	getufield.c iocv.c itoa.c length.c \
	xalloc.c need.c oatoi.c pad.c pmove.c capital.c
SRCS2=	prargs.c scompare.c sequal.c set_si_buf.c set_so_buf.c \
	setflag.c trace.c atol.c smove.c \
	xputchar.c ztack.c syserr.c \
	min.c xfree.c lprintf.c clrmem.c perror.c locv.c
ASSRC=  bmove.VAX.s bmove.SUN.c
SRCS=	$(SRCS1) $(SRCS2) $(ASSRC)
LINTSRCS= $(SRCS1) $(SRCS2)

CFLAGS=	-I$H -O
LINTFLAGS= -huc -I$H
AR=	-ar
ARFLAGS=urv
RANLIB=	ranlib
GET=	sccs get
REL=
LINT=	lint
PR=	print
VGRIND=	csh /usr/ucb/vgrind
HEADER=	Gutil Library
CP=	cp
CHMOD=	chmod
LIBMODE=664

gutil:	$(OBJS)
	$(AR) $(ARFLAGS) gutil $?
	$(CHMOD) $(LIBMODE) gutil
	$(RANLIB) gutil

install: $(LIB)/gutil

$(LIB)/gutil: gutil
	$(CP) gutil $(LIB)/gutil
	$(RANLIB) $(LIB)/gutil

new:
	rm -f gutil

clean:
	rm -f [0-m]*.o
	rm -f [n-z]*.o
	rm -f core gutil

sources: $(SRCS)

$(SRCS):
	$(GET) $(REL) $@

lint:
	$(LINT) $(LINTFLAGS) $(LINTSRCS)
print: sources
	$(PR) Makefile *.[hm] *.[csy]

vgrind: sources
	cp /dev/null index
	$(VGRIND) -h "$(HEADER) (Release 8)" -n Makefile
	$(VGRIND) -h "$(HEADER) (Release 8)" *.[csy]
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
	sed "s/	/	gutil\//" tags > systags
	cat systags >> ../tags

#
# We assume that systags has already been done.
#
installtags:
	sed "/	gutil/d" ../tags | sed "s/	/	..\//" > othertags
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

bequal.o:$H/useful.h
cat.o:$H/useful.h
fullwait.o:$H/useful.h
iocv.o:$H/useful.h
xalloc.o:$H/useful.h
scompare.o:$H/useful.h
sequal.o:$H/useful.h
set_si_buf.o:$H/ingres.h
set_so_buf.o:$H/ingres.h
trace.o:$H/useful.h
atol.o:$H/useful.h
ztack.o:$H/useful.h
lprintf.o:$H/useful.h
locv.o:$H/useful.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
