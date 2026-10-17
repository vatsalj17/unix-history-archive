#
/*
 *	copyright 1976 ian johnstone
 *
 *	-->	this line printer driver will handle printers
 *		in the best most efficient way.
 *	-->	no upper to lower case conversion.
 *	-->	recognize zero,blank suppression.
 *	-->	support infinite size pages.
 *	-->	support 013 as skip to channel four.
 */

#include "../defines.h"
#include "../param.h"
#include "../conf.h"
#include "../user.h"
#include "../proc.h"
#include "../buf.h"

#define	ienable	0100
#define	done	0200
#define	error	0100000

#define nlp	1	/* number of line printers */
#define	lppri	10
#define	maxcol	132
#define	open	01
#define	closin	02
#define  bmax	02	/* maximum number of buffers for a line printer */

int lpages 0;	/* pages printed since last boot */

struct lpdev {
	int lpsr;
	int lpbuf;
};
struct  { int lpuf;  };	/* dummy for lpint only */

struct lpst  {
	struct devtab dtab;	/* buffer pointer */
	struct lpdev *lpaddr;	/* device register address */
	int	flag;	/* control flags */
	int   ejline;   /* lines per page */
	int	mcc;
	int	ccc;	/* character count */
	int	mlc;	/* linecount */
#ifndef unsw_orig
	int	vtc;	/* vertical tab counter */
#endif
	int bfree;	/* number of buffers that are left to go */
	int	bc;	/* byte count (-ve) */
	char	*ba;	/* pointer of where up to */
	char	fc;	/* if non-zero it is char to be duplicated */
};
#ifndef unsw_orig
  struct lpst lp0 { 0,0,0,0,0,0,0177514,0,0,0,0,0,0,bmax,0,0,0 };
#endif
#ifdef unsw_orig
  struct lpst lp0 { 0,0,0,0,0,0177514,0,0,0,0,0,bmax,0,0,0 };
#endif

struct lpst *lp11[nlp] { &lp0, };

int lpflush 0;	/* set externally if buffer pool to be flushed !! */

lpopen(dev, flag)
{	register struct lpst *plp;

	plp = lp11[dev.d_minor];
	if( (dev.d_minor >= nlp) ||
	    (plp->flag & open)   ||
	    ( !flag )	         ||
	    ((plp->lpaddr)->lpsr < 0)	)  u.u_error = EIO;
	else {
	plp->flag = open;	plp->ejline = 60;
	(plp->lpaddr)->lpsr =| ienable;
	plp->ccc = plp->mlc = plp->mcc = 0;
	lpflush = 0;	/* reinit */
	}
}

lpclose(dev, flag)
{	register struct lpst *plp;

	plp = lp11[dev.d_minor];
	plp->flag = open|closin;
	while(plp->bfree != bmax) sleep(plp,lppri);
	(plp->lpaddr)->lpsr  =& ~ienable;
	if(plp->mcc | plp->mlc)  (plp->lpaddr)->lpbuf = 014;
	plp->flag=0;
}

lpwrite(dev)
{	register struct buf *bp;
	register struct lpst *plp;	int register n;

	plp= lp11[dev.d_minor];
	spl4();
 while(u.u_count && !u.u_error) {
	while(!plp->bfree)	sleep(plp,lppri);
	if( lpflush ) {
		u.u_error = EIO;
		break;
	}
	plp->bfree--;
	bp = getblk(NODEV); bp->av_forw = 0;
	spl4();
	if( (n=u.u_count) > 512 ) n=512;
	bp->b_wcount = -n;  iomove(bp,0,n,B_WRITE);
	if(!plp->dtab.d_actf) {
		plp->dtab.d_actf = plp->dtab.d_actl = bp;
		plp->ba = bp->b_addr;
		plp->bc = bp->b_wcount;
		lpint(dev);
	} else {
		(plp->dtab.d_actl)->av_forw = bp;
		plp->dtab.d_actl = bp;
	}
     }
	spl0();
}

lpint(dev)
{	register struct lpst *plp;
	struct buf *bp; register int *lpa;
	register  c1;

	if( !(bp=(plp= lp11[dev.d_minor])->dtab.d_actf) ) goto fin;
	if(lpflush) {
		do {
			lpa = bp->av_forw;
			brelse(bp);
			plp->bfree++;
		} while( bp = lpa );
		plp->dtab.d_actf = 0;
		wakeup(plp);
		goto fin;
	}
	if((plp->lpaddr)->lpsr&error) { timeout(lpint,dev,120); return; }
	lpa = &(plp->lpaddr)->lpbuf;
#ifndef unsw_orig
	if(plp->vtc) {
		lpa->lpuf = '\n';
		plp->vtc--;
		return;
	}
#endif
loop: while( ((plp->lpaddr)->lpsr&done) && (plp->bc++ <0) ) {
	c1 = (*plp->ba++)&0377;
	if( plp->fc ) {
		plp->ccc =+(--c1);
		if(plp->fc=='0') {
			if(plp->ccc>maxcol) c1=- plp->ccc-maxcol;
			while(c1-- > 0) lpa->lpuf = '0';
		}
		plp->fc=0;
		goto loop;
	}
	switch( c1 ) {
	case '{':
			c1 = '(';	goto esc;
	case '}':
			c1 = ')';	goto esc;
	case '`':
			c1 = '\'';	goto esc;
	case '|':
			c1 = '!';	goto esc;
	case '~':
			c1 = '^';
	esc:
			while(plp->ccc > plp->mcc){
				lpa->lpuf = ' ';
				plp->mcc++;
			}
			lpa->lpuf = c1;
			*(--plp->ba) = '-'; --plp->bc;
			lpa->lpuf = '\r';
			plp->mcc=0;
	case 0:		break;
	case '\t':	plp->ccc = (plp->ccc+8) & ~7; break;
	case 0377:	plp->fc = c1 = ' '; goto dflt;
	case 0376:	plp->fc = c1 = '0'; goto dflt;
	case 0375:	plp->ejline = 32767;
			break;
	case 013:
#ifndef unsw_orig
			plp->vtc = 61-plp->mlc;
			plp->mlc = -3;
			plp->ccc=0; plp->mcc=1;
			lpa->lpuf = '\n';
			return;
#endif
#ifdef unsw_orig
			break;
			plp->mlc=plp->ccc=0; plp->mcc=1;
			break;
#endif
	case 014:
	case '\n':
			if(plp->mlc||plp->mcc) {
				plp->mcc = 0;
				plp->mlc++;
				if(plp->mlc >= plp->ejline ) c1 = 014;
				lpa->lpuf=c1;
				if(c1 == 014) { 
				    	plp->mlc=0; lpages++; 
				}
			}
	case '\r':
			plp->ccc = 0;
			break;
	case 010:
			if(plp->ccc > 0)
				plp->ccc--;
			break;
	case ' ':
			plp->ccc++;
			break;
	default:
	dflt:
			if(plp->ccc < plp->mcc) {
				lpa->lpuf= '\r';
				plp->mcc = plp->fc = 0;
				--plp->ba; --plp->bc;
				break;
			}
			if(plp->ccc < maxcol) {
				while(plp->ccc > plp->mcc) {
					lpa->lpuf= ' ';
					plp->mcc++;
				}
				lpa->lpuf= c1;
				plp->mcc++;
			}
			plp->ccc++;
	}
      }
	if(plp->bc>=0) {
		plp->dtab.d_actf = (bp = plp->dtab.d_actf)->av_forw;
		brelse(bp);
		if(!(plp->bfree++)) wakeup(plp);
		if(bp=plp->dtab.d_actf) {
			plp->ba = bp->b_addr;
			plp->bc = bp->b_wcount;
			goto loop;
		}
	fin:
	    if( ((plp->lpaddr)->lpsr&done)&&(plp->flag&closin) )wakeup(plp);
	}
}
#ifdef	POWER_FAIL

lppowf()
{
	register struct lpst *plp;
	register i;

	for( i = 0 ; i < nlp ; i++ ) {
		plp = lp11[i];
		if( plp->flag&open ) (plp->lpaddr)->lpsr =| ienable;
	}
}
#endif	POWER_FAIL
