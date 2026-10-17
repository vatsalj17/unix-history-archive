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
 * Test the signal mask of the given process to determine whether the given
 * signal is masked.
 */

#if	__USE_PROTO__
int proc_signal_masked (__proc_t * process, int signal)
#else
int
proc_signal_masked (process, signal)
__proc_t      *	process;
int		signal;
#endif
{
	ASSERT (process != NULL && signal > 0 && signal <= _SIGNAL_MAX);
	return __SIGSET_TSTBIT (process->p_signal_mask, signal);
}

