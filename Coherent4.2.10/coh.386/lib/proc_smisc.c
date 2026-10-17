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
 * Return the given process's miscellaneous signal information. Note that we
 * don't try and deal with synchronization issues here, so whatever code tests
 * this data has to find some way of being multi-processor safe (concretely,
 * it means that the uses of the signal information here are not atomic at the
 * system-call level).
 */

#if	__USE_PROTO__
__sigmiscfl_t proc_signal_misc (__proc_t * process)
#else
__sigmiscfl_t
proc_signal_misc (process)
__proc_t      *	process;
#endif
{
	ASSERT (process != NULL);
	return process->p_sigflags;
}

