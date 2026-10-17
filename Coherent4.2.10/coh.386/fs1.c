/* $Header: /ker/coh.386/RCS/fs1.c,v 2.7 93/10/29 00:55:09 nigel Exp Locker: nigel $ */
/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or persuant to the license agreement is unlawful.
 *
 *	COHERENT Version 2.3.37
 *	Copyright (c) 1982, 1983, 1984.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
/*
 * Filesystem (mostly handling of in core inodes).
 *
 * $Log:	fs1.c,v $
 * Revision 2.7  93/10/29  00:55:09  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.6  93/09/13  07:56:57  nigel
 * Added some extra checks to ensure that inodes are locked in the right
 * places.
 * 
 * Revision 2.5  93/09/02  18:05:35  nigel
 * Nigel's r85, minor edits only
 * 
 * Revision 2.4  93/08/19  03:26:27  nigel
 * Nigel's r83 (Stylistic cleanup)
 * 
 * Revision 2.2  93/07/26  15:19:00  nigel
 * Nigel's R80
 * 
 * Revision 1.8  93/04/14  10:06:28  root
 * r75
 * 
 * Revision 1.7  93/02/23  15:50:51  root
 * after caddr_t change, before blclear
 * 
 * Revision 1.4  92/07/16  16:33:32  hal
 * Kernel #58
 */

#include <kernel/proc_lib.h>
#include <sys/cmn_err.h>
#include <sys/debug.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/cred.h>
#include <dirent.h>
#include <stddef.h>

#define	_KERNEL		1

#include <kernel/trace.h>
#include <acct.h>		/* for ASU */
#include <sys/uproc.h>
#include <sys/buf.h>
#include <canon.h>
#include <sys/filsys.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/io.h>
#include <sys/mount.h>
#include <sys/types.h>

/*
 * Get character for `ftoi' depending on what space the characters are
 * coming from.
 */

#define ftoic(p,seg)	((seg) == IOSYS ? * (p) : getubd (p))

/*
 * Map the given filename to an inode.  If an error is encountered,
 * `set_user_error ()' is set, and the error is returned.  As this routine
 * needs to set several things, depending on the type of access, `t',
 * there are places in the processes' user area reserved for this routine
 * to set.  These are defined in the user process structure.  The seek
 * position is always set to the position of the directory entry of the
 * child if the child exists or the first free position if it doesn't.
 *  'r' =>  Reference.  A pointer to the child's inode is returned locked.
 *  'c' =>  Create.  If the child exists, a pointer to the inode is returned
 *          locked.  Otherwise if the parent directory exists, a pointer to
 *          the parent directory is returned locked.  Otherwise, an error.
 *  'u' =>  Unlink.  The parent directory is returned unlocked.  The child's
 *          inode number is returned.  The seek position is also set.
 */

#if	__USE_PROTO__
int ftoi (__CONST__ char * np, int t, int space, IO * iop,
	  struct direct * dirent, cred_t * credp)
#else
int
ftoi (np, t, space, iop, dirent, credp)
__CONST__ char * np;
int		t;
int		space;
IO	      *	iop;
struct direct *	dirent;
cred_t	      *	credp;
#endif
{
	return file_to_inode (np, t, 0, space, iop, dirent, credp);
}


/*
 * Compare the strings in the directory entries for equality.
 */

#if	__USE_PROTO__
__LOCAL__ int direq (struct direct * dp1, struct direct * dp2)
#else
__LOCAL__ int
direq (dp1, dp2)
struct direct *	dp1;
struct direct * dp2;
#endif
{
	return dp1->d_ino == 0 ? 0 :
		strncmp (dp1->d_name, dp2->d_name, sizeof (dp1->d_name)) == 0;
}


/*
 * Does main ftoi job. Was created to solve the problem with rmdir.
 * doRmdir is 0 for all cases except when called from rmdir.
 */

#if	__USE_PROTO__
int file_to_inode (__CONST__ char * np, int t, int doRmdir, int space,
		   IO * iop, struct direct * dirent, cred_t * credp)
#else
int
file_to_inode (np, t, doRmdir, space, iop, dirent, credp)
__CONST__ char * np;
int		t;
int		doRmdir;
int		space;
IO	      *	iop;
struct direct *	dirent;
cred_t	      *	credp;
#endif
{
	struct inode *cip;
	char *cp;
	int c;
	struct direct *dp;
	buf_t *bp;
	off_t cseek, fseek, s;
	int fflag, mflag;
	dev_t dev;
	ino_t ino;
	daddr_t b;

	u.u_cdirn = 0;
	u.u_cdiri = NULL;
	u.u_pdiri = NULL;

	switch (t) {

	case 'r':
		ASSERT (iop == NULL);
		break;

	case 'c':
	case 'u':
		ASSERT (iop != NULL);
		iop->io_seg = space;
		break;

	default:
		cmn_err (CE_PANIC, "Bad cookie in file_to_inode ()");
		break;
	}

	if ((c = ftoic (np ++, space)) != '/') {
		if (c == 0) {
			set_user_error (ENOENT);
			return ENOENT;
		}
		cip = u.u_cdir;
	} else {
		c = ftoic (np ++, space);
		cip = u.u_rdir;
	}
	while (c == '/')
		c = ftoic (np ++, space);
	ilock (cip, "file_to_inode ()");
	cip->i_refc ++;

	if (c == 0) {
		if (t == 'r') {
			if (get_user_error () == 0) {
				u.u_cdiri = cip;
				return get_user_error ();
			}
		} else
			set_user_error (ENOENT);

		idetach (cip);
		return get_user_error ();
	}

	for (;;) {
		cp = dirent->d_name;
		while (c != '/' && c != 0) {
			if (cp < & dirent->d_name [sizeof (dirent->d_name)])
				* cp ++ = c;
			c = ftoic (np ++, space);
		}

		while (c == '/')
			c = ftoic (np ++, space);

		while (cp < & dirent->d_name [sizeof (dirent->d_name)])
			* cp ++ = 0;

		if ((cip->i_mode & IFMT) != IFDIR)
			set_user_error (ENOTDIR);
		else {
			/* For rmdir we need only write */
			if (doRmdir)
				iaccess (cip, IPW, credp);
			else
				iaccess (cip, IPE, credp);
		}

		if (get_user_error ()) {
			idetach (cip);
			return get_user_error ();
		}

		cp = dirent->d_name;

		if (cip->i_ino == ROOTIN && cip->i_dev != rootdev &&
		    * cp ++ == '.' && * cp ++ == '.' && * cp ++ == 0)
			cip = ftoim (cip);

		b = 0;
		fflag = 0;
		mflag = 0;
		cseek = 0;
		s = cip->i_size;

		while (s > 0) {
			if ((bp = vread (cip, b ++)) == NULL) {
				idetach (cip);
				return get_user_error ();
			}

			dp = (struct direct *) bp->b_vaddr;
			while ((caddr_t) dp < bp->b_vaddr + BSIZE) {

				if ((s -= sizeof (* dp)) < 0)
					break;

				if ((ino = dp->d_ino) == 0) {
					if (fflag == 0) {
						fflag ++;
						fseek = cseek;
					}
				} else if (direq (dp, dirent)) {
					canino (ino);
					mflag = 1;
					s = 0;
					break;
				}
				cseek += sizeof (* dp);
				dp ++;
			}
			brelease (bp);
		}

		dev = cip->i_dev;
		if (fflag == 0)
			fseek = cseek;

		if (mflag == 0) {
			if (c == 0 && t == 'c') {
				u.u_pdiri = cip;
				iop->io_seek = fseek;
			} else {
				set_user_error (ENOENT);
				idetach (cip);
			}
			return get_user_error ();
		}

		if (c == 0) {
			if (t == 'u') {
				u.u_cdirn = ino;
				u.u_pdiri = cip;
				iop->io_seek = cseek;
				return get_user_error ();
			}

			idetach (cip);
			u.u_cdiri = iattach (dev, ino);
			return get_user_error ();
		}

		idetach (cip);
		if ((cip = iattach (dev, ino)) == NULL)
			return get_user_error ();
	}
}


/*
 * Given an inode which is the root of a file system, return the inode
 * on which the file system was mounted.
 */

#if	__USE_PROTO__
struct inode * ftoim (struct inode * ip)
#else
struct inode *
ftoim(ip)
struct inode *ip;
#endif
{
	MOUNT *mp;

	for (mp = mountp ; mp != NULL ; mp = mp->m_next) {
		if (mp->m_dev == ip->i_dev) {
			idetach (ip);
			ip = mp->m_ip;
			ilock (ip, "ftoim ()");
			ip->i_refc ++;
			break;
		}
	}
	return ip;
}


/*
 * Make an inode of the given mode and device.  The parent directory,
 * name and such stuff is set by ftoi.
 */

#if	__USE_PROTO__
struct inode * imake (unsigned mode, o_dev_t rdev, IO * iop,
		      struct direct * dirent, cred_t * credp)
#else
struct inode *
imake (mode, rdev, iop, dirent, credp)
unsigned	mode;
o_dev_t		rdev;
IO	      *	iop;
struct direct *	dirent;
cred_t	      *	credp;
#endif
{
	struct inode  *	ip = NULL;

	mode &= ~ u.u_umask;
	if ((mode & ISVTXT) != 0 && drv_priv (credp) == 0)
		goto det;

	if (iaccess (u.u_pdiri, IPW, credp) == 0)
		goto det;

	if ((ip = ialloc (u.u_pdiri->i_dev, mode, credp)) == NULL)
		goto det;

	ip->i_nlink = 1;
	ip->i_rdev = rdev;
	idirent (ip->i_ino, iop, dirent);
	iaccmodcreat (ip);	/* creat/mknod - atime/mtime/ctime */
	icopymd (ip);
det:
	idetach (u.u_pdiri);
	return ip;
}


/*
 * Write a directory entry out.  Everything necessary has been conveniently
 * set by `ftoi', except the new inode number of this directory entry.
 */

#if	__USE_PROTO__
void idirent (o_ino_t ino, IO * iop, struct direct * dirent)
#else
void
idirent (ino, iop, dirent)
o_ino_t		ino;
IO	      *	iop;
struct direct *	dirent;
#endif
{
	dirent->d_ino = ino;
	canino (dirent->d_ino);

	iop->io_ioc = sizeof (struct direct);
	iop->io.vbase = (caddr_t) dirent;
	iop->io_seg = IOSYS;
	iop->io_flag = 0;

	iwrite (u.u_pdiri, iop);
}


/*
 * NIGEL: Try and reign in the stupidity; here is a simple inode-table search
 * routine.
 */

#if	__USE_PROTO__
struct inode * inode_find (o_dev_t dev, o_ino_t ino, struct inode ** inodepp)
#else
struct inode *
inode_find (dev, ino, inodepp)
o_dev_t		dev;
o_ino_t		ino;
struct inode **	inodepp;
#endif
{
	struct inode  *	scan;
	struct inode  *	freep;

retry:
	freep = NULL;
	for (scan = inode_table ; scan < inode_table_end ; scan ++) {
		if (scan->i_dev == dev && scan->i_ino == ino)
			return scan;

		if (scan->i_refc == 0 &&
		    (freep == NULL || scan->i_lrt < freep->i_lrt))
			freep = scan;
	}
	ASSERT (scan == inode_table_end);

	if (inodepp != NULL) {
		if ((* inodepp = freep) == NULL) {
			cmn_err (CE_WARN, "Inode table overflow");
			return NULL;
		}

		ilock (freep, "inode_find ()");

		if (freep->i_refc != 0) {
			freep->i_lrt = timer.t_time;
			iunlock (freep);
			goto retry;
		}

		freep->i_dev = dev;
		freep->i_ino = ino;
		freep->i_refc = 1;
		freep->i_lrt = timer.t_time;
		freep->i_lastblock = -1;
	}
	return NULL;
}


/*
 * NIGEL: In order to support device cloning, here we deal with creating a
 * fake inode for the clone.
 *
 * We use the 16-bit old Coherent device number as an inode number on a
 * non-existent filesystem to fake it. Note that the inode we return is
 * unlocked, because the device system (in bio.c and friends) traffics in
 * unlocked inodes. In addition, the special-case code in fs3.c and fd.c that
 * deals with cloning expects to receive an unlocked inode.
 */

#if	__USE_PROTO__
struct inode * inode_clone (struct inode * inodep, o_dev_t dev)
#else
struct inode *
inode_clone (inodep, dev)
struct inode  *	inodep;
o_dev_t		dev;
#endif
{
	struct inode  *	temp;
	struct inode  *	freep;

	while ((temp = inode_find (NODEV, dev, & freep)) != NULL) {

		ilock (temp, "inode_clone");

		if (temp->i_ino != dev || temp->i_dev != NODEV) {
			iunlock (temp);
			continue;
		}
		temp->i_refc ++;
		iunlock (temp);
		return temp;
	}

	if (freep == NULL)
		return NULL;

	/*
	 * So that device dispatching works, we also have to store the clone
	 * device number as the "real" device.
	 */

	freep->i_flag = IFWPROT;
	freep->i_rdev = dev;
	freep->i_size = 0;
	freep->i_mode = inodep->i_mode;
	freep->i_nlink = 0;
	freep->i_uid = inodep->i_uid;
	freep->i_gid = inodep->i_gid;
	freep->i_atime = inodep->i_atime;
	freep->i_mtime = inodep->i_mtime;
	freep->i_ctime = inodep->i_ctime;

	iunlock (freep);
	return freep;
}


/*
 * Return a pointer to a locked inode in core containing the given
 * inode number and device.
 */

#if	__USE_PROTO__
struct inode * iattach (o_dev_t dev, o_ino_t ino)
#else
struct inode *
iattach (dev, ino)
o_dev_t	dev;
o_ino_t	ino;
#endif
{
	struct inode  *	ip;
	struct inode  *	fip;
	MOUNT	      *	mp;

	for (;;) {
		if ((ip = inode_find (dev, ino, & fip)) == NULL) {
			if ((ip = fip) == NULL) {
				set_user_error (ENFILE);
				return NULL;
			}
			if (icopydm (ip) == 0) {
				ip->i_ino = 0;
				ip->i_refc = 0;
				iunlock (ip);
				return NULL;
			}
			return ip;
		}

		if ((ip->i_flag & IFMNT) != 0) {
			for (mp = mountp ; mp != NULL ; mp = mp->m_next) {
				if (mp->m_ip == ip) {
					ino = ROOTIN;
					dev = mp->m_dev;
					break;
				}
			}
			continue;
		}

		ilock (ip, "iattach () #2");

		if (ip->i_ino != ino || ip->i_dev != dev) {
			iunlock (ip);
			continue;
		}

		ASSERT (ip->i_refc >= 0);

		ip->i_refc ++;
		ip->i_lrt = timer.t_time;
		return ip;
	}
}


/*
 * Free all the space associated with the given inode.
 */

#if	__USE_PROTO__
void itruncate (struct inode * ip)
#else
void
itruncate (ip)
struct inode  *	ip;
#endif
{
	int		n;
	daddr_t		b;

	ASSERT (ilocked (ip));

	switch (ip->i_mode & IFMT) {
	case IFPIPE:
		ip->i_pnc = ip->i_prx = ip->i_pwx = 0;
		n = ND;
		while (n > 0) {
			if ((b = ip->i_pipe [-- n]) != 0)
				bfree (ip->i_dev, b);
		}
		memset (ip->i_pipe, 0, sizeof (ip->i_pipe));
		break;

	case IFDIR:
	case IFREG:
		n = NADDR;
		while (n > ND) {
			if ((b = ip->i_a.i_addr [-- n]) != 0)
				indfree (ip->i_dev, b, 1 + n - ND);
		}
		while (n > 0) {
			if ((b = ip->i_a.i_addr [-- n]) != 0)
				bfree (ip->i_dev, b);
		}
		memset (ip->i_a.i_addr, 0, sizeof (ip->i_a.i_addr));
		break;

	default:
		return;
	}

	ip->i_size = 0;
	iaccmodcreat (ip);	/* creat/pipe - atime/mtime/ctime */
}


/*
 * Copy an inode from memory back on to disk performing canonicalization.
 */

#if	__USE_PROTO__
void icopymd (struct inode * ip)
#else
void
icopymd (ip)
struct inode  *	ip;
#endif
{
	struct dinode *	dip;
	struct dinode	dinode;
	buf_t	      *	bp;
	o_ino_t 	ino;
	caddr_t		v;

	ASSERT (ilocked (ip));
#if	TRACER & INODE_TRACE
	if (itraced (ip))
		idump (ip, "icopymd ()");
#endif

	if (ip->i_dev == NODEV) {
		/*
		 * NIGEL: This is one of the ersatz inodes I create to support
		 * DDI/DKI clone device semantics.
		 */

		return;
	}
	if (getment (ip->i_dev, 0) == NULL) {
		cmn_err (CE_WARN, "icopymd () : read-only fs on dev %d,%d",
			 major(ip->i_dev), minor(ip->i_dev));
		return;
	}

	ino = ip->i_ino;
	if (ip->i_refc == 0 && ip->i_nlink == 0 && ino != BADFIN &&
	    ino != ROOTIN) {
	    	/*
	    	 * First, we discard all the space allocated to the node.
	    	 */

		itruncate (ip);

		/*
		 * And now, we clear out whatever other fields remain; this
		 * must include i_mode (since that is what ialloc () looks at
		 * too see whether we are free) and i_rdev, (since for block
		 * and character devices it will be non-zero, but itruncate ()
		 * will not zero it for us). For simplicity, we vaporize the
		 * whole i_a union, since other details like i_iev can be
		 * lurking here too.
		 */

		ip->i_lrt = 0;
		ip->i_mode = 0;

		memset (& ip->i_a, 0, sizeof (ip->i_a));
	}

	dip = & dinode;
	dip->di_mode = ip->i_mode;
	canshort (dip->di_mode);
	dip->di_nlink = ip->i_nlink;
	canshort (dip->di_nlink);
	dip->di_uid = ip->i_uid;
	canshort (dip->di_uid);
	dip->di_gid = ip->i_gid;
	canshort (dip->di_gid);
	dip->di_size = ip->i_size;
	cansize (dip->di_size);

	switch (ip->i_mode & IFMT) {
	case IFBLK:
	case IFCHR:
		dip->di_a.di_rdev = ip->i_rdev;
		candev (dip->di_a.di_rdev);
		break;

	case IFREG:
	case IFDIR:
		ltol3 (dip->di_addr, ip->i_a.i_addr, NADDR);
		break;

	case IFPIPE:
		ltol3 (dip->di_addp, ip->i_pipe, ND);
		dip->di_pnc = ip->i_pnc;
		canshort (dip->di_pnc);
		dip->di_prx = ip->i_prx;
		canshort (dip->di_prx);
		dip->di_pwx = ip->i_pwx;
		canshort (dip->di_pwx);
		break;

	default:
		memset (& dip->di_a, 0, sizeof (dip->di_a));
		break;
	}

	dip->di_atime = ip->i_atime;
	cantime (dip->di_atime);
	dip->di_mtime = ip->i_mtime;
	cantime (dip->di_mtime);
	dip->di_ctime = ip->i_ctime;
	cantime (dip->di_ctime);

	if ((bp = bread (ip->i_dev, (daddr_t) iblockn (ino),
			 BUF_SYNC)) == NULL) {
		cmn_err (CE_WARN, "icopymd () could not read block %d",
			 iblockn (ino));
		return;
	}

	v = (char *) ((struct dinode *) bp->b_vaddr + iblocko (ino));
	memcpy (v, dip, sizeof (dinode));
	bp->b_flag |= BFMOD;
	brelease (bp);
	ip->i_flag &= ~ (IFACC | IFMOD | IFCRT);
	if (ip->i_refc == 0 && ip->i_nlink == 0 && ino != BADFIN &&
	    ino != ROOTIN)
		ifree (ip->i_dev, ino);
}


/*
 * Given a locked inode, deaccess it.
 */

#if	__USE_PROTO__
void idetach (struct inode * ip)
#else
void
idetach (ip)
struct inode  *	ip;
#endif
{
	ASSERT (ilocked (ip));
	ASSERT (ip->i_refc > 0);

	if (-- ip->i_refc == 0) {
		ASSERT (ip->i_rl == NULL);

		if ((ip->i_flag & (IFACC | IFMOD | IFCRT)) != 0 ||
		    ip->i_nlink == 0)
			icopymd (ip);
	}

	iunlock (ip);
}


/*
 * Given a inode which isn't locked, lock it and then deaccess.
 */

#if	__USE_PROTO__
void ldetach (struct inode * ip)
#else
void
ldetach(ip)
struct inode *ip;
#endif
{
	ilock (ip, "ldetach ()");
	idetach (ip);
}


/*
 * A specialized routine for finding whether the given inode may be unlinked.
 * Quite simple you say, but we already have an inode locked and could run
 * into gating problems if we were to lock another.  So we look through the
 * cache to see if the inode is there.  If it is, we can easily tell.  If it
 * isn't, `icopydm' is called with a static.  This routine is only used by
 * `uunlink'.
 */

#if	__USE_PROTO__
int iucheck (o_dev_t dev, o_ino_t ino)
#else
int
iucheck (dev, ino)
o_dev_t dev;
o_ino_t ino;
#endif
{
	struct inode * ip;
	struct inode inode;

	if ((ip = inode_find (dev, ino, NULL)) == NULL) {
		int		read_bad;

		ip = & inode;

		__INIT_INODE_LOCK (ip);
		ilock (ip, "iucheck ()");

		ip->i_dev = dev;
		ip->i_ino = ino;

		read_bad = icopydm (ip) == 0;

		iunlock (ip);
		if (read_bad)
			return 0;
	}

	if ((ip->i_mode & IFMT) == IFDIR && super () == 0)
		return 0;

	return 1;
}


/*
 * Copy an inode from disk to memory performing canonization.
 */

#if	__USE_PROTO__
int icopydm (struct inode * ip)
#else
int
icopydm (ip)
struct inode  *	ip;
#endif
{
	struct dinode *	dip;
	buf_t	      *	bp;
	ino_t ino;
	struct dinode dinode;
	caddr_t v;

	ASSERT (ilocked (ip));

	ip->i_flag = getment (ip->i_dev, 0) == NULL ? IFWPROT : 0;

	ino = ip->i_ino;

#if	TRACER & TRACE_INODE
	if (ino == t_inumber)
		itrace (ip);
#endif

	if ((bp = bread (ip->i_dev, (daddr_t) iblockn (ino),
			 BUF_SYNC)) == NULL)
		return 0;

	dip = & dinode;
	v = (char *) ((struct dinode *) bp->b_vaddr + iblocko (ino));
	memcpy (dip, v, sizeof (dinode));
	brelease (bp);

	ip->i_mode = dip->di_mode;
	canshort (ip->i_mode);
	ip->i_nlink = dip->di_nlink;
	canshort (ip->i_nlink);
	ip->i_uid = dip->di_uid;
	canshort (ip->i_uid);
	ip->i_gid = dip->di_gid;
	canshort (ip->i_gid);
	ip->i_size = dip->di_size;
	cansize (ip->i_size);

	/*
	 * Begin by clearing the target space.
	 */

	memset (& ip->i_a, 0, sizeof (ip->i_a));

	switch (ip->i_mode & IFMT) {
	case IFBLK:
	case IFCHR:
		ip->i_rdev = dip->di_a.di_rdev;
		candev (ip->i_rdev);
		break;

	case IFREG:
	case IFDIR:
		l3tol (ip->i_a.i_addr, dip->di_a.di_addb, NADDR);
		break;

	case IFPIPE:
		l3tol (ip->i_pipe, dip->di_addp, ND);
		ip->i_pnc = dip->di_pnc;
		canshort (ip->i_pnc);
		ip->i_prx = dip->di_prx;
		canshort (ip->i_prx);
		ip->i_pwx = dip->di_pwx;
		canshort (ip->i_pwx);
		break;

	default:
		break;
	}

	ip->i_atime = dip->di_atime;
	cantime (ip->i_atime);
	ip->i_mtime = dip->di_mtime;
	cantime (ip->i_mtime);
	ip->i_ctime = dip->di_ctime;
	cantime (ip->i_ctime);
	ip->i_rl = NULL;

#if	TRACER & INODE_TRACE
	if (itraced (ip))
		idump (ip, "icopydm ()");
#endif
	return 1;
}


/*
 * Copy all relevant inodes out on device `dev'.
 */

#if	__USE_PROTO__
void isync (o_dev_t dev)
#else
void
isync (dev)
o_dev_t		dev;
#endif
{
	struct inode  *	ip;

	for (ip = inode_table ; ip < inode_table_end ; ip ++) {
		if (ip->i_refc != 0 && ip->i_dev == dev &&
		    (ip->i_mode & IFMT) != IFCHR &&
		    (ip->i_mode & IFMT) != IFPIPE &&
		    (ip->i_flag & (IFACC | IFMOD | IFCRT)) != 0) {
		    	int		lock;

			if ((lock = ! ilocked (ip)) != 0)
				ilock (ip, "isync ()");
			icopymd (ip);
			if (lock)
				iunlock (ip);
		}
	}
	ASSERT (ip == inode_table_end);
}


/*
 * blclear(ip, lbn)  --  Clear all blocks in inode ip beginning with
 * logical blocks number lbn.  Called from uchsize() in sys5.c
 */

void
blclear(ip, lbn)
INODE *ip;
fsize_t lbn;
{
}


/*
 * Copy the appropriate information from the inode to the stat buffer.
 */

#if	__USE_PROTO__
void istat (struct inode * ip, struct stat * sbp)
#else
void
istat(ip, sbp)
struct inode  *	ip;
struct stat   *	sbp;
#endif
{
	sbp->st_dev = ip->i_dev;
	sbp->st_ino = ip->i_ino;
	sbp->st_mode = ip->i_mode;
	sbp->st_nlink = ip->i_nlink;
	sbp->st_uid = ip->i_uid;
	sbp->st_gid = ip->i_gid;
	sbp->st_rdev = NODEV;
	sbp->st_size = ip->i_size;
	sbp->st_atime = ip->i_atime;
	sbp->st_mtime = ip->i_mtime;
	sbp->st_ctime = ip->i_ctime;

	switch (ip->i_mode & IFMT) {
	case IFBLK:
	case IFCHR:
		sbp->st_rdev = ip->i_rdev;
		sbp->st_size = 0;
		break;

	case IFPIPE:
		sbp->st_size = ip->i_pnc;
		break;
	}
}


/*
 * See if it is possible to access the given inode with the bits in
 * the given mode.
 * If the mode includes writing, and i_refc is > 1, then check for
 * shared text problems.
 */

#if	__USE_PROTO__
int iaccess (struct inode * ip, int request, cred_t * credp)
#else
int
iaccess (ip, request, credp)
struct inode  *	ip;
int		request;
cred_t	      *	credp;
#endif
{
	/*
	 * Requests to write to regular files and pipes that have protected
	 * inodes must fail. However, write requests to device inodes must not
	 * be affected by protection on the inode.
	 */

	if ((request & IPW) != 0 && iprotected (ip)) {
		switch (ip->i_mode & __IFMT) {

		case __IFPIP:
		case __IFREG:
			set_user_error (EROFS);
			return 0;

		default:
			break;
		}
	}

	if (drv_priv (credp)) {
		/*
		 * For now, the super-user can do anything, even with
		 * directories; for the super-user, execute permission is true
		 * if any execute bit is set in the inode perms.
		 */

		mode_t		mode;

		if (((mode = ip->i_mode | __IREAD | __IWRITE) & __IXUGO) != 0)
			mode |= __IXUGO;

		if ((__PERM_EXTRACT (mode, _CRED_OWNER) & request) != request) {
			set_user_error (EACCES);
			return 0;
		}

		u.u_flag |= ASU;
	} else if ((__PERM_EXTRACT (ip->i_mode,
				    cred_match (credp, ip->i_uid,
						ip->i_gid)) & request) !=
		    request) {
		set_user_error (EACCES);
		return 0;
	}

	if ((request & IPW) != 0 && ip->i_refc > 1 && sbusy (ip)) {
		set_user_error (ETXTBSY);
		return 0;
	}
	return 1;
}


/*
 * NIGEL: Originally the following were very unhygenic macros; now they are
 * functions, and we try and deal with inodes on read-only filesystems.
 */

#if	__USE_PROTO__
void iaccessed (struct inode * ip)
#else
void
iaccessed (ip)
struct inode  *	ip;
#endif
{
	if ((ip->i_flag & IFWPROT) != 0)
		return;

	ip->i_flag |= IFACC;
	ip->i_atime = posix_current_time ();
}


#if	__USE_PROTO__
void imodcreat (struct inode * ip)
#else
void
imodcreat (ip)
struct inode  *	ip;
#endif
{
	if ((ip->i_flag & IFWPROT) != 0)
		return;

	ip->i_flag |= IFMOD | IFCRT;
	ip->i_ctime = ip->i_mtime = posix_current_time ();
}


#if	__USE_PROTO__
void icreated (struct inode * ip)
#else
void
icreated (ip)
struct inode  *	ip;
#endif
{
	if ((ip->i_flag & IFWPROT) != 0)
		return;

	ip->i_flag |= IFCRT;
	ip->i_ctime = posix_current_time ();
}


#if	__USE_PROTO__
void iaccmodcreat (struct inode * ip)
#else
void
iaccmodcreat (ip)
struct inode  *	ip;
#endif
{
	if ((ip->i_flag & IFWPROT) != 0)
		return;

	ip->i_flag |= IFACC | IFMOD | IFCRT;
	ip->i_atime = ip->i_mtime = ip->i_ctime = posix_current_time ();
}


#if	TRACER & INODE_TRACE
/*
 * For tracing, dump an inode.
 */

#if	__USE_PROTO__
void idump (struct inode * ip, __CONST__ char * where)
#else
void
idump (ip, where)
struct inode  *	ip;
__CONST__ char * where;
#endif
{
	char	      *	tmp;
	int		i;

	switch (ip->i_mode & __IFMT) {
	case __IFREG:
		tmp = "file";
		break;

	case __IFBLK:
		tmp = "block dev";
		break;

	case __IFCHR:
		tmp = "char dev";
		break;

	case __IFPIP:
		tmp = "pipe";
		break;

	default:
		tmp = "BAD MODE";
		break;
	}

	cmn_err (CE_NOTE,
		 "%s inode %d  size = %ld  mode = %s  refc = %d  links = %d",
		 where, ip->i_ino, ip->i_size, tmp, ip->i_refc, ip->i_nlink);

	switch (ip->i_mode & __IFMT) {
	case __IFREG:
		cmn_err (CE_CONT, "block numbers: ");
		for (i = 0 ; i < NADDR ; i ++)
			cmn_err (CE_CONT, "%d ", ip->i_a.i_addr [i]);
		cmn_err (CE_CONT, "\n");
		break;

	case __IFBLK:
	case __IFCHR:
	case __IFPIP:
		break;
	}
}
#endif	/* TRACER & INODE_TRACE */
