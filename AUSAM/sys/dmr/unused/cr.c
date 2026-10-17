#
/*
 *	copyright 1975  ian inc.
 */
/*
 *	cr11 card reader driver
 *	ascii and binary version
 *	for buffered card reader
 */
#include "../defines.h"
#include "../param.h"
#include "../conf.h"
#include "../user.h"
#include "../buf.h"
/*
 *	to add another card reader see notes
 * 	below with struct cr11
 */
#define cripri	20	/* sleeping priority */
#define ncols	80	/* no. of columns    */
#define splcr	spl4	/* hardware priority level */
/*
 *	card reader status bits
 *	see the dec peripherals handbook
 *	for more information
 */
#define error   	(1<<15)
#define cardone		(1<<14)
#define hopper		(1<<13)
#define motion		(1<<12)
#define cbuffer		(1<<11)
#define online		(1<<10)
#define busy		(1<<9)
#define notready	(1<<8)
#define coldone		(1<<7)
#define ienable		(1<<6)
#define clear		(1<<1)
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
struct
{
	char crstat0,crstat1;	/* chars of the status register */
	char crbbin0,crbbin1;	/* the two bytes of the binary buffer */
};
/*
 *	card reader handler status
 */
#define closed		0
#define reading		1
#define cardread	2
#define endfile		3
#define valerr		4
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
				3 - endfile */
			/*    020 - B_PHYS for iomove */
	char crmode;	/* 0 - 029
			   1 - 026
			   2 - binary */
	long	crdcnt;		/* double word card count */
	int crvalerr;	/* count of validity errors */
	int crrdchk;	/* count of read checks */
	int *craddr;	/* device register address */
	int crchars;	/* for passing back the number of
				characters left to pass back */
/*********************************************************************
 *		this next word must be the 8th in the structure      *
 *              for the purposes of iomove                           *
 *********************************************************************/
	char *nextch;	/* buffer position pointer */
	char buffer[2*ncols+1];	/* one card buffer */
};
struct cr11 cr0
{
	closed,		/* initially cr11 closed */
	0,		/* initial mode (o29) */
	0,		/* initial card count */
	0,		/* initial validity error count */
	0,		/* initial read check count */
	0177160,	/* device register craddr  */
	0,		/* initial characters to be passed back */
	0,		/* initial ptr */
};
/* second card reader address:- 160030 */
struct cr11 *cr11[ncr11]
{
	&cr0,
};
/*	to add another card reader -
 *   1. increment ncr11 by 1.
 *   2. add another element to the array "cr11"
 *      and initialise the element to the address of
 *      a cr11 structure (similar to "cr0").
 *   3. add the cr11 struct complete with
 *      its initialisation. if you cant work out
 *      how to initialise it then you should not
 *      be doing it......
 */
/*
 *	special character codes
 */
#define eoi   	017	/*   6 7 8 9 punching */
#define	csueoi	07417	/*  12 11 0 1  6 7 8 9 punching */
#define eof	015	/*   6 7   9 punching */
#define eor	007	/*     7 8 9 punching */
#define cnv	025	/* 5   7   9 punching */

#define cntrlf	'\06'	/* eof returns cntrlf */
#define cntrlr	'\022'	/* eor returns cntrlr */
/*
 *	validity checking table
 */
int crvalid[8]
 {
	0,1<<8,1<<7,1<<6,1<<5,1<<4,1<<3,1<<2
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

craopen(dev,flag)
{
	cropen(dev,flag,o29);
}
crbopen(dev,flag)
{
	cropen(dev,flag,binary);
}

cropen(dev,flag,mode)

{
	register struct cr11 *cr;

	if((flag != 0) || (dev.d_minor >= ncr11))
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
	cr->crchars = 0;
	crstart(cr);
}

crclose(dev,flag)

{
	register struct cr11 *cr;

	cr = cr11[dev.d_minor];
	cr->craddr->crstatus = 0;
	cr->crstate = closed;
}

crint(dev)

{
	register struct cr11 *cr;
	register int *devreg;
	register int status;

	cr = cr11[dev.d_minor];
	devreg = cr->craddr;
	status = devreg->crstatus;
	if(status &  (error|cardone|online))
	{
		devreg->crstatus = ienable; /* clear interrupt*/
		if(status & motion)
		{
			cr->crrdchk++;
		}
		else
		if((status&(cardone|cbuffer)) == (cardone|cbuffer))
		{
			devreg->crstatus = 0;
			cr->crstate = cardread;
			wakeup(cr);
		}
		else
		if(status & online)
			crstart(cr);
	}
}

crread(dev)

{
	register struct cr11 *cr;
	register int *devreg;
	register char *pch;
	char qch;

	cr = cr11[dev.d_minor];
	if(cr->crchars == 0){
		if(cr->crstate == endfile)
			crstart(cr);
	loop:	
		splcr();
		while(cr->crstate!=cardread)
			sleep(cr,cripri);
		spl0();
		/* a card has been read */
		cr->crdcnt++;
		pch = cr->buffer;
		devreg = cr->craddr;
		do
		{
			if(cr->crmode == binary)
			{
				*pch++ = devreg->crbbin0;
				*pch++ = devreg->crbbin1;
			}
			else
			{	/* ascii conversion - 1st validity check */
				if(crvalid[devreg->crbcode&07]!=(devreg->crbbin&0774))
				{
					switch(devreg->crbbin)
					{
					case eoi:
					case csueoi:
						cr->crstate = endfile;
						cr->crmode = o29;
		return; /*<-----------------------------------------*/
					case eof:
						*pch++ = cntrlf;
					 break;
					case cnv:
						if(pch == cr->buffer)
						{
						 crrdcol(devreg);
						 cr->crmode=devreg->crbbin==0;
						 crstart(cr);
		 goto loop; /*<------------------------------*/
						}
						/* if not conversion default*/
					default:
						crerror(dev,cr);
		 goto loop; /*<-------------------------------*/
					} /* switch */
				}
				else
				{	/* valid - translate */
					qch = crtab[devreg->crbcode];
					/* check o26 or o29 specials */
					if (qch <= 0)
						if (qch == 0)
						{
							crerror(dev,cr);
		 goto loop; /*<--------------------------------*/
						}
						else
						{
						 *pch++=(cr->crmode?
							crcd26:crcd29)[-qch];
						}
					else
						*pch++ = qch;
				} /* fi */
			} /* fi */
		} while (pch < &cr->buffer[ncols*2] && crrdcol(devreg));
		if (cr->crmode != binary)
			*pch++ = '\n';
		cr->nextch = cr->buffer;
		cr->crchars = pch - cr->buffer;
		if (cr->crstate != endfile)
			crstart(cr);
	}
	crpass(cr);
}

crstart(xcr)
struct cr11 *xcr;

{
	register struct cr11 *cr;
	register int *devreg;

	cr = xcr;
	cr->crstate = reading;
	/* flush the buffer in card reader */
	devreg = cr->craddr;
	devreg->crstatus = clear;
	devreg->crstatus = ienable|read;
	/* card in motion */
}

crerror(dev,xcr)
struct cr11 *xcr;

{
	register struct cr11 *cr;
	register int *devreg;
	register dummy;

	cr =  xcr;
	devreg = cr->craddr;
	cr->crstate = valerr;
	cr->crvalerr++;
#ifndef	UPRINTS
	printf("cr%d validity - reread last card\n",
		dev.d_minor);
#else
	uprints("Invalid punch - reread last card\n");
#endif	UPRINTS
	dummy = devreg->crbbin;		/* clear coldone */
	devreg->crstatus = ienable|clear;
}



crpass(xcr)
struct cr11 *xcr;
{
	register struct cr11 *cr;
	register int count1,count2;

	cr = xcr;
	/* send back the min of cols on card and use count */
	count1 = (cr->crchars < u.u_count ? cr->crchars : u.u_count);
	/* iomove an even number of characters */
	if(count2 = count1 & 0177776)
	{
#ifdef	MAPPED_BUFFERS
		cr->crstate =| 020;		/* B_PHYS */
#endif	MAPPED_BUFFERS
		iomove(cr,0,count2,B_READ); /* uses cr->nextch */
#ifdef	MAPPED_BUFFERS
		cr->crstate =& ~020;		/* off again */
#endif	MAPPED_BUFFERS
		cr->crchars =- count2;
		cr->nextch =+ count2;
	}
	if(count1 & 1)
	{
		passc(*cr->nextch++);
		cr->crchars--;
	}
}


crrdcol(xdevreg)
{
	register int *devreg;
	int x,y;

	devreg = xdevreg;
	devreg->crstatus = read; /* shift */
	/* wait a microsecond or 5 */
	x = y; 	/* waste the required time */
	if(devreg->crstatus&cbuffer)
		return(1);
	else
		return(0);
}
crsgtty(dev, av)	int *av;
{
	register struct cr11 *cr;
	register *v;
	cr = cr11[dev.d_minor];
	if( v=av ){		/* gtty */
		*v++ = cr->crstate &0377;
		*v++ = cr->crmode &0377;
		*v++ = cr->craddr->crstatus;
		/**/
		return(1);
	}
	/* stty */
	v = u.u_arg;
	v++;		/* lets skip this bit ... */
	cr->crmode = *v++;
	/**/
	return(0);
}

#ifdef	POWER_FAIL

crpowf()
{
	register i;
	register struct cr11 *cr;

	for( i = 0 ; i < ncr11 ; i++ ) {
		cr = cr11[i];
		if( cr->crstate == reading || cr->crstate == cardread)
		{
			crerror(i, cr);
		}
	}
}
#endif	POWER_FAIL
