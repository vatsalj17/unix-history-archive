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

#define	_KERNEL	1

#include <sys/uproc.h>
#include <sys/proc.h>


/*
 * Remove a signal from the pending signal mask.
 */

#if	__USE_PROTO__
void proc_unkill (__proc_t * process, int signal)
#else
void
proc_unkill (process, signal)
__proc_t      *	process;
int		signal;
#endif
{
	ASSERT (process != NULL);
	ASSERT (signal > 0 && signal <= _SIGNAL_MAX);

	__SIGSET_CLRBIT (process->p_pending_signals, signal);
}

