/* $Header: /ker/i386/RCS/sys1632.c,v 2.7 93/10/29 00:57:24 nigel Exp Locker: nigel $ */
/*
 * This file contains the implementations of system calls for Coherent 286,
 * and the machinery for making a system call from a 286 process.
 *
 * $Log:	sys1632.c,v $
 * Revision 2.7  93/10/29  00:57:24  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.6  93/09/13  07:51:09  nigel
 * Extra debugging (show 286 system-call return value)
 * 
 * Revision 2.5  93/09/02  18:12:18  nigel
 * Minor edits to use new flag system
 * 
 * Revision 2.4  93/08/19  03:40:15  nigel
 * Nigel's R83
 */

#include <common/_limits.h>
#include <common/_tricks.h>
#include <common/_gregset.h>
#include <sys/errno.h>
#include <sys/debug.h>
#include <sys/cmn_err.h>
#include <signal.h>
#include <stddef.h>

#define	_KERNEL		1

#include <kernel/trace.h>
#include <kernel/reg.h>
#include <sys/acct.h>
#include <sys/buf.h>
#include <sys/con.h>
#include <sys/filsys.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/proc.h>
#include <sys/uproc.h>
#include <sys/sched.h>
#include <sys/seg.h>
#include <sys/timeb.h>
#include <sys/fd.h>
#include <l.out.h>
#include <canon.h>

#include <kernel/systab.h>

/*
 * Structure returned by COH-286 stat and fstat system calls.
 */

struct oldstat {
	o_dev_t	 st_dev;		/* Device */
	o_ino_t	 st_ino;		/* Inode number */
	unsigned short st_mode;		/* Mode */
	short	 st_nlink;		/* Link count */
	short	 st_uid;		/* User id */
	short	 st_gid;		/* Group id */
	o_dev_t	 st_rdev;		/* Real device */
#pragma	align 2
	long	 st_size __ALIGN (2);	/* Size */
	long	 st_atime __ALIGN (2);	/* Access time */
	long	 st_mtime __ALIGN (2);	/* Modify time */
	long	 st_ctime __ALIGN (2);	/* Change time */
#pragma align
#pragma	align 2
};
#pragma align	/* controls structure padding in Coherent 'cc' */


/*
 * emulate a 16 bit system call
 * called from trap.c
 */

static char cvtsig [] = 
{
	0,
	SIGHUP, SIGINT, SIGQUIT, SIGALRM, SIGTERM, SIGPWR, 
	SIGSYS, SIGPIPE, SIGKILL, SIGTRAP, SIGSEGV,
	SIGEMT, /* SIGDIVE */
	SIGEMT, /* SIGOVFL */
	SIGUSR1,
	SIGUSR2,
	SIGUSR2
};

int	ostat();
int	ofstat();
int	oftime();
int	upgrp();
int	ugetuid();
int	ugetgid();
int	usysi86();
int	ulock();
int	ufcntl();
int	uexece();
long	oalarm2 ();
long	otick ();


/*
 * Duplicate a file descriptor number.  This has the same calling
 * sequence as the dup2 system call and even uses the silly DUP2 bit.
 */

int
coh286dup(ofd, nfd)
unsigned ofd;
unsigned nfd;
{
	__fd_t	      *	fdp;

	if ((fdp = fd_get (ofd & ~ DUP2)) == NULL)
		return -1;
	if ((ofd & DUP2) != 0) {
		if (nfd >= NOFILE) {
			SET_U_ERROR (EBADF, "coh286dup ()");
			return -1;
		}
		ofd &= ~ DUP2;
		if (ofd == nfd)
			return nfd;
		if (u.u_filep [nfd] != NULL) {
			fd_close (nfd);
			if (get_user_error ())
				return -1;
		}
	} else
		nfd = 0;

	return fd_dup (ofd, nfd);
}


int
ostime (timep)
time_t	      *	timep;
{
	return ustime (getuwd (timep));
}


int
opipe (pipep)
short	      *	pipep;
{
	short		res;

	res = upipe ();

	putusd (pipep, res);
	putusd (pipep + 1, u.u_rval2);
	return 0;
}


int
osetpgrp ()
{
	return setpgrp ();
}

int
ogetpgrp ()
{
	return getpgrp ();
}


int
ogeteuid ()
{
	(void) ugetuid ();
	return u.u_rval2;
}

int
ogetegid ()
{
	(void) ugetgid ();
	return u.u_rval2;
}


int
ounique ()
{
	return usysi86 (SYI86UNEEK);
}


int
okill (pid, signal)
short		pid;
unsigned	signal;
{
	if (signal >= __ARRAY_LENGTH (cvtsig)) {
		SET_U_ERROR (EINVAL, "286 kill ()");
		return -1;
	}

	return ukill (pid, cvtsig [signal]);
}


__sighand_t *
osignal (signal, func, regsetp)
unsigned	signal;
__sighand_t   *	func;
gregset_t     *	regsetp;
{
	if (signal >= __ARRAY_LENGTH (cvtsig)) {
		SET_U_ERROR (EINVAL, "286 signal ()");
		return (__sighand_t *) -1;
	}

	return (__sighand_t *) usigsys (cvtsig [signal], func, regsetp);
}


#if	__SHRT_BIT != 16
# error	This code expects 16-bit shorts
#endif

long
olseek (fd, seeklo, seekhi, whence)
unsigned	fd;
unsigned short	seeklo;
unsigned short	seekhi;
unsigned	whence;
{
	return ulseek (fd, seeklo + (seekhi << 16), whence);
}


/* msgsys, shmsys, and semsys are not emulated */
/* poll is not emulated;NOTE:the code calls putuwd */

int
oldsys (regsetp)
gregset_t     *	regsetp;
{
	register struct	systab	*stp;
	unsigned int	callnum;
	int		i;
	int		res;
	int		args [MSACOUNT];
	struct __menv	sigenv;

	set_user_error (0);
	callnum = getusd (NBPS + regsetp->_i286._ip - sizeof (short));

	/*
	 * Check that we are on an INT instruction, and that the fetch did
	 * not cause a memory fault. Note that the magic NBPS number above,
	 * which presumably means "Number of Bytes Per Segment", is how to
	 * get to 286 code.
	 */

	if (get_user_error () || (callnum & 0xFF) != 0xCD) 
		return SIGSYS;
	callnum = (callnum >> 8) & 0x7F;

	if (callnum >= __ARRAY_LENGTH (sys286tab))
		return SIGSYS;
	stp = sys286tab + callnum;

	/* Print out this 286 call only if tracing is on.  */
	T_ERRNO (4, cmn_err (CE_CONT, "[%s", stp->s_name));
	stp->s_stat ++;

	if (envsave (u.u_sigenvp = & sigenv)) {
		set_user_error (EINTR);
		goto done;
	}

	i = stp->s_nargs + 1;
	while (-- i > 0) {
		args [i - 1] = getusd (regsetp->_i286._usp +
				       i * sizeof (short));
	}

	if (get_user_error ())
		return SIGSYS;

	/*
	 * Perform the system call and collect the return value in "res".
	 */

	res = __DOSYSCALL (stp->s_nargs, stp->s_func, args, regsetp);

	if (stp->s_type == __SYSCALL_LONG)
		regsetp->_i286._dx = res >> 16;
	else
		regsetp->_i286._dx = u.u_rval2;
	regsetp->_i286._ax = res;

done:
	u.u_sigenvp = NULL;
	if (get_user_error ()) {
		T_ERRNO (4, cmn_err (CE_NOTE, "-err"));
		regsetp->_i286._ax = regsetp->_i286._dx = -1;
		putubd (MUERR, get_user_error ());
		if (get_user_error () == EFAULT)
			return SIGSYS;
	}
	T_ERRNO (4, cmn_err (CE_NOTE, "=%d] ", regsetp->_i286._ax));
	return 0;
}


/*
 * Copy the appropriate information from the inode to the stat buffer.
 */

#if	__USE_PROTO__
__LOCAL__ void oistat (struct inode * ip, struct oldstat * sbp)
#else
__LOCAL__ void
oistat(ip, sbp)
struct inode  *	ip;
struct oldstat *sbp;
#endif
{
	sbp->st_dev = ip->i_dev;
	sbp->st_ino = ip->i_ino;
	sbp->st_mode = ip->i_mode;
	sbp->st_nlink = ip->i_nlink;
	sbp->st_uid = ip->i_uid;
	sbp->st_gid = ip->i_gid;
	sbp->st_rdev = (o_dev_t) -1;
	sbp->st_size = ip->i_size;
	sbp->st_atime = ip->i_atime;
	sbp->st_mtime = ip->i_mtime;
	sbp->st_ctime = ip->i_ctime;

	switch (ip->i_mode & IFMT) {
	case IFBLK:
	case IFCHR:
		sbp->st_rdev = ip->i_rdev;
		sbp->st_size = 0;
		break;

	case IFPIPE:
		sbp->st_size = ip->i_pnc;
		break;
	}
}


/*
 * Given a file descriptor, return a status structure.
 */

int
ofstat(fd, stp)
int	fd;
struct oldstat *stp;
{
	INODE *ip;
	__fd_t	      *	fdp;
	struct oldstat stat;

	if ((fdp = fd_get (fd)) == NULL)
		return -1;
	ip = fdp->f_ip;
	oistat (ip, & stat);
	kucopy (& stat, stp, sizeof (stat));
	return 0;
}


/*
 * Return a status structure for the given file name.
 */

int
ostat(np, stp)
char *np;
struct oldstat *stp;
{
	struct oldstat stat;
	struct direct	dir;

	if (ftoi (np, 'r', IOUSR, NULL, & dir, SELF->p_credp) != 0)
		return -1;

	oistat (u.u_cdiri, & stat);

	if (kucopy (& stat, stp, sizeof (stat)) != sizeof (stat))
		SET_U_ERROR (EFAULT, "286 stat ()");

	idetach (u.u_cdiri);
	return 0;
}


/*
 * Return date and time.
 */

int
oftime(tbp)
struct timeb *tbp;
{
	struct timeb timeb;

	timeb.time = timer.t_time;
	/* This should be a machine.h macro to avoid
	 * unnecessary long arithmetic and roundoff errors
	 */
	timeb.millitm = timer.t_tick*(1000/HZ);
	timeb.timezone = timer.t_zone;
	timeb.dstflag = timer.t_dstf;

	if (kucopy (& timeb, tbp, sizeof (timeb)) != sizeof (timeb)) {
		SET_U_ERROR (EFAULT, "286 ftime ()");
		return -1;
	}
	return 0;
}


/*
 * Send a SIGALARM signal in `n' clock ticks.
 */

long
oalarm2(n)
long n;
{
	register PROC * pp = SELF;
	long s;
	extern sigalrm ();

	/*
	 * Calculate time left before current alarm timeout.
	 */
	s = 0;
	if (pp->p_alrmtim.t_last != NULL)
		s = pp->p_alrmtim.t_lbolt - lbolt;

	/*
	 * Cancel previous alarm [if any], start new alarm [if n != 0].
	 */

	timeout2 (& pp->p_alrmtim, (long) n, sigalrm, pp);

	/*
	 * Return time left before previous alarm timeout.
	 */
	return s;
}


/*
 * Return elapsed ticks since system startup.
 */

long
otick()
{
	return lbolt;
}


/*
 * Cause a signal routine to be executed.
 * Called from [coh/sig.c]
 */

void
oldsigstart (sig, func, regsetp)
int		sig;
__sighand_t   *	func;
gregset_t     *	regsetp;
{
	int		i;
	struct {
		ushort_t	sf_signo;
		ushort_t	sf_prev_ip;
		__286_flags_t	sf_flags;
	} signal_frame;

	/*
	 *                 -1
	 * calculate cvtsig  [sig]
	 *
 	 */

	signal_frame.sf_signo = sig;
	for (i = 0 ; i < __ARRAY_LENGTH (cvtsig) ; i ++)
		if (cvtsig [i] == sig) {
			signal_frame.sf_signo = i;
			break;
		}

	signal_frame.sf_prev_ip = regsetp->_i286._ip;
	signal_frame.sf_flags = regsetp->_i286._flags;

	/*
	 * Turn off single-stepping in signal handler.
	 */

	__FLAG_REG (regsetp) = __FLAG_CLEAR_FLAG (__FLAG_REG (regsetp),
						  __TRAP);
	regsetp->_i286._ip = (ushort_t) (ulong_t) func;
	regsetp->_i286._usp -= sizeof (signal_frame);

	i = kucopy (& signal_frame, regsetp->_i286._usp,
		    sizeof (signal_frame));
	ASSERT (i == sizeof (signal_frame));
}


/*
 * obrk()
 *
 * Argument is the new linear space value for the end of the PDATA segment.
 * As was done in COH286, arg of zero asks for the old upper limit.
 */

__EXTERN_C__	caddr_t		ubrk	__PROTO ((unsigned cp));

caddr_t
obrk (cp)
unsigned	cp;
{
	/*
	 * If cp nonzero
	 *	resize user data segment
	 * else
	 *	just give info - current brk address
	 */

	if (cp)
		return ubrk (cp);
	else
		return SELF->p_segl [SIPDATA].sr_base +
			SELF->p_segl [SIPDATA].sr_segp->s_size;
}
