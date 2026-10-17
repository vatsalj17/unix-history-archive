/* $Header: $ */
/*
 * This file contains definitions for the functions which support the Coherent
 * internal binary-compatibility scheme.
 *
 * $Log: $
 */

#include <common/ccompat.h>
#include <kernel/proc_lib.h>

#define	_KERNEL	1

#include <sys/proc.h>


/*
 * Simply deal with getting the current process ID
 */

#if	__USE_PROTO__
__pid_t current_pid (void)
#else
__pid_t
current_pid ()
#endif
{
	return SELF->p_pid;
}
