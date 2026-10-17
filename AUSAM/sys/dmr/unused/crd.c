#
/*
 *	copyright 1975  ian inc.
 */
/*
 *	cr11 card reader driver
 *	ascii and binary version
 */
#include "../defines.h"
#include "../param.h"
#include "../buf.h"
#include "../conf.h"
#include "../user.h"
/*
 *	to add another card reader see notes
 * 	below with struct cr11
 */
#define cripri	20	/* sleeping priority */
#define ncols	80	/* no. of columns    */
#define splcr	spl6	/* hardware priority level */
/*
 *	card reader status bits
 *	see the dec peripherals handbook
 *	for more information
 */
#define error   	(1<<15)
#define cardone		(1<<14)
#define hopper		(1<<13)
#define motion		(1<<12)
#define timing		(1<<11)
#define online		(1<<10)
#define busy		(1<<9)
#define notready	(1<<8)
#define coldone		(1<<7)
#define ienable		(1<<6)
#define eject		(1<<1)
#define read		(1<<0)
/*
 *	card reader device registers
 *	the order of these fields is important
 */
struct
{
	int crstatus;	/* status register */
	int crbbin;	/*   12 bit binary buffer */
	int crbcode;	/* 8 bit encoded buffer */
};
/*
 *	card reader handler status
 */
#define closed		0
#define reading		1
#define cardread	2
#define endfile		3
#define err		4
/*
 *	card reader mode
 */
#define o29 0
#define o26 1
#define binary 2
/*
 *	status info for card reader(s)
 */
#define ncr11	1	/* no. of cr11 */
struct cr11
{
	char crstate;	/* current state of reader */
			/*	0 - closed
				1 - reading
				2 - cardread
				3 - endfile
				4 - err
			      020 - B_PHYS don't use */
	char crmode;	/* 0 - 029
			   1 - 026
			   2 - binary */
	long	crdcnt;		/* dble word card count */
	int crerror;	/* copy of status register on error */
	int *addr;	/* device register address */
	int crcols;	/* for reading the number of columns
				left in the buffer */
	int crchars;	/* for passing back the number of
				characters left to pass back */
/*********************************************************************
 *		this next word must be the 8th in the structure      *
 *              for the purposes of iomove                           *
 *********************************************************************/
	char *nextch;	/* buffer position pointer */
	int  *nextcol;	/* pointer to the last column read */
	int buffer[ncols+1];	/* one card buffer */
};
struct cr11 cr0
{
	closed,		/* initially cr11 closed */
	0,		/* initial mode (o29) */
	0,		/* initial card count */
	0,		/* initial error status */
	0177160,	/* device register addr  */
	0,		/* initial columns */
	0,		/* initial characters to go back */
	0,		/* initial character ptr */
	0,		/* initial column pointer */
};
/*
/*	struct cr11 cr1
/*	{
/*		0,		/* initial card count */
/*		closed,		/* initially cr11 closed */
/*		0,		/* initial mode (o29) */
/*		0,		/* initial error status */
/*		0160030,	/* device register addr  */
/*		0,		/* initial columns */
/*		0,		/* initial characters to go back */
/*		0,		/* initial character ptr */
/*		0,		/* initial column pointer */
/*	};
 */
struct cr11 *cr11[ncr11]
{
	&cr0,
/*	&cr1	*/
};
/*	to add another card reader -
 *   1. increment ncr11 by 1.
 *   2. add initialisation for the next card reader
 *	into the initialisation for cr11 structure.
 *	if you can't follow the structure then
 *	perhaps you shouldn't be modifying it.
 */
/*
 *	special character codes
 */
#define eoi   	01437	/*   6 7 8 9 punching */
#define eof	01427	/*   6 7   9 punching */
#define eor	00437	/*     7 8 9 punching */
#define cnv	02427	/* 5   7   9 punching */

#define cntrlf	'\06'	/* eof returns cntrlf */
#define cntrlr	'\022'	/* eor returns cntrlr */
/*
 *	validity checking table
 */
char crvalid[8]
 {
	0,1<<6,1<<5,1<<4,1<<3,1<<2,1<<1,1<<0
 };
char crcd26[]
{ 0,'+','=','(',')','\'','^','#','>','?',';','<','[',']',
	'`','"','&',':','!','?','^','@','%' };
char crcd29[]
{ 0,'&','#','%','<','@',')',';','^','(','!','{','"','\\',
	'_','>','?',']','}','[','+','\'','=' };
/*
 *	coded to ascii translation
 */
char crtab[256]
 {
	' ','1','2','3','4','5','6','7',	/*		*/
	'8','`',':',-2 ,-5 ,-21,-22,-12,	/*	8	*/
	'9', 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	/*	9	*/
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , cntrlr ,	/*      9 8	*/
	'0','/','s','t','u','v','w','x',	/*    0		*/
	'y','^',-13,',',-3 ,-14,-15,-16,	/*    0   8	*/
	'z', 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	/*    0 9	*/
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	/*    0 9 8	*/
	'-','j','k','l','m','n','o','p',	/* 11		*/
	'q','|',-17,'$','*',-6 ,-7 ,-8 ,	/* 11     8	*/
	'r', 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	/* 11   9	*/
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	/* 11   9 8	*/
	-18,'~','S','T','U','V','W','X',	/* 11 0		*/
	'Y', 0 , 0 , 0 , 0 , 0 ,'\t',0 ,	/* 11 0   8	*/
	'Z', 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	/* 11 0 9	*/
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,	/* 11 0 9 8	*/
	-1 ,'a','b','c','d','e','f','g',   /*   12		*/
	'h', 0 ,-19,'.',-4 ,-9 ,-20,-10,   /*   12        8	*/
	'i', 0 , 0 , 0 , 0 , 0 , 0 , 0 ,   /*   12      9	*/
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,   /*   12      9 8	*/
	-11,'A','B','C','D','E','F','G',   /*   12    0		*/
	'H', 0 , 0 , 0 , 0 , 0 , 0 , 0 ,   /*   12    0   8	*/
	'I', 0 , 0 , 0 , 0 , 0 , 0 , 0 ,   /*   12    0 9	*/
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,   /*   12    0 9 8	*/
	 0 ,'J','K','L','M','N','O','P',   /*   12 11		*/
	'Q', 0 , 0 , 0 , 0 , 0 , 0 , 0 ,   /*   12 11     8	*/
	'R', 0 , 0 , 0 , 0 , 0 , 0 , 0 ,   /*   12 11   9	*/
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,   /*   12 11   9 8	*/
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,   /*   12 11 0		*/
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,   /*   12 11 0   8	*/
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,   /*   12 11 0 9	*/
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,   /*   12 11 0 9 8	*/
 };
/*
 *	and now for something completely different
 *	the routines to handle all
 */

crdaopen(dev,flag)
{
	crdopen(dev,flag,o29);
}
crdbopen(dev,flag)
{
	crdopen(dev,flag,binary);
}

crdopen(dev,flag,mode)

{
	register struct cr11 *cr;

	if((flag != 0) || (dev.d_minor > ncr11))
	{
		u.u_error = ENXIO;
  return;
	}
	cr = cr11[dev.d_minor];
	if(cr->crstate != closed)
	{
		u.u_error = EEXIST;
  return;
	}
	cr->crmode = mode;
	crdstart(cr);
}

crdclose(dev,flag)

{
	register struct cr11 *cr;

	cr = cr11[dev.d_minor];
	cr->addr->crstatus = 0;
	cr->crstate = closed;
}

crdint(dev)

{
	register struct cr11 *cr;
	register int *devreg;
	register int status;

	cr = cr11[dev.d_minor];
	devreg = cr->addr;
	status = devreg->crstatus;
	if(status & coldone)
		if(cr->crcols)
		{
			cr->crcols--;
			*++(cr->nextcol) = (cr->crmode == binary ?
					devreg->crbbin :
					(devreg->crbcode) |
					(devreg->crbbin<<6 & 077400));
		}
		else
		{
			int dummy;
			dummy = devreg->crbbin;	/* clear interrupt */
		}
	if(status &  (error|cardone|online))
	{
		if(status & error)
		{
			devreg->crstatus = eject|ienable;
			cr->crerror = status;
		}
		if(status&cardone)
		{
			devreg->crstatus = 0;
			cr->crstate = cardread;
			wakeup(cr);
		}
		else
		if(status & online)
			crdstart(cr);
	}
}

crdread(dev)

{
	register struct cr11 *cr;

	cr = cr11[dev.d_minor];
	if(cr->crchars == 0)
	{
		if(cr->crstate == endfile)
			crdstart(cr);
	loop:	{
			splcr();
			while(cr->crstate!=cardread)
				sleep(cr,cripri);
			spl0();
			cr->crdcnt++;
			if(cr->crerror & timing)
			{
				crderror(dev,cr,"timing");
		goto loop;	/* next time put in a bigger card */
			}
			if(cr->crerror&motion && cr->crcols==ncols)
			{
				crdstart(cr);
		goto loop;
			}
		}
		cr->crcols = ncols - cr->crcols;
		if (cr->crmode == binary)
		{
			cr->crchars = cr->crcols*2;
		}
		else
		{
			if(crascii(dev,cr))
		goto loop;	/* must have been an invalid card or conv */
		}
		cr->nextch = cr->buffer;
	}
	crdpass(cr);
	if ((cr->crstate != endfile) && (cr->crchars == 0))
		crdstart(cr);
}

crdstart(xcr)
struct cr11 *xcr;

{
	register struct cr11 *cr;
	register int *devreg;
	register dummy;

	cr = xcr;
	cr->nextcol = cr->buffer - 1;
	cr->crstate = reading;
	cr->crerror = 0;
	cr->crcols = ncols;
	cr->crchars = 0;
	devreg = cr->addr;
	dummy = devreg->crbbin;	/* must clear column done in esoteric errors */
	devreg->crstatus = ienable|read;
}

crderror(dev,cr,errmsg)
register struct cr11 *cr;
char *errmsg;
{
	register dummy;

#ifdef UPRINTS
	uprints("reread last card\n");
#endif
	printf("cr%d %s - reread last card\n", dev.d_minor, errmsg);
	dummy = cr->addr->crbbin;	/* clear any column done interrupt */
	cr->crstate = err;
	cr->addr->crstatus = ienable;
}

int crascii(dev,xcr)
struct cr11 *xcr;
{
	register struct cr11 *cr;
	register int *pch;
	register char *qch;

	cr = xcr;
	pch = qch = cr->buffer;
	while(pch <= cr->nextcol)
	{
		/* at most one punching in columns 1-7 */
		if (crvalid[*pch&07] != (*pch>>8))
		{
			switch(*pch)
			{
			case eoi:	cr->crstate = endfile;
					cr->crmode = o29;
	return(0);/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
			case eof:	*pch = cntrlf;
			break;/*<<<<<<<<<<<<<<<<<<<<<*/
			case cnv:	if(pch == cr->buffer)
					{
					 cr->crmode = cr->buffer[1]==0;
					 crdstart(cr);
	return(1);/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
					}
			default:
					crderror(dev,cr,"validity");
	return(1);/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
			} /* switch */
		}
		else
		{
			/* translate character */
			*qch = crtab[*pch&0377];
			/* check 026 029 and validity */
			if (*qch <= 0)
				if (*qch == 0)
				{
					crderror(dev,cr,"validity");
	return(1);/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
				}
				else
				{
					*qch = (cr->crmode?crcd26:
							crcd29)[-*qch];
				}


		}
		pch++;
		qch++;
	}
	*qch++ = '\n';
	cr->crchars = cr->crcols + 1;
	return(0);
}


crdpass(xcr)
struct cr11 *xcr;
{
	register struct cr11 *cr;
	register int count1,count2;

	cr = xcr;
	/* send back the min of cols on card and use count */
	count1 = (cr->crchars < u.u_count ? cr->crchars : u.u_count);
	/* iomove an even number of characters */
	if(count1 > 1)
	{
		count2 = count1 & 0177776;
#ifdef	MAPPED_BUFFERS
		cr->crstate =| B_PHYS;
#endif	MAPPED_BUFFERS
		iomove(cr,0,count2,B_READ);
#ifdef	MAPPED_BUFFERS
		cr->crstate =& ~B_PHYS;
#endif	MAPPED_BUFFERS
		cr->crchars =- count2;
		cr->nextch =+ count2;
	}
	/* passc the last character if odd */
	if(count1 & 1)
	{
		passc(*(cr->nextch++));
		cr->crchars--;
	}
}
crdsgtty(dev, av)  int *av;
{
	register struct cr11 *cr;
	register int *v;

	cr = cr11[dev.d_minor];
	if( v=av ){		/* gtty */
		*v++ = cr->crstate &0377;
		*v++ = cr->crmode &0377;
		*v++ = cr->addr->crstatus;
		return(1);
	}

	/* stty */
	v = u.u_arg;
	v++;		/* skip this bit ... */
	cr->crmode = *v++;
	/* only allowed to set mode ... */
	return(0);
}
#ifdef	POWER_FAIL

crdpowf()
{
	register i;
	register struct cr11 *cr;

	for( i = 0 ; i < ncr11 ; i++ ) {
		cr = cr11[i];
		if( cr->crstate == reading || cr->crstate == err)
		{
			crderror(i, cr, "power fail");
		}
	}
}
#endif	POWER_FAIL
