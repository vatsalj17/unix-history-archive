/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1994 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */
/*
 * Configurable information for "haiss" Host adapters.
 */

#define __KERNEL__	 1

#include <sys/haiscsi.h>
#include "conf.h"

int haiss_type = HAISS_TYPE;	/* From the above enum */
int haiss_intr = HAISS_INTR;	/* IRQ number          */
int haiss_base = HAISS_BASE;	/* Paragraph addr for BIOS */
int haiss_slowdev = HAISS_SLOWDEV; /* Maks of device require handshake */

extern haft_t		ss_haft;
haft_t			* ss_haftp = & ss_haft;

/* End of file */
