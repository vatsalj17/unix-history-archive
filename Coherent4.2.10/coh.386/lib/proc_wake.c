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

#include <kernel/proc_lib.h>

/*
 * These two pull in all the old-style trash.
 */

#define	_KERNEL	1

#include <sys/uproc.h>
#include <sys/proc.h>


/*
 * Wake a process from a sleep state, with the indicated "reason" code so that
 * the process can know whether sleep was interrupted or not.
 */

#if	__USE_PROTO__
void process_wake (__proc_t * process, int reason)
#else
void
process_wake (process, reason)
__proc_t      *	process;
int		reason;
#endif
{
	ASSERT (process != NULL);
	PROC_ASSERT_LOCKED (process);

	/*
	 * Dequeue the process from the hash list used for old-style sleep ().
	 */

	process->p_lback->p_lforw = process->p_lforw;
	process->p_lforw->p_lback = process->p_lback;

	/*
	 * Change the process state to runnable and queue the process on the
	 * appropriate run queue.
	 */

	process_set_runnable (process);
}
