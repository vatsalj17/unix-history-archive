/* $Header: /ker/coh.386/RCS/sys5.c,v 2.6 93/10/29 00:55:50 nigel Exp Locker: nigel $ */
/*
 * System calls introduced when going from COH 286 to COH 386.
 * $Log:	sys5.c,v $
 * Revision 2.6  93/10/29  00:55:50  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.5  93/08/19  03:26:55  nigel
 * Nigel's r83 (Stylistic cleanup)
 */

#include <kernel/proc_lib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <ulimit.h>
#include <dirent.h>
#include <fcntl.h>
#include <stddef.h>
#include <limits.h>

#define	_KERNEL		1

#include <kernel/alloc.h>
#include <kernel/trace.h>
#include <kernel/reg.h>
#include <sys/uproc.h>
#include <sys/proc.h>
#include <sys/mmu.h>
#include <sys/buf.h>
#include <canon.h>
#include <sys/con.h>
#include <sys/fd.h>
#include <sys/filsys.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/io.h>
#include <sys/mount.h>
#include <sys/utsname.h>
#include <sys/mount.h>
#include <ustat.h>
#include <sys/statfs.h>
#include <sys/sysi86.h>


time_t
utime ()
{
	return timer.t_time;
}


/*
 * Return an unique number.
 */

int
usysi86 (f, arg1)
int f;
int arg1;
{
	MOUNT *mp;
	struct filsys *fsp;
	extern void (*ndpEmFn)();
	int fpval;
	extern short ndpType;

	switch (f) {
	case SYI86UNEEK:
		if ((mp = getment (rootdev, 1)) == NULL)
			return -1;
		fsp = & mp->m_super;
		fsp->s_fmod = 1;
		return ++ fsp->s_unique;

	case SI86FPHW:
		/* 
		 * 2's bit: floating point ndp is present (80287/80387/80486dx)
		 * 1's bit (when 2's bit = 1): 80387/486dx is present
		 */

		if (! useracc (arg1, sizeof (int), 1)) {
			SET_U_ERROR (EFAULT, "sysi386:SI86FPHW");
			return -1;
		}
		if (ndpType <= 1) { /* no ndp */
			fpval = (ndpEmFn) ? FP_SW : FP_NO;
		} else
			fpval = (ndpType > 2) ? FP_387 : FP_287;

		putuwd (arg1, fpval);
		return 0;
	}

	set_user_error (EINVAL);
	return -1;
}


/*
 * sys_unknown - write name unknown to utsname struct
 */

#if	__USE_PROTO__
__LOCAL__ int sys_unknown (struct utsname * name)
#else
__LOCAL__ int
sys_unknown (name)
struct utsname	*name;
#endif
{
static	char	unknown[] = "UNKNOWN";

	if (! kucopy (unknown, name->sysname, sizeof (unknown)))
		return -1;
	if (! kucopy (unknown, name->nodename, sizeof (unknown)))
		return -1;
	return 0;
}


/*
 * uname - get name of the current operating system.
 */

extern char	version[];	/* Defined in main.c */
extern char 	release[];	/* Defined in main.c */

int
uname(name)
struct utsname	*name;
{
	register char	*rcp;			/**/
	register int	i;			/* Counter, loop index */
	BUF		*bp;			/* Read buffer */
	char		namebuf[SYS_NMLN];	/* System name */
	int		fl;			/* File length*/

	/* Check if *name is an available user area */
	if (! useracc ((char *) name, sizeof (struct utsname), 1)) {
		set_user_error (EFAULT);
		return -1;
	}
	/* Find the size of the version number */
	for (rcp = version, i = 0; *rcp != 0 && i < SYS_NMLN; i++, rcp++)
		/* DO NOTHING */ ;

	/* Write version number to user area */
	if (! kucopy (version, name->version, i))
		return -1;

	/* Find the size of the release number */
	for (rcp = release, i = 0 ; * rcp != 0 && i < SYS_NMLN ;
	     i ++, rcp ++)
		/* DO NOTHING */ ;

	/* Write release number to user area */
	if (! kucopy (release, name->release, i))
		return -1;

	/* Write "machine" to user area */
	if (! kucopy ("i386", name->machine, 4))
		return -1;

	/*
	 * We supposed that system name and nodename are in /etc/uucpname
	 * NIGEL: Set the global io segment to IOSYS so that ftoi () will use
	 * the right method of getting at the argument passed to it.
	 */

	{
		struct direct	dir;

		if (ftoi ("/etc/uucpname", 'r', IOSYS, NULL, & dir,
			  SELF->p_credp) != 0)
			return sys_unknown (name);
	}

	if ((fl = u.u_cdiri->i_size) == 0) {
		idetach (u.u_cdiri);
		return sys_unknown (name);
	}

	if (iaccess (u.u_cdiri, IPR, SELF->p_credp) == 0 ||
	    (bp = vread (u.u_cdiri, (daddr_t) 0)) == NULL) {
		idetach (u.u_cdiri);
		return -1;
	}

	/* namebuf should be not more than SYS_NMLN - 1 characters long */
	fl = fl > SYS_NMLN ? SYS_NMLN : fl;
	memcpy (namebuf, bp->b_vaddr, fl);

	brelease (bp);
	idetach (u.u_cdiri);

	if (fl == 1 && namebuf [0] == '\n')
		return sys_unknown (name);

	for (rcp = namebuf, i = 0 ; i < fl ; rcp ++) {
		i ++;
		if (* rcp == '\n') {
			* rcp = 0;
			break;
		}
	}
	namebuf[i - 1] = 0;

	/* Write system name to user area */
	if (! kucopy (namebuf, name->sysname, i))
		return -1;

	/* Write system name to user area */
	if (! kucopy (namebuf, name->nodename, i))
		return -1;
	return 0;
}


/* 
 * u_ustat - get file system statistics. (Name ustat in use for stat s.c.)
 */

int
u_ustat(dev, buf)
dev_t	dev;
struct ustat	*buf;
{
	MOUNT	*mp;

	/* Check if buf is an available user area. */
	/* B_READ | B_WRITE is not implemented yet. */
	if (! useracc ((char *) buf, sizeof (struct ustat), 1)) {
		set_user_error (EFAULT);
		return -1;
	}

	/* Take mount filesystem, check if dev is mounted device */
	for (mp = mountp ; mp != NULL ; mp = mp->m_next)
		if (mp->m_dev == dev)
			break;

	if (mp == NULL) {
		set_user_error (EINVAL);
		return -1;
	}

	/* Pickup information from superblock */
	/* Number of free blocks */
	if (! kucopy (& mp->m_super.s_tfree, & buf->f_tfree,
		     sizeof (mp->m_super.s_tfree)))
		return -1;

	/* Number of free inodes */
	if (! kucopy (& mp->m_super.s_tinode, & buf->f_tinode,
		      sizeof (mp->m_super.s_tinode)))
		return -1;

	/* File system name */
	if (! kucopy (mp->m_super.s_fname, buf->f_fname,  
		      sizeof (mp->m_super.s_fname)))
		return -1;

	/* File system pack name */
	if (! kucopy (mp->m_super.s_fpack, buf->f_fpack,  
		      sizeof (mp->m_super.s_fpack)))
		return -1;
	return 0;
}


/*
 * uname and ustat system calls.
 *
 * int uname(struct utsname *name)
 * Before lcall instruction 4(%esp) contains name, 8(%esp) contains
 * an unspecified value, and 12(%esp) contains the value 0.
 *
 * int ustat(int dev, struct ustat *buf)
 * Before lcall instruction 4(%esp) contains buf (REVERSE order of argument)
 * 8(%esp) contains dev, and 12(%esp) contains the value 2.
 */

int
uutssys (arg1, arg2, func)
caddr_t		arg1;
caddr_t		arg2;
int		func;
{
	switch (func) {	
	case 0: return uname ((struct utsname *) arg1);

	case 2: return u_ustat (arg2, arg1);

	default:
		set_user_error (EINVAL);
		return -1;
	}
}


/* Don't tell user process about last remaining 64k of RAM */
#define BRK_CUSHION	16

unsigned long
uulimit (cmd, newlimit)
int cmd;
unsigned long newlimit;
{
	int freeClicks;

	switch (cmd) {
	case UL_GETFSIZE:	/* Get max # of 512-byte blocks per file. */
		return u.u_bpfmax;
		break;

	case UL_SETFSIZE: /* Set max # of 512-byte blocks per file. */
		/* (only superuser may increase this) */
		if (newlimit <= u.u_bpfmax || super ()) {
			u.u_bpfmax = newlimit;
			return 0;
		}
		/* else super() will have set_user_error () to EPERM */
		break;

	case UL_GMEMLIM: /* Get max break value. */
		/* return (current brk value) + (amount of free space) */
		/* Don't report all free clicks - leave a cushion. */
		freeClicks = allocno () - BRK_CUSHION;
		if (freeClicks < 0)
			freeClicks = 0;
		return (unsigned long)
			(SELF->p_segl [SIPDATA].sr_base +
			 SELF->p_segl [SIPDATA].sr_segp->s_size + NBPC * freeClicks);

	case UL_GDESLIM:
		/* Return configured number of open files per process. */
		return NOFILE;
		break;

	default:
		set_user_error (EINVAL);
	}

	return -1UL;
}


/*
 *  Change the size of a file.
 */

int
uchsize (fd, size)
int fd;
long size;
{
	__fd_t	      *	fdp;
	INODE	      *	ip;

	if (size < 0) {
		set_user_error (EINVAL);
		return -1;
	}
	if ((fdp = fd_get (fd)) == NULL || (fdp->f_flag & IPW) == 0) {
		set_user_error (EBADF);
		return -1;
	}

	ip = fdp->f_ip;
	switch (ip->i_mode & IFMT) {
	case IFREG:
		if (size > (long) u.u_bpfmax * BSIZE) {
			set_user_error (EFBIG);
			return -1;
		}
		if (size == ip->i_size)
			break;
		ilock (ip, "uchsize () IFREG");
		if (size < ip->i_size)
			blclear (ip, __DIVIDE_ROUNDUP (ip->i_size, BSIZE));
		ip->i_size = size;
		imodcreat (ip);
		iunlock (ip);
		break;

	case IFPIPE:
		if (size > PIPE_MAX) {
			set_user_error (EFBIG);
			return -1;
		}

		ilock (ip, "uchsize () IFPIPE");

		if (! ip->i_par && ! ip->i_psr) {
			set_user_error (EPIPE);
			sendsig (SIGPIPE, SELF);
			iunlock (ip);
			return -1;
		}
		ip->i_pwx += (size - ip->i_pnc);
		if (size > ip->i_pnc) {
			if (ip->i_pwx >= PIPE_MAX)
				ip->i_pwx -= PIPE_MAX;
		} else if (size < ip->i_pnc) {
			if (ip->i_pwx < 0)
				ip->i_pwx += PIPE_MAX;
		}

		ip->i_pnc = size;
		imodcreat (ip);

		if (size > 0)
			pwake (ip, 2);	/* 2==IFWFW, see pipe.c	*/
		if (size < PIPE_MAX)
			pwake (ip, 1);	/* 1==IFWFR, see pipe.c	*/
		iunlock (ip);
		break;

	default:
		set_user_error (EBADF);
		return -1;
	}
	return 0;
}



/*
 * This is a copy of iucheck. The only one difference is that that allows
 * to remove a directory to a regular user.
 */

int
diucheck (dev, ino)
o_dev_t dev;
o_ino_t ino;
{
	if (inode_find (dev, ino, NULL) == NULL) {
		struct inode *ip;
		struct inode inode;

		ip = & inode;
		ip->i_dev = dev;
		ip->i_ino = ino;
		if (icopydm (ip) == 0)
			return 0;
	}
	return 1;
}


/*
 * Unlink the given directory.
 */

void
dunlink (np)
char *np;
{
	INODE	      *	ip;
	dev_t dev;
	IO		io;
	struct direct	dir;

	if (file_to_inode (np, 'u', 1, IOSYS, & io, & dir, SELF->p_credp) != 0)
		return;
 
	if (iaccess (u.u_pdiri, IPW, SELF->p_credp) == 0) {
		idetach (u.u_pdiri);
		return;
	}

	dev = u.u_pdiri->i_dev;
	if (diucheck (dev, u.u_cdirn) == 0) {
		idetach (u.u_pdiri);
		return;
	}

	idirent (0, & io, & dir);
	idetach (u.u_pdiri);

	if ((ip = iattach (dev, u.u_cdirn)) == NULL)
		return;

	if (ip->i_nlink > 0)
		-- ip->i_nlink;

	icreated (ip);	/* unlink - ctime */
	idetach (ip);
	return;
}


/*
 * Remove a directory.
 * path is a pointer to user area.
 */

void
removedir(path, iPathLen)
char	*path;	/* Directory name */
int	iPathLen;	/* Path length */
{
	char		*buf;
	char		*cpbuf,		/* internal file_name buffer */
			*cppath;	/* user file_name buffer */

	/*
	 * Allocate kernel buffer. We need extra space for '/', '.', '..' 
	 * and 0
	 */

	if ((buf = kalloc (iPathLen + 4)) == NULL) {
		SET_U_ERROR (ENOSPC, "rmdir: out of kernel space");
		return;
	}
	cpbuf = buf;
	cppath = path;

	/* Copy path to the kernel buffer. */
	while ((* cpbuf = getubd (cppath)) != 0) {
		cppath ++;
		cpbuf ++;
	}
	* cpbuf++ = '/';
	* cpbuf++ = '.';
	* cpbuf = 0;

	dunlink (buf);
	if (get_user_error ()) {
		kfree (buf);
		return;
	}

	* cpbuf ++ = '.';
	* cpbuf = 0;

	dunlink (buf);
	if (get_user_error ()) {
		/* We have to link '.' back here. */
		kfree (buf);
		return;
	}

	buf [iPathLen] = 0;

	dunlink (buf);
	if (get_user_error ()) {
		/* We have to link '.' and '..' back here. */
		kfree (buf);
		return;
	}
	kfree (buf);
	return;
}


/* 
 * Check if directory is empty.
 */

int
isdirempty (ip)
struct inode *ip;
{
	char	*cp;
	int		count;
	BUF		*bp;

	for (count = 0 ; count < ip->i_size ; count += BSIZE) {
		if ((bp = vread (ip, count)) == NULL) 
			break;

		for (cp = (char *) bp->b_vaddr ; 
		     cp < (char *) bp->b_vaddr + BSIZE ; cp += 16) {

			if (* cp == 0 && cp [1] == 0) 
				continue;
			if (cp [2] != '.')
				goto bad;
			if (cp [3] == 0)
				continue;
			if (cp [3] != '.' || cp [4] != 0)
				goto bad;
		}

		brelease (bp);
	}
	return 1;

bad:
	brelease (bp);
	return 0;
}


/*
 * Remove a directory.
 */

int
urmdir (path)
char * path;
{
	int		iPathLen;	/* Size of the string */
	extern int	strUserAcc ();
	struct direct	dir;
	
	/* Check if path points to a valid user buffer.*/
	if ((iPathLen = strUserAcc (path, 0)) < 0) {
		set_user_error (EFAULT);
		return -1;
	}

	if (ftoi (path, 'r', IOUSR, NULL, & dir, SELF->p_credp) != 0)
		return -1;

	/* Check if path is a directory */
	if ((u.u_cdiri->i_mode & IFMT) != IFDIR) {
		SET_U_ERROR (ENOTDIR, "rmdir: no such directory");
		idetach (u.u_cdiri);
		return -1;
	}

	/* We have to check if directory is empty */
	if (! isdirempty (u.u_cdiri)) {
		SET_U_ERROR (EEXIST, "rmdir: directory is not empty");
		idetach (u.u_cdiri);
		return -1;
	}
	idetach (u.u_cdiri);
	removedir (path, iPathLen);
	return 0;
}


/*
 * SysV compatible mkdir() system call.
 *
 *	Create a directory of the given "path" and "mode", if possible.
 *	Creating the directory is straight forward.  Trying to clean
 *	up in case we run out of inodes or freee blocks in the process 
 * 	is not trivial.
 *	This system call was implemented in very press time.
 * 	Vlad 6-04-92
 */

int
umkdir (path, mode)
char	*path;
int	mode;
{
	INODE 		*dmknod();		/* make directory node */
	INODE 		*pip;			/* parent inode pointer */
	char 		*cp_path, 
			*cpb_path, 
			*cp_parent;
	char		*bufpath, *bufparent, *mark;

	/* Check if path points to a valid user buffer.*/
	if (strUserAcc (path, 0) < 0) {
		set_user_error (EFAULT);
		return -1;
	}

	/*
	 * Create a local copies of "path" which we can use to build up
	 * the required directory links:
	 *	path/. -- bufdot
	 *	path/..-- bufdotdot
	 * Verify that the given path is not too long.
	 */

	cp_path = path;

	cpb_path = bufpath = kalloc ((PATH_MAX + 5) * sizeof (char));
	if (cpb_path == NULL) {
		set_user_error (EAGAIN);
		return -1;
	}

	cp_parent = bufparent = kalloc ((PATH_MAX + 5) * sizeof (char));
	if (cp_parent == NULL) {
		kfree (cpb_path);
		set_user_error (EAGAIN);
		return -1;
	}

	while ((* cpb_path = getubd (cp_path)) != 0) {
		* cp_parent ++ = * cpb_path;
		++ cp_path;
		if (++ cpb_path >= & bufpath [PATH_MAX - 3]) {
			SET_U_ERROR (ENOENT, "sys5: mkdir: path too long");
			kfree (bufpath);
			kfree (bufparent);
			return -1;
		}
	}

	if ((pip = dmknod (path, mode, IOUSR)) == NULL) {
		kfree (bufpath);
		kfree (bufparent);
		return -1;
	}

	/* 
	 * Temporarily change parent to the 'dot' directory
	 */

	mark = cp_parent;
	* cp_parent ++ = '/';
	* cp_parent ++ = '.';
	* cp_parent ++ = 0;
	do_link (bufpath, bufparent, IOSYS, 0);

	/*
	 * Get rid of dot entry.
	 */
	* (cp_parent = mark) = 0;

	if (get_user_error ()) {
		set_user_error (do_unlink (bufpath, IOSYS));
		kfree (bufparent);
		kfree (bufpath);
		return -1;
	}

	/*
	 * Now really find the parent
	 */

	while (-- cp_parent >= bufparent) {
		if (* cp_parent == '/') {
			* ++ cp_parent = 0;
			break;
		}
	}

	if (cp_parent < bufparent) {
		* ++ cp_parent = '.';
		* ++ cp_parent = 0;
	}

	/*
	 * We can hack into the pathname now.
	 */

	mark = cpb_path;
	* cpb_path ++ = '/';
	* cpb_path ++ = '.';
	* cpb_path ++ = '.';
	* cpb_path ++ = 0;


	do_link (bufparent, bufpath, IOSYS, 0);
	* (cpb_path = mark) = 0;

	if (get_user_error ()) {
		/* 
	 	 * Temporarily change parent to the 'dot' directory
	  	 */

		mark = cp_parent;
		*cp_parent++ = '/';
		*cp_parent++ = '.';
		*cp_parent++ = 0;

		set_user_error (do_unlink (bufparent, IOSYS));
		set_user_error (do_unlink (bufpath, IOSYS));
	}

	kfree (bufpath);
	kfree (bufparent);
	return 0;
}


/*
 * Create a directory.
 *
 * We cannot use original ulink because it makes the directories only
 * for superuser.
 */

INODE *
dmknod (np, mode, space)
char	*np;	/* Direcotory name */
int	mode;
int		space;
{
	INODE *ip;
	IO		io;
	struct direct	dir;

	/* If ftoi returns nonzero, get_user_error () has been set. */
	if (ftoi (np, 'c', space, & io, & dir, SELF->p_credp) != 0)
		return NULL;

	if (u.u_cdiri != NULL) {
		SET_U_ERROR (EEXIST, "dmknod: path already exists");
		idetach (u.u_cdiri);
		return NULL;
	}
	if ((ip = imake (mode | S_IFDIR, (o_dev_t) 0, & io, & dir,
			 SELF->p_credp)) != NULL)
		idetach (ip);

	return u.u_pdiri;	/* grab ptr to parent inode */
}


/*
 * Read `n' bytes from the directory `fd' using the buffer `bp'.
 */

int
dirio(fd, bp, n, offset)
int fd;
struct direct	*bp;
unsigned 	n;
off_t		*offset;
{
	__fd_t	      *	fdp;
	IO		io;

	/* Check file descriptor */
	if ((fdp = fd_get (fd)) == NULL ||
	    (fdp->f_flag & IPR) == 0 ||
	    (fdp->f_ip->i_mode & IFMT) != IFDIR) {
		set_user_error (EBADF);
		return 0;
	}

	ilock (fdp->f_ip, "dirio ()");

	io.io_seg = IOSYS;
	io.io_seek = fdp->f_seek;
	io.io.vbase = (caddr_t) bp;
	io.io_ioc  = n;
	io.io_flag = 0;

	if ((fdp->f_flag & IPNDLY) != 0)
		io.io_flag |= IONDLY;

	if ((fdp->f_flag & IPNONBLOCK) != 0)
		io.io_flag |= IONONBLOCK;

	iread (fdp->f_ip, & io);
	iaccessed (fdp->f_ip);		/* read - atime */

	n -= io.io_ioc;
	* offset = fdp->f_seek;
	fdp->f_seek += n;

	iunlock (fdp->f_ip);
	return n;
}


/*
 * Get directory entry in file system independent format.
 */

int
ugetdents(fd, bp, n)
int 		fd;	/* File descriptor to an open directory */
char 		*bp;	/* Buffer where entries should be read */
unsigned 	n;	/* Number of bytes to be read */
{
	struct direct	r_dir;	
	unsigned	bytes;	/* Number of bytes */
	struct dirent	sd;
	ino_t		inode;	/* Inode number */
	unsigned short	ofnm;	/* Offset to file name in dirent */
	char		*cw, 
			*cr;
	int		minbuf;	/* Minimum possible size of the bp */
	int		i, mod;
	int		entry;
	char		ends[3] = "";
	int		total = 0;

	cw = bp;

	ofnm = sizeof (sd.d_ino) + sizeof (sd.d_off) + sizeof (sd.d_reclen);

	/*
	 * Find minimum possible size of bp. It should be enough to contain
	 * the header of dirent, file name + 0, and be on a sizeof(long)
	 * boundary.
	 */

	entry = ofnm + DIRSIZ + 1;
	mod = entry % sizeof(long);
	minbuf = entry + (mod ? sizeof(long) - mod : 0); 

	/* Is user buffer available? */
	if (! useracc (bp, n, 1) || n < minbuf) {
		set_user_error (EFAULT);
		return -1;
	}

	while(n - (cw - bp) >= minbuf) {
		/*
		 * Read next entry from the directory. 
		 * inode == 0 for rm(ed) entries 
		 */

		do {
			if ((bytes = dirio (fd, & r_dir,
					    sizeof (struct direct), 
					    & sd.d_off)) == 0)
				return total;

			inode = r_dir.d_ino;
		} while (! inode);

		/* Find the size of file name */
		for (cr = r_dir.d_name, i = 0 ; * cr != 0 && i < DIRSIZ ; 
		     i ++, cr ++)
			/* DO NOTHING */ ;

		/* Copy file name */
		if (! kucopy (r_dir.d_name, cw + ofnm, i))
			return -1;

		/* Write 0 */
		putubd (cw + ofnm + i ++, 0);

		/* Round up to long boundary */
		if ((mod = (ofnm + i) % sizeof(long)) != 0)
			if (! kucopy (ends, cw + ofnm + i, sizeof
				      (long) - mod))
				return -1;

		sd.d_ino = r_dir.d_ino;
		sd.d_reclen = ofnm + i;		/* Size of directory entry */
		if (mod)
			sd.d_reclen += sizeof (long) - mod;

		if (! kucopy(& sd, cw, ofnm))
			return -1;

		total += sd.d_reclen;
		cw += sd.d_reclen;
	}
	return total;
}


/* 
 * Get file system information by file name.
 */

int
ustatfs(path, stfs, len, fstyp)
char		*path;	/* File name */
struct statfs	*stfs;	/* Pointer to a user structure */
int		len;	/* Size of the structure */
int		fstyp;	/* File system type */
{
	struct filsys	*statmount();	/* Get mp for mounted device */
	struct filsys	*statunmount();	/* Get mp for unmounted device */
	struct filsys	*sb;		/* Pointer to superblock */
	int		count = 0;	/* Number of copied bytes */
	short		systype = 1;	/* System type */
	long		bsize = BSIZE;	/* Block size */
	long		frsize = 0;	/* Fragment size */

	/* Check if stfs is an available user area. */
	if (! useracc ((char *) stfs, len, 1)) {
		SET_U_ERROR (EFAULT, "ustatfs 0");
		return -1;
	}

	/* Filesystem type is 1 for 512 bytes blocks. */
	count += sizeof (systype);
	if (count > len) {
		SET_U_ERROR (EFAULT, "ustatfs 1");
		return -1;
	}
	if (! kucopy (& systype, & stfs->f_fstyp, sizeof (systype)))
		return -1;
	
	/* Block size */
	count += sizeof (bsize);
	if (count > len) {
		SET_U_ERROR (EFAULT, "ustatfs 2");
		return -1;
	}
	if (! kucopy (& bsize, & stfs->f_bsize, sizeof (bsize)))
		return -1;

	/* Fragment size. */
	count += sizeof (int);
	if (count > len) {
		SET_U_ERROR (EFAULT, "ustatfs 3");
		return -1;
	}
	if (! kucopy (& frsize, & stfs->f_frsize, sizeof (frsize)))
		return -1;

	if (! fstyp) {
		if ((sb = statmount (-1, path)) == NULL)
			return -1;
		devinfo (sb, stfs, len, & count);
	} else {
		if ((sb = statunmount (-1, path)) == NULL)
			return -1;
		devinfo (sb, stfs, len, & count);
		kfree (sb);
	}
	return 0;
}


/*
 * statmount - get superblock for mounted file system.
 * fd - file descriptor or -1, path -  file name or NULL.
 */

struct filsys *
statmount(fd, path)
int	fd;
char	*path;
{	
	MOUNT	*mp;		/* Pointer to device */
	dev_t		device;		/* Mounted device */

	/* Find the device */
	if (path) {	/* Find ip by file name */
		struct direct	dir;

		if (ftoi (path, 'r', IOUSR, NULL, & dir, SELF->p_credp))
			return NULL;

		device = u.u_cdiri->i_dev;
		idetach (u.u_cdiri);
	} else {		/* Find ip by file descriptor */
		__fd_t	      *	fdp;

		if ((fdp = fd_get (fd)) == NULL || (fdp->f_flag & IPR) == 0) {
			SET_U_ERROR (EBADF, "statmount 1");
			return NULL;
		}
		device = fdp->f_ip->i_dev;
	}	

	/* Take mount filesystem, check if dev is mounted device */
	for (mp = mountp; mp != NULL; mp = mp->m_next)
		if (mp->m_dev == device)
			break;

	if (mp == NULL) {
		set_user_error (EINVAL);
		return NULL;
	}
	return & mp->m_super;
}


/*
 * devinfo() write system information to user area
 */

int
devinfo (sb, stfs, len, count)
struct filsys	*sb;	/* File name */
struct statfs	*stfs;	/* Pointer to a user structure */
int		len;	/* Size of the structure */
int		*count;
{
	long		inode;

	/* Total number of blocks */
	* count += sizeof (sb->s_fsize);
	if (* count > len)
		return -1;

	if (! kucopy (& sb->s_fsize, & stfs->f_blocks, sizeof (sb->s_fsize)))
		return -1;

	/* Count of free blocks */
	* count += sizeof (sb->s_tfree);
	if (* count > len)
		return -1;
	if (! kucopy (& sb->s_tfree, & stfs->f_bfree, sizeof (sb->s_tfree)))
		return -1;

	/* Total number of file inodes */
	* count += sizeof (inode);
	if (* count > len)
		return -1;

	inode = (long) (sb->s_isize - INODEI) * INOPB;
	if (! kucopy (& inode, & stfs->f_files, sizeof (inode)))
		return -1;

	/* Number of free inodes */
	* count += sizeof(inode);
	if (* count > len)
		return -1;
	inode = sb->s_tinode;
	if (! kucopy (& inode, & stfs->f_ffree, sizeof (inode)))
		return -1;

	/* Volume name */
	* count += sizeof (sb->s_fname);
	if (* count > len)
		return -1;
	if (! kucopy (sb->s_fname, stfs->f_fname, sizeof (sb->s_fpack)))
		return -1;

	/* Pack name */
	* count += sizeof(sb->s_fpack);
	if (* count > len)
		return -1;
	if (! kucopy (sb->s_fpack, stfs->f_fpack, sizeof (sb->s_fpack)))
		return -1;
	return 0;
}

/*
 * statunmount - get superblock for unmounted file system.
 * fd - file descriptor or -1, path -  file name or NULL.
 */

struct filsys *
statunmount(fd, path)
int	fd;	/* File descriptor */
char	*path;	/* File name */
{
	MOUNT	*mp;
	dev_t	rdev;
	int		mode;
	BUF		*bp;
	struct filsys	*sb;

	/* Find the device */
	if (path) {	/* Find ip by file name */
		struct direct	dir;

		if (ftoi (path, 'r', IOUSR, NULL, & dir, SELF->p_credp)) 
			return NULL;

		mode = u.u_cdiri->i_mode;
		rdev = u.u_cdiri->i_rdev;
		idetach (u.u_cdiri);
	} else {		/* Find ip by file descriptor */
		__fd_t		*fdp;

		if ((fdp = fd_get (fd)) == NULL || (fdp->f_flag & IPR) == 0) {
			set_user_error (EBADF);
			return NULL;
		}

		ilock (fdp->f_ip, "statunmount ()");

		mode = fdp->f_ip->i_mode;
		rdev = fdp->f_ip->i_rdev;

		iunlock (fdp->f_ip);
	}	

	/* Check for block special device */
	if ((mode & IFMT) != IFBLK) {
		set_user_error (ENOTBLK);
		return NULL;
	}

	/* Check if device is mounted device */
	for (mp = mountp; mp != NULL; mp = mp->m_next) {
		if (mp->m_dev == rdev) {
			set_user_error (EBUSY);
			return NULL;
		}
	}

	(void) dopen (rdev, IPR, DFBLK, NULL);
	if (get_user_error ()) 
		return NULL;
	
	/*
 	 * NIGEL: Modified for new dclose ().
	 */

	bp = bread (rdev, (daddr_t) SUPERI, BUF_SYNC);
	dclose (rdev, IPR, DFBLK, NULL);

	if (bp == NULL)
		return NULL;

	if ((sb = kalloc (sizeof (struct filsys))) == NULL)
		return NULL;

	memcpy (sb, bp->b_vaddr, sizeof (struct filsys));
	/*
	 * We must invalidate the buffer or the next try will
	 * use this buffer even if the disk has changed (e.g.,
	 * for a floppy disk).
	 */
	buf_invalidate(bp);

	brelease (bp);

	cansuper (sb);		/* canonicalize superblock */
	if (tstf (sb) == 0) {	/* check for consistency */
		kfree (sb);
		set_user_error (EINVAL);
		return NULL;
	}
	return sb;
}



/* 
 * Get file system information by file descriptor
 */

int
ufstatfs (fildes, stfs, len, fstyp)
int		fildes;	/* File descriptor */
struct statfs	*stfs;	/* Pointer to a user structure */
int		len;	/* Size of the structure */
int		fstyp;	/* File system type */
{	
	struct filsys	*statmount();	/* Get mp for mounted device */
	struct filsys	*statunmount();	/* Get mp for unmounted device */
	struct filsys	*sb;		/* Pointer to superblock */
	int		count = 0;	/* Number of copied bytes */
	short		systype = 1;	/* System type */
	long		bsize = BSIZE;	/* Block size */
	long		frsize = 0;	/* Fragment size */

	/* Check if stfs is an available user area. */
	if (! useracc ((char *) stfs, len)) {
		set_user_error (EFAULT);
		return -1;
	}

	/* Filesystem type is 1 for 512 bytes blocks. */
	count += sizeof (systype);
	if (count > len) {
		SET_U_ERROR (EFAULT, "ufstatfs 0");
		return -1;
	}
	if (! kucopy (& systype, & stfs->f_fstyp, sizeof (systype)))
		return -1;
	
	/* Block size */
	count += sizeof (bsize);
	if (count > len) {
		SET_U_ERROR (EFAULT, "ufstatfs 1");
		return -1;
	}
	if (! kucopy (& bsize, & stfs->f_bsize, sizeof (bsize)))
		return -1;

	/* Fragment size. */
	count += sizeof (int);
	if (count > len) {
		SET_U_ERROR (EFAULT, "ufstatfs 2");
		return -1;
	}
	if (! kucopy (& frsize, & stfs->f_frsize, sizeof (frsize)))
		return -1;

	if (! fstyp) {
		if ((sb = statmount (fildes, NULL)) == NULL)
			return -1;
		if (devinfo (sb, stfs, len, & count) != 0)
			return -1;
	} else {
		if ((sb = statunmount (fildes, NULL)) == NULL)
			return -1;
		if (devinfo (sb, stfs, len, & count) != 0) {
			kfree (sb);
			return -1;
		}
		kfree (sb);
	}
	return 0;
}


/*
 * Check superblock for consistency.
 */

int
tstf(fp)
struct filsys *fp;
{
	daddr_t *dp;
	ino_t *ip;
	ino_t maxinode;

	maxinode = (fp->s_isize - INODEI) * INOPB + 1;
	if (fp->s_isize >= fp->s_fsize)
		return 0;
	if (fp->s_tfree < fp->s_nfree ||
	    fp->s_tfree >= fp->s_fsize - fp->s_isize + 1)
		return 0;
	if (fp->s_tinode < fp->s_ninode || fp->s_tinode >= maxinode - 1)
		return 0;

	for (dp = fp->s_free ; dp < fp->s_free + fp->s_nfree ; dp += 1)
		if (* dp < fp->s_isize || * dp >= fp->s_fsize)
			return 0;

	for (ip = fp->s_inode ; ip < fp->s_inode + fp->s_ninode ; ip += 1)
		if (* ip < 1 || * ip > maxinode)
			return 0;
	return 1;
}

/* the following calls are not in the BCS */

int
uadmin()
{
	set_user_error (EINVAL);
	return -1;
}
