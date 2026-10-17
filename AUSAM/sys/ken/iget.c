#
#include "../defines.h"
#include "../param.h"
#include "../file.h"
#ifdef	AUSAML
#include	"../lnode.h"
#endif	AUSAML
#include "../systm.h"
#include "../user.h"
#include "../inode.h"
#include "../filsys.h"
#include "../conf.h"
#include "../buf.h"

/*
 * Look up an inode by device,inumber.
 * If it is in core (in the inode structure),
 * honor the locking protocol.
 * If it is not in core, read it in from the
 * specified device.
 * If the inode is mounted on, perform
 * the indicated indirection.
 * In all cases, a pointer to a locked
 * inode structure is returned.
 *
 * printf warning: no inodes -- if the inode
 *	structure is full
 * panic: no imt -- if the mounted file
 *	system is not in the mount table.
 *	"cannot happen"
 */
#ifdef	MAPPED_BUFFERS
/*
 *	ka5 must be preserved
 */
#endif	MAPPED_BUFFERS
iget(dev, ino)
{
	register struct inode *p;
#ifndef	MAPPED_BUFFERS
	register *ip2;
	int *ip1;
#else	MAPPED_BUFFERS
	register unsigned ka5sav;
#endif	MAPPED_BUFFERS
	register struct mount *ip;

loop:
	ip = NULL;
	for(p = &inode[0]; p < &inode[NINODE]; p++) {
		if(dev==p->i_dev && ino==p->i_number) {
			if((p->i_flag&ILOCK) != 0) {
				p->i_flag =| IWANT;
				sleep(p, PINOD);
				goto loop;
			}
			if((p->i_flag&IMOUNT) != 0) {
				for(ip = &mount[0]; ip < &mount[NMOUNT]; ip++)
				if(ip->m_inodp == p) {
					dev = ip->m_dev;
					ino = ROOTINO;
					goto loop;
				}
				panic("no imt");
			}
			p->i_count++;
			p->i_flag =| ILOCK;
			return(p);
		}
#ifndef	LRU_INODE
		if(ip==NULL && p->i_count==0)
			ip = p;
#endif	LRU_INODE
#ifdef	LRU_INODE
		if(p->i_count==0)
			if( (ip==NULL) || (ip->i_lrt > p->i_lrt) )
				ip = p;
#endif	LRU_INODE
	}
	if((p=ip) == NULL) {
		printf("Inode table overflow\n");
		u.u_error = ENFILE;
		return(NULL);
	}
	p->i_dev = dev;
	p->i_number = ino;
	p->i_flag = ILOCK;
	p->i_count++;
	p->i_lastr = -1;
#ifdef LOCKING
	p->i_lockf = p->i_lockc = 0;
#endif
	ip = bread(dev, ldiv(ino+31,16));
	/*
	 * Check I/O errors
	 */
	if (ip->b_flags&B_ERROR) {
		brelse(ip);
		iput(p);
		return(NULL);
	}
#ifndef	MAPPED_BUFFERS
	ip1 = ip->b_addr + 32*lrem(ino+31, 16);
	ip2 = &p->i_mode;
	while(ip2 < &p->i_addr[8])
		*ip2++ = *ip1++;
#else	MAPPED_BUFFERS
	ka5sav = ka5;
	bswtch( ip );
	bcopy( b.buff + 32*lrem(ino+31, 16) , &p->i_mode , 12 );
	ka5 = ka5sav;
#endif	MAPPED_BUFFERS
	brelse(ip);
	return(p);
}

/*
 * Decrement reference count of
 * an inode structure.
 * On the last reference,
 * write the inode out and if necessary,
 * truncate and deallocate the file.
#ifdef	MAPPED_BUFFERS
 *
 * KA5 to be preserved
 *
#endif	MAPPED_BUFFERS
 */
iput(p)
struct inode *p;
{
	register *rp;
#ifdef	MAPPED_BUFFERS
	register ka5sav = ka5;
#endif	MAPPED_BUFFERS

	rp = p;
	if(rp->i_count == 1) {
		rp->i_flag =| ILOCK;
		if(rp->i_nlink <= 0) {
			itrunc(rp);
			rp->i_mode = 0;
			rp->i_flag =| IUPD;			/* fix025 */
			ifree(rp->i_dev, rp->i_number);
		}
		iupdat(rp, time);
		prele(rp);
		rp->i_flag = 0;
#ifndef	LRU_INODE
		rp->i_number = 0;
#endif	LRU_INODE
#ifdef	LRU_INODE
		rp->i_lrt = time.loint;
#endif	LRU_INODE
	}
	rp->i_count--;
	prele(rp);
#ifdef	MAPPED_BUFFERS
	ka5 = ka5sav;
#endif	MAPPED_BUFFERS
}

/*
 * Check accessed and update flags on
 * an inode structure.
 * If either is on, update the inode
 * with the corresponding dates
 * set to the argument tm.
#ifdef	MAPPED_BUFFERS
 *
 * KA5 must be preserved
 *
#endif	MAPPED_BUFFERS
 */
iupdat(p, tm)
int *p;
long tm;	/* fix000 */
{
	register *ip1, *ip2, *rp;
	int *bp, i;
#ifdef	MAPPED_BUFFERS
	int ka5sav = ka5;
#endif	MAPPED_BUFFERS

	rp = p;
	if((rp->i_flag&(IUPD|IACC)) != 0) {
		if(getfs(rp->i_dev)->s_ronly)
#ifdef	MAPPED_BUFFERS
			goto ret;
#else	MAPPED_BUFFERS
			return;
#endif	MAPPED_BUFFERS
		i = rp->i_number+31;
		bp = bread(rp->i_dev, ldiv(i,16));
#ifdef	MAPPED_BUFFERS
		bswtch( bp );
		ip1 = b.buff + 32*lrem(i, 16);
#else	MAPPED_BUFFERS
		ip1 = bp->b_addr + 32*lrem(i, 16);
#endif	MAPPED_BUFFERS
		ip2 = &rp->i_mode;
		while(ip2 < &rp->i_addr[8])
			*ip1++ = *ip2++;
		if(rp->i_flag&IACC) {
			*ip1++ = time.hiint;		/* fix000 */
			*ip1++ = time.loint;		/* fix000 */
		} else
			ip1 =+ 2;
		if(rp->i_flag&IUPD) {
			*ip1++ = tm.hiint;		/* fix000 */
			*ip1++ = tm.loint;		/* fix000 */
		}
		rp->i_flag =& ~(IUPD | IACC);		/* fix025 */
		bdwrite(bp);				/* fix025 */
	}
#ifdef	MAPPED_BUFFERS
 ret:
	ka5 = ka5sav;
#endif	MAPPED_BUFFERS
}

/*
 * Free all the disk blocks associated
 * with the specified inode structure.
 * The blocks of the file are removed
 * in reverse order. This FILO
 * algorithm will tend to maintain
 * a contiguous free list much longer
 * than FIFO.
 */
itrunc(ip)
int *ip;
{
	register *rp, *bp, *cp;
	int *dp, *ep;

	rp = ip;
	if((rp->i_mode&(IFCHR&IFBLK)) != 0)
		return;
	for(ip = &rp->i_addr[7]; ip >= &rp->i_addr[0]; ip--)
	if(*ip) {
		if((rp->i_mode&ILARG) != 0) {
			bp = bread(rp->i_dev, *ip);
#ifdef	MAPPED_BUFFERS
			bswtch( bp );
	/* fix025 */	for(cp = b.buff+510; cp >= b.buff; cp--)
#else	MAPPED_BUFFERS
	/* fix025 */	for(cp = bp->b_addr+510; cp >= bp->b_addr; cp--)
#endif	MAPPED_BUFFERS
			if(*cp) {
				if(ip == &rp->i_addr[7]) {
#ifdef	MAPPED_BUFFERS
					int ka5sav;

#endif	MAPPED_BUFFERS
					dp = bread(rp->i_dev, *cp);
#ifdef	MAPPED_BUFFERS
					ka5sav = ka5;
					bswtch( dp );
			/* fix025 */	for(ep = b.buff+510; ep >= b.buff; ep--)
#else	MAPPED_BUFFERS
			/* fix025 */	for(ep = dp->b_addr+510; ep >= dp->b_addr; ep--)
#endif	MAPPED_BUFFERS
					if(*ep)
						free(rp->i_dev, *ep);
#ifdef	MAPPED_BUFFERS
					ka5 = ka5sav;
#endif	MAPPED_BUFFERS
					brelse(dp);
				}
				free(rp->i_dev, *cp);
			}
			brelse(bp);
		}
		free(rp->i_dev, *ip);
		*ip = 0;
	}
	rp->i_mode =& ~ILARG;
	rp->i_size0 = 0;
	rp->i_size1 = 0;
	rp->i_flag =| IUPD;
}

/*
 * Make a new file.
 */
maknode(mode)
{
	register *ip;

	ip = ialloc(u.u_pdir->i_dev);
	if (ip==NULL)
	{
		iput( u.u_pdir );		/* fix025 */
		return(NULL);
	}
	ip->i_flag =| IACC|IUPD;
	ip->i_mode = mode|IALLOC;
	ip->i_nlink = 1;
#ifndef	AUSAM16
	ip->i_uid = u.u_uid;
	ip->i_gid = u.u_gid;
#endif	AUSAM16
#ifdef	AUSAM16
	ip->i_uidl = u.u_uid.lobyte;
	ip->i_uidh = u.u_uid.hibyte;
#endif	AUSAM16
	wdir(ip);
	return(ip);
}

/*
 * Write a directory entry with
 * parameters left as side effects
 * to a call to namei.
 */
wdir(ip)
int *ip;
{
	register char *cp1, *cp2;

	u.u_dent.u_ino = ip->i_number;
	cp1 = &u.u_dent.u_name[0];
	for(cp2 = &u.u_dbuf[0]; cp2 < &u.u_dbuf[DIRSIZ];)
		*cp1++ = *cp2++;
	u.u_count = DIRSIZ+2;
	u.u_segflg = SEG_KER;
	u.u_base = &u.u_dent;
	writei(u.u_pdir);
	iput(u.u_pdir);
}
