/* $Header: /ker/coh.386/RCS/fs3.c,v 2.8 93/10/29 00:55:16 nigel Exp Locker: nigel $ */
/*
 * Filesystem (I/O).
 *
 * $Log:	fs3.c,v $
 * Revision 2.8  93/10/29  00:55:16  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.7  93/09/13  07:58:07  nigel
 * Added some extra checks to see that inodes are locked, changed the tests
 * in fread () to reduce the possibility of trying to read past EOF.
 * 
 * Revision 2.6  93/09/02  18:07:11  nigel
 * Nigel's r85, minor edits only
 * 
 * Revision 2.5  93/08/19  10:37:18  nigel
 * r83 ioctl (), corefile, new headers
 * 
 * Revision 2.4  93/08/19  03:26:30  nigel
 * Nigel's r83 (Stylistic cleanup)
 * 
 * Revision 2.2  93/07/26  14:28:33  nigel
 * Nigel's R80
 */

#include <common/_tricks.h>
#include <kernel/proc_lib.h>
#include <sys/cmn_err.h>
#include <sys/debug.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <stddef.h>
#include <limits.h>

#define	_KERNEL		1

#include <kernel/trace.h>
#include <kernel/reg.h>
#include <sys/buf.h>
#include <canon.h>
#include <sys/con.h>
#include <sys/filsys.h>
#include <sys/mount.h>
#include <sys/io.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/types.h>


/*
 * Given an inode, open it.
 */

#if	__USE_PROTO__
struct inode * iopen (struct inode * ip, unsigned mode)
#else
struct inode *
iopen (ip, mode)
struct inode  *	ip;
unsigned	mode;
#endif
{
	int		type;
	struct inode  *	newip;

	ASSERT (ilocked (ip));

	type = ip->i_mode & IFMT;

	switch (type) {
	case IFCHR:
	case IFBLK:
		iunlock (ip);
		newip = dopen (ip->i_rdev, mode,
			       type == IFCHR ? DFCHR : DFBLK, ip);
		ilock (ip, "iopen ()");
		ip = newip;

		/*
		 * We assume that if we are returned a different inode from
		 * the one we passed in, that dopen () returned it unlocked.
		 * Our caller should deal with that.
		 */
		break;

	case IFDIR:
		if (mode & IPW) {

			/* Return (EISDIR) if not superuser. */
			if (super () == 0) {
				/* Override EPERM set when super () failed. */
				set_user_error (EISDIR);
				break;
			}

			/*
			 * Opening a directory O_WRONLY is insane, even
			 * if you are superuser!
			 */
			if (mode == IPW)
				set_user_error (EISDIR);
		}
		break;

	case IFPIPE:
		popen (ip, mode);
		break;
	}

	return ip;
}


/*
 * Given an inode, close it.
 *
 * NIGEL: Modified for new dclose ().
 */

#if	__USE_PROTO__
void iclose (struct inode * ip, unsigned mode)
#else
void
iclose (ip, mode)
INODE	      *	ip;
unsigned	mode;
#endif
{
	int		type;

	ilock (ip, "iclose () #1");

	switch (type = ip->i_mode & IFMT) {
	case IFBLK:
		if (getment (ip->i_rdev, 0) == NULL)
			bflush (ip->i_rdev);
		/* FALL THROUGH */

	case IFCHR:
		iunlock (ip);
		dclose (ip->i_rdev, mode,  type == IFCHR ? DFCHR : DFBLK,
			ip->i_private);
		ilock (ip, "iclose () #2");
		break;

	case IFPIPE:
		pclose (ip, mode);
		break;
	}
	idetach (ip);
}


/*
 * Read from a file described by an inode and an io strucuture.
 */

#if	__USE_PROTO__
void iread (struct inode * ip, IO * iop)
#else
void
iread (ip, iop)
struct inode  *	ip;
IO	      *	iop;
#endif
{
	ASSERT ((ip->i_mode & IFMT) == IFCHR || ilocked (ip));

	if (iop->io_ioc == 0)
		return;

	switch (ip->i_mode & IFMT) {
	case IFCHR:
		dread (ip->i_rdev, iop, ip->i_private);
		break;

	case IFBLK:
	case IFREG:
	case IFDIR:
		fread (ip, iop);
		break;

	case IFPIPE:
		pread (ip, iop);
		break;

	default:
		set_user_error (ENXIO);
		break;
	}
}


/*
 * Write to a file described by an inode and io structure.
 */

#if	__USE_PROTO__
void iwrite (struct inode * ip, IO * iop)
#else
void
iwrite (ip, iop)
struct inode  *	ip;
IO	      *	iop;
#endif
{
	ASSERT ((ip->i_mode & IFMT) == IFCHR || ilocked (ip));

	imodcreat (ip);	/* write - mtime, ctime */

	if (iop->io_ioc == 0)
		return;

	switch (ip->i_mode & IFMT) {
	case IFCHR:
		dwrite (ip->i_rdev, iop, ip->i_private);
		break;

	case IFBLK:
		fwrite (ip, iop);
		break;

	case IFREG:
	case IFDIR:
		if (getment (ip->i_dev, 1) == NULL)
			return;
		fwrite (ip, iop);
		break;

	case IFPIPE:
		pwrite (ip, iop);
		break;

	default:
		set_user_error (ENXIO);
		break;
	}
}


/*
 * Given a block offset within an inode, store the offsets for the indirect
 * blocks backwards in the array, `listp', and return a pointer just after the
 * position where the first offset is stored.
 */

#if	__USE_PROTO__
__LOCAL__ int * lmap (daddr_t blockofs, int * listp, int * numblocks)
#else
__LOCAL__ int *
lmap (blockofs, listp, numblocks)
daddr_t		blockofs;
int	      *	listp;
int	      *	numblocks;
#endif
{
	int n;

	if ((n = ND - blockofs) > 0) {
		/*
		 * Just the one direct block, and further blocks up to the end
		 * of the block list in the inode.
		 */
		* listp ++ = blockofs;
		* numblocks = n;
		return listp;
	}
	blockofs -= ND;

	/*
	 * First, the initial indirect block, followed by as many further
	 * layers of indirection as we need.
	 */

	n = nbnrem (blockofs);
	* numblocks = NBN - n;
	* listp ++ = n;

	if ((blockofs = nbndiv (blockofs)) == 0) {
		* listp ++ = ND;
		return listp;
	}


#if	NI > 1
	blockofs --;	/* Make offset in next indirect block zero-based */

	* listp ++ = nbnrem (blockofs);
	if ((blockofs = nbndiv (blockofs)) == 0) {
		* listp ++ = ND + 1;
		return listp;
	}

#if	NI > 2
	blockofs --;	/* Make offset in next indirect block zero-based */

	* listp ++ = nbnrem (blockofs);
	if ((blockofs = nbndiv (blockofs)) == 0) {
		* listp ++ = ND + 2;
		return listp;
	}
#endif
#endif
	SET_U_ERROR (EFBIG, "lmap");
	return NULL;
}


/*
 * Convert the given virtual block to a physical block for the given inode.
 * If the block does not map onto a physical block because the file is sparse
 * but it does exist, 0 is returned.  If an error is encountered, -1 is
 * returned.
 *
 * The parameter below is experimental.
 */

#define	EMPTY_BLOCK	((daddr_t) -1)

int	t_groupmode = 0;

#if	__USE_PROTO__
__LOCAL__ int vmap (struct inode * ip, daddr_t blockofs, int count,
		    daddr_t * blocklist, int allocflag)
#else
__LOCAL__ int
vmap (ip, blockofs, count, blocklist, allocflag)
INODE	      *	ip;
daddr_t		blockofs;
int		count;
daddr_t	      *	blocklist;
int		allocflag;
#endif
{
	daddr_t		block;
	int		list [1 + NI];
	int		nblocks;
	daddr_t	      *	outlist;
	buf_t	      *	buf;
	int	      *	listp;
	int		resid = count;

more:
	if ((listp = lmap (blockofs, list, & nblocks)) == NULL)
		return -1;

	if (nblocks > resid)
		nblocks = resid;
	resid -= nblocks;
	blockofs += nblocks;

	outlist = ip->i_a.i_addr;
	buf = NULL;

	while (-- listp != list) {
		if ((block = outlist [* listp]) == 0) {
			/*
			 * If an indirect block is not present, then this
			 * implies that at least the next "nblocks" leaf
			 * blocks are also not present.
			 */

			do
				* blocklist ++ = EMPTY_BLOCK;
			while (-- nblocks > 0);
			goto done;
		}

		if (buf != NULL) {
			brelease (buf);
			candaddr (block);
		}

		if ((buf = bread (ip->i_dev, block, BUF_SYNC)) == NULL)
			return -1;

		outlist = (daddr_t *) buf->b_vaddr;
	}

	do {
		if ((block = outlist [list [0] ++]) == 0)
			block = EMPTY_BLOCK;
		else if (buf != NULL)
			candaddr (block);
		* blocklist ++ = block;
	} while (-- nblocks > 0);

done:
	if (buf != NULL)
		brelease (buf);

	if (t_groupmode && resid > 0)
		goto more;

	return count - resid;
}


/*
 * The parameter below controls the amount of readahead that happens.
 */

extern int	t_readahead;

#define	READGROUP	16		/*
					 * Maximum # of blocks to read as a
					 * single normal group.
					 */
#define	READAHEAD	8		/*
					 * Maximum # of blocks to read ahead.
					 */

/*
 * Read from a regular or block special file.
 */

#if	__USE_PROTO__
void fread (struct inode * ip, IO * iop)
#else
void
fread (ip, iop)
struct inode  *	ip;
IO	      *	iop;
#endif
{
	off_t		res;
	unsigned	off;
	dev_t		dev;
	daddr_t		lbn;
	daddr_t		abn;
	daddr_t		zbn;
	buf_t	      *	bp;
	int		blk;
	daddr_t		maxblk;
	daddr_t		list [READGROUP + READAHEAD];
	int		do_readahead;

	if ((ip->i_mode & IFMT) == IFBLK) {
		blk = 1;
		dev = ip->i_rdev;
	} else {
		blk = 0;
		dev = ip->i_dev;
	}
	abn = 0;
	zbn = 0;
	lbn = blockn (iop->io_seek);
	off = blocko (iop->io_seek);

	/*
	 * NIGEL: The commented-out code talks about a mysterious "unsigned
	 * prob" which does not in reality exist. All this really wants to
	 * do is pick the minimum of the remaining size and the requested
	 * size.
	 */

#if	0
	res = ip->i_size - iop->io_seek;

	if (blk != 0 || (res > 0 && res > iop->io_ioc)) 
		res = iop->io_ioc;	/* unsigned prob with io_ioc */
	if (res <= 0)
		return;
#endif

	if (blk)
		res = iop->io_ioc;
	else {
		if (iop->io_seek > ip->i_size)
			return;
		if ((res = ip->i_size - iop->io_seek) > iop->io_ioc)
			res = iop->io_ioc;
	}
	if (res == 0)
		return;

	/*
	 * Check for sequential access to see whether we should enable read-
	 * ahead.
	 */

	if (lbn == ip->i_lastblock + 1) {
		if ((do_readahead = t_readahead) < 0)
			do_readahead = 0;
	} else
		do_readahead = 0;

	/*
	 * We record the larget block-offset within the file to avoid trying
	 * to read past the end of file with readahead. This causes Bad Things
	 * to happen with pipes, where funky data is stored in the indirect-
	 * block slots. For block devices, there is sadly no way to get this
	 * information under the Coherent device-driver system.
	 */

	maxblk = blk ? INT_MAX : blockn (ip->i_size + BSIZE - 1);

	do {
		int		count;

		if (lbn >= zbn) {
			unsigned	i;

			if ((count = blockn (res + BSIZE - 1) +
				     do_readahead) > __ARRAY_LENGTH (list))
				count = __ARRAY_LENGTH (list);

			ASSERT (count > do_readahead);

			if (lbn + count >= maxblk)
				count = maxblk - lbn;

			if (blk == 0 &&
			    (count = vmap (ip, lbn, count, list, 0)) < 0)
				return;

			abn = lbn;
			for (i = 0, zbn = lbn ; i < count ; i ++, zbn ++) {
				if (blk != 0)
					list [i] = zbn;
				else if (list [i] == EMPTY_BLOCK)
					continue;

				if (t_readahead == -1)
					continue;

				(void) bread (dev, list [i], BUF_ASYNC);
			}
		}

		if (res < (count = BSIZE - off))
			count = res;

		if (list [lbn - abn] == EMPTY_BLOCK)
			ioclear (iop, count);
		else {
			if ((bp = bread (dev, list [lbn - abn],
					 BUF_SYNC)) == NULL)
				return;
			iowrite (iop, bp->b_vaddr + off, count);
			brelease (bp);
		}

		if (get_user_error ())
			return;
		lbn ++;
		off = 0;
		res -= count;
	} while (res > 0);

	ip->i_lastblock = lbn - 1;
}


/*
 * Given an inode pointer, read the requested virtual block and return a
 * buffer with the data.  In sparse files, the necessary blocks are allocated.
 * If the flag, `fflag' is set, the final buffer is just claimed rather than
 * read as we are going to change it's contents completely.
 */

#if	__USE_PROTO__
__LOCAL__ buf_t * aread (struct inode * ip, daddr_t blkofs, int claim)
#else
__LOCAL__ buf_t *
aread (ip, blkofs, claim)
struct inode  *	ip;
daddr_t		blkofs;
int		claim;
#endif
{
	buf_t	      *	bp;
	int	      *	listp;
	dev_t		dev;
	int		l;
	int		aflag;
	int		lflag;
	daddr_t	      *	dp;
	daddr_t		block;
	daddr_t		blocksave;
	int		list [1 + NI];
	int		nblocks;

	if ((listp = lmap (blkofs, list, & nblocks)) == NULL)
		return NULL;
	aflag = 0;
	dev = ip->i_dev;
	block = ip->i_a.i_addr [l = * -- listp];
	if (block == 0) {
		aflag = 1;
		if ((block = balloc (dev)) == 0)
			return NULL;
		T_INODE (ip, cmn_err (CE_NOTE,
				      "inode %d allocated block %d",
				      ip->i_ino, block));
		ip->i_a.i_addr [l] = block;
	}
	for (;;) {
		lflag = listp == list;

		/*
		 * If we are not allocating a new block and the caller is
		 * going to preserve any of the data that we are going to
		 * return, then read in the previous block contents.
		 */

		if (! (aflag || (claim && lflag))) {
			if ((bp = bread (dev, block, BUF_SYNC)) == NULL)
				return NULL;
		} else {
			bp = bclaim (dev, block, BSIZE, BUF_SYNC);

			/*
			 * If this is the last block and the caller is just
			 * going to overwrite it, don't zero-fill.
			 */

			if (! (claim && lflag))
				clrbuf (bp);
			bp->b_flag |= BFMOD;
		}

		blocksave = block;

		if (lflag)
			return bp;

		aflag = 0;
		dp = (daddr_t *) bp->b_vaddr;
		block = dp [l = * -- listp];
		candaddr (block);

		/* 
		 * WARNING!  This is only legal if the inode is locked!
		 * Sleazier than anything you've seen before, eh?
		 * Love, your pal, Louis.
		 */
		ASSERT(ilocked(ip));
		brelease(bp);

		if (block == 0) {
			aflag = 1;
			if ((block = balloc (dev)) == 0) {
				return NULL;
			}
			T_INODE (ip, cmn_err (CE_NOTE,
					      "inode %d allocated block %d",
					      ip->i_ino, block));
			if ((bp = bread(dev, blocksave, BUF_SYNC)) == NULL)
				cmn_err(CE_PANIC,
				         "Fatal error updating free list");
			dp = (daddr_t *)bp->b_vaddr;
			dp [l] = block;
			candaddr (dp [l]);
			bp->b_flag |= BFMOD;
			brelease(bp);
		}
	}
}


/*
 * The parameter below controls the way in which blocks are written. It is
 * currently experimental.
 *
 * (2 is best for avoiding disk thrashing iff we can do something clever like
 * block-sorting the I/O, especially on syncs)
 */

int	t_writemode = BUF_ASYNC;


/*
 * Write to a regular or block special file.
 */

#if	__USE_PROTO__
void fwrite (struct inode * ip, IO * iop)
#else
void
fwrite (ip, iop)
struct inode  *	ip;
IO	      *	iop;
#endif
{
	unsigned	n;
	unsigned	off;
	daddr_t		lbn;
	buf_t	      *	bp;
	int		blk;
	int		com;

	lbn = blockn (iop->io_seek);
	off = blocko (iop->io_seek);
	blk = (ip->i_mode & IFMT) == IFBLK;
	while (iop->io_ioc > 0) {
		if (iop->io_ioc < (n = BSIZE - off))
			n = iop->io_ioc;
		com = off == 0 && n == BSIZE;

		if (blk == 0)
			bp = aread (ip, lbn, com);
		else {
			if (com)
				bp = bclaim (ip->i_rdev, lbn, BSIZE, BUF_SYNC);
			else
				bp = bread (ip->i_rdev, lbn, BUF_SYNC);
		}
		if (bp == NULL)
			return;
		ioread (iop, bp->b_vaddr + off, n);
		bp->b_flag |= BFMOD;
		if (com && t_writemode != 2 && (ip->i_mode & IFMT) != IFPIPE) {
			bwrite (bp, t_writemode);
			if (t_writemode == BUF_SYNC)
				brelease (bp);
		} else
			brelease (bp);
		if (get_user_error ())
			return;
		lbn ++;
		off = 0;
		if ((iop->io_seek += n) > ip->i_size)
			if (blk == 0)
				ip->i_size = iop->io_seek;
	}
}


/*
 * Given an inode pointer, read the requested virtual block and return
 * a buffer with the data.
 */

#if	__USE_PROTO__
buf_t * vread (struct inode * ip, daddr_t blockofs)
#else
buf_t *
vread (ip, blockofs)
struct inode  *	ip;
daddr_t		blockofs;
#endif
{
	daddr_t		block;
	buf_t	      *	bp;

	if (vmap (ip, blockofs, 1, & block, 0) < 0)
		return NULL;

	if (block != EMPTY_BLOCK)
		return bread (ip->i_dev, block, BUF_SYNC);

	bp = geteblk ();
	bp->b_dev = ip->i_dev;
	clrbuf (bp);
	return bp;
}
