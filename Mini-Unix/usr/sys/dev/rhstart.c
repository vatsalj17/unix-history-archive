#include "../param.h"
#include "../buf.h"

/*
 * startup routine for RH controllers.
 */
#define IENABLE 0100
#define	RHWCOM	060
#define	RHRCOM	070
#define GO	01

rhstart(bp, devloc, devblk, abae)
struct buf *bp;
int *devloc, *abae;
{
	register int *dp;
	register struct buf *rbp;
	register int com;

	dp = devloc;
	rbp = bp;
	*dp = devblk;			/* block address */
	*--dp = rbp->b_addr;		/* buffer address */
	*--dp = rbp->b_wcount;		/* word count */
	com = IENABLE | GO;
	if (rbp->b_flags&B_READ)	/* command */
		com =| RHRCOM; else
		com =| RHWCOM;
	*--dp = com;
}

