/*
 *	RX-11 Driver
 *	Peter Collinson May 1977
 *	Version 1
 *	Each UNIX block maps into 4 RX sectors, so a READ or
 *	WRITE operation has four actions upon it. The flag d_active
 *	is used to count these phases.
 *	The sectors are usually spaced 6 apart, to minimised access time. But
 *	they may be contiguous by specifying minor device number 8(010) or 9(011)
 *	Modified January 1977 
 *	defines ERRLOG
 *	keeps a global count of retrys per physical device
 *	stores contents of error register for first error
 */
#define ERRLOG 1

#include "../param.h"
#include "../buf.h"
#include "../conf.h"
#include "../user.h"

#define RXADDR	0177170
#define NRXBLK	499
#define	NPRX	2	/* Number of physical drives */

struct
{	int	rxcs;
	int	rxbuf;
};

#ifdef ERRLOG
struct
{	int	rx_retry[NPRX];
	int	rx_edrive;
	int	rx_esect;
	int	rx_etrk;
	int	rx_estat;	/* error status */
} rx_err;

#endif

/* Various RX interface values */

#define GO	01
#define FILL	0
#define EMPTY	02
#define WRITE	04
#define READ	06
#define DEV1	020
#define DONE	040
#define IENABLE	0100
#define TR	0200
#define INIT	040000
#define INITDONE	04


struct devtab	rxtab;

int rx_sector	0;			/* Current sector being dealt with */
int rx_track	0;			/* Current track */


rxopen(dev)
int	dev;
{
}

rxclose(dev)
{	bflush(dev);	}

/*
 * General strategy routine - deals with buffers and calls rxstart
 * if nothing happening
 */

rxstrategy(abp)
struct buf *abp;
{	register struct buf *bp;

	bp = abp;

#ifdef PDP1170
	/* 11/70 requires this call */
	if(bp->b_flags&B_PHYS)
		mapalloc(bp);
#endif
	/* Check for block no in range */
	if(bp->b_blkno > NRXBLK)
	{	if( !((bp->b_blkno == NRXBLK+1) && (bp->b_dev.d_minor&010)))
		{	bp->b_flags =| B_ERROR;
			iodone(bp);
			return;
		}
	}

	/* Link the block into lists */

	bp->av_forw = 0;
	spl5();
	if(rxtab.d_actf == 0)
		rxtab.d_actf = bp;
	else
		rxtab.d_actl->av_forw = bp;
	rxtab.d_actl = bp;
	if(rxtab.d_active == 0)
		rxstart();
	spl0();
}

rxstart()
{	register struct buf *bp;
	register int gblk;

	if((bp = rxtab.d_actf) == 0) return;
	rxtab.d_active++;

	/* Now calculate sector and track from the block number */
	gblk = (bp->b_blkno<<2);
	rx_track = gblk/26;
	rx_sector = (gblk%26) + 1;
	rxsect(bp);
}

/*
 * rxsect initiates I/O for a single sector 
 */

rxsect(abp)
struct buf *abp;
{	register struct buf *bp;

	bp = abp;

	if(bp->b_flags&B_READ) rxcom(bp, READ);
	else
	{	rxmov(bp);		/* Move the current 128 bytes into RX */
		rxcom(bp, WRITE);	/* and write it */
	}
}


/*
 * rxcom issues a write or read command to RX, only these commands are
 *	allowed to give an interrupt
 */

rxcom(abp, acom)
struct buf *abp;
int acom;
{	register struct buf *bp;
	register int com;

	bp = abp;
	com = acom;

	com =| GO|IENABLE;
	/* Check for Unit 1 */
	if((bp->b_dev.d_minor&01) == 1) com =| DEV1;

	RXADDR->rxcs = com;
	while((RXADDR->rxcs&TR) == 0);
	RXADDR->rxbuf = rx_sector & 037;
	while((RXADDR->rxcs&TR) == 0);
	RXADDR->rxbuf = rx_track & 0177;
}

/*
 * rxmov moves 128 bytes from/to the UNIX buffer to/from the RX
 * this is done at low priority and not using interrupts
 */

rxmov(abp)
struct buf *abp;
{	register struct buf *bp;
	register int ct;
	register char *ch;

	bp = abp;
	/* Set up FILL or EMPTY command */
	RXADDR->rxcs = GO|((bp->b_flags&B_READ)? EMPTY: FILL);
	/* Calculate start of character area
	 * = base address + (d_active-1) * 128
	 */
	spl0();
	ch = bp->b_addr +((rxtab.d_active-1)<<7);

	for(ct = 128; ct--; ch++)
	{	while((RXADDR->rxcs&TR) == 0);
		if(bp->b_flags&B_READ)
			*ch = RXADDR->rxbuf;
		else
			RXADDR->rxbuf = *ch;
	}

	/* Wait for done flag to ignore interrupts */
	while((RXADDR->rxcs&DONE) == 0);
	spl5();
}

/*
 * Interrupt routine
 */

rxintr()
{	register struct buf *bp;

	bp = rxtab.d_actf;

	if(RXADDR->rxcs < 0)		/* Test error bit */
	{
#ifdef ERRLOG
		rx_err.rx_edrive = (bp->b_dev.d_minor)&03;
		rx_err.rx_retry[rx_err.rx_edrive]++;
		if(rxtab.d_errcnt == 0)
		{	rx_err.rx_etrk = rx_track;
			rx_err.rx_esect = rx_sector;
			rx_err.rx_estat = RXADDR->rxbuf;	/* pick up error status */
		}
#endif
		RXADDR->rxcs = INIT;	/* Init the device to try again */
		while((RXADDR->rxbuf&INITDONE) == 0);
		if(rxtab.d_errcnt++ < 10)
		{	rxsect(bp);		/* try again */
			return;
		}

		/* No - get out */
		bp->b_flags =| B_ERROR;
		rxtab.d_active = 4;		/* to force exit */
	}

	rxtab.d_errcnt = 0;
	/* No find out what to do next */
	if(((bp->b_flags&B_ERROR) == 0) && (bp->b_flags&B_READ))
		rxmov(bp);

	if(rxtab.d_active++ != 4)
	{	rxinc(bp->b_dev.d_minor);
		rxsect(bp);
	}
	else
	{
		rxtab.d_active = 0;
		rxtab.d_actf = bp->av_forw;
		iodone(bp);
		rxstart();
	}

}


/*
 * rxinc increments the global sector and track numbers
 */

rxinc(minordev)
char minordev;
{
	register int ct;

	ct = (minordev&010) ? 1 : 5;

	spl0();
	while(ct--)
	{	if(rx_sector++ == 26)
		{	rx_sector = 1;
			if(rx_track++ == 76) 
			{	rx_track = 0;
				rx_sector = 3;	/* To avoid discontinuity */
			}
		}
	}
	spl5();
}
