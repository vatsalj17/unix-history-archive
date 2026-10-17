#
#include "../hd/param.h"
#include "../hd/systm.h"
#include "../hd/reg.h"
#include "../hd/buf.h"
#include "../hd/filsys.h"
#include "../hd/user.h"
#include "../hd/inode.h"
#include "../hd/file.h"
#include "../hd/conf.h"
#include "../hd/var.h"

/*
 * the fstat system call.
 */
fstat()
{
	register *fp, *base;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	base	= u.u_arg[0];
	stat1(fp->f_inode, base);
	if (fp->f_flag&FPIPE)		/* correct if pipe */
		suword(&base[5],fp->f_inode->i_size1-fp->f_offset[1]);
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

	iupdat(ip);
	bp = bread(ip->i_dev, ldiv(ip->i_number+31, 16));
	cp = bp->b_addr + 32*lrem(ip->i_number+31, 16) + 24;
	ip = &(ip->i_dev);
	for(i=0; i<14; i++) {
		if (suword(ub, *ip) != *ip++)
			u.u_error = EFAULT;
		ub =+ 2;
	}
	for(i=0; i<4; i++) {
		if (suword(ub, *cp) != *cp++)
			u.u_error = EFAULT;
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
	u.u_ofile[i] = fp;
	fp->f_count++;
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

	if(!suser())
		return;
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
	for(mp = &mount[0]; mp < v.ve_mount; mp++) {
		if(mp->m_bufp != NULL) {
			if(d == mp->m_dev)
				goto out;
		} else
		if(smp == NULL)
			smp = mp;
	}
	if(smp == NULL)
		goto out;
	(*bdevsw[major(d)].d_open)(minor(d), !u.u_arg[2]);
	if(u.u_error)
		goto out;
	mp = bread(d, 1);
	if(u.u_error) {
		brelse(mp);
		goto out1;
	}
	smp->m_inodp = ip;
	smp->m_dev = d;
	smp->m_bufp = getblk(NODEV);
	bcopy(mp->b_addr, smp->m_bufp->b_addr, 256);
	smp = smp->m_bufp->b_addr;
	smp->s_ilock = 0;
	smp->s_flock = 0;
	smp->s_ninode = 0;
	smp->s_ronly = u.u_arg[2] & 1;
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

	if(!suser())
		return;
	update();
	d = getmdev();
	if(u.u_error)
		return;
	for(mp = &mount[0]; mp < v.ve_mount; mp++)
		if(mp->m_bufp!=NULL && d==mp->m_dev)
			goto found;
	u.u_error = EINVAL;
	return;

found:
	for(ip = &inode[0]; ip < v.ve_inode; ip++)
		if(ip->i_number!=0 && d==ip->i_dev) {
			u.u_error = EBUSY;
			return;
		}
	(*bdevsw[major(d)].d_close)(minor(d), 0);
	ip = mp->m_inodp;
	ip->i_flag =& ~IMOUNT;
	plock(ip);
	iput(ip);
	ip = mp->m_bufp;
	mp->m_bufp = NULL;
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
		return(NODEV);
	if((ip->i_mode&IFMT) != IFBLK)
		u.u_error = ENOTBLK;
	d = ip->i_addr[0];
	if(major(ip->i_addr[0]) >= bdevcnt)
		u.u_error = ENXIO;
	iput(ip);
	return(d);
}
