/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1994 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */
/*
 * Configurable information for "hai154x" Host adapters.
 */

#define __KERNEL__	 1

#include <sys/haiscsi.h>
#include "conf.h"

/*
 * Adaptec 154x configuration (factory defaults shown).
 *
 * HAI154X_BASE = 0x330		Port Base.
 * HAI154X_INTR = 11		Host Adapter Interrupt vector.
 * HAI154X_DMA  = 5		DMA Channel.
 */

unsigned short	hai154x_base = HAI154X_BASE;
unsigned short	hai154x_intr = HAI154X_INTR;
unsigned short	hai154x_dma  = HAI154X_DMA;
unsigned short  hai154x_haid = HAI154X_HAID;

unsigned char	hai154x_xferspeed =	HAI154X_XFERSPEED;
unsigned char	hai154x_busofftime =	HAI154X_BUSOFFTIME;
unsigned char	hai154x_busontime =	HAI154X_BUSONTIME;

extern haft_t	_154x_haft;
haft_t		* _154x_haftp = & _154x_haft;

/* End of file */
