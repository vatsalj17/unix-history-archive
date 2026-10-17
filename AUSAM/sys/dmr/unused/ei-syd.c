#
/*
 *	cdc u200 emulator.
 *
 *	copyright	march 1976 ian johnstone.
 *
 *	amended by Piers Lauder for DU11 interface
 *		Sep 1977
 */

#include	"../param.h"
#include	"../conf.h"
#include	"../user.h"
#include	"../buf.h"

#include	"du.h"

#ifdef	POWER_FAIL
#define	PUDELAY	1*HZ	/* wait one second before restart */
#define	PURETRY	5*HZ	/* wait five secs before trying again */
#define	PATIENCE 5	/* give up after 5 retrys */
#endif



struct	{ int pxx; };
#define	ps	0177776->pxx

/* buffers and buffer pointers */

  /* receive buffer pointers */
char	ycrbuffer[1050];
char	*ycrbuf		ycrbuffer;
char	*ycrbufe	ycrbuffer+1049;
char	*ycrbufp	ycrbuffer;

  /* transmit buffer pointers */
char	*yctbufp	0;

  /* card buffer pointers */
int	ycjpadding 0;
char	ycjbuffer[984-14]	0;
struct buf ycjbaddr { 0,0,0,0,0,0,0,ycjbuffer };

  /* print buffer pointers */
char	yclbuffer[1050-14];
struct buf yclbaddr { 0,0,0,0,0,0,0,yclbuffer };
#define	yclbuf	yclbaddr.b_addr
char	*yclbufp	yclbuffer;
char	*yclbufe	yclbuffer+1049;
int	yclbufl		0;

  /* answer buffer pointers */
char	ycabuf[740]	0;
int	ycabufl		0;
#define	ycabufe		(&ycabuf[730])

  /* command buffer pointers */
char	yccbuf[22]	0;
#define	yccbufl		(22-(1+3))

/* flags to control all */
int	ycflag	0;
int	yceoj	0;

#define	crdy	(1<<0)	/* set if cmd queued to cyber */
#define	rrdy	(1<<1)	/* set if message from cyber avail */
#define	ruse	(1<<2)	/* set if message being passed */

#define	jrdy	(1<<3)	/* another batch of cards ready to go */
#define	rde2	(1<<4)	/* set if a 'read e2'	sent for card reader */
#define	wjopn	(1<<5)	/* cyb.jobs open for write */

#define	rjopn	(1<<6)	/* cyb.jobs open for read */
#define	wcopn	(1<<7)	/* cyb.cntrl open for write */
#define	rcopn	(1<<8)	/* cyb.cntrl open for read */

#define	jfavl	(1<<9)	/* set if print buffer ready */
#define	pre2	(1<<10)	/* set if a 'read e2' sent for printer */
#define	wprt	(1<<11)	/* set if waiting for prt output */

#define	cyopn	((1<<5)+(1<<6)+(1<<7)+(1<<8))


/* symbolic equates */

#define	this_site	0170
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
#define	space1				/* cyber */
#define	space0		0060		/* cyber */
#define	eject		0101		/* cyber */
#define	skipc4		0105		/* cyber */
#define	eoj		0126		/* cyber */
#define	ceol		0120		/* cyber */
#define	ccr		0101		/* cyber */
#define	lowbcd		0040
#define	highbcd		0137
#define	e1		0102
#define	qe1		0302
#define	e2		0040
#define	qe2		0240
#define	e3		0041
#define	qe3		0241
#define	eoi		0126		/* cyber */
#define	qeoi		0326
#define	eor		0127		/* cyber */
#define	qeor		0327
#define	ctrlr		0022
#define	null		0000

/* cyber talking control words */

char	ycrerrs		0;	/* non-zero => xmit errors */
char	ycerrf		0;	/* non-zero => receive error */
	/*
	 * 0 ==> o.k.
	 * 1 ==> format error in received msg.
	 * 2 ==> lpc error in received msg.
	 * 3 ==> hardware detected receive error.
	 * 4 ==> last msg. transmitted no good.
	 * 5 ==> last msg. received too long.
	 * 6 ==> talk syncronizing error.
	 * 7 ==> hardware detected char overun error
	 * 8 ==> hardware detected char parity error
	 */

	/* short error description
				 1  2  3  4  5  6  7  8 */
	char	ycerrfm[]	"fmtlpcrecxmtlenproovrpar";
	int	ycerrfc[]		/* error counts */
	{	0,0,0,0,0,0,0,0	};

char	ycsite		0;	/* site address from last msg. */
char	ycstat		0;	/* station address from last msg. */
char	yccode		0;	/* control code from last msg. */
char	ycetyp		0;	/* } code from last msg. */
char	ycwstat		0141;	/* station address from last write msg. */
char	yctstat		0;	/* station address to be transmitted */

char	*ycbmsg[10]	0;	/* a 10 msg stack */
int	ycb		0;	/* msg stack pointer */
int	ycnsync		1;
char	*ycmsg		-1;	/* ptr to last 'msg' sent to cyber */
char	*ycnowmsg	0;	/* ptr to an immediate msg */


	int	ycrnext	1;
	int	yctnext	1;

/* std.	messages for cyber */

char	ycackm[]	{	qack, qeot			};
char	ycrejm[]	{	qreject, qeot			};
char	ycerrm[]	{	qerror, qeot			};
char	yclogin[]	{	qread, 'l',',','u','n','i','x', qescp, qe1, qeot	};
char	ycreed[]	{	qread, 'r', qescp, qe1, qeot	};
char	yccont[]	{	qread, 'c', qescp, qe1, qeot	};
char	yci[]		{	qread, 'i', qescp, qe1, qeot	};
char	ycgo[]		{	qread, 'g', qescp, qe1, qeot	};
char	yce1m[]		{	qread, qescp, qe1, qeot		};
char	yce2m[]		{	qread, qescp, qe2, qeot		};
char	yce3m[]		{	qread, qescp, qe3, qeot		};

char	ycdiag1[]	"\ncyber modem not ready\n";
char	ycdiag2[]	"\ncyberr    ";


char	ycbtoa[]		/* bcd to ascii conversion */
	{
	0055,0112,0113,0114,0115,0116,0117,0120,
	0121,0122,0041,0044,0052,0047,0077,0076,
	0053,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0074,0056,0051,0134,0072,0073,
	0072,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,0060,0075,0042,0100,0045,0133,
	0040,0057,0123,0124,0125,0126,0127,0130,
	0131,0132,0135,0054,0050,0137,0043,0046
	};
char	ycatob[]		/* ascii to bcd conversion */
	{
	0120,0052,0114,0136,0053,0116,0137,0055,
	0134,0074,0054,0060,0133,0040,0073,0121,
	0112,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0100,0077,0072,0113,0057,0056,
	0115,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,0041,0042,0043,0044,0045,0046,
	0047,0050,0051,0122,0123,0124,0125,0126,
	0127,0130,0131,0117,0075,0132,0075,0135,
	0120,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,0041,0042,0043,0044,0045,0046,
	0047,0050,0051,0122,0123,0124,0125,0126,
	0127,0130,0131,0120,0120,0120,0120,0120
	};


/******************/
/* cyber commands */
/******************/

ycopen(dev,flag)
{
	register int x;

	if ( flag==0 ) x = rcopn; else x = wcopn;
	if ( (ycflag&x) | cystart() )
		u.u_error = ENXIO;
	else
		ycflag =| x;
}
ycclose(dev,flag)
{
	ycflag =& ~(flag==0 ? rcopn : wcopn);
}

ycmsgout(msg,len)  /* place msgs for ops for cyb.cdn */
  char	msg[];
  int	len;
{
	char register *x,*y,*z;

  if ( !(ycflag&ruse) ) {
	if ( (z=ycabufe-ycabuf) > (y=len) ) z = ycabufl = y;
		else	ycabufl=z;
	x = ycabuf; y = msg;
	while(z--) *x++ = *y++;
	ycflag =| rrdy; wakeup(&ycabuf);
	return(x);	/* indicate all okay */
  }
  return(0);
}

ycwrite()	/* to send commands to cyber */
{
	register char x, *y;

	if ( ycflag&crdy || u.u_count>yccbufl ) u.u_error = ENXIO;
	else	{
		y = yccbuf;
		*y++ = qread;
		while	((x=cpass())>0) *y++ = x&0177;
		*y++ = qescp;
		*y++ = qe1;
		*y++ = qeot;
		ycflag =| crdy; ycqmsg(yccbuf);
	}
}

ycread()  /* to read response from cyber */
{
	register char	*x;
	register int y;

  spl6();
	if (!(ycflag&rrdy))  sleep(&ycabuf,1);
  spl0();
  if (!(ycflag&rrdy))  return;
  ycflag =& ~rrdy;	ycflag =| ruse;
  x = ycabuf;
  y = (ycabufl>u.u_count) ? u.u_count : ycabufl;
  while(y--)  passc(*x++);
  ycflag =& ~ruse;
}



/***************/
/*  cyber jobs */
/***************/

yjopen(dev,flag)
{
	register int x;

  if ( flag==0 ) x = rjopn;	else x = wjopn;
  if ( (ycflag&x) | cystart() )
	u.u_error = ENXIO;
  else
	ycflag =| (flag==0 ? (rjopn | pre2) : (wjopn | rde2 ));
}

yjclose(dev,flag)
{
  if ( flag )
	ycflag =& ~(wjopn | jrdy | rde2);
  else	ycflag =& ~(rjopn | jfavl);
}

yjwrite()  /* to send jobs to the cyber */
{
	int register n, m;

	spl6();
		if (ycflag&jrdy) sleep(&ycjbuffer,1);
	spl0();

	if (ycflag&jrdy) return;

	ycjbaddr.b_addr=ycjbuffer;
	if ( m=(n=u.u_count)&01776 )
		iomove(&ycjbaddr, 0, m, B_WRITE);
	if (n&1)  ycjbuffer[m++] = cpass();

	if (!m)  return;

	ycjbuffer[m++]=qescp;	ycjbuffer[m++]=qe3; ycjbuffer[m++]=qeot;
	ycflag =| jrdy;
	if ( ycflag&rde2 ) {
		ycqmsg(ycreed);
		ycflag =& ~rde2;
	}
}

yjread()  /* to read jobs output from the cyber. as
		much as possible each time	*/
{
	int register n, m;

  spl6();
	if ( !(ycflag&jfavl) && !yceoj ) sleep(&yclbuf,1);
  spl0();
  if (yceoj)  { yceoj=0; u.u_error=ENXIO; return; } /* terminate last job */

  if ( !(ycflag&jfavl) ) return;
  m = (n = (u.u_count<yclbufl?u.u_count:yclbufl) )&03776;
  if ( m ) iomove(&yclbaddr,0,m,B_READ);
  if ( n&1 ) passc(yclbuf[m]);

  ycflag =& ~jfavl; /* buffer all used */
}



/******************/
/* other          */
/******************/

ycawful()	/* stir up export -- yes it is needed */
{
	static count;

  if ( (ycflag&cyopn) )  {
	if ( ycb < 2 ) {
		ycqmsg(yci);
		if ( count++ >= 2 )  {
			count = 0;
			ycqmsg(yccont);
			if ( ycflag & jrdy )  ycqmsg(ycgo);
		}
	}
	timeout(ycawful,0,10*HZ);
  }
}

ycqmsg(mp)	/* place messages for cyber in 'fifo' queue */
  char	*mp;
{
	int register x, y;

  if ( ycb > 8 ) return;
  y = ps;
  spl6();
  x = ++ycb;
  while( x > 1 )  ycbmsg[x-1] = ycbmsg[(x--)-2];
  ycbmsg[0] = mp;
  ps = y;
}

ycerep(en)	/* called to log errors && optionally tell op */
  register int	en;
{
	char	register *x,*y;

  ycerrfc[en-1]++;
  if (SW->integ&020000)  return;

  x = &ycerrfm[en*3-3];	y = ycdiag2+8;
  *y++ = *x++; *y++ = *x++; *y = *x;
  ycmsgout(ycdiag2,11);
}



/******************/
/* interrupt      */
/******************/

ycwpro()  /* process cyber write commands -- called from
		receive interrupt routine */
{
	char register *x, *y;
	int register *z;

	if ( ycmsg == &ycjbuffer[-1])  {
		ycflag =& ~jrdy;
		wakeup(&ycjbuffer);
	}else
		if ( ycmsg==yccbuf ) ycflag =& ~crdy;

	ycmsg = 0;	/* last 'msg' to cyber was accepted */

	switch(ycetyp)  /* process write according to } code */
	{
	case e1:
		z = &ycrbuf[2];
		if ( *z == '**' )  ycqmsg(yclogin);	/* ' **** idle ....' */
		if ( *z == 'UT' )  {  yceoj++;  wakeup( &yclbuf );  }	/* ' output complete' */
		/* if bit #12 in swreg is set throw away useless crap */
		if (SW->integ&010000)  {
			if ( (*z=='EA') ||	/* ' ready' */
			     (*z=='O ') ||	/* ' no [file is suspended][input active]' */
			     (*z=='UT') ||	/* ' output complete.' */
			     (*z=='ER') ||	/* ' terminal idle */
			     (*z=='..') ||	/* ' ........' */
			     (*z=='AR') ||	/* ' card reader not ready' */
			     (*z=='RI') ||	/* ' printer not ready' */
			     (*z=='OO')	/* ' too many jobs' */
			) break;
		}

		/* can't just ignore last job read messages */
		if ((!ycmsgout(ycrbuf,ycrbufp-ycrbuf))&&(*z=='OB')){	/* ' JOB ID =  XXX.' */
			yctbufp = ycrejm;
			return;
		}
		break;

	case e2:		/* print output */
		/* if cant accept pretend not ready */
		if ( (ycflag&jfavl) || (!(ycflag&rjopn)) ) {
			ycnowmsg = yce2m;
			ycflag =| pre2;
			break;
		}
		x = ycrbufp; y = ycrbuf; z = ycrbufe;

		switch(*y)  {		/* adjust first char	*/
			default:	 *y = '\n';
			case '\n':
			case '\r':
			case ff:
			case vt:	break;
		}

		ycrbufp = ycrbuf = yclbuf;
		ycrbufe = yclbufe;
		yclbufp = yclbuf = y;
		yclbufe = z; yclbufl = x - y;
		ycflag =| jfavl;
		wakeup(&yclbuf);
		/* tell cyber want more output */
		if ( !ycb ) ycnowmsg = yce3m;
		break;

	case e3:				/* card input */
		if ( ycflag&jrdy ) {
			ycnowmsg = &ycjbuffer[-1];
			ycjbuffer[-1] = qread;
		}
			/* no cards to go so go not ready */
		else	{
			ycnowmsg = yce2m;
			ycflag =| rde2;
		}
	}
	yctbufp = ycackm;		/* acknowlege receipt of write */
}


cyrint()  /* receive interrupt routine */

#define	ycrstoh	1
#define	ycrsita	2
#define	ycrstaa	3
#define	ycrcc	4
#define	ycrmsg	5
#define	ycrmsg1	6
#define	ycrmsg2	7
#define	ycrend	8
#define	ycreot	9
#define	ycrlpc	10
#define	ycrerr	11

{
	int register c;
	int static lpc;

rintdone:

  if ( (c = DURCSR) < 0 )  {	/* data set change */
	if ( (c & (CLR_TO_SND|REQ_TO_SND)) == (CLR_TO_SND|REQ_TO_SND) )  {
		DUTCSR = DNA_INTEB|HALF_DUP|SEND;
		DUTBUF = SYN;
	}else if ((c & (CARRIER|RX_INTEB)) == (CARRIER|RX_INTEB))  DURCSR =| (STRIP_SYNC|SCH_SYNC);
	else  if ((c & DATA_SET_RDY) == 0)  printf( ycdiag1 );
	else  if (c & RING)  printf( "\ncyber modem ring!\n" );
	return;
  }

  if ( (c & RX_DONE) == 0 ) return;	/* if (char not available) rti */

  if ( (c = DURBUF) < 0 )  {	/* byte error */
	ycerrf = 3;	/* e_recv */
	if ( c & E_PARITY )  ycerrf = 8;	/* e_parity */
	if ( c & E_OVR_RUN )  ycerrf = 7;	/* e_overun */
	goto	ycrerros;
  }

  c =& 0177;	/* obtain next char sans parity */
  lpc =^ c;	/* calc longtitudinal parity */

  switch(ycrnext) {		/* message sequence */

	case ycrstoh:		/* first char must be start of header */
		if ( c != stoh ) goto ycrerror;
ycrstoh1:
		ycrnext = ycrsita;	lpc = stoh;
		goto	rintdone;

	case ycrsita:		/* second char <=0177 & >=0160 */
		if ( c<0160 ) goto ycrerror;
		ycrnext = ycrstaa;	ycsite = c;
		goto	rintdone;

	case ycrstaa:		/* third char 0140,0141,0160,0161 */
		switch(c)  {
			default:	goto ycrerror;

			case 0140: case 0141:
			case 0160: case 0161:
					ycrnext = ycrcc; ycstat = c;
		}
		goto	rintdone;

	case ycrcc:		/* fourth must be acceptable command */
		switch(c)  {
			default:	goto ycrerror;

			case write: case cwrite:
			case rwrite:
				yccode = write; ycrnext = ycrmsg;
				goto	rintdone;
			case poll:
			case alert:
				yccode = c; ycrnext =ycreot;
				goto rintdone;
		}

	case ycrmsg:		/* process data portion of message */
		ycrnext = ycrmsg2;
		switch(c)  {	/* carriage control */
			case space3:	*ycrbufp++ ='\n';
			case space2:	*ycrbufp++ ='\n';
			default:	if ( ycrbufp==ycrbuf ) goto ycrtr;
					c ='\n';	break;
			case space0:	c ='\r';	break;
			case eject:	c = ff;	break;
			case skipc4:	c = vt;	break;
			case escp:	ycrnext = ycrmsg1; goto rintdone;
		}
		goto	ycrmsg3;

	case ycrmsg1:
		switch(c)  {	/* function code */
			case e1: case e2: case e3:	/* ending code */
				DURCSR =& ~STRIP_SYNC;	/* recognise lpc==SYN after eot */
				ycetyp = c;
				ycrnext = ycreot;
				goto rintdone;
			case eoj:	yceoj++;
			case ceol: case ccr:
				ycrnext = ycrmsg;
				goto rintdone;
			default:		/* '0' or ' ' expansion */
				if ( ((c=- 040)>=3) && (c<=037) ) *ycrbufp++=0377;
				else	{ if (((c=- 040)>=3)&&(c<=017)) *ycrbufp++ = 0376;
						else c=' ';
					}
				ycrnext = ycrmsg2;
				goto ycrmsg3;
		}

	case ycrmsg2:
		switch(c)  {	/* bcd data */
			case escp: ycrnext = ycrmsg1; goto rintdone;
ycrtr:			default:	if ( (c>highbcd) || (c<lowbcd) )  c = ' ';
					else c = ycbtoa[c-lowbcd];
		}
ycrmsg3:
		if ( ycrbufp > ycrbufe ) { if( !ycerrf ) ycerrf = 5; goto ycrerros; }	/* e_length */
		*ycrbufp++ = c;
		goto	rintdone;

	case ycrend:			/* look for end of message */
		if ( c==stoh ) { ycrbufp=ycrbuf; goto ycrstoh1; }
		goto	rintdone;

	case ycreot:			/* this char must be "eot" */
		if ( c!=eot ) goto ycrerror;
		ycrnext = ycrlpc;
		goto	rintdone;

	case ycrerr:
ycrerror:	if ( !ycerrf ) ycerrf = 1;	/* e_format */
ycrerros:
		ycrnext = ycrend;
		goto	rintdone;

	case ycrlpc:			/* this char is longtitudinal parity */
		if ( (lpc != 0177) && !ycerrf ) ycerrf = 2;	/* e_lpc */
		ycrnext = ycrstoh;	/* =| for next receive */

		if ( ycsite != this_site ) {
reset:
			DURCSR = RX_INTEB|DS_INTEB|DATA_TERM_RDY;
			ycerrf = 0;
		}else  {

			if ( ycerrf ) {
				ycerep(ycerrf);
				ycerrf = 0;
				yctbufp = ycerrm;
			} else {
				if ( ycstat & 1 )
					switch(yccode) {	/* odd station address */
					case write:
						ycrerrs = 0;
						/* save write address */
						ycwstat = ycstat;
						ycwpro();
						break;
					case alert:
						yctbufp = ycackm;
						ycqmsg(yce1m);
						break;
					default:
						yctbufp = ycerrm;
						ycerep(6);	/* e_protocol */
					}
				else switch(yccode) {		/* even station address */
					case poll:
						if ( ycmsg == -1){
							ycmsg = 0;
							yctbufp = ycrejm;
						} else
							if ( (yctbufp=ycmsg) )  {
								ycerep(4);	/* e_xmit */
								if ( ++ycrerrs > 10 )  {
									/* printf( "\ncyber fail/reset\n" ); */
									ycrerrs = 0;
									/* goto reset; */
									if ( ycmsg == ycbmsg[ycb] )  ycb++;
									yctbufp = ycmsg = yci;
								}
							}else	if ( ycnowmsg ) {
									yctbufp=ycmsg=ycnowmsg;
									ycnowmsg=0;
								}else
									yctbufp= (ycb ? (ycmsg=ycbmsg[--ycb]) : ycrejm);
						break;
					default:	yctbufp = ycerrm;
							ycerep(6);	/* e_protocol */
				}

				/* reject to 'poll' is different */
				if ( (yccode==poll) && (*yctbufp==0177630) )	/* ==qreject */
					yctstat = ycwstat & 0176;
				else	yctstat = ycwstat;
			}

			/* SEND */

			ycrbufp = ycrbuf;
			DURCSR = DS_INTEB|REQ_TO_SND|DATA_TERM_RDY;
		}
  }
}


cyxint()	/* transmitter status interrupt routine */

/* yctbufp points to the message to be sent to the cyber */
/* qeot signals end of message. */
/* any char with parity bit on is not translated */

#define	ycxsync	1
#define	ycxstoh	2
#define	ycxsita	3
#define	ycxstaa	4
#define	ycxmsg	5
#define	ycxlpc	6
#define	ycxend	7

{
	register c;
	int static lpc;

  switch(yctnext) {	/* transmit message sequence */

	case ycxsync:			/* send 4 syncs */
		c = DUTCSR;		/* clear DNA interrupt */
		if ( ++ycnsync < 4 )  return;
		DUTCSR = TX_INTEB|HALF_DUP|SEND;

	case ycxstoh:			/* 1st char is start of header */
		c = stoh;
		yctnext = ycxsita;	lpc = 0;
		goto	ycxmit;

	case ycxsita:			/* 2nd char is site address */
		c=ycsite;
		yctnext = ycxstaa;
		goto	ycxmit;

	case ycxstaa:			/* 3rd char is station address */
		c = yctstat;
		yctnext = ycxmsg;
		goto	ycxmit;

	case ycxmsg:			/* output data till eot found */
		c = *yctbufp++;
		if ( c&0200 ) {
			c =& 0177;
			if ( c==eot ) yctnext = ycxlpc;
		}else
			if ( c<' ' ) c = 0120;
			else	c = ycatob[c-040];
ycxmit:
		lpc =^ c;		/* long. parity */
		DUTBUF = c;
		return;

	case ycxlpc:			/* output long. parity */
		c = (~ lpc);
		yctnext = ycxend;
		DUTCSR = DNA_INTEB|HALF_DUP|SEND;
		goto	ycxmit;

	case ycxend:			/* all msg sent - tidy up */
		if ( ycflag&cyopn ) {
			DUTCSR = 0;
			DURCSR = RX_INTEB|DS_INTEB|DATA_TERM_RDY;
		}else {
			DUTCSR = MSTRST;	/* no open devices so close down */
		}
		ycrnext = ycrstoh;	yctnext = ycxsync;	ycnsync = 1;
  }
}

cystart()  /* initiate receive on DU11 if necessary */
{
  if ( (DURCSR & DATA_SET_RDY)==0 ) {
	printf(ycdiag1);	/* modem not ready */
	return(1);
  }

  if ( !(ycflag&cyopn) ) {

	DUPSR = INT_SYNC|C8BIT|ODD_PAR|SYN;
	DURCSR = RX_INTEB|DS_INTEB|DATA_TERM_RDY;

	/* reinit necessary variables */

	ycerrf = ycflag = ycb = 0;
	ycmsg = -1;
	ycrbuf = ycrbufp = ycrbuffer; ycrbufe = ycrbuffer+1049;
	yclbuf = yclbufp = yclbuffer; yclbufe = yclbuffer+1049;
	ycrnext = ycrstoh;	yctnext = ycxsync;	ycnsync = 1;

	timeout(ycawful,0,60*HZ);
  }

  return(0);
}


#ifdef	POWER_FAIL
cypowerf()
{
	extern cyrestart();

  if ( ycflag & cyopen )
	timeout( cyrestart , 0 , PUDELAY );
}

cyrestart( count )
{
	register f;

  if ( DURCSR & DATA_SET_RDY || ++count > PATIENCE )  {
	f = ycflag;
	ycflag = 0;
	cystart();
	ycflag = f;
  }else
	timeout( cyrestart , count , PURETRY );
}
#endif
