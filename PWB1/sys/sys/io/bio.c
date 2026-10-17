#
#include "../hd/param.h"
#include "../hd/user.h"
#include "../hd/buf.h"
#include "../hd/conf.h"
#include "../hd/systm.h"
#include "../hd/var.h"

/*
 * Declarations of the tables for the magtape devices;
 * see bdwrite.
 */
int	tmtab, httab;

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
 *	breada
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
	(*bdevsw[major(dev)].d_strategy)(rbp);
	iowait(rbp);
	return(rbp);
}

/*
 * Read in the block, like bread, but also start I/O on the
 * read-ahead block (which is not allocated to the caller)
 */
breada(adev, blkno, rablkno)
{
	register struct buf *rbp, *rabp;
	register int dev;

	dev = adev;
	rbp = 0;
	if (!incore(dev, blkno)) {
		rbp = getblk(dev, blkno);
		if ((rbp->b_flags&B_DONE) == 0) {
			rbp->b_flags =| B_READ;
			(*bdevsw[major(adev)].d_strategy)(rbp);
		}
	}
	if (rablkno && bfreelist.b_wcount>1 && !incore(dev, rablkno)) {
		rabp = getblk(dev, rablkno);
		if (rabp->b_flags & B_DONE)
			brelse(rabp);
		else {
			rabp->b_flags =| B_READ|B_ASYNC;
			(*bdevsw[major(adev)].d_strategy)(rabp);
		}
	}
	if (rbp==0)
		return(bread(dev, blkno));
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
	rbp->b_flags =& ~(B_READ | B_DONE | B_ERROR | B_DELWRI | B_AGE);
	(*bdevsw[major(rbp->b_dev)].d_strategy)(rbp);
	if ((flag&B_ASYNC) == 0) {
		iowait(rbp);
		brelse(rbp);
	} else if ((flag&B_DELWRI)==0)
		geterror(rbp); else
		rbp->b_flags =| B_AGE;
}

/*
 * Release the buffer, marking it so that if it is grabbed
 * for another purpose it will be written out before being
 * given up (e.g. when writing a partial block where it is
 * assumed that another write for the same block will soon follow).
 * This can't be done for magtape, since writes must be done
 * in the same order as requested.
 */
bdwrite(bp)
struct buf *bp;
{
	register struct buf *rbp;
	register struct devtab *dp;

	rbp = bp;
	dp = bdevsw[major(rbp->b_dev)].d_tab;
	if (dp == &tmtab || dp == &httab)
		bawrite(rbp);
	else {
		rbp->b_flags =| B_DELWRI | B_DONE;
		brelse(rbp);
	}
}

/*
 * Release the buffer, start I/O on it, but don't wait for completion.
 */
bawrite(bp)
struct buf *bp;
{
	register struct buf *rbp;

	rbp = bp;
	if(bfreelist.b_wcount>4)
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
	if (rbp->b_flags&B_ERROR) {
		rbp->b_dev =| 0200;
		rbp->b_flags =& ~(B_ERROR|B_DELWRI);
	}
	sps = PS->integ;
	spl6();
	if(rbp->b_flags & B_AGE) {
		backp = &bfreelist.av_forw;
		(*backp)->av_back = rbp;
		rbp->av_forw = *backp;
		*backp = rbp;
		rbp->av_back = &bfreelist;
	} else {
		backp = &bfreelist.av_back;
		(*backp)->av_forw = rbp;
		rbp->av_back = *backp;
		*backp = rbp;
		rbp->av_forw = &bfreelist;
	}
	rbp->b_flags =& ~(B_WANTED|B_BUSY|B_ASYNC|B_AGE);
	bfreelist.b_wcount++;
	PS->integ = sps;
}

/*
 * See if the block is associated with some buffer
 * (mainly to avoid getting hung up on a wait in breada)
 */
incore(adev, blkno)
{
	register int dev;
	register struct buf *bp;
	register struct devtab *dp;

	dev = adev;
	dp = bdevsw[major(adev)].d_tab;
	for (bp=dp->b_forw; bp != dp; bp = bp->b_forw)
		if (bp->b_blkno==blkno && bp->b_dev==dev)
			return(bp);
	return(0);
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

    loop:
	if (dev < 0)
		dp = &bfreelist;
	else {
		if (major(dev) >= bdevcnt)
			panic("blkdev");
		dp = bdevsw[major(dev)].d_tab;
		if(dp == NULL)
			panic("devtab");
		for (bp=dp->b_forw; bp != dp; bp = bp->b_forw) {
			if (bp->b_blkno!=blkno || bp->b_dev!=dev)
				continue;
			spl6();
			if (bp->b_flags&B_BUSY) {
				bp->b_flags =| B_WANTED;
				sleep(bp, PRIBIO+1);
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
		sleep(&bfreelist, PRIBIO+1);
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
	bp->b_flags = B_BUSY;
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
	bfreelist.b_wcount--;
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
	for (i=0; i<v.v_buf; i++) {
		bp = &buf[i];
		bp->b_dev = -1;
		bp->b_addr = buffers[i];
		bp->b_back = &bfreelist;
		bp->b_forw = bfreelist.b_forw;
		bfreelist.b_forw->b_back = bp;
		bfreelist.b_forw = bp;
		bp->b_flags = B_BUSY;
		bp->b_wcount = -256;
		brelse(bp);
	}
	for (i=0, bdp = bdevsw; i<bdevcnt; i++, bdp++) {
		dp = bdp->d_tab;
		if(dp) {
			dp->b_forw = dp;
			dp->b_back = dp;
		}
	}
}

/*
 * make sure all write-behind blocks on dev (or NODEV for all)
 * are flushed out.
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
 * if there is an error but the number is 0 set a generalized code.
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
