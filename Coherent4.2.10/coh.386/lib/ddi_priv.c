/* $Header: $ */

#define	_DDI_DKI	1
#define	_SYSV4		1

/*
 * This file contains the implementation of the DDI/DKI function drv_priv ().
 *
 * $Log: $
 */

#include <common/ccompat.h>
#include <sys/debug.h>
#include <sys/types.h>
#include <sys/cred.h>
#include <stddef.h>

#include <sys/types.h>

#if	__COHERENT__

int		super		__PROTO ((void));

#endif


/*
 *-STATUS:
 *	DDI/DKI
 *
 *-NAME:
 *	drv_priv	Determine whether credentials are priveleged.
 *
 *-SYNOPSIS:
 *	#include <sys/types.h>
 *	#include <sys/ddi.h>
 *
 *	int drv_priv (cred_t * crp);
 *
 *-ARGUMENTS:
 *	crp		Pointer to the user credential structure.
 *
 *-DESCRIPTION:
 *	The drv_priv () function determines whether the credentials specified
 *	by the credential structure pointer to by "crp" identify a priveleged
 *	process. This function should only be used when file access modes and
 *	special minor device numbers are insufficient to provide the necessary
 *	protection for the driver operation being performed. Calls to
 *	drv_priv () should replace all calls to suser () and any explicit
 *	checks for effective user ID equal to zero in driver code.
 *
 *	A credential structure pointer is passed into various driver entry
 *	point functions [open (), close (), read () and ioctl ()] and can also
 *	be obtained by calling drv_getparm () from base level driver code.
 *
 *-RETURN VALUE:
 *	This routine returns 0 if the specified credentials identify a
 *	priveleged process and EPERM otherwise.
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
 *	The only valid use for a credential structure pointer is an an
 *	argument to drv_priv (). The contents of a credential structure are
 *	not defined by the DDI/DKI and a driver may not examine the contents
 *	of the structure directly.
 *
 *-SEE ALSO:
 *	drv_getparm ()
 */

#if	__USE_PROTO__
int (drv_priv) (cred_t * credp)
#else
int
drv_priv __ARGS ((credp))
cred_t	      *	credp;
#endif
{
	ASSERT (credp != NULL);

	/*
	 * Note that we do not check for "credp->cr_ref > 0" since we can be
	 * passed the the fake credentials required for "access ()"-style
	 * permissions checking.
	 */

#if	__COHERENT__
	return credp->cr_uid == 0;
#else
	return 1;
#endif
}

