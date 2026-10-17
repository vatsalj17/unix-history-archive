#
/*
 * general TTY subroutines
 */

#include "../defines.h"
#include "../param.h"
#ifdef	AUSAML
#include "../lnode.h"
#endif	AUSAML
#include "../systm.h"
#include "../file.h"
#include "../user.h"
#include "../tty.h"
#include "../proc.h"
#include "../inode.h"
#include "../reg.h"
#include "../conf.h"

/*
 * Input mapping table-- if an entry is non-zero, when the
 * corresponding character is typed preceded by "\" the escape
 * sequence is replaced by the table value.  Mostly used for
 * upper-case only terminals.
 */
char	maptab[]
{
	000,000,000,000,004,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
#ifdef SLOSHED
	000,'|',000,000,000,000,000,'`',
#endif
#ifndef SLOSHED
	000,'|',000,'#',000,000,000,'`',
#endif
	'{','}',000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
#ifdef SLOSHED
	000,000,000,000,000,000,000,000,
#endif
#ifndef SLOSHED
	'@',000,000,000,000,000,000,000,
#endif
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
#ifdef SLOSHED
	000,000,000,000,'\\',000,'~',000,
#endif
#ifndef SLOSHED
	000,000,000,000,000,000,'~',000,
#endif
	000,'A','B','C','D','E','F','G',
	'H','I','J','K','L','M','N','O',
	'P','Q','R','S','T','U','V','W',
	'X','Y','Z',000,000,000,000,000,
};

#ifndef	ONCE
#include "../cinit.h"
#endif

/*
 * structure of device registers for KL, DL, and DC
 * interfaces-- more particularly, those for which the
 * SSTART bit is off and can be treated by general routines
 * (that is, not DH).
 */
struct {
	int ttrcsr;
	int ttrbuf;
	int tttcsr;
	int tttbuf;
};

/*
 * The routine implementing the gtty system call.
 * Just call lower level routine and pass back values.
 */
gtty()
{
	int v[3];
	register *up, *vp;

	vp = v;
	sgtty(vp);
	if (u.u_error)
		return;
	up = u.u_arg[0];
	suword(up, *vp++);
	suword(++up, *vp++);
	suword(++up, *vp++);
}

/*
 * The routine implementing the stty system call.
 * Read in values and call lower level.
 */
stty()
{
	register int *up;

	up = u.u_arg[0];
	u.u_arg[0] = fuword(up);
	u.u_arg[1] = fuword(++up);
	u.u_arg[2] = fuword(++up);
	if(u.u_error)			/* fix037 */
		return;

	sgtty(0);
}

/*
 * Stuff common to stty and gtty.
 * Check legality and switch out to individual
 * device routine.
 * v  is 0 for stty; the parameters are taken from u.u_arg[].
 * c  is non-zero for gtty and is the place in which the device
 * routines place their information.
 */
sgtty(v)
int *v;
{
	register struct file *fp;
	register struct inode *ip;

	if ((fp = getf(u.u_ar0[R0])) == NULL)
		return;
	ip = fp->f_inode;
	if ((ip->i_mode&IFMT) != IFCHR) {
		u.u_error = ENOTTY;
		return;
	}
	(*cdevsw[ip->i_addr[0].d_major].d_sgtty)(ip->i_addr[0], v);
}

/*
 * Wait for output to drain, then flush input waiting.
 */
wflushtty(tp)
register struct tty *tp;	/* fix000 */
{
	spl5();
	while (tp->t_outq.c_cc) {
#		ifdef	TTY_MULTIPLEXED_PORTS
		ttstart( tp );
#		endif	TTY_MULTIPLEXED_PORTS
		tp->t_state =| ASLEEP;
		sleep(&tp->t_outq, TTOPRI);
	}
	flushtty(tp);
	spl0();
}

/*
 * flush all TTY queues
 */
flushtty(tp)
register struct tty *tp;	/* fix000 */
{
	register int sps;
#ifdef	TTY_FLOW_CONTROL
	register *state = &tp->t_state;
#endif

	while (getc(&tp->t_canq) >= 0);
#ifndef	TTY_FLOW_CONTROL
	while (getc(&tp->t_outq) >= 0);
#endif
#ifdef	TTY_FLOW_CONTROL
	while ( getc(&tp->t_outq) >= 0 )
		tp->t_count--;
#endif
	wakeup(&tp->t_rawq);
	wakeup(&tp->t_outq);
	sps = PS->integ;
	spl5();
#ifdef	TTY_FLOW_CONTROL
	if ( *state & ACKFLAG )  {
		*state =| FCINTFL;
		if ( *state & FCSLEEP )
			wakeup( state );
		*state =& ~(ACKFLAG|FCSLEEP);
	}
#endif
	while (getc(&tp->t_rawq) >= 0);
	tp->t_delct = 0;
	PS->integ = sps;
}

/*
 * transfer raw input list to canonical list,
 * doing erase-kill processing and handling escapes.
 * It waits until a full line has been typed in cooked mode,
 * or until any character has been typed in raw mode.
 */
canon(tp)
register struct tty *tp;	/* fix000 */
{
	register char *bp;
	char *bp1;
#ifdef SLOSHED
	int slosh;
	register char *c;
#endif
#ifndef	SLOSHED
	register int c;
#endif

	spl5();
	while (tp->t_delct==0) {
		if ((tp->t_state&CARR_ON)==0)
			return(0);
		sleep(&tp->t_rawq, TTIPRI);
	}
	spl0();
#ifndef SLOSHED
loop:
	bp = &canonb[2];
#endif
#ifdef SLOSHED
	bp = canonb;
	slosh = 0;
#endif
	while ((c=getc(&tp->t_rawq)) >= 0) {
		if (c==0377) {
			tp->t_delct--;
			break;
		}
#ifdef SLOSHED
#ifndef	TTY_TRUE_RAW
		if ((tp->t_flags&RAW)==0) {
#endif
			if( slosh)
			{
				slosh = 0;
				if(maptab[c] &&
					(tp->t_flags & LCASE) )
				{
					c = maptab[c];
					bp--;
				}
				else
				{
					if(c == tp->t_erase || c == tp->t_kill)
						bp--;
				}
			}
			else
			{
				if( c == tp->t_erase)
				{
					if(bp > canonb) bp--;
					continue;
				}
				if( c == tp->t_kill) c = 0377;
				else if( c == CEOT) continue;
				if( c == '\\') slosh = 1;
			}
#ifndef	TTY_TRUE_RAW
		}
#endif
		*bp++ = c;
		if(bp >= canonb+TTYHOG) break;
	}
	c = canonb; bp1 = c;
	while( c < bp)
	{
	/**** 177777 below should read 377 but  *c++ causes a 'movb' and ensuing sign-bit propagation screws it */
		if( *c++ == 0177777) bp1 = c;
	}
	c = bp1;
	tp = &tp->t_canq;
	while(c < bp)
	{
		putc( *c, tp);
		*c++ = 0;	/* so that passwords won't appear in core dumps */
	}
	return(1);
#endif
#ifndef SLOSHED
#ifndef	TTY_TRUE_RAW
		if ((tp->t_flags&RAW)==0) {
#endif
			if (bp[-1]!='\\') {
				if (c==tp->t_erase) {
					if (bp > &canonb[2])
						bp--;
					continue;
				}
				if (c==tp->t_kill)
					goto loop;
				if (c==CEOT)
					continue;
			} else
			if (maptab[c] && (maptab[c]==c || (tp->t_flags&LCASE))) {
				if (bp[-2] != '\\')
					c = maptab[c];
				bp--;
			}
#ifndef	TTY_TRUE_RAW
		}
#endif
		*bp++ = c;
		if (bp>=canonb+TTYHOG)
			break;
	}
	bp1 = bp;
	bp = &canonb[2];
	c = &tp->t_canq;
	while (bp<bp1)
		putc(*bp++, c);
	return(1);
#endif
}

/*
 * Place a character on raw TTY input queue, putting in delimiters
 * and waking up top half as needed.
 * Also echo if required.
 * The arguments are the character and the appropriate
 * tty structure.
 */
ttyinput(ac, tp)
register struct tty *tp;	/* fix000 */
{
	register c;
	register int t_flags;

	c = ac & 0177;
#ifdef	TTY_CONNECT
	if(tp->t_rtype)
	{
		/*
		 * redirection of this tty port in progress
		 */
#		ifdef	TTY_MULTIPLEXED_PORTS
		if ( tp->t_rtype == TMULTIPLEX )
		{
			/** This line is being multiplexed **/
			struct { int (*func)(); };

			(*tp->t_redirect.func)( ac, tp );
			return;
		}
#		endif	TTY_MULTIPLEXED_PORTS
		if(c == tp->t_cchar && tp->t_rtype&TMASTER)
		{
			/*
			 * master terminate received - stop slave&master
			 */
			tp = tp->t_redirect;
			tp->t_rtype = tp->t_cchar = 0;
			tp = tp->t_redirect;
			tp->t_rtype = tp->t_cchar = 0;
			tp->t_redirect = (tp->t_redirect)->t_redirect = 0;
			wakeup(&tp->t_rtype);
			return;
		}
		switch(tp->t_rtype)
		{
	case TMASTER+TCONLOG:
	case	     TCONLOG:
			tp = tp->t_redirect;
			if(tp->t_outq.c_cc < TTYHOG)
			{
				putc(c, &tp->t_outq);
				ttstart(tp);
			}
			return;
#ifdef	TTY_SPECIAL_POWERS
	case TMASTER+TCONSHARE:
	case TMASTER+TCONGRAB:
			tp = tp->t_redirect;
			break;
	case	     TCONGRAB:
			return;
#endif	TTY_SPECIAL_POWERS
		}
	}
#endif	TTY_CONNECT
	t_flags = tp->t_flags;
	if(c == '\r')
	{
		if (t_flags&CRMOD)
			c = '\n';
	}
#ifdef	TTY_FLOW_CONTROL
	else if(c == ACK && (t_flags & (FLOWCNTRL|RAW)) == FLOWCNTRL)
	{
		if ( tp->t_state & FCSLEEP )
			wakeup( &tp->t_state );
		tp->t_state =& ~(ACKFLAG|FCINTFL|FCSLEEP);
		return;
	}
#endif	TTY_FLOW_CONTROL
	else if((c == CQUIT || c == CINTR) && (t_flags&RAW) == 0)
	{
		signal(tp, c==CINTR? SIGINT:SIGQIT);
		flushtty(tp);
		return;
	}

	if (tp->t_rawq.c_cc>=TTYHOG-1)  {	/* fix041 */
		flushtty(tp);
	}
	else					/* fix041 */
	{
#ifdef	TTY_INVCASE
		if ( t_flags&INVCASE && c>='A' )  {
			if ( c <= 'Z' )  c =+ 'a'-'A';
			else  if ( c >= 'a' && c <= 'z' )  c =- 'a'-'A';
		}else
#endif	TTY_INVCASE
		if (t_flags&LCASE && c>='A' && c<='Z')
			c =+ 'a'-'A';
#ifdef	TTY_TRUE_RAW
		if ( t_flags & RAW )  {
			if ( tp->t_state & RAWSLEEP )  {
				tp->t_state =& ~RAWSLEEP;
				wakeup( &tp->t_rawq );
			}
			if ( t_flags == RAW )
				c = ac;
			putc(c, &tp->t_rawq);
		}
		else
		{
			putc(c, &tp->t_rawq);
			if(c == '\n' || c == 004)
			{
				wakeup(&tp->t_rawq);
				if (putc(0377, &tp->t_rawq)==0)
					tp->t_delct++;
			}
		}
#else
		putc(c, &tp->t_rawq);
		if(t_flags&RAW || c=='\n' || c==004)
		{
			wakeup(&tp->t_rawq);
			if (putc(0377, &tp->t_rawq)==0)
				tp->t_delct++;
		}
#endif	TTY_TRUE_RAW
	}					/* fix041 */
	if(t_flags&ECHO || (c == '\n' && t_flags&CRMOD))	/* fix007 */
	{
		ttyoutput(c, tp);
		if( c == '\b' && tp->t_erase == '\b')	/* fix033 */
		{					/* fix033 */
			ttyoutput(' ', tp);		/* fix033 */
			ttyoutput(c, tp);		/* fix033 */
		}					/* fix033 */
		ttstart(tp);
	}
#ifdef	TTY_SPECIAL_POWERS
	else if(tp->t_rtype == TCONVIEW || tp->t_rtype == TCONSHARE)
	{
		ttyoutput(c, tp->t_redirect);
		ttstart(tp->t_redirect);
	}
#endif	TTY_SPECIAL_POWERS
}

/*
 * put character on TTY output queue, adding delays,
 * expanding tabs, and handling the CR/NL bit.
 * It is called both from the top half for output, and from
 * interrupt level for echoing.
 * The arguments are the character and the tty structure.
 */
ttyoutput(c, tp)
register char c;			/* fix000 */
register struct tty *tp;		/* fix000 */
{
	register union { char *colp; int flags; } i;	/* fasterfix */
	int ctype;

#ifdef	TTY_SPECIAL_POWERS
	if(tp->t_rtype)
	{
		switch(tp->t_rtype)
		{
	case TCONVIEW:
	case TCONSHARE:
			ttyoutput(c, tp->t_redirect);
			ttstart(tp->t_redirect);
			break;
	case TCONGRAB:
			ttyoutput(c, tp->t_redirect);
			ttstart(tp->t_redirect);
			return;
		}
	}
#endif	TTY_SPECIAL_POWERS
	i.flags = tp->t_flags;	/* fasterfix */

#ifdef	TTY_FLOW_CONTROL
	if ( i.flags & FLOWCNTRL && tp->t_state & ESCFL )
		return;	/* don't interrupt escape sequence */
#endif	TTY_FLOW_CONTROL

#ifdef	TTY_TRUE_RAW
	if ( ( i.flags & RAW ) == 0 )
#endif
		c =& 0177;
	/*
	 * Ignore EOT in normal mode to avoid hanging up
	 * certain terminals.
	 */
	if (c==004 && (i.flags&RAW)==0)
		return;
	/*
	 * Turn tabs to spaces as required
	 */
	if (c=='\t' && i.flags&XTABS) {
		c = 8 - (tp->t_col&07);	/* fix022 */
		do
			putc(' ', &tp->t_outq);
		while ( --c );		/* fix022 */
		tp->t_col = (tp->t_col+8) & ~07;
	/*	while (tp->t_col&07);	/* fix022 */
		return;
	}
	/*
	 * for upper-case-only terminals,
	 * generate escapes.
	 */
	if (i.flags&LCASE) {
		i.colp = "({)}!|^~'`";
		while(*i.colp++)
			if(c == *i.colp++) {
				putc('\\', &tp->t_outq);
				tp->t_col++;
				c = i.colp[-2];
				break;
			}
		if ('a'<=c && c<='z')
			c =+ 'A' - 'a';
		i.flags = tp->t_flags;
	}
	/*
	 * turn <nl> to <cr><lf> if desired.
	 */
	if (c=='\n' && i.flags&CRMOD)
		ttyoutput('\r', tp);

	if (putc(c, &tp->t_outq))
		return;

#ifdef	TTY_TRUE_RAW
	if ( i.flags == RAW )  return;	/* true RAW only */
#endif
#ifdef	TTY_FLOW_CONTROL
	tp->t_count++;
#endif

	/*
	 * Calculate delays.
	 * The numbers here represent clock ticks
	 * and are not necessarily optimal for all terminals.
	 * The delays are indicated by characters above 0200,
	 * thus (unfortunately) restricting the transmission
	 * path to 7 bits.
	 */
	i.colp = &tp->t_col;
	ctype = partab[c];
	c = 0;
	switch (ctype&077) {

	/* ordinary */
	case 0:
		(*i.colp)++;

	/* non-printing */
	case 1:
		break;

	/* backspace */
	case 2:
		if (*i.colp)
			(*i.colp)--;
		break;

	/* newline */
	case 3:
		ctype = (tp->t_flags >> 8) & 03;
		if(ctype == 1) { /* tty 37 */
			if (*i.colp)
				c = max((*i.colp>>4) + 3, 6);
		} else
		if(ctype == 2) { /* vt05 */
			c = 6;
		}
		*i.colp = 0;
		break;

	/* tab */
	case 4:
		ctype = (tp->t_flags >> 10) & 03;
		if(ctype == 1) { /* tty 37 */
			c = 1 - (*i.colp | ~07);
			if(c < 5)
				c = 0;
		}
		*i.colp =| 07;
		(*i.colp)++;
		break;

	/* vertical motion */
	case 5:
		if(tp->t_flags & VTDELAY) /* tty 37 */
			   c = 0177;
		break;

	/* carriage return */
	case 6:
		ctype = (tp->t_flags >> 12) & 03;
		if(ctype == 1) { /* tn 300 */
			c = 5;
		} else
		if(ctype == 2) { /* ti 700 */
			c = 10;
		}else
		if ( ctype == 3 )  {	/* NCR260 */
			/* need 1 tic delay for every 8 cols */
			c = (*i.colp>>3);
		}
		*i.colp = 0;
	}

	if(c)
		putc(c|0200, &tp->t_outq);
}

/*
 * Restart typewriter output following a delay
 * timeout.
 * The name of the routine is passed to the timeout
 * subroutine and it is called during a clock interrupt.
 */
ttrstrt(tp)
register struct tty *tp;
{
	tp->t_state =& ~TIMEOUT;
	ttstart(tp);
}

/*
 * Start output on the typewriter. It is used from the top half
 * after some characters have been put on the output queue,
 * from the interrupt routine to transmit the next
 * character, and after a timeout has finished.
 * If the SSTART bit is off for the tty the work is done here,
 * using the protocol of the single-line interfaces (KL, DL, DC);
 * otherwise the address word of the tty structure is
 * taken to be the name of the device-dependent startup routine.
 *
 * Always assumed to be called at level five or greater
 *
 */
ttstart(tp)
register struct tty *tp;	/* fix000 */
{
	register int *addr, c;
	struct { int (*func)(); };

	if ( tp->t_state & TIMEOUT )	/* fix008 */
		return;

	addr = tp->t_addr;
	if (tp->t_state&SSTART) {
		(*addr.func)(tp);
		return;
	}
	if ((addr->tttcsr&DONE)==0)	/* fix008 */
		return;
	if ((c=getc(&tp->t_outq)) >= 0) {
#ifdef	TTY_TRUE_RAW
		if ( tp->t_flags == RAW )  addr->tttbuf = c;
		else
#endif
		if (c<=0177)
			addr->tttbuf = c | (partab[c]&0200);
		else {
			timeout(ttrstrt, tp, c&0177);
			tp->t_state =| TIMEOUT;
		}
	}
}

/*
 * Called from device's read routine after it has
 * calculated the tty-structure given as argument.
 */
ttread(tp)
register struct tty *tp;	/* fix000 */
{

#ifdef	TTY_TRUE_RAW
	if ( tp->t_flags & RAW )  {
		do
		{
			spl5();
			while ( tp->t_rawq.c_cc == 0 )
				if ( tp->t_state & CARR_ON )  {
					tp->t_state =| RAWSLEEP;
					sleep( &tp->t_rawq , TTIPRI );
				}else  {
					spl0();
					return;
				}
			spl0();
		}
		while
			( passc( getc( &tp->t_rawq ) ) >= 0 );
	}else
#endif
#ifndef	TTY_TRUE_RAW
	if ((tp->t_state&CARR_ON)==0)
		return;
#endif
	if (tp->t_canq.c_cc || canon(tp))
		while (tp->t_canq.c_cc && passc(getc(&tp->t_canq))>=0);
}

/*
 * Called from the device's write routine after it has
 * calculated the tty-structure given as argument.
 */
ttwrite(tp)
register struct tty *tp;	/* fix000 */
{
	register int *state, c;
#ifdef	TTY_FLOW_CONTROL
	extern	int	ttackfix();
#endif

	state = &tp->t_state;
	if ((*state&CARR_ON)==0)
		return;
	while ((c=cpass())>=0) {
		spl5();
#ifdef	TTY_HISPEED
		while ( (tp->t_outq.c_cc>TTHIWAT) && ( ((tp->t_speeds&017)<=B1200) || (tp->t_outq.c_cc>TTHSHIWAT) ) )  {
#endif
#ifndef	TTY_HISPEED
		while (tp->t_outq.c_cc > TTHIWAT) {
#endif
			ttstart(tp);
			*state =| ASLEEP;
			sleep(&tp->t_outq, TTOPRI);
		}
#ifdef	TTY_FLOW_CONTROL
		if ( (tp->t_flags & FLOWCNTRL) && (tp->t_count >= FCBUFZ - 1) && ((*state & ESCFL) == 0) )  {
			while ( putc( ETX , &tp->t_outq ) )
				delay( 2 );
			while ( *state & ACKFLAG )  {
				if ( !tp->t_timep )
					tp->t_timep = timeout( ttackfix, tp, FCTIMEOUT );
				*state =| FCSLEEP;
				sleep( state , TTOPRI );
			}
			untimeout( tp->t_timep, ttackfix, tp );
			tp->t_timep = 0;
			*state =| ACKFLAG;
			tp->t_count =- FCBUFZ-1;
		}
#endif
		spl0();
#ifdef	TTY_FLOW_CONTROL
		if ( tp->t_flags & FLOWCNTRL )
		if ( (*state & ESCFL) == 0 )  {  if ( (c & 0177) == ESC )  {  *state =| ESCFL;  goto put; }  }
		else
		{
			if ( *state & ESC2FL )  *state =& ~(ESCFL|ESC2FL);
			else
				switch ( c & 0177 )  {
				 default:	*state =& ~ESCFL;  break;
				 case VT: case HT: case RS: case US:  *state =| ESC2FL;	/* per DIABLO HYTERM */
				}

put:
			while ( putc( c, &tp->t_outq ) )
				delay( 2 );
			tp->t_count++;
			continue;
		}
#endif
		ttyoutput(c, tp);
	}
	spl5(); ttstart(tp); spl0();	/* fix014 */
}

/*
 * Common code for gtty and stty functions on typewriters.
 * If v is non-zero then gtty is being done and information is
 * passed back therein;
 * if it is zero stty is being done and the input information is in the
 * u_arg array.
 */
ttystty(tp, v)
register int *v;		/* fix000 */
register struct tty *tp;	/* fix000 */
{

	if( v ) {
		*v++ = tp->t_speeds;
		v->lobyte = tp->t_erase;
		v->hibyte = tp->t_kill;
		v[1] = tp->t_flags;
		return(1);
	}
#	ifdef	TTY_SUSER
	if (tp != u.u_procp->p_ttyp && ! suser()) return 1;
#	endif	TTY_SUSER

	wflushtty(tp);
	v = u.u_arg;

#	ifdef	TTY_SUSER
	if( (v->hibyte>tp->t_speeds.hibyte || v->lobyte>tp->t_speeds.lobyte)
	 && (v->hibyte > TTY_MAXSPD || v->lobyte > TTY_MAXSPD)
	 && ! suser() )
		*v = tp->t_speeds;	/* don't allow `bad' speeds */

#	endif	TTY_SUSER
	tp->t_speeds = *v++;
	tp->t_erase = v->lobyte;
	tp->t_kill = v->hibyte;
	tp->t_flags = v[1];
	return(0);
}

#ifdef	TTY_FLOW_CONTROL
ttackfix( tp )
  register struct tty *tp;
{
  tp->t_timep = 0;
  if ( tp->t_state & FCSLEEP )
	wakeup( &tp->t_state );
  tp->t_state =& ~(FCSLEEP|ACKFLAG);
}
#endif	TTY_FLOW_CONTROL

#ifdef	TTY_CONNECT
struct tty *ttyget(arg)
int arg;
{
	register struct tty *tp;
	register struct inode *ip;
	register struct file *fp;
	int dev;

	if((fp = getf(arg)) == NULL)
	{
		return 0;	/* illegal fd passed */
	}
	if((fp->f_flag&(FREAD|FWRITE)) != (FREAD|FWRITE))
	{
		u.u_error = EACCES;	/* no permission */
		return 0;
	}
	ip = fp->f_inode;
	if((ip->i_mode&IFMT) != IFCHR)
	{
		u.u_error = ENOTTY;
		return 0;
	}
	dev = ip->i_addr[0];
	if((tp = cdevsw[dev.d_major].d_tty) == 0)
	{
		u.u_error = ENOREDIRECT; /* not capable */
		return 0;
	}
	return tp + dev.d_minor;
}

ttyconnect()
{
	register struct tty *mtp, *stp;
	register int rtype;

	if(!suser())
		return;
	rtype = u.u_arg[0];	/* master control character */
	if((mtp = ttyget(u.u_ar0[R0])) == 0) return;
	if((stp = ttyget(u.u_ar0[R1])) == 0) return;

	spl5();

	switch(rtype)
	{
    case 0:	/* disconnect */
		if(mtp != stp || mtp->t_rtype == 0)
		{
			u.u_error = EREDIRECT;
		}
		else
		{
			stp = mtp->t_redirect;
			if(stp->t_redirect != mtp)
			{
				u.u_error = EREDIRECT;
			}
			else
			{
				/*
				 * wakeup master tty after
				 * determining which it is
				 */
				if(mtp->t_rtype&TMASTER)
					wakeup(&mtp->t_rtype);
				else
					wakeup(&stp->t_rtype);
				mtp->t_rtype = stp->t_rtype = 0;
				mtp->t_cchar = stp->t_cchar = 0;
				mtp->t_redirect = stp->t_redirect = 0;
			}
		}
		break;
    case TCONLOG:
#ifdef	TTY_SPECIAL_POWERS
    case TCONVIEW:
    case TCONGRAB:
    case TCONSHARE:
#endif	TTY_SPECIAL_POWERS
		if(mtp == stp || mtp->t_rtype || stp->t_rtype )
		{
			/* already redirected */
			u.u_error = EREDIRECT;
		}
		else
		{
			mtp->t_rtype = TMASTER+rtype;
			stp->t_rtype = rtype;
			mtp->t_cchar = u.u_arg[1];
			mtp->t_redirect = stp;
			stp->t_redirect = mtp;
			while(mtp->t_rtype)
				sleep(&mtp->t_rtype, PRIREDIRECT);
		}
		break;
    default:
		u.u_error = EINVAL;
		break;
	}

	spl0();
}
#endif	TTY_CONNECT
