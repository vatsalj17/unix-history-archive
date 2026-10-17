#
/*
 */ 
#include "../defines.h"
#include "../param.h"
#include "../user.h"
#include "../proc.h"
#include "../text.h"
#include "../file.h"
#include "../inode.h"
#include "../buf.h"
#ifdef	AUSAML
#include "../lnode.h"
#endif	AUSAML
#include "../systm.h"
#ifdef	ONCE
#include "../seg.h"
#endif	ONCE

#ifdef	MAX_PROC
struct proc *max_proc &proc[0];
#endif	MAX_PROC

#ifdef	PROCESS_QUEUES
struct	proc	*runq;
#endif	PROCESS_QUEUES

/*
 * Give up the processor till a wakeup occurs
 * on chan, at which time the process
 * enters the scheduling queue at priority pri.
 * The most important effect of pri is that when
 * pri<0 a signal cannot disturb the sleep;
 * if pri>=0 signals will be processed.
 * Callers of this routine must be prepared for
 * premature return, and check that the reason for
 * sleeping has gone away.
 */ 
#ifdef	PROCESS_QUEUES
sleep(chan, pri)
{
	register *rp, s;


#ifdef	BETTER_PANIC
	extern char *panicstr;
#endif	BETTER_PANIC
	s = PS->integ;
#ifdef	BETTER_PANIC
	if(panicstr)
	{
		/*
		 * panic has been called
		 * Refuse to reschedule
		 */
		spl0();
		idle();
		PS->integ = s;
		return;
	}
#endif	BETTER_PANIC
	rp = u.u_procp;
	spl6();
	rp->p_stat = SSLEEP;
	rp->p_wchan = chan;
	rp->p_pri = pri;
	if(pri >= 0)
	{
		if(issig())
		{
			rp->p_wchan = 0;
			rp->p_stat = SRUN;
			spl0();
			goto psig;
		}
		spl0();
		swtch();
		if(issig())
			goto psig;
	}
	else
	{
		spl0();
		swtch();
	}
	PS->integ = s;
	return;

	/*
	 * If priority was low (>=0) and
	 * there has been a signal,
	 * execute non-local goto to
	 * the qsav location.
	 * (see trap1/trap.c)
	 */ 
psig:
	aretu(u.u_qsav);
}
#else		(PROCESS_QUEUES)
sleep(chan, pri)
{
	register *rp, s;

	s = PS->integ;
	rp = u.u_procp;
	if(pri >= 0) {
		if(issig())
			goto psig;
		spl6();
		rp->p_wchan = chan;
		rp->p_stat = SWAIT;
		rp->p_pri = pri;
		spl0();
		if(runin != 0) {
			runin = 0;
			setrun(&proc[0]);	/* no need for wakeup */
		}
		swtch();
		if(issig())
			goto psig;
	} else {
		spl6();
		rp->p_wchan = chan;
		rp->p_stat = SSLEEP;
		rp->p_pri = pri;
		spl0();
		swtch();
	}
	PS->integ = s;
	return;

	/*
	 * If priority was low (>=0) and
	 * there has been a signal,
	 * execute non-local goto to
	 * the qsav location.
	 * (see trap1/trap.c)
	 */
psig:
	aretu(u.u_qsav);
}
#endif	PROCESS_QUEUES

/*
 * Wake up all processes sleeping on chan.
 */ 
wakeup(chan)
{
	register struct proc *p;
	register c, i;


	c = chan;
	p = &proc[0];
#ifdef	MAX_PROC
	i = max_proc - p + 1;
#else
	i = NPROC;
#endif	MAX_PROC
	do
	{
#ifdef	PROCESS_QUEUES
		if(p->p_wchan == c && p->p_stat == SSLEEP)
#else
		if(p->p_wchan == c)
#endif	PROCESS_QUEUES
			setrun(p);
		p++;
	}
	while(--i);
}

#ifdef	PROCESS_QUEUES
/*
 * Put a process on the runable queue.
 * The process is always put on the front of the queue,
 * but round-robin scheduling is assured by swtch, which
 * takes the last of eqivalent processes.
 */
setrq(p)
register struct proc *p;
{
	register struct proc *q;
	register s;


	s = PS->integ;
	spl6();
	for(q = runq; q != NULL; q = q->p_link)
	{
		if(q == p)
		{
			printf("proc on q\n");
			goto out;
		}
	}
	p->p_link = runq;
	runq = p;
out:
	PS->integ = s;
}
#endif	PROCESS_QUEUES

/*
 * Set the process running;
 * arrange for it to be swapped in if necessary.
 * The rescheduling flag (runrun)
 * is set if the priority is better
 * than the currently running process.
 */ 
setrun(p)
register struct proc *p;
{
	p->p_wchan = 0;
	p->p_stat = SRUN;
#ifdef	PROCESS_QUEUES
	setrq(p);
#endif	PROCESS_QUEUES
	if((p->p_flag&SLOAD) == 0)
	{
		p->p_time = 0;
		if(runout != 0)
		{
			runout = 0;
			setrun(&proc[0]);
		}
	}
	else if(p->p_pri < curpri)
		runrun++;
}

/*
 * Set user priority.
 *
 * PRIORATE (the rate at which priority is aged
 * for cpu-bound processes) can be found in param.h
 */ 
setpri(up)
register struct proc *up;	/* fix000 */
{
	register p;


	p = (up->p_cpu & 0377)/PRIORATE;
	p =+ PUSER+up->p_nice;
	if(p > 127)
		p = 127;
	up->p_pri = p;
	return(p);		/* fix025 See trap.c */
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
#ifndef	PROCESS_QUEUES
sched()
{
	struct proc *p1;
	register struct proc *rp;
	register a, n;

	/*
	 * find user to swap in
	 * of users ready, select one out longest
	 */

	goto loop;

sloop:
	spl6();		/* fix019 */
	runin++;
	sleep(&runin, PSWP);
	spl0();		/* fix019 */

loop:
/*	spl6();	/* fix019 */
	n = -1;
#ifndef	MAX_PROC
	for(rp = &proc[0]; rp < &proc[NPROC]; rp++)
#else
	for(rp = &proc[0]; rp <= max_proc; rp++)
#endif	MAX_PROC
	if(rp->p_stat==SRUN && (rp->p_flag&SLOAD)==0 && rp->p_time > n) {
		p1 = rp;
		n = rp->p_time;
	}
	if(n == -1) {
		spl6();		/* fix019 */
		runout++;
		sleep(&runout, PSWP);
		spl0();		/* fix019 */
		goto loop;
	}

	/*
	 * see if there is core for that process
	 */

/*	spl0();	/* fix019 */
	rp = p1;
	a = rp->p_size;
	if((rp=rp->p_textp) != NULL)
	{
#ifdef	LOWER_TEXT_SWAPS | SHARED_DATA
		xlock(rp);
#endif	LOWER_TEXT_SWAPS | SHARED_DATA
		if(rp->x_ccount == 0)
			a =+ rp->x_size;
	}
	if((a=malloc(coremap, a)) != NULL)
		goto found2;

	/*
	 * none found,
	 * look around for easy core
	 */

/*	spl6();	/* fix019 */
#ifndef	MAX_PROC
	for(rp = &proc[0]; rp < &proc[NPROC]; rp++)
#else
	for(rp = &proc[0]; rp <= max_proc; rp++)
#endif	MAX_PROC
	if((rp->p_flag&(SSYS|SLOCK|SLOAD))==SLOAD &&
	    (rp->p_stat == SWAIT || rp->p_stat==SSTOP))
		goto found1;

	/*
	 * no easy core,
	 * if this process is deserving,
	 * look around for
	 * oldest process in core
	 */

	if(n < 3)
		goto sloop;
	n = -1;
#ifndef	MAX_PROC
	for(rp = &proc[0]; rp < &proc[NPROC]; rp++)
#else
	for(rp = &proc[0]; rp <= max_proc; rp++)
#endif	MAX_PROC
	if((rp->p_flag&(SSYS|SLOCK|SLOAD))==SLOAD &&
	   (rp->p_stat==SRUN || rp->p_stat==SSLEEP) &&
	    rp->p_time > n) {
		p1 = rp;
		n = rp->p_time;
	}
	if(n < 2)
		goto sloop;
	rp = p1;

	/*
	 * swap user out
	 */

found1:
/*	spl0();	/* fix019 */
	rp->p_flag =& ~SLOAD;
	xswap(rp, 1, 0);
	goto loop;

	/*
	 * swap user in
	 */

found2:
	if(rp != NULL) {
		if(rp->x_ccount == 0) {
#ifdef LOWER_TEXT_SWAPS | SHARED_DATA
			rp->x_flag =| TXTBUSY;
#endif LOWER_TEXT_SWAPS | SHARED_DATA
			swap(rp->x_daddr, a, rp->x_size, B_READ);
			rp->x_caddr = a;
			a =+ rp->x_size;
#ifdef LOWER_TEXT_SWAPS | SHARED_DATA
			xrele(rp);
#endif LOWER_TEXT_SWAPS | SHARED_DATA
		}
		rp->x_ccount++;
	}
	rp = p1;
	swap(rp->p_addr, a, rp->p_size, B_READ);
	mfree(swapmap, (rp->p_size+7)/8, rp->p_addr);
	rp->p_addr = a;
	rp->p_flag =| SLOAD;
	rp->p_time = 0;
	goto loop;
}
#else
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
	for(rp = runq; rp != NULL; rp = rp->p_link)
	{
		if((rp->p_flag&SLOAD) == 0 && rp->p_time > outage)
		{
			p = rp;
			outage = rp->p_time;
		}
	}
	/*
	 * If there is no one there, wait.
	 */ 
	if(outage < 0)
	{
		runout++;
		sleep(&runout, PSWP);
		goto loop;
	}
	spl0();

	/*
	 * See if there is core for that process;
	 * if so, swap it in.
	 */ 

	if(swapin(p))
		goto loop;

	/*
	 * none found, so search for a candidate for swapping.
	 */ 

/*	spl6();		/* fix019 */
	swapri = 0;	/* this is a tunable value (max = 0) */
	inage = 0;
#ifdef	MAX_PROC
	for(rp = &proc[0]; rp <= max_proc; rp++)
#else
	for(rp = &proc[0]; rp < &proc[NPROC]; rp++)
#endif	MAX_PROC
	{
		if((rp->p_flag&(SSYS|SLOCK|SLOAD)) != SLOAD)
			continue;
#ifdef	ZOMBIE
		if(rp->p_stat == SZOMB)
			continue;
#endif	ZOMBIE
#ifdef	SHARED_DATA | LOWER_TEXT_SWAPS
		if(rp->p_textp && rp->p_textp->x_flag&TXTBUSY)
			continue;
#endif	SHARED_DATA | LOWER_TEXT_SWAPS
		if(rp->p_stat == SSLEEP || rp->p_stat == SSTOP)
		{
			a = (rp->p_pri>>1) + rp->p_time;
			if(a > swapri)
			{
				p = rp;
				swapri = a;
			}
		}
		else if(swapri <= 0 && rp->p_stat == SRUN && rp->p_time > inage)
		{
			p = rp;
			inage = rp->p_time;
		}
	}
/*	spl0();		/* fix019 */
	/*
	 * Swap found user out if sleeping,
	 * or if he has spent at least 4 seconds in core and
	 * the swapped-out process has spent at 2 seconds out.
	 * Otherwise wait a bit and try again.
	 */ 
	if(swapri > 0 || (outage >= 2 && inage >= 4))
	{
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
swapin(p)
register struct proc *p;
{
	register struct text *xp;
	register int a;
	int x;


	if((a = malloc(coremap, p->p_size)) == NULL)
		return(0);
	if(xp = p->p_textp)
	{
#ifdef	SHARED_DATA | LOWER_TEXT_SWAPS
		xlock(xp);
#endif	SHARED_DATA | LOWER_TEXT_SWAPS
		if(xp->x_ccount == 0)
		{
			if((x = malloc(coremap, xp->x_size)) == NULL)
			{
				mfree(coremap, p->p_size, a);
				return(0);
			}
#ifdef	SHARED_DATA | LOWER_TEXT_SWAPS
			xp->x_flag =| TXTBUSY;
#endif	SHARED_DATA | LOWER_TEXT_SWAPS
			xp->x_caddr = x;
			swap(xp->x_daddr, x, xp->x_size, B_READ);
#ifdef	SHARED_DATA | LOWER_TEXT_SWAPS
			xrele(xp);
#endif	SHARED_DATA | LOWER_TEXT_SWAPS
		}
		xp->x_ccount++;
	}
	swap(p->p_addr, a, p->p_size, B_READ);
	mfree(swapmap, (p->p_size+7)/8, p->p_addr);
	p->p_addr = a;
	p->p_flag =| SLOAD;
	p->p_time = 0;
	return(1);
}
#endif	PROCESS_QUEUES

#ifdef	PROCESS_QUEUES
qswtch()
{

	setrq(u.u_procp);
	swtch();
}
#endif	PROCESS_QUEUES

/*
 * This routine is called to reschedule the CPU.
 * if the calling process is not in RUN state,
 * arrangements for it to restart must have
 * been made elsewhere, usually by calling via sleep.
 */ 
#ifdef	SWITCH_DISPLAY
int	swi_count;
int	lst_count;
#endif	SWITCH_DISPLAY

#ifdef	PROCESS_QUEUES
swtch()
{
	register n;
	register struct proc *p, *q;
	static struct proc *pp, *pq;


#ifdef	SWITCH_DISPLAY
	swi_count++;
#endif	SWITCH_DISPLAY

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
	for(p = runq; p != NULL; p = p->p_link)
	{
		if(p->p_flag&SLOAD)
		{
			/*
			 * Choose the last on queue
			 * with equivalent p_pri
			 */
			if(p->p_pri <= n)
			{
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
	if(p == NULL)
	{
		spl0();
		idle();
		goto loop;
	}
	q = pq;
	if(q == NULL)
		runq = p->p_link;
	else
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
	if(p->p_flag&SSWAP)
	{
		p->p_flag =& ~SSWAP;
		aretu(u.u_ssav);
	}
	/*
	 * The value returned here has many subtle implications.
	 * See the newproc comments.
	 */ 
	return(1);
}
#else
swtch()
{
	static struct proc *p;
	register i, n;
	register struct proc *rp;

#ifdef BETTER_PANIC
	extern char *panicstr;
	if(panicstr)
	{
		/* panicing... refuse to reschedule */
		idle();
		return;
	}
#endif BETTER_PANIC
#ifdef	SWITCH_DISPLAY
	swi_count++;
#endif	SWITCH_DISPLAY
#ifndef	MAX_PROC
	if(p == NULL)
#endif	MAX_PROC
#ifdef	MAX_PROC
	if( (p == NULL) || (p > max_proc) )
#endif	MAX_PROC
		p = &proc[0];
	/*
	 * Remember stack of caller
	 */
	savu(u.u_rsav);
	/*
	 * Switch to scheduler's stack
	 */
	retu(proc[0].p_addr);

loop:
	runrun = 0;
	rp = p;
	p = NULL;
	n = 128;
	/*
	 * Search for highest-priority runnable process
	 */
#ifndef	MAX_PROC
	i = NPROC;
#endif	MAX_PROC
#ifdef	MAX_PROC
	i = max_proc - &proc[0] + 1;
#endif	MAX_PROC
	do {
		rp++;
#ifndef	MAX_PROC
		if(rp >= &proc[NPROC])
#endif	MAX_PROC
#ifdef	MAX_PROC
		if(rp > max_proc)
#endif	MAX_PROC
			rp = &proc[0];
		if(rp->p_stat==SRUN && (rp->p_flag&SLOAD)!=0) {
			if(rp->p_pri < n) {
				p = rp;
				n = rp->p_pri;
			}
		}
	} while(--i);
	/*
	 * If no process is runnable, idle.
	 */
	if(p == NULL) {
		p = rp;
		idle();
		goto loop;
	}
	rp = p;
	curpri = n;
	/*
	 * Switch to stack of the new process and set up
	 * his segmentation registers.
	 */
	retu(rp->p_addr);
	sureg();
	/*
	 * If the new process paused because it was
	 * swapped out, set the stack level to the last call
	 * to savu(u_ssav).  This means that the return
	 * which is executed immediately after the call to aretu
	 * actually returns from the last routine which did
	 * the savu.
	 *
	 * You are not expected to understand this.
	 */
	if(rp->p_flag&SSWAP) {
		rp->p_flag =& ~SSWAP;
		aretu(u.u_ssav);
	}
	/*
	 * The value returned here has many subtle implications.
	 * See the newproc comments.
	 */
	return(1);
}
#endif	PROCESS_QUEUES

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
	mpid++;
	if(mpid < 0) {
		mpid = 0;
		goto retry;
	}
	for(rpp = &proc[0]; rpp < &proc[NPROC]; rpp++) {
		if(rpp->p_stat == NULL && p==NULL)
			p = rpp;
		if (rpp->p_pid==mpid)
			goto retry;
	}
	if ((rpp = p)==NULL)
		panic("no procs");

	/*
	 * make proc entry for new proc
	 */

	rip = u.u_procp;
	up = rip;
	rpp->p_stat = SRUN;
	rpp->p_flag = SLOAD;
	rpp->p_uid = rip->p_uid;
	rpp->p_ttyp = rip->p_ttyp;
	rpp->p_nice = rip->p_nice;
	rpp->p_textp = rip->p_textp;
	rpp->p_pid = mpid;
	rpp->p_ppid = rip->p_pid;
	rpp->p_time = rip->p_time;	/* fix024 */
	rpp->p_cpu = rip->p_cpu;	/* fix024 */
#ifdef	MEW_SLEEP
	rpp->p_stl = 0;
#endif	MEW_SLEEP
#ifdef	AUSAML
	rpp->p_lnode = rip->p_lnode;
#endif	AUSAML
#ifdef	TIME_LIMITS
	rpp->p_rtl = rip->p_rtl;
#endif	TIME_LIMITS
#ifdef	IGNORE_SIGNALS
	rpp->p_ignsig = rip->p_ignsig;
#endif	IGNORE_SIGNALS
#ifdef	MAX_PROC
	if( rpp > max_proc )
	{
		max_proc = rpp;
#	ifdef	HIGH_PROC
		if( rpp > high_proc )
		{
			high_proc  = rpp;
			high_nproc = rpp - &proc[0] + 1;
		}
#	endif	HIGH_PROC
	}
#endif	MAX_PROC

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
#ifdef	PROCESS_QUEUES
	setrq(rpp);
#endif	PROCESS_QUEUES
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
#ifdef	PROCESS_QUEUES
		qswtch();
#else
		swtch();
#endif	PROCESS_QUEUES
		/* no return */
	}
	p->p_addr = a2;
	for(i=0; i<n; i++)
		copyseg(a1+i, a2++);
	mfree(coremap, n, a1);
	retu(p->p_addr);
	sureg();
}

#ifdef	ONCE
#include "../estabur.h"
#endif
