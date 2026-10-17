#

/*
 * 10-MB cartridge disk driver: Multi-disc version
 *	- overlapped seeks
 *	- seeks ordered by 'FSCAN' algorithm
 */

#include "param.h"
#include "buf.h"
#include "conf.h"
#include "selch.h"

/* Configuration data */

#define NSELCH		0	/* attached to first selector channel */
#define NDRV		1	/* number of disk drives */
#define NDSK		2	/* number of disks */
#define NDSKBLK		9792	/* number of blocks on disk */
#define NDSKERR		10	/* number of error retries */

int	cntladdr	0xb6;		/* disk controller address */
char	dskaddr[NDSK] {			/* disk file addresses */
	0xc6,
	0xc7
};
#define drivemap(dev)	&dsk[dev.d_minor>>1]	/* map minor dev => drive */
#define devmap(addr)	&dsk[(addr-0xc6)>>4]	/* map phys addr => drive */

/* Data areas */

int dskstart(), dskscintr(), dsknrdy();

struct buf	rdskbuf;	/* raw I/O buffer */

struct devtab	dsktab;	/* major device table (per controller)
				 * d_active = controller busy flag
				 * d_actf = ptr to dsk table for last
				 *	accessed drive
				 */

struct dsk {		/* disk table (per drive) */
	char	d_active;	/* drive busy flag */
	char	d_errcnt;	/* error count (for recovery) */
	int	dk_cyl;		/* current cylinder position */
	int	dk_dir;		/* current direction of arm motion */
	struct buf *av_forw;	/* head of I/O queue for next pass */
	struct buf *dk_actf;	/* head of I/O queue for this pass */
} dsk[NDRV];
#define DSEEK	1
#define DIO	2
#define DNRDY	3

struct selchq dskscq {	/* channel queue entry */
	&dskstart,
	&dskscintr,
	0
};

/* Redefined buffer fields */

#define b_cyl		av_back.integ
#define b_sector	b_resid.integ

/* Disk file status & commands */

#define UNRCV		0x62	/* wrt chk|ill addr|seek inc */
#define ADDR_INTLK	0x10
#define WRT_PROT	0x80
#define NOT_READY	0x01

#define SEEK		0x42

/* Disk controller status & commands */

#define CNTL_UNRCV	0xe1	/* overrun|addr comp fail|def trk|data err */
#define OVERRUN		0x80
#define CYL_OV		0x10

#define READ		0x01
#define WRITE		0x02

/*
 * Disk strategy routine
 *	- check block no. & translate to cylinder/head/sector
 *	- put buffer on queue for corresponding drive
 *	- if controller free, call dskstart() to start I/O
 */

dskstrategy(bp)
register struct buf	*bp;
{
	register struct dsk	*dp;
	register struct buf	*p1, *p2;

trace(1<<16, "dstrategy", bp);

	if (bp->b_dev.d_minor >= NDSK) {
		bp->b_flags |= B_ERROR;
		bp->b_resid = bp->b_bcount;
		iodone(bp);
		return;
	}

	/*
	 * Block no. too high -- looks like EOF for raw read, error otherwise
	 */
	if (bp->b_blkno >= NDSKBLK) {
		if ((bp->b_flags&(B_PHYS|B_READ)) != (B_PHYS|B_READ))
			bp->b_flags |= B_ERROR;
		bp->b_resid = bp->b_bcount;
		iodone(bp);
		return;
	}

	/*
	 * Calculate cylinder/head/sector
	 */
	bp->b_cyl = bp->b_blkno / 24;
	if ((bp->b_sector = (bp->b_blkno<<1) % 48) >= 24)
		bp->b_sector += 32-24;

	/*
	 * Put buffer on queue for drive
	 */
	dp = drivemap(bp->b_dev);
	spl(5);
		for (p1 = dp; (p2 = p1->av_forw) &&
			(dp->dk_dir? (p2->b_cyl <= bp->b_cyl)
			    : (p2->b_cyl >= bp->b_cyl) ); )
			p1 = p2;
		bp->av_forw = p2;
		p1->av_forw = bp;

		/*
		 * Start I/O
		 */
		if (!(dp->d_active || dsktab.d_active))
			selchreq(NSELCH, &dskscq);
	spl(0);
}

/*
 * Disk startup routine
 *	- start overlapped seeks on all drives
 *	- check status of finished seeks
 *	- start I/O transfer on 'next' drive (round robin)
 */

dskstart()
{
	register struct dsk	*dp, *idp;
	register struct buf	*bp;
	register int		file, stat;

	idp = 0;
	if ((dp = dsktab.d_actf) == 0)
		dsktab.d_actf = dp = dsk;

trace(1<<16, "dstart", dp);
	do {		/* for each drive */
	/*
	 * Start overlapped seeks on all drives
	 */
		if (++dp >= &dsk[NDRV])
			dp = dsk;

		while (!dp->d_active) {
			if (!(bp = dp->dk_actf)) {
				if (!(bp = dp->av_forw))
					break;
				else {
					dp->dk_actf = bp;
					dp->av_forw = 0;
					dp->dk_dir = ~dp->dk_dir;
				}
			}
			file = dskaddr[bp->b_dev.d_minor];
			while ((stat = ss(file)) & ADDR_INTLK)
				;
			if (bp->b_cyl == dp->dk_cyl) {	/* seek done */
				if (idp)
					break;
				/* check status from seek */
				if (stat & ~WRT_PROT) {
					dskerror(dp, stat, file);
					dp->dk_cyl = -1;	/* force seek */
					continue;
				}
				idp = dp;
				break;
			}
			/* seek required */
			if (stat & NOT_READY) {
				dp->d_active = DNRDY;
				timeout(&dsknrdy, dp, 1000);
				break;
			}
			if (stat & UNRCV) {
				dp->d_errcnt = NDSKERR;	/* no retry */
				dskerror(dp, stat, file);
				continue;
			}
			dp->d_active = DSEEK;
			wh(file, bp->b_cyl);
			oc(file, SEEK);
trace(2<<16, "seek", file);
trace(2<<16, "cyl", bp->b_cyl);
			break;
		}
	} while (dp != dsktab.d_actf);

	/*
	 * Start next I/O transfer
	 */
	if (!idp)	/* no I/O to start */
		selchfree(NSELCH);
	else {
		dsktab.d_actf = idp;
		bp = idp->dk_actf;
		file = dskaddr[bp->b_dev.d_minor];
		while (ss(file) & ADDR_INTLK)
			;
		oc(selchaddr[NSELCH], STOP);
		wdh(selchaddr[NSELCH], bp->b_addr);
		wdh(selchaddr[NSELCH], bp->b_addr+bp->b_bcount-1);
		wh(file, bp->b_cyl);
		wd(cntladdr, bp->b_sector);
		if (bp->b_flags & B_READ) {
			oc(cntladdr, READ);
			oc(selchaddr[NSELCH], READ_GO);
		} else {
			oc(cntladdr, WRITE);
			oc(selchaddr[NSELCH], GO);
		}
trace(2<<16, "dio", file);
trace(2<<16, "sector", bp->b_sector);
		idp->d_active = DIO;
		dsktab.d_active++;
	}
}

/*
 * Disk not ready routine:
 *	- restart disk every 10 seconds until it comes ready again
 *	- (this should not be necessary: according the Interdata manual there
 *	  should be an interrupt when NRSRW drops)
 *
 * NOTE:
 *	This will not work when multi-level interrupts are implemented.  As 
 * this routine is called from the clock interrupt, it may be out of sync with
 * dskstart(), which will operate at a lower interrupt level.
 */
dsknrdy(dp)
struct dsk *dp;
{
	dp->d_active = 0;
	if (dsktab.d_active == 0)
		selchreq(NSELCH, &dskscq);
}

/*
 * Disk interrupt routine
 *	- disk interrupts only after a seek
 *	- status will be checked by dskstart() when controller is free
 */

dskintr(dev, stat)
{
	register struct dsk	*dp;

trace(4<<16, "interrupt", dev);
trace(4<<16, "status", stat);
	dp = devmap(dev);

	if (dp->d_active != DSEEK)
		return;
	dp->d_active = 0;

	dp->dk_cyl = dp->dk_actf->b_cyl;
	if (!dsktab.d_active)
		selchreq(NSELCH, &dskscq);
}

/*
 * Selch interrupt routine
 *	- selch interrupt will be followed by a controller interrupt
 *	  (unless OVERRUN status is set)
 */

dskscintr(dev, stat)
{
	register struct dsk	*dp;

	if (!(dp = dsktab.d_actf))
		return;

	oc(selchaddr[NSELCH], STOP);
	if ((stat = ss(cntladdr)) & OVERRUN) {
		dskerror(dp, stat, cntladdr);
		dsktab.d_active = dp->d_active = 0;
		dskstart();
	}
}

/*
 * Disk controller interrupt routine
 *	- check transfer status
 *	- on cylinder overflow, restart I/O on next cylinder
 *	- signal I/O done
 */

cntlintr(dev, stat)
register int	stat;
{
	register struct dsk	*dp;
	register struct buf	*bp;
	register int		resid;

trace(4<<16, "interrupt", dev);
trace(4<<16, "status", stat);
	dsktab.d_active = 0;
	if (!(dp = dsktab.d_actf) || dp->d_active != DIO)
		return;
	dp->d_active = 0;
	bp = dp->dk_actf;

	if (stat & CNTL_UNRCV) {
		dskerror(dp, stat, cntladdr);
		dp->dk_cyl = -1;		/* force seek */
		dskstart();
		return;
	}

	/* cylinder overflow */
	if ((stat & CYL_OV) == 0)
		bp->b_resid = 0;
	else {
		oc(selchaddr[NSELCH], STOP);
		resid = (rdh(selchaddr[NSELCH])-bp->b_addr+2) & ~0xff;
		bp->b_addr += resid;
		bp->b_bcount -= resid;
		bp->b_sector = 0;
trace(1<<16, "cylov", bp->b_cyl);
		if (++bp->b_cyl < NDSKBLK/24) {
			dskstart();
			return;
		} else {
			if ((bp->b_flags&(B_PHYS|B_READ)) != (B_PHYS|B_READ))
				bp->b_flags |= B_ERROR;
			bp->b_resid = resid;
		}
	}

	dp->d_errcnt = 0;
	dp->dk_actf = bp->av_forw;
	iodone(bp);
	dskstart();
}

/*
 * Common error routine
 */
dskerror(dp, stat, dev)
register struct dsk	*dp;
{
	register struct buf	*bp;

	bp = dp->dk_actf;
	deverror(bp, stat, dev);
	if (++dp->d_errcnt > NDSKERR) {
		dp->d_errcnt = 0;
		bp->b_flags |= B_ERROR;
		dp->dk_actf = bp->av_forw;
		iodone(bp);
	}
}

/*
 * 'Raw' disk interface
 */

dskread(dev)
{
	physio(dskstrategy, &rdskbuf, dev, B_READ);
}

dskwrite(dev)
{
	physio(dskstrategy, &rdskbuf, dev, B_WRITE);
}
