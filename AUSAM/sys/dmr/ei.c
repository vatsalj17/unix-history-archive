#
/*
 *	copyright  march 1976 ian johnstone.
 *
 *	cdc u200 emulator.
 */

#include	"../defines.h"
#include	"../param.h"
#include	"../conf.h"
#include	"../user.h"
#include	"../buf.h"


extern	char	partab[];

#define	EISPL	spl6
#define	ps	PS->integ
#ifdef	_1170
#define	EISW	050		/* 11/70 uses switch register differently */
#else
#define	EISW	0177570
#endif	_1170

	/*  dp11  device  registers  */

#define	DPADDR	0174770
#define	DPRCSR	DPADDR->xxrcsr
#define	DPRBUF	DPADDR->xxrbuf
#define	DPSYN0	DPADDR->xxsyn0
#define	DPTCSR	DPADDR->xxtcsr
#define	DPTBUF	DPADDR->xxtbuf

struct {
	int	xxrcsr;	/* receive status register	*/
	char	xxrbuf;	/* receive buffer		*/
	char	xxsyn0;	/* sync register		*/
	int	xxtcsr;	/* transmit status register	*/
	char	xxtbuf;	/* transmit buffer		*/
	char	syn1;
       };

#define	ODDPAR	010000
#define	HDUPLX      02
#define	STRPSYNC    01
#define	CTRANS 0100000
#define	RORUN	040000
#define	RING	020000
#define	DSRDY	010000
#define	CARRIER  04000
#define	DONE	  0200
#define	IENABLE	  0100
#define	SIENABLE   040
#define	IDLESYNC    02

/*
 *	buffers and buffer pointers
 */

		/* receive buffer pointers */
char	ycrbuffer[1050];
char	*ycrbuf		ycrbuffer;
char	*ycrbufe	ycrbuffer+1049;
char	*ycrbufp	ycrbuffer;

		/* transmit buffer pointers */
char	*yctbufp;

		/* card buffer pointers */
int	ycjpadding;
char	ycjbuffer[984-14];
struct	buf	ycjbaddr
	{
		B_PHYS,0,0,0,0,0,0,
		/* for iomove */
		ycjbuffer
	};

		/* print buffer pointers */
char	yclbuffer[1050-14];
struct	buf	yclbaddr
	{
		B_PHYS,0,0,0,0,0,0,
		/* for iomove */
		yclbuffer
	};
#define	yclbuf	yclbaddr.b_addr
char	*yclbufp	yclbuffer;
char	*yclbufe	yclbuffer+1049;
int	yclbufl;

		/* answer buffer pointers */
char	ycabuf[740];		/* enow for ??? char msgs */
int	ycabufl;
char	*ycabufe;

		/* command buffer pointers */
char	yccbuf[22];
int	yccbufl		 22-(1+3);

/*
 *	flags to control all
 */

int	ycflag;

#define	CRDY	(1<<0)			/* =| if cmd queued to cyber */
#define	RRDY	(1<<1)			/* =| if message from cyber avail */
#define	RUSE	(1<<2)			/* =| if message being passed */

#define	JRDY	(1<<3)			/* =| another batch of cards ready */
#define	RDE2	(1<<4)			/* =| if 'read e2' sent for reader */

#define	WJOPN	(1<<5)			/* =| cyb.jobs open for write */
#define	RJOPN	(1<<6)			/* =| cyb.jobs open for read */
#define	WCOPN	(1<<7)			/* =| cyb.cmds open for write */
#define	RCOPN	(1<<8)			/* =| cyb.cmds open for read */

#define	JFAVL	(1<<9)			/* =| set if print buffer ready */
#define	PRE2	(1<<10)			/* =| if 'read e2' sent for printer */

#define	WPRT	(1<<11)			/* =| if waiting for print output */
#define	WCRD	(1<<12)			/* =| if waiting for card buffer */
#define	WMSG	(1<<13)			/* =| if waiting for message */
#define	WCOM	(1<<14)			/* =| if waiting for command buffer */


#define	cyopn	(WJOPN + RJOPN + WCOPN + RCOPN)

int	yceoj;

#define	FINJ	01
#define	ENDJ	02

/*
 *	symbolic equates
 */

#define	this_site	0160
#define	stoh		0001
#define	eot		0003
#define	qeot		0203
#define	escp		0076
#define	qescp		0276
#define	reject		0030		/* u-200 */
#define	qreject		0230
#define	sync		0026		/* u-200 */
#define	error		0025		/* u-200 */
#define	qerror		0225
#define	read		0023		/* u-200 */
#define	qread		0223
#define	ack		0006		/* u-200 */
#define	qack		0206
#define	write		0021		/* u-200 */
#define	cwrite		0022		/* u-200 */
#define	rwrite		0014		/* u-200 */
#define	poll		0005		/* u-200 */
#define	alert		0007		/* u-200 */
#define	ff		0014
#define	vt		0013
#define	space3		0040		/* cyber */
#define	space2		0112		/* cyber */
#define	space1		0120		/* cyber */
#define	space0		0060		/* cyber */
#define	eject		0101		/* cyber */
#define	skipc4		0105		/* cyber */
#define eoj		0126		/* cyber */
#define	ceol		0120		/* cyber */
#define	ccr		0101		/* cyber */
#define	lowbcd		0040
#define	highbcd		0137
#define	e1		0102
#define	qe1		0302
#define	e2		0040
#define qe2		0240
#define	e3		0041
#define	qe3		0241
#define	eoi		0126		/* cyber */
#define	qeoi		0326
#define	eor		0127		/* cyber */
#define	qeor		0327
#define	ctrlr		0022
#define	null		0000

/*
 *	cyber talking control words
 */

char	ycerrf;				/* non-zero => receive error */
	/*
	 *	0 ==> o.k.
	 *	1 ==> format error in received msg.
	 *	2 ==> lpc error in received msg.
	 *	3 ==> Carrier drop on line (hard)
	 *	4 ==> Ring detected? (hard)
	 *	5 ==>  Receiver overrun. Too slow response to interupt.
	 *	6 ==> last msg. transmitted no good.
	 *	7 ==> last msg. received too long.
	 *	8 ==> talk syncronizing error.
	 */

	/*
	 * short error description 
	 *		 1  2  3  4  5  6  7  8	*/
char	*ycerrfm	"fmtlpccarrinovrxmtlenpro" ;
int	ycerrfc[8];			/* error counts */

#define	fmterr	1
#define	lpcerr	2
#define	carerr	3
#define	rinerr	4
#define	ovrerr	5
#define	xmterr	6
#define	lenerr	7
#define	proerr	8


char	ycsite;				/* site address from last msg. */
char	ycstat;				/* station address from last msg. */
char	yccode;				/* control code from last msg. */
char	ycetyp;				/* type code from last msg. */
char	ycwstat	     0141;		/* station addr from last write msg. */
char	yctstat;			/* station addr to be transmitted */

char	*ycbmsg[10];			/* a 10 msg stack */
int	ycb;				/* msg stack pointer */
int	ycnsync		1;
char	*ycmsg	       -1;		/* ptr to last 'msg' sent to cyber */
char	*ycnowmsg;			/* ptr to an immediate msg */


int	ycrnext		1;
int	yctnext		1;

/*
 *	std. messages for cyber
 */

char	ycackm[]
	{
		qack, qeot
	};
char	ycrejm[]
	{
		qreject, qeot
	};
char	ycerrm[]
	{
		qerror, qeot
	};
char	ycreed[]
	{
		qread, 'r', qescp, qe1, qeot
	};
char	ycgo[]
	{
		qread, 'g', qescp, qe1, qeot
	};
char	yccont[]
	{
		qread, 'c', qescp, qe1, qeot
	};
char	ycbye[]
	{
		qread, 'b',  qescp, qe1, qeot
	};
char	yclog[]
	{
		qread, 'l',',','0','0','e','l','e','c','b', qescp, qe1, qeot
	};
char	yce1m[]
	{
		qread,	qescp, qe1, qeot
	};
char	yce2m[]
	{
		qread,	qescp, qe2, qeot
	};
char	yce3m[]
	{
		qread,	qescp, qe3, qeot
	};

char	ycdiag1[]	"\nmodem not ready\n" ;
char	ycdiag2[]	"\nlinerr    ";

int	ycsusp;
#define CONTINUED	0


char	ycbtoa[]		/* bcd to ascii conversion */
	{   /*	 00   01   02   03   04   05   06   07   */	/******/
		0055,0112,0113,0114,0115,0116,0117,0120,	/* 00 */
		0121,0122,0041,0044,0052,0136,0043,0076,	/* 10 */
		0053,0101,0102,0103,0104,0105,0106,0107,	/* 20 */
		0110,0111,0074,0056,0051,0077,0072,0073,	/* 30 */
		0072,0061,0062,0063,0064,0065,0066,0067,	/* 40 */
		0070,0071,0060,0075,0047,0134,0045,0133,	/* 50 */
		0040,0057,0123,0124,0125,0126,0127,0130,	/* 60 */
		0131,0132,0135,0054,0050,0046,0042,0100		/* 70 */
	};

char	ycatob[]		/* ascii to bcd conversion */
	{   /*	 00   01   02   03   04   05   06   07   */	/*******/
		0120,0052,0136,0056,0053,0116,0135,0114,	/* 000 */
		0134,0074,0054,0060,0133,0040,0073,0121,	/* 010 */
		0112,0101,0102,0103,0104,0105,0106,0107,	/* 020 */
		0110,0111,0100,0077,0072,0113,0057,0075,	/* 030 */
		0137,0061,0062,0063,0064,0065,0066,0067,	/* 040 */
		0070,0071,0041,0042,0043,0044,0045,0046,	/* 050 */
		0047,0050,0051,0122,0123,0124,0125,0126,	/* 060 */
		0127,0130,0131,0117,0115,0132,0055,0120,	/* 070 */
		0120,0061,0062,0063,0064,0065,0066,0067,	/* 100 */
		0070,0071,0041,0042,0043,0044,0045,0046,	/* 110 */
		0047,0050,0051,0122,0123,0124,0125,0126,	/* 120 */
		0127,0130,0131,0120,0120,0120,0120,0120		/* 130 */
	};

/*
 *	cyber commands
 */

ycopen(dev, flag)
{
	register int x;

	if( flag == 0 )
		x = RCOPN;
	else
		x = WCOPN;
	if( (ycflag & x) || cystart() )
		u.u_error = ENXIO;
	else 
		ycflag =| x;
}

ycclose(dev, flag)
{
	ycflag =& ~(flag == 0 ? RCOPN : WCOPN);
}

ycmsgout(msg, len)		/* place msgs for cyb.cdn */
char *msg;
int len;
{
	char register *x,*y,*z;

	if( !(ycflag & RUSE) )
	{
		if( (z = ycabufe-ycabuf) > (y = len))
			z = ycabufl = y;
		else
			ycabufl = z;
		x = ycabuf; y = msg;
		while(z--)
			*x++ = *y++;
		ycflag =| RRDY;
		if( ycflag & WMSG)
		{
			ycflag =& ~WMSG;
			wakeup( &ycabuf);
		}
		return(x);	/* indicate all okay */
	};
	return(0);
}

ycwrite()			/* to send commands to cyber */
{
	register char x, *y;

	EISPL();

	if( (ycflag & CRDY) || (u.u_count > yccbufl))
		u.u_error = ENXIO;
	else
	{
		y = yccbuf;
		*y++ = qread;
		while ((x = cpass()) > 0)
			*y++ = x & 0177;
		*y++ = qescp;
		*y++ = qe1;
		*y++ = qeot;
		switch( yccbuf[1])
		{
	    case 'b':
	    case 'c':	ycsusp = CONTINUED;
			break;
	    case 's':	ycsusp++;
			break;
		}
		ycflag =| CRDY;
		ycqmsg(yccbuf);
	};
	spl0();
}

ycread()		/* to read response from cyber */
{
	register char *x;
	register int y;

	EISPL();
	while( !(ycflag & RRDY))
	{
		ycflag =| WMSG;
		sleep(&ycabuf,1);
	}
	ycflag =& ~RRDY;
	ycflag =|  RUSE;
	spl0();
	x = ycabuf; 
	y = (ycabufl > u.u_count) ? u.u_count : ycabufl;
	while(y--)
		passc(*x++);
	ycflag =& ~RUSE;
};

/*
 *	Cyber jobs
 */

yjopen(dev, flag)
{
	register int x;

	if( flag == 0 )
		x = RJOPN;
	else
		x = WJOPN;
	if( (ycflag & x) || cystart() )
		u.u_error = ENXIO;
	else 
		ycflag =| (flag == 0 ? (RJOPN | PRE2) : (WJOPN | RDE2 ));
}

yjclose(dev, flag)
{
	if( flag )
		ycflag =& ~(WJOPN | JRDY | RDE2);
	else
		ycflag =& ~(RJOPN | JFAVL);
}

yjwrite()			/* to send jobs to the cyber */
{
	register int n, m;

	EISPL();
	while( ycflag & JRDY)
	{
		ycflag =| WCRD;
		sleep(&ycjbuffer, 1);
	}
	spl0();
	ycjbaddr.b_addr = ycjbuffer;
	if( m = (n = u.u_count) & 01776 )
		iomove(&ycjbaddr, 0, m, B_WRITE);
	if( n & 1)
		ycjbuffer[m++] = cpass();
	if( !m)
		return;
	ycjbuffer[m++] = qescp;
	ycjbuffer[m++] =   qe3;
	ycjbuffer[m++] =  qeot; 
	EISPL();
	ycflag =| JRDY;
	if( ycflag & RDE2 )
	{
		ycqmsg(ycreed);
		ycflag =& ~RDE2;
	}
	spl0();
}

yjread()			/*
				 * To read jobs from cyber.
				 * As much as possible each time.
				 */
{
	register int n, m;

	EISPL();
	if( yceoj & ENDJ)		/* terminate last job */
	{
		yceoj =& ~ENDJ;
		u.u_error = ENXIO;
		spl0();
		return;
	}
	while( !(ycflag & JFAVL))
	{
		ycflag =| WPRT;
		sleep(&yclbuf, 1);
	}
	if( yceoj & FINJ)
		yceoj = ENDJ;
	spl0();
	n = (u.u_count < yclbufl ? u.u_count : yclbufl);
	m = n & 03776;
	if( m )
		iomove(&yclbaddr, 0, m, B_READ);
	if( n&1 )
		passc( yclbuf[m]);
	EISPL();
	ycflag =& ~JFAVL;	/* buffer all used */
	spl0();
}

ycawful()		/*
			 * Kick Cyber up the Kyber
			 *
			 * Once every twenty seconds ...
			 */
{
	if( (ycflag & cyopn))
	{
		if( ycb < 2 )
		{
			if(ycsusp == CONTINUED)
				ycqmsg(yccont);
			ycqmsg(ycgo);
		}
		timeout(ycawful, 0, 20*HZ);
	}
}

ycqmsg(mp)		/* place commands for cyber in 'fifo' queue */
char *mp;
{
	register int x, y;
	
	if( ycb > 8 )
		return;
	y = ps;
	EISPL();
	x = ++ycb;
	while( x > 1 )
		ycbmsg[x-1]  =  ycbmsg[(x--) - 2];
	ycbmsg[0] = mp;
	ps = y;
}

ycerep(en)		/* called to log errors && optionally tell op */
int en;
{
	register char *x,*y;

	ycerrfc[en - 1]++;
	if(EISW->integ & 020000)
		return;
	x = &ycerrfm[en*3 - 3];
	y = ycdiag2 + 8;
	*y++ = *x++; *y++ = *x++; *y = *x;
	ycmsgout(ycdiag2, 11);
}
ycwpro()		/*
			 * Process cyber write commands
			 * Called from receive interrupt routine
			 */
{
	register char *x, *y;
	register int *z;

	if( ycmsg == &ycjbuffer[-1])
	{
		ycflag =& ~JRDY;
		if( ycflag & WCRD)
		{
			ycflag =& ~WCRD;
			wakeup( &ycjbuffer);
		}
	}
	else if( ycmsg == yccbuf)
		ycflag =& ~CRDY;
	/*
	 * last 'msg' to cyber was accepted
	 */

	ycmsg = 0;
	
	/*
	 * process write according to type code
	 */
	switch( ycetyp)
	{
    case e1:
		/*
		 * if bit #12 in swreg is set throw away useless crap
		 */
		if(EISW->integ & 010000)
		{
			z = ycrbuf;
			if( (z[0] == ' R') ||	/* ' READY' */
			    (z[0] == ' C') ||	/* ' CARD READER NOT READY' */
			    (z[1] == 'TE') ||	/* '  TERMINAL IDLE */
			    (z[1] == 'RI') ||	/* ' PRINTER NOT READY' */
			    (z[6] == 'SU') ||	/* ' NO FILE IS SUSPENDED' */
			    (z[1] == 'OO'))	/* ' TOO MANY JOBS' */
					break;
		}
		/*
		 * can't just ignore ' LAST JOB READ' messages
		 */
		if( (!ycmsgout(ycrbuf, ycrbufp - ycrbuf)) && (z[0] == ' L'))
		{
			yctbufp = ycrejm;
			return;
		}
		break;

		/*
		 * print output
		 */
    case e2:
		/* if cant accept pretend not ready */
		if( (ycflag & JFAVL) || (!(ycflag & RJOPN)) )
		{
			ycnowmsg = yce2m;
			ycflag =| PRE2;
			break;
		}
		x = ycrbufp;
		y =  ycrbuf;
		z = ycrbufe;
		/*
		 * Adjust first char
		 */
		switch(*y)
		{
	    default:	*y = '\n';
	    case '\n':
	    case '\r':
	    case   ff:
	    case   vt:	break;
		}
		ycrbufp = ycrbuf = yclbuf;
		ycrbufe = yclbufe; 
		yclbufp = yclbuf = y;
		yclbufe = z;
		yclbufl = x - y;
		ycflag =| JFAVL;
		if( ycflag & WPRT)
		{
			ycflag =& ~WPRT;
			wakeup( &yclbuf);
		}
		/*
		 * tell cyber want more output
		 */
		if( !ycb )
			ycnowmsg = yce3m;
		break;

		/*
		 * Card input
		 */
    case e3:
		if( ycflag & JRDY )
		{
			ycnowmsg = &ycjbuffer[-1];
			ycjbuffer[-1] = qread;
		}
		else
		{
			/*
			 * No cards to go so go not ready
			 */
			ycnowmsg = yce2m;
			ycflag =|  RDE2;
		}
	}
	yctbufp = ycackm;		/* acknowlege receipt of write */
}

/*
 * Receive interrupt routine
 */

#define	YCRSTOH		1
#define	YCRSITA		2
#define	YCRSTAA		3
#define	YCRCC		4
#define	YCRMSG		5
#define	YCRMSG1		6
#define	YCRMSG2		7
#define	YCREND		8
#define	YCREOT		9
#define	YCRLPC		10
#define	YCRERR		11

cyrint()
{
	register int c;
	static int lpc;

	/*
	 * Loop while char available
	 */
	while( DPRCSR & DONE )
	{
		c = DPRBUF & 0177;		/* obtain next char sans parity */
		lpc =^ c;			/* calc longtitudinal parity */
	
		if( !(DPRCSR & ODDPAR) )
			ycrnext = YCRERR;
	
		/*
		 * Message sequence
		 */
		switch(ycrnext)
		{
	
	    case YCRSTOH:
			/*
			 * first char must be start of header
			 */
			if( c != stoh )
				goto ycrerror;
	    ycrstoh1:
			ycrnext = YCRSITA;
			lpc = stoh;
			continue;
	
	    case YCRSITA:
	
			/*
			 * second char <=0177 && >=0160
			 */
			if( c<0160 )
				goto ycrerror;
			ycrnext = YCRSTAA;
			ycsite = c;
			continue;
	
	    case YCRSTAA:
			/*
			 * third char 0140, 0141, 0160, 0161
			 */
			switch(c)
			{
		    default:	goto ycrerror;
	
		    case 0140:
		    case 0141:
		    case 0160:
		    case 0161:	ycrnext = YCRCC;
				ycstat = c;
			}
			continue;
	
	    case YCRCC:
			/*
			 * Fourth must be acceptable command 
			 */
			switch(c)
			{
		    default:	goto ycrerror;
	
		    case write:
		    case cwrite:
		    case rwrite:
				yccode = write;
				ycrnext = YCRMSG;
				continue;
		    case poll:
		    case alert:
				yccode = c;
				ycrnext = YCREOT;
			   	continue;
			}
	
	    case YCRMSG:
			/*
			 * Process data portion of message
			 */
	
			ycrnext = YCRMSG2;
	
			/*
			 * Carriage control
			 */
			switch(c)
			{
		    case space3:
				*ycrbufp++ = '\n';
		    case space2:
				*ycrbufp++ = '\n';
		    default:
				if( ycrbufp == ycrbuf)
					goto ycrtr;
				c = '\n';
				break;
		    case space0:
				c ='\r';
				break;
		    case eject:
				c = ff;
				break;
		    case skipc4:
				c = vt;
				break;
		    case escp:
				ycrnext = YCRMSG1;
				continue;
			};
			goto ycrmsg3;
	
	    case YCRMSG1:
			/*
			 * Function code
			 */
			switch(c)
			{
		    case e1:
		    case e2:
		    case e3:	/* ending code */
				DPRCSR =& ~STRPSYNC;
				ycetyp = c;
				ycrnext = YCREOT;
				continue;
		    case eoj:
				yceoj = FINJ; 
		    case ceol:
		    case ccr:
				ycrnext = YCRMSG;
				continue;
		    default:		/*
					 * '0' or ' ' expansion
					 */
				if( ((c =- 040) >= 3) && (c <= 037))
					*ycrbufp++ = 0377;
				else if( ((c =- 040) >= 3) && (c <= 017))
					*ycrbufp++ = 0376;
				else
					c = ' ';
				ycrnext = YCRMSG2;
				goto ycrmsg3;
			}
	
	    case YCRMSG2:
			/*
			 * BCD data
			 */
			switch(c)
			{
		    case escp:
				ycrnext = YCRMSG1;
				continue;
		    default:
	    ycrtr:
				if( (c > highbcd) || (c < lowbcd))
					c = ' ';
				else
					c = ycbtoa[c - lowbcd];
			}
	
	    ycrmsg3:
			if( ycrbufp > ycrbufe )
			{
				if( !ycerrf )
					ycerrf = lenerr;
				goto ycrerros;
			}
			*ycrbufp++ = c;
			continue;
	
			/*
			 * Look for end of message
			 */
	    case YCREND:
			if( c == stoh )
			{
				ycrbufp = ycrbuf;
				goto ycrstoh1;
			}
			continue;
	
			/*
			 * This char must be "eot"
			 */
	    case YCREOT:
			if( c != eot )
				goto ycrerror;
			ycrnext = YCRLPC;
			continue;
	
	    case YCRERR:
	ycrerror:
			if( !ycerrf )
				ycerrf = fmterr;
	ycrerros:
			ycrnext = YCREND;
			DPRCSR =& ~STRPSYNC;
			continue;
	
			/*
			 * This char is longtitudinal parity
			 */
	    case YCRLPC:
			if( (lpc != 0177) && !ycerrf )
				ycerrf = lpcerr;
			DPTCSR = 0;	/* terminate input */
			DPRCSR = 0;	/* terminate input */
			ycrnext = YCRSTOH;	/* =| for next receive */
	
	
			if( ycsite != this_site )
			{
				DPSYN0 = sync;
				DPRCSR = HDUPLX | STRPSYNC | IENABLE;
				DPTCSR =| SIENABLE;
			}
			else
			{
				if( ycerrf )
				{
					ycerep(ycerrf);
					ycerrf = 0;
					yctbufp = ycerrm;
				}
				else
				{
					if( ycstat & 1 )
					/*
					 * Odd station address
					 */
					switch(yccode)
					{
				    case write:
						/* save write address */
						ycwstat = ycstat;
						ycwpro();
						break;
				    case alert:
						yctbufp = ycackm;
						ycqmsg(yce1m);
						break;
				    default:	/* ??????? */
						yctbufp = ycerrm;
						ycerep(proerr);
					}
					else
					/*
					 * Even station address
					 */
					switch(yccode)
					{
				    case poll:
	
						if( ycmsg == -1)
						{
							ycmsg = 0;
							yctbufp = ycrejm;
						}
						else if( (yctbufp = ycmsg) )
							ycerep(xmterr);
						else if( ycnowmsg )
						{
							yctbufp = ycmsg = ycnowmsg;
							ycnowmsg = 0;
						}
						else
							yctbufp = (ycb ? (ycmsg = ycbmsg[--ycb]) : ycrejm);
						break;
	
	
				    default:	yctbufp = ycerrm;
						ycerep(proerr);
					}
				/*
				 * Reject to 'poll' is different
				 */
					if( (yccode == poll) && (*yctbufp == 0177630))
						yctstat = ycwstat & 0176;
					else
						yctstat = ycwstat;
				}
				ycrbufp = ycrbuf;
				DPTCSR = DSRDY | IENABLE | IDLESYNC;
				DPRCSR = HDUPLX;
				DPTBUF = sync;
			}
			return;
		} /*
		   * Switch
		   */
	} /*
	   * While
	   */
} /*
   * Cyrint
   */

/*
 * Transmitter status interrupt routine
 *
 * 'yctbufp' points to message to be sent to Cyber
 * 'qeot' signals end of message.
 * Any char with parity bit on is not translated.
 */

#define	YCXSYNC		1
#define	YCXSTOH		2
#define	YCXSITA		3
#define	YCXSTAA		4
#define	YCXMSG		5
#define	YCXLPC		6
#define	YCXEND		7

cyxint()
{
	register c;
	static int lpc;

	if( DPTCSR & SIENABLE )
	{
		/*
		 * Receive error
		 */
		if( !ycerrf )
		{
			if( DPTCSR & RORUN )
				ycerrf = ovrerr;
			if( DPTCSR & RING )
				ycerrf = rinerr;
			if( DPTCSR & CTRANS )
				ycerrf = carerr;
		}
		DPTCSR =& ~(CTRANS | RORUN | RING);
		DPSYN0 = sync;	/* reset just in case */
		return;
	}
	/*
	 * Transmit message sequence
	 */
	switch(yctnext)
	{
		/*
		 * Send 4 syncs
		 */
    case YCXSYNC:
		c = sync;
		if( ++ycnsync == 4 )
			yctnext = YCXSTOH;
		goto ycxmit;

		/*
		 * 1st char is start of header
		 */
    case YCXSTOH:
		c = stoh;
		yctnext = YCXSITA;
		lpc = 0;
		goto ycxmit;

		/*
		 * 2nd char is site address
		 */
    case YCXSITA:
		c = ycsite;
		yctnext = YCXSTAA;
		goto ycxmit;

		/*
		 * 3rd char is station address
		 */
    case YCXSTAA:
		c = yctstat;
		yctnext = YCXMSG;
		goto ycxmit;

		/*
		 * Output data till eot found
		 */
    case YCXMSG:
		c = *yctbufp++;
		if( c & 0200 )
		{
			c =& 0177;
			if( c == eot )
				yctnext = YCXLPC;
		}
		else if( c<' ' )
			c = 0120;
		else
			c = ycatob[c - 040];
ycxmit:
		c =| (( ~partab[c]) & 0200);	/* odd parity gen */
		lpc =^ c;			/* long. parity */
		DPTBUF = c;
		return;

		/*
		 * Output long. parity
		 */
    case YCXLPC:
		c = (~lpc) & 0177;
		yctnext = YCXEND;
		goto ycxmit;

		/*
		 * All msg sent - tidy up
		 */
    case YCXEND:
		if( ycflag & cyopn )
		{
			DPSYN0 = sync;	/* =| Just in case */
			DPRCSR = HDUPLX | STRPSYNC | IENABLE;	/* enable receive */
			DPTCSR = SIENABLE;			/* enable receive */
		}
		else
		{
			DPRCSR = 0;	/* no open devices so close down */
			DPTCSR = 0;	/* no open devices so close down */
		}
		ycrnext = YCRSTOH;
		yctnext = YCXSYNC;
		ycnsync = 1;
	}
}

/*
 * Initiate receive on dp-11 if necessary
 */
cystart()
{
	if( (DPTCSR & DSRDY) == 0 )
	{
		printf(ycdiag1); /* modem not ready */
		return(1);
	}
	if( !(ycflag & cyopn) )
	{
		DPSYN0 = sync;
		DPRCSR = HDUPLX | STRPSYNC | IENABLE;
		DPTCSR = SIENABLE;

		/* reinit necessary variables */
	
		ycerrf = ycflag = ycb = ycsusp = 0;
		ycmsg = -1;
		ycrbuf = ycrbufp = ycrbuffer;
		ycrbufe = ycrbuffer + 1049;
		yclbuf = yclbufp = yclbuffer;
		yclbufe = yclbuffer + 1049;
		ycrnext = YCRSTOH;
		yctnext = YCXSYNC;
		ycnsync = 1;
		ycabufe = &ycabuf[730];
		ycqmsg(ycbye);
		ycqmsg(yclog);
		timeout(ycawful, 0, 60*HZ);	/* Give batch a little while to settle in */
	}
	return(0);
}
