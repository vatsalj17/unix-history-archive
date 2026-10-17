#include "sys/param.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/pte.h"
#include "sys/vm.h"
#include "sys/buf.h"
#include "sys/proc.h"
#include "sys/conf.h"
#include "sys/label.h"
#include "sys/log.h"
#include "sys/pex.h"

/*
 * READ AND WRITE ARE NEAR DUPLICATES OF EACH OTHER TO SAVE
 * AS MUCH TIME AS POSSIBLE.
 */

/*
 * read system call
 */
read()
{
	struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap;

	uap = (struct a *)u.u_ap;
	if ((int)uap->count < 0) {
		u.u_error = EINVAL;
		return;
	}
	ip = geti(uap->fdes, FREAD|FHUNGUP, &fp);
	if(ip == NULL)
		return;
	if((ip->i_pex!=0 && ip->i_pex!=u.u_procp) || (ip->i_pexflag&PEX_IMPURE)) {
		u.u_error = ECONC;
		return;
	}
	*u.u_pofilep |= DIDREAD;
	u.u_base = (caddr_t)uap->cbuf;
	u.u_count = uap->count;
	u.u_segflg = SEGUDATA;
	if (setjmp(u.u_qsav)) {
		if (u.u_count == uap->count)
			u.u_error = EINTR;
	} else {
		int m = ip->i_mode & IFMT;
		if((m==IFREG||m==IFDIR) && ip->i_sptr==NULL) {
			plock(ip);
			u.u_offset = fp->f_offset;
			readi(ip);
			fp->f_offset = Lladd(fp->f_offset, uap->count-u.u_count);
			prele(ip);
		} else {
			u.u_offset = fp->f_offset;
			readi(ip);
			fp->f_offset = Lladd(fp->f_offset, uap->count-u.u_count);
		}
	}
	u.u_r.r_val1 = uap->count-u.u_count;
}

/*
 * write system call
 */
write()
{
	struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap;

	uap = (struct a *)u.u_ap;
	if ((int)uap->count < 0) {
		u.u_error = EINVAL;
		return;
	}
	ip = geti(uap->fdes, FWRITE|FHUNGUP, &fp);
	if(ip == NULL)
		return;
	if((ip->i_pex!=0 && ip->i_pex!=u.u_procp)
	    || (ip->i_pexflag&(ISLOGFILE|PEX_IMPURE))) {
		u.u_error = ECONC;
		return;
	}
	*u.u_pofilep |= DIDWRITE;
	u.u_base = (caddr_t)uap->cbuf;
	u.u_count = uap->count;
	u.u_segflg = SEGUDATA;
	if (setjmp(u.u_qsav)) {
		if (u.u_count == uap->count)
			u.u_error = EINTR;
	} else {
		if((ip->i_mode&IFMT)==IFREG && ip->i_sptr==0) {
			plock(ip);
			u.u_offset = fp->f_offset;
			writei(ip);
			fp->f_offset = Lladd(fp->f_offset, uap->count-u.u_count);
			prele(ip);
		} else {
			u.u_offset = fp->f_offset;
			writei(ip);
			fp->f_offset = Lladd(fp->f_offset, uap->count-u.u_count);
		}
	}
	u.u_r.r_val1 = uap->count-u.u_count;
}

/*
 * open system call
 */
open()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	rwmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	ip = namei(uap->fname, SEGUDATA, &nilargnamei, 1);
	if(ip == NULL)
		return;
	open1(ip, ++uap->rwmode, 0);
}

/*
 * creat system call
 */
creat()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	fmode;
	} *uap;
	struct argnamei nmarg;
	short mode;

	uap = (struct a *)u.u_ap;
	nmarg = nilargnamei;
	nmarg.flag = NI_CREAT;
	nmarg.un.mode = mode = uap->fmode & 07777 & ~u.u_cmask;
	ip = namei(uap->fname, SEGUDATA, &nmarg, 1);
	if(ip == NULL)
		return;
	open1(ip, FWRITE, (nmarg.un.mode == mode)? 1: 2);
}

/*
 * common code for open and creat.
 * Check permissions, allocate an open file structure,
 * and call the device open routine if any.
 */
open1(ip, mode, trf)
register struct inode *ip;
register mode;
{
	if(trf != 2) {
		if(ip->i_count>1 && (ip->i_mode&ICONC))
			concurrency(ip, mode);
		if(mode&FREAD)
			(void) access(ip, IREAD);
		if(mode&FWRITE) {
			(void) access(ip, IWRITE);
			if((ip->i_mode&IFMT) == IFDIR)
				u.u_error = EISDIR;
			else if((ICONC&~ip->i_mode) && (ip->i_mode&(ISUID|ISGID)))
				u.u_error = EPERM;
		}
	}
	if( (ip->i_pexflag&ISLOGFILE) && (trf==1 || (mode&FWRITE) ))
		u.u_error = EPERM;
	if(trf == 1 && ip->i_pex != NULL && ip->i_pex != u.u_procp)
		u.u_error = ECONC;
	if(u.u_error)
		goto out;
	if(trf == 1 && (ip->i_mode & ICCTYP) != IAPPEND) {
		if(ip->i_size > 0 && !labWchk(ip))
			goto out;
		(*fstypsw[ip->i_fstyp]->t_trunc)(ip);
	}
	prele(ip);

	mode &= FREAD|FWRITE;
	ip = openi(ip, mode);
	if(ip != NULL)
		u.u_r.r_val1 = f_open(-1, ip, mode);
	if(u.u_r.r_val1 == -1)
		goto out;
	return;

out:
	iput(ip);
}


/* Mkdir system call */
mkdir()
{
	register struct a {
		char *fname;
		int fmode;
	} *uap;
	struct argnamei nmarg;

	uap = (struct a *)u.u_ap;
	nmarg = nilargnamei;
	nmarg.flag = NI_MKDIR;
	nmarg.un.mode = (uap->fmode & 0777 & ~u.u_cmask) | IFDIR;
	(void)namei(uap->fname, SEGUDATA, &nmarg, 0);
}

/* Rmdir system call */
rmdir()
{	struct a {
		char *fname;
	};
	struct argnamei nmarg;

	nmarg = nilargnamei;
	nmarg.flag = NI_RMDIR;
	(void) namei(((struct a *)u.u_ap)->fname, SEGUDATA, &nmarg, 0);
}

/*
 * Select system call.
 */
int	nselcoll;
int	selwait;

select()
{
	register struct uap  {
		int	nfd;
		fd_set	*rp, *wp;
		int	timo;
	} *ap = (struct uap *)u.u_ap;
	fd_set rd, wr, readable, writeable;
	int nfds = 0;
	time_t t = time;
	int s, tsel, ncoll, rem;
	unsigned len;
	fd_set selscan();

	if (ap->nfd > NOFILE)
		ap->nfd = NOFILE;
	if (ap->nfd < 0) {
		u.u_error = EBADF;
		return;
	}

	/* read in only as many words as we need */
	len = ((ap->nfd+NBPW*NBBY-1)/(NBPW*NBBY))*NBPW;
	if (ap->rp && copyin((caddr_t)ap->rp, (caddr_t)&rd, len))
		return;
	if (ap->wp && copyin((caddr_t)ap->wp, (caddr_t)&wr, len))
		return;
retry:
	ncoll = nselcoll;
	u.u_procp->p_flag |= SSEL;
	if (ap->rp)
		readable = selscan(ap->nfd, &rd, &nfds, FREAD);
	if (ap->wp)
		writeable = selscan(ap->nfd, &wr, &nfds, FWRITE);
	if (u.u_error)
		goto done;
	if (nfds)
		goto done;
	rem = (ap->timo+999)/1000 - (time - t);
	if (ap->timo == 0 || rem <= 0)
		goto done;
	s = spl6();
	if ((u.u_procp->p_flag & SSEL) == 0 || nselcoll != ncoll) {
		u.u_procp->p_flag &= ~SSEL;
		splx(s);
		goto retry;
	}
	u.u_procp->p_flag &= ~SSEL;
	tsel = tsleep((caddr_t)&selwait, PZERO+1, rem);
	splx(s);
	switch (tsel) {

	case TS_OK:
		goto retry;

	case TS_SIG:
		u.u_error = EINTR;
		return;

	case TS_TIME:
		break;
	}
done:
	u.u_r.r_val1 = nfds;
	if (ap->rp)
		(void)copyout((caddr_t)&readable, (caddr_t)ap->rp, len);
	if (ap->wp)
		(void)copyout((caddr_t)&writeable, (caddr_t)ap->wp, len);
}

fd_set
selscan(nfd, fdsp, nfdp, flag)
int nfd;
fd_set *fdsp;
int *nfdp, flag;
{
	struct file *fp;
	struct inode *ip;
	unsigned int off, i, fd, able;
	unsigned long bits;
	fd_set	res;

	/* loop through all words of the set */
	for (off=0; off<FDWORDS; off++) {

		/* loop once for each bit on in a word of the set */
		res.fds_bits[off] = 0;
		bits = fdsp->fds_bits[off];
		while (i = ffs(bits)) {
			fd = off*(NBPW*NBBY)+i-1;
			if (fd >= nfd)
				return res;
			bits &= ~(1<<(i-1));
			fp = u.u_ofile[fd] ? (u.u_ofile[fd]->fr_file) : NULL;
			if (fp == NULL) {
				u.u_error = EBADF;
				return res;
			}
			ip = fp->f_inode;
			able = 1;
			if(ip->i_pex!=0 && ip->i_pex!=u.u_procp)
				able = 0;
			else if(ip->i_pexflag&PEX_IMPURE)
				;
			else if (ip->i_sptr)
				able = stselect(ip->i_sptr, flag, *nfdp);
			if (able) {
				res.fds_bits[off] |= (1<<(i-1));
				(*nfdp)++;
			}
		}
	}
	return res;
}

selwakeup(p, coll)
register struct proc *p;
int coll;
{
	int s;

	if (coll) {
		nselcoll++;
		wakeup((caddr_t)&selwait);
	}
	if (p) {
		s = spl6();
		if (p->p_wchan == (caddr_t)&selwait)
			setrun(p);
		else
			p->p_flag &= ~SSEL;
		splx(s);
	}
}

/*
 * close system call
 */
close()
{
	register struct file *fp;
	register struct a {
		int	fdes;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if(fp == NULL) {
		u.u_error = EBADF;
		return;
	}
	f_close(uap->fdes);
}

/*
 * [l]seek system calls
 */
seek()
{
	doseek(0,3);
}

lseek()
{
	doseek(1,3);
}

seeknoret()
{
	doseek(0,1);
}

tell()
{
	doseek(0,2);
}

/*
 * islong: dmr 64 bit seek pts
 * flag: secunix 2='read' 1='write' the seek ptr
 */
doseek(islong, flag)
{
	struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		off_t	off;
		int	sbase;
	} *uap1;
	register struct b {
		int	fdes;
		llong_t	off;
		int	sbase;
	} *uap2;
	llong_t off;
	int fdes, sbase;

	if (islong) {
		uap2 = (struct b *)u.u_ap;
		fdes = uap2->fdes;
		off = uap2->off;
		sbase = uap2->sbase;
	} else {
		uap1 = (struct a *)u.u_ap;
		fdes = uap1->fdes;
		sbase = uap1->sbase;
		off = ltoL(uap1->off);
		if (sbase!=0) {		/* signed offset when not w.r.t. base */
			if (uap1->off<0)
				off.hi = -1;
		}
	}
	ip = geti(fdes, FREAD|FWRITE, &fp);
	if(ip == NULL)
		return;
	if(ip->i_sptr) {
		u.u_error = ESPIPE;
		return;
	}
	if(loggable(LS))
		seclog(LOG_SEEK, flag, "12I", fdes, fp-file, fp->f_inode);
	if(flag & 1) {
		/*
		 * draconian: if file structure not shared with exclusive
		 * proc should be OK to seek
		 */
		if((ip->i_pex!=0 && ip->i_pex!=u.u_procp) || (ip->i_pexflag&PEX_IMPURE)) {
			u.u_error = ECONC;
			return;
		}
		if( (*u.u_pofilep&SAFETOREAD) || labSEEK(sbase, fp))
			;
		else
			return;

		if(sbase == 1)
			off = LLadd(fp->f_offset, off);
		else if(sbase == 2)
			off = Lladd(off, fp->f_inode->i_size);
		fp->f_offset = off;
	}
	if(flag & 2) {
		if((*u.u_pofilep&SAFETOREAD) || labRSchk(fp)) {
			u.u_r.r_val1 = Ltol(off);
			u.u_r.r_val2 = off.hi;
		}
	}
}


/*
 * link system call
 */
link()
{
	register struct inode *ip;
	register struct a {
		char	*target;
		char	*linkname;
	} *uap;
	struct argnamei nmarg;

	uap = (struct a *)u.u_ap;
	ip = namei(uap->target, SEGUDATA, &nilargnamei, 0);   /* well, this routine is doomed anyhow */
	if(ip == NULL)
		return;
	if((ip->i_mode&IFMT)==IFDIR && !suser(0))
		goto out1;
	ip->i_nlink++;
	ip->i_flag |= ICHG;
	iupdat(ip, &time, &time, 1);
	prele(ip);
	nmarg = nilargnamei;
	nmarg.flag = NI_LINK;
	nmarg.un.il = ip;
	u.u_labcheck = 0;
	(void) namei(uap->linkname, SEGUDATA, &nmarg, 0);
	if(u.u_error) {
		ip->i_nlink--;
		ip->i_flag |= ICHG;
	}
out1:
	iput(ip);
}

/*
 * mknod system call
 */
mknod()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	fmode;
		int	dev;
	} *uap;
	struct argnamei nmarg;

	uap = (struct a *)u.u_ap;
	if(suser(0)) {
		nmarg = nilargnamei;
		nmarg.flag = NI_NXCREAT;
		nmarg.un.mode = uap->fmode & ~u.u_cmask;
		ip = namei(uap->fname, SEGUDATA, &nmarg, 0);
	}
	if(u.u_error)
		return;
	if (uap->dev) {
		/*
		 * Want to be able to use this to make badblock
		 * inodes, so don't truncate the dev number.
		 */
		ip->i_un.i_rdev = uap->dev;
		ip->i_flag |= IACC|IUPD|ICHG;
	}
	iput(ip);
}

/*
 * access system call
 */
saccess()
{
	register svuid, svgid;
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	fmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	svuid = u.u_uid;
	svgid = u.u_gid;
	u.u_uid = u.u_ruid;
	u.u_gid = u.u_rgid;
	ip = namei(uap->fname, SEGUDATA, &nilargnamei, 1);
	if (ip != NULL) {
		if (uap->fmode&(IREAD>>6))
			(void) access(ip, IREAD);
		if (uap->fmode&(IWRITE>>6))
			(void) access(ip, IWRITE);
		if (uap->fmode&(IEXEC>>6))
			(void) access(ip, IEXEC);
		iput(ip);
	}
	u.u_uid = svuid;
	u.u_gid = svgid;
}
