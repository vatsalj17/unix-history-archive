/* $Header: $ */

#define	_DDI_DKI	1
#define	_SYSV4		1

/*
 * This file contains the implementation of the DDI/DKI function
 * drv_hztousec () and drv_usectohz ().
 *
 * $Log: $
 */

/*
 *-IMPORTS:
 *	<common/ccompat.h>
 *		__USE_PROTO__
 *		__ARGS ()
 *	<common/__clock.h>
 *		__clock_t
 *	<sys/debug.h>
 *		ASSERT ()
 *	<sys/types.h>
 *		ulong_t
 *	<stddef.h>
 *		NULL
 */

#include <common/ccompat.h>
#include <sys/debug.h>
#include <sys/types.h>
#include <stddef.h>

#include <sys/types.h>


#if	__COHERENT__

#include <kernel/param.h>

/*
 * Converting between ticks and microseconds accurately is a nightmare in the
 * general case. I've make the routine that does this system-specific so that
 * it's easy to do; for Coherent, there are 1000 microseconds per tick (in
 * theory), so we can just multiply and/or divide.
 */

#if	(1000000L % HZ) != 0
#error	These routines tuned for an even number of microseconds per tick
#endif

#define	U_PER_TICK	(1000000L / HZ)

#elif	__BORLANDC__ || defined (GNUDOS)

# include <time.h>

/*
 * Under MSDOS, just use the 18.2Hz minimum.
 */

#define	U_PER_TICK		54945

#endif


/*
 * Routines for time processing basic on microseconds per tick... these won't
 * work when the tick and microseconds times are too close or ticks are in
 * fact smaller than microseconds.
 */

/*
 *-STATUS:
 *	DDI/DKI
 *
 *-NAME:
 *	drv_hztousec	Convert clock ticks to microseconds.
 *
 *-SYNOPSIS:
 *	#include <sys/types.h>
 *	#include <sys/ddi.h>
 *
 *	clock_t	drv_hztousec (clock_t ticks);
 *
 *-ARGUMENTS:
 *	ticks		The number of clock ticks to convert to equivalent
 *			microseconds.
 *
 *-DESCRIPTION:
 *	drv_hztousec () converts the length of time expressed by "ticks",
 *	which is in units of clock ticks, into units of microseconds.
 *
 *	Several functions either take time values expressed in clock ticks as
 *	arguments [itimeout (), delay ()] or return time values expressed in
 *	clock ticks [drv_getparm ()]. The length of a clock tick can vary
 *	across different implementations and therefore drivers should not
 *	include any hard-coded assumptions about the length of a tick.
 *	drv_hztousec () and the complementary function drv_usectohz () can be
 *	used as necessary to convert between clock ticks and microseconds.
 *
 *-RETURN VALUE:
 *	The number of microseconds equivalent to the "ticks" argument. No
 *	error value is returned. If the microsecond equivalent to "ticks" is
 *	too large to be represented as a "clock_t", then the maximum "clock_t"
 *	value will be returned.
 *
 *-LEVEL:
 *	Base or interrupt.
 *
 *-NOTES:
 *	Does not sleep.
 *
 *	Driver-defined basic locks, read/write locks, and sleep locks may be
 *	held across calls to this function.
 *
 *	The time value returned by drv_getparm () with an "LBOLT" argument
 *	will frequently be too large to represent in microseconds as a
 *	"clock_t". When using drv_getparm () together with drv_hztousec () to
 *	time operations, drivers can help avoid overflow by converting the
 *	difference between return values from successive calls to
 *	drv_getparm () instead of trying to convert the return values
 *	themselves.
 *
 *-SEE ALSO:
 *	delay (), drv_getparm (), drv_usectohz (), dtimeout (), itimeout ()
 */

#if	__USE_PROTO__
__clock_t (drv_hztousec) (__clock_t ticks)
#else
__clock_t
drv_hztousec __ARGS ((ticks))
__clock_t	ticks;
#endif
{
#if	U_PER_TICK >= 1000
	ASSERT ((__clock_t) -1 > 0);

	if (ticks >= (__clock_t) -1 / U_PER_TICK)
		return (__clock_t) -1;

	return ticks * U_PER_TICK;
#else
# error
#endif
}


/*
 *-STATUS:
 *	DDI/DKI
 *
 *-NAME:
 *	drv_usectohz	Convert microseconds to clock ticks.
 *
 *-SYNOPSIS:
 *	#include <sys/types.h>
 *	#include <sys/ddi.h>
 *
 *	clock_t drv_usectohz (clock_t microsecs);
 *
 *-ARGUMENTS:
 *	microsecs	The number of microseconds to convert to equivalent
 *			clock ticks.
 *
 *-DESCRIPTION:
 *	drv_usectohz () converts the length of time expressed by "microsecs",
 *	which is in units of microseconds, into units of clock ticks.
 *
 *	Several functions either take time values expressed in clock ticks as
 *	arguments [itimeout (), delay ()] or return time values expressed in
 *	clock ticks [drv_getparm ()]. The length of a clock tick can vary
 *	across different implementations, and therefore drivers should not
 *	include any hard-coded assumptions about the length of a tick.
 *	drv_usectohz () and the complementary function drv_hztousec () can be
 *	used as necessary to convert between microseconds and clock ticks.
 *
 *-RETURN VALUE:
 *	The value returned is the smallest number of clock ticks that
 *	represent a time interval equal to or greater than the "microsecs"
 *	argument. No error value is returned. If the number of ticks
 *	equivalent to the "microsecs" argument is too large to be represented
 *	as a "clock_t", then the maximum "clock_t" value will be returned.
 *
 *-LEVEL:
 *	Base or interrupt.
 *
 *-NOTES:
 *	Does not sleep.
 *
 *	Driver-defined basic locks, read/write locks, and sleep locks may be
 *	held across calls to this function.
 *
 *-SEE ALSO:
 *	delay (), drv_getparm (), drv_hztousec (), dtimeout (), itimeout ()
 */

#if	__USE_PROTO__
__clock_t (drv_usectohz) (__clock_t microsecs)
#else
__clock_t
drv_usectohz __ARGS ((microsecs))
__clock_t	microsecs;
#endif
{
#if	U_PER_TICK >= 1000
	ulong_t		ticks = microsecs / U_PER_TICK;

	if ((microsecs % U_PER_TICK) != 0)
		ticks ++;

	return ticks;
#else
# error
#endif
}
