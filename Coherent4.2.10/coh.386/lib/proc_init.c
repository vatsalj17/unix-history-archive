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

#include <common/ccompat.h>
#include <kernel/sig_lib.h>
#include <kernel/cred_lib.h>
#include <sys/types.h>
#include <sys/kmem.h>
#include <stdlib.h>

#include <kernel/proc_lib.h>

/*
 * These three pull in all the old-style trash.
 */

#define	_KERNEL	1

#include <sys/uproc.h>
#include <sys/proc.h>
#include <sys/sched.h>


/*
 * Initialize a freshly-allocated process structure to a copy of the current
 * process. As a special case, if there is no current process, the new process
 * is given some default values.
 */

#if	__USE_PROTO__
__proc_t * new_process_init (__proc_t * process)
#else
__proc_t *
new_process_init (process)
__proc_t      *	process;
#endif
{
	if (process == NULL &&
	    (process = kmem_alloc (sizeof (* process), KM_NOSLEEP)) == NULL)
		return NULL;

	process->p_event = NULL;	/* Wakeup event channel */
	process->p_alarm = 0;		/* Timer for alarms */

	process->p_utime = 0L;		/* User time (HZ) */
	process->p_stime = 0L;		/* System time */
	process->p_cutime = 0L;		/* Sum of childs user time */
	process->p_cstime = 0L;		/* Sum of childs system time */
	process->p_exit = 0;		/* Exit status */
	process->p_polls = NULL;	/* Enabled polls */

	/* Poll timer */
	process->p_polltim.t_next = NULL;
	process->p_polltim.t_last = NULL;
	process->p_polltim.t_lbolt = 0L;
	process->p_polltim.t_func = NULL;
	process->p_polltim.t_farg = NULL;

	/* Alarm timer */
	process->p_alrmtim.t_next = NULL;
	process->p_alrmtim.t_last = NULL;
	process->p_alrmtim.t_lbolt = 0L;
	process->p_alrmtim.t_func = NULL;
	process->p_alrmtim.t_farg = NULL;

	process->p_prl = NULL;		/* Pending record lock */
	process->p_semu = NULL;		/* Semaphore undo */

	process->p_flags = PFCORE;
	process->p_state = PSRUN;
	process->p_ttdev = NODEV;

	process->p_pid = 0;

	___SIGSET_SET (process->p_pending_signals, 0);

	if (SELF == NULL) {

		/* There is no current process. */

		process->p_ppid = 0;
		process->p_nice = 0;
		
		/* Scale starting priority and foodstamps */
		process->p_schedPri = NCRTICK * (100 / 2);
		process->p_foodstamp = 0;

		process->p_group = process->p_sid = 0;
		process->p_credp = cred_alloc ();
		process->p_comm [0] = 0;

		/*
		 * Set ttdev to null so that we do not accidentally set a tty
		 * for init.
		 */

		process->p_ttdev = makedev (0, 0);

		___SIGSET_SET (process->p_signal_mask, 0);
		___SIGSET_SET (process->p_queued_signals, 0);

		/*
		 * The default action for the following signals is to ignore
		 * them.
		 */

#if	_SIGNAL_MAX <= __LONG_BIT
		process->p_ignored_signals._sigbits [0] =
			__SIGSET_MASK (SIGCHLD) | __SIGSET_MASK (SIGPWR) |
			__SIGSET_MASK (SIGWINCH) | __SIGSET_MASK (SIGURG) |
			__SIGSET_MASK (SIGPOLL) | __SIGSET_MASK (SIGCONT);
#else
		___SIGSET_SET (process->p_ignored_signals, 0);
		__SIGSET_CLRBIT (process->p_ignored, SIGCHLD);
		__SIGSET_CLRBIT (process->p_ignored, SIGPWR);
		__SIGSET_CLRBIT (process->p_ignored, SIGWINCH);
		__SIGSET_CLRBIT (process->p_ignored, SIGURG);
		__SIGSET_CLRBIT (process->p_ignored, SIGPOLL);
		__SIGSET_CLRBIT (process->p_ignored, SIGCONT);
#endif

		process->p_sigflags = 0;
	} else {

		/* There is a current process.  "process" is its child. */

		process->p_ppid = SELF->p_pid;
		process->p_nice = SELF->p_nice;
		process->p_schedPri = SELF->p_schedPri;
		process->p_foodstamp = SELF->p_foodstamp;

		process->p_group = SELF->p_group;
		process->p_sid = SELF->p_sid;
		cred_ref (process->p_credp = SELF->p_credp);

		memcpy (process->p_comm, SELF->p_comm, sizeof (SELF->p_comm));

		process->p_ttdev = SELF->p_ttdev;

		process->p_signal_mask = SELF->p_signal_mask;
		process->p_ignored_signals = SELF->p_ignored_signals;
		process->p_queued_signals = SELF->p_queued_signals;

		process->p_sigflags = SELF->p_sigflags;

		process->p_sysconp = NULL;
		process->p_sleep = NULL;
	}

	return process;
}
