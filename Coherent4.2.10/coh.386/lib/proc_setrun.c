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
#include <sys/types.h>
#include <stdlib.h>

#include <kernel/proc_lib.h>

/*
 * These two pull in all the old-style trash.
 */

#define	_KERNEL	1

#include <sys/uproc.h>
#include <sys/proc.h>


extern	__proc_t      *	iprocp;


/*
 * Add a process to the run queue, just forward of iprocp.
 */

#if	__USE_PROTO__
void process_set_runnable (__proc_t * procp)
#else
void
process_set_runnable (procp)
__proc_t      *	procp;
#endif
{
	PROC_ASSERT_LOCKED (procp);

	procp->p_lback = iprocp;
	procp->p_lforw = iprocp->p_lforw;
	procp->p_lback->p_lforw = procp;
	procp->p_lforw->p_lback = procp;
	procp->p_state = PSRUN;
}
