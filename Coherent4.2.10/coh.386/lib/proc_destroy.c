/* $Header: $ */
/*
 * This file contains definitions for the functions which support the Coherent
 * internal binary-compatibility scheme. We select _SYSV3 to get some old
 * definitions like makedev () visible.
 *
 * $Log: $
 */

#define	_SYSV3		1

#include <common/ccompat.h>
#include <kernel/cred_lib.h>
#include <sys/types.h>
#include <sys/debug.h>
#include <sys/kmem.h>
#include <stdlib.h>

#include <kernel/proc_lib.h>

/*
 * These two pull in all the old-style trash.
 */

#define	_KERNEL	1

#include <sys/uproc.h>
#include <sys/proc.h>


/*
 * Initialize a freshly-allocated process structure to a copy of the current
 * process. As a special case, if there is no current process, the new process
 * is given some default values.
 */

#if	__USE_PROTO__
void proc_destroy (__proc_t * process)
#else
void
proc_destroy (process)
__proc_t      *	process;
#endif
{
	ASSERT (process != NULL);

	cred_unref (process->p_credp);
	kmem_free (process, sizeof (* process));
}
