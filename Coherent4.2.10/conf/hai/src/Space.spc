/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1994 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */
/*
 * Configurable information for "hai" (SCSI, tape support) device driver.
 */

#define __KERNEL__	1
#define _KERNEL		1

#include "conf.h"
#include <sys/haiscsi.h>
#include <sys/scsiwork.h>

/*
 * Patchable bit maps
 *   Bit n is 1 in HAI_DISK if there is a hard disk at SCSI ID n.
 *   Bit n is 1 in HAI_TAPE if there is a tape at SCSI ID n.
 */

int HAI_DISK = HAI_DISK_SPEC;
int HAI_TAPE = HAI_TAPE_SPEC;
int HAI_CDROM = HAI_CDROM_SPEC;

int HAI_HAINDEX = HAI_HAINDEX_SPEC;		/* Host adapter index */

/* enable the following, and disable matching part in cohmain/Space.c,
   when ss driver no longer ships. */

#if 0

#define _TAG(tag)

_drv_parm_t _sd_drv_parm[MAX_SCSI_ID] = {
_TAG(SD0)	{ 0, 0, 0},
_TAG(SD1)	{ 0, 0, 0},
_TAG(SD2)	{ 0, 0, 0},
_TAG(SD3)	{ 0, 0, 0},
_TAG(SD4)	{ 0, 0, 0},
_TAG(SD5)	{ 0, 0, 0},
_TAG(SD6)	{ 0, 0, 0},
_TAG(SD7)	{ 0, 0, 0}
};
#endif

/* End of file */
