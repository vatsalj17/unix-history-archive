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
#include "../defines.h"
#include "../param.h"
#include "../conf.h"
#include "../user.h"
#include "../tty.h"
#include "../proc.h"
 
#define DJSCANRATE	2	/* define this if dj11 is strapped
				   to interrrupt when input silo
				   has more than 5,9,17, .. ,31 chars in it.
				   2 is best any more and eching becomes chunky */
#define	NDJ11	1		/* one only dj11s */
#define	NLINES	(NDJ11*16)	/* number of terminals connected */
#define SSPEED	12		/* standard speed 4800 bd */

/* device register equates */

#define	DJTIE		0040000		/* xmit interupt enable */
#define	DJTSE		0000400		/* xmit scanner enable */
#define	DJTR		0100000		/* transmitter ready */
#define	DJRIE		0000100		/* receive interupt enable */
#define	DJRE		0000001		/* receive enable */
#define	DJOVERUN	0040000		/* overrun error */
#define	DJFRAME		0020000		/* framing error */
#define	DJPARITY	0010000		/* parity error */

struct djregs {
	int djcsr;
	int djrbuf;
	int djtcr;
	int djtbuf;
};
 
struct dj {
	struct djregs *djaddr;	/* address of registers for this dj11 */
	unsigned openl;		/* flags for open lines */
	unsigned sopen;		/* If bit 'n' set then line 'n' is an exclusive use.
				That is only one open allowed (e.g. LA180)	*/
	} dj[NDJ11] {
		0160010, 0000000, 0000210,
};

unsigned djoverrun;	/* Total count of overrun errors.
	  		   Don't init - want bss for big unix systems 
			   so can put in root segment */

unsigned djscanning;	/* True when scanning in operation.
			Equal to total number of open lines.
			When true scan open lines every DJSCANRATE ticks. */


struct  tty dj11[NLINES];
 
djopen(dev, flag)
{
	register struct dj *djp = &dj[dev.d_minor>>4];
	register struct tty *tp = &dj11[dev.d_minor];
	register t_bit = (1 << (dev.d_minor&017));
	extern djstart(),djrint();
 
	if(dev.d_minor >= NLINES)
	{
		u.u_error = ENXIO;
		return;
	}
 
	if ((tp->t_state&ISOPEN) == 0)
	{
		if( djp->openl == 0 )
			djp->djaddr->djcsr =  DJTIE|DJTSE|DJRE|DJRIE;
		djp->openl =| t_bit;
#ifdef	DJSCANRATE
		if( djscanning==0 )
			timeout( &djrint, -1, DJSCANRATE );
#endif
		djscanning++;
		tp->t_addr = djstart;   /* special start routine */
		tp->t_dev = dev;
		tp->t_speeds = SSPEED|(SSPEED<<8);
		tp->t_state = SSTART|ISOPEN|CARR_ON;
		tp->t_flags = ODDP|EVENP|XTABS|RAW;
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
	}
	else if( djp->sopen & t_bit )
	{
		u.u_error = EOPENFAIL;
		return;
	}

	if (u.u_procp->p_ttyp == 0)
		u.u_procp->p_ttyp = tp;
}
 
djclose(dev)
{
	register struct dj *djp = &dj[dev.d_minor>>4];
	register struct tty *tp = &dj11[dev.d_minor];
	register t_bit = (1 << (dev.d_minor&017));
 
	wflushtty( tp );
	tp->t_state = SSTART;
	djp->openl =& ~t_bit;
	if( djp->openl == 0 )
		djp->djaddr->djcsr = 0;
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
	register line;

	line = atp - dj11;
	/* enable transmit for this line */
	dj[ line>>4 ].djaddr->djtcr =| 1 << (line&017) ;
}
 
djxint()
{
	register int c;
	register struct tty *tp ;
	register struct dj *djp;
	int line,djn;
	extern ttrstrt();
 
  for( djp=dj , djn=0 ; djp<&dj[NDJ11] ; djp++ , djn=+16 ) {
	while (djp->djaddr->djcsr&DJTR) {	/* loop till trans. happy */
	  tp = &dj11[djn+(line=(djp->djaddr->djtbuf)>>8)];	/* get tty structure address + line no. */
	  if( !(tp->t_state&TIMEOUT) )
	    if((c = getc(&tp->t_outq)) >= 0)  {
		if( tp->t_flags == RAW ) {
			djp->djaddr->djtbuf = c;
			continue;
		}
		if( c<=0177 ) {
			djp->djaddr->djtbuf = c + (partab[c]&0200);
			continue;
		} else {
			timeout( &ttrstrt, tp, c&0177 );
			tp->t_state =| TIMEOUT;
		}
	    }
	  djp->djaddr->djtcr =& ~(1<<line); /*stop*/
 
	  if (tp->t_outq.c_cc <= TTLOWAT && tp->t_state & ASLEEP )  {
		tp->t_state =& ~ASLEEP;
		wakeup(&tp->t_outq);
	  }
	}
  }
}
 
djrint( dev )
{
	register int c;
	register struct tty *tp;
	register djn;
	extern djrint();
 
  for( djn=0 ; djn<NDJ11 ; djn++ ) {
	while( (c = dj[djn].djaddr->djrbuf) < 0)	/* char present in silo */
	{
		tp = &dj11[(djn<<4)+((c>>8) & 017)]; /* line number */
		if( c & DJOVERUN )
			djoverrun++;
		if( c & DJFRAME )		/* break */
			if(tp->t_flags & RAW)
				c = 0;	/* null for getty */
			else
				continue; /* ignore framing errors if not raw */
		ttyinput(c, tp);
	}
  }
#ifdef	DJSCANRATE
	if( (dev == -1) && (djscanning) ) timeout( &djrint, -1, DJSCANRATE );
#endif
}
 
djsgtty(dev, v)
int *v;
{
	ttystty(&dj11[dev.d_minor], v);
}
#ifdef	POWER_FAIL

djpowf()
{
	register line;
	register struct djregs *dja;
	register struct tty *tp;
	int djn;

	tp = &dj11[0];
	for( djn = 0 ; djn < NDJ11 ; djn++ ) {
		dja = dj[djn].djaddr;
		for( line = 0 ; line < 16 ; line++, tp++ )
			if( tp->t_state & ISOPEN ) {
				dja->djcsr = DJTIE|DJTSE|DJRE|DJRIE;
				dja->djtcr =| 1 << line;
			}
	}
}
#endif	POWER_FAIL
