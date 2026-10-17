/* $Header: /ker/coh.386/RCS/sys3.c,v 2.9 93/10/29 00:55:45 nigel Exp Locker: nigel $ */
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
 * System calls (more filesystem related calls).
 *
 * $Log:	sys3.c,v $
 * Revision 2.9  93/10/29  00:55:45  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.8  93/09/13  08:03:55  nigel
 * Improved the synchronous write support to not flush the *entire* inode
 * cache, just write the relevant i-node.
 * 
 * Revision 2.7  93/09/02  18:08:35  nigel
 * Nigel's r85, minor edits only
 * 
 * Revision 2.6  93/08/19  10:37:38  nigel
 * r83 ioctl (), corefile, new headers
 * 
 * Revision 2.5  93/08/19  03:26:53  nigel
 * Nigel's r83 (Stylistic cleanup)
 */

#include <kernel/proc_lib.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <stddef.h>

#define	_KERNEL		1

#include <kernel/trace.h>
#include <sys/uproc.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/con.h>
#include <sys/fd.h>
#include <sys/filsys.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/io.h>
#include <sys/mount.h>
#include <sys/file.h>

/*
 * Open the file `np' with the mode `mode'.
 */

int
uopen (np, oflag, mode)
char *np;
int		oflag;
mode_t		mode;
{
	int f;
	struct inode * ip;
	fd_t fd;
	int cflag;	/* Flag is set if we create a file.  */
	IO		io;
	struct direct	dir;

	cflag = 0;	/* Nothing created so far.  */

	/*
	 * NIGEL: As reported numerous times by customers, this stupid code
	 * will create a file before looking to see whether it can open a file
	 * descriptor. In fact, any error in here will leave a new file around
	 * despite the error return.
	 *
	 * Do it right; allocate the resources first!
	 */

	if ((fd = fd_alloc ()) == ERROR_FD)
		return -1;


	/* Determine read or write status for fdopen.  */

	switch (oflag & 3) {
	case O_RDONLY:
		f = IPR;
		break;

	case O_WRONLY:
		f = IPW;
		break;

	case O_RDWR:
		f = IPR | IPW;
		break;

	default:
		SET_U_ERROR (EINVAL, "bad oflag");
		goto done;
	}

	/* Process the O_CREAT flag.  */
	if ((oflag & O_CREAT) != 0) {

		if (ftoi (np, 'c', IOUSR, & io, & dir, SELF->p_credp) != 0)
			goto done;

		/* If it didn't exist, but its parent did, then make it.  */
		if ((ip = u.u_cdiri) == NULL) {
			if ((ip = imake ((mode & ~ IFMT) | IFREG,
					 (o_dev_t) 0, & io, & dir,
					 SELF->p_credp)) == NULL)
				goto done;

			cflag = 1;	/* Note that we just created a file.  */
		} else {	/* The file already exists.  */
			/*
			 * Exclusive O_CREAT on existing file should fail.
			 */

			if ((oflag & O_EXCL) != 0) {
				idetach (ip);
				SET_U_ERROR (EEXIST,
					 "exclusive creat on existing file");
				goto done;
			}

			/*
			 * Do not write to a read only file system;
			 * never write to a directory;
			 * always write to block and character special devices.
			 */

			switch (ip->i_mode & IFMT) {
			case IFBLK:
			case IFCHR:
				break;

			case IFDIR:
				idetach (ip);
				SET_U_ERROR (EISDIR, "<open: EISDIR>");
				goto done;

			default:
				if (getment (ip->i_dev, 1) == NULL) {
					idetach (ip);
					SET_U_ERROR (EROFS,
						"Could not fetch mount entry");
					goto done;
				}
			}
		} /* Did the file exist?  */

	} else { /* O_CREAT was not set--just reference the file.  */

		if (ftoi (np, 'r', IOUSR, NULL, & dir, SELF->p_credp) != 0)
			goto done;

		ip = u.u_cdiri;	/* This must be the inode we wanted.  */
	}

	/*
	 * ASSERTION: We probably have an inode for an existing file.
	 * If we don't, the ip will be NULL and iaccess() will fail (as
	 * desired.)
	 */

	/*
	 * Only check permissions on a pre-existing file.
	 */

	if (0 == cflag && iaccess (ip, f, SELF->p_credp) == 0) {
		idetach (ip);
		goto done;
	}

	/*
	 * ASSERTION: We have an inode for a file we
	 * have valid permissions on.
	 */


	/*
	 * LOUIS:
	 * According to the POSIX and SVID specs, this alternate handling
	 * of O_EXCL is bogus.  O_EXCL only has meaning if used with
	 * O_CREAT  It is now being removed for the first time in
	 * Coherent history (even though it was broken long before!)
	 * and is #ifdef'ed out in case it breaks just a few too many
	 * utilities.
	 */

#if 0
	if ((ip->i_flag & IFEXCL) != 0) {
		idetach (ip);
		SET_U_ERROR (EEXIST, "open: file already open O_EXCL");
		goto done;	/* Somebody else has an exclusive open.  */
	}

	/*
	 * If requesting exclusive open, fail if someone else has it open.
	 */

	if ((oflag & O_EXCL) != 0) {
		if (ip->i_refc != 1) {
			idetach (ip);
			SET_U_ERROR (EEXIST, "<open: O_EXCL but already open>");
			goto done;
		}

		/* Mark this open inode as exclusive.  */

		/* This had read &= IFEXCL -- I think
		 * that was a mistake -- LOUIS
		 */
		ip->i_flag &= IFEXCL;
	}
#endif

	if ((oflag & O_NDELAY) != 0)
		f |= IPNDLY;
	if ((oflag & O_NONBLOCK) != 0)
		f |= IPNONBLOCK;
	if ((oflag & O_APPEND) != 0)
		f |= IPAPPEND;
	if ((oflag & O_SYNC) != 0)
		f |= IPSYNC;
#if 0
	if ((oflag & O_EXCL) != 0)
		f |= IPEXCL;
#endif

	if ((oflag & O_NOCTTY) != 0)
		f |= IPNOCTTY;

	/*
	 * NIGEL: special hack for FS debugging.
	 */

	if ((oflag & O_TRACE) != 0)
		itrace (ip);

	if (fd_init (fd, ip, f) < 0) {
		idetach (ip);
		goto done;
	}

	/*
	 * Change our notion of what inode we are working with in case the
	 * open created a new one.
	 */

	ip = fd_get (fd)->f_ip;


	/*
	 * If requested, truncate the file.
	 */

	if ((oflag & O_TRUNC) != 0 && (ip->i_mode & IFPIPE) != IFPIPE &&
	    cflag == 0) {
		if (iaccess (ip, IPW, SELF->p_credp) != 0) {
			itruncate (ip);
		} else {
			idetach (ip);
			goto done;
		}
	}

#if	0
	/*
	 * This is code to test where a bug in BBx/4 is showing up; their
	 * self-tests fail on O_APPEND for some unknown reason; this didn't
	 * help.
	 */

	if ((oflag & O_APPEND) != 0)
		fd_get (fd)->f_seek = ip->i_size;
#endif

	iunlock (ip);

done:
	return (fd = fd_finish (fd)) == ERROR_FD ? -1 : fd;
}


/*
 * Create a pipe.  Notice, we must do the IPR fdopen with IPNDLY so that
 * we don't block waiting for the writer we are about to create.  Then
 * after we are done, we ufcntl() to turn off the IPNDLY on fd1.
 */

int
upipe (fdp)
short fdp[2];
{
	INODE *ip;
	fd_t fd1;
	fd_t fd2;

	if ((ip = pmake (0)) == NULL)
		return -1;

	if ((fd1 = fd_open (ip, IPR | IPNDLY)) != ERROR_FD) {
		/*
		 * We assume that fd_open () will not switch inodes on us.
		 */

		ASSERT (fd_get (fd1)->f_ip == ip);

		ip->i_refc ++;
		if ((fd2 = fd_open (ip, IPW)) != ERROR_FD) {

			ASSERT (fd_get (fd2)->f_ip == ip);

			iunlock (ip);
			u.u_rval2 = fd2;
			ufcntl (fd1, F_SETFL, 0);
			return fd1;
		}

		-- ip->i_refc;
		iunlock (ip);
		fd_close (fd1);
		return -1;
	}
	idetach (ip);
	return -1;
}


/*
 * Read or write `n' bytes from the file number `fd' using the buffer
 * `bp'.  If `do_write' is nonzero, write, else read.
 */

#if	__USE_PROTO__
__LOCAL__ int sysio (int fd, caddr_t bp, size_t n, int do_write)
#else
__LOCAL__ int
sysio (fd, bp, n, do_write)
int fd;
caddr_t bp;
size_t n;
int do_write;
#endif
{
	__fd_t	      *	fdp;
	int type;
	IO		io;

	if ((fdp = fd_get(fd)) == NULL ||
	    (fdp->f_flag & (do_write ? IPW : IPR)) == 0) {
		set_user_error (EBADF);
		return -1;
	}


	/*
	 * When reading (writing into user memory), buffer may NOT be in text
	 * segment.  When writing (reading from user memory), buffer may
	 * be in text segment.
	 */

	if (! useracc (bp, n, ! do_write)) {
		set_user_error (EFAULT);
		return -1;
	}

	type = fdp->f_ip->i_mode & IFMT;
	if (type != IFCHR)
		ilock (fdp->f_ip, "sysio ()");

	/* Writes in append mode are forced to end of file. */
	if ((fdp->f_flag & IPAPPEND) != 0 && do_write)
		fdp->f_seek = fdp->f_ip->i_size;

	if (do_write && (fdp->f_ip->i_mode & IFMT) == IFREG) {
		long maxbyte = (long) u.u_bpfmax * BSIZE;
		if (maxbyte <= fdp->f_seek)
			n = 0;
		else if ((long) n > maxbyte - fdp->f_seek)
			n = (unsigned) (maxbyte - fdp->f_seek);
	}

	io.io_seg = IOUSR;
	io.io_seek = fdp->f_seek;
	io.io.vbase = bp;
	io.io_ioc  = n;
	io.io_flag = 0;

	if ((fdp->f_flag & IPNDLY) != 0)
		io.io_flag |= IONDLY;
	if ((fdp->f_flag & IPNONBLOCK) != 0)
		io.io_flag |= IONONBLOCK;

	if (do_write) {
		iwrite (fdp->f_ip, & io);
	} else {
		iread (fdp->f_ip, & io);
		iaccessed (fdp->f_ip);		/* read - atime */
	}
	n -= io.io_ioc;
	fdp->f_seek += n;

	if (type != IFCHR) {
		/* Was this inode opened for synchronous writes?  */
		if ((fdp->f_flag & IPSYNC) != 0)
			icopymd (fdp->f_ip);

		iunlock (fdp->f_ip);
	}

	return n;
}


/*
 * Read `n' bytes into the buffer `bp' from file number `fd'.
 */

int
uread(fd, bp, n)
int fd;
caddr_t bp;
size_t n;
{
	return sysio (fd, bp, n, 0);
}


/*
 * Return a status structure for the given file name.
 */

int
ustat(np, stp)
char *np;
struct stat *stp;
{
	struct stat	stat;
	struct direct	dir;

	if (ftoi (np, 'r', IOUSR, NULL, & dir, SELF->p_credp) != 0)
		return -1;

	istat (u.u_cdiri, & stat);
	idetach (u.u_cdiri);

	if (kucopy (& stat, stp, sizeof (stat)) != sizeof (stat)) {
		set_user_error (EFAULT);
		return -1;
	}
	return 0;
}


/*
 * Write out all modified buffers, inodes and super blocks to disk.
 */

int
usync()
{
	MOUNT * mp;
	static __DUMB_GATE syngate = __GATE_DECLARE ("sync");

	__GATE_LOCK (syngate, "lock : usync ()");
	for (mp = mountp ; mp != NULL ; mp = mp->m_next)
		msync (mp);
	bsync ();
	__GATE_UNLOCK (syngate);
	return 0;
}


/*
 * Set the mask for file access.
 */

int
uumask (mask)
int mask;
{
	int		omask;

	omask = u.u_umask;
	u.u_umask = mask & 0777;
	return omask;
}


/*
 * Unmount the given device.
 */

int
uumount(sp)
char *sp;
{
	INODE *ip;
	MOUNT *mp;
	MOUNT **mpp;
	dev_t rdev;
	int mode;
	struct direct	dir;

	if (ftoi (sp, 'r', IOUSR, NULL, & dir, SELF->p_credp) != 0)
		return -1;

	u.u_cdiri = u.u_cdiri;
	if (iaccess (u.u_cdiri, IPR | IPW, SELF->p_credp) == 0) {
		idetach (u.u_cdiri);
		return -1;
	}

	rdev = u.u_cdiri->i_rdev;
	mode = u.u_cdiri->i_mode;

	idetach (u.u_cdiri);
	if ((mode & IFMT) != IFBLK) {
		set_user_error (ENOTBLK);
		return -1;
	}

	for (mpp = & mountp ; (mp = * mpp) != NULL ; mpp = & mp->m_next)
		if (mp->m_dev == rdev)
			break;

	if (mp == NULL) {
		set_user_error (EINVAL);
		return -1;
	}

	msync (mp);
	for (ip = inode_table ; ip < inode_table_end ; ip ++) {
		if (ip->i_dev == rdev && ip->i_refc > 0) {
			set_user_error (EBUSY);
			return -1;
		}
	}
	ASSERT (ip == inode_table_end);

	for (ip = inode_table ; ip < inode_table_end ; ip ++) {
		if (ip->i_dev == rdev) {
			ASSERT (! ilocked (ip));
#if	0
			cmn_err (CE_NOTE, "umount ino = %d size = %ld",
				 ip->i_ino, ip->i_size);
#endif
			ip->i_ino = 0;
		}
	}
	ASSERT (ip == inode_table_end);

	bflush (rdev);
	dclose (rdev, mp->m_flag ? IPR : IPR | IPW, DFBLK, NULL);
	* mpp = mp->m_next;
	mp->m_ip->i_flag &= ~ IFMNT;

	ldetach (mp->m_ip);
	kmem_free (mp, sizeof (* mp));
	return 0;
}


/*
 * Internal version of unlink () called by uunlink () and umkdir ().
 */

int
do_unlink (path, space)
caddr_t		path;
int		space;
{
	INODE * ip;
	dev_t dev;
	IO		io;
	struct direct	dir;
	unsigned	olderror;

	/*
	 * We start by clearing u_error because we are called from umkdir ()
	 * in a situation where the active error number is not relevant to
	 * us. We return the old error number so that umkdir () can restore
	 * the error number it wants easily.
	 */

	olderror = get_user_error ();
	set_user_error (0);

	if (ftoi (path, 'u', space, & io, & dir, SELF->p_credp) != 0)
		return olderror;

	if (iaccess (u.u_pdiri, IPW, SELF->p_credp) == 0) {
		set_user_error (EACCES);
		idetach (u.u_pdiri);
		return olderror;
	}
	dev = u.u_pdiri->i_dev;

	if (iucheck (dev, u.u_cdirn) == 0) {
		idetach (u.u_pdiri);
		return olderror;
	}

	idirent (0, & io, & dir);
	idetach (u.u_pdiri);

	if ((ip = iattach (dev, u.u_cdirn)) == NULL)
		return 0;

	if (ip->i_nlink > 0)
		-- ip->i_nlink;
	icreated (ip);	/* unlink - ctime */
	idetach (ip);
	return olderror;
}


/*
 * Unlink the given file.
 */

int
uunlink (np)
char *np;
{
	(void) do_unlink (np, IOUSR);
	return 0;
}


/*
 * Set file times.
 */

int
uutime (np, utime)
char *np;
time_t utime [2];
{
	struct {
		time_t		_time [2];
	} stime;
	struct direct	dir;

	if (ftoi (np, 'r', IOUSR, NULL, & dir, SELF->p_credp) != 0)
		return -1;

	if (iprotected (u.u_cdiri)) {
		set_user_error (EROFS);
		idetach (u.u_cdiri);
		return -1;
	}

	if (owner (u.u_cdiri->i_uid)) {
		iaccmodcreat (u.u_cdiri);	/* utime - atime/mtime/ctime */
		if (utime != NULL) {
			if (ukcopy (utime, & stime,
				    sizeof (stime)) != sizeof (stime)) {
				set_user_error (EFAULT);
			} else {
				u.u_cdiri->i_atime = stime._time [0];
				u.u_cdiri->i_mtime = stime._time [1];
			}
		}
	}
	idetach (u.u_cdiri);
	return 0;
}


/*
 * Write `n' bytes from buffer `bp' on file number `fd'.
 */

int
uwrite(fd, bp, n)
int fd;
caddr_t bp;
unsigned n;
{
	return sysio (fd, bp, n, 1);
}


/*
 *
 * int
 * useracc(base, count, writeUsr) -- determine user accessibility
 * caddr_t base;
 * int count;
 * int writeUsr;
 *
 *	Input:	base  = offset in user data space of the region to be accessed.
 *		count = size of access region in bytes.
 *		writeUsr = 0 if read access to be checked, else write
 *
 *	Action:	Verify user has desired access mode into specified region.
 *
 *	Return:	0 = permission denied.
 *		1 = access allowed.
 *
 *	Notes:	Mode is ignored for now, but is required for compatibility
 *		with System V, and future protected mode extensions.
 */

int
useracc(base, count, writeUsr)
register char *base;
int writeUsr, count;
{
	if (base + count >= base) {
		int		ret;

		ret = accdata (base, count) || accstack (base, count) ||
			accShm (base, count);
		if (! writeUsr)
			ret = ret || acctext (base, count);

		return ret;
	}
	return 0;
}


/*
 * strUserAcc(str, writeUsr) - Check user accessibility of 0 terminated string.
 *
 * char	*str;		null-terminated string,
 * int	writeUsr;	0 if read access to be checked, else write.
 *
 * Returns string size on success (without 0), -1 otherwise.
 *
 * It is interface to useracc() when count is not known.
 */

int
strUserAcc(str, writeUsr)
char	*str;
int	writeUsr;
{
	register char	*ch;

	if (! useracc (str, 1, writeUsr))
		return -1;

	for (ch = str ; * ch != 0 ; ch ++) 
		if (! useracc (ch + 1, 1, writeUsr)) 
			return -1;

	return ch - str;
}

/*
 * "Safe" ukcopy and kucopy - use useracc to check user address supplied.
 */

int
kucopyS(kernel, user, n)
caddr_t kernel;
caddr_t user;
size_t n;
{
	if (useracc (user, n, 1))
		return kucopy (kernel, user, n);
	else {
		set_user_error (EFAULT);
		return 0;
	}
}

int
ukcopyS(user, kernel, n)
caddr_t kernel;
caddr_t user;
size_t n;
{
	if (useracc (user, n, 0))
		return ukcopy (user, kernel, n);
	else {
		set_user_error (EFAULT);
		return 0;
	}
}
