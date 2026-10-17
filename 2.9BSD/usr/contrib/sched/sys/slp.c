#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/map.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../h/buf.h"

/*
 * This file contains all of the really interesting differences between
 * the new scheduler and the standard UNIX scheduler.  I believe that
 * its workings are more transparent than those of the standard scheduler,
 * but that may just be a result of designing and coding it.
 * Darwyn Peachey, 28-Jun-83.
 */

struct proc *ready;	/* linked list of processes ready to run */
#define TTIINC		6	/* tty input increment */
#define TTOINC		4	/* tty output increment */
#define OTHINC		2	/* increment for other sleeps */

short curlevel = 0;
#define QUANTUM 4	/* CPU quantum -- must be <= 7 to avoid overflow */
short quantum = QUANTUM;
#define SQSIZE 0100	/* Must be power of 2 */
#define HASH(x)	(( (int) x >> 5) & (SQSIZE-1))
struct proc *slpque[SQSIZE];

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
{
	register struct proc *rp;
	register s, h;

	rp = u.u_procp;
	s = spl6();
	if (chan==0)
		panic("wchan=0");
	rp->p_wchan = chan;
	if(pri <= PZERO)
		rp->p_stat = SSLEEP;
	else
		rp->p_stat = SWAIT;
	h = HASH(chan);
	/* in order to sleep, you must be running -- therefore, you should */
	/* have p_link set to NULL */
	rp->p_link = slpque[h];
	slpque[h] = rp;
	if(pri > PZERO) {
		if(issig()) {
			rp->p_wchan = 0;
			rp->p_stat = SRUN;
			slpque[h] = rp->p_link;
			spl0();
			goto psig;
		}
		spl0();
		if(runin != 0) {
			runin = 0;
			wakeup((caddr_t)&runin);
		}
		swtch();
		if(issig())
			goto psig;
	} else {
		spl0();
		swtch();
	}
	splx(s);
	return;

	/*
	 * If priority was low (>PZERO) and
	 * there has been a signal,
	 * execute non-local goto to
	 * the qsav location.
	 * (see system call case of trap in trap.c)
	 */
psig:
	resume(u.u_procp->p_addr, u.u_qsav);
}

/*
 * Wake up all processes sleeping on chan.
 */
wakeup(chan)
register caddr_t chan;
{
	register struct proc *p, *q;
	register i;
	int s;

	s = spl6();
	i = HASH(chan);
loop:	p = slpque[i];
	q = NULL;
	while(p != NULL) {
		if(p->p_wchan==chan && p->p_stat!=SZOMB) {
			if (q == NULL)
				slpque[i] = p->p_link;
			else
				q->p_link = p->p_link;
			p->p_wchan = 0;
			p->p_link = NULL;
			setrun(p);
			goto loop;
		}
		q = p;
		p = p->p_link;
	}
	splx(s);
}

/*
 * SETRQ -- called only from within this source file, SETRQ inserts
 * a specified process into the "ready" process queue used by the
 * CPU dispatcher "swtch".  Sorting into the ready list is based on
 * the priority level p_level of the process.  The flag argument
 * determines whether the process is put at the head or the tail
 * of the processes at its level (head if nonzero).
 */

setrq(p,flag)
register struct proc *p;
int flag;	/* if nonzero, insert at head */
{
	register struct proc *q, *ip;	/* ip is "insertion point" */
	int s;

	s = spl6();

	ip = NULL;

	/* process p will be inserted before the first process q which */
	/* satisfies the necessary condition (q worse prio than p)     */
	for(q = ready; q != NULL; q = q->p_link){
		if(q->p_level < p->p_level ||(flag && q->p_level == p->p_level))
			break;
		ip = q;
	}

	if(ip == NULL){	/* insert at start of list */
		p->p_link = ready;
		ready = p;
	}
	else { /* general insertion case */
		p->p_link = ip->p_link;	/* save rest of list */
		ip->p_link = p;
	}

	if(p->p_level > curlevel)
		runrun++;	/* this line implements preemptive scheduling */
	splx(s);
}

/*
 * Set the process running;
 * arrange for it to be swapped in if necessary.
 */
setrun(p)
register struct proc *p;
{
	register caddr_t w;
	register int i;

	if (p->p_stat==SEMPTY || p->p_stat==SZOMB)
		panic("dead proc");
	/*
	 * The assignment to w is necessary because of
	 * race conditions. (Interrupt between test and use)
	 */
	if (w = p->p_wchan) {
		wakeup(w);
		return;
	}
	p->p_stat = SRUN;
	/* adjust level if necessary */
	if((i = p->p_baslev) < RTLEVEL){
		if(p->p_level > i)
			p->p_level--;
		if(p->p_sflag & STTYIN)
			i += TTIINC;
		else if(p->p_sflag & STTYOUT)
			i += TTOINC;
		else
			i += OTHINC;
		if(i >= RTLEVEL)
			i = RTLEVEL - 1;
		if(i > p->p_level)
			p->p_level = i;
	}
	p->p_quant = 0;
	p->p_sflag &= ~ (STTYIN | STTYOUT);
	setrq(p,0);
	if(runout != 0 && (p->p_flag&SLOAD) == 0) {
		runout = 0;
		wakeup((caddr_t)&runout);
	}
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
 * selected swapped process.  It is awakened when the
 * core situation changes and in any case once per second.
 */
sched()
{
	register struct proc *p, *rp, *q;
	int maxsize, desperate;

	for(;;){	/* keep loading processes !!! */
		/*
		 * Find process to swap in; the eligible process is the first
		 * one in the "ready" list which is not loaded at present.
		 */
		spl6();
		for (p = ready; p != NULL; p = p->p_link){
			if((p->p_stat == SRUN) && (p->p_flag & SLOAD) == 0)
				break;
		}
		if (p == NULL){
			runout++;
			sleep((caddr_t)&runout, PSWP);
			continue;
		}
		spl0();
	
		/* if there is enough free core, swap in the process */
	
		if (swapin(p)){
			continue;
		}
	
		/* find core by out-swapping largest eligible process */
		/* This is certainly the most suspect part of this
		 * version of the swap scheduler.  I am experimenting
		 * with a version that knows the location of all processes
		 * and texts in main memory, and makes more informed
		 * swap-out decisions.  This version is still too
		 * experimental to distribute, I'm afraid.  If you
		 * are REALLY interested in experimenting with it, you
		 * can contact me:
		 * Darwyn Peachey, (306) 343-3783, or uucp net mail
		 * at ...utzoo!utcsrgv!sask!kimnovax!peachey or
		 *             utah-cs!sask!kimnovax!peachey.
		 */
		spl6();
		q = NULL;
		for(desperate = 0; q == NULL && desperate <= 1; desperate++){
			maxsize = -1;
			for (rp = &proc[0]; rp < &proc[NPROC]; rp++) {
				if(swapok(p,rp,desperate)){
					if (maxsize < rp->p_size) {
						q = rp;
						maxsize = rp->p_size;
					}
				}
			}
		}
		if(q != NULL){
			spl0();
			q->p_flag &= ~SLOAD;
			xswap(q, 1, 0);
		}
		else {
			runin++;
			sleep((caddr_t)&runin, PSWP);
		}
	}
}
/*
 * SWAPOK is a predicate (truth-valued function) used to determine the
 * desirability of swapping process 'loser' in order to bring in process
 * 'winner'.  The 'desperate' flag captures the distinction between
 * 'easy core' and 'hard core'.
 */

int
swapok(winner,loser,desperate)
register struct proc *winner, *loser;
int desperate;
{
	register struct proc *p;

	if((loser->p_flag & (SSYS|SLOCK|SULOCK|SLOAD)) != SLOAD)
		return(0);
	/* don't swap out a process which is at "real-time" priority level */
	if(loser->p_level >= RTLEVEL)
		return(0);
	/* avoid deadlocks -- don't swap out process with locked text */
	if(loser->p_textp != NULL && (loser->p_textp->x_flag & XLOCK) != 0)
		return(0);
	switch(loser->p_stat){
	case SSTOP:
	case SWAIT:
		break;
	case SRUN:
		if(!desperate)
			return(0);
		if(loser->p_sflag & SEXEC){
			return(0);	/* avoid swapping an EXECing proc */
		}
		/* make sure that winner is higher priority than loser */
		if(winner->p_level < loser->p_level)
			return(0);
		if(winner->p_level == loser->p_level){
			p = winner;
			while(p && p != loser && p->p_level == winner->p_level)
				p = p->p_link;
			if(p != loser)
				return(0);
		}
		break;
	case SSLEEP:
		if(!desperate)
			return(0);
		if(loser->p_sflag & SEXEC){
			return(0);	/* avoid swapping an EXECing proc */
		}
		break;
	default:
		return(0);
	}
	/* if loser has been loaded less than time-to-swap, or */
	/* loser has never been scheduled to run since being loaded */
	return(1);
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
	mfree(swapmap, ctod(p->p_size), p->p_addr);
	p->p_addr = a;
	p->p_quant = 0;		/* make sure it runs a full quantum */
	p->p_flag |= SLOAD;
	return(1);
}

/*
 * Give up the CPU at end of quantum or when preempted.
 * put the current process on
 * the Q of running processes and
 * call the scheduler.
 */
qswtch()
{
	register struct proc *p;
	register int f = 0;	/* setrq flag */
	register int i;

	p = u.u_procp;

	if(quantum <= 0){	/* end-of-quantum processing */
		/* adjust level if necessary */
		if((i = p->p_baslev) < RTLEVEL){
			if(p->p_level > i)
				p->p_level--;
			if(p->p_sflag & STTYIN)
				i += TTIINC;
			else if(p->p_sflag & STTYOUT)
				i += TTOINC;
			if(i >= RTLEVEL)
				i = RTLEVEL - 1;
			if(i > p->p_level)
				p->p_level = i;
		}
		p->p_quant = 0;
		p->p_sflag &= ~ (STTYIN | STTYOUT);
	}
	else {			/* process preempted by higher level process */
		p->p_quant = quantum;
		f++;	/* insert process at head of level subqueue */
	}
	setrq(p,f);
	swtch();
}

/*
 * This routine is called to reschedule the CPU.
 * if the calling process is not in RUN state,
 * arrangements for it to restart must have
 * been made elsewhere, usually by calling via sleep.
 * There is a race here. A process may become
 * ready after it has been examined.
 * In this case, idle() will be called and
 * will return in at most 1HZ time.
 * i.e. its not worth putting an spl() in.
 */
swtch()
{
	register struct proc *p, *q;
	register int n;

	if (save(u.u_rsav)) {
		/* save only returns true when you are resuming */
		/* the process after it has been asleep -- thus, */
		/* the thing to do here is to set up registers and */
		/* return back to the code which originally called */
		/* the "swtch" */
		sureg();
		return;
	}
	if (u.u_fpsaved==0) {
		savfp(&u.u_fps);
		u.u_fpsaved = 1;
	}
	spl6();
loop:
	runrun = 0;
	q = NULL;	/* keep track of predecessor of p for deletion */
	for(p = ready; p != NULL && (p->p_flag & SLOAD) == 0; p = p->p_link){
		q = p;	/* update predecessor */
	}
	if(p == NULL){
		/* no loaded ready process -- processor idle */
		/* if idle() called, but "starved" is nonzero, CPU is wasted */
		idle();
		goto loop;
	}
	else
		if(p->p_stat != SRUN)
			panic("not ready");
	if(q == NULL)
		ready = p->p_link;
	else
		q->p_link = p->p_link;
	p->p_link = NULL;
	n = p->p_level;
	curlevel = n;
	if(p->p_quant > 0)	/* last quantum partially used */
		quantum = p->p_quant;
	else {
		if(n >= RTLEVEL)
			quantum = QUANTUM;
		else
			quantum = QUANTUM * (RTLEVEL - n);
	}
	spl0();
	/*
	 * The rsav (ssav) contents are interpreted in the new address space
	 */
	n = p->p_flag&SSWAP;
	p->p_flag &= ~SSWAP;
	resume(p->p_addr, n? u.u_ssav: u.u_rsav);
}

/*
 * Create a new process-- the internal version of
 * sys fork.
 * It returns 1 in the new process, 0 in the old.
 */
newproc()
{
	int a1, a2;
	struct proc *p;
	register struct proc *new, *old;
	register n;

	p = NULL;
	/*
	 * First, just locate a slot for a process
	 * and copy the useful info from this process into it.
	 * The panic "cannot happen" because fork has already
	 * checked for the existence of a slot.
	 */
retry:
	mpid++;
	if(mpid >= 30000) {
		mpid = 0;
		goto retry;
	}
	for(new = &proc[0]; new < &proc[NPROC]; new++) {
		if(new->p_stat == SEMPTY && p==NULL)
			p = new;
		if (new->p_pid==mpid || new->p_pgrp==mpid)
			goto retry;
	}
	if ((new = p)==NULL)
		panic("no proc");

	/*
	 * make proc entry for new proc
	 */

	old = u.u_procp;
	new->p_stat = SRUN;
	new->p_clktim = 0;
	new->p_flag = SLOAD;
	new->p_uid = old->p_uid;
	new->p_pgrp = old->p_pgrp;
	new->p_textp = old->p_textp;
	new->p_pid = mpid;
	new->p_ppid = old->p_pid;
	new->p_link = NULL;
	new->p_quant = 0;
	new->p_sflag = 0;
	new->p_baslev = old->p_baslev;
	new->p_level = (new->p_baslev>=RTLEVEL)?new->p_baslev:(RTLEVEL-1);
	/*
	 * make duplicate entries
	 * where needed
	 */

	for(n=0; n<NOFILE; n++)
		if(u.u_ofile[n] != NULL)
			u.u_ofile[n]->f_count++;
	if(old->p_textp != NULL) {
		old->p_textp->x_count++;
		old->p_textp->x_ccount++;
	}
	u.u_cdir->i_count++;
	if (u.u_rdir)
		u.u_rdir->i_count++;
	/*
	 * Partially simulate the environment
	 * of the new process so that when it is actually
	 * created (by copying) it will look right.
	 */
	u.u_procp = new;
	n = old->p_size;
	a1 = old->p_addr;
	new->p_size = n;
	/*
	 * When the resume is executed for the new process,
	 * here's where it will resume.
	 */
	if (save(u.u_ssav)) {
		sureg();
		return(1);
	}

	if(u.u_fpsaved == 0){
		savfp(&u.u_fps);
		u.u_fpsaved = 1;
	}

	/*
	 * If there is not enough core for the
	 * new process, swap out the current process to generate the
	 * copy.
	 */
	a2 = malloc(coremap, n);
	if(a2 == NULL) {
		old->p_stat = SIDL;
		new->p_addr = a1;
		xswap(new, 0, 0);
		old->p_stat = SRUN;
	} else {
		/*
		 * There is core, so just copy.
		 */
		new->p_addr = a2;
		while(n--)
			copyseg(a1++, a2++);
	}
	u.u_procp = old;
	setrq(new,0);
	new->p_flag |= SSWAP;
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
 *
 * After the expansion, the caller will take care of copying
 * the user's stack towards or away from the data area.
 */
expand(newsize)
{
	register i, n;
	register struct proc *p;
	register a1, a2;

	p = u.u_procp;
	n = p->p_size;
	p->p_size = newsize;
	a1 = p->p_addr;
	if(n >= newsize) {
		mfree(coremap, n-newsize, a1+newsize);
		return;
	}
	if (save(u.u_ssav)) {
		sureg();
		return;
	}
	a2 = malloc(coremap, newsize);
	if(a2 == NULL) {

		if(u.u_fpsaved == 0){
			savfp(&u.u_fps);
			u.u_fpsaved = 1;
		}
		xswap(p, 1, n);
		p->p_flag |= SSWAP;
		qswtch();
		/* no return */
	}
	p->p_addr = a2;
	for(i=0; i<n; i++)
		copyseg(a1+i, a2+i);
	mfree(coremap, n, a1);
	resume(a2, u.u_ssav);
}

/* NICE system call (transplanted here from sys4.c for better modularity) */
/* emulates old NICE by mapping to setblev */
nice()
{
	register int nlev;

	nlev = *(int *) u.u_ap;
	if (nlev < -20 || nlev > 20) {
		u.u_error = EINVAL;
		return;
	}
	if(nlev < 0 && suser() == 0)
		return;
	nlev = (-nlev * RTLEVEL) / 40;	/* -8 to +8 */
	nlev += u.u_procp->p_baslev;	/* -8 to 39 */
	if (nlev < 0)
		nlev = 0;
	else if(nlev > RTLEVEL - 1)
		nlev = RTLEVEL - 1;
	setbaslev(-1, nlev);
	u.u_rval1 = ((RTLEVEL/2 - nlev) * 40)/ RTLEVEL;
}

/*
 * The SETBASLEV routine is the key means of manipulating process
 * levels for the new scheduler.  Our "UNIMAX" system (registered
 * trademark of Hospital Systems Study Group, of course) has a
 * "setblev" system call which simply grabs arguments from user
 * space and calls this SETBASLEV kernel subroutine.  As you can see,
 * the emulation of the NICE system call (above) also uses SETBASLEV.
 */
setbaslev(pid,lev)
register int pid;
register int lev;
{
	register struct proc *p;

	/* normalize new base level */

	if(lev < 0)
		lev = 0;
	else if(lev > MAXLEVEL)
		lev = MAXLEVEL;


	/* locate process */

	if(pid < 0)
		p = u.u_procp;
	else {
		for(p = &proc[0]; p < &proc[NPROC]; p++)
			if(p->p_pid == pid)
				break;

		if(p >= &proc[NPROC]){
			u.u_error = ESRCH;
			return;
		}
	}

	/* can't increase level or zap someone else's process unless root */
	if(((lev > p->p_baslev) || (p->p_uid != u.u_uid)) && !suser())
		return;

	/* OK, set everything appropriately */
	p->p_baslev = lev;
	if(p->p_level < lev){
		p->p_level = lev;
		if(p == u.u_procp)
			curlevel = lev;
	}
}
