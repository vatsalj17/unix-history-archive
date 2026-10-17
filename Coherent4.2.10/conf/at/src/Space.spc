/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1994 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */

/*
 * Configurable information for "at" (ST506-compatible) device driver.
 */

#define __KERNEL__	 1

#include <sys/hdioctl.h>
#include "conf.h"

#define _TAG(tag)

/*
 * ATSECS is number of seconds to wait for an expected interrupt.
 * ATSREG needs to be 3F6 for most new IDE drives;  needs to be
 *	1F7 for Perstor controllers and some old IDE drives.
 *	Either value works with most drives.
 * at_nsecmax is the maximum number of sectors in a multi-sector transfer.
 */

unsigned	ATSECS = ATSECS_SPEC;		/* Wait time any request */
unsigned	ATSREG = ATSREG_SPEC;		/* Reset/Control register */
int		at_nsecmax = ATNSECMAX_SPEC;	/* Maximum sectors to transfer */

/*
 * Drive parameters (translation mode, if used).
 * Arguments for the macro _HDPARMS are:
 *   cylinders, heads, sectors per track, control byte, write precomp cylinder.
 */
struct hdparm_s	atparm[N_ATDRV] = {
_TAG(AT0)	_HDPARMS(0,0,0,0,0),
_TAG(AT1)	_HDPARMS(0,0,0,0,0)
};
