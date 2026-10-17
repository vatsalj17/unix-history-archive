/* $Header: /ker/coh.386/RCS/sys2.c,v 2.8 93/10/29 00:55:42 nigel Exp Locker: nigel $ */
/*
 * Filesystem related system calls.
 * $Log:	sys2.c,v $
 * Revision 2.8  93/10/29  00:55:42  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.7  93/09/13  08:00:36  nigel
 * In an earlier cleanup of uaccess (), a call got moved outside of schizo ()
 * that shouldn't have been, causing a variety of problems with the old-style
 * directory utilities.
 * 
 * Revision 2.6  93/08/19  03:26:51  nigel
 * Nigel's r83 (Stylistic cleanup)
 */

#include <common/_gregset.h>
#include <common/whence.h>
#include <kernel/_sleep.h>
#include <kernel/proc_lib.h>
#include <kernel/cred_lib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/cred.h>
#include <stddef.h>
#include <fcntl.h>

#define	_KERNEL		1

#include <kernel/_timers.h>
#include <kernel/trace.h>
#include <sys/uproc.h>
#include <sys/proc.h>
#include <sys/fd.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/mount.h>
#include <sys/sched.h>


/*
 * Determine accessibility of the given file.
 */

int
uaccess(np, mode)
char *np;
int mode;
{
	struct direct	dir;
	cred_t		fake_cred;

	(void) cred_fake (& fake_cred, SELF->p_credp);

	if (ftoi (np, 'r', IOUSR, NULL, & dir, & fake_cred))
		return -1;

	if (! iaccess (u.u_cdiri, mode, & fake_cred))
		set_user_error (EACCES);

	idetach (u.u_cdiri);
	return 0;
}


/*
 * Turn accounting on or off.
 */

int
uacct(np)
char *np;
{
	if (super () == 0)
		return -1;

	if (np == NULL) {
		if (acctip == NULL) {
			set_user_error (EINVAL);
			return -1;
		}

		ldetach (acctip);
		acctip = NULL;
	} else {
		INODE	      *	ip;
		struct direct	dir;

		if (acctip != NULL) {
			set_user_error (EINVAL);
			return -1;
		}

		if (ftoi (np, 'r', IOUSR, NULL, & dir, SELF->p_credp))
			return -1;

		ip = u.u_cdiri;
		if ((ip->i_mode & IFMT) != IFREG) {
			set_user_error (EINVAL);
			idetach (ip);
			return -1;
		}
		iunlock (ip);
		acctip = ip;
	}
	return 0;
}


/*
 * Given a directory name and a pointer to a working directory pointer,
 * Save the inode associated with the directory name in the working
 * directory pointer and release the old one.  This is used to change
 * working and root directories.
 */

#if	__USE_PROTO__
__LOCAL__ int setcdir (__CONST__ char * path, struct inode ** ipp)
#else
__LOCAL__ int
setcdir(path, ipp)
__CONST__ char *path;
INODE **ipp;
#endif
{
	struct direct	dir;

	if (ftoi (path, 'r', IOUSR, NULL, & dir, SELF->p_credp))
		return -1;

	if ((u.u_cdiri->i_mode & IFMT) != IFDIR) {
		set_user_error (ENOTDIR);
		idetach (u.u_cdiri);
		return -1;
	}
	if (iaccess (u.u_cdiri, IPE, SELF->p_credp) == 0) {
		set_user_error (EACCES);
		idetach (u.u_cdiri);
		return -1;
	}
	iunlock (u.u_cdiri);
	ldetach (* ipp);
	* ipp = u.u_cdiri;
	return 0;
}


/*
 * Set current directory.
 */

int
uchdir(path)
char *path;
{
	return setcdir (path, & u.u_cdir);
}


/*
 * Change the mode of a file.
 */

int
uchmod (np, mode)
char *np;
int mode;
{
	struct direct	dir;

	if (ftoi (np, 'r', IOUSR, NULL, & dir, SELF->p_credp))
		return -1;

	if (iprotected (u.u_cdiri)) {
		set_user_error (EROFS);
		idetach (u.u_cdiri);
		return -1;
	}
		
	if (owner (u.u_cdiri->i_uid)) {
		if (drv_priv (SELF->p_credp))
			mode &= ~ ISVTXT;

		u.u_cdiri->i_mode &= IFMT;
		u.u_cdiri->i_mode |= mode & ~ IFMT;
		icreated (u.u_cdiri);	/* chmod - ctime */
	}
	idetach (u.u_cdiri);
	return 0;
}


/*
 * Change owner and group of a file.
 */

int
uchown(np, uid, gid)
uid_t		uid;
gid_t		gid;
char *np;
{
	struct direct	dir;

	if (ftoi (np, 'r', IOUSR, NULL, & dir, SELF->p_credp))
		return -1;

	if (iprotected (u.u_cdiri)) {
		set_user_error (EROFS);
		idetach (u.u_cdiri);
		return -1;
	}

	if (uid != (uid_t) -1 && u.u_cdiri->i_uid != uid) {
		if (! super ()) {
			idetach (u.u_cdiri);
			return -1;	/* only root can change uid */
		}

		u.u_cdiri->i_uid = uid;
	}

	if (gid != (gid_t) -1 && u.u_cdiri->i_gid != gid) {
		if (! super ()) {
			/*
			 * We must be the file owner to change group... note
			 * that super () has set EPERM for us.
			 */

			if (cred_match (SELF->p_credp, u.u_cdiri->i_uid,
					(gid_t) -1) != _CRED_OWNER ||
			    cred_match (SELF->p_credp, (uid_t) -1,
					gid) != _CRED_GROUP) {
				idetach (u.u_cdiri);
				return -1;
			}

			set_user_error (0);
		}

		u.u_cdiri->i_gid = gid;
	}

	if (! drv_priv (SELF->p_credp)) {
		/*
		 * If we are not root, always turn off set-[ug]id bits, unless
		 * the -1 (no change) convention was used.
		 */

		if (uid != (uid_t) -1)
			u.u_cdiri->i_mode &= ~ ISUID;
		if (gid != (gid_t) -1)
			u.u_cdiri->i_mode &= ~ ISGID;
	}

	icreated (u.u_cdiri);	/* chown - ctime */
	idetach (u.u_cdiri);
	return 0;
}


/*
 * Set root directory.
 */

int
uchroot(np)
char *np;
{
	if (super ())
		return setcdir (np, & u.u_rdir);
	return -1;
}


/*
 * Close the given file descriptor.
 */

int
uclose(fd)
int fd;
{
	fd_close (fd);
	return 0;
}


/*
 * Create a file with the given mode.
 */

int
ucreat(np, mode)
char *np;
int mode;
{
	return uopen (np, O_WRONLY | O_CREAT | O_TRUNC, mode);
}


/*
 * Duplicate a file descriptor.
 */

int
udup (ofd)
int ofd;
{
	return fd_dup (ofd, 0);
}


/*
 * Given a file descriptor, return a status structure.
 */

int
ufstat (fd, stp)
int fd;
struct __R3STAT_TAG
	      *	stp;
{
	__fd_t	      *	fdp;
	struct __R3STAT_TAG
			stat;

	if ((fdp = fd_get (fd)) == NULL) {
		set_user_error (EBADF);
		return -1;
	}

	istat (fdp->f_ip, & stat);

	if (kucopy (& stat, stp, sizeof (stat)) != sizeof (stat)) {
		set_user_error (EFAULT);
		return -1;
	}
	return 0;
}


/*
 * File control.
 */

int
ufcntl (fd, cmd, arg)
unsigned	fd;
unsigned	cmd;
unsigned	arg;
{
	__fd_t	      *	fdp;
	struct flock sfl;

	T_VLAD(2, printf("fcntl(%d,%x,%x) ", fd, cmd, arg));

	/*
	 * Validate file descriptor.
	 */

	if ((fdp = fd_get (fd)) == NULL) {
		set_user_error (EBADF);
		return -1;
	}

	switch (cmd) {

	case F_DUPFD:
		/*
		 * Validate base file descriptor.
		 */
		if (arg >= NOFILE) {
			set_user_error (EINVAL);
			return -1;
		}

		/*
		 * Search for next available file descriptor.
		 */

		return fd_dup (fd, arg);

	case F_SETFL:
		fdp->f_flag &= ~ (IPNDLY | IPAPPEND | IPNONBLOCK);
		if (arg & O_NDELAY)
			fdp->f_flag |= IPNDLY;
		if (arg & O_APPEND)
			fdp->f_flag |= IPAPPEND;
		if (arg & O_NONBLOCK)
			fdp->f_flag |= IPNONBLOCK;

		/*
		 * NIGEL: Special hack for FS debugging.
		 */

		if ((arg & O_TRACE) != 0)
			itrace (fdp->f_ip);
		else
			iuntrace (fdp->f_ip);

		/*
		 * Originally, this call returned the previous flag values,
		 * as permitted by the various standards. However, many
		 * programs incorrectly check for "== 0" as the return
		 * condition from this function rather than "!= -1" as they
		 * should.
		 */
		return 0;

	case F_GETFL:
		switch (fdp->f_flag & (IPR | IPW)) {
		case IPR: arg = O_RDONLY; break;
		case IPW: arg = O_WRONLY; break;
		default:  arg = O_RDWR;   break;
		}

		if ((fdp->f_flag & IPNDLY) != 0)
			arg |= O_NDELAY;

		if ((fdp->f_flag & IPAPPEND) != 0)
			arg |= O_APPEND;

		if ((fdp->f_flag & IPNONBLOCK) != 0)
			arg |= O_NONBLOCK;
		/*
		 * NIGEL: Special hack for FS debugging.
		 */

		if (itraced (fdp->f_ip))
			arg |= O_TRACE;
		return arg;

	case F_GETLK:
	case F_SETLK:
	case F_SETLKW:
		if (ukcopy ((struct flock *) arg, & sfl,
			    sizeof (struct flock)) != sizeof (struct flock)) {
			set_user_error (EFAULT);
			return -1;
		}
		if (rlock (fdp, cmd, & sfl))
			return -1;
		if (cmd == F_GETLK &&
		    kucopy (& sfl, (struct flock *) arg,
			    sizeof (struct flock)) != sizeof (struct flock)) {
			set_user_error (EFAULT);
			return -1;
		}
		return 0;

	case F_GETFD:
		return fd_get_flags (fd);

	case F_SETFD:
		return fd_set_flags (fd, arg & FD_CLOEXEC);

	default:
		T_VLAD (0x02, printf ("'fcntl - unknown cmd=%d arg=0x0%x' ",
				      cmd, arg));
		set_user_error (EINVAL);
		return -1;
	}
}


/*
 * Device control information.
 */

int
uioctl (fd, cmd, argp, regsetp)
unsigned	fd;
unsigned	cmd;
__VOID__      *	argp;
gregset_t     *	regsetp;
{
	__fd_t	      *	fdp;
	INODE *ip;
	int mode;

	if ((fdp = fd_get (fd)) == NULL) {
		set_user_error (EBADF);
		return -1;
	}
	ip = fdp->f_ip;
	mode = ip->i_mode & IFMT;
	if (mode != IFCHR && mode != IFBLK) {
		set_user_error (ENOTTY);
		return -1;
	}
	return dioctl (ip->i_rdev, cmd, argp, fdp->f_flag, ip->i_private,
		       regsetp);
}


/*
 * Internal version of link (), used by ulink () and umkdir ().
 */

int
do_link (path1, path2, space, needperm)
char	      *	path1;
char	      *	path2;
int		space;
int		needperm;	/* need superuser permissions */
{
	INODE *ip1;
	IO		io;
	struct direct	dir;

	if (ftoi (path1, 'r', space, NULL, & dir, SELF->p_credp))
		return -1;

	ip1 = u.u_cdiri;
	if ((ip1->i_mode & IFMT) == IFDIR && needperm && super () == 0) {
		idetach (ip1);
		return -1;
	}

	iunlock (ip1);

	if (ftoi (path2, 'c', space, & io, & dir, SELF->p_credp)) {
		ldetach (ip1);
		return -1;
	}

	if (u.u_cdiri != NULL) {
		set_user_error (EEXIST);
		idetach (u.u_cdiri);
		ldetach (ip1);
		return -1;
	}

	if (ip1->i_dev != u.u_pdiri->i_dev) {
		set_user_error (EXDEV);
		idetach (u.u_pdiri);
		ldetach (ip1);
		return -1;
	}

	if (iaccess (u.u_pdiri, IPW, SELF->p_credp) == 0) {
		idetach (u.u_pdiri);
		ldetach (ip1);
		return -1;
	}

	idirent (ip1->i_ino, & io, & dir);
	idetach (u.u_pdiri);
	ilock (ip1, "do_link ()");

	/*
	 * idirent() can fail during iwrite. In this case we should not 
         * increase link count. 
	 * As result of this old bug, 286 mkdir utility destroys file 
	 * system when runs out of free blocks.
	 */

	if (! get_user_error ())
		ip1->i_nlink ++;

	icreated (ip1);	/* link - ctime */
	idetach (ip1);
	return 0;
}


/*
 * Create a link, `np2' to the already existing file `np1'.
 */

int
ulink (np1, np2)
char *np1;
char *np2;
{
	return do_link (np1, np2, IOUSR, 1);
}


/*
 * Seek on the given file descriptor.
 */

off_t
ulseek (fd, off, whence)
int fd;
off_t off;
int whence;
{
	__fd_t	      *	fdp;

	if ((fdp = fd_get (fd)) == NULL) {
		set_user_error (EBADF);
		return -1;
	}

	if ((fdp->f_ip->i_mode & IFMT) == IFPIPE) {
		set_user_error (ESPIPE);
		return -1;
	}

	switch (whence) {
	case SEEK_SET:
		break;

	case SEEK_CUR:
		off += fdp->f_seek;
		break;

	case SEEK_END:
		off += fdp->f_ip->i_size;
		break;

	default:
		set_user_error (EINVAL);
		return -1;
	}

	if (off < 0) {
		set_user_error (EINVAL);
		return -1;
	}

	fdp->f_seek = off;

	return off;
}


/*
 * Create a special file.
 */

int
umknod (np, mode, rdev)
char * np;
int mode;
o_dev_t rdev;
{
	int		type;
	INODE	      *	ip;
	IO		io;
	struct direct	dir;

	type = mode & IFMT;
	if (type != IFPIPE && super () == 0)
		return -1;
	if (type != IFBLK && type != IFCHR)
		rdev = 0;

	if (ftoi (np, 'c', IOUSR, & io, & dir, SELF->p_credp))
		return -1;

	if (u.u_cdiri != NULL) {
		set_user_error (EEXIST);
		idetach (u.u_cdiri);
		return -1;
	}

	if ((ip = imake (mode, rdev, & io, & dir, SELF->p_credp)) == NULL)
		return -1;

	idetach (ip);
	return 0;
}


/*
 * Mount the device `sp' on the pathname `np'.  The flag, `f',
 * indicates that the device is to be mounted read only.
 */

int
umount(sp, np, readonly)
char *sp;
char *np;
int readonly;
{
	MOUNT *mp;
	dev_t rdev;
	int mode;
	struct direct	dir;

	if (ftoi (sp, 'r', IOUSR, NULL, & dir, SELF->p_credp))
		return -1;

	if (iaccess (u.u_cdiri, IPR | IPW, SELF->p_credp) == 0)
		goto err;

	mode = u.u_cdiri->i_mode;
	rdev = u.u_cdiri->i_rdev;
	if ((mode & IFMT) != IFBLK) {
		set_user_error (ENOTBLK);
		goto err;
	}

	idetach (u.u_cdiri);

	if (ftoi (np, 'r', IOUSR, NULL, & dir, SELF->p_credp))
		return -1;

	if ((u.u_cdiri->i_mode & IFMT) != IFDIR) {
		set_user_error (ENOTDIR);
		goto err;
	}

	if (iaccess (u.u_cdiri, IPR, SELF->p_credp) == 0)
		goto err;

	/* Check for current directory, open, or mount directory */

	if (u.u_cdiri->i_refc > 1 || u.u_cdiri->i_ino == ROOTIN) {
		set_user_error (EBUSY);
		goto err;
	}
	for (mp = mountp ; mp != NULL ; mp = mp->m_next) {
		if (mp->m_dev == rdev) {
			set_user_error (EBUSY);
			goto err;
		}
	}

	if ((mp = fsmount (rdev, readonly)) == NULL) {
err:
		idetach (u.u_cdiri);
		return -1;
	}

	mp->m_ip = u.u_cdiri;
	u.u_cdiri->i_flag |= IFMNT;
	u.u_cdiri->i_refc ++;
	idetach (u.u_cdiri);
	return 0;
}


/*
 * Suspend execution for a short interval.
 *
 * Return the number of milliseconds actually slept.
 */

int
unap (msec)
int msec;
{
	int ret, lbolt0;
	int ticksToWait, ticksWaited;

	if (msec <= 0)
		return 0;

	/*
	 * Convert milliseconds to clock ticks.
	 *
	 * Wait for at least the specified number of milliseconds.
	 * For 100 Hz clock, if nap is for 11 msec, timeout is for 2 ticks.
	 */
	ticksToWait = ((msec * HZ) + 999) / 1000;
	timeout (& SELF->p_polltim, ticksToWait, wakeup, & SELF->p_polls);

	/*
	 * Wake for timeout or signal.
	 */

	lbolt0 = lbolt;
	if (x_sleep (& SELF->p_polls, pritty, slpriSigCatch,
		     "nap") == PROCESS_SIGNALLED) {
		/*
		 * Signal woke us up.
		 */
		set_user_error (EINTR);
		ret = -1;
	} else {
		/*
		 * We were awakened by a timeout.
		 * Return number of milliseconds actually waited.
		 */
		ticksWaited = lbolt - lbolt0;
		if (ticksWaited >= 0)
			ret = (ticksWaited * 1000) / HZ;
		else
			ret = 0;
	}

	/*
	 * Cancel timeout
 	 */

	timeout (& SELF->p_polltim, 0, NULL, NULL);
	return ret;
}

