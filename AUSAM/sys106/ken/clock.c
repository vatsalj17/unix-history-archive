#
#include "../defines.h"
#include "../param.h"
#include "../file.h"
#ifdef	AUSAML
#include	"../lnode.h"
#endif	AUSAML
#include "../systm.h"
#include "../user.h"
#include "../proc.h"
#ifdef	LRU_INODE
#include "../inode.h"
#endif	LRU_INODE

#ifndef	SETPSW
#define	UMODE	0170000
#endif
#ifdef	SETPSW
#define	UMODE	0140000
#endif
/* #define	SCHMAG	10	/* This is now in param.h */

#ifdef	NEW_TIMEOUT
int		ntimeouts;	/* count of timeouts ( limited by NCALL ) */
struct	timout	*cfreelist;	/* redeclare cblock freelist for use by timeouts */
#endif

#ifndef CBLOCK_16
	int cblockm  007; int cblockl  010;
#endif
#ifdef CBLOCK_16
	int cblockm  017; int cblockl  020;
#endif

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
 *	maintain inode last reference time ( LRU_INODE )
 *	profile
 *	tout wakeup (sys sleep)
 *	lightning bolt wakeup (every 4 sec)
 *	alarm clock signals
 *	jab the scheduler
 */

#ifndef	_1170 | _1145
clock(dev, sp, r1, nps, r0, pc, ps)
#endif
#ifdef	_1170 | _1145
clock(dev, sp, r1, nofault, r0, pc, ps)
#endif
{
#ifdef	NEW_TIMEOUT
	register struct timout  *tp;
	register  (*func)();
	int  arg;
#endif
#ifndef	NEW_TIMEOUT
	register struct callo *p1, *p2;
#endif
	register struct proc *pp;

	/*
	 * restart clock
	 */

#ifndef	PROG_CLOCK
	*lks = 0115;
#endif

	/*
	 * display register
	 */

#ifndef _1140
	display();
#endif

#ifdef	NEW_TIMEOUT

	/*
	 * decrement callout times
	 */

	if ( (tp = timouts.t_next) == 0 )  goto out;

	do
		if ( --tp->t_time >= 0 )  break;
	while
		( tp = tp->t_next );

	/*
	 * skip callouts if ps high
	 */

	if ( ps & 0340 )  goto out;

	/*
	 * callouts
	 */

	while ( (tp = timouts.t_next)->t_time <= 0 )  {
		func = tp->t_func;
		arg = tp->t_arg;
		timouts.t_next = tp->t_next;

		if ( tfreecount < NTFREE )  {
			tfreecount++;
			tp->t_next = tfreelist;
			tfreelist = tp;
		}else  {
			tp->t_next = cfreelist;
			cfreelist = tp;
		}

		ntimeouts--;

		spl5();
			(*func)( arg );
		spl6();
	}

	spl5();
#endif

#ifndef	NEW_TIMEOUT

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
#endif

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
#ifndef	AUSAMSCHD
	if(++pp->p_cpu == 0)
		pp->p_cpu--;
#endif
#ifdef	AUSAMSCHD
	pp-p_cpu++;
	pp->p_shedtime =+ pp->p_size;
	pp->p_lnode->l_usage =+ pp->p_size;
#endif
#ifdef	TIME_LIMITS
	if( u.u_cpusec ){
		if( --u.u_tix== 0 ){
			if( --u.u_cpusec == 0 )
				psignal(u.u_procp, SIGCPUTL);
			u.u_tix = HZ;
		}
	}
#endif
	if(++lbolt >= HZ) {
		if((ps&0340) != 0)
			return;
		lbolt =- HZ;
		time++;	/* fix000 */
		spl1();
#ifdef	DEBUG_SWAP
		tightmap();
#endif	DEBUG_SWAP
#ifdef	LRU_INODE
		/*
		 * On clock overflow zap last inode reference times.
		 */
		if(time.loint==0){
			struct inode *ip;
			for(ip=inode;ip!= &inode[NINODE];ip++) ip->i_lrt=0;
		}
#endif	LRU_INODE
#ifndef	NEW_SLEEP
		if(time==tout) wakeup(&tout);	/* fix000 */
#endif
		runrun++;	/* fix025 swtch every second at least */
		if((time&03) == 0)
			wakeup(&lbolt);
#ifndef	MAX_PROC
		for(pp = &proc[0]; pp < &proc[NPROC]; pp++)
#else
		for(pp = &proc[0]; pp <= max_proc; pp++)
#endif	MAX_PROC
#ifndef	ZOMBIE
		if (pp->p_stat) {
#endif
#ifdef	ZOMBIE
		if( (pp->p_stat) && (pp->p_stat != SZOMB) ) {
#endif
#ifdef	TIME_LIMITS
			if( pp->p_rtl )
				if( --pp->p_rtl == 0 )
					psignal(pp, SIGTIMEOUT);
#endif
#ifdef	NEW_SLEEP
			if( pp->p_stl &&
			    --pp->p_stl == 0 &&
			    pp->p_wchan.integptr == &pp->p_stl)
				setrun(pp);	/* no need for wakeup */
#endif
#ifndef	AUSAMSCHD
			if(pp->p_time != 127)
				pp->p_time++;
			if((pp->p_cpu & 0377) > SCHMAG)
				pp->p_cpu =- SCHMAG; else
				pp->p_cpu = 0;
			if(pp->p_pri > PUSER)
				setpri(pp);
#endif
#ifdef	AUSAMSCHD
			if(pp->p_time)
				pp->p_time--;
			if(pp->p_pri > pp->p_user) {
				setpri(pp);
				pp->p_cpu = 0;
			}
#endif
		}
		if(runin!=0) {
			runin = 0;
			setrun( &proc[0] ); /* no need for wakeup */
		}
		if((ps&UMODE) == UMODE) {
			u.u_ar0 = &r0;
			if(issig())
				psig();
			pp = u.u_procp;				/* fix025 */
			if( pp->p_uid != 0			/* fix025 */
			&&  pp->p_nice < 4			/* fix025 */
			&&  u.u_utime > 30*HZ )			/* fix025 */
				pp->p_nice =+ 10;		/* fix025 */
			setpri( pp );				/* fix025 */
		}
#ifdef	AUSAMSCHD
		if(++sbolt ==  SCHEDTIME) {
			sbolt = 0;
			resched();
		}
#endif
	}
}

#ifdef	NEW_TIMEOUT

/*
 *	New timeout using blocks off the character freelist
 *
 *	Timeout is called to arrange that func( arg ) is
 *	called in tim / HZ seconds.
 *
 *	An entry is sorted into the timeout list so that
 *	updating the first entry has the effect of updating
 *	all entries.
 *
 *	A list of backup blocks is kept in a timeout freelist
 *	for use in the event that there are no character blocks 
 *	free. This list has a high water mark NTFREE.
 *
 *	Timeout returns a value which is a pointer to the block
 *	containing the timeout info. This value is zero in
 *	the event of either there being no blocks available, or
 *	the number of timeouts has exceeded NCALL.
 */

struct timout *timeout( func , arg , tim )
  int (*func)();
{
	register struct timout  *p1, *p2;
	register t;
	int  sps;

  t = tim;
  p1 = &timouts;
  sps = PS->integ;
  spl6();

  if ( ++ntimeouts > NCALL )  {
	p2 = NULL;
	goto fail;
  }

  while ( p2 = p1->t_next )
	if ( t <= p2->t_time )  {  p2->t_time =- t;  break;  }
	else  {
		t =- p2->t_time;
		p1 = p2;
	}

  if ( p2 = cfreelist )  {
	cfreelist = p2->t_next;
found:
	p2->t_next = p1->t_next;
	p1->t_next = p2;
	p2->t_time = t;
	p2->t_func = func;
	p2->t_arg = arg;
  }else
	if ( p2 = tfreelist )  {
		tfreelist = p2->t_next;
		tfreecount--;
		goto found;
	}else  {
		if ( p1->t_next )
			p1->t_next->t_time =+ t;
		toverloadcount++;
fail:
		ntimeouts--;
	}

  PS->integ = sps;
  return( p2 );
}

#endif



#ifndef	NEW_TIMEOUT

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

#endif


#ifdef	NEW_TIMEOUT & UN_TIMEOUT

/*
 *	untimeout - cancel effect of previous call to timeout
 *		    returns "true" if successful
 */

int  untimeout( pp , func , arg )
  register struct timout *pp;	/* value previously returned by timeout */
  int (*func)();		/* function argument previously given to timeout */
  int arg;			/* argument previously given to timeout */
{
	register struct timout  *p2, *p3;
	int  sps;

  if ( !pp )
	return( pp );

  p2 = &timouts;
  sps = PS->integ;
  spl6();

  while ( p3 = p2->t_next )
	if ( pp != p3 )  p2 = p3;
	else  {
		if ( p3->t_func != func || p3->t_arg != arg )  {
			p3 = 0;
			break;
		}
		p2->t_next = p3->t_next;
		if ( tfreecount < NTFREE )  {
			tfreecount++;
			p3->t_next = tfreelist;
			tfreelist = p3;
		}else  {
			p3->t_next = cfreelist;
			cfreelist = p3;
		}
		ntimeouts--;
		break;
	}

  PS->integ = sps;
  return( p3.integ );
}

#endif


#ifdef	DELAY

/*
 * delay goes to sleep on an unique address for a
 * guaranteed minimum period tim/HZ secs.
 */
delay( tim )
  register tim;
{
	register sps;
	register struct timout *tp;
	extern setrun();

  sps = PS->integ;
  spl6();

  while ( (tp = timeout( &setrun , u.u_procp, tim)) == 0 )  {
	tim =- ((4-(time.loint&03))*HZ);
	sleep( &lbolt , PDELAY );	/* PDELAY must be negative */
	if ( tim <= 0 )
		goto out;
  }

  sleep( tp , PDELAY );	/* PDELAY must be negative */
  tim = 0;

out:
  PS->integ = sps;
  return( tim );
}

#endif
#ifdef	AUSAMSCHD
/*
 *	resched....
 *
 *	goes thru the proc array twice in order to bring equity
 *	and order to every process. (I hope)
 */
resched()
{
	register struct proc *rp;
	register i, pu;
	long dif, delta;
	long totis, totshould;
	static int down;	/* need to shift all puser's down */

	totis = totshould = 0;
/*
 * down is used to indicate whether or not pusers are jamming up
 * against an end of the priority scale(PUSER or PLOW);
 */
	if(down) pu = down<0 ? -1 : 1;
	else pu = 0;
/*
 *	scan the proc array getting what was, and what should have been
 */
	spl1();
	for(i=0,rp= &proc[0]; rp < &proc[NPROC]; i++,rp++)
	if(rp->stat) {
		/* what they got was p_schedtime */
		totis =+ rp->p_schedtime;
		/* put what they should have got in scratch */
		totshould =+ (rp->scratch = 10000000L*rp->p_lnode->
#endif
