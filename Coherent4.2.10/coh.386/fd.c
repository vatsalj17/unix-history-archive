/* $Header: /ker/coh.386/RCS/fd.c,v 2.6 93/10/29 00:55:07 nigel Exp Locker: nigel $ */
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
 * File descriptor routines.
 *
 * $Log:	fd.c,v $
 * Revision 2.6  93/10/29  00:55:07  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.5  93/08/19  03:26:25  nigel
 * Nigel's r83 (Stylistic cleanup)
 * 
 * Revision 2.4  93/07/26  15:09:56  nigel
 * Nigel's R80
 * 
 * Revision 1.5  93/04/14  10:06:26  root
 * r75
 * 
 * Revision 1.3  92/06/10  12:52:39  hal
 * First record locking changes.
 */

#include <common/ccompat.h>
#include <kernel/proc_lib.h>
#include <sys/debug.h>
#include <sys/errno.h>
#include <sys/kmem.h>
#include <fcntl.h>
#include <stddef.h>

#define	_KERNEL		1

#include <kernel/param.h>
#include <sys/uproc.h>
#include <sys/fd.h>
#include <sys/inode.h>


/*
 * Operations for converting from tagged to untagged "pointer to system file
 * table entry" and vice versa.
 */

enum {
	TAGMASK		= sizeof (int) - 1
};

#define	MAKE_TAGGED_FD(untag,flag) \
	(ASSERT (((int) (untag) & TAGMASK) == 0), \
	 ASSERT (((flag) & ~ TAGMASK) == 0), \
	 (tagfd_t) ((unsigned long) (untag) + (flag)))

#define	GET_UNTAGGED_FD(tagged) \
	((__fd_t *) ((unsigned long) (tagged) & ~ TAGMASK))

#define	GET_TAGGED_FLAG(tagged) \
	((int) (tagged) & TAGMASK)

/*
 * Given a file number, return the file descriptor. Now that file descriptors
 * stored in the U area have their low bits tagged to store flag information
 * that is FD-specific (gotta avoid that space crunch in the U area), this
 * interface deals with masking out any extra stuff and returning a plain
 * pointer.
 */

#if	__USE_PROTO__
__fd_t * fd_get (fd_t fd)
#else
__fd_t *
fd_get (fd)
fd_t		fd;
#endif
{
	__fd_t	      *	fdp;

	if (fd >= NOFILE || (fdp = GET_UNTAGGED_FD (u.u_filep [fd])) == NULL)
		return NULL;
	return fdp;
}


/*
 * Return tag bits from file descriptor.
 */

#if	__USE_PROTO__
int fd_get_flags (fd_t fd)
#else
int
fd_get_flags (fd)
fd_t		fd;
#endif
{
	if (fd >= NOFILE || u.u_filep [fd] == NULL)
		return -1;
	return GET_TAGGED_FLAG (u.u_filep [fd]);
}


/*
 * This function allows clients to set the flag bits.
 */

#if	__USE_PROTO__
int fd_set_flags (fd_t fd, int flags)
#else
int
fd_set_flags (fd, flags)
fd_t		fd;
int		flags;
#endif
{
	if (fd >= NOFILE || u.u_filep [fd] == NULL)
		return -1;

	if ((flags & ~ TAGMASK) != 0) {
		set_user_error (EINVAL);
		return -1;
	}

	u.u_filep [fd] = MAKE_TAGGED_FD (GET_UNTAGGED_FD (u.u_filep [fd]),
					 flags);
	return 0;
}


/*
 * This function performs something similar to the F_DUPFD function of
 * fcntl ()... this functionality is needed several places internally.
 */

#if	__USE_PROTO__
fd_t fd_dup (fd_t old, fd_t base)
#else
fd_t
fd_dup (old, base)
fd_t		old;
fd_t		base;
#endif
{
	while (base < NOFILE)
		if (u.u_filep [base] == NULL) {
			__fd_t	      *	fdp = fd_get (old);

			if (fdp == NULL)
				return ERROR_FD;
			u.u_filep [base] = MAKE_TAGGED_FD (fdp, 0);
			fdp->f_refc ++;
			return base;
		} else
			base ++;

	set_user_error (EMFILE);
	return ERROR_FD;
}


/*
 * This function finds a free slot in the process file descriptor table and
 * returns the index.
 */

#if	__USE_PROTO__
fd_t fd_get_free (void)
#else
fd_t
fd_get_free ()
#endif
{
	int		i;

	for (i = 0 ; i < sizeof (u.u_filep) / sizeof (* u.u_filep) ; i ++) {
		if (u.u_filep [i] == NULL)
			return i;
	}

	return ERROR_FD;
}


/*
 * This function finds a free slot in the process file descriptor table and
 * fills it in with a partially initialised entry.
 *
 * This function returns a file descriptor number on success, -1 on failure.
 */

#if	__USE_PROTO__
fd_t fd_alloc (void)
#else
fd_t
fd_alloc ()
#endif
{
	fd_t		fd;

	if ((fd = fd_get_free ()) == ERROR_FD) {
		set_user_error (EMFILE);
	} else {
		__fd_t	      *	filep;

		if ((filep = kmem_alloc (sizeof (__fd_t),
					 KM_NOSLEEP)) == NULL) {
			/*
			 * Insufficient resources!
			 */

			set_user_error (ENFILE);
			return ERROR_FD;
		}

		u.u_filep [fd] = MAKE_TAGGED_FD (filep, 0);

		filep->f_flag = 0;
		filep->f_refc = 0;
		filep->f_seek = 0;
		filep->f_ip = NULL;
	}

	return fd;
}


/*
 * This function performs the second half of the file open process by filling
 * in the inode member of the file table entry and requesting that the inode be
 * opened.
 *
 * This function returns -1 on error, 0 on success.
 */

#if	__USE_PROTO__
int fd_init (fd_t fd, struct inode * ip, int mode)
#else
int
fd_init (fd, ip, mode)
fd_t		fd;
struct inode  *	ip;
int		mode;
#endif
{
	__fd_t	      *	filep;
	INODE	      *	newip;

	if ((filep = fd_get (fd)) == NULL)
		return -1;

	ASSERT (ilocked (ip));

	newip = iopen (ip, mode);

	if (get_user_error () != 0)
		return -1;

	if (newip == NULL) {
		/*
		 * This should only happen when there is some kind of internal
		 * error. Still, we cope with it. iopen () should have still
		 * made sure the "ip" was locked, BTW.
		 */

		set_user_error (ENXIO);
		return -1;
	}

	if (newip != ip) {
		/*
		 * In the case of something like a STREAMS clone operation,
		 * iopen () will have returned a new inode but not locked it,
		 * and left the old inode locked. Transfer over to the new
		 * inode.
		 */

		idetach (ip);
		ilock (newip, "fd_init ()");
	}
	ASSERT (ilocked (newip));

	filep->f_ip = newip;
	filep->f_flag = mode;
	filep->f_refc = 1;

	return 0;
}


/*
 * This function finalises an open; normally this does nothing, but if there
 * has been an error, this code will take care of deallocating the entry.
 *
 * This function returns the file descriptor number on success, or -1 on error.
 */

#if	__USE_PROTO__
fd_t fd_finish (fd_t fd)
#else
fd_t
fd_finish (fd)
fd_t		fd;
#endif
{
	__fd_t	      *	filep;

	if ((filep = fd_get (fd)) == NULL)
		return ERROR_FD;

	if (filep->f_refc == 0) {
		/*
		 * The open never really succeeded, release resources.
		 */

		kmem_free (filep, sizeof (* filep));
		u.u_filep [fd] = NULL;
		fd = ERROR_FD;
	}

	return fd;
}


/*
 * Given an inode, and a mode containing permission flags, open the
 * inode with the appropriate permissions and return a file descriptor
 * containing it.
 */

#if	__USE_PROTO__
fd_t fd_open (INODE * ip, int mode)
#else
fd_t
fd_open (ip, mode)
INODE	      *	ip;
int		mode;
#endif
{
	int		fd;

	if ((fd = fd_alloc ()) != ERROR_FD) {
		fd_init (fd, ip, mode);
		fd = fd_finish (fd);
	}

	return fd;
}


/*
 * Given an existing system file table directory, allocate a file table entry
 * and return the index.
 */

#if	__USE_PROTO__
int fd_recv (fd_t fd, __fd_t * fdp)
#else
int
fd_recv (fd, fdp)
fd_t		fd;
__fd_t	      *	fdp;
#endif
{
	ASSERT (fd != ERROR_FD);
	ASSERT (fdp != NULL);

	u.u_filep [fd] = MAKE_TAGGED_FD (fdp, 0);
	fdp->f_refc ++;
	return 0;
}


/*
 * Close the given file number.
 */

#if	__USE_PROTO__
void fd_close (fd_t fd)
#else
void
fd_close (fd)
fd_t		fd;
#endif
{
	__fd_t	      *	fdp;
	static	struct flock	flock = { F_UNLCK, 0, 0, 0 };

	if (fd >= NOFILE ||
	    (fdp = GET_UNTAGGED_FD (u.u_filep [fd])) == NULL) {
		set_user_error (EBADF);
		return;
	}

	if (fdp->f_ip->i_rl != NULL)
		rlock (fdp, F_SETLK, & flock);	/* delete all record locks */

	u.u_filep[fd] = NULL;

	ASSERT (fdp->f_refc != 0);

	if (-- fdp->f_refc == 0) {
		do {
			set_user_error (0);
			iclose (fdp->f_ip, fdp->f_flag);
		} while (get_user_error () == EINTR);
		kmem_free (fdp, sizeof (* fdp));
	}
}


/*
 * Assuming we have made a copy of the user area, increment the reference
 * count of all open files.  (used in fork).
 */

#if	__USE_PROTO__
void fd_dup_all (void)
#else
void
fd_dup_all ()
#endif
{
	int		i;

	for (i = 0 ; i < NOFILE ; i ++)
		if (u.u_filep [i] != NULL)
			GET_UNTAGGED_FD (u.u_filep [i])->f_refc ++;
}


/*
 * Close all open files in the current process.
 */

#if	__USE_PROTO__
void fd_close_all (void)
#else
void
fd_close_all ()
#endif
{
	int		fd;

	for (fd = 0 ; fd < NOFILE ; fd ++)
		if (u.u_filep [fd] != NULL)
			fd_close (fd);
}
