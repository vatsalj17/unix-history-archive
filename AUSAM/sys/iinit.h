/*
 * iinit is called once (from main)
 * very early in initialization.
 * It reads the root's super block
 * and initializes the current date
 * from the last modified date.
 *
 * panic: iinit -- cannot read the super
 * block. Usually because of an IO error.
 */
iinit()
{
	register *cp, *bp;

	(*bdevsw[rootdev.d_major].d_open)(rootdev, 1);
	(*bdevsw[swapdev.d_major].d_open)(swapdev, 1);	/* fix0xx */
	bp = bread(rootdev, 1);
#ifndef	QMOUNT
	cp = getblk(NODEV);
#endif	QMOUNT
	if(u.u_error)
		panic("iinit");
#ifndef	QMOUNT
	bcopy(bp->b_addr, cp->b_addr, 256);
	brelse(bp);
	mount[0].m_bufp = cp;
#else
	mount[0].m_bufp = bp;
	bp->b_flags =| B_MOUNT;
#endif	QMOUNT
	mount[0].m_dev = rootdev;
#ifndef	MAPPED_BUFFERS
#ifndef	QMOUNT
	cp = cp->b_addr;
#else
	cp = bp->b_addr;
#endif	QMOUNT
	cp->s_flock = 0;
	cp->s_ilock = 0;
	cp->s_ronly = 0;
	time = cp->s_time;	/* fix000 */
#else
	bswtch( bp );	/* enable ka5 */
	b.s_flock = 0;
	b.s_ilock = 0;
	b.s_ronly = 0;
	b.s_ilowa = 0;	/* fix026 */
	time = b.s_time;
#endif	MAPPED_BUFFERS
#ifdef	QMOUNT
	brelse( bp );
#endif	QMOUNT
}
