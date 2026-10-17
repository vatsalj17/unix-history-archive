#include "sys/param.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/conf.h"
#include "sys/label.h"
#include "sys/log.h"
#include "sys/pex.h"

#ifndef isstream
#define isstream(ip)	((ip)->i_sptr!=0)
#endif
struct inode * otherend();
struct proc * itoproc();

int uzero =	T_EXTERN|T_SETLIC|T_NOCHK|T_UAREA;	/* max file lic. */
int tzero =	0;				/* min file capab. */

/* tzero0: cheat.  see newcap(), iexpand(), insecure() */
int tzero0 =	0;

int labIcnt;
int labWcnt;
int labRcnt;
int labchpcnt;
int labchscnt;
int labchfcnt;
int labwschkcnt;
int labrschkcnt;
int labwchkcnt;
int labrchkcnt;
int labseekcnt;

newcap(ip,p)
struct inode *ip;
struct jlabel *p;
{
	struct jlabel *f = ip->i_lab;
	struct jlabel *c = ip->i_ceil;
	int t, u;

	t = jl_t(f);
	u = jl_u(f);
	if(c != NULL && jl_flag(c) != L_YES) {
		t &= jl_t(c);
		u &= jl_u(c);
	}
	t |= tzero;
	u &= uzero;
	u |= jl_u(p);

	return tzero0 | ( t & u );
}

/*
 * initialize labels on remote network inodes: totally frozen at bot.
 */
labnetinit(ip)
struct inode *ip;
{
	struct jlabel *j;

	labCPMOV(ip->i_ceil ? ip->i_ceil : labelno, ip->i_lab); /* was labelbot */
	j = ip->i_lab;
	jl_setfix(j, F_CONST);
}

/*
 * general purpose label inequality checker. 
 * the var array tells what the initial values of the variables are
 * the le array tells what inequalities of form Xleft <= Xright must hold.
 */
struct var {
	struct jlabel *val;
	struct jlabel *orig;
	int mask;
	int chdate;		/* when the variable was last updated */
};
struct le {
	int left;
	int right;
	int date;		/* when the inequality last verified */
};
int cxerr;			/* for debugging */

critchk(nvars, nle, mask, varvec, levec)
struct le *levec;
struct var *varvec;
{
	register int t, oldt;
	struct le *cp, *letop = levec+nle;
	register struct var *rp, *lp, *topvar = varvec+nvars;
	register struct jlabel *rv, *lv;

	for(cp=levec; cp<letop; cp++)
		cp->date = -2;
	t = 1;
	for(rp=varvec; rp<topvar; rp++) {
		rp->mask = t;	/* could be set at compile time */
		rp->orig = 0;
		rp->chdate = -1;
		t += t;
	}

	oldt = 0;
	cp = levec;
	for(t=0;  t < oldt+nle; t++, cp++) {
		if(cp >= letop) cp = levec;

		lp = varvec+cp->left;
		if(cp->date >= lp->chdate)
			continue;
		rp = varvec+cp->right;
		rv = rp->val;
		lv = lp->val;
		if(labLE(lv, rv))
			;
		else if(jl_fix(rv) != F_LOOSE || jl_flag(rv) == L_NO) {
			cxerr = cp->right;
			goto bad;
		}
		else if(mask & rp->mask) {
			if(rp->orig == 0)
				rv = rp->val = labCP(rp->orig = rv);
			labMOV(labMAX(lv, rv), rv);
			oldt = rp->chdate = t;
		} else {
			cxerr = -cp->right;
			goto bad;
		}
		cp->date = t;
	}

	mask = 0;
	for(rp=varvec; rp<topvar; rp++) {
		if(rp->orig != 0) {
			mask |= rp->mask;
			labMOV(rp->val, rp->orig);
		}
	}
	return mask;
bad:
	for(rp=varvec; rp<topvar; rp++)
		if(rp->orig != 0)
			jfree(rp->val);
	return -1;
}

#define P	0
#define	CP	1
#define	F	2
#define	CF	3
#define S	4
#define	SX	2		/* seek ptrs when files not present */
#define U	2

#define LEQ(x,y)	{x,y}
#define varset(x,y) cv[x].val=y
#define VARS(x)	struct var cv[x]
#define NLE(x)	sizeof(x)/sizeof(x[0])
#define cx(lelist, mask)  critchk(NLE(cv),NLE(lelist),mask,cv,lelist)

/*
 * general label check for read-like calls on files
 */
struct le critR[] = {
	LEQ (F,CF),
	LEQ (F,P),
	LEQ (F,CP),
};

labRchk(ip)
register struct inode *ip;
{
	register struct jlabel *plp = u.u_procp->p_lab;
	int x;
	VARS(4);


labrchkcnt++;
	if(jl_t(plp) & T_NOCHK) {
		if(loggable(LX))
			seclog(LOG_NOCHK, 70, "CI", ip);
		return 1;
	}
	varset(F, ip->i_lab);
	varset(CF, ip->i_ceil);
	varset(CP, u.u_procp->p_ceil);
	varset(P, plp);

	x = cx(critR, 1<<P);

	if(x == -1) goto bad;
	else if(x&(1<<P)) labCHP();
	
	return 1;
bad:
	/* uprintf("Rchk %s.%d\n", u.u_comm, ip->i_number); */
	u.u_error = ELAB;
	return 0;
}

/*
 * general label check for write-like calls on files
 */

struct le critW[] = {
	LEQ (P, CF),
	LEQ (F, CP),
	LEQ (P, F),
};

labWchk(ip)
register struct inode *ip;
{
	register struct jlabel *ilp = ip->i_lab;
	register struct jlabel *plp = u.u_procp->p_lab;
	struct proc *p = itoproc(ip);
	int x;
	VARS(4);

labwchkcnt++;
	if(jl_t(ilp) != 0 || jl_u(ilp) != 0)
		goto bad;
	if(jl_t(plp) & T_NOCHK) {
		if(loggable(LX))
			seclog(LOG_NOCHK, 71, "CI", ip);
		return 1;
	}
	if(p && !labLE(plp, p->p_ceil))
		goto bad;
	varset(F, ilp);
	varset(CF, ip->i_ceil);
	varset(P, plp);
	varset(CP, u.u_procp->p_ceil);

	x = cx(critW, 1<<F);
	if(x == -1)
		goto bad;
	if(x & (1<<F))
		labCHF(ip);
	return 1;
bad:
	/* uprintf("Wchk %s.%d\n", u.u_comm, ip->i_number); */
	u.u_error = ELAB;
	return 0;
}
/*
 * Ditto, for seek ptrs.
 */
struct le critRS[] = {
	LEQ (SX, P),
	LEQ (SX, CP),
};

labRSchk(fp)
register struct file *fp;
{
	register struct inode *ip = fp->f_inode;
	register struct jlabel *slp = fp->f_lab;
	register struct jlabel *plp = u.u_procp->p_lab;
	VARS(3);
	int x;

labrschkcnt++;
	if(ip == 0)
		return 1;
	if(T_NOCHK & jl_t(plp)) {
		if(loggable(LX))
			seclog(LOG_NOCHK, 72, "CI", ip);
		return 1;
	}
	varset(P, plp);
	varset(CP, u.u_procp->p_lab);
	varset(SX, slp);

	x = cx(critRS, 1<<P);
	if(x == -1)
		goto bad;
	if(x & (1<<P))
		labCHP();
	return 1;
bad:
	/* uprintf("RSchk %d\n", ip->i_number); */
	u.u_error = ELAB;
	return 0;
}

/*
 * high level check for seek pointer write
 */
struct le critWS[] = {
	LEQ (SX, CP),
	LEQ (P, SX),
};

labSEEK(b, fp)
register struct file *fp;
{
	register struct jlabel *plp = u.u_procp->p_lab;
	register int x;
	register struct jlabel *slp;
	register struct inode *ip = fp->f_inode;
	VARS(3);

labseekcnt++;
	if(ip == 0)
		return 1;
	if( jl_t(plp) & T_NOCHK ) {
		if(loggable(LX))
			seclog(LOG_NOCHK, 73, "CI", ip);
		return 1;
	}

	if(b == 0)
		slp = labCP(labelbot);
	else if(b == 1)
		slp = labCP(fp->f_lab);
	else if(b == 2)
		slp = labCP(ip->i_lab);
	else
		panic("labSEEK");
	
	varset(SX, slp);
	varset(P, plp);
	varset(CP, u.u_procp->p_ceil);

	x = cx(critWS, 1<<SX);
	if(x == -1) { 
		jfree(slp);
		/* uprintf("WSchk %d\n", ip->i_number); */
		u.u_error = ELAB;
		return 0;
	}
	if(!labEQ(slp, fp->f_lab)) {
		labMOV(slp, fp->f_lab);
		labCHS(fp);
	} else
		jfree(slp);
	return 1;
}

/* check for removability of file or directory
*/
struct le critRM[] = {
	LEQ (F, CP),
	LEQ (F, CF)
};
labrmchk(ip, isblind)
struct inode *ip;
{
	register struct jlabel *ilp = ip->i_lab;
	register struct jlabel *plp = u.u_procp->p_lab;
	int x;
	VARS(4);

	if(jl_t(ilp) != 0 || jl_u(ilp) != 0)
		goto bad;
	if(isblind && (u.u_uid != ip->i_uid)) {
		u.u_error = EPERM;
		return 0;
	}
	if(jl_t(plp) & T_NOCHK) {
		if(loggable(LX))
			seclog(LOG_NOCHK, 74, "CI", ip);
		return 1;
	}
	varset(F, ilp);
	varset(CF, ip->i_ceil);
	varset(P, plp);
	varset(CP, u.u_procp->p_ceil);
	x = cx(critRM, 0);
	if(x != -1)
		return 1;
bad:
	/* uprintf("rmchk %s.%d\n", u.u_comm, ip->i_number); */
	u.u_error = ELAB;
	return 0;
}

/* check for getting labeled item from uarea 
*/
struct le critUA[] = {
	LEQ (U, P),
	LEQ (P, CP)
};

labuachk(ulp)
struct jlabel *ulp;
{
	struct jlabel *plp = u.u_procp->p_lab;
	int x;
	VARS(3);

	if(jl_t(plp) & T_NOCHK) {
		if(loggable(LX))
			seclog(LOG_NOCHK, 81, "C");
		return 1;
	}
	varset(P, plp);
	varset(CP, u.u_procp->p_ceil);
	varset(U, ulp);
	x = cx(critUA, 1<<P);
	if(x == (1<<P))
		labCHP();
	if(x != -1)
		return 1;
	u.u_error = ELAB;
	return 0;
}

/*
 * Announce change of file label.
 * Special treatment for process files and pipe ends.
 */
labCHF(ip)
register struct inode *ip;
{
	register struct inode *jp;
	register struct fildes *fr;
	register struct proc *p;

labchfcnt++;
	ip->i_flag |= ICHG;
	iupdat(ip, &time, &time, 1);  /* tell disk before next crash */ 
	for(fr=ip->i_fr; fr; fr=fr->fr_ni) {
		fr->fr_pofile &= ~(SAFETOREAD|SAFETOWRITE);
		psignal(fr->fr_proc, SIGLAB);
	}
	if(ip->i_lab->jl_refcnt == 1) return;
	if(p = itoproc(ip)) {
		psafeclear(p, SAFETOWRITE);
		psignal(p, SIGLAB);
	} else
	if(jp = otherend(ip)) {
		jp->i_flag |= ICHG;
		for(fr=jp->i_fr; fr; fr=fr->fr_ni) {
			fr->fr_pofile &= ~(SAFETOREAD|SAFETOWRITE);
			psignal(fr->fr_proc, SIGLAB);
		}
	}
	if(loggable(LA))
		seclog(LOG_FLAB, 75, "Ij", ip, ip->i_lab);
}

labCHS(fp)
register struct file *fp;
{
	register struct fildes *fr;
	register struct proc *p;

labchscnt++;
	for(fr=fp->f_fref; fr; fr=fr->fr_nf) {
		p = fr->fr_proc;
		fr->fr_pofile &= ~(SAFETOREAD|SAFETOWRITE);
		psignal(p, SIGLAB);
	}
	if(loggable(LA))
		seclog(LOG_FLAB, 76, "2j", fp->f_fref-fildes, fp->f_lab);
}

labCHP()
{
	register struct inode *ip;
	register struct fildes *fr;
	register struct proc *p = u.u_procp;

labchpcnt++;
	psafeclear(p, SAFETOWRITE);
	psignal(p, SIGLAB);

	if(ip = p->p_trace) {
		ip->i_flag |= ICHG;
		for(fr=ip->i_fr; fr; fr=fr->fr_ni) {
			fr->fr_pofile &= ~(SAFETOREAD|SAFETOWRITE);
			psignal(fr->fr_proc, SIGLAB);
		}
	}
	if(loggable(LA))
		seclog(LOG_PLAB, 78, "jj", p->p_lab, p->p_ceil);
}


psafeclear(p, safebits)
register struct proc *p;
register safebits;
{
	register struct fildes *fr, *fnext;
	
	for(fr=p->p_ofile; fr; fr=fnext) {
		fnext = fr->fr_np;
		fr->fr_pofile &= ~safebits;
	}
}





/*
 * file descriptor read (read(), readdir())
 * honor per-fd T_NOCHK treatment
 */
struct le critREAD[] = {
	LEQ (F, CF),
	LEQ (F, S),
	LEQ (F, P),
	LEQ (F, CP),
	LEQ (S, P),
	LEQ (S, CP),
	LEQ (P, S),
};
labREAD()
{
	register struct fildes *fr = 0;
	register struct proc *p;
	register struct inode *f;
	struct file *s;
	int x;
	VARS(5);



labRcnt++;
	if(u.u_labcheck & RCHD)
		fr = u.u_ofile[u.u_checkfd];
	if(fr == 0)
		return 1;
	if(fr->fr_pofile & SAFETOREAD)
		return 1;
	p = fr->fr_proc;
	s = fr->fr_file;
	if(s == NULL) panic("rsnull");
	f = s->f_inode;

	if(jl_t(p->p_lab) & fr->fr_pofile & T_NOCHK) {
		if(loggable(LX))
			seclog(LOG_NOCHK, 79, "CI", f);
		goto ok;
	}
	if(isstream(f)) {
		(void)jfree(s->f_lab);
		s->f_lab = labDUP(p->p_lab);
	}
	varset(P, p->p_lab);
	varset(CP, p->p_ceil);
	varset(F, f->i_lab);
	varset(CF, f->i_ceil);
	varset(S, s->f_lab);

	x = cx(critREAD, (1<<P)|(1<<S));
	
	if(x == -1) goto bad;
	else if(x > 0) {
		if(x & (1<<P)) {
			fr->fr_pofile |= DIDRAISE;
			labCHP();
		}
		if(x & (1<<S)) labCHS(s);
	}
ok:
	fr->fr_pofile |= SAFETOREAD;
	return 1;
bad:
	/* uprintf("%s.READ %d.%d\n", u.u_comm, d, f->i_number); */
	u.u_error = ELAB;
	return 0;
}

/*
 * file descriptor write
 * honor per-fd T_NOCHK treatment
 */
struct le critWRITE[] = {
	LEQ (P, CF),
	LEQ (F, CP),
	LEQ (S, F),
	LEQ (S, CP),
	LEQ (S, CF),
	LEQ (P, S),
	LEQ (P, F),
};

labWRITE()
{
	register struct fildes *fr = 0;
	register struct proc *p, *q;
	register struct inode *f;
	struct file *s;
	VARS(5);
	int x;

labWcnt++;
	if(u.u_labcheck & WCHD)
		fr = u.u_ofile[u.u_checkfd];
	if(fr == 0)
		return 1;
	if(fr->fr_pofile & SAFETOWRITE)
		return 1;
	p = fr->fr_proc;
	s = fr->fr_file;
	if(s == NULL) panic("wsnull");
	f = s->f_inode;
	if(jl_t(f->i_lab) != 0 || jl_u(f->i_lab) != 0) {
		cxerr = 300;
		goto bad;
	}
	if(jl_t(p->p_lab) & fr->fr_pofile & T_NOCHK) {
		if(loggable(LX))
			seclog(LOG_NOCHK, 80, "CI", f);
		goto ok;
	}

	if(isstream(f)) {
		(void)jfree(s->f_lab);
		s->f_lab = labDUP(f->i_lab);
	}
	q = itoproc(f);
	if(q && (!labLE(p->p_lab,q->p_ceil) ||
		 !labLE(s->f_lab,q->p_ceil)))
		goto bad;
	varset(P, p->p_lab);
	varset(CP, p->p_ceil);
	varset(F, f->i_lab);
	varset(CF, f->i_ceil);
	varset(S, s->f_lab);

	x = cx(critWRITE, (1<<F)|(1<<S));
	
	if(x == -1) goto bad;
	else if(x > 0) {
		if(x & (1<<F)) {
			labCHF(f);
			fr->fr_pofile |= DIDRAISE;
		}
		if(x & (1<<S)) labCHS(s);
	}
ok:
	fr->fr_pofile |= SAFETOWRITE;
	return 1;
bad:
	u.u_error = ELAB;
/* printf("%s.WRITE %d.%d\n", u.u_comm,d,cxerr); */
	psignal(p, SIGPIPE);
	return 0;
}

#include "sys/dkio.h"
#include "sys/filio.h"
#include "sys/nbio.h"
#include "sys/pioctl.h"
#include "sys/stream.h"
#include "sys/ttyio.h"
#include "sys/udaioc.h"

labIOCTL(cmd, ip)
struct inode *ip;
{
labIcnt++;
	switch(cmd) {
	 
	/*
	 * known to be innocuous (save PIOCKILL)
	 */
	case FIOACCEPT: case FIOCLEX: case FIONCLEX: case FIOREJECT:
	case FIOPX: case FIONPX: case FIOQX:
	case PIOCOPENT: case PIOCREXEC: case PIOCSEXEC: case PIOCSMASK: 
	case PIOCSTOP: case PIOCWSTOP: case PIOCNICE: case PIOCKILL:
	case PIOCRUN:
	case TIOCEXCL: case TIOCGPGRP: case TIOCNXCL: case TIOCSPGRP:

		return 1;
	/*
	 * known to be read-like
	 */
	case FIOGSRC:
	case FIOLOOKLD: case FIONREAD: case FIORCVFD:
	case PIOCGETPR:
	case TIOCGDEV: case TIOCGETC: case TIOCGETP:
	case UIOCHAR: case UIORRCT:

		return labRchk(ip);
	/*
	 * known to be write-like
	 */
	case FIOSSRC:
	case FIOINSLD: case FIOPOPLD: case FIOPUSHLD: case FIOSNDFD:
	case TIOCFLUSH: case TIOCSBRK: case TIOCSDEV: case TIOCSETC:
	case TIOCSETP:
	case UIOREPL: case UIOWRCT:
		return labWchk(ip);
	/*
	 * Otherwise, assume the worst
	 */
	default:
		return labWchk(ip) && labRchk(ip);
	}
}
