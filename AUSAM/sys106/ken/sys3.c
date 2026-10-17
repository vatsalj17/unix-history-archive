#
/*
 */

#include "../defines.h"
#include "../param.h"
#ifdef	AUSAML
#include	"../lnode.h"
#endif	AUSAML
#include "../systm.h"
#include "../reg.h"
#include "../buf.h"
#include "../filsys.h"
#include "../file.h"
#include "../user.h"
#include "../inode.h"
#include "../conf.h"

/*
 * the fstat system call.
 */
fstat()
{
	register *fp;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	stat1(fp->f_inode, u.u_arg[0]);
}

/*
 * the stat system call.
 */
stat()
{
	register ip;
	extern uchar;

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	stat1(ip, u.u_arg[1]);
	iput(ip);
}

/*
 * The basic routine for fstat and stat:
 * get the inode and pass appropriate parts back.
 */
stat1(ip, ub)
int *ip;
{
	register i, *bp, *cp;

	iupdat(ip, time);
	bp = bread(ip->i_dev, ldiv(ip->i_number+31, 16));
#ifndef	MAPPED_BUFFERS
	cp = bp->b_addr + 32*lrem(ip->i_number+31, 16) + 24;
#else	MAPPED_BUFFERS
	cp = b.buff + 32*lrem(ip->i_number+31, 16) + 24;
	bswtch( bp );
#endif	MAPPED_BUFFERS
	ip = &(ip->i_dev);
	for(i=0; i<14; i++) {
		suword(ub, *ip++);	/* fix037 supercedes fix025 */
		ub =+ 2;
	}
	for(i=0; i<4; i++) {
		suword(ub, *cp++);	/* fix037 supercedes fix025 */
		ub =+ 2;
	}
	brelse(bp);
}

/*
 * the dup system call.
 */
dup()
{
	register i, *fp;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	if ((i = ufalloc()) < 0)
		return;
#ifndef	LARGE_FILE_REFERENCES
	u.u_ofile[i] = fp;
	fp->f_count++;
#endif
#ifdef	LARGE_FILE_REFERENCES
	if (++fp->f_count == 0)  {
		u.u_error = ENREF;
		fp->f_count--;
		return;
	}
	u.u_ofile[i] = fp;
#endif
}

/*
 * the mount system call.
 */
smount()
{
	int d;
	register *ip;
	register struct mount *mp, *smp;
	extern uchar;

	if(!suser())		/* fix039 */
		return;		/* fix039 */
	d = getmdev();
	if(u.u_error)
		return;
	u.u_dirp = u.u_arg[1];
	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	if(ip->i_count!=1 || (ip->i_mode&(IFBLK&IFCHR))!=0)
		goto out;
	smp = NULL;
	for(mp = &mount[0]; mp < &mount[NMOUNT]; mp++) {
		if(mp->m_bufp != NULL) {
			if(d == mp->m_dev)
				goto out;
		} else
		if(smp == NULL)
			smp = mp;
	}
	if(smp == NULL)
		goto out;
	(*bdevsw[d.d_major].d_open)(d, !u.u_arg[2]);
	if(u.u_error)
		goto out;
	mp = bread(d, 1);
	if(u.u_error) {
		brelse(mp);
		goto out1;
	}
#ifdef	QMOUNT
	mp->b_flags =| B_MOUNT;
#endif	QMOUNT
	smp->m_inodp = ip;
	smp->m_dev = d;
#ifdef QMOUNT
	smp->m_bufp = mp;
#else
	smp->m_bufp = getblk(NODEV);
	bcopy(mp->b_addr, smp->m_bufp->b_addr, 256);
#endif QMOUNT
#ifndef	MAPPED_BUFFERS
	smp = smp->m_bufp->b_addr;
	smp->s_ilock = 0;
	smp->s_flock = 0;
	smp->s_ronly = u.u_arg[2] & 1;
#else	MAPPED_BUFFERS
	bswtch( mp );
	b.s_ilock = b.s_flock = 0;
	b.s_ronly = u.u_arg[2] & 1;
	b.s_ilowa = 0;		/* fix026 */
#endif	MAPPED_BUFFERS
	brelse(mp);
	ip->i_flag =| IMOUNT;
	prele(ip);
	return;

out:
	u.u_error = EBUSY;
out1:
	iput(ip);
}

/*
 * the umount system call.
 */
sumount()
{
	int d;
	register struct inode *ip;
	register struct mount *mp;

	if(!suser())		/* fix039 */
		return;		/* fix039 */
	update();
	d = getmdev();
	if(u.u_error)
		return;
	for(mp = &mount[0]; mp < &mount[NMOUNT]; mp++)
		if(mp->m_bufp!=NULL && d==mp->m_dev)
			goto found;
	u.u_error = EINVAL;
	return;

found:
	for(ip = &inode[0]; ip < &inode[NINODE]; ip++)
#ifndef	LRU_INODE
		if(ip->i_number!=0 && d==ip->i_dev) {
			u.u_error = EBUSY;
			return;
		}
#endif	LRU_INODE
#ifdef	LRU_INODE
		if(d==ip->i_dev)
			if(ip->i_count) {
				u.u_error = EBUSY;
				return;
			} else
				ip->i_number = 0;
#endif	LRU_INODE
	(*bdevsw[d.d_major].d_close)(d, 0);
	binval( d );	/* fix030 */
	ip = mp->m_inodp;
	ip->i_flag =& ~IMOUNT;
	plock(ip);			/* fix025 */
	iput(ip);
	ip = mp->m_bufp;
	mp->m_bufp = NULL;
#ifdef QMOUNT
	ip->b_flags =& ~B_MOUNT;
#endif QMOUNT
	brelse(ip);
}

/*
 * Common code for mount and umount.
 * Check that the user's argument is a reasonable
 * thing on which to mount, and return the device number if so.
 */
getmdev()
{
	register d, *ip;
	extern uchar;

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return NODEV;			/* fix025 */
	if((ip->i_mode&IFMT) != IFBLK)
		u.u_error = ENOTBLK;
	d = ip->i_addr[0];
	if(ip->i_addr[0].d_major >= nblkdev)
		u.u_error = ENXIO;
	iput(ip);
	return(d);
}


#ifdef LOCKING
/*
 * Locking facilities for a file...
 */

slock()		/* lock system call */
{
	register struct file *fp;

	switch(u.u_arg[0])
	{
    case 0:

		fp = getf(u.u_ar0[R0]);
		if(fp != NULL)
		{
			if((fp->f_flag & FREAD) == 0)
				u.u_error = EPERM;
			else
				lock(fp->f_inode,RLOCK);
		}
		return;
    case 1:
		fp = getf(u.u_ar0[R0]);
		if(fp != NULL)
		{
			if((fp->f_flag & FWRITE) == 0)
				u.u_error = EPERM;
			else
				lock(fp->f_inode,WLOCK);
		}
		return;
    case 2:	unlock(); break;
	}
}

unlock()
{
	register struct inode *ip;

	if((ip = u.u_locki) == NULL)
	{
		u.u_error = EINVAL;
		return;
	}
	if(u.u_lockf & RLOCK)
	{
		ip->i_lockc --;
		if(ip->i_lockc == 0)
		{
			ip->i_lockf =& ~RLOCK;
			if(ip->i_lockf & WREQD)
				wakeup(&ip->i_lockc);
		}
	}
	else if(u.u_lockf & (WREQD | WLOCK))
	     {
		ip->i_lockf =& ~u.u_lockf;
		if(ip->i_lockf & WAITING)
			wakeup(&ip->i_lockf);
	     }
	u.u_locki = NULL;
	u.u_lockf = 0;
}

lock(ip, type)
register struct inode *ip;
register int type;
{
	if(!(ip->i_mode & ILPROTOCOL) || ((ip->i_mode & IFMT) == IFDIR))
	{
		u.u_error = EBADF;
		return;
	}
	if(u.u_locki != NULL)
		unlock();
	while(ip->i_lockf & (WLOCK | WREQD))
	{
		ip->i_lockf =| WAITING;
		sleep(&ip->i_lockf,PLOCK);
	}
	u.u_locki = ip;
	switch(type)
	{
	case RLOCK:
		u.u_lockf = RLOCK;
		ip->i_lockf =| RLOCK;
		ip->i_lockc ++;
		break;
	case WLOCK:
		while(ip->i_lockf & RLOCK)
		{
			u.u_lockf = WREQD;
			ip->i_lockf =| WREQD;
			sleep(&ip->i_lockc, PLOCK);
		}
		u.u_lockf = WLOCK;
		ip->i_lockf = (ip->i_lockf & ~WREQD) | WLOCK;
		break;
	}
}
#endif
