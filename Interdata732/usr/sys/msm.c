#

#include "param.h"
#include "buf.h"
#include "conf.h"
#include "selch.h"

/*
 * 67-mb 'MSM' disk driver
 */

/* Configuration */

#define NDSK	2		/* number of disks */

/* device addresses */

#define NSELCH	0	/* connected to selch 1 */

int	msmcntl	0xfb;		/* disk controller */
char	msmaddr[NDSK] {			/* disk drives */
	0xfc,
	0xfd
};

int msmseek(), msmscintr();

struct devtab	msmtab;
struct buf	rmsmbuf;

struct selchq msmscq {
	&msmseek,
	&msmscintr,
	0
};

int	msmdrive;
int	msmcyl;
int	msmhead;
int	msmsector;
int	msmsad, msmead;
int	msmseekf;
int	msmqlen;		/* current length of i/o queue
				   (for display only */

#define	NBPC	(32*5)		/* blocks per cylinder */
#define NBPT	32		/* blocks per track */
#define NDSKERR	10		/* number of error retries */

/* logical device mapping */

struct dskmap {
	int	dm_baddr;		/* block offset */
	int	dm_nblk;		/* number of blocks */
} msmap[] {
	0,		60*NBPC,
	60*NBPC,	60*NBPC,
	120*NBPC,	702*NBPC,
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		822*NBPC
};

/* disk drive status & commands */
#define	BSY	0x20
#define	UNS	0x10
#define UNREADY	0x08
#define	SINC	0x02
#define	OFFL	0x01

#define	DISABLE	0x80
#define ENABLE	0x40
#define	DISARM	0xc0
#define SETHEAD	0x20
#define	SETCYL	0x10
#define	SEEK	0x02

/* disk controller status & commands */
#define	CNTL_UNRCV	0xc1
#define	CYL_OV		0x10
#define IDLE		0x02

#define	READ		0x01
#define	WRITE		0x02
#define	RESET		0x08


/*
 *	disk strategy routine
 */
msmstrategy(abp)
{
	register struct buf *bp;

	bp = abp;
	bp->b_resid = bp->b_bcount;

	if ((bp->b_dev.d_minor>>3) >= NDSK) {
		bp->b_flags =| B_ERROR;
		iodone(bp);
		return;
	}

	/*
	 * Block no. too high -- looks like EOF for raw read, error otherwise
	 */
	if (bp->b_blkno >= msmap[bp->b_dev.d_minor&07].dm_nblk) {
		if ((bp->b_flags&(B_PHYS|B_READ)) != (B_PHYS|B_READ))
			bp->b_flags =| B_ERROR;
		iodone(bp);
		return;
	}
	bp->av_forw = 0;
	spl(5);
	msmqlen = (msmqlen<<1) | 01;
	if (msmtab.d_actf == 0)
		msmtab.d_actf = bp;
	else
		msmtab.d_actl->av_forw = bp;
	msmtab.d_actl = bp;
	if (msmtab.d_active == 0)
		msmstart();
	spl(0);
}

/*
 * start next disk i/o operation
 *	- set up drive address, cylinder/head/sector address
 *	- set up memory start & end addresses
 *	- initiate seek
 */

msmstart()
{
	register struct buf *bp;
	register stat;
	register bn;

	if (!(bp = msmtab.d_actf))
		return;
	msmtab.d_active++;

	trace(010<<16, "mstart", bp);

	msmdrive = msmaddr[bp->b_dev.d_minor>>3];
	bn = bp->b_blkno + msmap[bp->b_dev.d_minor&07].dm_baddr;
	msmcyl = bn / NBPC;
	bn =% NBPC;
	msmhead = bn / NBPT;
	msmsector = (bn % NBPT)<<1;

	msmsad = bp->b_addr;
	msmead = bp->b_addr + bp->b_bcount - 1;

	selchreq(NSELCH, &msmscq);
}

msmseek()
{
	register stat;

	trace(010<<16, "seek", msmdrive);
	trace(010<<16, "cyl", msmcyl);

	msmseekf++;
	wh(msmdrive, msmcyl);
	oc(msmdrive, SETCYL|DISARM);
	oc(msmdrive, SEEK|ENABLE);
}

/*
 * disk interrupt routine
 *	-disk interrupts only after a seek (I hope)
 *	-check status, initiate read/write
 */

msmintr(dev,stat)
{
	register struct buf *bp;
	register scmd, ccmd;

	trace(020<<16, "interrupt", dev);
	trace(020<<16, "status", stat);

	if (!(bp = msmtab.d_actf) || !msmseekf)
		return;
	msmseekf = 0;

	if (stat & (BSY|UNS|UNREADY|SINC|OFFL)) {
		msmerror(bp, stat, msmdrive);
		return;
	}

	if (bp->b_flags & B_READ) {
		scmd = READ_GO;
		ccmd = READ|ENABLE;
	} else {
		scmd = GO;
		ccmd = WRITE|ENABLE;
	}

	trace(010<<16, "mcmd", ccmd);
	trace(010<<16, "head", msmhead);
	trace(010<<16, "sector", msmsector);

	oc(selchaddr[NSELCH], STOP);
	trace(010<<16, "bufstart", msmsad);
	trace(010<<16, "bufend", msmead);
	wdh(selchaddr[NSELCH], msmsad);
	wdh(selchaddr[NSELCH], msmead);
	wh(msmdrive, msmhead);
	oc(msmdrive, SETHEAD|DISARM);
	while ((ss(msmcntl)&IDLE) == 0)
		;
	wd(msmcntl, msmsector);
	wh(msmcntl, (msmhead<<10)+msmcyl);
	wh(msmdrive, msmhead);
	oc(msmdrive, SETHEAD|DISARM);
	while ((ss(msmcntl)&IDLE) == 0)
		;
	oc(msmcntl, ccmd);
	oc(selchaddr[NSELCH], scmd);
}

/*
 * selch interrupt routine
 *	-selch interrupt will be followed by a controller interrupt
 *		( nothing to do here? )
 */

msmscintr(dev, stat)
{
	register struct buf *bp;

	oc(selchaddr[NSELCH], STOP);
}

/*
 * disk controller interrupt routine
 *	-check ending status, signal i/o done
 */

msmcintr(dev, stat)
{
	register struct buf *bp;
	register struct dskmap *dm;

	trace(020<<16, "interrupt", dev);
	trace(020<<16, "status", stat);


	if (!(bp = msmtab.d_actf))
		return;
	if (msmseekf)
		return;

	if (stat & CNTL_UNRCV) {
		oc(msmcntl, RESET);
		msmerror(bp, stat, msmcntl);
		return;
	}
	bp->b_resid = 0;

	/*
	 * Cylinder overflow
	 */
	if (stat & CYL_OV) {
		oc(msmcntl, RESET);
		oc(selchaddr[NSELCH], STOP);
		msmsad =+ (rdh(selchaddr[NSELCH])-msmsad+2)&~0377;
		trace(020<<16, "cyl ov", msmsad);
		msmsector = msmhead = 0;
		bp->b_resid = msmead - msmsad;
		dm = &msmap[bp->b_dev.d_minor&07];
		if ((++msmcyl*NBPC) < dm->dm_baddr + dm->dm_nblk) {
			msmseek();
			return;
		} else
			if ((bp->b_flags&(B_PHYS|B_READ)) != (B_PHYS|B_READ))
				bp->b_flags =| B_ERROR;
	}

	msmtab.d_errcnt = 0;
	msmtab.d_active = 0;
	msmtab.d_actf = bp->av_forw;
	msmqlen =>> 1;
	iodone(bp);
	selchfree(NSELCH);
	msmstart();
}

msmerror(abp, stat, addr)
struct buf *abp;
{
	register struct buf *bp;

	bp = abp;
	deverror(bp, stat, addr);
	if (++msmtab.d_errcnt <= NDSKERR) {
		msmseek();
		return;
	}
	msmtab.d_errcnt = 0;
	bp->b_flags =| B_ERROR;
	msmtab.d_active = 0;
	msmtab.d_actf = bp->av_forw;
	msmqlen =>> 1;
	iodone(bp);
	selchfree(NSELCH);
	msmstart();
}

/*
 * 'Raw' disc interface
 */
msmread(dev)
{
	physio(msmstrategy, &rmsmbuf, dev, B_READ);
}

msmwrite(dev)
{
	physio(msmstrategy, &rmsmbuf, dev, B_WRITE);
}
