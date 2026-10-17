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

/*
 * These two pull in all the old-style trash.
 */

#define	_KERNEL		1

#include <sys/uproc.h>
#include <sys/proc.h>


/*
 * Obtain and/or modify the current process's miscellaneous signal-flags. The
 * previous flags are returned, making "curr_signal_misc (0, 0)" the fetch.
 */

#if	__USE_PROTO__
__sigmiscfl_t curr_signal_misc (__sigmiscfl_t clear, __sigmiscfl_t set)
#else
__sigmiscfl_t
curr_signal_misc (clear, set)
__sigmiscfl_t	clear;
__sigmiscfl_t	set;
#endif
{
	__sigmiscfl_t	temp = SELF->p_sigflags;

	SELF->p_sigflags = (temp & ~ clear) | set;
	return temp;
}

