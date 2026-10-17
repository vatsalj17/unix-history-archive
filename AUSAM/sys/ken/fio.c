#
/*
 */

#include "../defines.h"
#include "../param.h"
#ifdef	AUSAML
#include	"../lnode.h"
#endif	AUSAML
#include "../systm.h"
#include "../file.h"
#include "../user.h"
#include "../filsys.h"
#include "../conf.h"
#include "../inode.h"
#include "../reg.h"

/*
 * Convert a user supplied
 * file descriptor into a pointer
 * to a file structure.
 * Only task is to check range
 * of the descriptor.
 */
getf(f)
{
	register *fp, rf;

	rf = f;
	if( 0 <= rf && rf < NOFILE )			/* fix025 */
	{						/* fix025 */
		fp = u.u_ofile[rf];			/* fix025 */
		if(fp != NULL)				/* fix025 */
			return(fp);			/* fix025 */
	}						/* fix025 */
	u.u_error = EBADF;
	return(NULL);
}
/*
 * Internal form of close.
 * Decrement reference count on
 * file structure and call closei
 * on last closef.
 * Also make sure the pipe protocol
 * does not constipate.
 */
#ifdef	LOCKING
/*
 * If this process has a file locked, unlock it.
 */
#endif	LOCKING
closef(fp)
register struct file *fp;
{
	register *ip;

	ip = fp->f_inode;
#ifdef	LOCKING
	if(u.u_locki == ip)
		unlock();
#endif	LOCKING
	if(fp->f_flag&FPIPE)
	{
		ip->i_mode =& ~(IREAD|IWRITE);
		wakeup(ip+1);
		wakeup(ip+2);
	}
#ifdef	LARGE_FILE_REFERENCES
	if(fp->f_count == 1)
#else
	if(fp->f_count <= 1)
#endif	LARGE_FILE_REFERENCES
		closei(ip, fp->f_flag&FWRITE);
	fp->f_count--;
}

/*
 * Decrement reference count on an
 * inode due to the removal of a
 * referencing file structure.
 * On the last closei, switchout
 * to the close entry point of special
 * device handler.
 * Note that the handler gets called
 * on every open and only on the last
 * close.
 */
closei(ip, rw)
register int *ip;
{
	register struct mount *mp;		/* fix030 */
	register dev, maj;

	plock(ip);				/* fix025 */
	dev = ip->i_addr[0];
	maj = ip->i_addr[0].d_major;
	if(ip->i_count <= 1)
	{
		prele(ip);			/* fix025 (ammended) */
		switch(ip->i_mode&IFMT)
		{
	
	    case IFCHR:
			(*cdevsw[maj].d_close)(dev, rw);
			break;
	
	    case IFBLK:
			/* fix030
			 * If device is mounted its still open.
			 */
			for(mp = mount; mp < &mount[NMOUNT]; mp++)
			{
				if(mp->m_bufp != NULL && mp->m_dev == dev)
					goto out;
			}
			bflush(dev);			/* fix030 */
			(*bdevsw[maj].d_close)(dev, rw);
			binval(dev);			/* fix030 */
		}
	}
out:
	iput(ip);
}

/*
 * openi called to allow handler
 * of special files to initialize and
 * validate before actual IO.
 * Called on all sorts of opens
 * and also on mount.
 */
openi(ip, rw)
int *ip;
{
	register *rip;
	register dev, maj;

	rip = ip;
	dev = rip->i_addr[0];
	maj = rip->i_addr[0].d_major & 0377;	/* fix025 */
	switch(rip->i_mode&IFMT) {

	case IFCHR:
		if(maj >= nchrdev)
			goto bad;
		(*cdevsw[maj].d_open)(dev, rw);
		break;

	case IFBLK:
		if(maj >= nblkdev)
			goto bad;
		(*bdevsw[maj].d_open)(dev, rw);
	}
	return;

bad:
	u.u_error = ENXIO;
}

/*
 * Check mode permission on inode pointer.
 * Mode is READ, WRITE or EXEC.
 * In the case of WRITE, the
 * read-only status of the file
 * system is checked.
 * Also in WRITE, prototype text
 * segments cannot be written.
 * The mode is shifted to select
 * the owner/group/other fields.
 * The super user is granted all
 * permissions except for EXEC where
 * at least one of the EXEC bits must
 * be on.
 */
access(aip, mode)
int *aip;
{
	register *ip, m;

	ip = aip;
	m = mode;
	if(m == IWRITE) {
		if(getfs(ip->i_dev)->s_ronly != 0) {
			u.u_error = EROFS;
			return(1);
		}
		if(ip->i_flag & ITEXT) {
			u.u_error = ETXTBSY;
			return(1);
		}
	}
	if(u.u_uid == 0) {
		if(m == IEXEC && (ip->i_mode & 
			(IFDIR | IEXEC | (IEXEC>>3) | (IEXEC>>6))) == 0) /* fix013 */
				goto bad;
		return(0);
	}
#ifndef	AUSAM16
	if(u.u_uid != ip->i_uid) {
#ifdef	GROUP_ACCESS
		m =>> 3;
		if(u.u_gid != ip->i_gid)
			m =>> 3;
#endif
#ifndef	GROUP_ACCESS
		m =>> 6;
#endif
#endif	AUSAM16
#ifdef	AUSAM16
	if(u.u_uid != guid(ip)) {
		m =>> 6;
#endif	AUSAM16
	}
	if((ip->i_mode&m) != 0)
		return(0);

bad:
	u.u_error = EACCES;
	return(1);
}

/*
 * Look up a pathname and test if
 * the resultant inode is owned by the
 * current user.
 * If not, try for super-user.
 * If permission is granted,
 * return inode pointer.
 */
owner()
{
	register struct inode *ip;
	extern uchar();

	if ((ip = namei(uchar, 0)) == NULL)
		return(NULL);
#ifndef	AUSAM16
	if(u.u_uid == ip->i_uid)
#endif	AUSAM16
#ifdef	AUSAM16
	if(u.u_uid == guid(ip))
#endif AUSAM16
		return(ip);
	if (suser())
		return(ip);
	iput(ip);
	return(NULL);
}

/*
 * Test if the current user is the
 * super user.
 */
suser()
{

	if(u.u_uid == 0)
		return(1);
	u.u_error = EPERM;
	return(0);
}

/*
 * Allocate a user file descriptor.
 */
ufalloc()
{
	register i;

	for (i=0; i<NOFILE; i++)
		if (u.u_ofile[i] == NULL) {
			u.u_ar0[R0] = i;
			return(i);
		}
	u.u_error = EMFILE;
	return(-1);
}

/*
 * Allocate a user file descriptor
 * and a file structure.
 * Initialize the descriptor
 * to point at the file structure.
 *
 * no file -- if there are no available
 * 	file structures.
 */
falloc()
{
	register struct file *fp;
	register i;

	if ((i = ufalloc()) < 0)
		return(NULL);
	for (fp = &file[0]; fp < &file[NFILE]; fp++)
		if (fp->f_count==0) {
			u.u_ofile[i] = fp;
			fp->f_count++;
			fp->f_offset = 0;	/* fix000 */
			return(fp);
		}
	printf("no file\n");
	u.u_error = ENFILE;
	return(NULL);
}
