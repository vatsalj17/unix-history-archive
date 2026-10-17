#
#include "../hd/param.h"
#include "../hd/inode.h"
#include "../hd/user.h"
#include "../hd/buf.h"
#include "../hd/systm.h"

/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 * When convenient, it also leaves the physical
 * block number of the next block of the file in rablock
 * for use in read-ahead.
 */
bmap(ip, bn)
struct inode *ip;
int bn;
{
	register *bp, *bap, nb;
	int *nbp, d, i;

	d = ip->i_dev;
	if(bn & ~03777) {
		u.u_error = EFBIG;
		return(0);
	}

	if((ip->i_mode&ILARG) == 0) {

		/*
		 * small file algorithm
		 */

		if((bn & ~7) != 0) {

			/*
			 * convert small to large
			 */

			if ((bp = alloc(d)) == NULL)
				return(NULL);
			bap = bp->b_addr;
			for(i=0; i<8; i++) {
				*bap++ = ip->i_addr[i];
				ip->i_addr[i] = 0;
			}
			ip->i_addr[0] = bp->b_blkno;
			bdwrite(bp);
			ip->i_mode =| ILARG;
			ip->i_flag =| IUPD;
			goto large;
		}
		nb = ip->i_addr[bn];
		if(nb == 0 && (bp = alloc(d)) != NULL) {
			bdwrite(bp);
			nb = bp->b_blkno;
			ip->i_addr[bn] = nb;
			ip->i_flag =| IUPD;
		}
		rablock = 0;
		if (bn<7)
			rablock = ip->i_addr[bn+1];
		return(nb);
	}

	/*
	 * large file algorithm
	 */

    large:
	i = bn>>8;
	if((nb=ip->i_addr[i]) == 0) {
		if ((bp = alloc(d)) == NULL)
			return(NULL);
		ip->i_addr[i] = bp->b_blkno;
		ip->i_flag =| IUPD;
	} else
		bp = bread(d, nb);
	bap = bp->b_addr;

	/*
	 * normal indirect fetch
	 */

	i = bn & 0377;
	if((nb=bap[i]) == 0 && (nbp = alloc(d)) != NULL) {
		nb = nbp->b_blkno;
		bap[i] = nb;
		bdwrite(nbp);
		bdwrite(bp);
	} else
		brelse(bp);
	rablock = 0;
	if(i < 255)
		rablock = bap[i+1];
	return(nb);
}

/*
 * Pass back  c  to the user at his location u_base;
 * update u_base, u_count, and u_offset.  Return -1
 * on the last character of the user's read.
 * u_base is in the user address space unless u_segflg is set.
 */
passc(c)
char c;
{
	register id;

	if((id = u.u_segflg) == 1)
		*u.u_base = c;
	else
		if(id?suibyte(u.u_base, c):subyte(u.u_base, c) < 0) {
			u.u_error = EFAULT;
			return(-1);
		}
	u.u_count--;
	if(++u.u_offset[1] == 0)
		u.u_offset[0]++;
	u.u_base++;
	return(u.u_count == 0? -1: 0);
}

/*
 * Pick up and return the next character from the user's
 * write call at location u_base;
 * update u_base, u_count, and u_offset.  Return -1
 * when u_count is exhausted.  u_base is in the user's
 * address space unless u_segflg is set.
 */
cpass()
{
	register c, id;

	if(u.u_count == 0)
		return(-1);
	if((id = u.u_segflg) == 1)
		c = *u.u_base;
	else
		if((c = id==0?fubyte(u.u_base):fuibyte(u.u_base)) < 0) {
			u.u_error = EFAULT;
			return(-1);
		}
	u.u_count--;
	if(++u.u_offset[1] == 0)
		u.u_offset[0]++;
	u.u_base++;
	return(c&0377);
}

/*
 * Routine which sets a user error; placed in
 * illegal entries in the bdevsw and cdevsw tables.
 */
nodev()
{

	u.u_error = ENODEV;
}

/*
 * Null routine; placed in insignificant entries
 * in the bdevsw and cdevsw tables.
 */
nulldev()
{
}

/*
 * copy count words from from to to.
 */
bcopy(from, to, count)
int *from, *to;
{
	register *a, *b, c;

	a = from;
	b = to;
	c = count;
	do
		*b++ = *a++;
	while(--c);
}
