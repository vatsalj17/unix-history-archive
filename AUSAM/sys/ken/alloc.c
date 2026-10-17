#
/*
 */

#include "../defines.h"
#include "../param.h"
#include "../file.h"
#ifdef	AUSAML
#include "../lnode.h"
#include "../proc.h"
#endif	AUSAML
#include "../systm.h"
#include "../filsys.h"
#include "../conf.h"
#include "../buf.h"
#include "../inode.h"
#include "../user.h"

#ifndef	ONCE
#include "../iinit.h"
#endif

/*
 * alloc will obtain the next available
 * free disk block from the free list of
 * the specified device.
 * The super block has up to 100 remembered
 * free blocks; the last of these is read to
 * obtain 100 more . . .
 *
 * no space on dev x/y -- when
 * the free list is exhausted.
 */
#ifndef	MAPPED_BUFFERS
alloc(dev)
{
	unsigned bno;	/* fix000 */
	register *bp, *ip, *fp;

#ifdef	AUSAML
	if( (fp = u.u_procp->p_lnode) && (fp->l_flags & DLIMIT) ) {
		/* no further allocation for this user */
		u.u_error = EDISKLIM;
		return( NULL );
	}

#endif	AUSAML
	fp = getfs(dev);
	while(fp->s_flock)
		sleep(&fp->s_flock, PINOD);
	do {
		if(fp->s_nfree <= 0)
			goto nospace;
		bno = fp->s_free[--fp->s_nfree];
		if(bno == 0)
			goto nospace;
	} while (badblock(fp, bno, dev));
	if(fp->s_nfree <= 0) {
		fp->s_flock++;
		bp = bread(dev, bno);
		ip = bp->b_addr;
		fp->s_nfree = *ip++;
		bcopy(ip, fp->s_free, 100);
		brelse(bp);
		fp->s_flock = 0;
		wakeup(&fp->s_flock);
	}
	bp = getblk(dev, bno);
	clrbuf(bp);
	fp->s_fmod = 1;
	return(bp);

nospace:
	fp->s_nfree = 0;
	prdev("no space", dev);
	u.u_error = ENOSPC;
#ifdef	UPRINTS
	uprints( "\nwrite error - device full\n" );
#endif
#ifdef	COOL_NO_SPACE
#ifndef	DELAY
	sleep( &lbolt , PRIBIO );
#endif
#ifdef	DELAY
	delay( 5*HZ );	/* hang 5 */
#endif
#endif
	return(NULL);
}
#else	MAPPED_BUFFERS
/*
 *	ka5 must be preserved
 */
alloc(dev)
{
	unsigned bno;	/* fix000 */
	register *bp;
#ifdef	MAPPED_BUFFERS
	int ka5sav1;
#endif	MAPPED_BUFFERS

#ifdef	AUSAML
	if( (bp = u.u_procp->p_lnode) && (bp->l_flags & DLIMIT) ) {
		/* no further allocation for this user */
		u.u_error = EDISKLIM;
		return( NULL );
	}

#endif	AUSAML
	ka5sav1 = ka5;
	getfs(dev);
	while(b.s_flock & S_BUSY)		/* fix026 */
	{					/* fix026 */
		b.s_flock =| S_WANTED;		/* fix026 */
		sleep(&b.s_flock, PINOD+1);	/* fix026 */
	}					/* fix026 */
	do {
		if(b.s_nfree <= 0)
			goto nospace;
		bno = b.s_free[--b.s_nfree];
		if(bno == 0)
			goto nospace;
	} while (badblock(bno, dev));
	if(b.s_nfree <= 0) {
		register int ka5sav;

		b.s_flock++;
		ka5sav = ka5;
		bp = bread(dev, bno);
		bswtch( bp );
		bcopy( &b , bufscr , 101 );
		brelse(bp);
		ka5 = ka5sav;
		b.s_nfree = bufscr[0];
		bcopy(bufscr+1, b.s_free, 100);
		if(b.s_flock & S_WANTED)		/* fix026 */
			wakeup(&b.s_flock);		/* fix026 */
		b.s_flock = 0;
	}
	bp = getblk(dev, bno);
	clrbuf(bp);
	b.s_fmod = 1;
	ka5 = ka5sav1;
	return(bp);

nospace:
	b.s_nfree = 0;
	prdev("no space", dev);
	u.u_error = ENOSPC;
#ifdef	UPRINTS
	uprints( "\nwrite error - device full\n" );
#endif
#ifdef	COOL_NO_SPACE
#ifndef	DELAY
	sleep( &lbolt , PRIBIO );
#endif
#ifdef	DELAY
	delay( 5*HZ );	/* hang 5 */
#endif
#endif
	ka5 = ka5sav1;
	return(NULL);
}
#endif	MAPPED_BUFFERS

/*
 * place the specified disk block
 * back on the free list of the
 * specified device.
 */
#ifndef	MAPPED_BUFFERS
free(dev, bno)
  unsigned bno;	/* fix000 */
{
	register *fp, *bp, *ip;

	fp = getfs(dev);
	fp->s_fmod = 1;
	while(fp->s_flock)
		sleep(&fp->s_flock, PINOD);
	if (badblock(fp, bno, dev))
		return;
	if(fp->s_nfree <= 0) {
		fp->s_nfree = 1;
		fp->s_free[0] = 0;
	}
	if(fp->s_nfree >= 100) {
		fp->s_flock++;
		bp = getblk(dev, bno);
		ip = bp->b_addr;
		*ip++ = fp->s_nfree;
		bcopy(fp->s_free, ip, 100);
		fp->s_nfree = 0;
		bwrite(bp);
		fp->s_flock = 0;
		wakeup(&fp->s_flock);
	}
	fp->s_free[fp->s_nfree++] = bno;
	fp->s_fmod = 1;
}
#else	MAPPED_BUFFERS
/*
 *	ka5 is preserved by this routine
 */
free(dev, bno)
unsigned bno;	/* fix000 */
{
	register *bp;
	register ka5sav1 = ka5;

	getfs(dev);
	b.s_fmod = 1;
	while(b.s_flock & S_BUSY)		/* fix026 */
	{
		b.s_flock =| S_WANTED;		/* fix026 */
		sleep(&b.s_flock, PINOD);
	}
	if (badblock(bno, dev))
		goto ret;
	if(b.s_nfree <= 0) {
		b.s_nfree = 1;
		b.s_free[0] = 0;
	}
	if(b.s_nfree >= 100) {
		register int ka5sav;

		b.s_flock++;
		bp = getblk(dev, bno);
		bufscr[0] = b.s_nfree;
		bcopy( &b.s_free , bufscr+1 , 100);
		ka5sav = ka5;
		bswtch( bp );
		bcopy( bufscr , &b , 101 );
		ka5 = ka5sav;
		b.s_nfree = 0;
		bwrite(bp);
		if( b.s_flock & S_WANTED)		/* fix026 */
			wakeup(&b.s_flock);		/* fix026 */
		b.s_flock = 0;
	}
	b.s_free[b.s_nfree++] = bno;
	b.s_fmod = 1;
ret:
	ka5 = ka5sav1;
}
#endif	MAPPED_BUFFERS

/*
 * Check that a block number is in the
 * range between the I list and the size
 * of the device.
 * This is used mainly to check that a
 * garbage file system has not been mounted.
 *
 * bad block on dev x/y -- not in range
 */
#ifndef	MAPPED_BUFFERS
badblock(fp, bn, dev)
  register struct filsys *fp;	/* fix000 */
  register unsigned bn;	/* fix000 */
{

	if (bn < fp->s_isize+2 || bn >= fp->s_fsize) {
		prdev("bad block", dev);
		return(1);
	}
	return(0);
}
#else	MAPPED_BUFFERS
badblock(bn, dev)
register unsigned bn;	/* fix000 */
{

	if (bn < b.s_isize+2 || bn >= b.s_fsize) {
		prdev("bad block", dev);
		return(1);
	}
	return(0);
}
#endif	MAPPED_BUFFERS

/*
 * Allocate an unused I node
 * on the specified device.
 * Used with file creation.
 * The algorithm keeps up to
 * 100 spare I nodes in the
 * super block. When this runs out,
 * a linear search through the
 * I list is instituted to pick
 * up 100 more.
 */
#ifndef	MAPPED_BUFFERS
ialloc(dev)
{
	register *fp, *bp, *ip;
	int i, j, k, ino;

#ifdef	AUSAML
	if( (fp = u.u_procp->p_lnode) && (fp->l_flags & DLIMIT) ) {
		/* no further allocation for this user */
		u.u_error = EDISKLIM;
		return( NULL );
	}

#endif	AUSAML
	fp = getfs(dev);
	while(fp->s_ilock)
		sleep(&fp->s_ilock, PINOD);
loop:
	if(fp->s_ninode > 0) {
		ino = fp->s_inode[--fp->s_ninode];
		ip = iget(dev, ino);
		if (ip==NULL)
			return(NULL);
		if(ip->i_mode == 0) {
			for(bp = &ip->i_mode; bp < &ip->i_addr[8];)
				*bp++ = 0;
			fp->s_fmod = 1;
			return(ip);
		}
		/*
		 * Inode was allocated after all.
		 * Look some more.
		 */
		iput(ip);
		goto loop;
	}
	fp->s_ilock++;
	ino = 0;
	for(i=0; i<fp->s_isize; i++) {
		bp = bread(dev, i+2);
		ip = bp->b_addr;
		for(j=0; j<256; j=+16) {
			ino++;
			if(ip[j] != 0)
				continue;
			for(k=0; k<NINODE; k++)
			if(dev==inode[k].i_dev && ino==inode[k].i_number)
				goto cont;
			fp->s_inode[fp->s_ninode++] = ino;
			if(fp->s_ninode >= 100)
				break;
		cont:;
		}
		brelse(bp);
		if(fp->s_ninode >= 100)
			break;
	}
	fp->s_ilock = 0;
	wakeup(&fp->s_ilock);
	if (fp->s_ninode > 0)
		goto loop;
	prdev("Out of inodes", dev);
#ifdef	UPRINTS
	uprints("\ncreate error - out of inodes\n");
#endif	UPRINTS
	u.u_error = ENOSPC;
	return(NULL);
}
#else	MAPPED_BUFFERS
/*
 *	ka5 is not preserved by this routine
 */
ialloc(dev)
{
	register struct inode *ip;
	int ka5sav, ino;
	register int i;

#ifdef	AUSAML
	if( (ip = u.u_procp->p_lnode) && (ip->l_flags & DLIMIT) ) {
		/* no further allocation for this user */
		u.u_error = EDISKLIM;
		return( NULL );
	}

#endif	AUSAML
	getfs(dev);
loop:				/* fix026 */
	while(b.s_ilock & S_BUSY)		/* fix026 */
	{
		b.s_ilock =| S_WANTED;		/* fix026 */
		sleep(&b.s_ilock, PINOD);	/* fix026 */
	}
	if(b.s_ninode > 0) {
		ino = b.s_inode[--b.s_ninode];
		ip = iget(dev, ino);
		if (ip==NULL)
			return(NULL);
		if(ip->i_mode == 0) {
			register int *p;

			p =  &ip->i_mode;
			i = 12;		/* &i_addr[8] - &i_mode */
			do *p++ = 0; while( --i);
			b.s_fmod = 1;
			return(ip);
		}
		/*
		 * Inode was allocated after all.
		 * Look some more.
		 */
		iput(ip);
		goto loop;
	}
	b.s_ilock =| S_BUSY;		/* fix026 */
	ka5sav = ka5;
	for(ino = (i = b.s_ilowa) << 4; i < b.s_isize; i++)	/* fix026 */
	{
		register int j;
		int bp;

		bp = breada(dev, i+2, i+3);		/* fix026 */
		for(j=0; j<256; j=+16) {
			bswtch( bp );
			ino++;
			if(b.intarray[j] != 0)
				continue;
			for(ip = inode; ip < &inode[NINODE]; ip++)
			if(dev == ip->i_dev && ino == ip->i_number)
				goto cont;
			ka5 = ka5sav;
			b.s_inode[b.s_ninode++] = ino;
			if(b.s_ninode >= 100)
				break;
		cont:;
		}
		brelse(bp);
		ka5 = ka5sav;
		if(b.s_ninode >= 100)
			break;
	}
	b.s_ilowa = i;				/* fix026 */
	if(b.s_ilock & S_WANTED)		/* fix026 */
	{
		wakeup(&b.s_ilock);		/* fix026 */
	}
	b.s_ilock = 0;
	if (b.s_ninode > 0)
		goto loop;
	prdev("Out of inodes", dev);
#ifdef	UPRINTS
	uprints("\ncreate error - out of inodes\n");
#endif	UPRINTS
	u.u_error = ENOSPC;
	return(NULL);
}
#endif	MAPPED_BUFFERS

/*
 * Free the specified I node
 * on the specified device.
 * The algorithm stores up
 * to 100 I nodes in the super
 * block and throws away any more.
 */
#ifndef	MAPPED_BUFFERS
ifree(dev, ino)
{
	register *fp;

	fp = getfs(dev);
	if(fp->s_ilock)
		return;
	if(fp->s_ninode >= 100)
		return;
	fp->s_inode[fp->s_ninode++] = ino;
	fp->s_fmod = 1;
}
#else	MAPPED_BUFFERS
/*
 *	ka5 is preserved
 */
ifree(dev, ino)
{
	register ka5sav;
	register unsigned bno;

	ka5sav = ka5;
	getfs(dev);
	while( b.s_ilock & S_BUSY )	/* fix026 */
	{
		b.s_ilock =| S_WANTED;
		sleep( &b.s_ilock , PINOD );
	}				/* fix026 */
	if( b.s_ninode < 100 )		/* fix026 */
	{
		b.s_inode[b.s_ninode++] = ino;
		b.s_fmod = 1;
	}
	else				/* fix026 */
	{
		/* see if inode is before s_ilowa */
		if( (bno = (ino - 1) >> 4) < b.s_ilowa)
			b.s_ilowa = bno;
	}				/* fix026 */
	ka5 = ka5sav;
}
#endif	MAPPED_BUFFERS

/*
 * getfs maps a device number into
 * a pointer to the incore super
 * block.
 * The algorithm is a linear
 * search through the mount table.
 * A consistency check of the
 * in core free-block and i-node
 * counts.
 *
 * bad count on dev x/y -- the count
 *	check failed. At this point, all
 *	the counts are zeroed which will
 *	almost certainly lead to "no space"
 *	diagnostic
 * panic: no fs -- the device is not mounted.
 *	this "cannot happen"
 */
#ifndef	MAPPED_BUFFERS
/*
 *	on return ka5 addresses the super block just located
 */
getfs(dev)
{
	register struct mount *p;
	register unsigned n1, n2;	/* fix000 */

	for(p = &mount[0]; p < &mount[NMOUNT]; p++)
	if(p->m_bufp != NULL && p->m_dev == dev) {
		p = p->m_bufp->b_addr;
		n1 = p->s_nfree;
		n2 = p->s_ninode;
		if(n1 > 100 || n2 > 100) {
			prdev("bad count", dev);
			p->s_nfree = 0;
			p->s_ninode = 0;
		}
		return(p);
	}
	panic("no fs");
}
#else	MAPPED_BUFFERS
getfs(dev)
{
	register struct mount *p;

	for(p = &mount[0]; p < &mount[NMOUNT]; p++)
	if(p->m_bufp != NULL && p->m_dev == dev) {
		bswtch( p->m_bufp );
		if( b.s_nfree > 100 || b.s_ninode > 100 ) {
			prdev("bad count", dev);
			b.s_nfree = 0;
			b.s_ninode = 0;
		}
		return b.buff;
	}
	panic("no fs");
}
#endif	MAPPED_BUFFERS

/*
 * update is the internal name of
 * 'sync'. It goes through the disk
 * queues to initiate sandbagged IO;
 * goes through the I nodes to write
 * modified nodes; and it goes through
 * the mount table to initiate modified
 * super blocks.
#ifdef	BETTER_TIME
 *	always rewrite the super block of the root
 *	keep better time
#endif
 */
#ifndef	MAPPED_BUFFERS
update()
{
	register struct inode *ip;
	register struct mount *mp;
	register *bp;

	if(updlock)
		return;
	updlock++;
	for(mp = &mount[0]; mp < &mount[NMOUNT]; mp++)
		if(mp->m_bufp != NULL) {
			ip = mp->m_bufp->b_addr;
#ifdef BETTER_TIME
			if((mp->m_dev != rootdev && ip->s_fmod==0) ||
			ip->s_ilock!=0 ||
#endif
#ifndef BETTER_TIME
			if(ip->s_fmod==0 || ip->s_ilock!=0 ||
#endif
			   ip->s_flock!=0 || ip->s_ronly!=0)
				continue;
			bp = getblk(mp->m_dev, 1);
#ifdef	QMOUNT
			if( bp != mp->m_bufp ) panic("update block ??");
#endif	QMOUNT
			ip->s_fmod = 0;
			ip->s_time = time;	/* fix000 */
#ifndef QMOUNT
			bcopy(ip, bp->b_addr, 256);
#endif	QMOUNT
			bwrite(bp);
		}
	for(ip = &inode[0]; ip < &inode[NINODE]; ip++)
		if( (ip->i_flag&ILOCK) == 0 && ip->i_count != 0 )	/* fix025 */
		{
			ip->i_flag =| ILOCK;
			ip->i_count++;		/* fix025 */
			iupdat(ip, time);
			iput(ip);		/* fix025 */
		}
	updlock = 0;
	bflush(NODEV);
}
#else	MAPPED_BUFFERS
/*
 *	ka5 is not preserved by this routine
 */
update()
{
	register struct inode *ip;
	register struct mount *mp;
	register *bp;

	if(updlock)
		return;
	updlock++;
	for(mp = &mount[0]; mp < &mount[NMOUNT]; mp++)
		if(mp->m_bufp != NULL) {
			bswtch(mp->m_bufp);
#ifdef BETTER_TIME
			if((mp->m_dev != rootdev && b.s_fmod==0) ||
			   (b.s_ilock & S_BUSY) ||
#else
			if(b.s_fmod==0 || (b.s_ilock & S_BUSY) ||
#endif	BETTER_TIME
			   b.s_flock!=0 || b.s_ronly!=0)
				continue;
			bp = getblk(mp->m_dev, 1);
#ifdef	QMOUNT
			if( bp != mp->m_bufp ) panic("update block ??");
#else
			;***** QMOUNT not defined?? ****;
#endif	QMOUNT
			b.s_fmod = 0;
			b.s_time = time;	/* fix000 */
			bwrite(bp);
		}
	for(ip = &inode[0]; ip < &inode[NINODE]; ip++)
		if( (ip->i_flag&ILOCK) == 0 && ip->i_count != 0 )	/* fix025 */
		{						/* fix025 */
			ip->i_flag =| ILOCK;
			ip->i_count ++;		/* fix025 */
			iupdat(ip, time);
			iput(ip);		/* fix025 */
		}
	updlock = 0;
	bflush(NODEV);
}
#endif	MAPPED_BUFFERS
