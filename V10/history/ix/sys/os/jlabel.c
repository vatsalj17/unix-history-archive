/*
 * Administer shared labels.
 * To save space the actual bits of labels are not kept in
 * in-core inodes, but instead in a separate table, slabel.
 * Slabel contains only one instance of each distinct label.
 */

#include "sys/param.h"
#include "sys/user.h"
#include "sys/label.h"
#include "sys/log.h"

static struct jlabel *jfreelist;
static struct slabel *sfreelist;
static struct jlabel clearj;
int slabdebt = 2, slabhigh;

struct jlabel *labelyes;
struct jlabel *labelno;
struct jlabel *labelnox;
struct jlabel *labelbot;
struct jlabel *labeltop;

/*
 * initialize the shared label table
 */

static struct slabel *slaballoc();

static struct jlabel *jinit(flag, fix, p)
unsigned char *p;
{
	struct jlabel *j = jalloc();
	jl_setflag(j,flag);
	jl_setfix(j,fix);
	j->jl_slabp = /* p==0? 0: */ slaballoc(p);
	return j;
}

labinit()
{
	register i;
	unsigned char temp[LABSIZ];

	clearj.jl_refcnt = 1;

	for(i=0; i<jlabelcnt-1; i++)
		jlabel[i].jl_free = &jlabel[i+1];
	jlabel[i].jl_free = 0;
	jfreelist = jlabel;

	for(i=0; i<slabelcnt-1; i++)
		slabel[i].sl_next = &slabel[i+1];
	sfreelist = slabel;

	bzero((caddr_t)temp, sizeof temp);
	labelbot = jinit(L_BITS, F_RIGID, temp);
	labelyes = jinit(L_YES, F_CONST, temp);
	labelno = jinit(L_NO, F_RIGID, temp);
	labelnox = jinit(L_NO, F_LOOSE, temp);
	for(i=0; i<LABSIZ; i++)
		temp[i] = ~0;
	labeltop = jinit(L_BITS, F_LOOSE, temp);
}

int slaba, slabf;

slabfree(p)
register struct slabel *p;
{
	register struct slabel *sp;
	register int i;

	if(p==0) return;
	slabf++;
	if(--p->sl_refcnt > 0)
		return;
	slabdebt--;
	i = p->sl_hash % slabixcnt;
	sp = slabix[i];
	if(p == sp)
		slabix[i] = p->sl_next;
	else {
		for( ; ; sp=sp->sl_next) {
			if(sp == 0) panic("slabfree");
			if(p == sp->sl_next) break;
		}
		sp->sl_next = p->sl_next;
	}
	p->sl_next = sfreelist;
	sfreelist = p; 
}
/*
 * Find place in shared label table for the bits in disc label at p.
 * Coalesce identical bit strings.
 */
static struct slabel *
slaballoc(p)
unsigned char *p;
{
	register unsigned short *ps, *ts;
	register unsigned long *pl, *sl, *tl;
	register struct slabel *sp;
	unsigned long hash = 0;
	unsigned short fullhash, parthash;

		/* the test on LABSIZE is optimized away by -O,
		   fails to test alignment, and is intended to
  		   guard labLE and labMAX also */
	if(p==0 || LABSIZ % sizeof(*pl)!=0)
		panic("slaballoc1");
	slaba++;
	ps = (unsigned short*)p;
	ts = (unsigned short*)&p[LABSIZ];
	while(ps < ts)
		hash += *ps++;
	fullhash = hash + (hash>>16);
	parthash = fullhash % slabixcnt;

	for(sp = slabix[parthash]; sp; sp=sp->sl_next) {
		if(sp->sl_hash != fullhash)
			continue;
		pl = (unsigned long*)p;
		tl = (unsigned long*)&p[LABSIZ];
		sl = (unsigned long*)sp->sl_bits;
		while(pl < tl)
			if(*pl++ != *sl++)
				goto contin;
		sp->sl_refcnt++;
		return sp;
contin:		;
	}

	sp = sfreelist;
	if(!sp || sp->sl_refcnt)
		panic("slaballoc2");
	if(++slabdebt > slabhigh) {
		slabhigh = slabdebt;
		if(slabhigh % 50 == 0)
			printf("slabhigh = %d\n", slabhigh);
	}	
	sfreelist = sfreelist->sl_next;
	sp->sl_refcnt++;
	sp->sl_hash = fullhash;
	sp->sl_next = slabix[parthash];
	slabix[parthash] = sp;
	bcopy((caddr_t)p, (caddr_t)sp->sl_bits, LABSIZ);
	return sp;
}

int jdebt = 0, jhigh = 0;

struct jlabel *
jalloc()
{
	register struct jlabel *j = jfreelist;

	if(j) {
		jfreelist = j->jl_free;
		*j = clearj;
	} else {
		u.u_error = ENLAB;
		panic("jalloc");
	}
	if(++jdebt > jhigh) {
		jhigh = jdebt;
		if(jhigh % 100 == 0)
			printf("jh=%d\n", jhigh);
	}
	return j;
}

/*
 * NEVER do jfree(labelbot), etc.
 */
struct jlabel *
jfree(j)
register struct jlabel *j;
{
	if(j == 0)
		return 0;
	if(--j->jl_refcnt > 0)
		return 0;
	slabfree(j->jl_slabp);
	j->jl_free = jfreelist;
	jfreelist = j;
	jdebt--;
	return 0;
}

/* 
 * Convert disc label at d to joint label
 * (do not copy log bits)
 */
labdtoj(d, j)
register struct label *d;
register struct jlabel *j;
{
	struct jlabel jj;
	if(j==0 || d==0) {
		panic("labdtoj");
	}
	slabfree(j->jl_slabp);
	j->jl_slabp = slaballoc(d->lb_bits);
	jj.jl_Priv = d->lb_priv;
	j->jl_priv = jj.jl_priv;
	if(j->jl_slabp->sl_refcnt == 1)
		seclog(LOG_SLAB, 40, "J", j);
}
/*
 * Convert joint label at j to disc label at d.
 * (provide zero log bits)
 */
labjtod(j,d)
register struct jlabel *j;
register struct label *d;
{
	register struct slabel *sp;

	d->lb_priv = j->jl_Priv;
	d->lb_priv.lp_junk = 0;
	if((sp = j->jl_slabp) == NULL)
		panic("labjtod");
	else
		bcopy((caddr_t)sp->sl_bits, (caddr_t)d->lb_bits, sizeof(d->lb_bits));
}

/* Provide another pointer to an existing joint label */

struct jlabel *
labDUP(j)
register struct jlabel *j;
{
	j->jl_refcnt++;
	return j;
}

/*
 * labMOV(q, p)	give p the value q had, keep p's privs and fixity, kill q. 
 *	(Other sharers of q are still alive.)
 * labCP(p)	return a new joint label with same value as p, 
 *	no privs, unfrozen
 * labCPMOV(q, p) combined effect: q stays alive, p has no privs,
 *	is not frozen
 *
 * labMOVX(q, p), labCPX(p), labCPMOVX(q, p), same but copy privs
 *	and fixity
 */
int labcp, labm, labcpm;
int labcpx, labmx, labcpmx;

labMOV(q, p)
register struct jlabel *q, *p;
{
	labm++;
	q->jl_slabp->sl_refcnt++;
	slabfree(p->jl_slabp);
	p->jl_slabp = q->jl_slabp;
	jl_setflag(p, jl_flag(q));
	(void)jfree(q);
}

/*
 * same, but copy privs.
 */
labMOVX(q, p)
register struct jlabel *q, *p;
{
	labmx++;
	q->jl_slabp->sl_refcnt++;
	slabfree(p->jl_slabp);
	p->jl_slabp = q->jl_slabp;
	p->jl_priv = q->jl_priv;
	(void)jfree(q);
}

labCPMOV(q, p)	
register struct jlabel *q, *p;
{
	labcpm++;
	q->jl_slabp->sl_refcnt++;
	slabfree(p->jl_slabp);
	p->jl_priv = clearj.jl_priv;
	p->jl_slabp = q->jl_slabp;
	jl_setflag(p, jl_flag(q));
}	

labCPMOVX(q, p)	
register struct jlabel *q, *p;
{
	labcpmx++;
	q->jl_slabp->sl_refcnt++;
	slabfree(p->jl_slabp);
	p->jl_priv = q->jl_priv;
	p->jl_slabp = q->jl_slabp;
}	

/*
 * Provide a copy of joint label q; giving a new one, 
 * unfrozen and unprivileged.
 */
struct jlabel *
labCP(q)
register struct jlabel *q;
{
	register struct jlabel *p = jalloc();

	labcp++;
	if(p == 0) {
		u.u_error = ENLAB;
		return 0;
	}
	jl_setflag(p, jl_flag(q));
	p->jl_slabp = q->jl_slabp;
	q->jl_slabp->sl_refcnt++;
	return p;
}

struct jlabel *
labCPX(q)
register struct jlabel *q;
{
	register struct jlabel *p = jalloc();

	labcpx++;
	if(p == 0) {
		u.u_error = ENLAB;
		return 0;
	}
	p->jl_priv = q->jl_priv;
	p->jl_slabp = q->jl_slabp;
	q->jl_slabp->sl_refcnt++;
	return p;
}
int labeq, lable, lable1, lable2, lable3;
/*
 * Is label p <= label q?
 */
labLE(p, q)
register struct jlabel *p, *q;
{
	register unsigned long *pl, *ql, *tl;

	lable++;
	if (jl_flag(p)== L_YES || jl_flag(q) == L_YES)
		return 1;
	if (jl_flag(p) == L_NO || jl_flag(q) == L_NO)
		return 0;

	if(p->jl_slabp == 0 || q->jl_slabp == 0 || jl_flag(p) != L_BITS)
		panic("labLE 2");
	if(p->jl_slabp == q->jl_slabp) 
		return 1;
	if(p->jl_slabp == labelbot->jl_slabp) {
		lable1++;
		return 1;
	}
	else if(q->jl_slabp == labelbot->jl_slabp) {
		lable3++;
		return 0;
	}
	lable2++;
	pl = (unsigned long*)p->jl_slabp->sl_bits;
	tl = pl + LABSIZ/sizeof(*tl);
	ql = (unsigned long*)q->jl_slabp->sl_bits;
	while(pl < tl)
		if (*pl++ & ~*ql++)
			return 0;
	return 1;
}

/*
 * Is label at p == label at q, aside from privileges?
 * Not same as labLE(p,q) && labLE(q,p) because of L_YES and L_NO.
 */
labEQ(p, q)
register struct jlabel *p, *q;
{
	labeq++;
	if( jl_flag(p) == L_UNDEF || jl_flag(q) == L_UNDEF)
		panic("labEQ 1");
	if (jl_flag(p) != jl_flag(q)) 
		return 0;
	if(jl_flag(p) == L_BITS && p->jl_slabp != q->jl_slabp)
		return 0;
	return 1;
}
int labmax, labmax2;
/*
 * label max
 */
struct jlabel *
labMAX(q, r)
register struct jlabel *q, *r;
{
	register unsigned long *qp, *rp, *mp, *tp;
	unsigned char m[LABSIZ];

	labmax++;
	if (jl_flag(q) == L_YES) 
		return labCP(r);
	if (jl_flag(r) == L_YES) 
		return labCP(q);
	if (jl_flag(q) == L_NO || jl_flag(r) == L_NO) 
		return labCP(labelno);
	if (jl_flag(q) != L_BITS || jl_flag(r) != L_BITS ||
	    q->jl_slabp == 0 || r->jl_slabp == 0)
		panic("labMAX");
	if (r->jl_slabp == q->jl_slabp)
		return labCP(r);

	
	labmax2++;
	rp = (unsigned long*)r->jl_slabp->sl_bits;
	qp = (unsigned long*)q->jl_slabp->sl_bits;
	mp = (unsigned long*)m;
	tp = (unsigned long*)&m[LABSIZ];
	while(mp < tp)
		*mp++ = *rp++ | *qp++;
	r = jalloc();
	jl_setflag(r, L_BITS);
	r->jl_slabp = slaballoc(m);
	return r;
}
