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

size_t haict_cache = HAICT_CACHE;
int haict_tdcbug = HAICT_TDCBUG;

/*End of file */
