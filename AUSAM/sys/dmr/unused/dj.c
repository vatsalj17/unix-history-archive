#
/*
 **   	copyright 1975.    ian.  inc
 *
 *
 **	also that dj11 parity checking is screwed if terminal attached
 **	does not generate parity. SO IT AIN'T SUPPORTED !!
 */
 
/*
 *	dj-11 driver
 */
#include "../param.h"
#include "../conf.h"
#include "../user.h"
#include "../tty.h"
#include "../proc.h"
 
#define DJSCANRATE	2	/* define this if dj11 is strapped
				   to interrrupt when input silo
				   has more than 5,9,17, .. ,31 chars in it.
				   2 is best any more and eching becomes chunky */
#define	NDJ11	16		/* only one dj11 - but 16 lines */

/* device register equates */

#define	DJADDR		0160010		/* 1st dj register group*/
#define	DJTIE		0040000		/* xmit interupt enable */
#define	DJTSE		0000400		/* xmit scanner enable */
#define	DJTR		0100000		/* transmitter ready */
#define	DJRIE		0000100		/* receive interupt enable */
#define	DJRE		0000001		/* receive enable */
#define	DJOVERUN	0040000		/* overrun error */
#define	DJFRAME		0020000		/* framing error */
#define	DJPARITY	0010000		/* parity error */


unsigned djoverrun;	/* Total count of overrun errors.
	  		   Don't init - want bss for big unix systems 
			   so can put in root segment */

unsigned djscanning 0 ;	/* True when scanning in operation.
			   Equal to total number of open lines.
			   When true scan open lines every DJSCANRATE ticks. */

unsigned djsopen 0 ;	/* If bit 'n' set then line 'n' is an exclusive use.
			   That is only one open allowed (e.g. LA180)	*/

struct  tty dj11[NDJ11];
 
struct djregs {
	int djcsr;
	int djrbuf;
	int djtcr;
	int djtbuf;
}
 
djopen(dev, flag)
{
	register char *addr;
	register struct tty *tp;
	extern djstart(),djrint();
 
	if(dev.d_minor >= NDJ11) {
		u.u_error = ENXIO;
		return;
	}
	tp = &dj11[dev.d_minor];
 
	if ((tp->t_state&ISOPEN) == 0) {
		tp->t_addr = djstart;   /* special start routine */
		tp->t_dev  = dev;
		tp->t_state = SSTART|ISOPEN|CARR_ON;
		tp->t_flags = ODDP|EVENP|RAW;
	} else if( djsopen & (1<<dev.d_minor) ) {
			u.u_error = EIO;
			return;
		}
	if( djscanning==0 ) {
		DJADDR->djcsr =  DJTIE|DJTSE|DJRE|DJRIE;
#ifdef	DJSCANRATE
		timeout( &djrint, -1, DJSCANRATE );
#endif
	}
	djscanning++;

	if (u.u_procp->p_ttyp == 0)
		u.u_procp->p_ttyp = tp;
}
 
djclose(dev)
{
	register struct tty *tp;
 
	wflushtty( tp = &dj11[dev.d_minor] );
	tp->t_state = SSTART;
	djscanning--;
}
 
djread(dev)
{
	ttread(&dj11[dev.d_minor]);
}
 
djwrite(dev)
{
	ttwrite(&dj11[dev.d_minor]);
}
 
 
djstart(atp)
struct tty *atp;
{
	/* enable transmit for this line */
	DJADDR->djtcr =| 1<<(atp-dj11);
 
}
 
djxint(dev)
{
	register int c;
	extern ttrstrt();
	register struct tty *tp ;
 
  while (DJADDR->djcsr&DJTR) {	/* loop till trans. happy */
	tp = &dj11[(DJADDR->djtbuf)>>8];	/* get line number */
	if( !(tp->t_state&TIMEOUT) )
	  if((c = getc(&tp->t_outq)) >= 0)  {
		if( tp->t_flags & RAW ) {
			DJADDR->djtbuf = c;
			continue;
		}
		if( c<=0177 ) {
			DJADDR->djtbuf = c + (partab[c]&0200);
			continue;
		} else {
			timeout( &ttrstrt, tp, c&0177 );
			tp->t_state =| TIMEOUT;
		}
	  }
	DJADDR->djtcr =& ~(1<<(tp-dj11)); /*stop*/
 
	if (tp->t_outq.c_cc <= TTLOWAT && tp->t_state & ASLEEP )  {
		tp->t_state =& ~ASLEEP;
		wakeup(&tp->t_outq);
	  }
  }
}
 
djrint(dev)
{
	register int c, *addr;
	register struct tty *tp;
	extern djrint();
 
	while( (c = DJADDR->djrbuf) < 0)	/* char present in silo */
	{
		tp = &dj11[(c>>8) & 017]; /* line number */
		if( c & DJOVERUN )
			djoverrun++;
		if( c & DJFRAME )		/* break */
			if(tp->t_flags & RAW)
				c = 0;	/* null for getty */
			else
				continue; /* ignore framing errors if not raw */
		ttyinput(c, tp);
	}
#ifdef	DJSCANRATE
	if( (dev==-1)&&(djscanning>0) ) timeout( &djrint, -1, DJSCANRATE );
#endif
}
 
djsgtty(dev, v)
int *v;
{
	ttystty(&dj11[dev.d_minor], v);
}
