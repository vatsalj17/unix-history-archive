/* $Header: /ker/coh.386/RCS/var.c,v 2.7 93/10/29 00:56:29 nigel Exp Locker: nigel $ */
/*
 * Coherent global variables.
 *
 * $Log:	var.c,v $
 * Revision 2.7  93/10/29  00:56:29  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.6  93/08/25  12:38:42  nigel
 * Remove numerous unreferenced globals
 * 
 * Revision 2.5  93/08/19  10:37:46  nigel
 * r83 ioctl (), corefile, new headers
 * 
 * Revision 2.4  93/08/19  03:27:01  nigel
 * Nigel's r83 (Stylistic cleanup)
 */

#include <stddef.h>

#define	_KERNEL		1

#include <kernel/timeout.h>
#include <kernel/systab.h>
#include <kernel/_timers.h>
#include <kernel/trace.h>
#include <sys/uproc.h>
#include <sys/buf.h>
#include <sys/con.h>
#include <sys/inode.h>
#include <sys/mount.h>
#include <sys/proc.h>
#include <sys/ptrace.h>
#include <sys/seg.h>
#include <sys/mmu.h>
#include <sys/ino.h>

char		__PROC_V_SYM (__PROC_VERSION) [] = __DATE__;
char		__UAREA_V_SYM (__UAREA_VERSION) [] = __TIME__;

long	 lbolt;				/* _timers.h */

INODE	*acctip;			/* inode.h */

struct __sysmem	sysmem;				/* mmu.h */
MOUNT	*mountp;			/* mount.h */

#if	TRACER & TRACE_ERRNO
unsigned t_errno = 0;
#endif
#if	TRACER & TRACE_HAL
unsigned t_hal = 0;
#endif
#if	TRACER & TRACE_PIGGY
unsigned t_piggy = 0;
#endif
#if	TRACER & TRACE_VLAD
unsigned t_vlad = 0;
#endif
#if	TRACER & TRACE_CON
unsigned t_con = 0;
#endif
#if	TRACER & TRACE_MSGQ
unsigned t_msgq = 0;
#endif
#if	TRACER & TRACE_INODE
unsigned short t_inumber = 0;
#endif
#if	TRACER & TRACE_FILESYS
unsigned short t_filesys = 0;
unsigned short t_filedev = 0;
#endif


/*
 * Time.
 */

struct _TIME_OF_DAY timer = {
	0,				/* Initial time */
	0,				/* Ticks */
	-1 * 60,			/* Mittel Europa Zeit */
	1				/* Daylight saving time */
};

/* for ulimit - max # of blocks per file */
int	BPFMAX	= (ND + NBN + NBN*NBN + NBN*NBN*NBN);

unsigned	ISTSIZE	= 2048;		/* sys/param.h */
int	quantum;			/* proc.h */
int	disflag;			/* proc.h */

__DUMB_GATE	__pnxgate = __GATE_DECLARE ("process table");
PROC	procq;				/* proc.h */

PROC	*eprocp;			/* proc.h */
PROC	*cprocp;			/* proc.h */

TIM *	timq [32];			/* timeout.h */

int	vtactive;

/*
 * System call functions.
 */

int	unone();
int	uexit();
int	ufork();
int	uread();
int	uwrite();
int	uopen();
int	uclose();
int	uwait();
int	uwait386();
int	ucreat();
int	ulink();
int	uunlink();
int	uexece();
int	uchdir();
int	umknod();
int	uchmod();
int	uchown();
char	*ubrk();
int	ustat();
long	ulseek();
int	ugetpid();
int	umount();
int	uumount();
int	usetuid();
int	ugetuid();
int	ustime();
int	uptrace();
int	ualarm();
int	ufstat();
int	upause();
int	uutime();
int	uaccess();
int	unice();
int	uftime();
int	usync();
int	ukill();
int	udup();
int	upipe();
int	utimes();
int	utime();
int	uprofil();
int	usetgid();
int	ugetgid();
int	(*usigsys())();
int	uacct();
int	ulock();
int	uioctl();
int	ugetegid();
int	uumask();
int	uchroot();
int	usetpgrp();
int	uulimit();
int	ufcntl();
int	upoll();
/* int	upgrp(); supplanted by a real implementation of system call 39 */
int		upgrpsys ();
int	usysi86();
int	umsgsys();
int	ushmsys();
int	uutssys();
int	usemsys();
int	urmdir();
int	umkdir();
int	ugetdents();
int	ustatfs();
int	ufstatfs();
int	uadmin();

int		ugetmsg ();
int		uputmsg ();

/*
 * Added by hal 91/10/10.
 * These are undocumented Xenix/V7 compatibility calls.
 */

int	ustty();
int	ugtty();

/*
 * NIGEL: These are all defined in "i386/sys1632.c" for compatibility with
 * the i286 system calls. There were dealt with by an ugly system, now very
 * much cleaned up.
 */

int	obrk ();
int	ostat ();
int	ostime ();
int	ofstat ();
int	oftime ();
int	coh286dup ();
int	opipe ();
long	oalarm2 ();
long	otick ();
int	osetpgrp ();
int	ogetpgrp ();
int	ogeteuid ();
int	ogetegid ();
int	okill ();
int	osignal ();
long	olseek ();
int	ounique ();


/*
 * NIGEL: Use the following function to create static instances of system-
 * call entry points properly. The extra cast on the function member is bad
 * for error checking, but it's not as if there was any here before anyhow.
 */

#define	SYSCALL(nargs,type,func) \
	{ nargs, __CONCAT (__SYSCALL_, type), (__sysfunc_t) func, \
	  __STRING (func) }


/*
 * System call table.
 */

int	ucohcall();
struct systab cohcall =	SYSCALL (6, INT, ucohcall);

struct systab sysitab [NMICALL] = {
	SYSCALL (0, INT,  unone),		/*  0 = ??? */
	SYSCALL (1, INT,  uexit),		/*  1 = exit */
	SYSCALL (0, INT,  ufork),		/*  2 = fork */
	SYSCALL (3, INT,  uread),		/*  3 = read */
	SYSCALL (3, INT,  uwrite),		/*  4 = write */
	SYSCALL (3, INT,  uopen),		/*  5 = open */
	SYSCALL (1, INT,  uclose),		/*  6 = close */
	SYSCALL (3, INT,  uwait386),		/*  7 = wait/waitpid */
	SYSCALL (2, INT,  ucreat),		/*  8 = creat */
	SYSCALL (2, INT,  ulink),		/*  9 = link */
	SYSCALL (1, INT,  uunlink),		/* 10 = unlink */
	SYSCALL (0, INT,  unone),		/* 11 = exec */
	SYSCALL (1, INT,  uchdir),		/* 12 = chdir */
	SYSCALL (0, INT,  utime),		/* 13 = utime */
	SYSCALL (3, INT,  umknod),		/* 14 = mknod */
	SYSCALL (2, INT,  uchmod),		/* 15 = chmod */
	SYSCALL (3, INT,  uchown),		/* 16 = chown */
	SYSCALL (1, INT,  ubrk),		/* 17 = break */
	SYSCALL (2, INT,  ustat),		/* 18 = stat */
	SYSCALL (3, LONG, ulseek),		/* 19 = lseek */
	SYSCALL (0, INT,  ugetpid),		/* 20 = getpid */
	SYSCALL (3, INT,  umount),		/* 21 = mount */
	SYSCALL (1, INT,  uumount),		/* 22 = umount */
	SYSCALL (1, INT,  usetuid),		/* 23 = setuid */
	SYSCALL (0, INT,  ugetuid),		/* 24 = getuid */
	SYSCALL (1, INT,  ustime),		/* 25 = stime */
	SYSCALL (4, INT,  uptrace),		/* 26 = ptrace */
	SYSCALL (1, INT,  ualarm),		/* 27 = alarm */
	SYSCALL (2, INT,  ufstat),		/* 28 = fstat */
	SYSCALL (0, INT,  upause),		/* 29 = pause */
	SYSCALL (2, INT,  uutime),		/* 30 = utime */
	SYSCALL (2, INT,  ustty),		/* 31 = ustty */
	SYSCALL (2, INT,  ugtty),		/* 32 = ugtty */
	SYSCALL (2, INT,  uaccess),		/* 33 = access */
	SYSCALL (1, INT,  unice),		/* 34 = nice */
	SYSCALL (4, INT,  ustatfs),		/* 35 = statfs */
	SYSCALL (0, INT,  usync),		/* 36 = sync */
	SYSCALL (2, INT,  ukill),		/* 37 = kill */
	SYSCALL (4, INT,  ufstatfs),		/* 38 = ufstatfs */
	SYSCALL (3, INT,  upgrpsys),		/* 39 = pgrp */
	SYSCALL (0, LONG, unone),		/* 40 = ??? */
	SYSCALL (1, INT,  udup),		/* 41 = dup */
	SYSCALL (0, INT,  upipe),		/* 42 = pipe */
	SYSCALL (1, INT,  utimes),		/* 43 = times */
	SYSCALL (4, INT,  uprofil),		/* 44 = profil */
	SYSCALL (1, INT,  ulock),		/* 45 = lock */
	SYSCALL (1, INT,  usetgid),		/* 46 = setgid */
	SYSCALL (0, INT,  ugetgid),		/* 47 = getgid */
	SYSCALL (2, INT,  usigsys),		/* 48 = signal */
	SYSCALL (6, LONG, umsgsys),		/* 49 = msgsys */
	SYSCALL (5, LONG, usysi86),		/* 50 = sysi86 */	
	SYSCALL (1, INT,  uacct),		/* 51 = acct */
	SYSCALL (4, INT,  ushmsys),		/* 52 = shmsys */
	SYSCALL (5, INT,  usemsys),		/* 53 = semsys */
	SYSCALL (3, INT,  uioctl),		/* 54 = ioctl */
	SYSCALL (3, INT,  uadmin),		/* 55 = uadmin */
	SYSCALL (0, INT,  unone),		/* 56 = ??? */
	SYSCALL (3, INT,  uutssys),		/* 57 = utssys */
	SYSCALL (0, INT,  unone),		/* 58 = ??? */
	SYSCALL (3, INT,  uexece),		/* 59 = exec */
	SYSCALL (1, INT,  uumask),		/* 60 = umask */
	SYSCALL (1, INT,  uchroot),		/* 61 = chroot */
	SYSCALL (3, INT,  ufcntl),		/* 62 = fcntl */
	SYSCALL (2, INT,  uulimit),		/* 63 = ulimit */
	SYSCALL (0, INT,  unone),		/* 64 = ??? (sload) */
	SYSCALL (0, INT,  unone),		/* 65 = ??? (suload */
	SYSCALL (0, INT,  unone),		/* 66 = ??? (fcntl) */
	SYSCALL (0, INT,  unone),		/* 67 = ??? (poll) */
	SYSCALL (0, INT,  unone),		/* 68 = ??? (msgctl) */
	SYSCALL (0, INT,  unone),		/* 69 = ??? (msgget) */
	SYSCALL (0, INT,  unone),		/* 70 = ??? (msgrcv) */
	SYSCALL (0, INT,  unone),		/* 71 = ??? (msgsnd) */
	SYSCALL (0, LONG, unone),		/* 72 = ??? (alarm2) */
	SYSCALL (0, LONG, unone),		/* 73 = tick  */
	SYSCALL (0, INT,  unone),		/* 74 = ??? */
	SYSCALL (0, INT,  unone),		/* 75 = ??? */
	SYSCALL (0, INT,  unone),		/* 76 = ??? */
	SYSCALL (0, INT,  unone),		/* 77 = ??? */
	SYSCALL (0, INT,  unone),		/* 78 = ??? */
	SYSCALL (1, INT,  urmdir),		/* 79 = rmdir */
	SYSCALL (2, INT,  umkdir),		/* 80 = mkdir */
	SYSCALL (3, INT,  ugetdents),		/* 81 = getdents */
	SYSCALL (0, INT,  unone),		/* 82 = ??? */
	SYSCALL (0, INT,  unone),		/* 83 = ??? */
	SYSCALL (0, INT,  unone),		/* 84 = ??? */
	SYSCALL (4, INT,  ugetmsg),		/* 85 = getmsg */
	SYSCALL (4, INT,  uputmsg),		/* 86 = putmsg */
	SYSCALL (3, INT,  upoll)		/* 87 = poll */
};


/*
 * Table for 286 system calls; the arguments of 286 system calls are zero-
 * filled shorts.
 *
 * NIGEL: Since I built this table from the 386 table and a switch statement
 * that dealt with the differences, we permit many calls here that are not
 * part of the old 286 API. Of course, so did the code I based this on...
 */

struct systab sys286tab [NMICALL] = {
	SYSCALL (0, INT,  unone),		/*  0 = ??? */
	SYSCALL (1, INT,  uexit),		/*  1 = exit */
	SYSCALL (0, INT,  ufork),		/*  2 = fork */
	SYSCALL (3, INT,  uread),		/*  3 = read */
	SYSCALL (3, INT,  uwrite),		/*  4 = write */
	SYSCALL (3, INT,  uopen),		/*  5 = open */
	SYSCALL (1, INT,  uclose),		/*  6 = close */
	SYSCALL (1, INT,  uwait),		/*  7 = wait */
	SYSCALL (2, INT,  ucreat),		/*  8 = creat */
	SYSCALL (2, INT,  ulink),		/*  9 = link */
	SYSCALL (1, INT,  uunlink),		/* 10 = unlink */
	SYSCALL (3, INT,  uexece),		/* 11 = 286 exece */
	SYSCALL (1, INT,  uchdir),		/* 12 = chdir */
	SYSCALL (0, INT,  utime),		/* 13 = utime */
	SYSCALL (3, INT,  umknod),		/* 14 = mknod */
	SYSCALL (2, INT,  uchmod),		/* 15 = chmod */
	SYSCALL (3, INT,  uchown),		/* 16 = chown */
	SYSCALL (1, INT,  obrk),		/* 17 = 286 break */
	SYSCALL (2, INT,  ostat),		/* 18 = 286 stat */
	SYSCALL (4, LONG, olseek),		/* 19 = 286 lseek */
	SYSCALL (0, INT,  ugetpid),		/* 20 = getpid */
	SYSCALL (3, INT,  umount),		/* 21 = mount */
	SYSCALL (1, INT,  uumount),		/* 22 = umount */
	SYSCALL (1, INT,  usetuid),		/* 23 = setuid */
	SYSCALL (0, INT,  ugetuid),		/* 24 = getuid */
	SYSCALL (1, INT,  ostime),		/* 25 = 286 stime */
	SYSCALL (4, INT,  uptrace),		/* 26 = ptrace */
	SYSCALL (1, INT,  ualarm),		/* 27 = alarm */
	SYSCALL (2, INT,  ofstat),		/* 28 = 286 fstat */
	SYSCALL (0, INT,  upause),		/* 29 = pause */
	SYSCALL (2, INT,  uutime),		/* 30 = utime */
	SYSCALL (2, INT,  ustty),		/* 31 = ustty */
	SYSCALL (2, INT,  ugtty),		/* 32 = ugtty */
	SYSCALL (2, INT,  uaccess),		/* 33 = access */
	SYSCALL (1, INT,  unice),		/* 34 = nice */
	SYSCALL (1, INT,  oftime),		/* 35 = 286 ftime */
	SYSCALL (0, INT,  usync),		/* 36 = sync */
	SYSCALL (2, INT,  ukill),		/* 37 = kill */
	SYSCALL (4, INT,  ufstatfs),		/* 38 = ufstatfs */
	SYSCALL (0, INT,  unone),		/* 39 = pgrp */
	SYSCALL (0, INT,  unone),		/* 40 = ??? */
	SYSCALL (2, INT,  coh286dup),		/* 41 = 286 dup */
	SYSCALL (1, INT,  opipe),		/* 42 = 286 pipe */
	SYSCALL (1, INT,  utimes),		/* 43 = times */
	SYSCALL (4, INT,  uprofil),		/* 44 = profil */
	SYSCALL (0, INT,  ounique),		/* 45 = 286 unique */
	SYSCALL (1, INT,  usetgid),		/* 46 = setgid */
	SYSCALL (0, INT,  ugetgid),		/* 47 = getgid */
	SYSCALL (2, INT,  osignal),		/* 48 = 286 signal */
	SYSCALL (6, LONG, umsgsys),		/* 49 = msgsys */
	SYSCALL (5, LONG, usysi86),		/* 50 = sysi86 */	
	SYSCALL (1, INT,  uacct),		/* 51 = acct */
	SYSCALL (4, INT,  ushmsys),		/* 52 = shmsys */
	SYSCALL (1, INT,  ulock),		/* 53 = 286 ulock */
	SYSCALL (3, INT,  uioctl),		/* 54 = ioctl */
	SYSCALL (3, INT,  uadmin),		/* 55 = uadmin */
	SYSCALL (0, INT,  ogetegid),		/* 56 = 286 getegid */
	SYSCALL (0, INT,  ogeteuid),		/* 57 = 286 geteuid */
	SYSCALL (0, INT,  unone),		/* 58 = ??? */
	SYSCALL (3, INT,  uexece),		/* 59 = exec */
	SYSCALL (1, INT,  uumask),		/* 60 = umask */
	SYSCALL (1, INT,  uchroot),		/* 61 = chroot */
	SYSCALL (0, INT,  osetpgrp),		/* 62 = 286 setpgrp */
	SYSCALL (0, INT,  ogetpgrp),		/* 63 = 286 getpgrp */
	SYSCALL (0, INT,  unone),		/* 64 = ??? (sload) */
	SYSCALL (0, INT,  unone),		/* 65 = ??? (suload */
	SYSCALL (3, INT,  ufcntl),		/* 66 = 286 fcntl */
	SYSCALL (0, INT,  unone),		/* 67 = ??? (poll) */
	SYSCALL (0, INT,  unone),		/* 68 = ??? (msgctl) */
	SYSCALL (0, INT,  unone),		/* 69 = ??? (msgget) */
	SYSCALL (0, INT,  unone),		/* 70 = ??? (msgrcv) */
	SYSCALL (0, INT,  unone),		/* 71 = ??? (msgsnd) */
	SYSCALL (1, LONG, oalarm2),		/* 72 = 286 alarm2 */
	SYSCALL (0, LONG, otick),		/* 73 = 286 tick */
	SYSCALL (0, INT,  unone),		/* 74 = ??? */
	SYSCALL (0, INT,  unone),		/* 75 = ??? */
	SYSCALL (0, INT,  unone),		/* 76 = ??? */
	SYSCALL (0, INT,  unone),		/* 77 = ??? */
	SYSCALL (0, INT,  unone),		/* 78 = ??? */
	SYSCALL (1, INT,  urmdir),		/* 79 = rmdir */
	SYSCALL (2, INT,  umkdir),		/* 80 = mkdir */
	SYSCALL (3, INT,  ugetdents),		/* 81 = getdents */
	SYSCALL (0, INT,  unone),		/* 82 = ??? */
	SYSCALL (0, INT,  unone),		/* 83 = ??? */
	SYSCALL (0, INT,  unone),		/* 84 = ??? */
	SYSCALL (0, INT,  unone),		/* 85 = ??? */
	SYSCALL (0, INT,  unone),		/* 86 = ??? */
	SYSCALL (3, INT,  upoll)		/* 87 = poll */
};


/*
 *  System Calls Numbers of the form 0x??28, where 0x?? >= 0x01
 *  Assists the dispatching mechanism in i386/trap.c
 */

int	uchsize();
int	unap();

/*
 * Coherent 4.2 new system calls available only from 386 code.
 */

int	usigaction ();
int	usigpending ();
int	usigprocmask ();
int	usigsuspend ();
int	upathconf ();
int	ufpathconf ();
int	usysconf ();
int	ugetgroups ();
int	usetgroups ();
int	urename ();

struct systab h28itab [H28CALL] = {
	SYSCALL (0, INT,  unone),		/* 0x0128 = locking */
	SYSCALL (0, INT,  unone),		/* 0x0228 = creatsem */
	SYSCALL (0, INT,  unone),		/* 0x0328 = opensem */
	SYSCALL (0, INT,  unone),		/* 0x0428 = sigsem */
	SYSCALL (0, INT,  unone),		/* 0x0528 = waitsem */
	SYSCALL (0, INT,  unone),		/* 0x0628 = nbwaitsem */
	SYSCALL (0, INT,  unone),		/* 0x0728 = rdchk */
	SYSCALL (0, INT,  unone),		/* 0x0828 = ??? */
	SYSCALL (0, INT,  unone),		/* 0x0928 = ??? */
	SYSCALL (2, INT,  uchsize),		/* 0x0A28 = chsize */
	SYSCALL (1, INT,  oftime),		/* 0x0B28 = ftime */
	SYSCALL (1, INT,  unap),		/* 0x0C28 = nap */
	SYSCALL (0, INT,  unone),		/* 0x0D28 = _sdget */
	SYSCALL (0, INT,  unone),		/* 0x0E28 = sdfree */
	SYSCALL (0, INT,  unone),		/* 0x0F28 = sdenter */
	SYSCALL (0, INT,  unone),		/* 0x1028 = sdleave */
	SYSCALL (0, INT,  unone),		/* 0x1128 = sdgetv */
	SYSCALL (0, INT,  unone),		/* 0x1228 = sdwaitv */
	SYSCALL (0, INT,  unone),		/* 0x1328 = ?? */
	SYSCALL (0, INT,  unone),		/* 0x1428 = ?? */
	SYSCALL (0, INT,  unone),		/* 0x1528 = ?? */
	SYSCALL (0, INT,  unone),		/* 0x1628 = ?? */
	SYSCALL (0, INT,  unone),		/* 0x1728 = ?? */
	SYSCALL (0, INT,  unone),		/* 0x1828 = ?? */
	SYSCALL (0, INT,  unone),		/* 0x1928 = ?? */
	SYSCALL (0, INT,  unone),		/* 0x1A28 = ?? */
	SYSCALL (0, INT,  unone),		/* 0x1B28 = ?? */
	SYSCALL (0, INT,  unone),		/* 0x1C28 = ?? */
	SYSCALL (0, INT,  unone),		/* 0x1D28 = ?? */
	SYSCALL (0, INT,  unone),		/* 0x1E28 = ?? */
	SYSCALL (0, INT,  unone),		/* 0x1F28 = ?? */
	SYSCALL (0, INT,  unone),		/* 0x2028 = proctl */
	SYSCALL (0, INT,  unone),		/* 0x2128 = execseg */
	SYSCALL (0, INT,  unone),		/* 0x2228 = unexecseg */
	SYSCALL (0, INT,  unone),		/* 0x2328 = ?? */
	SYSCALL (0, INT,  unone),		/* 0x2428 = select */
	SYSCALL (0, INT,  unone),		/* 0x2528 = ?? */
	SYSCALL (0, INT,  unone),		/* 0x2628 = ?? */
	SYSCALL (3, INT,  usigaction),		/* 0x2728 = sigaction */
	SYSCALL (3, INT,  usigprocmask),	/* 0x2828 = sigprocmask */
	SYSCALL (1, INT,  usigpending),		/* 0x2928 = sigpending */
	SYSCALL (2, INT,  usigsuspend),		/* 0x2A28 = sigsuspend */
	SYSCALL (2, INT,  ugetgroups),		/* 0x2B28 = getgroups */
	SYSCALL (2, INT,  usetgroups),		/* 0x2C28 = setgroups */
	SYSCALL (1, INT,  usysconf),		/* 0x2D28 = sysconf */
	SYSCALL (2, INT,  upathconf),		/* 0x2E28 = pathconf */
	SYSCALL (2, INT,  ufpathconf),		/* 0x2F28 = fpathconf */
	SYSCALL (2, INT,  urename)		/* 0x3028 = rename */
};
