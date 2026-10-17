#
# @(#)Makefile nfs.1.0 85/06/27 SAI; from 1.1 85/05/30 SMI; from UCB 4.23 83/07/01
#

CRYPT=-DCRYPT
CFLAGS=	-O ${CRYPT}

OBJS=	alarm.o atoi.o atol.o \
	calloc.o closedir.o crypt.o ctime.o ctype_.o \
	errlst.o execvp.o \
	fstab.o \
	gcvt.o getenv.o getgrent.o getlogin.o \
	getpass.o getpwent.o getwd.o initgroups.o isatty.o \
	malloc.o mktemp.o \
	mntent.o \
	nlist.o opendir.o \
	perror.o popen.o psignal.o \
	qsort.o random.o readdir.o regex.o \
	scandir.o seekdir.o setgid.o setegid.o setrgid.o setuid.o \
	seteuid.o setruid.o siglist.o signal.o sleep.o \
	swab.o syslog.o system.o \
	telldir.o time.o timezone.o ttyname.o ttyslot.o \
	valloc.o

.c.o:
	${CC} -p ${CFLAGS} -c $*.c
	-ld -X -r $*.o
	mv a.out profiled/$*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

genlib genlib_p: ${OBJS}
	@echo "building profiled genlib"
	@cd profiled; ar cru ../genlib_p ${OBJS}
	@echo "buiding normal genlib"
	@ar cru genlib ${OBJS}

clean:
	rm -f genlib genlib_p ${OBJS} profiled/*.o

errlst.o: errlst.c
	cc -S errlst.c
	ed - <:errfix errlst.s
	as -o errlst.o errlst.s
	rm -f errlst.s
	cp errlst.o profiled/errlst.o

siglist.o: siglist.c
	cc -c -R siglist.c
	cp siglist.o profiled/siglist.o

