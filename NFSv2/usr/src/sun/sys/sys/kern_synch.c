/*	@(#)kern_synch.c 1.1 85/05/30 SMI; from UCB 4.26 83/05/21	*/

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/file.h"
#include "../h/vnode.h"
#include "../h/vm.h"
#include "../h/kernel.h"
#include "../h/buf.h"

#ifdef vax
#include "../vax/mtpr.h"	/* XXX */
#endif
/*
 * Force switch among equal priority processes every 100ms.
 */
roundrobin()
{

	runrun++;
	aston();
	timeout(roundrobin, (caddr_t)0, hz / 10);
}

/* constants to digital decay and forget 90% of usage in 5*loadav time */
int	nrscale = 2;
#ifdef vax
/* vax C compiler won't convert float to int for initialization */
long	ccpu = 95*FSCALE/100;		/* exp(-1/20) */
#else
long	ccpu = 0.95122942450071400909 * FSCALE;		/* exp(-1/20) */
#endif

/*
 * Recompute process priorities, once a second
 */
schedcpu()
{
	register struct proc *p;
	register int s, a;
	register long b;

	wakeup((caddr_t)&lbolt);
	for (p = proc; p < procNPROC; p++) if (p->p_stat && p->p_stat!=SZOMB) {
		if (p->p_time != 127)
			p->p_time++;
		if (p->p_stat==SSLEEP || p->p_stat==SSTOP)
			if (p->p_slptime != 127)
				p->p_slptime++;
		if (p->p_flag&SLOAD)
			p->p_pctcpu = ((ccpu * p->p_pctcpu) +
			    ((FSCALE - ccpu) * (p->p_cpticks*FSCALE/hz)))
			    >> FSHIFT;
		p->p_cpticks = 0;
		b = avenrun[0] * nrscale;
		a = ((p->p_cpu & 0377) * b) / (b + FSCALE) + p->p_nice - NZERO;
		if (a < 0)
			a = 0;
		if (a > 255)
			a = 255;
		p->p_cpu = a;
		(void) setpri(p);
		s = spl6();	/* prevent state changes */
		if (p->p_pri >= PUSER) {
			if ((p != u.u_procp || noproc) &&
			    p->p_stat == SRUN &&
			    (p->p_flag & SLOAD) &&
			    p->p_pri != p->p_usrpri) {
				remrq(p);
				p->p_pri = p->p_usrpri;
				setrq(p);
			} else
				p->p_pri = p->p_usrpri;
		}
		splx(s);
	}
	vmmeter();
	if (runin!=0) {
		runin = 0;
		wakeup((caddr_t)&runin);
	}
	if (bclnlist != NULL)
		wakeup((caddr_t)&proc[2]);
	timeout(schedcpu, (caddr_t)0, hz);
}

#define SQSIZE 0100	/* Must be power of 2 */
#define HASH(x)	(( (int) x >> 5) & (SQSIZE-1))
struct prochd slpque[SQSIZE];

/*
 * Give up the processor till a wakeup occurs
 * on chan, at which time the process
 * enters the scheduling queue at priority pri.
 * The most important effect of pri is that when
 * pri<=PZERO a signal cannot disturb the sleep;
 * if pri>PZERO signals will be processed.
 * Callers of this routine must be prepared for
 * premature return, and check that the reason for
 * sleeping has gone away.
 */
sleep(chan, pri)
	caddr_t chan;
	int pri;
{
	register struct proc *rp;
	register struct prochd *hp;
	register s;

	rp = u.u_procp;
	s = spl6();
#ifdef sun
	if (intsvc() || chan==0 || rp->p_stat != SRUN)
#else
	if (chan==0 || rp->p_stat != SRUN)
#endif
		panic("sleep");
	rp->p_wchan = chan;
	rp->p_slptime = 0;
	rp->p_pri = pri;
	/*
	 * put at end of sleep queue
	 */
	hp = &slpque[HASH(chan)];
	insque(rp, hp->ph_rlink);
	if (pri > PZERO) {
		if (ISSIG(rp)) {
			if (rp->p_wchan)
				unsleep(rp);
			rp->p_stat = SRUN;
			(void) spl0();
			goto psig;
		}
		if (rp->p_wchan == 0)
			goto out;
		rp->p_stat = SSLEEP;
		(void) spl0();
		u.u_ru.ru_nvcsw++;
		swtch();
		if (ISSIG(rp))
			goto psig;
	} else {
		rp->p_stat = SSLEEP;
		(void) spl0();
		u.u_ru.ru_nvcsw++;
		swtch();
	}
out:
	splx(s);
	return;

	/*
	 * If priority was low (>PZERO) and
	 * there has been a signal, execute non-local goto through
	 * u.u_qsave, aborting the system call in progress (see trap.c)
	 * (or finishing a tsleep, see below)
	 */
psig:
	longjmp(&u.u_qsave);
	/*NOTREACHED*/
}

/*
 * Remove a process from its wait queue
 */
unsleep(p)
	register struct proc *p;
{
	register s;

	s = spl6();
	if (p->p_wchan) {
		remque(p);
		p->p_rlink = 0;
		p->p_wchan = 0;
	}
	splx(s);
}

/*
 * Wake up all processes sleeping on chan.
 */
wakeup(chan)
	register caddr_t chan;
{
	register struct proc *p;
	register struct prochd *hp;
	int s;

	s = spl6();
	hp = &slpque[HASH(chan)];
restart:
	for (p = hp->ph_link; p != (struct proc *)hp; ) {
		if (p->p_stat != SSLEEP && p->p_stat != SSTOP)
			panic("wakeup");
		if (p->p_wchan==chan) {
			remque(p);
			p->p_rlink = 0;
			p->p_wchan = 0;
			p->p_slptime = 0;
			if (p->p_stat == SSLEEP) {
				/* OPTIMIZED INLINE EXPANSION OF setrun(p) */
				p->p_stat = SRUN;
				if (p->p_flag & SLOAD)
					setrq(p);
				if (p->p_pri < curpri) {
					runrun++;
					aston();
				}
				if ((p->p_flag&SLOAD) == 0) {
					if (runout != 0) {
						runout = 0;
						wakeup((caddr_t)&runout);
					}
					wantin++;
				}
				/* END INLINE EXPANSION */
				goto restart;
			}
		} else
			p = p->p_link;
	}
	splx(s);
}

/*
 * Initialize the (doubly-linked) run queues and sleep queues
 * to be empty.
 */
rqinit()
{
	register int i;

	for (i = 0; i < NQS; i++)
		qs[i].ph_link = qs[i].ph_rlink = (struct proc *)&qs[i];
	for (i = 0; i < SQSIZE; i++)
		slpque[i].ph_link = slpque[i].ph_rlink =
		    (struct proc *)&slpque[i];
}

/*
 * Set the process running;
 * arrange for it to be swapped in if necessary.
 */
setrun(p)
	register struct proc *p;
{
	register int s;

	s = spl6();
	switch (p->p_stat) {

	case 0:
	case SWAIT:
	case SRUN:
	case SZOMB:
	default:
		panic("setrun");

	case SSTOP:
	case SSLEEP:
		unsleep(p);		/* e.g. when sending signals */
		break;

	case SIDL:
		break;
	}
	p->p_stat = SRUN;
	if (p->p_flag & SLOAD)
		setrq(p);
	splx(s);
	if (p->p_pri < curpri) {
		runrun++;
		aston();
	}
	if ((p->p_flag&SLOAD) == 0) {
		if (runout != 0) {
			runout = 0;
			wakeup((caddr_t)&runout);
		}
		wantin++;
	}
}

/*
 * Set user priority.
 * The rescheduling flag (runrun)
 * is set if the priority is better
 * than the currently running process.
 */
setpri(pp)
	register struct proc *pp;
{
	register int p;

	p = (pp->p_cpu & 0377)/4;
	p += PUSER + 2*(pp->p_nice - NZERO);
	if (pp->p_rssize > pp->p_maxrss && freemem < desfree)
		p += 2*4;	/* effectively, nice(4) */
	if (p > 127)
		p = 127;
	if (p < curpri) {
		runrun++;
		aston();
	}
	pp->p_usrpri = p;
	return (p);
}
