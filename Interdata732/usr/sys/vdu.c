#

#include "param.h"
#include "conf.h"
#include "tty.h"
#include "user.h"
#include "proc.h"


/*
 *		Vdu driver -- local terminal on PALS
 *
 */

#define	NVDU	16		/* number of local terminals */

int	vdubase	0x10;		/* lowest PALS address */
extern char	conscmd2;	/* PALS command 2 for 'console' */

struct tty vdu[NVDU];

int vdustart(), vdurint(), vduxint(), ttrstrt();

/*
 * PALS commands & status bits
 */

	/* command 1 */
#define	DIS	0200
#define	EN	0100
#define	DTR	0040
#define	WRT	0002
#define	CMD1	0001

	/* command 2 */
#define	CLKA	0000
#define	CLKB	0100
#define	CLKC	0200
#define	CLKD	0300
#define	CMD2	0070

	/* Status */
#define	CARR_OFF	0002
#define	BSY		0010

/*
 * open routine:
 *	called each time a process opens a terminal as a character file
 *
 *	- if the terminal was previously inactive, set up the initial status
 *	  and arm interrupts
 */
vduopen(dev)
{
	register struct tty *tp;

	if (dev.d_minor >= NVDU) {
		u.u_error = ENXIO;
		return;
	}
	tp = &vdu[dev.d_minor];
	tp->t_dev = dev;
	if ((tp->t_state& ISOPEN) == 0) {
		tp->t_state = ISOPEN | SSTART;
		if (tp->t_flags == 0)
			tp->t_flags = XTABS | LCASE | CRMOD | ECHO;
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
		tp->t_addr = vdustart;

		vduenab(tp);
	}
	spl(4);
	while (!(tp->t_state&CARR_ON))
		sleep(&tp->t_rawq, TTIPRI);
	spl(0);
	if (u.u_procp->p_ttyp == 0)
		u.u_procp->p_ttyp = tp;
}

/*
 * close routine:
 *	- called only when last process using terminal releases it
 */
vduclose(dev)
{
	register struct tty *tp;
	tp = &vdu[dev.d_minor];
	wflushtty(tp);
	tp->t_state = 0;
	vdudisab(dev);
}

/*
 * read, write, stty, gtty routines:
 *	- call standard tty routines
 */
vduread(dev)
{
	ttread(&vdu[dev.d_minor]);
}

vduwrite(dev)
{
	ttwrite(&vdu[dev.d_minor]);
}

vdusgtty(dev, v)
{
	register struct tty *tp;

	tp = &vdu[dev.d_minor];
	ttystty(tp, v);

	/* Stty - reissue CMD 2 to change baud rate */
	if (v == 0)
		vduenab(tp);
}

/*
 * vdustart routine - called when there might be something to send
 *   to the terminal
 */

vdustart(atp)
struct tty *atp;
{
	register struct tty *tp;
	register c, waddr;

	tp = atp;
	waddr = 2*tp->t_dev + vdubase + 1;
	trace(040, "vdustart", waddr);
	if ((tp->t_state&(TIMEOUT|BUSY|SUSPEND|CARR_ON)) != CARR_ON
	    || (c = getc(&tp->t_outq)) < 0)
		return;
	if (c > 0177) {
		tp->t_state =| TIMEOUT;
		timeout(ttrstrt, tp, c&0177);
	} else {
		tp->t_state =| BUSY;
		c =| partab[c]&0200;
		wd(waddr, c);
		trace(0100, "wrt", ss(waddr));
	}
}

vdurint(raddr, stat)
{
	register struct tty *tp;
	register char c;

	tp = &vdu[(raddr-vdubase)>>1];

	if (!(tp->t_state & ISOPEN))
		return;
	trace(0100, "rint", raddr);
	trace(0100, "stat", stat);

	c = rd(raddr)&0177;
	trace(0100, "char", c);

	if (stat & CARR_OFF) {
		if (tp->t_state & CARR_ON) {
			signal(tp, SIGHUP);
			flushtty(tp);
		}
		tp->t_state =& ~CARR_ON;
		return;
	}
	if (!(tp->t_state & CARR_ON)) {
		tp->t_state =| CARR_ON;
		wakeup(&tp->t_rawq);
		return;
	}

	/*
	 * Carousel DC4 / DC2 (suspend / resume) signals
	 */
	if (c == '\024' && !(tp->t_flags&RAW))
		tp->t_state =| SUSPEND;
	else if (c == '\022' && !(tp->t_flags&RAW)) {
		if (tp->t_state&SUSPEND) {
			tp->t_state =& ~SUSPEND;
			wakeup(&tp->t_outq);
			ttstart(tp);
		}
	} else
		ttyinput(c, tp);
}

vduxint(waddr, stat)
{
	register struct tty *tp;
	register c;

	tp = &vdu[(waddr-vdubase)>>1];
	if (!(tp->t_state & ISOPEN))
		return;
	trace(0100, "wint", waddr);
	trace(0100, "stat", stat);
	tp->t_state =& ~BUSY;
	ttstart(tp);

	if (tp->t_outq.c_cc <= TTLOWAT && (tp->t_state&ASLEEP)) {
		tp->t_state =& ~ASLEEP;
		wakeup(&tp->t_outq);
	}
}

/*
 * Arm interrupts from PALS and set baud rate
 */
vduenab(atp)
struct tp *atp;
{
	register struct tp *tp;
	register radd, wadd;
	register cmd2, stat;

	switch ((tp = atp)->t_speeds&0377) {

	case B300:
		cmd2 = CLKA | CMD2;
		break;

	case B1200:
		cmd2 = CLKB | CMD2;
		break;

	case B2400:
		cmd2 = CLKC | CMD2;
		break;

	case B4800:
		cmd2 = CLKD | CMD2;
		break;

	default:		/* Use low-core 'console' definition */
		cmd2 = conscmd2;
	}

	radd = vdubase + tp->t_dev.d_minor*2;
	wadd = radd + 1;

	oc(radd, cmd2);
	oc(radd, EN | DTR | CMD1);
	oc(wadd, EN | DTR | WRT | CMD1);
	stat = ss(radd);
	if ((stat&CARR_OFF) == 0)
		tp->t_state =| CARR_ON;

	trace(0100, "vduenab", stat);
	rd(radd);
}

/*
 * Disarm interrupts from PALS
 */
vdudisab(dev)
{
	register radd, wadd;

	radd = vdubase + dev.d_minor*2;
	wadd = radd + 1;

	oc(radd, DIS | CMD1);
	oc(wadd, DIS | WRT | CMD1);

	trace(0100, "vdudisab", ss(radd));
}
