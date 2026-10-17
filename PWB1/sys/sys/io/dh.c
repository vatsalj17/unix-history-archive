#
/*
 *	DH11 & DM11-BB driver for multiple units
 */

#include "../hd/param.h"
#include "../hd/user.h"
#include "../hd/tty.h"

extern	dh_addr[], dh_cnt;
struct tty dh_tty[];
struct ttydma dh_ttydma[];
int	dhsar[8];

#define	BITS6	01
#define	BITS7	02
#define	BITS8	03
#define	TWOSB	04
#define	PENABLE	020
#define	OPAR	040
#define	HDUPLX	040000

#define	IENABLE	030100
#define	PERROR	010000
#define	FRERROR	020000
#define	XINT	0100000
#define	SSPEED	7	/* standard speed: 300 baud */

struct {
	int dhcsr, dhnxch, dhlpr, dhcar;
	int dhbcr, dhbar, dhbreak, dhsilo;
};

int	dm_addr[];

#define	DONE	0200
#define	SCENABL	040
#define	BSY	020
#define	TURNON	07	/* RTS, DTR, line enable */
#define	TURNOFF	1	/* line enable only */
#define	CARRIER	0140

struct {
	int	dmcsr, dmlstat;
};

dhopen(dev, flag)
{
	register struct tty *tp;
	extern dhstart();

	if (dev >= dh_cnt) {
		u.u_error = ENXIO;
		return;
	}
	tp = &dh_tty[dev];
	if ((tp->t_state&(ISOPEN|WOPEN)) == 0) {
		tp->t_addr = dhstart;
		tp->t_state = SSTART;
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
		tp->t_speeds = SSPEED | (SSPEED<<8);
		tp->t_flags = ODDP|EVENP|ECHO|HUPCL;
		dhparam(dev);
	}
	spl4();
	if (dmctl(dev, TURNON)&CARRIER)
		tp->t_state =| CARR_ON;
	while ((tp->t_state&CARR_ON)==0) {
		tp->t_state =| WOPEN;
		sleep(&tp->t_rawq, TTIPRI);
	}
	ttyopen(tp);
	spl0();
}

dhclose(dev)
{
	register struct tty *tp;

	tp = &dh_tty[dev];
	wflushtty(tp);
	if (tp->t_flags&HUPCL)
		dmctl(dev, TURNOFF);
	tp->t_state =& (CARR_ON|SSTART);
}

dhread(dev)
{
	ttread(&dh_tty[dev]);
}

dhwrite(dev)
{
	ttwrite(&dh_tty[dev]);
}

dhrint(dev)
{
	register struct tty *tp;
	register int c, *dhaddr;

	dhaddr = dh_addr[dev];
	while ((c = dhaddr->dhnxch) < 0) {	/* char. present */
		tp = &dh_tty[((c>>8)&017)|(dev<<4)];
		if (tp >= &dh_tty[dh_cnt])
			continue;
		if((tp->t_state&ISOPEN)==0) {
			wakeup(&tp->t_rawq);
			continue;
		}
		if (c&FRERROR)		/* break */
			if (tp->t_flags&RAW)
				c = 0;		/* null (for getty) */
			else
				c = 0177;	/* DEL (intr) */
		if (c&PERROR)
			if ((tp->t_flags&(EVENP|ODDP))==EVENP
			 || (tp->t_flags&(EVENP|ODDP))==ODDP )
				continue;
		ttyinput(c, tp);
	}
}

dhsgtty(dev, av)
int *av;
{
	register struct tty *tp;

	tp = &dh_tty[dev];
	if (ttystty(tp, av))
		return;
	dhparam(dev);
}

/*
 * Set parameters from open or stty into the DH hardware
 * registers.
 */
dhparam(dev)
{
	register struct tty *tp;
	register int lpr, *dhaddr;

	tp = &dh_tty[dev];
	dhaddr = dh_addr[dev>>4];
	spl5();
	dhaddr->dhcsr =& ~017;
	dhaddr->dhcsr =| (dev&017) | IENABLE;
	if (tp->t_speeds.lobyte==0) {	/* Hang up line */
		dhaddr->dhbcr = 0;
		spl0();
		dmctl(dev, TURNOFF);
		return;
	}
	lpr = (tp->t_speeds.hibyte<<10) | (tp->t_speeds.lobyte<<6);
	if (tp->t_speeds.lobyte == 4)		/* 134.5 baud */
		lpr =| BITS6|PENABLE|HDUPLX;
	else if (tp->t_flags&RAW)
		lpr =| BITS8;
	else
		lpr =| BITS7|PENABLE;
	if ((tp->t_flags&EVENP)==0)
		lpr =| OPAR;
	if (tp->t_speeds.lobyte == 3)	/* 110 baud */
		lpr =| TWOSB;
	dhaddr->dhlpr = lpr;
	spl0();
}

/*
 * DH11 transmitter interrupt.
 * Restart each line which used to be active but has
 * terminated transmission since the last interrupt.
 */
dhxint(dev)
{
	register struct tty *tp;
	register ttybit, bar;
	int *dhaddr;

	dhaddr = dh_addr[dev];
	bar = dhsar[dev] & ~dhaddr->dhbar;
	dhaddr->dhcsr =& ~XINT;
	ttybit = 1;
	for (tp = &dh_tty[dev<<4]; bar; tp++) {
		if(bar&ttybit) {
			dhsar[dev] =& ~ttybit;
			bar =& ~ttybit;
			tp->t_state =& ~BUSY;
			dhstart(tp);
		}
		ttybit =<< 1;
	}
}

/*
 * Start (restart) transmission on the given DH11 line.
 */
dhstart(atp)
struct tty *atp;
{
	extern ttrstrt();
	register c, nch;
	register struct tty *tp;
	int sps, dev;
	char *cp;
	int line, *dhaddr;

	sps = PS->integ;
	spl5();
	tp = atp;
	dev = tp-dh_tty;
	/*
	 * If it's currently active, or delaying,
	 * no need to do anything.
	 */
	if (tp->t_state&(TIMEOUT|BUSY))
		goto out;
	/*
	 * t_char is a delay indicator which may have been
	 * left over from the last start.
	 * Arrange for the delay.
	 */
	if (c = tp->t_char) {
		tp->t_char = 0;
		timeout(ttrstrt, tp, (c&0177)+6);
		tp->t_state =| TIMEOUT;
		goto out;
	}
	cp = &dh_ttydma[dev];
	nch = 0;
	/*
	 * Copy TTYDMA characters, or up to a delay indicator,
	 * to the DMA area.
	 */
	while (nch > -(sizeof (struct ttydma)) && (c = getc(&tp->t_outq))>=0) {
		if (c >= 0200 && (tp->t_flags&RAW)==0) {
			tp->t_char = c;
			break;
		}
		*cp++ = c;
		nch--;
	}
	/*
	 * If the writer was sleeping on output overflow,
	 * wake him when low tide is reached.
	 */
	if (tp->t_outq.c_cc<=TTLOWAT && tp->t_state&ASLEEP) {
		tp->t_state =& ~ASLEEP;
		wakeup(&tp->t_outq);
	}
	/*
	 * If any characters were set up, start transmission;
	 * otherwise, check for possible delay.
	 */
	if (nch) {
		dhaddr = dh_addr[dev>>4];
		line = dev & 017;
		dhaddr->dhcsr.lobyte = line | IENABLE;
		dhaddr->dhcar = cp+nch;
		dhaddr->dhbcr = nch;
		c = 1<<line;
		dhaddr->dhbar =| c;
		dhsar[dev>>4] =| c;
		tp->t_state =| BUSY;
	} else if (c = tp->t_char) {
		tp->t_char = 0;
		timeout(ttrstrt, tp, (c&0177)+6);
		tp->t_state =| TIMEOUT;
	}
    out:
	PS->integ = sps;
}

/*
 * Dump control bits into the DM registers.
 */
dmctl(dev, bits)
register dev;
{
	register *dmaddr;

	dmaddr = dm_addr[dev>>4];
	dmaddr->dmcsr =& ~SCENABL;
	while(dmaddr->dmcsr&BSY);
	dmaddr->dmcsr = dev&017;
	dmaddr->dmlstat = bits;
	dev = dmaddr->dmlstat;
	dmaddr->dmcsr =| IENABLE|SCENABL;
	return(dev);
}

/*
 * DM11 interrupt.
 * Mainly, deal with carrier transitions.
 */
dmintr(dev)
{
	register struct tty *tp;
	register *dmaddr;

	dmaddr = dm_addr[dev];
	if (dmaddr->dmcsr&DONE) {
		tp = &dh_tty[(dmaddr->dmcsr&017)|(dev<<4)];
		if (tp < &dh_tty[dh_cnt]) {
			wakeup(&tp->t_rawq);
			if ((dmaddr->dmlstat&CARRIER)==0) {
				if ((tp->t_state&WOPEN)==0) {
					signal(tp->t_pgrp, SIGHUP);
					dmaddr->dmlstat = 0;
					flushtty(tp);
				}
				tp->t_state =& ~CARR_ON;
			} else
				tp->t_state =| CARR_ON;
		}
		dmaddr->dmcsr =& ~DONE;
	}
}
