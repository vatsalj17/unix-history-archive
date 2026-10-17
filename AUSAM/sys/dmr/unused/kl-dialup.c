#
/*
#ifndef unsw_orig
 *	Copyright 1973 Bell Telephone Laboratories Inc
#endif
 */

/*
 *   KL/DL-11 driver
 */
#include "../param.h"
#include "../conf.h"
#include "../user.h"
#include "../tty.h"
#include "../proc.h"

/* base address */
#define	KLADDR	0177560	/* console */
#define	KLBASE	0176500	/* kl and dl11-a */
#define	DLBASE	0175610	/* dl-e */
#define	NKL11	1
#ifndef unsw_orig
#define	NDL11	2
#define DSC	0100000
#define RING    040000
#define CARRIER 010000
#define ANSR    04
#endif
#ifdef unsw_orig
#define	NDL11	0
#endif
#define DSRDY	02
#define	RDRENB	01

struct	tty kl11[NKL11+NDL11];

struct klregs {
	int klrcsr;
	int klrbuf;
	int kltcsr;
	int kltbuf;
#ifndef unsw_orig
};
#endif
#ifdef unsw_orig
}
#endif

#ifndef unsw_orig
	dialup[NKL11+NDL11]   { 0,1,1 };
/*
				  0-1-2	
*/

#endif
klopen(dev, flag)
{
	register char *addr;
	register struct tty *tp;

	if(dev.d_minor >= NKL11+NDL11) {
		u.u_error = ENXIO;
		return;
	}
	tp = &kl11[dev.d_minor];
	if (u.u_procp->p_ttyp == 0) {
		u.u_procp->p_ttyp = tp;
		tp->t_dev = dev;
	}
	/*
	 * set up minor 0 to address KLADDR
	 * set up minor 1 thru NKL11-1 to address from KLBASE
	 * set up minor NKL11 on to address from DLBASE
	 */
	addr = KLADDR + 8*dev.d_minor;
	if(dev.d_minor)
		addr =+ KLBASE-KLADDR-8;
	if(dev.d_minor >= NKL11)
		addr =+ DLBASE-KLBASE-8*NKL11+8;
	tp->t_addr = addr;
#ifndef unsw_orig
	if(tp->t_state&ISOPEN)
		return;
	tp->t_flags = XTABS|ECHO|CRMOD;
	tp->t_erase = CERASE;
	tp->t_kill = CKILL;
	tp->t_state = WOPEN;
	spl4();
	addr->kltcsr =| IENABLE;
	addr->klrcsr =| IENABLE|040;
	if (dialup[dev.d_minor]) {
		klring(addr);
		if(addr->klrcsr&CARRIER)
			tp->t_state =| CARR_ON;
		while ((tp->t_state & CARR_ON) == 0)
			sleep(&tp->t_rawq,TTIPRI);
#endif
#ifdef unsw_orig
	if ((tp->t_state&ISOPEN) == 0) {
		tp->t_state = ISOPEN|CARR_ON;
		tp->t_flags = XTABS|LCASE|ECHO|CRMOD;
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
#endif
	}
#ifndef unsw_orig
	tp->t_state =& ~WOPEN;
	tp->t_state =| CARR_ON | ISOPEN;
	addr->klrcsr =| RDRENB;
	spl0();
#endif
#ifdef unsw_orig
	addr->klrcsr =| IENABLE|DSRDY|RDRENB;
	addr->kltcsr =| IENABLE;
#endif
}

klclose(dev)
{
	register struct tty *tp;

	tp = &kl11[dev.d_minor];
#ifndef unsw_orig
if(tp->t_flags&HUPCL)
		tp->t_addr->klrcsr =& ~(ANSR|DSRDY);
#endif
	wflushtty(tp);
	tp->t_state = 0;
}

klread(dev)
{
	ttread(&kl11[dev.d_minor]);
}

klwrite(dev)
{
	ttwrite(&kl11[dev.d_minor]);
}

klxint(dev)
{
	register struct tty *tp;

	tp = &kl11[dev.d_minor];
	ttstart(tp);
#ifdef	unsw_orig
	if (tp->t_outq.c_cc == 0 || tp->t_outq.c_cc == TTLOWAT){
#endif
#ifndef	unsw_orig
	if (tp->t_outq.c_cc <= TTLOWAT && tp->t_state & ASLEEP )  {
		tp->t_state =& ~ASLEEP;
#endif
		wakeup(&tp->t_outq);
	}
}

klrint(dev)
{
	register int c, *addr;
	register struct tty *tp;

	tp = &kl11[dev.d_minor];
	addr = tp->t_addr;
	c = addr->klrbuf;
	addr->klrcsr =| RDRENB;
#ifndef unsw_orig
	if (dialup[dev.d_minor]) {
		klring(addr);

		if(tp->t_state&WOPEN && addr->klrcsr&CARRIER) {
			tp->t_state =| CARR_ON;
			wakeup(&tp->t_rawq);
			return;
		}
		if((addr->klrcsr & CARRIER) == 0) {
			if((tp->t_state&WOPEN) == 0) {
				if(tp->t_state & CARR_ON) {
					signal(tp, SIGHUP);
					addr->klrcsr = IENABLE|040;
				}
				flushtty(tp);
			}
		tp->t_state =& ~CARR_ON;
		return;
		}
	}

#endif
	if ((c&0177)==0)
		addr->kltbuf = c;	/* hardware botch */
	ttyinput(c, tp);
}

klsgtty(dev, v)
int *v;
{
	register struct tty *tp;

	tp = &kl11[dev.d_minor];
	ttystty(tp, v);
}
#ifndef unsw_orig

klcheck(addr)
register struct klregs *addr;
{
	if((addr->klrcsr&CARRIER)==0)
		addr->klrcsr =& ~DSRDY;
}

klring(addr)
register struct klregs *addr;
{
	if(addr->klrcsr&CARRIER)
			return;
	if(addr->klrcsr&RING) {
		addr->klrcsr =| DSRDY|ANSR;
		timeout(&klcheck, addr, 30*HZ);
	} else
		addr->klrcsr =& ~DSRDY;
}
#endif
