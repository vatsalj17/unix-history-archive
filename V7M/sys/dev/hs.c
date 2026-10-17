/*
 * RS03/04 disk driver
 *
 * This is a converted version six RS03/4 driver.
 * It has worked, but it has been modified slightly
 * and those modifications have NOT been tested !
 * The changes were to improve the block number overflow
 * test in hsstrategy and to use rh70hs in place of cputype.
 *
 * Fred Canter 8/8/81
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"


struct device {
	int	hscs1;	/* Control and Status register 1 */
	int	hswc;	/* Word count register */
	int	hsba;	/* UNIBUS address register */
	int	hsda;	/* Desired address register */
	int	hscs2;	/* Control and Status register 2 */
	int	hsds;	/* Drive Status */
	int	hser;	/* Error register */
	int	hsas;	/* not used */
	int	hsla;	/* not used */
	int	hsdb;	/* not used */
	int	hsmr;	/* not used */
	int	hsdt;	/* not used */
	int	hsbae;	/* 11/70 bus extension */
};

#define	HSADDR	((struct device *) 0172040)

struct	buf	hstab;
struct	buf	rhsbuf;


#define ERR	040000	/* hscs1 - composite error */

#define GO	01
#define RCLR	010
#define	DRY	0200	/* hsds - Drive Ready */
#define	WCOM	060
#define	RCOM	070
#define	IENABLE	0100

hsstrategy(bp)
register struct buf *bp;
{
	register int mblks;
	long sz;


	if((bp->b_flags&B_PHYS) && !rh70hs)
		mapalloc(bp);
	sz = bp->b_bcount;
	sz = (sz+511) >> 9;
	mblks = 1024; /* RJS03 */
	if(minor(bp->b_dev) >= 8)
		mblks = 2048; /* RJS04 */
	if((bp->b_blkno < 0) || ((bp->b_blkno + sz) > mblks)) {
		u.u_error = ENXIO;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
	bp->av_forw = 0;
	spl5();
	if (hstab.b_actf==0)
		hstab.b_actf = bp; else
		hstab.b_actl->av_forw = bp;
	hstab.b_actl = bp;
	if (hstab.b_active==0)
		hsstart();
	spl0();
}

hsstart()
{
	register struct buf *bp;
	register addr;
	register unit;

	if ((bp = hstab.b_actf) == 0)
		return;
	hstab.b_active++;
	addr = bp->b_blkno;
	if(minor(bp->b_dev) < 8)
		addr <<= 1; /* RJS03 */
	HSADDR->hscs2 = minor(bp->b_dev) & 07;
	HSADDR->hsda = addr << 1;
	HSADDR->hsba = bp->b_un.b_addr;

	if(rh70hs)
		HSADDR->hsbae = bp->b_xmem;
	HSADDR->hswc = -(bp->b_bcount>>1);
	unit = ((bp->b_xmem&3) << 8) | IENABLE | GO;
	if(bp->b_flags & B_READ)
		unit |= RCOM; else
		unit |= WCOM;
	HSADDR->hscs1 = unit;
}

hsintr()
{
	register struct buf *bp;

	if (hstab.b_active == 0)
		return;
	bp = hstab.b_actf;
	hstab.b_active = 0;
	if(HSADDR->hscs1 & ERR){	/* error bit */
		deverror(bp, HSADDR->hscs2, HSADDR->hser);
		HSADDR->hscs1 = RCLR|GO;
		if (++hstab.b_errcnt <= 10) {
			hsstart();
			return;
		}
		bp->b_flags |= B_ERROR;
	}
	hstab.b_errcnt = 0;
	hstab.b_actf = bp->av_forw;
	iodone(bp);
	hsstart();
}

hsread(dev)
{

	physio(hsstrategy, &rhsbuf, dev, B_READ);
}

hswrite(dev)
{

	physio(hsstrategy, &rhsbuf, dev, B_WRITE);
}
