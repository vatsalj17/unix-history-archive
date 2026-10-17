/* $Header: /ker/coh.386/RCS/sys1.c,v 2.7 93/10/29 00:55:39 nigel Exp Locker: nigel $ */
/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or persuant to the license agreement is unlawful.
 *
 *	COHERENT Version 2.3.37
 *	Copyright (c) 1982, 1983, 1984.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
/*
 * General system calls.
 *
 * $Log:	sys1.c,v $
 * Revision 2.7  93/10/29  00:55:39  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.6  93/09/02  18:08:16  nigel
 * Nigel's r85, minor edits only
 * 
 * Revision 2.5  93/08/25  12:32:16  nigel
 * Update wait () entry point for new flag system
 * 
 * Revision 2.4  93/08/19  03:26:50  nigel
 * Nigel's r83 (Stylistic cleanup)
 */

#include <common/_gregset.h>
#include <kernel/sig_lib.h>
#include <kernel/proc_lib.h>
#include <kernel/cred_lib.h>
#include <kernel/reg.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <sys/times.h>
#include <signal.h>
#include <sys/stat.h>
#include <stddef.h>
#include <unistd.h>

#define	_KERNEL		1

#include <kernel/_timers.h>
#include <kernel/trace.h>
#include <sys/uproc.h>
#include <sys/acct.h>
#include <sys/con.h>
#include <sys/proc.h>
#include <sys/sched.h>
#include <sys/seg.h>
#include <sys/cred.h>

/*
 * Send alarm signal to specified process - function timed by ualarm ()
 */

#if	__USE_PROTO__
void sigalrm (PROC * pp)
#else
void
sigalrm (pp)
PROC * pp;
#endif
{
	sendsig (SIGALRM, pp);
}


/*
 * Send a SIGALARM signal in `n' seconds.
 */

int
ualarm (n)
unsigned n;
{
	PROC * pp = SELF;
	unsigned s;

	/*
	 * Calculate time left before current alarm timeout.
	 */

	s = 0;
	if (pp->p_alrmtim.t_last != NULL)
		s = (pp->p_alrmtim.t_lbolt - lbolt + HZ - 1) / HZ;

	/*
	 * Cancel previous alarm [if any], start new alarm [if n != 0].
	 */

	timeout2 (& pp->p_alrmtim, (long) n * HZ, sigalrm, pp);

	/*
	 * Return time left before previous alarm timeout.
	 */

	return s;
}


/*
 * Change the size of our data segment.
 */

char *
ubrk (cp)
caddr_t cp;
{
	SEG * sp;
	caddr_t sb;
	SR	* stack_sr;
	caddr_t top_of_stack;

	/*
	 * Pick up the segment handle for our data segment.
	 */

	sp = SELF->p_segl [SIPDATA].sr_segp;


	/*
	 * Extract the starting virtual address for our data segment,
	 * as it is currently mapped into the memory space.
	 */

	sb = SELF->p_segl [SIPDATA].sr_base;


	/*
	 * We can not move the top of the data segment below the
	 * start of the data segment.
	 */

	if (cp < sb) {
		SET_U_ERROR (ENOMEM,
		    "Requested brk address is below start of data segment.");
		return -1;
	}


	/*
	 * Would the request cause a collision with the stack segment?
	 *
	 * Since the stack grows downward, its top is below its base :-).
	 */

	stack_sr = & SELF->p_segl [SISTACK];
	top_of_stack = stack_sr->sr_base - stack_sr->sr_size;

	if (btocru (cp) >= btocru (top_of_stack)) {
		SET_U_ERROR (ENOMEM,
		    "Requested brk address would collide with stack segment.");
 		return -1;
	}

	/*
	 * Attempt to establish the segment with the newly requested size.
	 */

	segsize (sp, cp - sb);


	/*
	 * Be sure to return the true new top of data segment.
	 */

	sb += sp->s_size;

	T_HAL (0x8000, printf ("=%x ", sb));
	return sb;
}


/*
 * Execute an l.out or COFF file.
 */

int
uexece (np, argp, envp, regsetp)
char	      *	np;
char	      *	argp [];
char	      *	envp [];
gregset_t     *	regsetp;
{
	return pexece (np, argp, envp, regsetp);
}


/*
 * Exit.
 */

void
uexit (s)
unsigned	s;
{
	pexit ((s & 0xFF) << 8);
}


/*
 * Fork.
 */

pid_t
ufork ()
{
	pid_t		child_pid;

	if ((child_pid = pfork ()) == 0) {
		/*
		 * Child.
		 */

		u.u_rval2 = SELF->p_pid;
		return 0;
	}

	/*
	 * Parent.
	 */

	u.u_rval2 = 0;
	return child_pid;
}


/*
 * Get group id.
 * Get effective group id.
 */

gid_t
ugetgid ()
{
	u.u_rval2 = SELF->p_credp->cr_gid;
	return SELF->p_credp->cr_rgid;
}


/*
 * Get user id.
 * Get effective user id.
 */

uid_t
ugetuid ()
{
	u.u_rval2 = SELF->p_credp->cr_uid;
	return SELF->p_credp->cr_ruid;
}


/*
 * Get process group.
 * Set the process group.
 *
 * This is System V type setpgrp ().
 * Set process group equal to process id (make process its own group leader).
 * If process was NOT already a group leader, lose its controlling terminal.
 */

int
upgrp (fl)
int	fl;
{
	if (fl) {
		if (SELF->p_group != SELF->p_pid)
			SELF->p_ttdev = NODEV;
		SELF->p_group = SELF->p_pid;
	}
	return SELF->p_group;
}


/*
 * Get process id.
 */

pid_t
ugetpid ()
{
	u.u_rval2 = SELF->p_ppid;
	return SELF->p_pid;
}


/*
 * See if we have permission to send the signal, `sig' to the process, `pp'.
 */

#if	__USE_PROTO__
__LOCAL__ int sigperm (int sig, __proc_t * procp)
#else
__LOCAL__ int
sigperm (sig, procp)
int sig;
__proc_t * procp;
#endif
{
	/*
	 * Note that when we support job control, SIGCONT requires only that
	 * the target process be in the same session.
	 */

#ifdef	_POSIX_SAVED_IDS
	if (SELF->p_credp->cr_suid == procp->p_credp->cr_suid ||
	    SELF->p_credp->cr_ruid == procp->p_credp->cr_ruid)
		return 1;
#else
	if (SELF->p_credp->cr_uid == procp->p_credp->cr_uid ||
	    SELF->p_credp->cr_ruid == procp->p_credp->cr_ruid)
		return 1;
#endif

#if	0
	/*
	 * NIGEL: This is wrong! I have moved the real-id match code above.
	 */

	if (SELF->p_credp->cr_ruid == procp->p_credp->cr_ruid &&
	    (sig == SIGHUP || sig == SIGINT || sig == SIGQUIT ||
	     sig == SIGTERM)) 
		return 1;
#endif

	if (SELF->p_credp->cr_uid == 0) {
		u.u_flag |= ASU;
		return 1;
	}
	return 0;
}


/*
 * Iteration function user below in ukill ().
 */

#if	__USE_PROTO__
int _kill_proc (__proc_t * procp, __VOID__ * arg)
#else
int
_kill_proc (procp, arg)
__proc_t      *	procp;
__VOID__      *	arg;
#endif
{
	int		sig;

	ASSERT (procp != NULL && arg != NULL);

	sig = * (int *) arg;

	if (sigperm (sig, procp)) {
		if (sig)
			sendsig (sig, procp);
	} else
		set_user_error (EPERM);

	return 0;	/* continue iterating */
}


/*
 * Send the signal `sig' to the process with id `pid'.
 */

int
ukill (pid, sig)
int pid;
unsigned sig;
{
	PROC	      *	procp;
	int sigflag;

	if (! __IN_RANGE (0, sig, _SIGNAL_MAX)) {
		set_user_error (EINVAL);
		return -1;
	}

	sigflag = 0;

	if (pid > 0) {	/* send to matching process */
		if ((procp = process_find_pid (pid)) != NULL &&
		    procp->p_state != PSDEAD) {
		    	/*
		    	 * Originally, this code didn't do the permissions
		    	 * checking if sig == 0.
		    	 */

			if (sigperm (sig, procp)) {
				if (sig)
					sendsig (sig, procp);
			} else
				set_user_error (EPERM);
			sigflag = 1;
		}
	} else if (pid < -1)
		sigflag = process_find_pgid (- pid, _kill_proc, & sig);
	else if (pid == 0)
		sigflag = process_find_pgid (SELF->p_group, _kill_proc, & sig);
	else if (pid == -1)
		sigflag = process_find_all (_kill_proc, & sig);

	if (sigflag == 0)
		set_user_error (ESRCH);

	return 0;
}


/*
 * Lock a process in core.
 */

int
ulock (f)
int f;
{
	if (super () == 0)
		return -1;
	if (f)
		SELF->p_flags |= PFLOCK;
	else
		SELF->p_flags &= ~ PFLOCK;
	return 0;
}


/*
 * Change priority by the given increment.
 */

int
unice (n)
int n;
{
	n += SELF->p_nice;
	if (n < MINNICE)
		n = MINNICE;
	if (n > MAXNICE)
		n = MAXNICE;
	if (n < SELF->p_nice && super () == 0)
		return -1;
	SELF->p_nice = n;
	return 0;
}


/*
 * Non existant system call.
 */

int
unone ()
{
	set_user_error (EFAULT);
	return -1;
}


/*
 * Pause.  Go to sleep on a channel that nobody will wakeup so that only
 * signals will wake us up.
 */

int
upause ()
{
	x_sleep ((char *) & u, prilo, slpriSigLjmp, "pause");
	return 0;
}


/*
 * Start / stop profiling.
 *
 * buff:	address in user data of an array of shorts
 * bufsiz:	number of bytes in the area at buff
 * offset:	address in user text of start of profiling area
 * scale:	0 or 1 - turn off profiling
 *		other - treat as 16 bit scale factor
 *
 * For purposes of compatibility with System 5, scale values work as follows:
 *	0xFFFF	profile buffer is same length as text being profiled.
 *	0x7FFF  profile buffer is half as long as text being profiled.
 *	0x4000	profile buffer is one fourth as long as text profiled.
 *		(each short in the buffer covers 8 bytes of text)
 *	...	...
 *	0x0002	each short in the buffer covers 64K bytes of text.
 *
 * Values 0xFFFF and 0x7FFF are used, for historical reasons, when 0x10000
 * and 0x8000, respectively, should be used.  To clean up the ensuing
 * arithmetic, there is an upward rounding kluge below.
 *
 * Each clock interrupt, take (pc - offset) * scale * (2 **-16) as a byte
 * offset into pbase.  Add 1 to the short at or below the given address
 * when profiling.
 */

int
uprofil (buff, bufsiz, offset, scale)
short * buff;
int bufsiz, offset, scale;
{
	u.u_pbase = (caddr_t) buff;
	u.u_pbend = (caddr_t) (buff + bufsiz);
	u.u_pofft = offset;
	u.u_pscale = scale & 0xffff;	/* scale is really unsigned short */

	/* round up kluge - see above */
	if ((scale & 0xfff) == 0xfff)
		u.u_pscale ++;
	return 0;
}


/*
 * Process trace.
 */

int
uptrace (req, pid, add, data)
int	req;
int * add;
pid_t		pid;
int		data;
{
	int ret;

#if	TRACER & TRACE_HAL
	int readChild = 0;	/* for debug, true if reading child memory */

	if (t_hal & 0x10000) {
		switch (req) {
		case 0:	/* init called by child */
			printf ("PSetup: child =%d  ", SELF->p_pid);
			break;
		case 1:	/* parent reads child text */
			printf ("PRdT: add =%x ", add);
			readChild = 1;
			break;
		case 2:	/* parent reads child data */
			printf ("PRdD: add =%x ", add);
			readChild = 1;
			break;
		case 3:	/* parent reads child u area */
			printf ("PRdU: add =%x ", add);
			readChild = 1;
			break;
		case 4:	/* parent writes child text */
			printf ("PWrT: add =%x data =%x  ", add, data);
			break;
		case 5:	/* parent writes child data */
			printf ("PWrD: add =%x data =%x  ", add, data);
			break;
		case 6:	/* parent writes child u area */
			printf ("PWrU: add =%x data =%x ", add, data);
			break;
		case 7:	/* resume child, maybe fake signal to child */
			printf ("PResume: sig =%d  ", data);
			break;
		case 8:	/* terminate child */
			printf ("PTerm: pid =%d  ", pid);
			break;
		case 9:	/* single-step child, maybe fake signal to child */
			printf ("PSStp: sig =%d  ", data);
			break;
		}
	}
#endif

	if (req == 0) {
		SELF->p_flags |= PFTRAC;
		ret = 0;
	} else
		ret = ptset (req, pid, add, data);

#if	TRACER & TRACE_HAL
	if (t_hal & 0x10000) {
		if (readChild)
			printf ("data =%x  ", ret);
	}
#endif

	return ret;
}


/*
 * Set group id.
 *
 * As in SVID issue 2:
 *
 * if effective gid is superuser
 *	set real, effective, and saved effective gid to argument "gid"
 * else if real gid is same as "gid"
 *	set effective gid to "gid"
 * else if saved effective gid is same as "gid"
 *	set effective gid to "gid"
 */

int
usetgid (gid)
o_gid_t gid;
{
	cred_t	      *	credp;

	if (super ()) {
		if ((credp = cred_newgid (SELF->p_credp, gid)) == NULL)
			set_user_error (EAGAIN);
		else
			SELF->p_credp = credp;
	} else {
		/* super () sets u_error when it fails */
		set_user_error (0);

		if (SELF->p_credp->cr_rgid == gid ||
		    SELF->p_credp->cr_sgid == gid) {
			if ((credp = cred_newegid (SELF->p_credp,
						   gid)) == NULL)
				set_user_error (EAGAIN);
			else
				SELF->p_credp = credp;
		} else
			SET_U_ERROR (EPERM, "Illegal gid");
	}
	return 0;
}


/*
 * Set user id.
 *
 * As in SVID issue 2:
 *
 * if effective uid is superuser
 *	set real, effective, and saved effective uid to argument "uid"
 * else if real uid is same as "uid"
 *	set effective uid to "uid"
 * else if saved effective uid is same as "uid"
 *	set effective uid to "uid"
 */

int
usetuid (uid)
o_uid_t uid;
{
	cred_t	      *	credp;

	if (super ()) {
		if ((credp = cred_newuid (SELF->p_credp, uid)) == NULL)
			set_user_error (EAGAIN);
		else
			SELF->p_credp = credp;
	} else {
		/* super () sets u_error when it fails */
		set_user_error (0);

		if (SELF->p_credp->cr_ruid == uid ||
		    SELF->p_credp->cr_suid == uid) {
			if ((credp = cred_neweuid (SELF->p_credp,
						   uid)) == NULL)
				set_user_error (EAGAIN);
			else
				SELF->p_credp = credp;
		} else
			SET_U_ERROR (EPERM, "Illegal uid");
	}
	return 0;
}


/*
 * Set time and date.
 *
 * Unlike the libc interface, this routine expects a time_t value
 * as an arg, not a time_t pointer.
 */

int
ustime (newtime)
time_t newtime;
{
	int s;

	if (super () == 0)
		return -1;

	s = sphi ();
	timer.t_time = newtime;
	spl (s);
	return 0;
}


/*
 * Return process times.
 */

int
utimes (tp)
struct tms * tp;
{
	struct tms tbuffer;

	if (tp != NULL) {
		tbuffer.tms_utime = SELF->p_utime;
		tbuffer.tms_stime = SELF->p_stime;
		tbuffer.tms_cutime = SELF->p_cutime;
		tbuffer.tms_cstime = SELF->p_cstime;
		kucopyS (& tbuffer, tp, sizeof (tbuffer));
	}
	return lbolt;
}


/*
 * Wait for a child to terminate. If this function is called from i286 code,
 * then we will have a "statp" argument, where we stash the exit status, but
 * for the i386 code we put a value in u.u_rval2.
 *
 * NIGEL: The idiom for looping over the process table must be changed to
 * work right ASAP.
 */

int
uwait (statp)
short	      *	statp;
{
	PROC * pp;
	PROC * ppp;
	PROC * cpp;
	int pid;

	/* Wait for a child to stop or die. */

	T_HAL (8, printf ("[%d]waits ", SELF->p_pid));
	ppp = SELF;

	for (;;) {
		/* Look at all processes. */
again:
		__GLOBAL_LOCK_PROCESS_TABLE ("uwait ()");
		cpp = NULL;
		pp = & procq;
		while ((pp = pp->p_nforw) != & procq) {

			/* Ignore the current process. */
			if (pp == ppp)
				continue;
			/*
			 * Ignore processes that aren't children of the
			 * current one.
			 */
			if (pp->p_ppid != ppp->p_pid ||
			    (pp->p_flags & PFSTOP) != 0)
				continue;

			/* Here is a child that hit a breakpoint. */
			if (pp->p_flags & PFWAIT) {
				pp->p_flags &= ~ PFWAIT;
				pp->p_flags |= PFSTOP;

				if (statp != NULL)
					putusd (statp, pp->p_exit);
				else
					u.u_rval2 = pp->p_exit;

				__GLOBAL_UNLOCK_PROCESS_TABLE ();
				T_HAL (8,
				       printf ("[%d]ends waiting, %d stopped ",
					       SELF->p_pid, pid));
				return pp->p_pid;
			}
			if (pp->p_state == PSDEAD) {
				ppp->p_cutime += pp->p_utime + pp->p_cutime;
				ppp->p_cstime += pp->p_stime + pp->p_cstime;

				if (statp != NULL)
					putusd (statp, pp->p_exit);
				else
					u.u_rval2 = pp->p_exit;
				pid = pp->p_pid;
				__GLOBAL_UNLOCK_PROCESS_TABLE ();
				relproc (pp);

				if ((proc_signal_misc (ppp) &
				     __SF_NOCLDWAIT) != 0)
					goto again;

				T_HAL (8, printf ("[%d]ends waiting, %d died ",
						  SELF->p_pid, pid));
				return pid;
			}
			cpp = pp;
		}
		__GLOBAL_UNLOCK_PROCESS_TABLE ();

		if (cpp == NULL) {
			T_HAL (8, printf ("[%d]ends waiting, no children ",
					  SELF->p_pid));
			set_user_error (ECHILD);
			return -1;
		}
		(void) x_sleep ((char *) ppp, prilo, slpriSigLjmp, "wait");
		/* Wait for a child to terminate.  */
	}
}


/*
 * waitpid () and wait () share the same system call number under BCS.
 *
 * pid argument:
 *	>  0	wait for child whose process matches pid
 *	=  0	wait for any child in current process group
 *	= -1	wait for any child - same as wait ()
 *	< -1	wait for any child in group given by -pid
 *
 * The only waitpid () options supported are WNOHANG and WUNTRACED.
 */

int
uwaitpid (opid, stat_loc, options)
pid_t opid;
int	* stat_loc, options;
{
	PROC * pp;
	PROC * ppp;
	PROC * cpp;
	int pid;

	if ((options & ~ (WUNTRACED | WNOHANG)) != 0) {
		cmn_err (CE_NOTE, "!waitpid (%d,0x%x,0x%x): unsupported", opid,
		  (unsigned) stat_loc, options);
		set_user_error (EINVAL);
		return -1;
	}

	/* Wait for a child to stop or die. */
	ppp = SELF;
	for (;;) {
		/* Look at all processes. */
again:
		__GLOBAL_LOCK_PROCESS_TABLE ("uwaitpid ()");
		cpp = NULL;
		pp = & procq;
		while ((pp = pp->p_nforw) != & procq) {

			/* Ignore the current process. */
			if (pp == ppp)
				continue;
			/*
			 * Ignore processes that aren't children of the
			 * current one.
			 */
			if (pp->p_ppid != ppp->p_pid ||
			    (pp->p_flags & PFSTOP) != 0)
				continue;

			/* If opid == 0 we want to match gids */
			/* If opid > 0, want to match opid to child pid */
			/* If opid <-1, want to match -opid to child gid */
			if ((opid == 0 && pp->p_group != ppp->p_group) ||
			    (opid > 0 && opid != pp->p_pid) ||
			    (opid < -1 && - opid != pp->p_group))
				continue;

			/* if opid == -1, then any child is acceptable */

			/* Here is an acceptable child that hit a breakpoint. */
			if (pp->p_flags & PFWAIT) {
				pp->p_flags &= ~ PFWAIT;
				pp->p_flags |= PFSTOP;
				u.u_rval2 = pp->p_exit;

				__GLOBAL_UNLOCK_PROCESS_TABLE ();
				return pp->p_pid;
			}

			/* Here is an acceptable child that is a zombie. */
			if (pp->p_state == PSDEAD) {
				ppp->p_cutime += pp->p_utime + pp->p_cutime;
				ppp->p_cstime += pp->p_stime + pp->p_cstime;
				u.u_rval2 = pp->p_exit;
				pid = pp->p_pid;
				__GLOBAL_UNLOCK_PROCESS_TABLE ();
				relproc (pp);

				if ((proc_signal_misc (ppp) &
				     __SF_NOCLDWAIT) != 0)
					goto again;

				return pid;
			}
			cpp = pp;
		}

		__GLOBAL_UNLOCK_PROCESS_TABLE ();

		if (cpp == NULL) {
			set_user_error (ECHILD);
			return -1;
		}

		if (options & WNOHANG) {
               		u.u_rval2 = 0;
			return 0;
		}

		/* Wait for a child to terminate. */
		(void) x_sleep ((char *) ppp, prilo, slpriSigLjmp,
				"waitpid");
	}
}


/*
 * Wait for a child to terminate.
 *
 * iBCS2 says the same system call number is wait () and waitpid (), the
 * distinction being in how the psw is set on entry.
 *
 * iBCS2 fails to mention that when wait () or waitpid () report status
 * by writing into the pointer supplied, the status is put into %edx by
 * the kernel, and moved from there into user space by the function in
 * libc.a.  uwait () and uwaitpid () specify a value for %edx by writing
 * to u.u_rval2.
 *
 * Do wait () unless (ZF | PF | SF | OF) (= WPMASK) are set in psw.
 */

#define	WPMASK	(__FLAG_MASK (1, __ZERO) | __FLAG_MASK (1, __PARITY) | \
		 __FLAG_MASK (1, __SIGN) | __FLAG_MASK (1, __OVERFLOW))

int
uwait386(arg1, arg2, arg3, regsetp)
unsigned long	arg1;
unsigned long	arg2;
unsigned long	arg3;
gregset_t     *	regsetp;
{
	return ((__flag_arith_t) __FLAG_REG (regsetp) & WPMASK) == WPMASK ?
			uwaitpid (arg1, arg2, arg3) : uwait (NULL);
}

