#include "sys/param.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/ino.h"
#include "sys/label.h"
#include "sys/log.h"

/*
 * sys5.c: Security system calls.
 */

/*
 * Copy label in and convert to core form.
 */
struct jlabel *
labelin(from)
struct label *from;
{
	struct label dlab;
	register struct jlabel *j;

	if(copyin((caddr_t)from, (caddr_t)&dlab, sizeof(struct label)) < 0) {
		u.u_error = EFAULT;
		return 0;
	}
	if(dlab.lb_flag == L_UNDEF) {
		u.u_error = EINVAL;
		return 0;
	}
	j = labCP(labelno);
	labdtoj(&dlab, j);
	return j;
}

/*
 * Convert label to disc form and copy out.
 */
static
labelout(from, to)
struct jlabel *from;
struct label *to;
{
	struct label dlab;

	labjtod(from, &dlab);
	if(copyout((caddr_t)&dlab, (caddr_t)to, sizeof(struct label)) < 0)
		u.u_error = EFAULT;
}

/*
 * fgetflab system call.
 */
fgetflab()
{
	register struct a {
		int	fdes;
		struct label *sb;
	} *uap;
	register struct inode *ip;

	uap = (struct a *)u.u_ap;
	ip = geti(uap->fdes, FREAD|FWRITE, (struct file **)NULL);
	if(ip == NULL)
		return;
	plock(ip);
	iupdat(ip, &time, &time, 0);
	labelout(ip->i_lab, uap->sb);
	prele(ip);
}

/*
 * get5state for testing
 */
get5state()
{
	register struct a {
		int	fdes;
		struct label *sb;
	} *uap;
	struct fildes *fr;
	register struct inode *ip;
	struct label s5[5];

	uap = (struct a *)u.u_ap;
	ip = geti(uap->fdes, FREAD|FWRITE, (struct file **)NULL);
	if(ip == NULL)
		return;
	fr = u.u_ofile[uap->fdes];
	
	plock(ip);
	
	labjtod(u.u_procp->p_lab,	&s5[0]);
	labjtod(u.u_procp->p_ceil,	&s5[1]);
	labjtod(fr->fr_file->f_lab,	&s5[2]);
	labjtod(ip->i_lab,		&s5[3]);
	labjtod(ip->i_ceil,		&s5[4]);

	prele(ip);

	if(copyout((caddr_t)s5, (caddr_t)uap->sb, 5*sizeof(struct label)) < 0)
		u.u_error = EFAULT;
}



/*
 * getflab system call.
 */
getflab()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		struct label *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	ip = namei(uap->fname, SEGUDATA, &nilargnamei, 1);
	if(ip == NULL)
		return;
	iupdat(ip, &time, &time, 0);
	labelout(ip->i_lab, uap->sb);
	iput(ip);
}

/*
 * fsetflab and setflab system calls.
 */

static Setflab(ip, sb)
struct inode *ip;
struct label *sb;
{
	register struct jlabel *ilp = ip->i_lab;
	register struct jlabel *lab;	/* free with impunity */
	register struct jlabel *plp = u.u_procp->p_lab;
	register struct jlabel *clp = u.u_procp->p_ceil;
	struct proc *itoproc();
	int x;
#define X(f) {x=__LINE__; f}	/* THE DEVIL MADE ME DO IT!!! */
	
	if( accowner(ip) == 0)
		return;
	if(itoproc(ip) != NULL) {	/* horrible! */
		u.u_error = ELAB;
		return;
	}
	lab = labelin(sb);
	if(u.u_error) {
		if(lab)
			jfree(lab);
		return;
	}
	if(jl_fix(ilp) == F_CONST) {	/* /dev/null ? */
		X(goto bad;)
	}
	if(jl_fix(lab) == F_CONST)
		X(goto bad;)
	if(jl_fix(lab) == F_RIGID && !ip->i_sptr)
		X(goto bad;)

	if(jl_fix(ilp) == F_RIGID) { /* F(ip) must be 1 on externals */
		jl_setfix(lab, F_RIGID);
	}

	if(jl_t(plp) & T_SETPRIV) {
		if(loggable(LX))
			seclog(LOG_SPRIV, 110, "CI", ip);
	}
	else if(jl_fix(lab) != jl_fix(ilp) &&
	     u.u_uid != ip->i_uid && u.u_uid != 0) { /* uid stuff redundant */
		jfree(lab);
		u.u_error = EPERM;
		return;
	}
	else if(jl_t(ilp) || jl_u(ilp))
		X(goto bad;)
	else if(jl_t(lab) ||  jl_u(lab))
		X(goto bad;)

	if(jl_flag(lab) == L_YES)
		X(goto bad;)
	else if(jl_flag(lab) == L_NO) {
		if((jl_t(plp) & T_EXTERN)) {
			if(loggable(LX))
				seclog(LOG_EXTERN, 111, "CI", ip);
		}
		else if(!labLE(ilp,u.u_procp->p_ceil))
			X(goto bad;)
	}
	else if(jl_flag(ilp) == L_NO && (jl_t(plp) & T_EXTERN)) {
		if(loggable(LX))
			seclog(LOG_EXTERN, 112, "CI", ip);
	}
	else if(!labLE(ilp, lab))
		X(goto bad;)
	else if(jl_t(plp) & T_NOCHK) {
		if(loggable(LX))
			seclog(LOG_NOCHK, 113, "CI", ip);
	}
	else if(labLE(plp, lab) && labLE(lab, clp))
		;
	else
		X(goto bad;)

	if(jl_fix(ilp) == F_LOOSE)
		;
	else if(jl_fix(ilp) == F_RIGID && (jl_t(plp) & T_EXTERN)) {
		if(loggable(LX))
			seclog(LOG_EXTERN, 114, "CI", ip);
	}
	else if(jl_fix(ilp) == F_FROZEN && u.u_uid == ip->i_uid)
		;
	else
		X(goto bad;)

	labMOVX(lab, ilp);
	labCHF(ip);
	if(loggable(LL))
		seclog(LOG_FLAB, 115, "Ij", ip, ilp);
	return;
bad:
	/* printf("[x=%d]\n", x); */
	if(loggable(LL))
		seclog(LOG_FLAB, 116, "2Ijj", x, ip, ilp, lab);
	(void)jfree(lab);
	u.u_error = ELAB;
	return;
}

fsetflab()
{
	register struct inode *ip;
	register struct a {
		int	fd;
		struct label *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	ip = geti(uap->fd, FREAD|FWRITE, (struct file **)NULL);
	if(ip == NULL)
		return;
	Setflab(ip, uap->sb);
}

setflab()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		struct label *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	if ((ip = namei(uap->fname, SEGUDATA, &nilargnamei, 1)) == NULL)
		return;
	Setflab(ip, uap->sb);
	iput(ip);
}

/*
 * getplab system call
 */
getplab()
{
	register struct a {
		struct label *lp;
		struct label *cp;
	} *uap;

	uap = (struct a *)u.u_ap;
	if(uap->cp) {
		if(labuachk(u.u_procp->p_ceillab))
			labelout(u.u_procp->p_ceil, uap->cp);
		else
			labelout(labelno, uap->cp);
	}
	if(uap->lp)
		labelout(u.u_procp->p_lab, uap->lp);
}

/*
 * setplab system call
 */
setplab()
{
	register struct a {
		struct label *lp;
		struct label *cp;
	} *uap;
	register struct proc *p = u.u_procp;
	register struct jlabel *plab, *clab;

	uap = (struct a *)u.u_ap;
	plab = uap->lp? labelin(uap->lp): labCPX(p->p_lab);
	clab = uap->cp? labelin(uap->cp): labCP(p->p_ceil);
	if(u.u_error)
		goto out;
	if( jl_t(plab) & ~ jl_t(p->p_lab)) {
		if(loggable(LX))
			seclog(LOG_SETLIC, 117, "C1", jl_t(plab));
		goto bad;
	}
	if(jl_u(plab) & ~ jl_u(p->p_lab)) { 
		if(jl_t(p->p_lab) & T_SETLIC) {
			if(loggable(LX))
				seclog(LOG_SETLIC, 118, "C1", jl_u(plab));
		} else {
			if(loggable(LX))
				seclog(LOG_SETLIC, 119, "C1", jl_u(plab));
			goto bad;
		}
	}
	if(jl_flag(plab) != L_BITS || jl_flag(clab) != L_BITS)
		goto bad;
	if(jl_fix(plab) > F_FROZEN || jl_fix(clab) > F_FROZEN)
		goto bad;
	if(!labLE(plab, clab))
		goto bad;
	if(labLE(p->p_lab, plab) )
		;
	else if (jl_t(p->p_lab) & T_SETLIC) {
		if(loggable(LX))
			seclog(LOG_SETLIC, 120, "C");
	} else
		goto bad;
	if(labLE(clab, p->p_ceil) )
		goto ok;
	else if (jl_t(p->p_lab) & T_SETLIC) {
		if(loggable(LX))
			seclog(LOG_SETLIC, 121, "C");
		goto ok;
	}
bad:
	u.u_error = ELAB;
out:
	(void)jfree(plab);
	(void)jfree(clab);
	return;
ok:
	psafeclear(p,SAFETOREAD);	/* if ceil or nochk change */
	if(uap->cp)
		if(jl_t(p->p_lab) & T_SETLIC)
			labCPMOV(labelbot, p->p_ceillab);
		else
			labCPMOV(p->p_lab, p->p_ceillab);
	labMOVX(plab, p->p_lab);
	labMOV(clab, p->p_ceil);
	labCHP();
	if(loggable(LL|LP))
		seclog(LOG_PLAB, 122, "jj", p->p_lab, p->p_ceil);
}

/*
 * Unsafe system call.
 */

unsafe()
{
	register struct uap  {
		int nfd;
		fd_set	*rp, *wp;
	} *ap = (struct uap *)u.u_ap;
	register int i;
	register struct fildes *fr;
	fd_set or, ow, nr, nw;
	int len = (ap->nfd+NBPW*NBBY-1)/(NBPW*NBBY)*NBPW;
	int nfds = 0;

	if(jl_t(u.u_procp->p_lab) & T_NOCHK) {
		if(loggable(LX))
			seclog(LOG_NOCHK, 123, "C");
		if (ap->rp && copyin((caddr_t)ap->rp, (caddr_t)&nr, len))
			return;
		if (ap->wp && copyin((caddr_t)ap->wp, (caddr_t)&nw, len))
			return;
	}
	FD_ZERO(or);
	FD_ZERO(ow);
	for(i=0; i<ap->nfd; i++) {
		int count = 0;
		fr = u.u_ofile[i];
		if(fr == NULL) continue;
		if(fr->fr_pofile & SAFETOREAD) {
			FD_SET(i, or);
			count = 1;
		}
		if(fr->fr_pofile & SAFETOWRITE) {
			FD_SET(i, ow);
			count = 1;
		}
		nfds += count;
	}
	if (ap->rp && copyout((caddr_t)&or, (caddr_t)ap->rp, len))
		return;
	if (ap->wp && copyout((caddr_t)&ow, (caddr_t)ap->wp, len))
		return;
	if(jl_t(u.u_procp->p_lab) & T_NOCHK) {
		if(loggable(LX))
			seclog(LOG_NOCHK, 124, "C");
		for(i=0; i<ap->nfd; i++) {
			fr = u.u_ofile[i];
			if(fr == NULL) continue;
			if(ap->rp && FD_ISSET(i, nr))
				fr->fr_pofile &= ~SAFETOREAD;
			if(ap->wp && FD_ISSET(i, nw))
				fr->fr_pofile &= ~SAFETOWRITE;
		}
	}
	u.u_r.r_val1 = nfds;
}


/*
 * nochk - try to make a file decriptor immune to security checking
 * default: use T_NOCHK if you have it.
 */
nochk() {
	register struct a {
		unsigned int	fdes;
		int	flag;
	} *uap = (struct a *)u.u_ap;
	register struct fildes *fr;
	register int x;
	
	if(uap->fdes > NOFILE || (fr = u.u_ofile[uap->fdes]) == NULL) {
		u.u_error = EBADF;
		return;
	}
	x = fr->fr_pofile;
	u.u_r.r_val1 = (x & T_NOCHK)? 1: 0;
	if(uap->flag == 0)
		x &= ~(T_NOCHK|SAFETOREAD|SAFETOWRITE);
	else 
		x |= T_NOCHK;
	fr->fr_pofile = x;
}

/*
 * labmount: report file system ceiling
 * labmount(fd, lp);
 */
labmount()
{
	register struct inode *ip;
	register struct a {
		int	fdes;
		struct label *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	if(ip = geti(uap->fdes, FREAD|FWRITE, (struct file **)NULL)) {
		labelout(ip->i_ceil ? ip->i_ceil : labelyes, uap->sb);
	}
}
