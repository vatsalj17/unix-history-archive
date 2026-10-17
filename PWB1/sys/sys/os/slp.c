#
#include "../hd/param.h"
#include "../hd/user.h"
#include "../hd/proc.h"
#include "../hd/text.h"
#include "../hd/systm.h"
#include "../hd/file.h"
#include "../hd/inode.h"
#include "../hd/buf.h"
#include "../hd/var.h"

/*
 * Give up the processor till a wakeup occurs
 * on chan, at which time the process
 * enters the scheduling queue at priority pri.
 * The most important effect of pri is that when
 * pri<=0 a signal cannot disturb the sleep;
 * if pri>0 signals will be processed.
 * Callers of this routine must be prepared for
 * premature return, and check that the reason for
 * sleeping has gone away.
 */
sleep(chan, pri)
{
	register *rp, s;

	s = PS->integ;
	rp = u.u_procp;
	spl6();
	rp->p_stat = SSLEEP;
	rp->p_wchan = chan;
	rp->p_pri = pri;
	if(pri > 0) {
		if(issig()) {
			rp->p_wchan = 0;
			rp->p_stat = SRUN;
			spl0();
			goto psig;
		}
		spl0();
		swtch();
		if(issig())
			goto psig;
	} else {
		spl0();
		swtch();
	}
	PS->integ = s;
	return;

	/*
	 * If priority was low (>0) and
	 * there has been a signal,
	 * execute non-local goto to
	 * the qsav location.
	 * (see trap1/trap.c)
	 */
psig:
	aretu(u.u_qsav);
}

/*
 * Wake up all processes sleeping on chan.
 */
wakeup(chan)
{
	register struct proc *p;
	register c, i;

	c = chan;
	p = &proc[0];
	i = v.ve_proc;
	do {
		if(p->p_wchan == c && p->p_stat==SSLEEP)
			setrun(p);
		p++;
	} while(p != i);
}

setrq(p)
struct proc *p;
{
	register struct proc *q;
	register s;

	s = PS->integ;
	spl6();
	for(q=runq; q!=NULL; q=q->p_link)
		if(q == p) {
			printf("proc on q\n");
			goto out;
		}
	p->p_link = runq;
	runq = p;
out:
	PS->integ = s;
}

/*
 * Set the process running;
 * arrange for it to be swapped in if necessary.
 */
setrun(p)
{
	register struct proc *rp;

	rp = p;
	rp->p_wchan = 0;
	rp->p_stat = SRUN;
	setrq(p);
	if((rp->p_flag&SLOAD)==0) {
		rp->p_time = 0;
		if(runout != 0) {
			runout = 0;
			setrun(&proc[0]);
		}
	} else
		if(rp->p_pri < curpri)
			runrun++;
}

/*
 * Set user priority.
 * The rescheduling flag (runrun)
 * is set if the priority is better
 * than the currently running process.
 */
setpri(up)
{
	register *pp, p;

	pp = up;
	p = pp->p_cpu/4;
	p =+ PUSER + pp->p_nice;
	if(p > 127)
		p = 127;
	pp->p_pri = p;
	return(p);
}

/*
 * The main loop of the scheduling (swapping)
 * process.
 * The basic idea is:
 *  see if anyone wants to be swapped in;
 *  swap out processes until there is room;
 *  swap him in;
 *  repeat.
 * The runout flag is set whenever someone is swapped out.
 * Sched sleeps on it awaiting work.
 *
 * Sched sleeps on runin whenever it cannot find enough
 * core (by swapping out or otherwise) to fit the
 * selected swapped process.  It is awakend when the
 * core situation changes and in any event once per second.
 */
sched()
{
	register struct proc *rp, *p;
	register outage, inage;
	int swapri, a;

	/*
	 * find user to swap in;
	 * of users ready, select one out longest
	 */

loop:
	spl6();
	outage = -1;
	for(rp = &proc[0]; rp < v.ve_proc; rp++)
	if (rp->p_stat==SRUN && (rp->p_flag&SLOAD)==0 &&
	    rp->p_time > outage) {
		p = rp;
		outage = rp->p_time;
	}
	/*
	 * If there is no one there, wait.
	 */
	if (outage<0) {
		runout++;
		sleep(&runout, PSWP);
		goto loop;
	}
	spl0();

	/*
	 * See if there is core for that process;
	 * if so, swap it in.
	 */

	if (swapin(p))
		goto loop;

	/*
	 * none found.
	 */

	spl6();
	swapri = 0;
	inage = 0;
	for(rp = &proc[0]; rp < v.ve_proc; rp++) {
		if ((rp->p_flag&(SSYS|SLOCK|SLOAD))!=SLOAD)
			continue;
		if (rp->p_textp && rp->p_textp->x_flag&XLOCK)
			continue;
		if(rp->p_stat==SSLEEP || rp->p_stat==SSTOP) {
			a = (rp->p_pri>>1) + rp->p_time;
			if(a>swapri) {
				p = rp;
				swapri = a;
			}
		} else
		if(swapri<=0 && rp->p_stat==SRUN && rp->p_time>inage) {
			p = rp;
			inage = rp->p_time;
		}
	}
	spl0();
	/*
	 * Swap found user out if sleeping,
	 * or if he has spent at least 4 seconds in core and
	 * the swapped-out process has spent at 2 seconds out.
	 * Otherwise wait a bit and try again.
	 */
	if (swapri>0 || (outage>=2 && inage>=4)) {
		p->p_flag =& ~SLOAD;
		xswap(p, 1, 0);
		goto loop;
	}
	spl6();
	runin++;
	sleep(&runin, PSWP);
	goto loop;

}

/*
 * Swap a process in.
 * Allocate data and possible text separately.
 * It would be better to do largest first.
 */
swapin(pp)
struct proc *pp;
{
	register struct proc *p;
	register struct text *xp;
	register int a;
	int x;

	p = pp;
	if ((a = malloc(coremap, p->p_size)) == NULL)
		return(0);
	if (xp = p->p_textp) {
		xlock(xp);
		if (xp->x_ccount==0) {
			if ((x = malloc(coremap, xp->x_size)) == NULL) {
				xunlock(xp);
				mfree(coremap, p->p_size, a);
				return(0);
			}
			xp->x_caddr = x;
			if ((xp->x_flag&XLOAD)==0)
				swap(xp->x_daddr,x,xp->x_size,B_READ);
		}
		xp->x_ccount++;
		xunlock(xp);
	}
	swap(p->p_addr, a, p->p_size, B_READ);
	mfree(swapmap, ctob(p->p_size), p->p_addr);
	p->p_addr = a;
	p->p_flag =| SLOAD;
	p->p_time = 0;
	return(1);
}

qswtch()
{

	setrq(u.u_procp);
	swtch();
}

/*
 * This routine is called to reschedule the CPU.
 * if the calling process is not in RUN state,
 * arrangements for it to restart must have
 * been made elsewhere, usually by calling via sleep.
 */
swtch()
{
	register n;
	register struct proc *p, *q;
	static struct proc *pp, *pq;

	/*
	 * Remember stack of caller
	 * and switch to schedulers stack.
	 */
	savu(u.u_rsav);
	retu(proc[0].p_addr);

loop:
	spl6();
	runrun = 0;
	pp = NULL;
	q = NULL;
	n = 128;
	/*
	 * Search for highest-priority runnable process
	 */
	for(p=runq; p!=NULL; p=p->p_link) {
		if((p->p_stat==SRUN) && (p->p_flag&SLOAD)) {
			if(p->p_pri <= n) {
				pp = p;
				pq = q;
				n = p->p_pri;
			}
		}
		q = p;
	}
	/*
	 * If no process is runnable, idle.
	 */
	p = pp;
	if(p == NULL) {
		idle();
		spl0();
		goto loop;
	}
	q = pq;
	if(q == NULL)
		runq = p->p_link; else
		q->p_link = p->p_link;
	curpri = n;
	spl0();
	/*
	 * Switch to stack of the new process and set up
	 * his segmentation registers.
	 */
	retu(p->p_addr);
	sureg();
	/*
	 * If the new process paused because it was
	 * swapped out, set the stack level to the last call
	 * to savu(u_ssav).  This means that the return
	 * which is executed immediately after the call to aretu
	 * actually returns from the last routine which did
	 * the savu.
	 */
	if(p->p_flag&SSWAP) {
		p->p_flag =& ~SSWAP;
		aretu(u.u_ssav);
	}
	/*
	 * The value returned here has many subtle implications.
	 * See the newproc comments.
	 */
	return(1);
}

/*
 * Create a new process-- the internal version of
 * sys fork.
 * It returns 1 in the new process.
 * How this happens is rather hard to understand.
 * The essential fact is that the new process is created
 * in such a way that appears to have started executing
 * in the same call to newproc as the parent;
 * but in fact the code that runs is that of swtch.
 * The subtle implication of the returned value of swtch
 * (see above) is that this is the value that newproc's
 * caller in the new process sees.
 */
newproc()
{
	int a1, a2;
	struct proc *p, *up;
	register struct proc *rpp;
	register *rip, n;
	struct proc *pend;

	p = NULL;
	/*
	 * First, just locate a slot for a process
	 * and copy the useful info from this process into it.
	 * The panic "cannot happen" because fork has already
	 * checked for the existence of a slot.
	 */
retry:
	mpid++;
	if(mpid < 0) {
		mpid = 0;
		goto retry;
	}
	for(rpp = &proc[0]; rpp < &proc[v.v_proc]; rpp++) {
		if(rpp->p_stat == NULL) {
			if(p == NULL)
				p = rpp;
		} else
			pend = rpp;
		if (rpp->p_pid==mpid || rpp->p_pgrp==mpid)
			goto retry;
	}
	if ((rpp = p)==NULL)
		panic("no procs");
	if (rpp > pend)
		pend = rpp;
	pend++;
	v.ve_proc = pend;

	/*
	 * make proc entry for new proc
	 */

	rip = u.u_procp;
	up = rip;
	rpp->p_stat = SRUN;
	rpp->p_clktim = 0;
	rpp->p_flag = SLOAD;
	rpp->p_uid = rip->p_uid;
	rpp->p_pgrp = rip->p_pgrp;
	rpp->p_nice = rip->p_nice;
	rpp->p_textp = rip->p_textp;
	rpp->p_pid = mpid;
	rpp->p_ppid = rip->p_pid;
	rpp->p_time = 0;
	rpp->p_cpu = 0;

	/*
	 * make duplicate entries
	 * where needed
	 */

	for(rip = &u.u_ofile[0]; rip < &u.u_ofile[NOFILE];)
		if((rpp = *rip++) != NULL)
			rpp->f_count++;
	if((rpp=up->p_textp) != NULL) {
		rpp->x_count++;
		rpp->x_ccount++;
	}
	u.u_cdir->i_count++;
	/*
	 * Partially simulate the environment
	 * of the new process so that when it is actually
	 * created (by copying) it will look right.
	 */
	savu(u.u_rsav);
	rpp = p;
	u.u_procp = rpp;
	rip = up;
	n = rip->p_size;
	a1 = rip->p_addr;
	rpp->p_size = n;
	a2 = malloc(coremap, n);
	/*
	 * If there is not enough core for the
	 * new process, swap out the current process to generate the
	 * copy.
	 */
	if(a2 == NULL) {
		rip->p_stat = SIDL;
		rpp->p_addr = a1;
		savu(u.u_ssav);
		xswap(rpp, 0, 0);
		rpp->p_flag =| SSWAP;
		rip->p_stat = SRUN;
	} else {
	/*
	 * There is core, so just copy.
	 */
		rpp->p_addr = a2;
		while(n--)
			copyseg(a1++, a2++);
	}
	u.u_procp = rip;
	setrq(rpp);
	return(0);
}

/*
 * Change the size of the data+stack regions of the process.
 * If the size is shrinking, it's easy-- just release the extra core.
 * If it's growing, and there is core, just allocate it
 * and copy the image, taking care to reset registers to account
 * for the fact that the system's stack has moved.
 * If there is no core, arrange for the process to be swapped
 * out after adjusting the size requirement-- when it comes
 * in, enough core will be allocated.
 * Because of the ssave and SSWAP flags, control will
 * resume after the swap in swtch, which executes the return
 * from this stack level.
 *
 * After the expansion, the caller will take care of copying
 * the user's stack towards or away from the data area.
 */
expand(newsize)
{
	int i, n;
	register *p, a1, a2;

	p = u.u_procp;
	n = p->p_size;
	p->p_size = newsize;
	a1 = p->p_addr;
	if(n >= newsize) {
		mfree(coremap, n-newsize, a1+newsize);
		return;
	}
	savu(u.u_rsav);
	a2 = malloc(coremap, newsize);
	if(a2 == NULL) {
		savu(u.u_ssav);
		xswap(p, 1, n);
		p->p_flag =| SSWAP;
		qswtch();
		/* no return */
	}
	p->p_addr = a2;
	for(i=0; i<n; i++)
		copyseg(a1+i, a2++);
	mfree(coremap, n, a1);
	retu(p->p_addr);
	sureg();
}
