#
/*
 *	Copyright 1976 Ian Johnstone.
 *
 *
 *	This line printer driver will handle printers
 *		in the best most efficient way. Maybe.
 *	No upper to lower case conversion.
 *	Recognize zero, blank suppression.
 *	Support adjustable size pages.
 *	Support 013 as skip to perforations.
 *	Talk to DEC and other ( CDC ) printers.
 *	Support special CDC skips.
 *	Kronos eor/eof taken as form feeds. 022, 006
 *	Stty calls supported to change modes.
 *
 */


#include "../defines.h"
#include "../param.h"
#include "../conf.h"
#include "../user.h"
#include "../proc.h"
#include "../buf.h"



/*
 * status register bits
 */

#define	IENABLE	0100
#define	READY	0200

#define	NLP	1	/* no. of line printers */
#define	LPPRI	3

/*
 * some status & mode bits
 */

#define	WOPEN	0000001		/* device open for writing */
#define	CLOSIN	0000002		/* device closing */

#define	TRNS	0000004		/* if half ASCII (May be undefined) */
#define	CDC	0000010		/* if cdc special skips  */
#define	VTAB	0000020		/* support vertical tabs */
#define	FLUSH	0000040		/* when set flush all */
#define	FEED	0000100		/* if always take ff's, lf's */
#define	NOEJECT	0000200		/* if disable auto skip */
#define	TMPNOEJ	0000400		/* if temporary NOEJECT */
#define	SIXLPI	0001000		/* 11" paper at 6 lines/inch */

#define	ERROR	0100000		/* Error bit set in status register */

#ifdef	TRNS
#define	MODES	(TRNS|CDC|VTAB|FLUSH|FEED|NOEJECT|SIXLPI)
#else
#define	MODES	(CDC|VTAB|FLUSH|FEED|NOEJECT|SIXLPI)
#endif	TRNS

/*
 * define characteristics of devices
 */

#define	LPF0	(TRNS|VTAB)		/* dec printer */
#define	MAXCOL	132			/* dec printer */
#define	PL	60			/* page length */

#define	BMAX	04	/* maximum number of buffers used per device */

int		lpages;	/* pages printed since last boot (in toto) */


struct lpdev
{
	int lpsr;
	int lpbuf;
};
struct { int lpuf;};	/* dummy for lpint only */

struct lpst
{
	int		flag;		/* control flags */
	int		bfree;		/* number of buffers unused */
	struct	buf    *bfwd;		/* first buffer in chain */
	struct	buf    *blst;		/* last  buffer in chain */
	struct	lpdev  *lpaddr;		/* device register address */
	int		ejline;		/* lines per page (settable) */
	unsigned	maxcol;		/* max cols per line (settable) */
	unsigned	ocol;		/* output column */
	unsigned	icol;		/* input column */
	int		lino;		/* output line */
	int		lini;		/* input line */
	unsigned	vtcnt;		/* count for vertical tabs */
	int		bc;		/* byte count (-ve) */
	char	       *ba;		/* next char in buffer */
	char		fc;		/* duplicated char if non-zero */
}
		lp0			/* define lp0 */
{
	LPF0,				/* flags */
	BMAX,				/* bfree */
	0, 0,				/* bfwd, blst */
	0177514,			/* lpaddr */
	PL,				/* ejline */
	MAXCOL,				/* maxcol */
	0, 0,				/* ocol, icol */
	0, 0,				/* lino, lini */
	0,				/* vtcnt */
	0, 0,				/* bc, ba */
	0,				/* fc */
},
#ifdef	TWO_PRINTERS
		lp1			/* define lp1 */
{
	LPF1,				/* flags */
	BMAX,				/* bfree */
	0, 0,				/* bfwd, blst */
	0160024,			/* lpaddr */
	PL,				/* ejline */
	MAXCOL,				/* maxcol */
	0, 0,				/* ocol, icol */
	0, 0,				/* lino, lini */
	0,				/* vtcnt */
	0, 0,				/* bc, ba */
	0,				/* fc */
},
#endif	TWO_PRINTERS
		*lp11[NLP]		/* define lp11 */
{
	&lp0,				/* printer 0 */
#ifdef	TWO_PRINTERS
	&lp1,				/* printer 1 */
#endif	TWO_PRINTERS
};

lpopen(dev, flag)
{
	register struct lpst *plp;

	if( flag )
	{
		plp = lp11[dev.d_minor];
		if( (dev.d_minor>=NLP) || (plp->flag&WOPEN) ||  (plp->lpaddr->lpsr<0) )
		{
			u.u_error = EOPENFAIL;
			return;
		}
		else
		{
			register mode = FLUSH|ERROR;

			if(plp->flag & TMPNOEJ)
				mode =| (NOEJECT | TMPNOEJ);
			plp->flag =& ~mode;
			plp->flag =| WOPEN;
			plp->lpaddr->lpsr =| IENABLE;
			plp->icol = plp->ocol = plp->lino = plp->lini = 0;
		}
	}
}

lpclose(dev, flag)
{
	register struct lpst *plp = lp11[dev.d_minor];

	if( plp->flag&WOPEN )
	{
		/* if final close after write */
		spl4();
		plp->flag =| CLOSIN;
		if( plp->bfwd == 0 )
			lpint(dev);
		spl0();
	}
}

lpwrite(dev)
{
	register struct buf  *bp;
	register struct lpst *plp;
	register int n;

	plp = lp11[dev.d_minor];
	spl4();
	while(u.u_count && !u.u_error)
	{
		while(!plp->bfree)
		{
			sleep(plp,LPPRI);
		}
		if( plp->flag & FLUSH )
		{
			u.u_error = EIO;
			break;
		}
		plp->bfree--;
		bp = getblk(NODEV);
		bp->av_forw = 0;
		spl4();
		if( (n = u.u_count) > 512 )
			n = 512;
		bp->b_wcount = -n;
		iomove(bp, 0, n, B_WRITE);
		if(!plp->bfwd)
		{
			plp->bfwd = plp->blst = bp;
#ifdef	MAPPED_BUFFERS
			plp->ba = b.buff;
#else	MAPPED_BUFFERS
			plp->ba = bp->b_addr;
#endif	MAPPED_BUFFERS
			plp->bc = bp->b_wcount;
			lpint(dev);
		}
		else
		{
			plp->blst->av_forw = bp;
			plp->blst = bp;
		}
	}
	spl0();
}

lpint(dev)
{
	register struct lpst *plp;
	register int *lpa;
#ifdef	MAPPED_BUFFERS
	int ka5sav = ka5;
#endif	MAPPED_BUFFERS

	if( (plp = lp11[dev.d_minor])->lpaddr->lpsr < 0 )	/* error */
	{
		plp->flag =| ERROR;
		timeout(lpint, dev, 1*HZ);
		return;
	}
	plp->flag =& ~ERROR;
	if( plp->bfwd == 0 )
		goto fin;
	if( plp->flag & FLUSH )
	{
		register struct buf *bp;

		bp = plp->bfwd;
		do
		{
			lpa = bp->av_forw;
			brelse(bp);
			plp->bfree++;
		} while( bp = lpa );
		plp->bfwd = 0;
		wakeup(plp);
		goto fin;
	}
	lpa = &(plp->lpaddr->lpbuf);
	if( plp->vtcnt )
	{
		lpa->lpuf = '\n';
		plp->vtcnt--;
		return;
	}
#ifdef	MAPPED_BUFFERS
	bswtch(plp->bfwd);
#endif	MAPPED_BUFFERS
loop:
	while( (plp->lpaddr->lpsr & READY) && (plp->bc++ < 0) )
	{
		register int c1;

		c1 = (*plp->ba++) & 0377;
		if( plp->fc )
		{
			plp->icol =+ (--c1);
			if(plp->fc == '0')
			{
				if(plp->icol > plp->maxcol)
					c1 =- plp->icol - plp->maxcol;
				while(c1-- > 0)
					lpa->lpuf = '0';
			}
			plp->fc = 0;
			continue;
		}
		switch( c1 )
		{
	    case 0:
			break;
	    case '\t':
			plp->icol = (plp->icol+8) & ~7;
			break;
	    case 0377:
			plp->fc = c1 = ' ';
			goto dflt;
	    case 0376:
			plp->fc = c1 = '0';
			goto dflt;
	    case 0375:
			if( !(plp->flag & NOEJECT))
				plp->flag =| (NOEJECT | TMPNOEJ);
			break;
	    case 013:
			if( plp->flag & VTAB )
			{
				if( plp->flag & CDC )
				{
					lpa->lpuf = 0100103;
				}
				else
				{
					plp->vtcnt = plp->ejline - plp->lino + 3;
					if(plp->flag & SIXLPI)
						plp->vtcnt--;
					lpa->lpuf = '\n';
				}
				plp->icol = plp->ocol = 0;
				if(plp->flag & SIXLPI)
					c1 = -3;
				else
					c1 = -4;
				plp->lini = plp->lino = c1;
			}
			break;
	    case 006:	/* kronos eof */
	    case 022:	/* kronos eor */
			c1 = 014;
	    case 014:
	    case '\n':
			if( plp->flag & FEED || plp->lini || plp->ocol )
			{
				plp->ocol = 0;
				plp->lini++;
				if( plp->lini >= plp->ejline &&
					!(plp->flag & NOEJECT) )
				{
					c1 = 014;
				}
				if(c1 == 014)
				{ 
					lpa->lpuf = c1;
				    	plp->lini = plp->lino = 0;
					lpages++; 
				}
			}
	    case '\r':
			plp->icol = 0;
			break;
	    case 010:
			if(plp->icol > 0)
				plp->icol--;
			break;
	    case ' ':
			plp->icol++;
			break;
	    default:
		dflt:
			if(plp->lini > plp->lino)
			{
				if(plp->flag & CDC)
				{
					c1 = plp->lini - plp->lino;
					if(c1 > 3)
						c1 = 3;
					lpa->lpuf = 0100120 + c1;
					plp->lino =+ c1;
				}
				else
				{
					plp->vtcnt = plp->lini - plp->lino - 1;
					plp->lino = plp->lini;
					lpa->lpuf = '\n';
				}
				plp->fc = 0;
				--plp->ba;
				--plp->bc;
				break;
			}
			if(plp->icol < plp->ocol)
			{
				lpa->lpuf = '\r';
				plp->ocol = plp->fc = 0;
				--plp->ba;
				--plp->bc;
				break;
			}
			if(plp->icol < plp->maxcol)
			{
				while(plp->icol > plp->ocol)
				{
					lpa->lpuf = ' ';
					plp->ocol++;
				}
#ifdef	TRNS
				if( plp->flag & TRNS )
				{
					switch( c1 )
					{
				    case '{':	c1 = '(';
						goto esc;
				    case '}':	c1 = ')';
						goto esc;
				    case '`':	c1 = '\'';
						goto esc;
				    case '|':	c1 = '!';
						goto esc;
				    case '~':	c1 = '^';
			
					esc:
						lpa->lpuf = c1;
						*(--plp->ba) = '-';
						--plp->bc;
						plp->ocol = 0;
						lpa->lpuf = '\r';
						continue;
					}
				}
#endif	TRNS
				lpa->lpuf = c1;
				plp->ocol++;
			}
			if( ++plp->icol == 0) plp->icol--;
		}
	}
	if(plp->bc >= 0)
	{
		register struct buf *bp;

		bp = plp->bfwd;
		plp->bfwd = bp->av_forw;
		brelse(bp);
		if( !(plp->bfree++))
			wakeup(plp);
		if(bp = plp->bfwd)
		{
#ifndef	MAPPED_BUFFERS
			plp->ba = bp->b_addr;
#else	MAPPED_BUFFERS
			plp->ba = b.buff;
			bswtch(bp);
#endif	MAPPED_BUFFERS
			plp->bc = bp->b_wcount;
			goto loop;
		}
	fin:
		if( (plp->lpaddr->lpsr & READY) && (plp->flag & CLOSIN) )
		{
			plp->flag =& ~(WOPEN | CLOSIN);
			plp->lpaddr->lpsr = 0;
			if( plp->lino  || plp->ocol )
				plp->lpaddr->lpbuf = 014;
		}
	}
#ifdef	MAPPED_BUFFERS
	ka5 = ka5sav;
#endif	MAPPED_BUFFERS
}

lpsgtty( dev, v )
register int *v;
{
	register struct lpst *plp;

	plp = lp11[dev.d_minor];

	switch( v )
	{
		/*
		 *	set
		 */
	case 0:
		v = u.u_arg;
		spl4();			/* look out for interrupt */
		plp->flag = (*v++ & MODES) | (plp->flag & ~MODES); /* protect status */
		spl0();
		plp->ejline = *v++;
		plp->maxcol = *v++;
		return(0);

		/*
		 *	get
		 */
	default:
		*v++ = plp->flag;
		*v++ = plp->ejline;
		*v++ = plp->maxcol;
		return(1);
	}
}

#ifdef	POWER_FAIL
lppowf()
{
	register struct lpst *plp;
	register i;

	for( i = 0; i < NLP; i++)
	{
		plp = lp11[i];
		if( plp->flag & WOPEN)
			plp->lpaddr->lpsr =| IENABLE;
	}
}
#endif	POWER_FAIL
