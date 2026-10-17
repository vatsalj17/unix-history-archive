#
/*
 *   KL/DL-11 driver
 */
#include "../hd/param.h"
#include "../hd/user.h"
#include "../hd/tty.h"

extern	kl_addr[], kl_cnt;
struct tty kl_tty[];
#define DSRDY	02
#define	RDRENB	01

struct klregs {
	int klrcsr, klrbuf;
	int kltcsr, kltbuf;
};

klopen(dev, flag)
{
	register char *addr;
	register struct tty *tp;

	if(dev >= kl_cnt) {
		u.u_error = ENXIO;
		return;
	}
	tp = &kl_tty[dev];
	addr = kl_addr[dev];
	tp->t_addr = addr;
	if ((tp->t_state&ISOPEN) == 0) {
		tp->t_state = ISOPEN|CARR_ON;
		tp->t_flags = XTABS|ECHO|CRMOD;
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
	}
	addr->klrcsr =| IENABLE|DSRDY|RDRENB;
	addr->kltcsr =| IENABLE;
	ttyopen(tp);
}

klclose(dev)
{
	register struct tty *tp;

	tp = &kl_tty[dev];
	wflushtty(tp);
	tp->t_state = 0;
}

klread(dev)
{
	ttread(&kl_tty[dev]);
}

klwrite(dev)
{
	ttwrite(&kl_tty[dev]);
}

klxint(dev)
{
	register struct tty *tp;

	tp = &kl_tty[dev];
	ttstart(tp);
	if (tp->t_outq.c_cc == 0 || tp->t_outq.c_cc == TTLOWAT)
		wakeup(&tp->t_outq);
}

klrint(dev)
{
	register int c, *addr;
	register struct tty *tp;

	tp = &kl_tty[dev];
	addr = tp->t_addr;
	c = addr->klrbuf;
	addr->klrcsr =| RDRENB;
	if ((c&0177)==0)
		addr->kltbuf = c;	/* hardware botch */
	ttyinput(c, tp);
}

klsgtty(dev, v)
int *v;
{
	register struct tty *tp;

	tp = &kl_tty[dev];
	ttystty(tp, v);
}
