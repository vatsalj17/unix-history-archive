#
#include "../defines.h"
#include "../param.h"
#ifdef	AUSAML
#include	"../lnode.h"
#endif	AUSAML
#include "../systm.h"
#include "../file.h"
#include "../user.h"
#include "../reg.h"
#include "../inode.h"
#include "../proc.h"

/*
 * read system call
 */
read()
{
	rdwr(FREAD);
}

/*
 * write system call
 */
write()
{
	rdwr(FWRITE);
}

/*
 * common code for read and write calls:
 * check permissions, set base, count, and offset,
 * and switch out to readi, writei, or pipe code.
 */
rdwr(mode)
{
	register *fp, *ip, m;			/* fix025 */

	m = mode;
	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	if((fp->f_flag&m) == 0) {
		u.u_error = EBADF;
		return;
	}
	u.u_base = u.u_arg[0];
	u.u_count = u.u_arg[1];
	u.u_segflg = SEG_USD;
	if(fp->f_flag&FPIPE) {
		if(m==FREAD)
			readp(fp); else
			writep(fp);
	} else {
		ip = fp->f_inode;				/* fix025 */
		u.u_offset = fp->f_offset; /* fix000 */
		if( (ip->i_mode & (IFCHR&IFBLK)) == 0 )		/* fix025 */
			plock(ip);				/* fix025 */
		if(m==FREAD)
			readi(ip); else				/* fix025 */
			writei(ip);				/* fix025 */
		if( (ip->i_mode & (IFCHR&IFBLK)) == 0 )		/* fix025 */
			prele(ip);				/* fix025 */
		fp->f_offset =+ u.u_arg[1].unsignd - u.u_count.unsignd; /* fix000 */
	}
	u.u_ar0[R0] = u.u_arg[1]-u.u_count;
}

/*
 * open system call
 */
open()
{
	register *ip;
	extern uchar;

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	u.u_arg[1]++;
	open1(ip, u.u_arg[1], 0);
}

/*
 * creat system call
 */
creat()
{
	register *ip,*lp;
	extern uchar;

#ifdef	AUSAML
	if( (lp = u.u_procp->p_lnode) && (lp->l_flags & DLIMIT) )
	{
		/* he can't write so don't let create */
		u.u_error = EDISKLIM;
		return;
	}
#endif	AUSAML
	ip = namei(&uchar, 1);
	if(ip == NULL)
	{
		if(u.u_error)
			return;
		ip = maknode(u.u_arg[1]&07777&(~ISVTX));
		if (ip==NULL)
			return;
		open1(ip, FWRITE, 2);
	}
	else
	{
		open1(ip, FWRITE, 1);
	}
}

/*
 * common code for open and creat.
 * Check permissions, allocate an open file structure,
 * and call the device open routine if any.
 */
open1(ip, mode, trf)
int *ip;
{
	register struct file *fp;
	register *rip, m;
	int i;

	rip = ip;
	m = mode;
	if(trf != 2) {
		if(m&FREAD)
			access(rip, IREAD);
		if(m&FWRITE) {
			access(rip, IWRITE);
			if((rip->i_mode&IFMT) == IFDIR)
				u.u_error = EISDIR;
		}
	}
	if(u.u_error)
		goto out;
	if(trf == 1)				/* fix025 */
		itrunc(rip);
	prele(rip);
	if ((fp = falloc()) == NULL)
		goto out;
	fp->f_flag = m&(FREAD|FWRITE);
	fp->f_inode = rip;
	i = u.u_ar0[R0];
	openi(rip, m&FWRITE);
	if(u.u_error == 0)
#ifndef	LOCKING
		return;
#else	LOCKING
	{
		if(ip->i_mode&ILPROTOCOL
		 && ip->i_mode&IAUTOLOCK
		 && (ip->i_mode&IFMT) != IFDIR)
		{
			lock(rip, (m&FWRITE) ? WLOCK : RLOCK);
			if(u.u_error)
				panic("lock?");
		}
		return;
	}
#endif	LOCKING
	u.u_ofile[i] = NULL;
	fp->f_count--;

out:
	iput(rip);
}

/*
 * close system call
 */
close()
{
	register *fp;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	u.u_ofile[u.u_ar0[R0]] = NULL;
	closef(fp);
}

/*
 * seek system call
 */
seek()
{
	long n; /* fix000 */
	register *fp, t;

	fp = getf(u.u_ar0[R0]);
	if(fp == NULL)
		return;
	if(fp->f_flag&FPIPE) {
		u.u_error = ESPIPE;
		return;
	}
	t = u.u_arg[1];
	if(t > 2) {
		n.loint = u.u_arg[0]<<9;	/* fix000 */
		n.hiint = u.u_arg[0]>>7;	/* fix000 */
		if(t == 3)
			n.hiint =& 0777;	/* fix000 */
	} else {
		if( t==0 )			/* fix000 */
			n = u.u_arg[0].unsignd;	/* fix000 */
		else	n = u.u_arg[0];		/* fix000 */
	}
	switch(t) {

	case 1:
	case 4:
		n =+ fp->f_offset;		/* fix000 */
		break;

	default:
		n.hiint =+ fp->f_inode->i_size0&0377;	/* fix000 */
		n =+ fp->f_inode->i_size1.unsignd;	/* fix000 */

	case 0:
	case 3:
		;
	}
	if(n.hiint & ~0777)				/* fix025 */
		u.u_error = EFBIG;			/* fix025 */
	else						/* fix025 */
	{
		fp->f_offset = n; /* fix000 */
	}
}

/*
 * tell system call				   fix034
 */
tell()						/* fix034 */
{						/* fix034 */
	register *fp;				/* fix034 */
						/* fix034 */
	fp = getf(u.u_ar0[R0]);			/* fix034 */
	if(fp == NULL)				/* fix034 */
		return;				/* fix034 */
	u.u_ar0[R0] = fp->f_offset.hiint;	/* fix034 */
	u.u_ar0[R1] = fp->f_offset.loint;	/* fix034 */
}						/* fix034 */

/*
 * link system call
 */
link()
{
	register *ip, *xp;
	extern uchar;

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	if(ip->i_nlink >= 127) {
		u.u_error = EMLINK;
		goto out;
	}
	if((ip->i_mode&IFMT)==IFDIR && !suser())
		goto out;
	/*
	 * unlock to avoid possibly hanging the namei.
	 * Sadly, this means races. (Suppose someone	fix025
	 * deletes the file in the meantime ?)		fix025
	 * Nor can it be locked again later because	fix025
	 * then there will be deadly embraces.		fix025
	 */
	prele(ip);				/* fix025 */
	u.u_dirp = u.u_arg[1];
	xp = namei(&uchar, 1);
	if(xp != NULL) {
		u.u_error = EEXIST;
		iput(xp);
	}
	if(u.u_error)
		goto out;
	if(u.u_pdir->i_dev != ip->i_dev) {
		iput(u.u_pdir);
		u.u_error = EXDEV;
		goto out;
	}
	wdir(ip);
	ip->i_nlink++;
	ip->i_flag =| IUPD;

out:
	iput(ip);
}

/*
 * mknod system call
 */
mknod()
{
	register *ip;
	extern uchar;

	if(suser()) {
		ip = namei(&uchar, 1);
		if(ip != NULL) {
			u.u_error = EEXIST;
			goto out;
		}
	}
	if(u.u_error)
		return;
	ip = maknode(u.u_arg[1]);
	if (ip==NULL)
		return;
	ip->i_addr[0] = u.u_arg[2];

out:
	iput(ip);
}




/*
 * sleep system call
 * not to be confused with the sleep internal routine.
 */
sslep()
{
#ifdef	NEW_SLEEP

	register unsigned *p;

	p = &u.u_procp->p_stl;
	*p = u.u_ar0[R0].unsignd;	/* sleep for 'r0' seconds */
	while( *p )
		sleep(p,PSLEP);		/* sleep till time up */
#endif

#ifndef	NEW_SLEEP

	long  d;						/* fix000 */

	spl7();
	d = time + u.u_ar0[R0].unsignd;				/* fix000 */
	while( d > time ){					/* fix000 */
		if( tout <= time || tout > d )  tout = d;	/* fix000 */
		sleep(&tout, PSLEP);				/* fix000 */
	}
	spl0();
#endif
}
#ifdef	ACCESS

/*
 * access system call
 *
 *	sys access ; name ; mode
 *	access( name , mode )
 *
 *	char *name;
 *	int mode
 *
 *	mode specifies type of access(------RWX) to be checked for
 *	u.u_error is set by errors including permission violations !!
 */
saccess()
{
	extern uchar;
	register svuid;
#ifdef	GROUP_ACCESS
	register svgid;
#endif	GROUP_ACCESS
	register *ip;

	svuid = u.u_uid;
	u.u_uid = u.u_ruid;
#ifdef	GROUP_ACCESS
	svgid = u.u_gid;
	u.u_gid = u.u_rgid;
#endif	GROUP_ACCESS
	ip = namei(&uchar, 0);
	if (ip != NULL) {
		if (u.u_arg[1]&(IREAD>>6))
			access(ip, IREAD);
		if (u.u_arg[1]&(IWRITE>>6))
			access(ip, IWRITE);
		if (u.u_arg[1]&(IEXEC>>6))
			access(ip, IEXEC);
		iput(ip);
	}
	u.u_uid = svuid;
#ifdef	GROUP_ACCESS
	u.u_gid = svgid;
#endif	GROUP_ACCESS
}
#endif	ACCESS
