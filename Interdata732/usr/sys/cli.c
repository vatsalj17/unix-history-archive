#

/*
 *	Current Loop Interface Driver
 *
 */

#include "param.h"
#include "conf.h"
#include "proc.h"
#include "tty.h"
#include "user.h"

#define NCLI	1	/* number of terminals */

char	cliaddr[NCLI] {		/* cli addresses */
	02
};

struct tty cli[NCLI];	/* common tty structure */

#define devmap(addr)	&cli[addr-02]	/* map phys addr => tty */

/* Current Loop Interface Command and Status Bits */

	/* commands */

#define	DISABLE	0200
#define	ENABLE	0100
#define	UNBLOCK	0040
#define	BLOCK	0020
#define	WRITE	0010
#define	READ	0004

	/* status */

#define	OV	0200
#define	BRK	0040
#define	BSY	0010
#define	EX	0004
#define	DU	0001

#define	WINIT	WOPEN		/* waiting to change mode read->write */

/*
 * open routine:
 *	called each time a process opens a terminal as a character file
 *
 *	- if the terminal was previously inactive, set up the initial status
 *	  and arm interrupts
 */
cliopen(dev,flag)
{
	register struct tty *tp;
	int clistart();

	if (dev.d_minor >= NCLI) {	/* minor device number too large ? */
		u.u_error = ENXIO;	/* yes, return error */
		return;
	}
	tp = &cli[dev.d_minor];		/* get tty struct for terminal */
	tp->t_dev = dev;		/* set device number in tty struct */
	if ((tp->t_state & ISOPEN) == 0) {
		tp->t_state = ISOPEN | CARR_ON | SSTART;
		if (tp->t_flags == 0)
			tp->t_flags = XTABS | LCASE | CRMOD | ECHO;
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
         	tp->t_addr = &clistart;  /* special output start routine */

		cliparam(tp);		/* enable read and set parms */
		trace(0x1000,"cliopen rd:",rd(cliaddr[dev.d_minor]));
	}
	if (u.u_procp->p_ttyp == 0)	/* give a process to the tty */
		u.u_procp->p_ttyp = tp;
}


/*
 * close routine:
 *	- called only when last process using terminal releases it
 */
cliclose (dev)
{
	register struct tty *tp;

	tp = &cli[dev.d_minor];		/* find tty struct for terminal */
	wflushtty(tp);			/* flush queues */
	tp->t_state = 0;		/* clear status */
	oc(cliaddr[dev.d_minor], DISABLE | READ | BLOCK);	/* disable read */
}

/*
 * read, write, stty, gtty routines:
 *	- call standard tty routines
 */
cliread(dev)
{
	ttread(&cli[dev.d_minor]);
}

cliwrite(dev)
{
	ttwrite(&cli[dev.d_minor]);
}

clisgtty(dev, v)
{

	register struct tty *tp;
	tp = &cli[dev.d_minor];
	ttystty(tp, v);
	if ( (v==0) && !(tp->t_state & BUSY))  /* if read and if ECHO, set hardware echo */
		cliparam(tp);
}


/* 
 *	set device parameters
 *      enable read; if ECHO, enable hardware echo.
 *
 */
cliparam(tp)
struct tty *tp;
{
/***
	oc(cliaddr[tp->t_dev.d_minor], ENABLE | READ | ((tp->t_flag & ECHO) ?
		    UNBLOCK : BLOCK));
***/	oc(cliaddr[tp->t_dev.d_minor], ENABLE | READ | BLOCK);
	return;
}

/*
 *
 *	When the interface changes from read to write, an interrupt
 *	is generated.  This is where we enable the transmit side.
 *	If busy is set , then a write is in progress and there is
 *	nothing to do.
 *
 */
clistart(atp)
struct	tty	*atp;
{
	register struct	tty	*tp;

	tp = atp;
	if ( !(tp->t_state & BUSY)) {
		tp->t_state =| (WINIT | BUSY);		/* set winit flag */
		oc(cliaddr[tp->t_dev.d_minor], ENABLE | WRITE | BLOCK );	/* enable write */
	}
}

/*
 *	second level output start routine
 *
 *	This routine is called from  cliint on write interrupt
 *	to send a character.
 *
 */

clistrto(atp)
struct tty *atp;
{
	int ttrstrt();
	register struct tty *tp;
	register c;

	tp = atp;

	/* get character to output  */

	c = getc(&tp->t_outq);
	if (c < 0)	/* no more chars on outq, start reading */
	{
		cliparam(tp);		/* enable read */
		return;
	}
	if (c > 0177) {
		/* timeout delay */
		tp->t_state =| TIMEOUT;
		timeout(&ttrstrt, tp, c&0177);
	} else {
		/* write character to device */
		tp->t_state =| BUSY;
		wd(cliaddr[tp->t_dev.d_minor], c);
		trace(0x1000, "wrt", ss(cliaddr[tp->t_dev.d_minor]));
	}
}

/*	interrupt handler
 */
cliint(addr,stat)
int	addr;
int	stat;
{
	register struct tty *tp;
	register c;

	tp = devmap(addr);
	if ( !(tp->t_state & ISOPEN))	/* return if device not open */
		return;
	if (tp->t_state & BUSY)  	/* if write enable or write complete */
	{
		if (stat & BRK)  		/* if BRK key hit */
		{
			trace(0x1000,"brk :",0);
			c = CINTR;	/* set character to del */
			tp->t_state =& ~(BUSY | WINIT);	/* reset flags */
			cliparam(tp);		/* reset to read mode */
			ttyinput(c, tp);
		}
		else
		if (tp->t_state & WINIT)  
		{
			/* if WINIT, interrupt was write enable */
         		trace(0x1000, "WINIT", stat);
         		tp->t_state =& ~(BUSY | WINIT);
         		clistrto(tp);
		}
		else 
		{
         		/* else interrupt was write complete */
         		trace(0x1000, "wint", stat);
         		tp->t_state =& ~BUSY;
         		clistrto(tp);
         
         		if (tp->t_outq.c_cc <= TTLOWAT && (tp->t_state&ASLEEP))
         		{
         			tp->t_state =& ~ASLEEP;
         			wakeup(&tp->t_outq);
         		}
		}
	}
	else	/*  read complete interrupt */
	{
		trace(0x1000, "rint", stat);
		c = rd(addr) & 0177;
		trace(0x1000, "c = ", c);
		ttyinput(c, tp);
	}
	trace(0x1000,"int exit ss:",ss(addr));
	trace(0x1000,"t_state",tp->t_state);
	trace(0x1000,"addr",addr);
}
