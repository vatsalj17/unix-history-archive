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
 * Return a zero value if there are no signals pending, otherwise return the
 * number of the first unmasked, non-ignored signal.
 *
 * NOTE: The code here interacts with curr_signal_action () and
 * proc_send_signal () in some ways that need comment; specifically, the
 * treatment of ignored signals may seem sloppy. However, the System V usage
 * of allowing ignored signals to be blocked, and then caught if a handler is
 * subsequently attached means that the approach of clearing pending, ignored
 * signals at this point is much simpler than dealing with them at all the
 * other points (arrival, unblocking, handler attach).
 */

#if	__USE_PROTO__
int curr_signal_pending (void)
#else
int
curr_signal_pending ()
#endif
{
	int		signo;

	/*
	 * If any signals have arrived, but which are not held, figure out
	 * what they are. This takes a little doing.
	 */

	for (signo = 0 ;
	     signo < __ARRAY_LENGTH (SELF->p_pending_signals._sigbits) ;
	     signo ++) {
		__sigmask_t	mask;

		mask = (SELF->p_pending_signals._sigbits [signo] &=
			~ SELF->p_ignored_signals._sigbits [signo]) &
				~ SELF->p_signal_mask._sigbits [signo];
		if (mask == 0)
			continue;

		/*
		 * There is at least one signal.  Extract its number
		 * from the signal bits.
		 */

		return signo * sizeof (__sigmask_t) * __CHAR_BIT +
				__SIGSET_FIRSTBIT (mask);
	}
	return 0;
}

