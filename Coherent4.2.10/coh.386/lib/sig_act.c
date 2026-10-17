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

#include <common/ccompat.h>
#include <kernel/sig_lib.h>
#include <sys/debug.h>
#include <stddef.h>

/*
 * These two pull in all the old-style trash.
 */

#define	_KERNEL		1

#include <sys/uproc.h>
#include <sys/proc.h>


/*
 * Internal version of sigaction (), which allows retrieval and optional
 * replacement of the current signal information.
 *
 * Note that we take a copy of the current information in a temporary before
 * trying to fiddle the new information; this allows the "act" and "oact"
 * pointers to point to the same space.
 */

#if	__USE_PROTO__
void curr_signal_action (int signal, __CONST__ __sigaction_t * act,
			 __sigaction_t * oact)
#else
void
curr_signal_action (signal, act, oact)
int		signal;
__CONST__ __sigaction_t
	      *	act;
__sigaction_t *	oact;
#endif
{
	__sigaction_t *	proc_act;

	ASSERT (oact != NULL || act != NULL);
	ASSERT (signal > 0 && signal <= _SIGNAL_MAX);

	proc_act = & u.u_sigact [signal - 1];

	if (act != NULL) {
		__sigmask_t	mask = __SIGSET_MASK (signal);
		__sigaction_t	temp;

		/*
		 * Exchange the "action" data.
		 */

		temp = * proc_act;
		* proc_act = * act;
		if (oact != NULL)
			* oact = temp;

		/*
		 * The SIGILL, SIGTRAP and SIGPWR signals do not have their
		 * handlers reset to SIG_DFL even if the user requests this.
		 * Again, this is "silently enforced" so it happens at this
		 * layer.
		 */

#if	_SIGNAL_MAX <= __LONG_BIT
		if ((mask & (__SIGSET_MASK (SIGILL) | __SIGSET_MASK (SIGTRAP) |
			     __SIGSET_MASK (SIGPWR))) != 0)
#else
		if (signal == SIGILL || signal == SIGTRAP || signal == SIGPWR)
#endif
			proc_act->sa_flags &= ~ __SA_RESETHAND;

		/*
		 * Set the ignore flags to match handler. Note that we do not
		 * clear the pending signals for the current process here;
		 * that will happen on the way out of the system call we are
		 * in during the check for signals to deliver.
		 */

		if (proc_act->sa_handler == SIG_IGN)
			__SIGSET_ADDMASK (SELF->p_ignored_signals, signal,
					  mask);
		else
			__SIGSET_CLRMASK (SELF->p_ignored_signals, signal,
					  mask);

		/*
		 * Existing System V, Release 4 systems zero the mask if the
		 * signal is defaulted or ignored, so we copy them.
		 */

		if (proc_act->sa_handler == SIG_IGN ||
		    proc_act->sa_handler == SIG_DFL)
			___SIGSET_SET (proc_act->sa_mask, 0);

		/*
		 * The default action for the following signals is for them to
		 * be ignored: SIGCHLD, SIGPWR, SIGWINCH, SIGURG, SIGPOLL,
		 * SIGCONT.
		 */

#if	_SIGNAL_MAX <= __LONG_BIT
		if (((__SIGSET_MASK (SIGCHLD) | __SIGSET_MASK (SIGPWR) |
		      __SIGSET_MASK (SIGWINCH) | __SIGSET_MASK (SIGURG) |
		      __SIGSET_MASK (SIGPOLL) | __SIGSET_MASK (SIGCONT))
		     & mask) != 0
#else
		if ((signal == SIGCHLD || signal == SIGPWR ||
		     signal == SIGWINCH || signal == SIGURG ||
		     signal == SIGPOLL || signal == SIGCONT)
#endif
		    && proc_act->sa_handler == SIG_DFL)
			__SIGSET_ADDMASK (SELF->p_ignored_signals, signal,
					  mask);

		if ((proc_act->sa_flags & __SA_SIGINFO) != 0)
			__SIGSET_ADDMASK (SELF->p_queued_signals, signal,
					  mask);
	} else
		* oact = * proc_act;
}

