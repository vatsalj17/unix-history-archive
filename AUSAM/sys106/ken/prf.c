#
/*
 */

#include "../defines.h"
#include "../param.h"
#include "../seg.h"
#include "../buf.h"
#include "../conf.h"

#ifdef	ERROR_LOG | NICE_PUTCHAR | UPRINTS
#include "../tty.h"
#endif

#ifdef	UPRINTS
#include "../user.h"
#include "../proc.h"
#endif

#ifdef	ERROR_LOG

#define	FULL		1	/* flag bits */
#define	OPEN		2

#define	HIWATER		100

struct	errorlog
{
  int	flag;
  struct tty errtty;
}
  errlog;

#endif

/*
 * Address and structure of the
 * KL-11 console device registers.
 */
struct
{
	int	rsr;
	int	rbr;
	int	xsr;
	int	xbr;
};

/*
 * In case console is off,
#ifndef	BETTER_PANIC
 * panicstr contains argument to last
#else
 * panicstr contains argument to first
#endif	BETTER_PANIC
 * call to panic.
 */

char	*panicstr;

/*
 * Scaled down version of C Library printf.
 * Only %s %l %d (==%l) %o are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 * Since it is not interrupt driven,
 * all system activities are pretty much
 * suspended.
 * Printf should not be used for chit-chat.
 */
printf(fmt,x1,x2,x3,x4,x5,x6,x7,x8,x9,xa,xb,xc)
char fmt[];
{
	register char *s;
	register *adx, c;

	adx = &x1;
loop:
	while((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(c);
	}
	c = *fmt++;
	if(c == 'd' || c == 'l' || c == 'o')
		printn(*adx, c=='o'? 8: 10);
	if(c == 's') {
		s = *adx;
		while(c = *s++)
			putchar(c);
	}
	adx++;
	goto loop;
}

/*
 * Print an unsigned integer in base b.
 */
printn(n, b)
{
	register a;

	if(a = ldiv(n, b))
		printn(a, b);
	putchar(lrem(n, b) + '0');
}

/*
 * Print a character on console.
 * Attempts to save and restore device
 * status.
 * If the switches are 0, all
 * printing is inhibited.
 */
putchar(c)
register c;	/* fix000 */
{
	register s;
	register sps = PS->integ;
#ifdef	NICE_PUTCHAR
	extern cfreelist;
	extern struct tty kl11;
#endif
#ifdef	POWER_FAIL
	extern powflag;	/* set if power fail in progress */

	if( powflag ) goto nofrills;
#endif	POWER_FAIL
#ifdef	ERROR_LOG | NICE_PUTCHAR
	/* if panicing or current prty greater than 5 use direct output */
	if((sps&0340)>0240)
		goto nofrills;
	spl5();
	if(panicstr)
		goto nofrills;
#endif	ERROR_LOG | NICE_PUTCHAR

#ifdef	ERROR_LOG
	if ( (errlog.flag & (OPEN|FULL)) == OPEN )
	{
		while( cfreelist == 0 )		/* don't lose messages */
		{
			spl0();		/* devices beware ... multiple interrupts */
			idle();
			spl5();
		}
		ttyinput( c , &errlog.errtty );
		if ( c == '\n' )  {
			if ( errlog.errtty.t_rawq.c_cc > HIWATER )
				errlog.flag =| FULL;
		}
		goto out;
	}
#endif
	if(SW->integ == 0)	/* fix016 */
		goto out;
#ifdef	NICE_PUTCHAR
	if ( kl11.t_state&ISOPEN )  {
		while( cfreelist == 0 ) 	/* don't lose messages */
		{
			spl0();
			idle();
			spl5();
		}
		uputchar( c , &kl11 );
		goto out;
	}
#endif
#ifdef	POWER_FAIL | ERROR_LOG | NICE_PUTCHAR
 nofrills:
#endif	POWER_FAIL | ERROR_LOG | NICE_PUTCHAR
	while((KL->xsr&0200) == 0)
		;
	if(c == 0)
		goto out;
	s = KL->xsr;
	KL->xsr = 0;
	KL->xbr = c;
	if(c == '\n') {
		putchar('\r');
#ifdef SLOW_CONSOLE
		putchar(0177);
		putchar(0177);
		putchar(0177);
		putchar(0177);
#endif
	}
	putchar(0);
	KL->xsr = s;
out:
	PS->integ = sps;
}

/*
 * Panic is called on unresolvable
 * fatal errors.
 * It syncs, prints "panic: mesg" and
 * then loops.
 */
panic(s)
char *s;
{
#ifdef	SYS_TIME
	char hhmm[6];

#endif	SYS_TIME
#ifndef	BETTER_PANIC
	panicstr = s;
#else
	if( panicstr < 0400  ) panicstr = s;
#endif	BETTER_PANIC
	update();
#ifdef	SYS_TIME
	printf("time = %s\n", systime(hhmm) );
#endif	SYS_TIME
	printf("panic: %s\n", s);
	for(;;)
		idle();
}

/*
 * prdev prints a warning message of the
 * form "mesg on dev x/y".
 * x and y are the major and minor parts of
 * the device argument.
 */
prdev(str, dev)
{

	printf("%s on dev %l/%l\n", str, dev.d_major, dev.d_minor);
}

/*
 * deverr prints a diagnostic from
 * a device driver.
 * It prints the device, block number,
 * and an octal word (usually some error
 * status register) passed as argument.
 */
deverror(bp, o1, o2)
int *bp;
{
	register *rbp;

	rbp = bp;
	prdev("err", rbp->b_dev);
	printf("bn%l er%o %o\n", rbp->b_blkno, o1, o2);
}

#ifdef	ERROR_LOG
/*
 * this pseudo device driver implements the read-only device
 * "errlog" which sucks off all non-panic "putchar" output.
 */
elopen()
{
  errlog.flag = OPEN;
  errlog.errtty.t_state = (ISOPEN|CARR_ON);
}

elread()
{
  ttread( &errlog.errtty );
  errlog.flag =& ~FULL;
}

elclose()
{
  errlog.flag = 0;
  flushtty( &errlog.errtty );
}
#endif


#ifdef	UPRINTS

/*
 * uprints provides a facility for warning messages from deep within
 * system to be presented to a particular user.
 */
uprints( s )
  register char *s;
{
	register c;
	register struct tty *ttyp;

  /* if cpu prty greater then five don't */
  if( (PS->integ&0340)>0240 ) return;
  if ( ttyp = u.u_procp->p_ttyp )
	while ( c = *s++ )
		uputchar( c , ttyp );
}
#endif


#ifdef	UPRINTS | NICE_PUTCHAR
/*
 * uputchar directs a character to the appropriate tty queue
 * via "ttyoutput", and guarantees not to sleep.
 * N.B.  high water marks are disregarded -- so don't be verbose !
 */
uputchar( c , ttyp )
  register struct tty *ttyp;
{
	register sps;

  sps = PS->integ;
  spl5();
	ttyoutput( c , ttyp );
	ttstart( ttyp );
  PS->integ = sps;
}
#endif
