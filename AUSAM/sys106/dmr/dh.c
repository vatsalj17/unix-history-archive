#
/*
**	DH-11 driver
*/

#include	"../defines.h"
#include	"../param.h"
#include	"../conf.h"
#include	"../user.h"
#include	"../tty.h"
#include	"../proc.h"

/*
**	Configuration definitions
*/

#define	DHADDR	0160060		/* device address */
#define	NDH11	16		/* number of lines */
#define	DHNCH	8		/* max number of DMA chars */
#define	DHSILO	32		/* silo interrupt level must be 0 1 2 4 8 16 or 32 */
#define	DHRATE	2		/* scan rate */
#define	DHSOPEN	0xe0d5		/* single open lines */

/*
**	Tty structures
*/

struct tty	dh11[NDH11];

/*
**	Place from which to do DMA on output
*/

char		dh_clist[NDH11][DHNCH];

/*
**	Miscellaneous Data
*/

int	dhsar;			/* Software copy of last dhbar */
int	dhscanning;		/* Is receiver scanning running */
int	dhopenl;		/* Bit set for each open line */

/*
**	Hardware register layout
*/

struct dhregs
{
	int	dhcsr;
	int	dhnxch;
	int	dhlpr;
	int	dhcar;
	int	dhbcr;
	int	dhbar;
	int	dhbreak;
	int	dhsilo;
};

/*
**	Hardware control bits
*/

#define	BITS6		01
#define	BITS7		02
#define	BITS8		03
#define	TWOSB		04
#define	PENABLE		020
#define	OPAR		040
#define	HDUPLX		040000

#define	IENABLE		0030100
#define	MCLEAR		0004000
#define	PERROR		0010000
#define	FRERROR		0020000
#define	XINT		0100000
#define	SSPEED		13		/* standard speed: 9600 baud */




/*
**	Open a DH11 line.
*/

dhopen(dev, flag)
{
	register struct tty *tp;
	extern dhstart(), dhscan();

	if (dev.d_minor >= NDH11)
	{
		u.u_error = ENXIO;
		return;
	}
	if ( DHSOPEN & dhopenl & (1<<dev.d_minor) )
	{
		u.u_error = EOPENFAIL;
		return;
	}
	tp = &dh11[dev.d_minor];
	spl5();
	if(dhopenl == 0)
	{
		DHADDR->dhcsr = MCLEAR;
		DHADDR->dhcsr = IENABLE;
		DHADDR->dhsilo = DHSILO;
	}
	if ((tp->t_state&ISOPEN) == 0)
	{
		tp->t_addr = dhstart;
		tp->t_dev = dev;
		tp->t_state =| ISOPEN|CARR_ON|SSTART;
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
		tp->t_speeds = SSPEED | (SSPEED<<8);
		tp->t_flags = ODDP|EVENP|ECHO|XTABS|CRMOD;
		dhopenl =| 1<<dev.d_minor;
		dhparam(tp);
	}
	if ( dhscanning == 0 )
		dhscan();
	spl0();
	if (u.u_procp->p_ttyp == 0)
		u.u_procp->p_ttyp = tp;
}

/*
**	Close a DH11 line.
*/
dhclose(dev)
{
	register struct tty *tp;

	tp = &dh11[dev.d_minor];
	wflushtty(tp);
	tp->t_state = SSTART;
	if((dhopenl =& ~(1<<dev.d_minor)) == 0)
		DHADDR->dhcsr = 0;
}

/*
**	Read from a DH11 line.
*/
dhread(dev)
{
	ttread(&dh11[dev.d_minor]);
}

/*
**	write on a DH11 line
*/
dhwrite(dev)
{
	ttwrite(&dh11[dev.d_minor]);
}

/*
**	DH11 input scanning
*/
dhscan()
{
	register struct tty *	tp;

	if(dhopenl)
	{
		dhscanning = timeout(dhscan, 0, DHRATE);
		dhrint();
		for ( tp = dh11 ; tp < &dh11[NDH11] ; tp++ )
			if ( (tp->t_state & TIMEOUT) && --tp->t_char <= 0 )
			{
				tp->t_char = 0;
				tp->t_state =& ~TIMEOUT;
				dhstart(tp);
			}
	}
	else
		dhscanning = 0;
}

/*
**	DH11 receiver interrupt.
*/
dhrint()
{
	register struct tty *tp;
	register int c;

	while ((c = DHADDR->dhnxch) < 0)
	{	/* char. present */
		tp = &dh11[(c>>8)&017];
		if((tp->t_state&ISOPEN)==0 || (c&PERROR))
		{
			wakeup(tp);
			continue;
		}
		if (c&FRERROR)		/* break */
			if (tp->t_flags&RAW)
				c = 0;		/* null (for getty) */
			else
				c = 0177;	/* DEL (intr) */
		if ( c & PERROR )	/* parity */
			continue;	/* ignored */
		ttyinput(c, tp);
	}
}

/*
**	stty/gtty for DH11
*/

dhsgtty(dev, av)
int *av;
{
	register struct tty *tp;
	register r;

	tp = &dh11[dev.d_minor];
	if (ttystty(tp, av))
		return;
	spl5();
	dhparam(tp);
	spl0();
}

/*
**	Set parameters from open or stty into the DH hardware
**	registers.
*/

dhparam(atp)
struct tty *atp;
{
	register struct tty *tp;
	register int lpr;

	tp = atp;
	lpr = (tp->t_speeds.hibyte<<10) | (tp->t_speeds.lobyte<<6);
	if (tp->t_flags&EVENP)
		if (tp->t_flags&ODDP)
			lpr =| BITS8;
		else
			lpr =| BITS7|PENABLE;
	else
		if ( tp->t_flags&ODDP )
			lpr =| BITS7|OPAR|PENABLE;
		else
			lpr =| BITS8;
	if (tp->t_speeds.lobyte == 3)	/* 110 baud */
		lpr =| TWOSB;
	delay(5);	/* delay while controller flushes */
	DHADDR->dhcsr.lobyte = tp->t_dev.d_minor | IENABLE;
	DHADDR->dhlpr = lpr;
}

/*
**	DH11 transmitter interrupt.
**	Restart each line which used to be active but has
**	terminated transmission since the last interrupt.
*/

dhxint()
{
	register struct tty *tp;
	register ttybit, bar;

	bar = dhsar & ~DHADDR->dhbar;
	DHADDR->dhcsr =& ~XINT;
	ttybit = 1;
	for (tp = dh11; bar; tp++)
	{
		if(bar&ttybit)
		{
			dhsar =& ~ttybit;
			bar =& ~ttybit;
			tp->t_state =& ~BUSY;
			dhstart(tp);
		}
		ttybit =<< 1;
	}
}

/*
**	Start (restart) transmission on the given DH11 line.
*/

dhstart(tp)
	register struct tty *tp;
{
	extern ttrstrt();
	register c, nch;
	char *cp;

	/*
	**	If it's currently active, or delaying,
	**	no need to do anything.
	*/
	if (tp->t_state&(TIMEOUT|BUSY))
		return;
	/*
	**	t_char is a delay indicator which may have been
	**	left over from the last start.
	**	Arrange for the delay.
	*/
	if (c = tp->t_char)
	{
		tp->t_char = (((c&0177)+(DHRATE-1))/DHRATE)+1;
		tp->t_state =| TIMEOUT;
		return;
	}
	cp = dh_clist[tp->t_dev.d_minor];
	nch = 0;
	/*
	**	Copy DHNCH characters, or up to a delay indicator,
	**	to the DMA area.
	*/
	while (nch > -DHNCH && (c = getc(&tp->t_outq))>=0)
	{
		if (c >= 0200 && tp->t_flags != RAW)
		{
			tp->t_char = c;
			break;
		}
		*cp++ = c;
		nch--;
	}
	/*
	**	If the writer was sleeping on output overflow,
	**	wake him when low tide is reached.
	*/
	if (tp->t_outq.c_cc<=TTLOWAT && tp->t_state&ASLEEP)
	{
		tp->t_state =& ~ASLEEP;
		wakeup(&tp->t_outq);
	}
	/*
	**	If any characters were set up, start transmission;
	**	otherwise, check for possible delay.
	*/
	if (nch)
	{
		DHADDR->dhcsr.lobyte = tp->t_dev.d_minor | IENABLE;
		DHADDR->dhcar = cp+nch;
		DHADDR->dhbcr = nch;
		c = 1<<tp->t_dev.d_minor;
		DHADDR->dhbar =| c;
		dhsar =| c;
		tp->t_state =| BUSY;
	}
	else
	if (c = tp->t_char)
	{
		tp->t_char = (((c&0177)+(DHRATE-1))/DHRATE)+1;
		tp->t_state =| TIMEOUT;
	}
}
