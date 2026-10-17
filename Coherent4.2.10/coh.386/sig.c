/* $Header: /ker/coh.386/RCS/sig.c,v 2.9 93/10/29 00:55:36 nigel Exp Locker: nigel $ */
/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or persuant to the license agreement is unlawful.
 *
 *	COHERENT Version 5.0
 *	Copyright (c) 1982, 1993.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
/*
 * $Log:	sig.c,v $
 * Revision 2.9  93/10/29  00:55:36  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.8  93/09/13  07:59:29  nigel
 * Changed new core-file format to accomodate putting the file name into the
 * dump for each segment, so now Steve can update 'db'.
 * 
 * Revision 2.7  93/09/02  18:08:05  nigel
 * Prepare for DDI/DKI merge
 * 
 * Revision 2.6  93/08/25  12:45:39  nigel
 * Update for new system of handling user flags register
 * 
 * Revision 2.5  93/08/19  03:26:46  nigel
 * Nigel's r83 (Stylistic cleanup)
 */

#include <common/__parith.h>
#include <common/_limits.h>
#include <common/_wait.h>
#include <common/_gregset.h>
#include <kernel/sig_lib.h>
#include <kernel/proc_lib.h>
#include <kernel/cred_lib.h>
#include <kernel/reg.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/cmn_err.h>
#include <sys/cred.h>
#include <stddef.h>

#define	_KERNEL		1

#include <kernel/trace.h>
#include <kernel/reg.h>
#include <sys/uproc.h>
#include <sys/mmu.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/io.h>
#include <sys/proc.h>
#include <sys/ptrace.h>
#include <sys/sched.h>
#include <sys/seg.h>
#include <sys/core.h>

int	ptset();
void	sendsig();

static struct _fpstate * empack();
static	int		ptret();

/*
 * Patchable variables.
 *
 * Patch DUMP_TEXT nonzero to make text segment show up in core files.
 * Patch DUMP_LIM set the upper limit in bytes of how much of a
 * segment is written to a core file.
 *
 * CATCH_SEGV can be patched to allow signal () to be used to catch the
 * SIGSEGV signal (). This is off by default because:
 *   i)	Experience has shown that kernel diagnostic messages being on by
 *	default is very useful for catching bugs.
 *  ii)	Certain extremely ill-behaved applications apparently catch SIGSEGV
 *	blindly as part of some catch-all behaviour, and when such faults
 *	happen have been known to loop generating kernel diagnostic output
 *	which bogs down the system unacceptably.
 * iii)	Turning the signal off to avoid ii) causes less actual disruption than
 *	leaving it on.
 * This only applies to the signal () interface; we assume that apps which are
 * smart enough to use sigset () or sigaction () also know what to do with
 * SIGSEGV.
 */

int		DUMP_TEXT = 0;
off_t		DUMP_LIM = 512 * 1024;
extern	unsigned char	CATCH_SEGV;

/*
 * Structure used for communication between parent and child.
 */

struct ptrace {
	int	 pt_req;		/* Request */
	int	 pt_pid;		/* Process id */
	caddr_t	 pt_addr;		/* Address */
	int	 pt_data;		/* Data */
	int	 pt_errs;		/* Error status */
	int	 pt_rval;		/* Return value */
	int	 pt_busy;		/* In use */
	__DUMB_GATE	__pt_gate;		/* Gate */
};

#define	__GLOBAL_LOCK_PTRACE(pt, where) \
		(__GATE_LOCK ((pt)->__pt_gate, "lock : ptrace " where))
#define	__GLOBAL_UNLOCK_PTRACE(pt) \
		(__GATE_UNLOCK ((pt)->__pt_gate))

struct	ptrace	pts;			/* Ptrace structure */


/*
 * Or an entire signal mask into the "hold" mask.
 */

static void
sigMask (mask)
__sigset_t * mask;
{
	int		i;
	__sigmask_t   *	loop = mask->_sigbits;
	__sigset_t	signal_mask;

	curr_signal_mask (NULL, & signal_mask);

	for (i = 0 ; i < __ARRAY_LENGTH (mask->_sigbits) ; i ++)
		signal_mask._sigbits [i] |= * loop ++;

	curr_signal_mask (& signal_mask, NULL);
}


/*
 * This function exists as a common internal method of generating a SIGCHLD.
 */

#if	__USE_PROTO__
void generate_sigchld (__proc_t * parent, __CONST__ __proc_t * child)
#else
void
generate_sigchld (parent, child)
__proc_t      *	parent;
__CONST__ __proc_t * child;
#endif
{
	__siginfo_t	sigchld;

	/*
	 * Send parent a notification of our demise.
	 */

	sigchld.__si_signo = SIGCHLD;
	sigchld.__si_errno = 0;
	if (__WIFEXITED (child->p_exit)) {
		sigchld.__si_code = __CLD_EXITED;
		sigchld.__si_status = __WEXITSTATUS (child->p_exit);
	} else {
		sigchld.__si_code = __WCOREDUMP (child->p_exit) ? __CLD_DUMPED :
								  __CLD_KILLED;
		sigchld.__si_status = __WTERMSIG (child->p_exit);
	}

	sigchld.__si_pid = child->p_pid;
	proc_send_signal (parent, & sigchld);
}


/*
 * Here is the iteration function used below to find a zombie child process.
 */

#if	__USE_PROTO__
__LOCAL__ int check_sigchld_iter (__proc_t * procp, __VOID__ * arg)
#else
__LOCAL__ int
check_sigchld_iter (procp, arg)
__proc_t      *	procp;
__VOID__      *	arg;
#endif
{
	/*
	 * If we get here the driver function has found a child process of the
	 * process passed in the extra argument. We check to see whether it is
	 * a zombie... if so, remember it and bail out. We don't actually send
	 * the signal in here because the process table is locked, we are good
	 * citizens and just bail out instead.
	 */

	if (procp->p_state == PSDEAD) {
		* ((__proc_t **) arg) = procp;
		return 1;		/* stop looking */
	}
	return 0;			/* keep looking */
}


/*
 * Here we scan the process table for zombie children of the current process
 * so that we can send an extra SIGCHLD when necessary.
 */

#if	__USE_PROTO__
void check_send_sigchld (__proc_t * parent)
#else
void
check_send_sigchld (parent)
__proc_t      *	parent;
#endif
{
	__proc_t      *	child = NULL;

	process_find_zombies (parent->p_pid, check_sigchld_iter, & child);

	if (child != NULL)
		generate_sigchld (parent, child);
}


/*
 * According to the SVR4 man-pages for signal () and sigset (), using either
 * of these calls to attach a signal-handling function prevents SIGCHLD from
 * being generated when a child process is stopped or continued. It is not
 * specified in the SVR4 Programmer's Reference Manual that using them in
 * other ways enables this; for now, this is a one-way deal.
 *
 * Note that for sigset (), the SIG_HOLD case has already been disposed of.
 */

#if	__USE_PROTO__
__LOCAL__ void suppress_job_sigchld (__CONST__ __sigaction_t * act)
#else
__LOCAL__ void
suppress_job_sigchld (act)
__CONST__ __sigaction_t * act;
#endif
{
	/*
	 * "If signal () or sigset () is used to set SIGCHLD's disposition to
	 * a signal handler, SIGCHLD will not be sent when the calling
	 * process's children are stopped or continued." Here, we set the
	 * __SF_NOCLDSTOP flag if the a handler is attached.
	 */

	if (act->sa_handler != SIG_IGN || act->sa_handler != SIG_DFL)
		curr_signal_misc (0, __SF_NOCLDSTOP);

	/*
	 * "If any of the above functions are used to set SIGCHLD's
	 * disposition to SIG_IGN, the calling process's children will not
	 * create zombie processes when they terminate." Here, we set the
	 * __SF_NOCLDWAIT flag if the address of the handler is SIG_IGN, and
	 * clear the flag otherwise.
	 */

	curr_signal_misc (__SF_NOCLDWAIT,
			  act->sa_handler == SIG_IGN ? __SF_NOCLDWAIT : 0);
}


/*
 * Set up the action to be taken on a signal.
 */

__sighand_t *
usigsys (signal, func, regsetp)
unsigned	signal;
__sighand_t   *	func;
gregset_t     *	regsetp;
{
	int		sigtype;
	__sigmask_t	mask;
	__sigset_t	signal_mask;
	__sigaction_t	signal_action;

	sigtype = signal & ~ 0xFF;
	signal &= 0xFF;

	/*
	 * Check the passed signal number. SIGKILL and SIGSTOP are not allowed
	 * to be caught.
	 */

	/* Range check on 1-based signal number. */
	if (signal <= 0 || signal > _SIGNAL_MAX || signal == SIGSTOP ||
	    signal == SIGKILL) {
		set_user_error (EINVAL);
		return (__sighand_t *) -1;
	}

	signal_action.sa_handler = func;
	signal_action.sa_flags = 0;
	___SIGSET_SET (signal_action.sa_mask, 0);
	func = 0;

	curr_signal_mask (NULL, & signal_mask);

	mask = __SIGSET_MASK (signal);

	switch (sigtype) {
	case SIGHOLD:
		__SIGSET_ADDMASK (signal_mask, signal, mask);
		break;

	case SIGRELSE:
		__SIGSET_CLRMASK (signal_mask, signal, mask);
		break;

	case SIGIGNORE:
		__SIGSET_CLRMASK (signal_mask, signal, mask);
		signal_action.sa_handler = SIG_IGN;
		curr_signal_action (signal, & signal_action, NULL);
		break;

	case 0:				/* signal () */
		if (signal == SIGSEGV && CATCH_SEGV == 0) {
			set_user_error (EINVAL);
			return 0;
		}
		u.u_sigreturn = (__sighand_t *) regsetp->_i386._edx;

		if (signal == SIGCHLD) {
			check_send_sigchld (SELF);
			suppress_job_sigchld (& signal_action);
		}

		signal_action.sa_flags |= __SA_RESETHAND;
		curr_signal_action (signal, & signal_action, & signal_action);

		/*
		 * Using the signal () interface automatically causes a
		 * pending signal to be discarded.
		 */

		proc_unkill (SELF, signal);
		return signal_action.sa_handler;		

	case SIGSET:
		u.u_sigreturn = (__sighand_t *) regsetp->_i386._edx;

		if (__SIGSET_TSTMASK (signal_mask, signal, mask))
			func = SIG_HOLD;

		if (signal_action.sa_handler == SIG_HOLD) {

			__SIGSET_ADDMASK (signal_mask, signal, mask);

			if (func != SIG_HOLD) {
				curr_signal_action (signal, NULL,
						    & signal_action);
				func = signal_action.sa_handler;
			}
			break;
		}


		if (signal == SIGCHLD)
			suppress_job_sigchld (& signal_action);

		__SIGSET_CLRMASK (signal_mask, signal, mask);
		__SIGSET_ADDMASK (signal_action.sa_mask, signal, mask);
		curr_signal_action (signal, & signal_action, & signal_action);

		if (func != SIG_HOLD)
			func = signal_action.sa_handler;
		break;

	case SIGPAUSE:
		/*
		 * Like upause(), do a sleep on an event which never gets a
		 * wakeup. The sleep returns immediately if a signal was
		 * already holding.
		 */

		if (signal == SIGCHLD)
			check_send_sigchld (SELF);

		__SIGSET_CLRMASK (signal_mask, signal, mask);
		curr_signal_mask (& signal_mask, NULL);
		(void) x_sleep ((char *) & u, prilo, slpriSigCatch,
				"sigpause");
		return 0;

	default:
		set_user_error (SIGSYS);
		return 0;
	}

	curr_signal_mask (& signal_mask, NULL);
	return func;
}


/*
 * Send a signal to the process `pp'. This function is only valid for use with
 * signals generated from user level with kill (), sigsend () or
 * sigsendset ().
 */

void
sendsig(sig, pp)
unsigned sig;
PROC *pp;
{
	__siginfo_t	siginfo;

#if 0
	T_HAL(8, if (sig == SIGINT) cmn_err (CE_CONT, "[%d]gets int ", pp->p_pid));
#else
	T_HAL(8, cmn_err (CE_CONT, "[%d] sig=%d ", pp->p_pid, sig));
#endif

	siginfo.__si_signo = sig;
	siginfo.__si_code = 0;
	siginfo.__si_errno = 0;
	siginfo.__si_pid = SELF->p_pid;
	siginfo.__si_uid = SELF->p_credp->cr_uid;

	proc_send_signal (pp, & siginfo);
}


/*
 * Return signal number if we have a non ignored or delayed signal, else zero.
 */

int
nondsig()
{
	return curr_signal_pending ();
}


/*
 * Create a dump of ourselves onto the file `core'.
 */

static int
sigdump (regsetp, signo)
gregset_t     *	regsetp;
int		signo;
{
	INODE *ip;
	SR *srp;
	SEG * sp;
	int n;
	struct {
		struct ch_info	chInfo;
		struct core_proc	cpInfo;
	} x;
	struct _fpstate * fpsp = empack ();
	IO		io;
	struct direct	dir;
	long		ssize;
	cred_t		fake_cred;

	if ((SELF->p_flags & PFNDMP) != 0)
		return 0;

	/* Make the core with the real owners */

	(void) cred_fake (& fake_cred, SELF->p_credp);

	io.io_flag = 0;
	if (ftoi ("core", 'c', IOSYS, & io, & dir, & fake_cred))
		return 0;

	if ((ip = u.u_cdiri) == NULL) {
		if ((ip = imake (IFREG | 0644, (o_dev_t) 0, & io, & dir,
				 & fake_cred)) == NULL)
			return 0;
	} else {
		if ((ip->i_mode & IFMT) != IFREG ||
		    iaccess (ip, IPW, & fake_cred) == 0 ||
		    getment (ip->i_dev, 1) == NULL) {
			idetach (ip);
			return 0;
		}
		itruncate (ip);
	}

	set_user_error (0);


	/*
	 * Write core file header
	 */

	x.chInfo.ch_magic = CORE_MAGIC;
	x.chInfo.ch_info_len = sizeof (x);

	/*
	 * Fill in the extra ersatz core-file stuff. Getting the FP state is
	 * ridiculously complex, so we wave the rubber chickens cut'n'pasted
	 * from elsewhere.
	 */

	x.cpInfo.cp_registers = * regsetp;
	x.cpInfo.cp_signal_number = signo;

	if (rdNdpUser ()) {
		if (! rdNdpSaved ()) {
			ndpSave (& u.u_ndpCon);
			wrNdpSaved (1);
		}
		x.cpInfo.cp_floating_point = u.u_ndpCon.fpstate;

		ndpRestore (& u.u_ndpCon);
		wrNdpSaved (0);
	} else if (fpsp)
		ukcopy (& x.cpInfo.cp_floating_point, fpsp, sizeof (* fpsp));
	else
		memset (& x.cpInfo.cp_floating_point, 0, sizeof (* fpsp));

	/*
	 * Debug stuff is not yet fully implemented.
	 */

	memset (& x.cpInfo.cp_debug_registers, 0,
		sizeof (x.cpInfo.cp_debug_registers));

	io.io_seek = 0;
	io.io_seg = IOSYS;
	io.io.vbase = (caddr_t) & x;
	io.io_ioc = sizeof (x);
	io.io_flag = 0;

	iwrite (ip, & io);

	/*
	 * Added to aid in kernel debugging - if DUMP_TEXT is nonzero,
	 * dump the text segment (to see if it was corrupted) and set
	 * the dump flag so that postmortem utilities will know that
	 * text is present in the core file.
	 */

	if (DUMP_TEXT)
		SELF->p_segl [SISTEXT].sr_flag |= SRFDUMP;

	for (srp = SELF->p_segl + 1 ;
	     get_user_error () == 0 && srp < SELF->p_segl + NUSEG ;
	     srp ++) {
	     	struct core_seg	csInfo;

	     	/*
	     	 * Segments that are not actually there are not written, since
	     	 * we cannot truly describe them.
	     	 */

	     	if ((sp = srp->sr_segp) == NULL)
	     		continue;

		/*
		 * Since writing out the whole U area is insane, we dump a
		 * segment header before each segment. Since Coherent 4.2
		 * has no support for mmap (), we gimmick up a name to include
		 * with the segment from the accounting info.
		 */

		memset (& csInfo, 0, sizeof (csInfo));

		if ((ssize = sp->s_size) > DUMP_LIM)
			ssize = DUMP_LIM;
		if ((srp->sr_flag & SRFDUMP) == 0)
			ssize = 0;

		csInfo.cs_pathlen = sizeof (SELF->p_comm);
		csInfo.cs_dumped = ssize;

		csInfo.cs_base = srp->sr_base;
		csInfo.cs_size = sp->s_size;

		io.io_seg = IOSYS;
		io.io.vbase = (caddr_t) & csInfo;
		io.io_ioc = sizeof (csInfo);
		io.io_flag = 0;

		iwrite (ip, & io);

		io.io_seg = IOSYS;
		io.io.vbase = SELF->p_comm;
		io.io_ioc = sizeof (SELF->p_comm);
		io.io_flag = 0;

		iwrite (ip, & io);

		io.io_seg = IOPHY;
		io.io.pbase = MAPIO (sp->s_vmem, 0);
		io.io_flag = 0;
		sp->s_lrefc ++;

		while (get_user_error () == 0 && ssize != 0) {
			n = ssize > SCHUNK ? SCHUNK : ssize;

			io.io_ioc = n;
			iwrite (ip, & io);

			/*
			 * This is already done by ioread (), so don't do it
			 * again.
			 * io.io.pbase += n;
			 */

			ssize -= n;
		}
		sp->s_lrefc --;
	}

	idetach (ip);

	return get_user_error () == 0;
}


/*
 * If we have a signal that isn't ignored, activate it. The register set is
 * not "const" because the low-level trap code that uses it wants to modify
 * it.
 */

#if	__USE_PROTO__
void curr_check_signals (gregset_t * regsetp, __sigset_t * oldmask)
#else
void
curr_check_signals (regsetp, oldmask)
gregset_t     *	regsetp;
__sigset_t    *	oldmask;
#endif
{
	register int signum;
	int ptval;

	ASSERT (regsetp != NULL);

	/*
	 * Fetch an unprocessed signal.
	 * Return if there are none.
	 * The while() structure is only for traced processes.
	 */

	while ((signum = curr_signal_pending ()) != 0) {
		__sigaction_t	signal_action;

		/*
		 * Reset the signal to indicate that it has been processed,
		 * and fetch the signal disposition.
		 */

got_signal:
		proc_unkill (SELF, signum);
		curr_signal_action (signum, NULL, & signal_action);

		/*
		 * If the signal is not defaulted, go run the requested
		 * function.
		 */

		if (signal_action.sa_handler != SIG_DFL) {
			if (__xmode_286 (regsetp))
				oldsigstart (signum, signal_action.sa_handler,
					     regsetp);
			else
				msigstart (signum, signal_action.sa_handler,
					   regsetp, oldmask);

	/*
	 * If the signal needs to be reset after delivery, do so. Note that
	 * all signal-related activity goes though the defined function
	 * interface; many subtle things may need to happen, so we let the
	 * layering take care of it.
	 */

			if ((signal_action.sa_flags & __SA_RESETHAND) != 0) {
				signal_action.sa_handler = SIG_DFL;
				curr_signal_action (signum, & signal_action,
						    NULL);
			} else
				sigMask (& signal_action.sa_mask);

			return;
		}

		/*
		 * When a traced process is signaled, it may need to exchange
		 * data with its parent (via ptret).
		 */

		if ((SELF->p_flags & PFTRAC) != 0) {
			/*
			 * Enter a special state, and write into out exit-
			 * status area a code for wait () or waitpid () to
			 * dig out.
			 */

			SELF->p_flags |= PFWAIT;
			SELF->p_exit = (signum << 8) | __WSTOPFLG;
			ptval = ptret (regsetp, signum);

			T_HAL(0x10000, cmn_err (CE_CONT, "ptret()=%x ", ptval));

			SELF->p_flags &= ~ (PFWAIT | PFSTOP);

			while ((signum = curr_signal_pending ()) != 0)
				proc_unkill (SELF, signum);

			if (ptval == 0)
				/* see if another signal came in */
				continue;

			if ((signum = ptval) != SIGKILL)
				goto got_signal;
		}

		/*
		 * Some signals cause a core file to be written.
		 */
		switch (signum) {
		case SIGQUIT:
		case SIGILL:
		case SIGTRAP:
		case SIGABRT:
		case SIGFPE:
		case SIGSEGV:
		case SIGSYS:
			if (sigdump (regsetp, signum))
				signum |= __WCOREFLG;
			break;
		}
		pexit (signum);
	}
}


/*
 * Send a ptrace command to the child.
 *
 * "pid" is child pid.
 */

int
ptset (req, pid, addr, data)
unsigned	req;
pid_t		pid;
char	      *	addr;
int		data;
{
	PROC *pp;

	__GLOBAL_LOCK_PROCESS_TABLE ("ptset ()");

	for (pp = procq.p_nforw ; pp != & procq ; pp = pp->p_nforw)
		if (pp->p_pid == pid)
			break;

	__GLOBAL_UNLOCK_PROCESS_TABLE ();

	if (pp == & procq || (pp->p_flags & PFSTOP) == 0 ||
	    pp->p_ppid != SELF->p_pid) {
		set_user_error (ESRCH);
		return -1;
	}

	__GLOBAL_LOCK_PTRACE (& pts, "ptset ()");
	pts.pt_req = req;
	pts.pt_pid = pid;
	pts.pt_addr = addr;
	pts.pt_data = data;
	pts.pt_errs = 0;
	pts.pt_rval = 0;
	pts.pt_busy = 1;

	wakeup ((char *) & pts.pt_req);

	while (pts.pt_busy) {
		x_sleep ((char *) & pts.pt_busy, primed, slpriSigCatch,
			 "ptrace");
		/* Send a ptrace command to the child.  */
	}

	set_user_error (pts.pt_errs);

	__GLOBAL_UNLOCK_PTRACE (& pts);
	return pts.pt_rval;
}


/*
 * This routine is called when a child that is being traced receives a signal
 * that is not caught or ignored.  It follows up on any requests by the parent
 * and returns when done.
 *
 * After ptrace handling done in this routine, a real or simulated signal
 * may need to be sent to the traced process.
 * Return a signal number to be sent to the child process, or 0 if none.
 */

static int
ptret (regsetp, signo)
gregset_t     *	regsetp;
int		signo;
{
	extern void (*ndpKfrstor)();
	PROC *pp;
	PROC *pp1;
	int sign;
	unsigned off;
	int doEmUnpack = 0;
	struct _fpstate * fstp = empack ();

	pp = SELF;
next:
	set_user_error (0);
	if (pp->p_ppid == 1)
		return SIGKILL;
	sign = -1;

	/* wake up parent if it is sleeping */

	__GLOBAL_LOCK_PROCESS_TABLE ("ptret ()");

	pp1 = & procq;
	for (;;) {
		if ((pp1 = pp1->p_nforw) == & procq) {
			sign = SIGKILL;
			break;
		}
		if (pp1->p_pid != pp->p_ppid)
			continue;
		if (ASLEEP (pp1))
			wakeup ((char *) pp1);
		break;
	}

	__GLOBAL_UNLOCK_PROCESS_TABLE ();

	while (sign < 0) {
		/* If no pending ptrace transaction for this process, sleep. */
		if (pts.pt_busy == 0 || pp->p_pid != pts.pt_pid) {
			/*
			 * If a signal bit is set now, just exit - let
			 * actvsig() handle it next time through.
			 * Doing sleep and goto next will stick us in a loop
			 */

			if (nondsig ())
				return 0;
			x_sleep ((char *) & pts.pt_req, primed,
				 slpriSigCatch, "ptret");
			goto next;
		}

		switch (pts.pt_req) {
		case PTRACE_RD_TXT:
			if (__xmode_286 (regsetp)) {
				pts.pt_rval = getuwd (NBPS + pts.pt_addr);
				break;
			}
			/* Fall through for 386 mode processes. */

		case PTRACE_RD_DAT:
			pts.pt_rval = getuwd (pts.pt_addr);
			break;

		case PTRACE_RD_USR:
			/* See ptrace.h for valid offsets. */
			off = (unsigned) pts.pt_addr;
			if (off & 3)
				set_user_error (EINVAL);
			else if (off < PTRACE_FP_CW) {
				/* Reading CPU general register state */
				if (off == PTRACE_SIG)
					pts.pt_rval = signo;
				else
					pts.pt_rval =
						((int *) regsetp) [off >> 2];
			} else if (off < PTRACE_DR0) {
				/*
				 * Reading NDP state.
				 * If NDP state not already saved, save it.
				 * Fetch desired info.
				 * Restore NDP state in case we will resume.
				 */
				if (rdNdpUser ()) {
					/* if using coprocessor */
					if (! rdNdpSaved ()) {
						ndpSave (& u.u_ndpCon);
						wrNdpSaved (1);
					}
pts.pt_rval = ((int *) & u.u_ndpCon) [(off - PTRACE_FP_CW) >> 2];
					ndpRestore (& u.u_ndpCon);
					wrNdpSaved (0);
				} else if (fstp) {
pts.pt_rval = getuwd(((int *) fstp) + ((off - PTRACE_FP_CW) >> 2));
					/* if emulating */
				} else { /* no ndp state to display */
					pts.pt_rval = 0;
					set_user_error (EFAULT);
				}
			} else /* Bad pseudo offset. */
				set_user_error (EINVAL);
			break;

		case PTRACE_WR_TXT:
			if (__xmode_286 (regsetp)) {
				putuwd (NBPS + pts.pt_addr, pts.pt_data);
				break;
			}
			/* Fall through for 386 mode processes. */

		case PTRACE_WR_DAT:
			putuwd (pts.pt_addr, pts.pt_data);
			break;

		case PTRACE_WR_USR:
			/* See ptrace.h for valid offsets. */
			off = (unsigned) pts.pt_addr;

			if (off & 3)
				set_user_error (EINVAL);
			else if (off < PTRACE_FP_CW) {
				/* Writing CPU general register state */
				if (off == PTRACE_SIG)
					set_user_error (EINVAL);
				else
					((int *) regsetp) [off >> 2] =
						pts.pt_data;
			} else if (off < PTRACE_DR0) {
				if (rdNdpUser ()) {
					/*
					 * Writing NDP state.
					 * If NDP state not already saved, save it.
					 * Store desired info.
					 * Restore NDP state in case we will resume.
					 */
					if (! rdNdpSaved ()) {
						ndpSave (& u.u_ndpCon);
						wrNdpSaved (1);
					}
((int *)&u.u_ndpCon)[(off - PTRACE_FP_CW)>>2] = pts.pt_data;
					ndpRestore (& u.u_ndpCon);
					wrNdpSaved (0);
				} else if (fstp && ndpKfrstor) {
putuwd(((int *)fstp) + ((off - PTRACE_FP_CW)>>2), pts.pt_data);
					doEmUnpack = 1;
				} else { /* No NDP state to modify. */
					set_user_error (EFAULT);
				}
			} else /* Bad pseudo offset. */
				set_user_error (EINVAL);
			break;

		case PTRACE_RESUME:
			__FLAG_REG (regsetp) =
				__FLAG_CLEAR_FLAG (__FLAG_REG (regsetp),
						   __TRAP);
			goto sig;

		case PTRACE_TERM:
			sign = SIGKILL;
			break;

		case PTRACE_SSTEP:
			__FLAG_REG (regsetp) =
				__FLAG_SET_FLAG (__FLAG_REG (regsetp),
						 __TRAP);

		sig:
			if (pts.pt_data < 0 || pts.pt_data > _SIGNAL_MAX) {
				set_user_error (EINVAL);
				break;
			}
			sign = pts.pt_data;
			break;

		default:
			set_user_error (EINVAL);
		}

		if ((pts.pt_errs = get_user_error ()) == EFAULT)
			pts.pt_errs = EINVAL;

		pts.pt_busy = 0;
		wakeup ((char *) & pts.pt_busy);
	}
	if (doEmUnpack)
		(* ndpKfrstor) (fstp, & u.u_ndpCon);
	return sign;
}


/*
 * If using floating point emulator, make room on user stack and save
 * floating point context there.  Code elsewhere takes care of floating
 * point context if there is a coprocessor.
 *
 * Return the virtual address in user space of the context area, or
 * return NULL if not using FP emulation.
 */

static struct _fpstate *
empack (regsetp)
gregset_t     *	regsetp;
{
	int		sphi;
	struct _fpstate * ret;
	SEG	      *	segp = SELF->p_segl [SISTACK].sr_segp;
	extern void (*ndpKfsave)();
	unsigned long sw_old;

	/* If not emulating, do nothing */
	if (rdNdpUser () || ! rdEmTrapped () || ! ndpKfsave)
		return NULL;

	/*
	 * Will copy at least u_sigreturn, _fpstackframe, and ndpFlags.
	 * If using ndp, need room for an _fpstate.
	 * If emulating, need room for an _fpemstate.
	 */

	ret = (struct _fpstate *)
		(__xmode_286 (regsetp) ? regsetp->_i286._usp :
					 regsetp->_i386._uesp) - 1;

	/* Add to user stack if necessary. */
	sphi = __xmode_286 (regsetp) ? ISP_286 : ISP_386;

	if (sphi - segp->s_size > (__ptr_arith_t) ret) {
		cseg_t	      *	pp;

		pp = c_extend (segp->s_vmem, btocru (segp->s_size));
		if (pp == 0) {
			cmn_err (CE_CONT, "Empack failed.  cmd=%s  c_extend(%x,%x)=0 ",
				 SELF->p_comm, (unsigned) segp->s_vmem,
				 btocru (segp->s_size));
			return NULL;
		}

		segp->s_vmem = pp;
		segp->s_size += NBPC;
		if (sproto (0) == 0) {
			cmn_err (CE_CONT, "Empack failed.  cmd=%s  sproto(0)=0 ",
				 SELF->p_comm);
			return NULL;
		}

		segload ();
	}

	(* ndpKfsave) (& u.u_ndpCon, ret);
	sw_old = getuwd (& ret->sw);
	putuwd (& ret->status, sw_old);
	putuwd (& ret->sw, sw_old & 0x7f00);

	return ret;
}

/*
 * NIGEL: cleanup for when a traced process exits; now a function to avoid the
 * export of the ptrace global shit.
 */

void
ptrace_cleanup ()
{
	wakeup ((char *) & pts.pt_req);
}

