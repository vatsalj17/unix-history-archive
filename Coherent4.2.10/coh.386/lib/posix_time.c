/* $Header: $ */
/*
 * Return the POSIX-format current time (seconds since Jan 1, 1970 UTC).
 *
 * $Log: $
 */

#define	_DDI_DKI	1

#include <common/ccompat.h>
#include <common/_time.h>
#include <kernel/_timers.h>
#include <sys/inline.h>
#include <sys/types.h>


/*
 */

#if	__USE_PROTO__
__time_t posix_current_time (void)
#else
__time_t
posix_current_time ()
#endif
{
	pl_t		prev_pl = spltimeout ();
	__time_t	time = timer.t_time;
	splx (prev_pl);

	return time;
}

