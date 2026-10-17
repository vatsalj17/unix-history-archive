#
#include "param.h"
#include "systm.h"
#include "user.h"
#include "proc.h"

/*
 * clock is called straight from
 * the real time clock interrupt.
 *
 * Functions:
 *	reprime clock
 *	implement callouts
 *	maintain user/system times
 *	maintain date
 *	profile
 *	tout wakeup (sys sleep)
 *	lightning bolt wakeup (every 4 sec)
 *	alarm clock signals
 *	jab the scheduler
 */
clock(dev, sp, r1, nps, r0, pc, ps)
char *pc;
{
	register struct callo *p1, *p2;
	register struct proc *pp;
	int a;

	/*
	 * restart clock
	 */

	*lks = 0115;

	/*
	 * callouts
	 * if none, just return
	 * else update first non-zero time
	 */

	if(callout[0].c_func == 0)
		goto out;
	p2 = &callout[0];
	while(p2->c_time<=0 && p2->c_func!=0)
		p2++;
	p2->c_time--;

	/*
	 * if ps is high, just return
	 */

	if((ps&0340) != 0)
		goto out;

	/*
	 * callout
	 */

	spl5();
	if(callout[0].c_time <= 0) {
		p1 = &callout[0];
		while(p1->c_func != 0 && p1->c_time <= 0) {
			(*p1->c_func)(p1->c_arg);
			p1++;
		}
		p2 = &callout[0];
		while(p2->c_func = p1->c_func) {
			p2->c_time = p1->c_time;
			p2->c_arg = p1->c_arg;
			p1++;
			p2++;
		}
	}

	/*
	 * lightning bolt time-out
	 * and time of day
	 */

out:
	if(pc > TOPSYS)
		u.u_utime++;
	else
		u.u_stime++;
	pp = u.u_procp;
	if(++lbolt >= HZ) {
		if((ps&0340) != 0)
			return;
		lbolt =- HZ;
		if(++time[1] == 0)
			++time[0];
		spl1();
		if(time[1]==tout[1] && time[0]==tout[0])
			wakeup(tout);
		if((time[1]&01) == 0) {
			runrun++;
		}
		for(pp = &proc[0]; pp < &proc[NPROC]; pp++)
		if (pp->p_stat) {
			if(pp->p_clktim)
				if(--pp->p_clktim == 0)
					psignal(pp, SIGCLK);
		}
		if(pc > TOPSYS) {
			u.u_ar0 = &r0;
			if(issig())
				psig();
		}
	}
}

/*
 * timeout is called to arrange that
 * fun(arg) is called in tim/HZ seconds.
 * An entry is sorted into the callout
 * structure. The time in each structure
 * entry is the number of HZ's more
 * than the previous entry.
 * In this way, decrementing the
 * first entry has the effect of
 * updating all entries.
 */
timeout(fun, arg, tim)
{
	register struct callo *p1, *p2;
	register t;
	int s;

	t = tim;
	s = PS->integ;
	p1 = &callout[0];
	spl7();
	while(p1->c_func != 0 && p1->c_time <= t) {
		t =- p1->c_time;
		p1++;
	}
	p1->c_time =- t;
	p2 = p1;
	while(p2->c_func != 0)
		p2++;
	while(p2 >= p1) {
		(p2+1)->c_time = p2->c_time;
		(p2+1)->c_func = p2->c_func;
		(p2+1)->c_arg = p2->c_arg;
		p2--;
	}
	p1->c_time = t;
	p1->c_func = fun;
	p1->c_arg = arg;
	PS->integ = s;
}
