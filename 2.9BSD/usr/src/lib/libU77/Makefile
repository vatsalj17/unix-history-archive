# Makefile for f77 system lib, libU77.a
# @(#)Makefile	1.1 (Berkeley) 5/20/83

CFLAGS	      = -O $(FP)

DEST	      = $(DESTDIR)/usr/lib

DESTDIR	      =

EXTHDRS	      =

FP	      =				# -f for machines without floating point

HDRS	      =

LIBRARY	      = libU77.a

MAKEFILE      = Makefile

OBJS	      = access_.o \
		alarm_.o \
		chdir_.o \
		chmod_.o \
		ctime_.o \
		dtime_.o \
		etime_.o \
		f77lid_.o \
		fdate_.o \
		fgetc_.o \
		fork_.o \
		fputc_.o \
		fseek_.o \
		fstat_.o \
		ftell_.o \
		gerror_.o \
		getarg_.o \
		getc_.o \
		getcwd_.o \
		getenv_.o \
		getgid_.o \
		getlog_.o \
		getpid_.o \
		getuid_.o \
		gmtime_.o \
		hostnm_.o \
		iargc_.o \
		idate_.o \
		ierrno_.o \
		isatty_.o \
		itime_.o \
		kill_.o \
		link_.o \
		loc_.o \
		lstat_.o \
		ltime_.o \
		perror_.o \
		putc_.o \
		qsort_.o \
		signal_.o \
		sleep_.o \
		stat_.o \
		symlnk_.o \
		system_.o \
		tapeio.o \
		time_.o \
		ttynam_.o \
		unlink_.o \
		wait_.o

OLDDIR	      = $(DESTDIR)/usr/old/lib

PRINT	      = pr

SRCS	      = access_.c \
		alarm_.c \
		chdir_.c \
		chmod_.c \
		ctime_.c \
		dtime_.c \
		etime_.c \
		f77lid_.c \
		fdate_.c \
		fgetc_.c \
		fork_.c \
		fputc_.c \
		fseek_.c \
		fstat_.c \
		ftell_.c \
		gerror_.c \
		getarg_.c \
		getc_.c \
		getcwd_.c \
		getenv_.c \
		getgid_.c \
		getlog_.c \
		getpid_.c \
		getuid_.c \
		gmtime_.c \
		hostnm_.c \
		iargc_.c \
		idate_.c \
		ierrno_.c \
		isatty_.c \
		itime_.c \
		kill_.c \
		link_.c \
		loc_.c \
		lstat_.c \
		ltime_.c \
		perror_.c \
		putc_.c \
		qsort_.c \
		signal_.c \
		sleep_.c \
		stat_.c \
		symlnk_.c \
		system_.c \
		tapeio.c \
		time_.c \
		ttynam_.c \
		unlink_.c \
		wait_.c

all:		$(LIBRARY)

$(LIBRARY):	$(OBJS) Version
		@echo -n "Loading $(LIBRARY) ... "
		@ar cru $(LIBRARY) $(OBJS) Version
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

cp:		$(LIBRARY)
		@-mv $(DEST)/$(LIBRARY) $(OLDDIR)/$(LIBRARY)
		cp $(LIBRARY) $(DEST)/$(LIBRARY)
		ranlib $(DEST)/$(LIBRARY)

library:        $(LIBRARY)

print:;		@$(PRINT) $(HDRS) $(SRCS)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:         $(DEST)/$(LIBRARY)

$(DEST)/$(LIBRARY): $(SRCS) $(HDRS) $(EXTHDRS)
		@-ar xo $(DEST)/$(LIBRARY)
		@make -f $(MAKEFILE) DEST=$(DEST) cp clean
.c.o:
		$(CC) $(CFLAGS) -c $*.c
		-ld -x -r $*.o
		mv a.out $*.o
