LIB = /lib
DD = tmp

CC = cc
CCFLAGS = -O
USRINC = /usr/include/

LD = -ld
LDFLAGS = -x

.c.o:
	$(CC) -c $(CCFLAGS) $<
	$(LD) -r $(LDFLAGS) $*.o
	mv a.out $*.o

AS = -as

.s.o:
	$(AS) -o $*.o $<
	$(LD) -r $(LDFLAGS) $*.o
	mv a.out $*.o

AR = ar


#------
# install: Create and install libc.a & *crt.o.
#------
install: in_libc.a in_crt0


#------
# libc.a: Create libc.a in local directory.
#------
libc.a: abort.o abs.o access.o alarm.o alloc.o malloc.o area.o atof.o atoi.o atol.o \
	chdir.o chmod.o chown.o ctime.o dup.o execl.o execv.o exect.o rin.o \
	fltpr.o fork.o fstat.o getcsw.o getgid.o getpw.o \
	getuid.o gtty.o hmul.o kill.o \
	ladd.o ldfps.o link.o locv.o longops.o ltod.o

libc.a:	makdir.o mcount.o mknod.o mktemp.o \
	mon.o mount.o nice.o nlist.o \
	perror.o pipe.o printf.o ptrace.o putc.o qsort.o reset.o \
	setgid.o setuid.o signal.o sleep.o snstat.o stime.o \
	stty.o sync.o time.o times.o \
	umount.o unlink.o wait.o close.o creat.o errlst.o \
	exit.o ffltpr.o getc.o getchr.o

libc.a:	getpid.o itol.o lseek.o nargs.o open.o prof.o \
	putchr.o read.o sbrk.o seek.o tell.o \
	stat.o write.o cerror.o csv.o ltoi.o setjmp.o _exit.o \
	acct.o realloc.o cleanup.o setpgrp.o \
	strlen.o strcpy.o strcat.o strcmp.o

libc.a:
	-rm -f libc.a tmp.a
	$(AR) r tmp.a abort.o abs.o access.o alarm.o \
			realloc.o alloc.o malloc.o area.o atof.o atoi.o atol.o
	$(AR) r tmp.a chdir.o chmod.o chown.o ctime.o dup.o execl.o execv.o exect.o rin.o
	$(AR) r tmp.a fltpr.o fork.o fstat.o getcsw.o getgid.o getpw.o \
			getuid.o gtty.o hmul.o kill.o
	$(AR) r tmp.a ladd.o ldfps.o link.o locv.o longops.o ltod.o
	$(AR) r tmp.a makdir.o mcount.o mknod.o mktemp.o \
			mon.o mount.o nice.o nlist.o
	$(AR) r tmp.a perror.o pipe.o printf.o ptrace.o putc.o qsort.o reset.o
	$(AR) r tmp.a setgid.o setuid.o signal.o sleep.o snstat.o stime.o \
			stty.o sync.o time.o times.o
	$(AR) r tmp.a umount.o unlink.o wait.o close.o creat.o errlst.o \
			exit.o ffltpr.o getc.o getchr.o
	$(AR) r tmp.a getpid.o itol.o lseek.o nargs.o open.o prof.o \
			putchr.o read.o sbrk.o seek.o tell.o
	$(AR) r tmp.a stat.o write.o cerror.o csv.o ltoi.o setjmp.o _exit.o
	$(AR) r tmp.a acct.o cleanup.o setpgrp.o \
			strlen.o strcpy.o strcat.o strcmp.o
	-chmod 644 tmp.a
	mv tmp.a libc.a


#------
# in_libc.a: Create libc.a, if it isn't already, and move it to $(LIB)/libc.a.
#------
in_libc.a: libc.a
	-mv $(LIB)/libc.a $(LIB)/$(DD)libc.a
	mv libc.a $(LIB)/libc.a
	-chown bin $(LIB)/libc.a
	-rm -f $(LIB)/$(DD)libc.a
	ls -l $(LIB)/libc.a


#------
# in_crt0: Create & install crt0.o, fcrt0.o, mcrt0.o.
#------
in_crt0: crt0.o fcrt0.o mcrt0.o
	-chmod 644 crt0.o fcrt0.o mcrt0.o
	mv crt0.o $(LIB)/crt0.o
	mv fcrt0.o $(LIB)/fcrt0.o
	mv mcrt0.o $(LIB)/mcrt0.o
	-chown bin $(LIB)/crt0.o $(LIB)/fcrt0.o $(LIB)/mcrt0.o
	ls -l $(LIB)/crt0.o $(LIB)/fcrt0.o $(LIB)/mcrt0.o


#------
# clean: Remove all .o files.
#------
clean:
	-rm -f abort.o abs.o access.o alarm.o alloc.o area.o atof.o atoi.o atol.o
	-rm -f chdir.o chmod.o chown.o ctime.o dup.o execl.o execv.o exect.o rin.o
	-rm -f fltpr.o fork.o fstat.o getcsw.o getgid.o getpw.o \
			getuid.o gtty.o hmul.o kill.o
	-rm -f ladd.o ldfps.o link.o locv.o longops.o ltod.o
	-rm -f makdir.o malloc.o mcount.o mknod.o mktemp.o \
			mon.o mount.o nice.o nlist.o
	-rm -f perror.o pipe.o printf.o ptrace.o putc.o qsort.o reset.o
	-rm -f setgid.o setuid.o signal.o sleep.o snstat.o stime.o \
			stty.o sync.o time.o times.o
	-rm -f umount.o unlink.o wait.o close.o creat.o errlst.o \
			exit.o ffltpr.o getc.o getchr.o
	-rm -f getpid.o itol.o lseek.o nargs.o open.o prof.o \
			putchr.o read.o sbrk.o seek.o tell.o
	-rm -f stat.o write.o cerror.o csv.o ltoi.o setjmp.o _exit.o
	-rm -f acct.o realloc.o cleanup.o setpgrp.o  \
			strlen.o strcpy.o strcat.o strcmp.o
	-rm -f crt0.o fcrt0.o mcrt0.o


#------
# Dependencies for individual .o files.
#------
abort.o:	abort.s
abs.o:	abs.s
access.o:	access.s
alarm.o:	alarm.s
alloc.o:	alloc.c
malloc.o:	malloc.c
area.o:	area.c
atof.o:	atof.s
atoi.o:	atoi.c
atol.o:	atol.c

chdir.o:	chdir.s
chmod.o:	chmod.s
chown.o:	chown.s
ctime.o:	ctime.c
dup.o:	dup.s
execl.o:	execl.s
execv.o:	execv.s
exect.o:	exect.s
rin.o:	rin.c

fltpr.o:	fltpr.s
fork.o:	fork.s
fstat.o:	fstat.s
getcsw.o:	getcsw.s
getgid.o:	getgid.s
getpw.o:	getpw.c
getuid.o:	getuid.s
gtty.o:	gtty.s
hmul.o:	hmul.s
kill.o:	kill.s

ladd.o:	ladd.s
ldfps.o:	ldfps.s
link.o:	link.s
locv.o:	locv.s
longops.o:	longops.s
ltod.o:	ltod.s

makdir.o:	makdir.s
mcount.o:	mcount.s
mknod.o:	mknod.s
mktemp.o:	mktemp.c
mon.o:	mon.c
mount.o:	mount.s
nice.o:	nice.s
nlist.o:	nlist.c

perror.o:	perror.c
pipe.o:	pipe.s
printf.o:	printf.s
ptrace.o:	ptrace.s
putc.o:	putc.s
qsort.o:	qsort.c
reset.o:	reset.s

setgid.o:	setgid.s
setuid.o:	setuid.s
signal.o:	signal.s
sleep.o:	sleep.s
snstat.o:	snstat.s
stime.o:	stime.s
stty.o:	stty.s
sync.o:	sync.s
time.o:	time.s
times.o:	times.s

umount.o:	umount.s
unlink.o:	unlink.s
wait.o:	wait.s
close.o:	close.s
creat.o:	creat.s
errlst.o:	errlst.c
exit.o:	exit.s
ffltpr.o:	ffltpr.s
getc.o:	getc.s
getchr.o:	getchr.s

getpid.o:	getpid.s
itol.o:	itol.s
lseek.o:	lseek.c
nargs.o:	nargs.s
open.o:	open.s
prof.o:	prof.s
putchr.o:	putchr.s
read.o:	read.s
sbrk.o:	sbrk.s
seek.o:	seek.s
tell.o:	tell.s

stat.o:	stat.s
write.o:	write.s
cerror.o:	cerror.s
csv.o:	csv.s
ltoi.o:	ltoi.s
setjmp.o:	setjmp.s
_exit.o:	_exit.s

acct.o:	acct.s
realloc.o:	realloc.c
cleanup.o:	cleanup.s
setpgrp.o:	setpgrp.s
strlen.o:	strlen.c
strcpy.o:	strcpy.c
strcat.o:	strcat.c
strcmp.o:	strcmp.c

crt0.o:	crt0.s
	$(AS) -o crt0.o crt0.s
fcrt0.o:	fcrt0.s
	$(AS) -o fcrt0.o fcrt0.s
mcrt0.o:	mcrt0.s
	$(AS) -o mcrt0.o mcrt0.s
