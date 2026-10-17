/* $Header: $ */

#define	_DDI_DKI	1
#define	_SYSV4		1

/*
 * This file contains the implementation of the DDI/DKI function
 * drv_getparm ().
 *
 * $Log: $
 */

#include <common/ccompat.h>
#include <sys/debug.h>
#include <sys/types.h>
#include <sys/cred.h>
#include <stddef.h>
#include <limits.h>

#include <sys/types.h>


#if	__COHERENT__

#include <kernel/ddi_base.h>
#include <kernel/param.h>

#define	_KERNEL		1

#include <kernel/_timers.h>
#include <sys/proc.h>

int		super		__PROTO ((void));

#define	PRIVELEGED(credp)	(super ())
#define	SYSTEM_LBOLT()		(lbolt)
#define	SYSTEM_UPROCP()		(SELF)
#define	SYSTEM_UCRED()		(SELF->p_credp)

#elif	__BORLANDC__ || defined (GNUDOS)

__LOCAL__ __clock_t	_lbolt;
__LOCAL__ cred_t	_cred = {
	1, 0, 0, 0, 0, 0, NULL
};

#define	PRIVELEGED(credp)	0
#define	SYSTEM_LBOLT()		(_lbolt ++)
#define	SYSTEM_UPROCP()		(1)
#define	SYSTEM_UCRED()		(& _cred)

#endif


/*
 *-STATUS:
 *	DDI/DKI
 *
 *-NAME:
 *	drv_getparm	Retrieve kernel state information.
 *
 *-SYNOPSIS:
 *	#include <sys/types.h>
 *	#include <sys/ddi.h>
 *
 *	int drv_getparm (ulong_t parm, ulong_t * value_p);
 *
 *-ARGUMENTS:
 *	parm		The kernel parameter to be obtained. Possible values
 *			are:
 *			LBOLT	Read the number of clock ticks since the last
 *				kernel reboot. The difference between the
 *				values returned from successive calls to
 *				retrieve this parameter provides an indication
 *				of the elapsed time between the calls in units
 *				of clock ticks. The length of a clock tick can
 *				vary across different implementations, and
 *				therefore drivers should not include any hard-
 *				coded assumptions about the length of a tick.
 *				The drv_hztousec () and drv_usectohz ()
 *				functions can be used to convert between clock
 *				ticks and microseconds.
 *
 *			UPROCP	Retrieve a pointer to the process structure
 *				for the current process. The value returned in
 *				"* value_p" is of type "(proc_t *)" and the
 *				only valid use of the value is as an argument
 *				to vtop (). Since this value is associated
 *				with the current process, the caller must have
 *				process context (that is, must be at base
 *				level) when attempting to retrieve this value.
 *				Also, this value should only be used in the
 *				context of the process in which it was
 *				retrieved.
 *
 *			UCRED	Retrieve a pointer to the credential structure
 *				describing the current user credentials for
 *				the current process. The value returned in
 *				"* value_p" is of type "(cred_t *)" and the
 *				only valid use of the value is an an argument
 *				to drv_priv (). Since this value is associated
 *				with the current process, the caller must have
 *				process context (ie, must be at base level)
 *				when attempting to retrieve this value. Also,
 *				this value should only be used in the context
 *				of the process in which it was retrieved.
 *
 *			TIME	Read the time in seconds. This is the same
 *				time value that is returned by the time ()
 *				system call. The value is defined as the time
 *				in seconds since "00:00:00 GMT, January 1,
 *				1970". This definition presupposes that the
 *				administrator has set the correct system time
 *				and date.
 *
 *	value_p		A pointer to the data space into which the value of
 *			the parameter is to be copied.
 *
 *-DESCRIPTION:
 *	drv_getparm () returns the value of the parameter specified by "parm"
 *	in the location pointed to by "value_p".
 *
 *	drv_getparm () does not explicitly check to see whether the driver has
 *	the appropriate context when the function is called. It is the
 *	responsibility of the driver to use this function only when it is
 *	appropriate to do so and to correctly declare the data space needed.
 *
 *-RETURN VALUE:
 *	If the function is successful, 0 is returned. Otherwise, -1 is
 *	returned to indicate that "parm" specified an invalid parameter.
 *
 *-LEVEL:
 *	Base only when using the UPROCP or UCRED argument values.
 *
 *	Base or interrupt when using the LBOLT or TIME argument values.
 *
 *-NOTES:
 *	Does not sleep.
 *
 *	Driver-defined basic locks, read/write locks, and sleep locks may be
 *	held across calls to this function.
 *
 *-SEE ALSO:
 *	drv_hztousec (), drv_priv (), drv_usectohz (), vtop ()
 */

#if	__USE_PROTO__
int (drv_getparm) (ulong_t parm, ulong_t * value_p)
#else
int
drv_getparm __ARGS ((parm, value_p))
ulong_t		parm;
ulong_t	      *	value_p;
#endif
{
	ASSERT (value_p != NULL);

	switch (parm) {

	case LBOLT:
		* value_p = SYSTEM_LBOLT ();
		break;

	case UPROCP:
		* value_p = (ulong_t) SYSTEM_UPROCP ();
		break;

	case UCRED:
		* value_p = (ulong_t) SYSTEM_UCRED ();
		break;

	case TIME:
		* value_p = posix_current_time ();
		break;

	default:
		return -1;
	}

	return 0;
}
