#
# @(#)Makefile nfs.1.0 85/06/29 SAI; from 1.1 85/05/30 SMI; from UCB 4.17 83/07/02
#

OBJS=	Ovfork.o Ovadvise.o \
	async_daemon.o \
	accept.o access.o acct.o \
	bind.o brk.o \
	cerror.o chdir.o chmod.o chown.o chroot.o close.o connect.o \
	creat.o \
	dup.o dup2.o \
	execl.o execle.o exect.o execv.o execve.o _exit.o \
	fchmod.o fchown.o fcntl.o flock.o fork.o fstat.o fstatfs.o \
	fsync.o ftruncate.o \
	getdirentries.o getdomainname.o getdtablesize.o \
	getegid.o geteuid.o getfh.o getgid.o \
	getgroups.o gethostid.o gethostname.o getitimer.o getrlimit.o \
	getpagesize.o getpeername.o getpgrp.o getpid.o getppid.o getpriority.o \
	getrusage.o getsockname.o getsockopt.o gettimeofday.o getuid.o \
	ioctl.o \
	kill.o killpg.o \
	link.o listen.o lseek.o lstat.o \
	nfsmount.o nfssvc.o \
	mkdir.o mknod.o mount.o \
	open.o \
	pipe.o profil.o ptrace.o \
	quota.o \
	read.o readlink.o readv.o reboot.o recv.o recvfrom.o \
	recvmsg.o rename.o rmdir.o \
	sbrk.o select.o send.o sendmsg.o sendto.o setdomainname.o \
	setregid.o setgroups.o sethostid.o \
	sethostname.o setitimer.o setquota.o setrlimit.o \
	setpgrp.o setpriority.o setsockopt.o settimeofday.o setreuid.o \
	shutdown.o sigblock.o sigpause.o \
	sigsetmask.o sigstack.o sigvec.o socket.o socketpair.o \
	stat.o statfs.o swapon.o symlink.o sync.o syscall.o \
	truncate.o \
	umask.o unlink.o unmount.o utimes.o \
	vhangup.o \
	wait.o wait3.o write.o writev.o

.c.o:
	/lib/cpp -E -DPROF $*.c > $*.pp
	 ${AS} -o $*.o $*.pp
	rm -f $*.pp
	-ld -x -r $*.o
	mv a.out profiled/$*.o
	/lib/cpp -E $*.c > $*.pp
	${AS} -o $*.o $*.pp
	rm -f $*.pp
	-ld -x -r $*.o
	mv a.out $*.o

syslib syslib_p: ${OBJS}
	@echo "building profiled syslib_p"
	@cd profiled; ar cru ../syslib_p ${OBJS}
	@echo "building normal syslib_p"
	@ar cru syslib ${OBJS}

exec.o: exec.c
	cc -DPROF ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out profiled/exec.o
	cc -c ${CFLAGS} $*.c
	-ld -x -r $*.o
	mv a.out $*.o

clean:
	rm -f syslib syslib_p ${OBJS} profiled/*.o *.pp
