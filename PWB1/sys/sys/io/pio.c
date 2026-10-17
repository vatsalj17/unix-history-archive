#
#include "../hd/param.h"
#include "../hd/user.h"
#include "../hd/buf.h"
#include "../hd/systm.h"
#include "../hd/proc.h"
#include "../hd/conf.h"
#include "../hd/seg.h"

struct	buf	swbuf[NSWB];

/*
 * Device start routine for disks
 * and other devices that have the register
 * layout of the older DEC controllers (RF, RK, RP, TM)
 */
#define	IENABLE	0100
#define	WCOM	02
#define	RCOM	04
#define	GO	01
devstart(bp, devloc, devblk, hbcom)
struct buf *bp;
int *devloc;
{
	register int *dp;
	register struct buf *rbp;
	register int com;

	dp = devloc;
	rbp = bp;
	*dp = devblk;			/* block address */
	*--dp = rbp->b_addr;		/* buffer address */
	*--dp = rbp->b_wcount;		/* word count */
	com = (hbcom<<8) | IENABLE | GO |
		((rbp->b_xmem & 03) << 4);
	if (rbp->b_flags&B_READ)	/* command + x-mem */
		com =| RCOM;
	else
		com =| WCOM;
	*--dp = com;
}

/*
 * startup routine for RH controllers.
 */
#define	RHWCOM	060
#define	RHRCOM	070

rhstart(bp, devloc, devblk, abae)
struct buf *bp;
int *devloc, *abae;
{
	register int *dp;
	register struct buf *rbp;
	register int com;

	dp = devloc;
	rbp = bp;
	if(cputype == 70)
		*abae = rbp->b_xmem;
	*dp = devblk;			/* block address */
	*--dp = rbp->b_addr;		/* buffer address */
	*--dp = rbp->b_wcount;		/* word count */
	com = IENABLE | GO |
		((rbp->b_xmem & 03) << 8);
	if (rbp->b_flags&B_READ)	/* command + x-mem */
		com =| RHRCOM; else
		com =| RHWCOM;
	*--dp = com;
}

/*
 * swap I/O
 */
swap(blkno, coreaddr, count, rdflg)
{
	static struct buf *sbp;
	register struct buf *bp;

	if (sbp==NULL)
		sbp = &swbuf[0];
	bp = sbp++;
	if (sbp > &swbuf[NSWB-1])
		sbp = &swbuf[0];
	spl6();
	while (bp->b_flags&B_BUSY) {
		bp->b_flags =| B_WANTED;
		sleep(bp, PSWP+1);
	}
	bp->b_flags = B_BUSY | B_PHYS | rdflg;
	bp->b_dev = swapdev;
	bp->b_wcount = - (count<<5);	/* 32 w/block */
	bp->b_blkno = blkno;
	bp->b_addr = coreaddr<<6;	/* 64 b/block */
	bp->b_xmem = (coreaddr>>10) & 077;
	(*bdevsw[major(swapdev)].d_strategy)(bp);
	spl6();
	while((bp->b_flags&B_DONE)==0)
		sleep(bp, PSWP);
	if (bp->b_flags&B_WANTED)
		wakeup(bp);
	spl0();
	bp->b_flags =& ~(B_BUSY|B_WANTED);
	if (bp->b_flags & B_ERROR)
		panic("IO err in swap");
}

/*
 * Raw I/O. The arguments are
 *	The strategy routine for the device
 *	A buffer, which will always be a special buffer
 *	  header owned exclusively by the device for this purpose
 *	The device number
 *	Read/write flag
 * Essentially all the work is computing physical addresses and
 * validating them.
 */
physio(strat, abp, dev, rw)
struct buf *abp;
int (*strat)();
{
	register struct buf *bp;
	register char *base;
	register int nb;
	int ts;

	bp = abp;
	base = u.u_base;
	/*
	 * Check odd base, odd count, and address wraparound
	 */
	if (base&01 || u.u_count&01 || base>=base+u.u_count)
		goto bad;
	ts = (u.u_tsize+127) & ~0177;
	if (u.u_sep)
		ts = 0;
	nb = (base>>6) & 01777;
	/*
	 * Check overlap with text. (ts and nb now
	 * in 64-byte clicks)
	 */
	if (nb < ts)
		goto bad;
	/*
	 * Check that transfer is either entirely in the
	 * data or in the stack: that is, either
	 * the end is in the data or the start is in the stack
	 * (remember wraparound was already checked).
	 */
	if ((((base+u.u_count)>>6)&01777) >= ts+u.u_dsize
	    && nb < 1024-u.u_ssize)
		goto bad;
	spl6();
	while (bp->b_flags&B_BUSY) {
		bp->b_flags =| B_WANTED;
		sleep(bp, PRIBIO+1);
	}
	bp->b_flags = B_BUSY | B_PHYS | rw;
	bp->b_dev = dev;
	/*
	 * Compute physical address by simulating
	 * the segmentation hardware.
	 */
	bp->b_addr = base&077;
	base = (u.u_sep? UDSA: UISA)->r[nb>>7] + (nb&0177);
	bp->b_addr =+ base<<6;
	bp->b_xmem = (base>>10) & 077;
	bp->b_blkno = lshift(u.u_offset, -9);
	bp->b_wcount = -((u.u_count>>1) & 077777);
	bp->b_error = 0;
	u.u_procp->p_flag =| SLOCK;
	(*strat)(bp);
	spl6();
	while ((bp->b_flags&B_DONE) == 0)
		sleep(bp, PRIBIO);
	u.u_procp->p_flag =& ~SLOCK;
	if (bp->b_flags&B_WANTED)
		wakeup(bp);
	if(runin) {
		runin = 0;
		wakeup(&runin);
	}
	spl0();
	bp->b_flags =& ~(B_BUSY|B_WANTED);
	u.u_count = bp->b_resid;
	geterror(bp);
	return;
    bad:
	u.u_error = EFAULT;
}

physck(nblocks, rw)
register unsigned nblocks;
{
	register unsigned base, limit;

	base = lshift(u.u_offset, -9);
	if (base >= nblocks) {
		if (base > nblocks || rw == B_WRITE)
			u.u_error = ENXIO;
		return(0);
	}
	limit = base + ldiv(u.u_count+511, 512);
	if (limit > nblocks) {
		base = (limit-nblocks)<<9;
		u.u_count =- base;
		u.u_arg[1] =- base;
	}
	return(1);
}
