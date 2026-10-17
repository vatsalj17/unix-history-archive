#
/*
 *	driver for BURROUGHS TD800 POLL/SELECT communications protocol
 *
 *		    by
 *	       piers lauder
 *	 DEPT OF COMPUTER SCIENCE
 *	   UNIVERSITY OF SYDNEY
 *	      november  1977
 *
 * This driver will simulate any number of "stations" on a BURROUGHS
 * asynchronous multi-drop TD800 type communications link attached to
 * a DL11. Minor device selects station, two major devices select
 * read or write. Any number of bytes may be written, but any bytes
 * not taken from a read message are lost. EOF is signalled by a
 * null block ( in either direction ) which is read as a zero.
 */


#include	"../defines.h"
#include	"../param.h"
#include	"../user.h"
#include	"../buf.h"
#include	"dl11.h"
#include	"../conf.h"

/* (not	ERROR_MESSAGE)
#define	ERROR_MESSAGE			/* if errors to be printed on close */


/*
 *	tunable constants
 */
#define	NSTATIONS	2		/* number of stations */
#define	READPRI		3		/* priority for sleep when reading */
#define	WRITEPRI	2		/* priority for sleep when writing */
#define	T_DELAY		2		/* b1726 turn around delay in 'tics' */
#define	NOECHO		01		/* reset "flags" bit 1 if echoes expected */
#define	ERRORS		02		/* set "flags" bit 2 if errors are to be reported */
#define	splbx		spl6		/* priority of controller */
/* following bytes should be even parity (programming glitch) */
#define	AD1		'0'		/* protocol address 1 ('0' = UNIX) */
#define	WMN		'0'		/* first message number for write */


struct	stn
{
  char		s_flags;	/* user synchronisation */
  char		ad2;		/* this station address */
  char		lmn;		/* last read message number */
  char		rmn;		/* read message number */
  char		wmn;		/* write message number */
  char		wlpc;		/* write lpc */
  struct buf	*rbp;		/* read buffer pointer */
  struct buf	*wbp;		/* write buffer pointer */
}
	bx_stns[NSTATIONS]
{
  {	0,060	},		/* station address '00' */
  {	0,0261	},		/* station address '01' */
/*  {	0,0262	}		/* station address '02' */
};

struct
{
  char		flags;		/* to control driver characteristics */
  char		echoes;		/* hardware echoes expected */
  char		rstate;		/* receive state */
  char		wstate;		/* write state */
  char		rlpc;		/* recieve lpc */
  char		*posn;		/* position of data in buffer */
  int		count;		/* data count */
  int		blks_sent;	/* blocks sent - statistics */
  int		blks_rcvd;	/* blocks recieved - statistics */
  struct stn	*station;	/* current station on line */
}
  bx;

/*
 *	s_flags bits
 */
#define	OPENR		1	/* open for reading */
#define	OPENW		2	/* open for writing */
#define	RD_DATA_RDY	4	/* data ready for reading */
#define	GET_RD_BLK	010	/* block needed for input */
#define	WTG_FOR_WRT	020	/* waiting to transfer data to output block */
#define	WRT_DATA_RDY	040	/* data ready for output */
#define	WCLOSE		0100	/* write null block on close */

/*
 *	significant bytes
 */
#define	SOH		1
#define STX		2
#define ETX		3
#define EOT		4
#define ENQ		5
#define ACK		6
#define NAK		025
#define	POL		'p'
#define	SEL		'q'

/*
 *	significant bytes + parity
 */
#define	SOHp		0201
#define STXp		0202
#define ETXp		3
#define EOTp		0204
#define ENQp		5
#define ACKp		6
#define NAKp		0225

/*
 *	decision table selectors
 */
#define Y_EOT		0
#define Y_default	1
#define Y_error 	2

/*
 *	define structure to log errors
 */
#define	NERR	8
struct	error
{
#ifdef	ERROR_MESSAGE
	char	*e_mess;
#endif	ERROR_MESSAGE
	int	e_count;
}
	bxerr[NERR]
#ifdef	ERROR_MESSAGE
{
  {	"resets", 0	},
  {	"xmit errors",0 },
  {	"xmit parity",0 },
  {	"recv bad", 0	},
  {	"recv lpc", 0	},
  {	"breaks", 0	},
  {	"overuns", 0	},
  {	"recv parity", 0},
  0
}
#endif	ERROR_MESSAGE
;
#define	PARITY		7
#define	OVERUN		6
#define	BREAK		5
#define	RECVLPC		4
#define	RECVBAD		3
#define	WPAR		2
#define	WERR		1
#define	RESET		0


char	partab[];	/* must use version fixed with bit 7 set for illegal data bytes */
			/* see "bxwparity" below */



/*
 *	general open:
 *	- check exclusive use & device free,
 *	- set vectors.
 *	- enable read interrupts
 */
bxopen( flagp , FLAG )
  register char *flagp;
  register FLAG;
{
	register char *port = &dl11[BP];
	extern bxrint(), bxwint();

  if ( (flagp >= &bx_stns[NSTATIONS]) || *flagp & FLAG || (*port && *port != COM) )  {
	u.u_error = ENXIO;
	return( 0 );
  }

  *flagp =| FLAG;
  if ( !*port )  {
	dl11v[BP].rintad = bxrint;
	dl11v[BP].wintad = bxwint;
	BPADDR->dlrs = IENABLE;
	*port = COM;
  }
  return( 1 );
}


	/*********
	 * write *
	 *********/

/*
 *	open for writing and set up write block
 *	N.B. this block is attached for the duration of the write.
 */
bxopenw( dev )
{
	register struct stn *sp = &bx_stns[dev.d_minor];

  splbx();
	if ( bxopen( &sp->s_flags , OPENW ) )  {
		if ( !sp->wbp )
			sp->wbp = getblk( NODEV );
		if ( !sp->wmn )
			sp->wmn = WMN;
	}
  spl0();
}


/*
 *	close off write
 */
bxwclose( dev )
{
	register struct stn *sp = &bx_stns[dev.d_minor];

  splbx();
	sp->s_flags =& ~WTG_FOR_WRT;
	sp->s_flags =| WCLOSE;
#	ifdef	ERROR_MESSAGE
	bxerrors();
#	endif	ERROR_MESSAGE
  spl0();
}


/*
 *	write data to line
 */
bxwrite( dev )
{
	struct stn *sp = &bx_stns[dev.d_minor];
	register struct buf *bp = sp->wbp;
	register char *flagp = &sp->s_flags;
	register count;

  do  {
	splbx();
		while ( *flagp & WRT_DATA_RDY )  {
			*flagp =| WTG_FOR_WRT;
			sleep( bp , WRITEPRI );
			*flagp =& ~WTG_FOR_WRT;
		}
	spl0();

	if ( bp->b_wcount = count = min( 512 , u.u_count ) )  {
		if ( count =& 01776 )
			iomove( bp , 0 , count , B_WRITE );
		if ( bp->b_wcount & 1 )
			bp->b_addr[count++] = cpass();
		bxwparity( count );
		*flagp =| WRT_DATA_RDY;
	}
  } while
	( u.u_count > 0 );
}


/*
 *	set parity bits on data in block and calculate lpc
 */
bxwparity( count , sp )
  struct stn *sp;
{
	register char *cp = sp->wbp->b_addr;
	register c, lpc;

  lpc = 0;
  do  {
	c = *cp & 0177;
	if ( partab[c] & 0100 )	/* illegal char */
		c = '?';
	c =| (partab[c] & 0200);
	lpc =^ c;
	*cp++ = c;
  } while
	( --count );

  sp->wlpc = lpc^sp->wmn^sp->ad2^(AD1^STXp^ETXp);
}


	/********
	 * read *
	 ********/

/*
 *	open for reading
 */
bxopenr( dev )
{
  splbx();
	bxopen( &bx_stns[dev.d_minor].s_flags , OPENR );
  spl0();
}


/*
 *	close off read
 */
bxrclose( dev )
{
	register struct stn *sp = &bx_stns[dev.d_minor];

  splbx();
	sp->s_flags =& ~OPENR ;
	sp->lmn = 0;
#	ifdef	ERROR_MESSAGE
	bxerrors();
#	endif	ERROR_MESSAGE
  spl0();
}


/*
 *	read data from line
 */
bxread( dev )
{
	struct stn *sp = &bx_stns[dev.d_minor];
	register struct buf *bp = sp->rbp;
	register char *flagp = &sp->s_flags;
	register count;
	int n;

  splbx();

  for (;;)  {
	if ( *flagp & RD_DATA_RDY )  {
		spl0();
		if ( count = (n = min( bp->b_wcount , u.u_count )) & 01776 )
			iomove( bp , 0 , count , B_READ );
		if ( n & 1 )
			passc( bp->b_addr[count] );
		/* geterror( bp ); */
		splbx();
		*flagp =& ~RD_DATA_RDY;
		if ( *flagp & GET_RD_BLK )
			*flagp =& ~GET_RD_BLK;
		else  {
			sp->rbp = 0;
			brelse( bp );
		}
		break;
	}

	if ( *flagp & GET_RD_BLK )  {
		spl0();
		sp->rbp = bp = getblk( NODEV );
		splbx();
		*flagp =& ~GET_RD_BLK;
	}

	sleep( flagp , READPRI );
  }

  spl0();
}


/*
 *	report any errors
 */
#ifdef	ERROR_MESSAGE
bxerrors()
{
	register struct error *ep = bxerr;
	register errs = 0;

  if ( bx.flags & ERRORS )  {
	do
	  	if ( ep->e_count )  {
			if ( errs++ == 0 )
				printf( "\nb1726 err:" );
	  		printf( " %s:%d" , ep->e_mess , ep->e_count );
	  		ep->e_count = 0;
	  	}
	while
	  	( (++ep)->e_mess );

	if ( errs )
		putchar( '\n' );
  }
}


#endif	ERROR_MESSAGE
	/**************
	 * interrupts *
	 **************/


/*
 *	deal with read interrupts
 */
bxrint()
{
	register char *state = &bx.rstate;
	register union
	{
		struct stn *sp;
		struct buf *bp;
		char *cp;
		int i;
	}
		c, x;
	extern bxstart(), bxone();

  x.i = BPADDR->dlrb;
  if ( bx.echoes > 0 )  {
	if ( bx.flags & NOECHO )  bx.echoes = 0;
	else  {
		bx.echoes--;
		return;
	}
  }

  if ( x.i < 0 )  {
	if ( x.i & 020000 )  bxerr[BREAK].e_count++;
	else  bxerr[OVERUN].e_count++;
	x.i = Y_error;
  }else
	if ( ( x.i ^ partab[(c.i = x.i & 0177)] ) & 0200 )  {  bxerr[PARITY].e_count++;  x.i = Y_error;  }
	else
		if ( c.i == EOT )  x.i = Y_EOT;
		else  {
			x.i = Y_default;
			bx.rlpc =^ c.i;
		}

  /*
  **	Here should be "switch ( states[state][x.i] )"
  **	but instead, for efficiency:-
  */
  goto in;

/** State **/
/** 0 **/

next:	(*state)++;
null:	return;

rset:	bxerr[RESET].e_count++;
reset1:	*state = 1;
	return;

/** 1/8 **/

ad1_c:	if ( c.i != AD1 )  goto idl;
	goto next;

/** 2 **/

ad2_c:
	for ( x.sp = bx_stns ; x.sp < &bx_stns[NSTATIONS] ; x.sp++ )
		if ( c.i == ((x.sp)->ad2 & 0177) )  {
			bx.station = x.sp;
			goto next;
		}
	goto idl;

/** 3 **/

polsel:	if ( c.i == POL )  goto next;
	if ( c.i != SEL )  goto idl;
select:	*state = 6;
	return;

/** 4 **/

testx:	if ( c.i != ENQ )  {
idl:		*state = 0;
		return;
	}
	if ( (x.i = bx.station->s_flags) & WRT_DATA_RDY )  {
send:		bx.wstate = 0;
		timeout( bxstart , SOHp , T_DELAY );
		goto next;
	}else  if ( x.i & WCLOSE )  {
		x.sp = bx.station;
		x.sp->wbp->b_wcount = 0;
		x.sp->wlpc = x.sp->ad2^x.sp->wmn^(AD1^STXp^ETXp);
		goto send;
	}else  {
		if ( !(x.i & OPENW) )  {
			if ( c.bp = bx.station->wbp )  {
				brelse( c.bp );
				bx.station->wbp = 0;
			}
			if ( !(x.i & OPENR) )  {
				for ( x.sp=bx_stns ; x.sp<&bx_stns[NSTATIONS] ; x.sp++ )
					if ( x.sp->s_flags & (OPENR|OPENW) )
						goto w_finish;
				dl11[BP] = 0;
				BPADDR->dlrs = 0;
			}
		}
w_finish:	c.i = EOTp;
w_idle:		*state = 0;
	}
w_out:	timeout( bxone , c.i , T_DELAY );
	return;

/** 5 **/

w_err:	bxerr[WERR].e_count++;
	goto reset1;
acknak:	if ( c.i == ACK )  goto sendok;
	if ( c.i != NAK )  goto w_err;
resend:	bxerr[WPAR].e_count++;
	*state = 4;
	goto send;

sendok:	bx.blks_sent++;
	c.sp = bx.station;
	c.sp->wmn =^ 0201;	/* flip seqn. no. ( & parity ) */
	if ( (x.i = c.sp->s_flags) & WTG_FOR_WRT )
		wakeup( c.sp->wbp );
	if ( x.i & WRT_DATA_RDY )
		c.sp->s_flags =& ~WRT_DATA_RDY;
	else  if ( x.i & WCLOSE )
		c.sp->s_flags =& ~(WCLOSE|OPENW);
	goto w_finish;

/** 6 **/

testr:	if ( c.i != ENQ )  goto idl;
	if ( *(c.cp = &(x.sp = bx.station)->s_flags) & OPENR )  {
		if ( x.sp->rbp && !(*(c.cp) & RD_DATA_RDY) )  {
			c.i = ACKp;
			bx.posn = x.sp->rbp->b_addr;
			bx.count = 0;
			(*state)++;
			goto w_out;
		}else  {
			*(c.cp) =| GET_RD_BLK;
			wakeup( c.cp );
		}
	}
	c.i = NAKp;
	goto w_idle;

/** 7 **/

soh:	if ( c.i != SOH )  goto idl;
	bx.rlpc = 0;
	goto next;

/** 9 **/

ad2_t:	if ( c.i != (bx.station->ad2 & 0177) )  goto idl;
	goto next;

/** 10 **/

xmn:	if ( c.i != '0' && c.i != '1' )  goto idl;
	bx.station->rmn = c.i;
	goto next;

/** 11 **/

stx:	if ( c.i != STX )
		goto idl;
	goto next;

/** 12 **/

recv:	if ( c.i == ETX )  goto next;
	if ( bx.count++ < 512 )
		*bx.posn++ = c.i;
	return;
rerr:	bxerr[RECVBAD].e_count++;
	return;

/** 13 **/

lpc:	if ( !bx.rlpc )  {
		x.sp = bx.station;
		if ( x.sp->rmn != x.sp->lmn )  {
			bx.blks_rcvd++;
			x.sp->lmn = x.sp->rmn;
			x.sp->rbp->b_wcount = bx.count;
			x.sp->s_flags =| RD_DATA_RDY;
			wakeup( &x.sp->s_flags );
		}
		c.i = ACKp;
		goto w_idle;
	}
	bxerr[RECVLPC].e_count++;
	c.i = NAKp;
	goto w_idle;


in:
  {
	static *bxrm_c[][3]
	{
	/*        EOT   ?   BAD  */
	/* 0  */ next,null,null,
	/* 1  */ rset,ad1_c,idl,
	/* 2  */ rset,ad2_c,idl,
	/* 3  */ rset,polsel,idl,
	/* 4  */ rset,testx,idl,
	/* 5  */ w_err,acknak,idl,
	/* 6  */ rset,testr,idl,
	/* 7  */ rset, soh, idl,
	/* 8  */ rset,ad1_c,idl,
	/* 9  */ rset,ad2_t,idl,
	/* 10 */ rset, xmn, idl,
	/* 11 */ rset, stx, idl,
	/* 12 */ rset,recv,rerr,
	/* 13 */  lpc, lpc, lpc
	};

	goto bxrm_c[*state][x.i];	/* yes folks - a computed goto */
  }
}


/*
 *	transmit 1 char & start write interrupts
 */
bxstart( c )
{
  BPADDR->dlrs = 0;
  BPADDR->dlwb = c;
  BPADDR->dlws = IENABLE;
}



/*
 *	transmit 1 char
 */
bxone( c )
{
	register  x;

  BPADDR->dlwb = c;
  x = BPADDR->dlrb;
  bx.echoes = 1;
}


/*
 *	deal with write interrupts
 */
bxwint()
{
	register c, x;

  switch ( bx.wstate )  {
	case 0:	c = AD1; break;
	case 1: c = bx.station->ad2; break;
	case 2:	c = bx.station->wmn; break;
	case 3: c = STXp;
		bx.count = bx.station->wbp->b_wcount;
		bx.posn = bx.station->wbp->b_addr;
		break;
	case 4: if ( bx.count-- == 0 )  {
			c = ETXp; break;
		}
		BPADDR->dlwb = *bx.posn++;
		return;
	case 5: c = bx.station->wlpc;
		bx.echoes = 2;
		x = BPADDR->dlrb;
		BPADDR->dlws = 0;
		BPADDR->dlrs = IENABLE;
  }

  bx.wstate++;
  BPADDR->dlwb = c;
}


#ifdef	POWER_FAIL
/*
 *	restart after a power restore
 */
bxpowerf()
{
	register struct stn *sp;

  for ( sp=bx_stns ; sp < &bx_stns[NSTATIONS] ; sp++ )
	if ( sp->s_flags & (OPENR|OPENW) )  {
		bx.echoes = 0;
		BPADDR->dlrb = IENABLE;
		return;
	}
}
#endif
