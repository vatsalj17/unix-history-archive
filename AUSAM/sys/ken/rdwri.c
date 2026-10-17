#
/*
 */

#include "../defines.h"
#include "../param.h"
#include "../inode.h"
#include "../user.h"
#include "../buf.h"
#include "../conf.h"
#ifdef	AUSAML
#include	"../lnode.h"
#endif	AUSAML
#include "../systm.h"

/*
 * Read the file corresponding to
 * the inode pointed at by the argument.
 * The actual read arguments are found
 * in the variables:
 *	u_base		core address for destination
 *	u_offset	byte offset in file
 *	u_count		number of bytes to read
 *	u_segflg	read to kernel/user
 */
readi(aip)
struct inode *aip;
{
	int *bp;
	int lbn, bn, on;
	register dn, n;
	register struct inode *ip;

	ip = aip;
#ifdef LOCKING
	/* check the locking mechanism */
	if(ip->i_mode & ILPROTOCOL)
		if((u.u_locki != ip) || ((ip->i_lockf & (RLOCK | WLOCK)) == 0))
		{
			u.u_error = EPERM;
			return;
		}
#endif
	if(u.u_count == 0)
		return;
	ip->i_flag =| IACC;
	if((ip->i_mode&IFMT) == IFCHR) {
		(*cdevsw[ip->i_addr[0].d_major].d_read)(ip->i_addr[0]);
		return;
	}

	do {
		lbn = bn = u.u_offset >> 9 ; /* fix000 */
		on = u.u_offset & 0777; /* fix000 */
		n = min(512-on, u.u_count);
		if((ip->i_mode&IFMT) != IFBLK) {
			dn = dpcmp(ip->i_size0&0377, ip->i_size1, u.u_offset); /* fix000 */
			if(dn <= 0)
				return;
			n = min(n, dn);
			if ((bn = bmap(ip, lbn)) == 0)
				return;
			dn = ip->i_dev;
		} else {
			dn = ip->i_addr[0];
			rablock = bn+1;
		}
		if (ip->i_lastr+1 == lbn)
			bp = breada(dn, bn, rablock);
		else
			bp = bread(dn, bn);
		ip->i_lastr = lbn;
		iomove(bp, on, n, B_READ);
		if((ip->i_flag&IPIPE) && (u.u_offset&0777) == 0) /* fix038 */
			bp->b_flags =& ~B_DELWRI;		 /* fix038 */
		brelse(bp);
	} while(u.u_error==0 && u.u_count!=0);
}

/*
 * Write the file corresponding to
 * the inode pointed at by the argument.
 * The actual write arguments are found
 * in the variables:
 *	u_base		core address for source
 *	u_offset	byte offset in file
 *	u_count		number of bytes to write
 *	u_segflg	write to kernel/user
 */
writei(aip)
struct inode *aip;
{
	int *bp;
	int n, on;
	register dn, bn;
	register struct inode *ip;

	ip = aip;
#ifdef LOCKING
	/* check the locking mechanism */
	if(ip->i_mode & ILPROTOCOL)
		if((u.u_locki != ip) || ((ip->i_lockf & WLOCK) == 0))
		{
			u.u_error = EPERM;
			return;
		}
#endif
	ip->i_flag =| IACC|IUPD;
	if((ip->i_mode&IFMT) == IFCHR) {
		(*cdevsw[ip->i_addr[0].d_major].d_write)(ip->i_addr[0]);
		return;
	}
	if (u.u_count == 0)
		return;

	do {
		bn = u.u_offset >> 9; /* fix000 */
		on = u.u_offset & 0777; /* fix000 */
		n = min(512-on, u.u_count);
		if((ip->i_mode&IFMT) != IFBLK) {
			if ((bn = bmap(ip, bn)) == 0)
				return;
			dn = ip->i_dev;
		} else
			dn = ip->i_addr[0];
		if(n == 512) 
			bp = getblk(dn, bn); else
			bp = bread(dn, bn);
		iomove(bp, on, n, B_WRITE);
		if(u.u_error != 0)
			brelse(bp); else
		if((u.u_offset&0777) == 0 && (ip->i_flag&IPIPE) == 0) /* fix000, fix038 */
			bawrite(bp); else
			bdwrite(bp);
		if(dpcmp(ip->i_size0&0377, ip->i_size1, u.u_offset) < 0 && /* fix000 */
		  (ip->i_mode&(IFBLK&IFCHR)) == 0) {
			ip->i_size0 = u.u_offset.hiint; /* fix000 */
			ip->i_size1 = u.u_offset.loint; /* fix000 */
		}
		ip->i_flag =| IUPD;
	} while(u.u_error==0 && u.u_count!=0);
}

/*
 * Return the logical maximum
 * of the 2 arguments.
 */
max(a, b)
unsigned  a, b;	/* fix001 */
{

	if(a > b)
		return(a);
	return(b);
}

/*
 * Return the logical minimum
 * of the 2 arguments.
 */
min(a, b)
unsigned  a, b;	/* fix001 */
{

	if(a < b)
		return(a);
	return(b);
}

/*
 * Move 'an' bytes at byte location
 * &bp->b_addr[o] to/from (flag) the
 * user/kernel (u.segflg) area starting at u.base.
 * Update all the arguments by the number
 * of bytes moved.
 *
 * There are 2 algorithms,
 * if source address, dest address and count
 * are all even in a user copy,
 * then the machine language copyin/copyout
 * is called.
 * If not, its done byte-by-byte with
 * cpass and passc.
 */
iomove(bp, o, an, flag)
struct buf *bp;
{
	register char *cp;
	register int n, t;

	n = an;
#ifndef	MAPPED_BUFFERS
	cp = bp->b_addr + o;
#else	MAPPED_BUFFERS
	if( bp->b_flags & B_PHYS )
		cp = bp->b_addr + o;
	else {
		cp = b.buff + o;
		bswtch( bp );
	}
#endif	MAPPED_BUFFERS
#ifndef	_1170 | _1145
	if(u.u_segflg==0 && ((n | cp | u.u_base)&01)==0) {
#endif
#ifdef	_1170 | _1145
	if( ((n | cp | u.u_base)&01)==0 ) {
#endif
		if (flag==B_WRITE)
#ifndef	_1170 | _1145
			cp = copyin(u.u_base, cp, n);
#endif
#ifdef	_1170 | _1145
			cp = copyin(u.u_base, cp, n, u.u_segflg);
#endif
		else
#ifndef	_1170 | _1145
			cp = copyout(cp, u.u_base, n);
#endif
#ifdef	_1170 | _1145
			cp = copyout(cp, u.u_base, n, u.u_segflg);
#endif
		if (cp) {
			u.u_error = EFAULT;
			return;
		}
		u.u_base =+ n;
		u.u_offset =+ n.unsignd; /* fix000 */
		u.u_count =- n;
		return;
	}
	if (flag==B_WRITE) {
		while(n--) {
			if ((t = cpass()) < 0)
				return;
			*cp++ = t;
		}
	} else
		while (n--)
			if(passc(*cp++) < 0)
				return;
}
