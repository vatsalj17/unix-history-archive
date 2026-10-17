/*
 * $Header: /v/src/rcskrnl/coh.386/RCS/fs2.c,v 420.7 1993/12/10 22:12:24 srcadm Exp srcadm $
 */

/*********************************************************************
 *
 * Coherent, Mark Williams Company
 * RCS Header
 * This file contains proprietary information and is considered
 * a trade secret.  Unauthorized use is prohibited.
 *
 *
 * $Id: fs2.c,v 420.7 1993/12/10 22:12:24 srcadm Exp srcadm $
 *
 * $Log: fs2.c,v $
 * Revision 420.7  1993/12/10  22:12:24  srcadm
 * Updated change to bclaim() arguments.
 *
 * Revision 420.6  1993/12/04  00:06:29  srcadm
 * Checks error on writing superblock and reports same to console.
 *
 * Revision 420.5  1993/12/01  23:34:43  srcadm
 * Initial RCS submission
 *
 *
 */

/* Embedded Version Constant */
char *MWC_FS2_VERSION = "MWC_FS2_VERSION($Revision: 420.7 $)";

/*
 * Filesystem (disk inodes).
 *
 * Revision 2.5  93/10/29  00:55:13  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.4  93/08/19  03:26:28  nigel
 * Nigel's r83 (Stylistic cleanup)
 * 
 * Revision 2.2  93/07/26  15:19:24  nigel
 * Nigel's R80
 */

#include <common/ccompat.h>
#include <kernel/proc_lib.h>
#include <sys/cmn_err.h>
#include <sys/debug.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/kmem.h>
#include <sys/cred.h>
#include <stddef.h>

#define	_KERNEL		1

#include <kernel/trace.h>
#include <sys/uproc.h>
#include <sys/acct.h>
#include <sys/buf.h>
#include <canon.h>
#include <sys/con.h>
#include <sys/filsys.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/io.h>
#include <sys/mount.h>
#include <sys/proc.h>

#define _INODE_BUSY_DUMP 1

/*
 * Globals used in filesystem code for iterating over inodes.
 */

struct inode  *	inode_table;
struct inode  *	inode_table_end;


/*
 * Initialise filesystem.
 */

void
fsminit()
{
	MOUNT	      *	mp;
	struct inode  *	ip;

	/*
	 * NIGEL: We begin by setting up all the inodes in the system.
	 */

	inode_table_end = inode_table + NINODE;

	for (ip = inode_table ; ip != inode_table_end ; ip ++) {
		ip->i_refc = 0;
		__INIT_INODE_LOCK (ip);
	}


	/*
	 * Mount the root file system.
	 */

	if ((mp = fsmount (rootdev, ronflag)) == NULL)
		cmn_err (CE_PANIC, "fsminit: no rootdev(%d,%d)",
			 major (rootdev), minor (rootdev));

	/*
	 * Set system time from the super block.
	 */
	timer.t_time = mp->m_super.s_time;

	/*
	 * Access the root directory.
	 */

	if ((u.u_rdir = iattach (rootdev, ROOTIN)) == NULL)
		cmn_err (CE_PANIC, "fsminit: no / on rootdev(%d,%d)",
			 major (rootdev), minor (rootdev));

	/*
	 * Record current directory.
	 */

	u.u_cdir = u.u_rdir;
	u.u_cdir->i_refc ++;
	iunlock (u.u_rdir);
}


/*
 * Canonicalze a super block.
 */

void
cansuper(fsp)
struct filsys *fsp;
{
	int i;

	canint (fsp->s_isize);
	candaddr (fsp->s_fsize);
	canshort (fsp->s_nfree);
	for (i = 0 ; i < NICFREE ; i ++)
		candaddr (fsp->s_free [i]);
	canshort (fsp->s_ninode);
	for (i = 0 ; i < NICINOD ; i ++)
		canino (fsp->s_inode [i]);
	cantime (fsp->s_time);
	candaddr (fsp->s_tfree);
	canino (fsp->s_tinode);
	canshort (fsp->s_m);
	canshort (fsp->s_n);
	canlong (fsp->s_unique);
}


/*
 * Mount the given device.
 */

MOUNT *
fsmount (dev, f)
dev_t dev;
int f;
{
	MOUNT *mp;
	buf_t	      *	bp;

	if ((mp = kmem_alloc (sizeof (* mp), KM_NOSLEEP)) == NULL) {
		cmn_err (CE_NOTE, "fsmount(%x,%x): kalloc failed ", dev, f);
		return NULL;
	}

	(void) dopen (dev, f ? IPR : IPR | IPW, DFBLK, NULL);

	if (get_user_error ()) {
		cmn_err (CE_NOTE, "fsmount(%x,%x): dopen failed %d\n",
			 dev, f, get_user_error ());
		kmem_free (mp, sizeof (* mp));
		return NULL;
	}

	if ((bp = bread (dev, (daddr_t) SUPERI, BUF_SYNC)) == NULL) {
		dclose (dev, f ? IPR : IPR | IPW, DFBLK, NULL);
		kmem_free (mp, sizeof (* mp));
		return NULL;
	}
	memcpy (& mp->m_super, bp->b_vaddr, sizeof (mp->m_super));
	brelease (bp);
	cansuper (& mp->m_super);

	mp->m_ip = NULL;
	mp->m_dev = dev;
	mp->m_flag = f;
	mp->m_super.s_fmod = 0;
	mp->m_next = mountp;

	__INIT_MOUNT_LOCK (mp);

	mountp = mp;
	return mp;
}


/********************************************************************
 * Function: void msync(MOUNT *mp)
 *
 * Description: Given a pointer to a mount entry, write out all
 *		inodes on that device.
 *
 * Returns: Nothing.
 *
 * Affects Globals: None.
 *
 * Affects U-Area: No.
 *
 * Side Effects: Updates time and mod flag in mp->m_super.
 *
 * Comments: Warns user if superblock cannot be written.
 */

void
msync (mp)
MOUNT *mp;
{
	struct filsys *	sbp;
	buf_t	      *	bp;

	if ((mp->m_flag & MFRON) != 0)
		return;

	isync (mp->m_dev);

	sbp = & mp->m_super;

	/*
	 * Only write superblock if it is marked as modified.
	 */
	if (sbp->s_fmod == 0)
		return;

	bp = bclaim (mp->m_dev, (daddr_t) SUPERI, BSIZE, BUF_SYNC);

	sbp->s_time = timer.t_time;
	sbp->s_fmod = 0;

	memcpy (bp->b_vaddr, sbp, sizeof (* sbp));
	cansuper (bp->b_vaddr);

	bwrite (bp, BUF_SYNC);
	if(bp->b_flags & BFERR)
		cmn_err(CE_WARN,
		   "msync: Cannot write superblock on device (%d, %d)!\n",
		   major(mp->m_dev), minor(mp->m_dev));
	brelease (bp);
}


/*
 * Return the mount entry for the given device.  If `f' is not set
 * and the device is read only, don't set the error status.
 */

MOUNT *
getment (dev, f)
dev_t dev;
int f;
{
	MOUNT * mp;

	for (mp = mountp ; mp != NULL ; mp = mp->m_next) {
		if (mp->m_dev != dev)
			continue;
		if ((mp->m_flag & MFRON) != 0) {
			if (f != 0)
				set_user_error (EROFS);
			return NULL;
		}
		return mp;
	}

	/*
	 * Contrary to one's intuition, this CAN happen; if pipedev is not
	 * the same as rootdev, and the device that pipes are meant to live
	 * on is not mounted, this will fail.
	 */

	return NULL;
}


/*
 * Allocate a new inode with the given mode.  The returned inode is locked.
 */

#if	__USE_PROTO__
struct inode * ialloc (o_dev_t dev, unsigned mode, cred_t * credp)
#else
struct inode *
ialloc (dev, mode, credp)
o_dev_t dev;
unsigned mode;
cred_t	      *	credp;
#endif
{
	struct dinode *dip;
	struct filsys *sbp;
	ino_t *inop;
	ino_t ino;
	buf_t	      *	bp;
	daddr_t b;
	struct dinode *dipe;
	ino_t *inope;
	MOUNT *mp;
	INODE *ip;
#if _INODE_BUSY_DUMP
	int	eninode, etinode;
	int	lninode, ltinode;
	int	xninode, xtinode;
#endif

	if ((mp = getment (dev, 1)) == NULL)
		return NULL;
	sbp = & mp->m_super;

#if _INODE_BUSY_DUMP
	eninode = sbp->s_ninode;
	etinode = sbp->s_tinode;
#endif

	for (;;) {
		__LOCK_FREE_INODE_LIST (mp, "ialloc () #1");

#if _INODE_BUSY_DUMP
		lninode = sbp->s_ninode;
		ltinode = sbp->s_tinode;
#endif

		if (sbp->s_ninode == 0) {
			isync (dev);
			ino = 1;
			inop = sbp->s_inode;
			inope = sbp->s_inode + NICINOD;
			for (b = INODEI ; b < sbp->s_isize ; b ++) {
				if (bad (dev, b)) {
					ino += INOPB;
					continue;
				}
				if ((bp = bread (dev, b, BUF_SYNC)) == NULL) {
					/*
					 * Let's tell people about the problem.
					 */

					cmn_err (CE_WARN,
						 "(%d,%d): Cannot read block %ld",
						 major (dev), minor (dev), b);
					ino += INOPB;
					continue;
				}
				dip = (struct dinode *) bp->b_vaddr;
				dipe = dip + INOPB;
				for (; dip < dipe ; dip ++, ino ++) {
					if (dip->di_mode != 0)
						continue;
					if (inop >= inope)
						break;
					* inop ++ = ino;
				}
				brelease (bp);
				if (inop >= inope)
					break;
			}
			sbp->s_ninode = inop - sbp->s_inode;
			if (sbp->s_ninode == 0) {
				sbp->s_tinode = 0;

				__UNLOCK_FREE_INODE_LIST (mp);
				devmsg (dev, "Out of inodes");
				set_user_error (ENOSPC);
				return NULL;
			}
		}

#if _INODE_BUSY_DUMP
		xninode = sbp->s_ninode;
		xtinode = sbp->s_tinode;
#endif

		ino = sbp->s_inode [-- sbp->s_ninode];
		-- sbp->s_tinode;
		sbp->s_fmod = 1;
		__UNLOCK_FREE_INODE_LIST (mp);

		if ((ip = iattach (dev, ino)) != NULL) {
			if (ip->i_mode != 0) {
				devmsg (dev, "Inode %u busy", ino);

#if _INODE_BUSY_DUMP
printf("%x %x rf=%x fl=%x md=%x nl=%x en=%x et=%x ln=%x lt=%x xn=%x xt=%x n=%x t=%x\n",
	mode, ino, ip->i_refc, ip->i_flag, ip->i_mode, ip->i_nlink,
	eninode, etinode, lninode, ltinode, xninode, xtinode,
	sbp->s_ninode, sbp->s_tinode);
#endif
				idetach (ip);

				__LOCK_FREE_INODE_LIST (mp, "ialloc () #2");
				++ sbp->s_tinode;
				sbp->s_fmod = 1;
				__UNLOCK_FREE_INODE_LIST (mp);

				continue;
			}
			ip->i_flag = 0;
			ip->i_mode = mode;
			ip->i_nlink = 0;

			ip->i_uid = credp->cr_uid;
			ip->i_gid = credp->cr_gid;
		}
		return ip;
	}
}


/*
 * Free the inode `ino' on device `dev'.
 */

#if	__USE_PROTO__
void ifree (o_dev_t dev, o_ino_t ino)
#else
void
ifree(dev, ino)
o_dev_t dev;
o_ino_t ino;
#endif
{
	struct filsys *sbp;
	MOUNT *mp;

	if ((mp = getment(dev, 1)) == NULL)
		return;

	__LOCK_FREE_INODE_LIST (mp, "ifree ()");

	sbp = & mp->m_super;
	sbp->s_fmod = 1;
	if (sbp->s_ninode < NICINOD)
		sbp->s_inode [sbp->s_ninode ++] = ino;
	sbp->s_tinode ++;

	__UNLOCK_FREE_INODE_LIST (mp);
}


/*
 * Flag to say whether we should try and keep the free-block list sorted.
 * See conf/cohmain/Space.c
 */

extern int	t_sortblocks;

/*
 * If we are sorting blocks, this routine can be used to keep blocks in sorted
 * order. Given a fixed-size list in reverse rank order, this routine returns
 * the new element if it is smaller than all others, or the smallest element
 * after the new element has been inserted in position.
 */

#if	__USE_PROTO__
__LOCAL__ daddr_t daddr_add (daddr_t * list, int count, daddr_t newblock)
#else
__LOCAL__ daddr_t
daddr_add (list, count, newblock)
daddr_t	      *	list;
int		count;
daddr_t		newblock;
#endif
{
	daddr_t	      *	end = list + count;

	ASSERT (count >= 0);

	while (list != end) {
		if (newblock == * list)
			return 0;

		if (newblock > * list ++) {
			daddr_t	      temp;
			/*
			 * We have found the insertion point... move all the
			 * elements from (list - 1) to (end - 1) up, and put
			 * the new element in place.
			 */

			temp = * -- end;
			list --;

			while (list != end) {
				end --;
				* (end + 1) = * end;
			}
			* list = newblock;
			return temp;
		}
	}

	return newblock;
}


/*
 * Canonicalize `n' disk addresses.
 */

void
canndaddr(dp, n)
daddr_t *dp;
int n;
{
	while (n --) {
		candaddr (* dp);
		dp ++;
	}
}


/*
 * Free the block `b' on the device `dev'.
 */

#if	__USE_PROTO__
void bfree (o_dev_t dev, daddr_t b)
#else
void
bfree (dev, b)
o_dev_t dev;
daddr_t b;
#endif
{
	struct filsys *	sbp;
	struct fblk   *	fbp;
	buf_t	      *	bp;
	MOUNT *mp;

	if ((mp = getment (dev, 1)) == NULL)
		return;
	sbp = & mp->m_super;
	if (b >= sbp->s_fsize || b < sbp->s_isize) {
		devmsg (dev, "Bad block %u (free)", (unsigned) b);
		return;
	}

	T_FILESYS (dev, 1,
		   cmn_err (CE_NOTE, "bfree (#%x,%ld): nfree = %d",
		   	    dev, b, sbp->s_nfree));

	__LOCK_FREE_BLOCK_LIST (mp, "bfree ()");

	/*
	 * NIGEL : Are we keeping things in order? If so, insert the new block
	 * in position (the smallest block-number is the new 'b'). Note that
	 * the zero-position of the free-block list never changes... this is
	 * an important invariant, because the block in that position is a
	 * link and *must* be preserved in that position so that the links get
	 * properly followed later on.
	 */

	if (t_sortblocks && sbp->s_nfree > 0) {
		daddr_t		newblk;
		newblk = daddr_add (sbp->s_free + 1, sbp->s_nfree - 1, b);
		if (newblk == 0) {
			cmn_err (CE_WARN,
				 "bfree (#%x, %ld) : duplicate free block",
				 dev, b);
			return;
		}
		b = newblk;
	}

	if (sbp->s_nfree == 0 || sbp->s_nfree == NICFREE) {
		T_FILESYS (dev, 1,
			   cmn_err (CE_NOTE, "bfree () write free blocks"));

		bp = bclaim (dev, b, BSIZE, BUF_SYNC);
		fbp = (struct fblk *) bp->b_vaddr;

		memset (bp->b_vaddr, 0, BSIZE);

		fbp->df_nfree = sbp->s_nfree;
		canshort (fbp->df_nfree);
		memcpy (fbp->df_free, sbp->s_free, sizeof (fbp->df_free));
		canndaddr (fbp->df_free, sbp->s_nfree);

		bp->b_flag |= BFMOD;
		brelease (bp);
		sbp->s_nfree = 0;
	}
	sbp->s_free [sbp->s_nfree ++] = b;
	sbp->s_tfree ++;
	sbp->s_fmod = 1;

	__UNLOCK_FREE_BLOCK_LIST (mp);
}


/*
 * Free all blocks in the indirect block `b' on the device `dev'.
 * `l' is the level of indirection.
 */

void
indfree (dev, b, l)
dev_t dev;
daddr_t b;
unsigned l;
{
	int i;
	buf_t	      *	bp;
	daddr_t * dp;
	daddr_t b1;
	char *cur_indirect;
	long buf_size;


	if (b == 0)
		return;
	if (l -- > 0 && (bp = bread (dev, b, BUF_SYNC)) != NULL) {
		if(!(cur_indirect = (char *)kmem_alloc(buf_size = bp->b_bufsiz,
		   KM_SLEEP)))
			cmn_err(CE_PANIC, "indfree: out of kernel heap");

		i = NBN;

		memcpy(cur_indirect, bp->b_vaddr, buf_size);
		brelease(bp);

		while (i -- > 0) {
#if 0
			dp = (daddr_t *) bp->b_vaddr;
#else
			dp = (daddr_t *)cur_indirect;
#endif
			if ((b1 = dp [i]) == 0)
				continue;
			candaddr (b1);
			if (l == 0)
				bfree (dev, b1);
			else
				indfree (dev, b1, l);
		}
#if 0
		brelease (bp);
#else
		kmem_free(cur_indirect, buf_size);
#endif
	}
	bfree (dev, b);
}


/*
 * Experimental routine to read free block lists blocks (ahead of time, but
 * if it works we'll subsume the synchronous read as well).
 */

static buf_t *
read_free_block_list (super, dev, block_no, sync_flag)
struct filsys *	super;
dev_t		dev;
daddr_t		block_no;
int		sync_flag;
{
	return block_no < super->s_fsize && block_no >= super->s_isize ?
			bread (dev, block_no, sync_flag) : NULL;
}


/*
 * Allocate a block from the filesystem mounted of device `dev'.
 */

#if	__USE_PROTO__
daddr_t balloc (o_dev_t dev)
#else
daddr_t
balloc (dev)
o_dev_t dev;
#endif
{
	struct filsys *	sbp;
	struct fblk   *	fbp;
	daddr_t b;
	buf_t	      *	bp;
	MOUNT *mp;

	if ((mp = getment(dev, 1)) == NULL)
		return 0;

	__LOCK_FREE_BLOCK_LIST (mp, "balloc ()");
	sbp = & mp->m_super;

	if (sbp->s_nfree == 0 || sbp->s_nfree > NICFREE) {
enospc:
		sbp->s_nfree = 0;
		devmsg (dev, "Out of space");
		set_user_error (ENOSPC);
		b = 0;
	} else {
		sbp->s_fmod = 1;
		if ((b = sbp->s_free [-- sbp->s_nfree]) == 0)
			goto enospc;

		sbp->s_free [sbp->s_nfree] = 0;

		T_FILESYS (dev, 1,
			   cmn_err (CE_NOTE,
				    "balloc (#%x): nfree = %d blk = %ld",
				    dev, sbp->s_nfree, b));

		if (sbp->s_nfree == 0) {
			if ((bp = read_free_block_list (sbp, dev, b,
							BUF_SYNC)) == NULL) {
ebadflist:
				devmsg (dev, "Bad free list");
				goto enospc;
			}
			fbp = (struct fblk *) bp->b_vaddr;
			sbp->s_nfree = fbp->df_nfree;
			canshort (sbp->s_nfree);
			memcpy (sbp->s_free, fbp->df_free,
				sizeof (sbp->s_free));

			if (sbp->s_nfree > NICFREE)
				goto ebadflist;
			brelease (bp);

			canndaddr (sbp->s_free, sbp->s_nfree);

			/*
			 * NIGEL: As an experiment, try reading ahead on the
			 * free block list.
			 */

			if (sbp->s_nfree > 0)
				read_free_block_list (sbp, dev,
						      sbp->s_free [0],
						      BUF_ASYNC);
		}
		-- sbp->s_tfree;
		if (b >= sbp->s_fsize || b < sbp->s_isize)
			goto ebadflist;
	}
	__UNLOCK_FREE_BLOCK_LIST (mp);
	return b;
}


/*
 * Determine if the given block is bad.
 */

int
bad(dev, b)
dev_t dev;
daddr_t b;
{
	struct inode  *	ip;
	buf_t	      *	bp;
	int i;
	int m;
	int n;
	daddr_t l;

	ip = iattach (dev, 1);

	ASSERT (ip != NULL);

	n = blockn (ip->i_size);
	if ((m = n) > ND)
		m = ND;
	for (i = 0 ; i < m ; i ++) {
		-- n;
		if (b == ip->i_a.i_addr [i]) {
			idetach (ip);
			return 1;
		}
	}
	l = ip->i_a.i_addr [ND];
	idetach (ip);
	if (n == 0)
		return 0;
	if ((bp = bread (dev, l, BUF_SYNC)) == NULL)
		return 0;
	if ((m = n) > NBN)
		m = NBN;
	for (i = 0 ; i < m ; i ++) {
		l = ((daddr_t *) bp) [i];
		candaddr (l);
		if (b == l) {
			brelease (bp);
			return 1;
		}
	}
	brelease (bp);
	return 0;
}


/*
 * Convert long to comp_t style number.
 * A comp_t contains 3 bits of base-8 exponent
 * and a 13-bit mantissa.  Only unsigned
 * numbers can be comp_t numbers.
 */

#define	MAXMANT		017777		/* 2^13-1 = largest mantissa */

static comp_t
ltoc(l)
long l;
{
	register int exp;

	if (l < 0)
		return 0;
	for (exp = 0 ; l > MAXMANT ; exp ++)
		l >>= 3;
	return (exp << 13) | l;
}

/*
 * Write out an accounting record.
 */

void
setacct()
{
	struct acct acct;
	IO acctio;

	if (acctip == NULL)
		return;

	ASSERT (sizeof (acct.ac_comm) == sizeof (SELF->p_comm));
	memcpy (acct.ac_comm, SELF->p_comm, sizeof (acct.ac_comm));

	acct.ac_utime = ltoc (SELF->p_utime);
	acct.ac_stime = ltoc (SELF->p_stime);
	acct.ac_etime = ltoc (timer.t_time - u.u_btime);
	acct.ac_btime = u.u_btime;
	acct.ac_uid = SELF->p_credp->cr_uid;
	acct.ac_gid = SELF->p_credp->cr_gid;
	acct.ac_mem = 0;
	acct.ac_io = 0;		/* ltoc (u.u_block); */
	acct.ac_tty = SELF->p_ttdev;
	acct.ac_flag = u.u_flag;

	ilock (acctip, "setacct ()");

	acctio.io_seek = acctip->i_size;
	acctio.io_ioc  = sizeof (acct);
	acctio.io.vbase = (caddr_t) & acct;
	acctio.io_seg  = IOSYS;
	acctio.io_flag = 0;
	iwrite (acctip, & acctio);

	iunlock (acctip);

	set_user_error (0);
}
