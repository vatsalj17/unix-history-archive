
/*
 * This is the set of buffers proper, whose heads
 * were declared in buf.h.  There can exist buffer
 * headers not pointing here that are used purely
 * as arguments to the I/O routines to describe
 * I/O to be done-- e.g. swbuf, just below, for
 * swapping.
 */
#ifdef	ONCE
char	buffers[][512];	/* fix025 */
#endif	ONCE
#ifdef	MAPPED_BUFFERS
#define	CORETIK	0100 /* bytes in a core unit */
#define	NUREG	((NBUF + 15) / 16)	/* number of unibus map registers needed for buffer pool */
#endif
#ifndef	RAW_BUFFER_POOL
struct	buf	swbuf;
#endif

/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device buffer lists to empty.
 */
binit()
{
	register int *bp;
	register struct devtab *dp;
	register int i;
#ifdef	MAPPED_BUFFERS
	unsigned ba;
#ifdef	UNIBUS_MAP
	long	l;
#endif	UNIBUS_MAP
#endif	MAPPED_BUFFERS
	struct bdevsw *bdp;
#ifdef	HASHED_BUFFERS
	struct	bhashelt *bhp;
#endif	HASHED_BUFFERS

	bfreelist.b_forw = bfreelist.b_back =
	    bfreelist.av_forw = bfreelist.av_back = &bfreelist;
#ifdef	MAPPED_BUFFERS
	if((bufarea = malloc(coremap,NBUF * (512/CORETIK))) == NULL)
		panic("no core for buffers");
#ifdef	UNIBUS_MAP
	/*
	 * set up UNIBUS-MAP
	 */
	l = bufarea;
	l =<< 6;
	bp = UBMAP;
	i = NUREG;
	do
	{
		*bp++ = l.loint;
		*bp++ = l.hiint;
		l =+ 020000;
	}
	while( --i);
	mfree( umap, 31 - NUREG, NUREG + 1);
	ba = 0;
#else	UNIBUS_MAP
	ba = bufarea;
#endif	UNIBUS_MAP
#endif	MAPPED_BUFFERS
	for (i=0; i<NBUF; i++) {
		bp = &buf[i];
		bp->b_dev = NODEV;
#ifdef	MAPPED_BUFFERS
#ifdef	UNIBUS_MAP
		bp->b_xmem = i>>7;
		bp->b_addr = ba;
		ba =+ 512;
#else	UNIBUS_MAP
		bp->b_xmem = (ba >> 10) & 077;
		bp->b_addr = ba << 6;
		ba =+ (512/CORETIK);
#endif	UNIBUS_MAP
#else
		bp->b_addr = buffers[i];
#endif	MAPPED_BUFFERS
		bp->b_back = &bfreelist;
		bp->b_forw = bfreelist.b_forw;
		bfreelist.b_forw->b_back = bp;
		bfreelist.b_forw = bp;
		bp->b_flags = B_BUSY;
		brelse(bp);
	}
	i = 0;
	for (bdp = bdevsw; bdp->d_open; bdp++) {
#ifndef	HASHED_BUFFERS
		dp = bdp->d_tab;
		if(dp) {
			dp->b_forw = dp;
			dp->b_back = dp;
		}
#endif	HASHED_BUFFERS
		i++;
	}
	nblkdev = i;
#ifdef	HASHED_BUFFERS
	for(bhp = btab; bhp < &btab[BHASHSZ]; bhp ++)
		bhp->b_forw = bhp->b_back = bhp;
#endif	HASHED_BUFFERS
}
