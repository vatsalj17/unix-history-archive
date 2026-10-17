/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1994 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */
/*
 * Configurable information for "haicmn" (SCSI, tape support) device driver.
 */

#define __KERNEL__	 1

#include <sys/haiscsi.h>
#include "conf.h"

/*
 * Patchable bit maps
 *   Bit n is 1 in HAI_DISK if there is a hard disk at SCSI ID n.
 *   Bit n is 1 in HAI_TAPE if there is a tape at SCSI ID n.
 */

size_t haisd_maxreq = HAISD_MAXREQ;

/*End of file */
