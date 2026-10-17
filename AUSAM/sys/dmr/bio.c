#
/*
 */

#include "../defines.h"
#include "../param.h"
#include "../file.h"
#include "../user.h"
#include "../buf.h"
#include "../conf.h"
#ifdef	AUSAML
#include	"../lnode.h"
#endif	AUSAML
#include "../systm.h"
#include "../proc.h"
#include "../seg.h"

#ifndef	ONCE | MAPPED_BUFFERS
/*
 * This is the set of buffers proper, whose heads
 * were declared in buf.h.  There can exist buffer
 * headers not pointing here that are used purely
 * as arguments to the I/O routines to describe
 * I/O to be done-- e.g. swbuf, just below, for
 * swapping.
 */
char	buffers[NBUF][512];	/* fix025 */
#endif
#ifndef	RAW_BUFFER_POOL
struct	buf	swbuf;
#endif

/*
 * Declarations of the tables for the magtape devices;
 * see bdwrite.
 */
int	tmtab;
int	httab;

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
	rbp->b_wcount = -256;
	(*bdevsw[dev.d_major].d_strategy)(rbp);
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
			rbp->b_wcount = -256;
			(*bdevsw[adev.d_major].d_strategy)(rbp);
		}
	}
	if (rablkno && !incore(dev, rablkno) && (bfreelist.av_forw != &bfreelist) ) { 	/* fix001 */
		rabp = getblk(dev, rablkno);
		if (rabp->b_flags & B_DONE)
			brelse(rabp);
		else {
			rabp->b_flags =| B_READ|B_ASYNC;
			rabp->b_wcount = -256;
			(*bdevsw[adev.d_major].d_strategy)(rabp);
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
bwrite( rbp )
register struct buf *rbp;	/* fix000 */
{
	register flag;

	flag = rbp->b_flags;
#ifndef	BUFFER_AGING
	rbp->b_flags =& ~(B_READ | B_DONE | B_ERROR | B_DELWRI);
#endif
#ifdef	BUFFER_AGING
	rbp->b_flags =& ~(B_READ | B_DONE | B_ERROR | B_DELWRI | B_AGE);
#endif
	rbp->b_wcount = -256;
	(*bdevsw[rbp->b_dev.d_major].d_strategy)(rbp);
	if ((flag&B_ASYNC) == 0) {
		iowait(rbp);
		brelse(rbp);
	} else if ((flag&B_DELWRI)==0)
		geterror(rbp);
#ifdef	BUFFER_AGING
	else
		rbp->b_flags =| B_AGE;
#endif
}

/*
 * Release the buffer, marking it so that if it is grabbed
 * for another purpose it will be written out before being
 * given up (e.g. when writing a partial block where it is
 * assumed that another write for the same block will soon follow).
 * This can't be done for magtape, since writes must be done
 * in the same order as requested.
 */
bdwrite( rbp )
register struct buf *rbp;	/* fix000 */
{
	register struct devtab *dp;

#ifndef	HASHED_BUFFERS
	dp = bdevsw[rbp->b_dev.d_major].d_tab;
	if (dp == &tmtab || dp == &httab)
#else
	if(bdevsw[rbp->b_dev.d_major].d_nodel)
#endif	HASHED_BUFFERS
		bawrite(rbp);
	else {
		rbp->b_flags =| B_DELWRI | B_DONE;
		brelse(rbp);
	}
}

/*
 * Release the buffer, start I/O on it, but don't wait for completion.
 */
bawrite( rbp )
register struct buf *rbp;	/* fix000 */
{

	rbp->b_flags =| B_ASYNC;
	bwrite(rbp);
}

/*
 * release the buffer, with no I/O implied.
 */
brelse( rbp )
register struct buf *rbp;	/* fix000 */
{
	register struct buf **backp;
	register int sps;

	if (rbp->b_flags&B_WANTED)
		wakeup(rbp);
	if (rbp->b_flags&B_ERROR)
		rbp->b_dev.d_minor = -1;  /* no assoc. on error */
	sps = PS->integ;
	spl6();
#ifdef	QMOUNT
	if( ! (rbp->b_flags & B_MOUNT) ) 
	{
#endif	QMOUNT
		if (bfreelist.b_flags&B_WANTED)
		{
			bfreelist.b_flags =& ~B_WANTED;
			wakeup(&bfreelist);
		}
#ifdef	BUFFER_AGING
		if ( rbp->b_flags & B_AGE ) 
		{
			backp = &bfreelist.av_forw;
			(*backp)->av_back = rbp;
			rbp->av_forw = *backp;
			*backp = rbp;
			rbp->av_back = &bfreelist;
		}
		else
		{
#endif
			backp = &bfreelist.av_back;
			(*backp)->av_forw = rbp;
			rbp->av_back = *backp;
			*backp = rbp;
			rbp->av_forw = &bfreelist;
#ifdef	BUFFER_AGING
		}
#endif	BUFFER_AGING
#ifdef	QMOUNT
	}
#endif	QMOUNT
#ifdef	BUFFER_AGING
	rbp->b_flags =& ~(B_WANTED | B_BUSY | B_ASYNC | B_AGE);
#else
	rbp->b_flags =& ~(B_WANTED|B_BUSY|B_ASYNC);
#endif
	PS->integ = sps;
}
#ifdef	HASHED_BUFFERS
#define bhash(d,b)	(&btab[((d+b)&077777)%BHASHSZ])
#endif	HASHED_BUFFERS

/*
 * See if the block is associated with some buffer
 * (mainly to avoid getting hung up on a wait in breada)
 */
incore(dev, blkno)
register unsigned blkno;	/* fix000 */
{
	register struct buf *bp;
	register struct devtab *dp;

#ifdef	HASHED_BUFFERS
	dp = bhash(dev, blkno);
#else
	dp = bdevsw[dev.d_major].d_tab;
#endif	HASHED_BUFFERS
	for (bp=dp->b_forw; bp != dp; bp = bp->b_forw)
		if (bp->b_blkno==blkno && bp->b_dev==dev && ! (bp->b_flags&B_INVAL) )	/* fix030 */
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
register unsigned blkno;		/* fix000 */
{
	register struct buf *bp;
	register struct devtab *dp;
	extern lbolt;

	if(dev.d_major >= nblkdev)
		panic("blkdev");

    loop:
	if (dev < 0)
		dp = &bfreelist;
	else {
#ifdef	HASHED_BUFFERS
		dp = bhash(dev, blkno);
#else
		dp = bdevsw[dev.d_major].d_tab;
		if(dp == NULL)
			panic("devtab");
#endif	HASHED_BUFFERS
		for (bp=dp->b_forw; bp != dp; bp = bp->b_forw) {
			if (bp->b_blkno!=blkno || bp->b_dev!=dev || (bp->b_flags&B_INVAL) )	/* fix030 */
				continue;
			spl6();
			if (bp->b_flags&B_BUSY) {
				bp->b_flags =| B_WANTED;
				sleep(bp, PRIBIO + 1);		/* fix025 */
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
		sleep(&bfreelist, PRIBIO + 1);			/* fix025 */
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
	bp->b_flags = B_BUSY;	/* fix025 */
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
iowait( rbp )
register struct buf *rbp;	/* fix000 */
{
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
notavail( rbp )
register struct buf *rbp;	/* fix000 */
{
	register int sps;

	sps = PS->integ;
	spl6();
#ifdef	QMOUNT
	if( ! (rbp->b_flags & B_MOUNT) ) {
#endif	QMOUNT
		rbp->av_back->av_forw = rbp->av_forw;
		rbp->av_forw->av_back = rbp->av_back;
#ifdef	QMOUNT
	}
#endif	QMOUNT
	rbp->b_flags =| B_BUSY;
	PS->integ = sps;
}

/*
 * Mark I/O complete on a buffer, release it if I/O is asynchronous,
 * and wake up anyone waiting for it.
 */
iodone(rbp)
register struct buf *rbp;	/* fix000 */
{
#ifdef	UNIBUS_MAP
	if(rbp->b_flags&B_MAP)
		mapfree(rbp);
#endif	UNIBUS_MAP
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
register struct buf *bp;	/* fix000 */
{
	register *p;
	register c;
#ifdef	MAPPED_BUFFERS
	register xx;
#endif	MAPPED_BUFFERS

#ifndef	MAPPED_BUFFERS
	p = bp->b_addr;
#else	MAPPED_BUFFERS
	xx = ka5;
	bswtch( bp );
	p = b.buff;
#endif	MAPPED_BUFFERS
	c = 256;
	do
		*p++ = 0;
	while (--c);
#ifdef	MAPPED_BUFFERS
	ka5 = xx;
#endif	MAPPED_BUFFERS
}

#ifndef	ONCE
#include	"../binit.h"
#endif

#define	IENABLE	0100
#define	GO	01
#ifdef	DEVSTART
/*
 * Device start routine for disks
 * and other devices that have the register
 * layout of the older DEC controllers (RF, RK, RP, TM)
 */
#define	WCOM	02
#define	RCOM	04
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

#endif	DEVSTART
#ifdef	RHSTART
/*
 * startup routine for RH controllers.
 */
#define	RHWCOM	060
#define	RHRCOM	070

rhstart(rbp, devloc, devblk, abae)
register struct buf *rbp;	/* fix000 */
int *devloc, *abae;
{
	register int *dp;
	register int com;

	dp = devloc;
	*dp = devblk;			/* block address */
#ifndef	MAPPED_BUFFERS & UNIBUS_MAP
#ifdef	_1170
	com = *abae = rbp->b_xmem;
#else
	com = rbp->b_xmem;
#endif	_1170
	*--dp = rbp->b_addr;		/* buffer address */
#else
	if( rbp->b_flags & B_PHYS)
	{
#ifdef	_1170
		com = *abae = rbp->b_xmem;
#else
		com = rbp->b_xmem;
#endif	_1170
		*--dp = rbp->b_addr;
	}
	else
	{
		com = ((rbp->b_addr >> 6) & 01777) + (rbp->b_xmem << 10) + bufarea;
		*--dp = com << 6;
#ifdef	_1170
		*abae = com =>> 10;
#else
		com =>> 10;
#endif	_1170
	}
#endif	MAPPED_BUFFERS & UNIBUS_MAP
	*--dp = rbp->b_wcount;		/* word count */
	com = IENABLE | GO | ((com & 03) << 8);	/* command + x-mem */
	if (rbp->b_flags&B_READ)
		com =| RHRCOM;
	else
		com =| RHWCOM;
	*--dp = com;
}
#endif	RHSTART

#ifdef	UNIBUS_MAP
/*
 * 11/70 routine to allocate the
 * UNIBUS map and initialize for
 * a unibus device.
 * The code here and in
 * rhstart assumes that an rh on an 11/70
 * is an rh70 and contains 22 bit addressing.
 */
int	maplock;
#ifndef	MALLOC_UMAP
mapalloc(bp)
register struct buf *bp;	/* fix000 */
{
	register i, a;

	if(cputype != 70)
		return;
	spl6();
	while(maplock&B_BUSY) {
		maplock =| B_WANTED;
		sleep(&maplock, PSWP + 1);		/* fix025 */
	}
	maplock =| B_BUSY;
	spl0();
	bp->b_flags =| B_MAP;
	a = bp->b_xmem;
	for(i=16; i<32; i=+2)
		UBMAP->r[i+1] = a;
	for(a++; i<48; i=+2)
		UBMAP->r[i+1] = a;
	bp->b_xmem = 1;
}

mapfree(bp)
struct buf *bp;
{

	bp->b_flags =& ~B_MAP;
	if(maplock&B_WANTED)
		wakeup(&maplock);
	maplock = 0;
}
#else	MALLOC_UMAP
mapalloc(bp)
register int *bp;
{
	register unsigned a,nreg;
	long l;

	nreg = 16 - ((bp->b_wcount >> 12) & 017);
	spl6();
	while( (a = malloc(umap,nreg)) == NULL)
	{
		maplock = B_WANTED;
		sleep(&maplock, PSWP + 1);
	}
	spl0();
	l.loint = bp->b_addr;
	l.hiint = bp->b_xmem;
	bp->b_addr = (--a) << 13;
	bp->b_xmem = (a >> 3) & 03;
	bp->b_flags =| B_MAP;
	bp = &UBMAP->r[a << 1];
	do
	{
		*bp++ = l.loint;
		*bp++ = l.hiint;
		l =+ 020000;
	}
	while( --nreg);
}

mapfree(bp)
register int *bp;
{
	register unsigned nreg,a;

	nreg = 16 - ((bp->b_wcount >> 12) & 017);
	a = (( (bp->b_addr >> 13) & 07) | (bp->b_xmem << 3)) + 1;
	mfree( umap, nreg, a);
	if(maplock)
	{
		maplock = 0;
		wakeup( &maplock);
	}
}
#endif	MALLOC_UMAP

#endif	UNIBUS_MAP

#ifdef	RAW_BUFFER_POOL
	/* allocate and free raw buffer headers */
	/* getrb returns at pl==6	        */
	/* freerb returns at pl==0	        */

struct buf rawbufs[NRAWBUFS];

struct buf *getrb(dev,flag)
unsigned dev;			/* device major/minor */
unsigned flag;			/* r/w flag */
{
  register struct buf *rpend =  &rawbufs[NRAWBUFS];
  register struct buf *rpw   =  &rawbufs[0];
  register struct buf *rp;

  spl6();
  for(;;) {
    for( rp = rpw ; rp < rpend; rp++ )
	if( (rp->b_flags&B_BUSY)==0 ) {
			rp->b_flags = B_BUSY|B_PHYS|flag;
			rp->b_dev = dev;
			return( rp );	/* leave at level6 */
	} else if( (rp->b_flags&B_WANTED)==0 )  rpw = rp;
    rpw->b_flags =| B_WANTED;
    sleep( rpw , PSWP+1 );
  }
}

freerb(rp)
register struct buf *rp;	/* fix000 */
{
	/*
	 *	note always should enter a level 6 prty
	 */
	if( rp->b_flags&B_WANTED ) wakeup(rp);
	rp->b_flags =& ~(B_PHYS|B_WANTED|B_BUSY);
	spl0();
}
	/*					*/

#endif

/*
 * swap I/O
 */
swap(blkno, coreaddr, count, rdflg)
{
#ifndef	RAW_BUFFER_POOL
	register int *fp;

	fp = &swbuf.b_flags;
	spl6();
#ifdef	DEBUG_SWAP
	if( inmap(coremap,count,coreaddr) || inmap(swapmap,(count+7)>>3,blkno) )
		panic("coremap <-> swap <-> swapmap");
#endif	DEBUG_SWAP
	while (*fp&B_BUSY) {
		*fp =| B_WANTED;
		sleep(fp, PSWP+1);	/* fix002 */
	}
	*fp = B_BUSY | B_PHYS | rdflg;
	swbuf.b_dev = swapdev;
	swbuf.b_wcount = - (count<<5);	/* 32 w/block */
	swbuf.b_blkno = blkno;
	swbuf.b_addr = coreaddr<<6;	/* 64 b/block */
	swbuf.b_xmem = (coreaddr>>10) & 077;
	(*bdevsw[swapdev>>8].d_strategy)(&swbuf);
	spl6();
	while((*fp&B_DONE)==0)
		sleep(fp, PSWP);	/* fix002 */
	if (*fp&B_WANTED)
		wakeup(fp);
	spl0();
	*fp =& ~(B_BUSY|B_WANTED);
	if(*fp & B_ERROR)		/* fix025 */
		panic("I/O error in swap");		/* fix025 */
#else	RAW_BUFFER_POOL
	register struct buf *fp;

	fp = getrb(swapdev, rdflg);	/* getrb returns at level 6 */
#ifdef	DEBUG_SWAP
	if(inmap(coremap, count, coreaddr) || inmap(swapmap, (count+7)>>3, blkno))
		panic("coremap <-> swap <-> swapmap");
#endif	DEBUG_SWAP
	fp->b_wcount = -(count<<5);	/* 32 w/block */
	fp->b_blkno = blkno;
	fp->b_addr = coreaddr<<6;	/* 64 b/block */
	fp->b_xmem = (coreaddr>>10) & 077;
	(*bdevsw[swapdev>>8].d_strategy)(fp);
	spl6();
	while((fp->b_flags&B_DONE) == 0)
		sleep(fp, PSWP);	/* fix002 */
	freerb(fp);
	if(fp->b_flags & B_ERROR)		/* fix025 */
		panic("I/O error in swap");		/* fix025 */
#endif	RAW_BUFFER_POOL
}

/*
 * make sure all write-behind blocks
 * on dev (or NODEV for all)
 * are flushed out.
 * (from umount and update)
 */
bflush(dev)
register dev;
{
	register struct buf *bp;

loop:
	spl6();
	for (bp = bfreelist.av_forw; bp != &bfreelist; bp = bp->av_forw)
	{
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
 * Raw I/O. The arguments are
 *	The strategy routine for the device
 *	A buffer, which will always be a special buffer
 *	  header owned exclusively by the device for this purpose
 *	The device number
 *	Read/write flag
 * Essentially all the work is computing physical addresses and
 * validating them.
 */
physio(strat, bp, dev, rw)
register struct buf *bp;	/* fix000 */
int (*strat)();
{
	register char *base;
	register int nb;
	int ts;
#ifdef	RAW_BUFFER_POOL
	int abp;	/* to hold original value of bp */
#endif

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
/*	if ( (((base+u.u_count)>>6)&01777) >= ts+u.u_dsize	/* fix020 */
	if ( (((base+u.u_count-1)>>6)&01777) >= ts+u.u_dsize	/* fix020 */
	    && nb < 1024-u.u_ssize)
		goto bad;
#ifdef	RAW_BUFFER_POOL
   if( abp = bp ) {
#endif
	spl6();
	while (bp->b_flags&B_BUSY) {
		bp->b_flags =| B_WANTED;
		sleep(bp, PRIBIO);
	}
	bp->b_flags = B_BUSY | B_PHYS | rw;
	bp->b_dev = dev;
#ifdef	RAW_BUFFER_POOL
   } else  bp = getrb(dev,rw);
#endif
	/*
	 * Compute physical address by simulating
	 * the segmentation hardware.
	 */
	bp->b_addr = base&077;
	base = (u.u_sep? UDSA: UISA)->r[nb>>7] + (nb&0177);
	bp->b_addr =+ base<<6;
	bp->b_xmem = (base>>10) & 077;
	bp->b_blkno = u.u_offset >> 9;	/* fix000 */
	bp->b_wcount = -((u.u_count>>1) & 077777);
	bp->b_error = 0;
	bp->b_resid = 0; /* fix017 */
#ifdef	U_LOCK
	nb = u.u_procp->p_flag;
#endif
	u.u_procp->p_flag =| SLOCK;
	(*strat)(bp);
	spl6();
	while ((bp->b_flags&B_DONE) == 0)
		sleep(bp, PRIBIO);
#ifndef	U_LOCK
	u.u_procp->p_flag =& ~SLOCK;
#else
	u.u_procp->p_flag = nb;
#endif
	if(runin)			/* fix025 */
	{
		runin = 0;
		setrun(&proc[0]);
	}
#ifdef	RAW_BUFFER_POOL
   if( abp ) {
#endif
	if (bp->b_flags&B_WANTED)
		wakeup(bp);
	spl0();
	bp->b_flags =& ~(B_BUSY|B_WANTED);
#ifdef	RAW_BUFFER_POOL
   } else freerb(bp);
#endif
	u.u_count = (-bp->b_resid)<<1;	/* bytes not transfered */
	geterror(bp);
	return;
    bad:
	u.u_error = EFAULT;
}

/*
 * Pick up the device's error number and pass it to the user;
 * if there is an error but the number is 0 set a generalized
 * code.  Actually the latter is always true because devices
 * don't yet return specific errors.
 */
geterror(bp)
register struct buf *bp;	/* fix000 */
{
	if (bp->b_flags&B_ERROR)
		if ((u.u_error = bp->b_error)==0)
			u.u_error = EIO;
}

/*
 * invalidate blocks for a dev	fix030
 * must be called when block devices are finally closed fix030
 */

binval( dev )
{
#ifdef	HASHED_BUFFERS
	register struct bhashelt *dp;
#else
	register struct devtab *dp;
#endif	HASHED_BUFFERS
	register struct buf *bp;

#ifdef	HASHED_BUFFERS
    for(dp = &btab[0]; dp < &btab[BHASHSZ]; dp ++)
#else
	dp = bdevsw[dev.d_major].d_tab;
#endif	HASHED_BUFFERS
	for( bp = dp->b_forw ; bp != dp ; bp = bp->b_forw )
#ifdef	BUFFER_AGING
		if( bp->b_dev == dev ) bp->b_flags =| (B_INVAL | B_AGE);
#else
		if( bp->b_dev == dev ) bp->b_flags =| B_INVAL;
#endif	BUFFER_AGING
}
