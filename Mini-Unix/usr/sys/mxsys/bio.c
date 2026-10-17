#
/*
 */

#include "param.h"
#include "user.h"
#include "buf.h"
#include "conf.h"
#include "systm.h"
#include "proc.h"

/*
 * This is the set of buffers proper, whose heads
 * were declared in buf.h.  There can exist buffer
 * headers not pointing here that are used purely
 * as arguments to the I/O routines to describe
 * I/O to be done-- e.g. swbuf, just below, for
 * swapping.
 */
char	buffers[NBUF][512];
struct	buf	swbuf;

/*
 * The following several routines allocate and free
 * buffers with various side effects.  In general the
 * arguments to an allocate routine are a device and
 * a block number, and the value is a pointer to
 * to the buffer header; the buffer is marked "busy"
 * so that no on else can touch it.  If the block was
 * already in core, no I/O need be done; if it is
 * already busy, the process waits until it becomes free.
 * The following routines allocate a buffer:
 *	getblk
 *	bread
 * Eventually the buffer must be released, possibly with the
 * side effect of writing it out, by using one of
 *	bwrite
 *	bdwrite
 *	bawrite
 *	brelse
 */

/*
 * Read in (if necessary) the block and return a buffer pointer.
 */
bread(dev, blkno)
{
	register struct buf *rbp;

	rbp = getblk(dev, blkno);
	if (rbp->b_flags&B_DONE)
		return(rbp);
	rbp->b_flags =| B_READ;
	rbp->b_wcount = -256;
	(*bdevsw[dev.d_major].d_strategy)(rbp);
	iowait(rbp);
	return(rbp);
}

/*
 * Write the buffer, waiting for completion.
 * Then release the buffer.
 */
bwrite(bp)
struct buf *bp;
{
	register struct buf *rbp;
	register flag;

	rbp = bp;
	flag = rbp->b_flags;
	rbp->b_flags =& ~(B_READ | B_DONE | B_ERROR | B_DELWRI);
	rbp->b_wcount = -256;
	(*bdevsw[rbp->b_dev.d_major].d_strategy)(rbp);
	if ((flag&B_ASYNC) == 0) {
		iowait(rbp);
		brelse(rbp);
	} else if ((flag&B_DELWRI)==0)
		geterror(rbp);
}

/*
 * Release the buffer, marking it so that if it is grabbed
 * for another purpose it will be written out before being
 * given up (e.g. when writing a partial block where it is
 * assumed that another write for the same block will soon follow).
 */
bdwrite(bp)
struct buf *bp;
{
	register struct buf *rbp;

	rbp = bp;
	rbp->b_flags =| B_DELWRI | B_DONE;
	brelse(rbp);
}

/*
 * Release the buffer, start I/O on it, but don't wait for completion.
 */
bawrite(bp)
struct buf *bp;
{
	register struct buf *rbp;

	rbp = bp;
	rbp->b_flags =| B_ASYNC;
	bwrite(rbp);
}

/*
 * release the buffer, with no I/O implied.
 */
brelse(bp)
struct buf *bp;
{
	register struct buf *rbp, **backp;
	register int sps;

	rbp = bp;
	if (rbp->b_flags&B_WANTED)
		wakeup(rbp);
	if (bfreelist.b_flags&B_WANTED) {
		bfreelist.b_flags =& ~B_WANTED;
		wakeup(&bfreelist);
	}
	if (rbp->b_flags&B_ERROR)
		rbp->b_dev.d_minor = -1;  /* no assoc. on error */
	backp = &bfreelist.av_back;
	sps = PS->integ;
	spl6();
	rbp->b_flags =& ~(B_WANTED|B_BUSY|B_ASYNC);
	(*backp)->av_forw = rbp;
	rbp->av_back = *backp;
	*backp = rbp;
	rbp->av_forw = &bfreelist;
	PS->integ = sps;
}

/*
 * Assign a buffer for the given block.  If the appropriate
 * block is already associated, return it; otherwise search
 * for the oldest non-busy buffer and reassign it.
 * When a 512-byte area is wanted for some random reason
 * (e.g. during exec, for the user arglist) getblk can be called
 * with device NODEV to avoid unwanted associativity.
 */
getblk(dev, blkno)
{
	register struct buf *bp;
	register struct devtab *dp;

	if(dev.d_major >= nblkdev)
		panic();

    loop:
	if (dev < 0)
		dp = &bfreelist;
	else {
		dp = bdevsw[dev.d_major].d_tab;
		if(dp == NULL)
			panic();
		for (bp=dp->b_forw; bp != dp; bp = bp->b_forw) {
			if (bp->b_blkno!=blkno || bp->b_dev!=dev)
				continue;
			spl6();
			if (bp->b_flags&B_BUSY) {
				bp->b_flags =| B_WANTED;
				sleep(bp, PRIBIO);
				spl0();
				goto loop;
			}
			spl0();
			notavail(bp);
			return(bp);
		}
	}
	spl6();
	if (bfreelist.av_forw == &bfreelist) {
		bfreelist.b_flags =| B_WANTED;
		sleep(&bfreelist, PRIBIO);
		spl0();
		goto loop;
	}
	spl0();
	notavail(bp = bfreelist.av_forw);
	if (bp->b_flags & B_DELWRI) {
		bp->b_flags =| B_ASYNC;
		bwrite(bp);
		goto loop;
	}
	bp->b_flags = B_BUSY | B_RELOC;
	bp->b_back->b_forw = bp->b_forw;
	bp->b_forw->b_back = bp->b_back;
	bp->b_forw = dp->b_forw;
	bp->b_back = dp;
	dp->b_forw->b_back = bp;
	dp->b_forw = bp;
	bp->b_dev = dev;
	bp->b_blkno = blkno;
	return(bp);
}

/*
 * Wait for I/O completion on the buffer; return errors
 * to the user.
 */
iowait(bp)
struct buf *bp;
{
	register struct buf *rbp;

	rbp = bp;
	spl6();
	while ((rbp->b_flags&B_DONE)==0)
		sleep(rbp, PRIBIO);
	spl0();
	geterror(rbp);
}

/*
 * Unlink a buffer from the available list and mark it busy.
 * (internal interface)
 */
notavail(bp)
struct buf *bp;
{
	register struct buf *rbp;
	register int sps;

	rbp = bp;
	sps = PS->integ;
	spl6();
	rbp->av_back->av_forw = rbp->av_forw;
	rbp->av_forw->av_back = rbp->av_back;
	rbp->b_flags =| B_BUSY;
	PS->integ = sps;
}

/*
 * Mark I/O complete on a buffer, release it if I/O is asynchronous,
 * and wake up anyone waiting for it.
 */
iodone(bp)
struct buf *bp;
{
	register struct buf *rbp;

	rbp = bp;
	rbp->b_flags =| B_DONE;
	if (rbp->b_flags&B_ASYNC)
		brelse(rbp);
	else {
		rbp->b_flags =& ~B_WANTED;
		wakeup(rbp);
	}
}

/*
 * Zero the core associated with a buffer.
 */
clrbuf(bp)
int *bp;
{
	register *p;
	register c;

	p = bp->b_addr;
	c = 256;
	do
		*p++ = 0;
	while (--c);
}

/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device buffer lists to empty.
 */
binit()
{
	register struct buf *bp;
	register struct devtab *dp;
	register int i;
	struct bdevsw *bdp;

	bfreelist.b_forw = bfreelist.b_back =
	    bfreelist.av_forw = bfreelist.av_back = &bfreelist;
	for (i=0; i<NBUF; i++) {
		bp = &buf[i];
		bp->b_dev = -1;
		bp->b_addr = buffers[i];
		bp->b_back = &bfreelist;
		bp->b_forw = bfreelist.b_forw;
		bfreelist.b_forw->b_back = bp;
		bfreelist.b_forw = bp;
		bp->b_flags = B_BUSY;
		brelse(bp);
	}
	i = 0;
	for (bdp = bdevsw; bdp->d_open; bdp++) {
		dp = bdp->d_tab;
		if(dp) {
			dp->b_forw = dp;
			dp->b_back = dp;
		}
		i++;
	}
	nblkdev = i;
}

/*
 * swap I/O
 */

#define USTACK	(TOPSYS-12)
struct	{ int *intp;};
struct	{char *chrp;};

swap(rp, rdflg)
struct proc *rp;
{
	register *pp, *p1, *p2;

	pp = rp;
	if(rdflg == B_WRITE) {
		p1 = USTACK->integ;
		p2 = TOPSYS + (u.u_dsize<<6) + (p1.integ&077);
		if(p2 <= p1) {
			pp->p_size = u.u_dsize + USIZE +
			    ((TOPUSR>>6)&01777) - ((p1.integ>>6)&01777);
			while(p1.chrp < TOPUSR)
				*p2++ = *p1++;
		} else
			pp->p_size = SWPSIZ<<3;
	}
	spl6();
	swbuf.b_flags = B_BUSY | rdflg;
	swbuf.b_dev = swapdev;
	swbuf.b_wcount = -(((pp->p_size+7)&~07)<<5);	/* 32 words per block */
	swbuf.b_blkno = SWPLO+rp->p_pid*SWPSIZ;
	swbuf.b_addr = &u;	/* 64 b/block */
	(*bdevsw[swapdev>>8].d_strategy)(&swbuf);
	spl6();
	while((swbuf.b_flags&B_DONE)==0)
		idle();
	spl0();
	if(rdflg == B_READ) {
		p1 = TOPUSR;
		p2 = (pp->p_size<<6) + TOPSYS - (USIZE<<6);
		if(p2 <= p1)
			while(p1 >= USTACK->integ.intp)
				*--p1 = *--p2;
	}
	swbuf.b_flags =& ~(B_BUSY|B_WANTED);
	return(swbuf.b_flags&B_ERROR);
}

/*
 * make sure all write-behind blocks
 * on dev (or NODEV for all)
 * are flushed out.
 * (from umount and update)
 */
bflush(dev)
{
	register struct buf *bp;

loop:
	spl6();
	for (bp = bfreelist.av_forw; bp != &bfreelist; bp = bp->av_forw) {
		if (bp->b_flags&B_DELWRI && (dev == NODEV||dev==bp->b_dev)) {
			bp->b_flags =| B_ASYNC;
			notavail(bp);
			bwrite(bp);
			goto loop;
		}
	}
	spl0();
}

/*
 * Pick up the device's error number and pass it to the user;
 * if there is an error but the number is 0 set a generalized
 * code.  Actually the latter is always true because devices
 * don't yet return specific errors.
 */
geterror(abp)
struct buf *abp;
{
	register struct buf *bp;

	bp = abp;
	if (bp->b_flags&B_ERROR)
		if ((u.u_error = bp->b_error)==0)
			u.u_error = EIO;
}
