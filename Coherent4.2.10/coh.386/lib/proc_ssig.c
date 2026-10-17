/* $Header: /ker/coh.386/RCS/ker_data.c,v 2.4 93/08/19 03:26:31 nigel Exp Locker: nigel $ */
/*
 * This file contains definitions for the functions which support the Coherent
 * internal binary-compatibility scheme. We select _SYSV3 to get some old
 * definitions like makedev () visible.
 *
 * $Log:	ker_data.c,v $
 * Revision 2.4  93/08/19  03:26:31  nigel
 * Nigel's r83 (Stylistic cleanup)
 * 
 * Revision 2.2  93/07/26  14:55:28  nigel
 * Nigel's R80
 */

#define	_SYSV3		1
#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1

#include <common/ccompat.h>
#include <kernel/sig_lib.h>
#include <kernel/proc_lib.h>
#include <sys/debug.h>
#include <sys/types.h>
#include <string.h>

/*
 * These two pull in all the old-style trash.
 */

#define	_KERNEL	1

#include <sys/uproc.h>
#include <sys/proc.h>


#if	_SIGNAL_MAX <= __LONG_BIT
/*
 * For optimizing handling of job-control signals.
 */

#define	STOP_SIGNALS	(__SIGSET_MASK (SIGSTOP) | __SIGSET_MASK (SIGTSTP) |\
			 __SIGSET_MASK (SIGTTIN) | __SIGSET_MASK (SIGTTOU))
#endif

/*
 * Add the given signal to the indicated process's pending-signal mask and if
 * the process requires some notification (such as being woken if asleep, or
 * interrupted if scheduled on another processor) then perform that.
 *
 * This function can only be called from base level, but we must be aware of
 * the fact that process state-changes can happen at interrupt level and that
 * we can be interacting with multiple processors.
 */

#if	__USE_PROTO__
void proc_send_signal (__proc_t * process, __CONST__ __siginfo_t * siginfo)
#else
void
proc_send_signal (process, siginfo)
__proc_t      *	process;
__CONST__ __siginfo_t
	      *	siginfo;
#endif
{
	__sigmask_t	mask;
	pl_t		prev_pl;

	ASSERT (process != NULL && siginfo != NULL);
	ASSERT (siginfo->__si_signo > 0 && siginfo->__si_signo <= _SIGNAL_MAX);

	mask = __SIGSET_MASK (siginfo->__si_signo);

	if (siginfo->__si_signo == SIGCONT) {
		/*
		 * When a SIGCONT is received, all pending stop signals are
		 * discarded and if the process is stopped, it is continued.
		 */

#if	_SIGNAL_MAX <= __LONG_BIT
		__SIGSET_CLRMASK (process->p_pending_signals, 0,
				  STOP_SIGNALS);
#else
		__SIGSET_CLRBIT (process->p_pending_signals, SIGSTOP);
		__SIGSET_CLRBIT (process->p_pending_signals, SIGTSTP);
		__SIGSET_CLRBIT (process->p_pending_signals, SIGTTIN);
		__SIGSET_CLRBIT (process->p_pending_signals, SIGTTOU);
#endif

#ifdef	PROC_IS_STOPPED
		if (PROC_IS_STOPPED (process))
			PROC_CONTINUE (process);
#endif
	}

#if	_SIGNAL_MAX <= __LONG_BIT
	if ((mask & STOP_SIGNALS) != 0) {
#else
	if (siginfo->__si_signo == SIGSTOP ||
	    siginfo->__si_signo == SIGTSTP ||
	    siginfo->__si_signo == SIGTTIN ||
	    siginfo->__si_signo == SIGTTOU) {
#endif
		/*
		 * When a stop signal is received, any pending SIGCONT signal
		 * is discarded.
		 */

		__SIGSET_CLRBIT (process->p_pending_signals, SIGCONT);
	}


#if	0
	if (__SIGSET_TSTMASK (process->p_queued_signals, siginfo->__si_signo,
			      mask) != 0) {
		/*
		 * Reliable signal-queueing is in effect, so we must stash
		 * away a copy of the structure pointed at by "siginfo" so we
		 * can deliver the information as an additional argument to
		 * the signal-catching function.
		 */
		ASSERT ("Not implemented" == NULL);
	}
#endif	/* NOT IMPLEMENTED */


	/*
	 * Here we are about to modify some shared per-process state; now is a
	 * good time to take a lock, so we structure the code so it can be
	 * released. We require a basic lock for the state-change code, so we
	 * use that lock to cover the process-level guard as well. In a uni-
	 * processor system, the process-level guard is not necessary, but the
	 * extra area covered is small enough that it doesn't matter.
	 */

	prev_pl = PROC_STATE_LOCK (process);

	__SIGSET_ADDMASK (process->p_pending_signals,
			  siginfo->__si_signo, mask);

	if (! __SIGSET_TSTMASK (process->p_signal_mask,
				siginfo->__si_signo, mask) &&
	    ! __SIGSET_TSTMASK (process->p_ignored_signals,
				siginfo->__si_signo, mask) &&
	    process->p_state == PSSLSIG) {
		/*
		 * The process is sleeping and wants to be woken by
		 * signals, arrange for it to come out of sleep so it
		 * can get to user-level and process the signal.
		 */

		process_wake_signal (process);
	}

	PROC_STATE_UNLOCK (process, prev_pl);
}

