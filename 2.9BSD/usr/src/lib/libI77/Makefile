# Makefile for f77 I/O lib, libI77.a
#
# @(#)Makefile	1.1 (Berkeley) 5/27/83

FC	      = f77

CFLAGS	      = -O $(FP)

DEST	      = $(DESTDIR)/usr/lib

DESTDIR	      =

EXTHDRS	      =

FFLAGS	      = -O $(FP)

FP	      = 			# -f for machines without floating point

HDRS	      = f_errno.h \
		fio.h \
		fiodefs.h \
		format.h \
		lio.h \
		ioprim.h \
		rat_io.h

LIBRARY	      = libI77.a

MAKEFILE      = Makefile

OBJS	      = backspace.o \
		close.o \
		dballoc.o \
		dfe.o \
		dofio.o \
		dolio.o \
		doprnt.o \
		doscan.o \
		douio.o \
		due.o \
		endfile.o \
		err.o \
		f_errlist.o \
		fmt.o \
		fmtlib.o \
		getlin.o \
		iio.o \
		inquire.o \
		ioinit.o \
		ioprim.o \
		lread.o \
		lwrite.o \
		open.o \
		printn_.o \
		putlin.o \
		rdfe.o \
		rdfmt.o \
		rewind.o \
		riio.o \
		rsfe.o \
		rwiio.o \
		scann_.o \
		sfe.o \
		sue.o \
		util.o \
		wdfe.o \
		wiio.o \
		wrtfmt.o \
		wsfe.o

OLDDIR	      = $(DESTDIR)/usr/old/lib

PRINT	      = pr

SRCS	      = backspace.c \
		close.c \
		dballoc.c \
		dfe.c \
		dofio.c \
		dolio.c \
		doprnt.c \
		doscan.c \
		douio.c \
		due.c \
		endfile.c \
		err.c \
		f_errlist.c \
		fmt.c \
		fmtlib.c \
		getlin.r \
		iio.c \
		inquire.c \
		ioinit.f \
		ioprim.c \
		lread.c \
		lwrite.c \
		open.c \
		printn_.c \
		putlin.r \
		rdfe.c \
		rdfmt.c \
		rewind.c \
		riio.c \
		rsfe.c \
		rwiio.c \
		scann_.c \
		sfe.c \
		sue.c \
		util.c \
		wdfe.c \
		wiio.c \
		wrtfmt.c \
		wsfe.c

all:		$(LIBRARY)

$(LIBRARY):	$(OBJS) Version
		@echo -n "Loading $(LIBRARY) ... "
		@ar cru $(LIBRARY) $(OBJS) Version ioinit.f
		@echo "done"

Version:	$(SRCS) mkvers
		@rm -f Version.c
		./mkvers $(LIBRARY) $(SRCS) > Version.c
		$(CC) -c Version.c
		mv Version.o Version

mkvers:		mkvers.c
		@$(CC) mkvers.c -o mkvers

clean:;		@rm -f $(OBJS) $(LIBRARY)

depend:;	@echo Construct dependencies manually

extract:;	@ar xo $(DEST)/$(LIBRARY); rm -f __.SYMDEF

index:;		@ctags -wx $(HDRS) $(SRCS)

cp:		$(LIBRARY) libI66.o
		@-mv $(DEST)/$(LIBRARY) $(OLDDIR)/$(LIBRARY)
		cp $(LIBRARY) $(DEST)/$(LIBRARY)
		ranlib $(DEST)/$(LIBRARY)
		cp libI66.o $(DEST)/libI66.a

library:        $(LIBRARY)

print:;		@$(PRINT) $(HDRS) $(SRCS)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:         $(DEST)/$(LIBRARY)

$(DEST)/$(LIBRARY): $(SRCS) $(HDRS) $(EXTHDRS)
		@-ar xo $(DEST)/$(LIBRARY)
		@make -f $(MAKEFILE) DEST=$(DEST) cp clean

f_errlist.o:	f_errlist.c
		$(CC) $(CFLAGS) -c f_errlist.c
.c.o:
		$(CC) $(CFLAGS) -c $*.c
		-ld -x -r $*.o
		mv a.out $*.o

.f.o:
		$(FC) $(FFLAGS) -c $*.f
		-ld -x -r $*.o
		mv a.out $*.o

.r.o:
		$(FC) $(FFLAGS) -c $*.r
		-ld -x -r $*.o
		mv a.out $*.o

backspace.o:	fio.h backspace.c
close.o:	fio.h close.c
dballoc.o:	f_errno.h dballoc.c
dfe.o:		fio.h dfe.c
dofio.o:	fio.h format.h dofio.c
dolio.o:	fio.h lio.h dolio.c
doscan.o:	ioprim.h doscan.c
douio.o:	fio.h douio.c
due.o:		fio.h due.c
endfile.o:	fio.h endfile.c
err.o:		fiodefs.h err.c
fmt.o:		fio.h format.h fmt.c
fmtlib.o:	fio.h fmtlib.c
getlin.o:	rat_io.h getlin.r
iio.o:		fio.h iio.c
inquire.o:	fio.h inquire.c
ioprim.o:	ioprim.h ioprim.c
lread.o:	fio.h lio.h lread.c
lwrite.o:	fio.h lio.h lwrite.c
open.o:		fio.h open.c
printn_.o:	ioprim.h printn_.c
rdfe.o:		fio.h rdfe.c
rdfmt.o:	fio.h format.h rdfmt.c
rewind.o:	fio.h rewind.c
riio.o:		fio.h lio.h riio.c
rsfe.o:		fio.h rsfe.c
rwiio.o:	fio.h lio.h rwiio.c
scann_.o:	ioprim.h scann_.c
sfe.o:		fio.h sfe.c
sue.o:		fio.h sue.c
util.o:		fio.h util.c
wdfe.o:		fio.h wdfe.c
wiio.o:		fio.h lio.h wiio.c
wrtfmt.o:	fio.h format.h wrtfmt.c
wsfe.o:		fio.h wsfe.c
