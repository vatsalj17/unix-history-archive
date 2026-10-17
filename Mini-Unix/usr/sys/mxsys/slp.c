#
#include "param.h"
#include "proc.h"
#include "user.h"
#include "systm.h"
#include "file.h"
#include "inode.h"
#include "buf.h"

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
	spl0();
	if(pri > 0) {
		if(issig())
			goto psig;
		swtch();
		if(issig())
			goto psig;
	} else
		swtch();
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
	rp->p_stat = SRUN;
	retu(u.u_qsav);
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
	i = NPROC;
	do {
		if(p->p_wchan == c) {
			setrun(p);
		}
		p++;
	} while(--i);
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
	if(rp->p_pri < 0)
		rp->p_pri = PSLEP;
	if(rp->p_pri < curpri)
		runrun++;
}

/*
 * This routine is called to reschedule the CPU.
 * if the calling process is not in RUN state,
 * arrangements for it to restart must have
 * been made elsewhere, usually by calling via sleep.
 */

/*
int hbuf[40];
int *hp hbuf;
*/
int swapflag;

swtch()
{
	static struct proc *p;
	register i, n;
	register struct proc *rp;

	if(p == NULL)
		p = &proc[0];

loop:
	rp = p;
	p = NULL;
	n = 128;
	/*
	 * Search for highest-priority runnable process
	 */
	i = NPROC;
	if(runrun && swapflag == 0) {
		do {
			rp++;
			if(rp >= &proc[NPROC])
				rp = &proc[0];
			if(rp->p_stat==SRUN) {
				p = rp;
				n = rp->p_pri;
				runrun = 0;
				break;
			}
		} while(--i);
	} else {
		if(rp->p_stat == SRUN) {
			p = rp;
			n = rp->p_pri;
		}
	}
	/*
	 * If no process is runnable, idle.
	 */
	if(p == NULL) {
		p = rp;
		idle();
		goto loop;
	}
	curpri = n;
	n = rp = p;
/*
	if(hp > &hbuf[36])
		hp = hbuf;
	*hp++ = n;
	*hp++ = u.u_procp;
	*hp++ = time[1];
*/
	if(n != u.u_procp && swapflag == 0) {
	/*
	 * Save stack of current user and
	 * and use system stack.
	 */
		swapflag++;
		savu(u.u_ssav);
		retu(u.u_rsav);
		u.u_procp->p_flag =& ~SLOAD;
		if(u.u_procp->p_stat != SZOMB)
			swap(u.u_procp, B_WRITE);
		swap(rp, B_READ);
		rp->p_flag =| SLOAD;
		retu(u.u_ssav);
		swapflag--;
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

	p = NULL;
	/*
	 * First, just locate a slot for a process
	 * and copy the useful info from this process into it.
	 * The panic "cannot happen" because fork has already
	 * checked for the existence of a slot.
	 */
retry:
	if(++mpid >= NPROC)
		mpid = 1;
	for(rpp = &proc[0]; rpp < &proc[NPROC]; rpp++) {
		if(rpp->p_stat == NULL && p==NULL)
			p = rpp;
		if (rpp->p_pid==mpid)
			goto retry;
	}
	if ((rpp = p)==NULL)
		panic();

	/*
	 * make proc entry for new proc
	 */

	rip = u.u_procp;
	up = rip;
	rpp->p_stat = SRUN;
	rpp->p_flag = SLOAD;
	rpp->p_uid = rip->p_uid;
	rpp->p_pgrp = rip->p_pgrp;
	rpp->p_nice = rip->p_nice;
	rpp->p_pid = mpid;
	rpp->p_ppid = rip->p_pid;

	/*
	 * make duplicate entries
	 * where needed
	 */

	for(rip = &u.u_ofile[0]; rip < &u.u_ofile[NOFILE];)
		if((rpp = *rip++) != NULL)
			rpp->f_count++;
	u.u_cdir->i_count++;
	/*
	 * Partially simulate the environment
	 * of the new process so that when it is actually
	 * created (by copying) it will look right.
	 */

	rpp = p;
	u.u_procp = rpp;
	rip = up;
	rip->p_stat = SIDL;
	savu(u.u_ssav);
	swap(rpp, B_WRITE);	/* swap out child */
	rpp->p_flag =| SSWAP;
	rip->p_stat = SRUN;
	u.u_procp = rip;
	return(0);	/* return to parent */
}
