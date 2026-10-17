#include "param.h"
#include "systm.h"
#include "user.h"
#include "proc.h"

#define	UMODE	01<<8	/*** 'USER MODE' IN PSW == PROT BIT ***/
#define	SCHMAG	(HZ/6)	/*** was 10 -- proportional to HZ? ***/

/*
 * clock is called straight from
 * the real time clock interrupt.
 *
 * Functions:
 *	reprime clock
 *	copy *switches to display
 *	implement callouts
 *	maintain user/system times
 *	maintain date
 *	profile
 *	tout wakeup (sys sleep)
 *	lightning bolt wakeup (every 4 sec)
 *	alarm clock signals
 *	jab the scheduler
 */

clock(dev, stat, r0, r1, r2, r3, r4, r5, r6, asp, rf, ps, pc, nps)
{
	register struct callo *p1, *p2;
	register struct proc *pp;
	static clkflag;		/*** used to prevent reentry ***/
	register pri;		/***/

	/*
	 * display register
	 */

	display(pc);

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
	 * if lbolt being processed already, just return
	 */

	if (clkflag)
		goto out;

	/*
	 * callout
	 */

	spl(5);
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
	if((ps&UMODE) == UMODE) {
		u.u_utime++;
		if(u.u_prof[3])
			incupc(pc, u.u_prof);
	} else
		u.u_stime++;
	pp = u.u_procp;
	if(++pp->p_cpu == 0)
		pp->p_cpu--;
	if(++lbolt >= HZ) {
		if (clkflag)
			return;
		clkflag++;
		lbolt =- HZ;
		if(++time[1] == 0)
			++time[0];
/***/		spl(0);
		if(time[1]==tout[1] && time[0]==tout[0])
			wakeup(tout);
		if((time[1]&03) == 0) {
			runrun++;
			wakeup(&lbolt);
		}
		for(pp = &proc[0]; pp < &proc[NPROC]; pp++)
		if (pp->p_stat) {
			if(pp->p_time != 127)
				pp->p_time++;
			if((pp->p_cpu & 0377) > SCHMAG)
				pp->p_cpu =- SCHMAG; else
				pp->p_cpu = 0;
			if ((pri = pp->p_pri) > PUSER && pri <= 127) {	/***/
				setpri(pp);
				if ((pri = pp->p_pri) > 127)	/***/
					pri =- 256;		/***/
				/*
				 * check whether other process is now
				 * more deserving
				 */
				if (pp->p_stat == SRUN && pri < curpri) /***/
					runrun++;			/***/
			}
		}
		if(runin!=0) {
			runin = 0;
			wakeup(&runin);
		}
		if((ps&UMODE) == UMODE) {
			clkflag = 0;
			u.u_ar0 = &r0;
			if(issig())
				psig();
			setpri(u.u_procp);
		}
		clkflag = 0;
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
	p1 = &callout[0];
	s = spl(7);
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
	spl(s);
}
