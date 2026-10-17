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
 * Obtain and possibly change the current process's signal mask. Note that
 * we capture the current mask to a temporary before beginning, thus allowing
 * "mask" and "omask" to point to the same storage.
 */

#if	__USE_PROTO__
void curr_signal_mask (__CONST__ __sigset_t * mask, __sigset_t * omask)
#else
void
curr_signal_mask (mask, omask)
__CONST__ __sigset_t
	      *	mask;
__sigset_t    *	omask;
#endif
{
	ASSERT (omask != NULL || mask != NULL);

	if (mask != NULL) {
		__sigset_t	temp;

		/*
		 * The caller of this function should arrange for collateral
		 * actions such as checking for newly-unmasked signals in such
		 * as way as to ensure that signal actions are dealt with
		 * promptly and to ensure that locking is not needed.
		 *
		 * For instance, at this point signals may be briefly held
		 * which in fact "cannot" be. We don't care about this; the
		 * above constraint on our callers will ensure that this has
		 * no effect.
		 */

		temp = SELF->p_signal_mask;
		SELF->p_signal_mask = * mask;
		if (omask != NULL)
			* omask = temp;

		/*
		 * SIGKILL and SIGSTOP may never be blocked or caught. Here we
		 * enforce the blocking part, which as the SVR4 PRM says is
		 * "silently enforced" by the system.
		 */

		__SIGSET_CLRBIT (SELF->p_signal_mask, SIGKILL);
		__SIGSET_CLRBIT (SELF->p_signal_mask, SIGSTOP);
	} else
		* omask = SELF->p_signal_mask;
}

