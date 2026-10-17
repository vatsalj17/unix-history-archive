#
/*
 *	Optimized RK-11/RK03/RK05/disk driver
 *
 *	Copyright (c) 1975, the Children's Museum.
 *	This program is proprietary. Permission is hereby
 *	granted to use this program and associated documentation
 *	by any UNIX licensee in accordance with the provisions of
 *	any UNIX licensing agreement. Other uses of 
 *	of this program, or distribution of this program
 *	in violation of the provisions of the UNIX license,
 *	must be approved by the Children's Museum in writing.
 *
 *	Inquiries, bug notices, etc, should be addressed to
 *		Computer Center
 *		The Children's Museum
 *		jamaicaway
 *		Boston, MA 02130
 *		(617) 522-4800 x25
 *	Authors: Bill Mayhew and Brent Byer, September 1975
 *
 *	See "rkvolmap" for the mapping of rk "volumes" onto drives
 */

#include "../defines.h"
#include "../param.h"
#include "../lnode.h"
#include "../systm.h"
#include "../buf.h"
#include "../conf.h"
#include "../user.h"



#define	NO_SEEK		/* disable seeks if only one drive */
/*#define	RKSTATS		/* collect statistics */
#define	SWEEP		/* enable directional optimisation */
#define	ERR_FLOOR 1	/* errors allowed before report */
#define	TEMP_CUT_OUT	/* sensitive drive temperature cut-out ! */



#define NRK	2	/* number of RK05 volumes on system */
#define NDRV	1	/* number of drives on system */
#define INTLV	1	/* magic interleaving number */
			/* This is dependant on the time to service an rk interrupt and start i/o on the next sector.
			 * Time/sector is 3.3*10(-3) secs. for 1500 rpm., and 2*10(-3) secs. for 2500 rpm.
			 * INTLV > (max. interrupt latency)/(time/sector).
			 */

#define RKADDR	0177400	/* base address of RK11 control registers */

#define NRKSEC	12	/* 12 sectors per track */
#define NTRACK	2	/* 2 tracks per cylinder */
#define	NRKCYLS	203	/* 203 cylinders per volume */
#define NRKBLK	4872	/* 4872 blocks per volume */
#define NHRKBLK	2435	/* offset in map algorithm */
#define NRKSKIP 10	/* number of times io can be preempted */

/* control register bits */
#define CRESET	0	/* control reset */
#define GO	01	
#define SEEK	010
#define DRESET  014	/* drive reset */
#define IENABLE	0100	/* interrupt enable */
#define CTLRDY	0200	/* control ready */
#define SEEKCMP	020000	/* seek complete */
#define	HARDERR	040000	/* hard error */

/* drive status bits */
#define	RWS	0100	/* on cylinder */
#define	DRY	0200	/* drive ready */
#define	SIN	01000	/* seek incomplete */

/* error bits */
#define	NXD	0200	/* non-existent drive */
#define	DATALT	01000	/* data late */
#define	SEEKERR	010000	/* seek error */
#define	DRE	0100000	/* drive error */

#ifdef	POWER_FAIL | TEMP_CUT_OUT
/* retry delays */
#define	PUDELAY	(25*HZ)	/* drive power up time */
#define	PURETRY	(1*HZ)	/* power up test retry delay */
#endif

#define B_SEEK	02000	/* seeking flag for buffer header */

struct devtab rktab;

struct {
	int rkds;
	int rker;
	int rkcs;
	int rkwc;
	int rkba;
	int rkda;
};

#ifndef	RAW_BUFFER_POOL
struct	buf	rrkbuf;
#endif

/*
 *	structure of an RK disk queue; one queue per drive.
 */

struct	rkq {
	struct	buf	*rk_bufp;	/* pointer to first buffer in queue */
#ifdef	SWEEP
	int	rk_lcyl;		/* last cylinder accessed on this drive */
	char	rk_dirf;		/* direction of head motion  */
#endif
#ifdef	RKSTATS
	char	rk_nreq;		/* number of requests in queue */
	char	rk_rmax;		/* high water mark for q */
	unsigned rk_cyls[NRKCYLS];	/* cylinder usage count */
#endif
	int	rk_errcnt;		/* error count */
} rk_q[ NDRV ];

struct	rkq	*rk_ap;			/* pointer to queue of drive
					   currently doing I/O */
struct	rkq	*rkvolmap[NRK]
{
	&rk_q[0]	/* PERTEC drive 1 fixed */
	,&rk_q[0]	/* PERTEC drive 1 removable */
};

#ifdef	ERR_FLOOR
int	rkreport;			/* floor for error reports */
#endif
#ifndef	ERR_FLOOR
#define	rkreport	0
#endif


/*
 *	Use b_scratch in the buffer header to save cylinder address;
 *	b_sector to save sector within cylinder; av_back to save
 *	disk address.
 */

#define rkcyl	b_scratch	/* must be int */
#define	rksec	b_sector	/* can be char */
#define rkaddr	av_back


/*
 *	Rkstrategy() does all the block mapping for
 *	the two varieties of RK formats supported by this driver.
 *	The differing formats are distinguished by the minor device
 *	number used in referencing the disk:
 *
 *	minor device 0-7: "traditional" straightforward RK format.
 *	minor device 010-017: new optimized split-up disk. In this
 *		format, block 0 is in its standard place so that
 *		boot programs can be put there; blocks 1 through
 *		NHRKBLK (2435) are located beginning at block #2436,
 *		all remaining blocks are between block 1 & 2435. the
 *		effect of this mapping is to centralize disk head motion
 *		about the center of the disk. 
 *		the optimization is ideal for those RK's
 *		which serve as both root device and swap device. It 
 *		is less than ideal, although probably still an 
 *		improvement over traditional form, for RK's used 
 *		exclusively as mounted file systems.
 */
rkstrategy(bp)
  register struct buf *bp;
{
	register unsigned p1, p2;
	int f;


  p2 = bp->b_blkno;
  p1 = bp->b_dev.d_minor;

/*
 *	check for validity of this request
 */

  if ( ((p1&07) >= NRK) || (p2 >= NRKBLK) )
  {
bad:
	bp->b_flags =| B_ERROR;
	iodone(bp);
	return;
  }


/*
 *	Here, we do the mapping for the various formats:
 */

  if ( (p1 & 010) && p2 && ((p2 =+ NHRKBLK) >= NRKBLK) )
	p2 =- NRKBLK - 1;

/*
 *	if raw I/O, check within physical limits
 */
  if ( bp->b_flags & B_PHYS )
  {
	if ( p2+((-bp->b_wcount)/256) > NRKBLK )
		goto bad;

#	ifdef	_1170
	mapalloc( bp );
#	endif
  }

  p1 =& 07;
  bp->b_error = NRKSKIP;
		/*
		 * b_error counts the number of times
		 * this io has been skipped in the queue,
		 * when it becomes 0 it cannot be ignored
		 * anymore. NRKSKIP should be set to the
		 * average size of contiguous command files
		 * eg. in bin...
		 */

/*
 *	calculate cylinder, sector, drive and surface for
 *	this request and save same.
 */

  bp->rksec = p2 % NRKSEC;
  p2 =/ NRKSEC;
  bp->rkaddr = (p1 << 13) | (p2 << 4) | bp->rksec;
  bp->rkcyl = p2 >> 1;
  p2 = rkvolmap[p1];
#ifdef	RKSTATS
  p2->rk_cyls[bp->rkcyl]++;
#endif

  spl5();

#ifdef	RKSTATS
	p2->rk_nreq++;
#endif
	if ((p1 = p2->rk_bufp)==NULL) {

		/* queue was empty */
		p2->rk_bufp = bp;
		bp->av_forw = 0;
		if ((rk_ap == NULL) && (RKADDR->rkcs & CTLRDY))
			rkstart();

	} else {

		/* non-empty; determine where to place this request
		 *  in the queue, taking into account current 
		 *  direction of head motion and minimization of 
		 *  head movement.
		 */
#ifdef	SWEEP
		f = p2->rk_dirf;
#endif

		p2 = p1->av_forw;
		while ( p2 )  {		/* skip any blocks overtaken too often */
			if ( !p2->b_error )
				p1 = p2;
			p2 = p2->av_forw;
		}

		for (; p2 = p1->av_forw; p1 = p2)

			if (   p1->rkcyl <= bp->rkcyl
			    && bp->rkcyl <= p2->rkcyl
			    || p1->rkcyl >= bp->rkcyl
			    && bp->rkcyl >= p2->rkcyl
			   )
			{
				while (bp->rkcyl==p2->rkcyl) {
					/*
					 * if a cylinder match is found,
					 * do rotational optimization.
					 */
					if (p2->rksec > p1->rksec) {
						if(bp->rksec > p1->rksec
						+INTLV &&bp->rksec < p2
						->rksec-INTLV)
							break;
					} else
						if(bp->rksec>p1->rksec
						+INTLV
						||bp->rksec<p2->rksec
						-INTLV)
							break;
					p1 = p2;
					if(!(p2 = p1->av_forw)) break;
				}
				break;
			}
#ifdef	SWEEP
			else
				if ( f ) {
					if(p2->rkcyl < p1->rkcyl)
						if(bp->rkcyl > p1->rkcyl)
							break;
						else
							f = 0;
				} else {
					if(p2->rkcyl > p1->rkcyl)
						if(bp->rkcyl < p1->rkcyl)
							break;
						else
							f++;
				}
#endif

		bp->av_forw = p2;
		p1->av_forw = bp;

		while( p2 )	/* following blocks overtaken */
		{
			if( p2->b_error)
				p2->b_error--;
			p2 = p2->av_forw;
		}
	}

  spl0();
}


/*
 *	rkstart() goes through all the queues and sets everybody
 *	seeking that should be.
 */
rkstart()
{
	register struct buf *bp;
	register struct rkq *qp;

	for (qp = rk_q; qp < &rk_q[ NDRV ]; qp++)
#ifndef	NO_SEEK
		if ((bp = qp->rk_bufp) && (bp->b_flags&B_SEEK)==0)
#else	NO_SEEK
		if ( bp = qp->rk_bufp )
#endif	NO_SEEK
		{
#ifdef	SWEEP
			if(bp->rkcyl > qp->rk_lcyl) qp->rk_dirf = 1;
			else if(bp->rkcyl < qp->rk_lcyl) qp->rk_dirf = 0;

#endif	SWEEP
#ifndef	NO_SEEK
			bp->b_flags =| B_SEEK;
			while((RKADDR->rkcs&CTLRDY)==0);
			RKADDR->rkda = bp->rkaddr;
			RKADDR->rkcs = IENABLE|SEEK|GO;
#else	NO_SEEK
#ifdef	SWEEP
			qp->rk_lcyl = bp->rkcyl;
#endif	SWEEP
			rk_ap = qp;
			bp->b_error = 0;
			devstart(bp, &RKADDR->rkda, bp->rkaddr, 0);
			return;
#endif	NO_SEEK
		}
}


/*
**	Command completion/error interrupt handler
**	Rewritten -- Piers Lauder	Jul '78
*/
rkintr()
{
	register  union { int i; int *ip; } n;
	register struct rkq *qp;
	register struct buf *bp;
	int dreset = 0;
	int rkerr;

	if(RKADDR->rkcs < 0)	/* error bit */
	{
		if ( rk_ap && (rk_ap->rk_errcnt >= rkreport) )  {
			n.ip = RKADDR;
			printf("\nRKDA=%o,BA=%o,WC=%o,CS=%o,ER=%o,DS=%o\n",
				*n.ip,*n.ip++,*n.ip++,*n.ip++,*n.ip++,*n.ip++);
		}
		rkerr = RKADDR->rker;
#ifndef	NO_SEEK
		for(qp=rk_q; qp < &rk_q[ NDRV ]; qp++)
			if(qp->rk_bufp)
				qp->rk_bufp->b_flags =& ~B_SEEK;
#endif	NO_SEEK

		n.i = (RKADDR->rkda >> 13) & 07;

		if ( RKADDR->rkcs & HARDERR )
		{
			RKADDR->rkcs = IENABLE|CRESET|GO;

			if ( rkerr & SEEKERR )
			{
				while ( (RKADDR->rkcs & CTLRDY)==0 );
#ifndef	NO_SEEK
rtz:
#endif
				RKADDR->rkda = n.i << 13;
				RKADDR->rkcs = IENABLE|DRESET|GO;
				dreset = 1;
				rk_ap = NULL;
			}
#ifdef	TEMP_CUT_OUT
			else
			if ( rkerr & NXD )
			{
				extern char *panicstr;

				rkpowerf();
				panicstr++;
				printf( "\nRK drive %d off line!\n", n.i );
				panicstr = 0;
				return;
			}
#endif	TEMP_CUT_OUT
		}
#ifndef	NO_SEEK
		else
			if(RKADDR->rkds & SIN)
			{
				/* Seek Incomplete */
				n.i = (RKADDR->rkds >> 13) & 07;
				goto rtz;
			}
#endif	NO_SEEK

		qp = rkvolmap[n.i];

		if ( !(bp = qp->rk_bufp) )
			return;		/* dummy interrupt ? */

		if(++(qp->rk_errcnt) == 10)
		{
			bp->b_flags =| B_ERROR;
			goto out;
		}

		if ( dreset )
		{
			if ( n.ip = bp->av_forw )
			{
				/** try next block first **/

				bp->av_forw = n.ip->av_forw;
				n.ip->av_forw = bp;
				qp->rk_bufp = n.ip;
				qp->rk_errcnt = 0;
			}
			return;	/* wait for ready interrupt */
		}

#ifndef	NO_SEEK
		goto xfer;
#else	NO_SEEK
		rkstart();
#endif
	}
#ifdef	NO_SEEK
	else
	if ( RKADDR->rkcs & SEEKCMP )
		rkstart();
#else	NO_SEEK

	if ( RKADDR->rkcs & SEEKCMP )
	{
		n.i = (RKADDR->rkds >> 13) & 07;
		if ( bp = (qp = rkvolmap[n.i])->rk_bufp )
		{
			bp->b_flags =& ~B_SEEK;
xfer:
#ifdef	SWEEP
			qp->rk_lcyl = bp->rkcyl;
#endif
			rk_ap = qp;
			bp->b_error = 0;
			devstart(bp, &RKADDR->rkda, bp->rkaddr, 0);
		}
	}
#endif
	else
	{
		if(qp = rk_ap)
		{
			/* we get here if this is an I/O completion interrupt */
			bp = qp->rk_bufp;
out:
			qp->rk_bufp = bp->av_forw;
#ifdef	RKSTATS
			if ( qp->rk_nreq-- > qp->rk_rmax )
				qp->rk_rmax = qp->rk_nreq+1;
#endif
			rk_ap = NULL;
			qp->rk_errcnt = 0;
			if ( !dreset )
				rkstart();
			iodone(bp);
		}
	}
}


/*
 *	Raw interfaces. Note that the raw interface will NOT work with
 *	the shuffled format if the i/o overflows the physical disk.
 *	However, one may still use the raw device
 *	interface for such applications as backups even when the disk in
 *	question is a shuffled one, since all one wants to do in such a
 *	case is copy the disk directly without caring at all what's there
 *	and in what order. Raw device files should only be made with
 *	minor devices zero through NRK.
 */
#ifndef	RAW_BUFFER_POOL
rkread(dev)
{
	physio(rkstrategy, &rrkbuf, dev, B_READ);
}

rkwrite(dev)
{
	physio(rkstrategy, &rrkbuf, dev, B_WRITE);
}
#endif

#ifdef	RAW_BUFFER_POOL
rkread(dev)
{
	physio(rkstrategy, 0, dev, B_READ);
}

rkwrite(dev)
{
	physio(rkstrategy, 0, dev, B_WRITE);
}
#endif


#ifdef	POWER_FAIL | TEMP_CUT_OUT
/*
 * restart after a power restore
 */
rkpowerf()
{
	extern rkrestart();
	register struct rkq *qp;
	register struct buf *bp;

#ifndef	NO_SEEK
  for ( qp=rk_q ; qp < &rk_q[NDRV] ; qp++ )
	if ( bp = qp->rk_bufp )
		bp->b_flags =& ~B_SEEK;
#endif

  rk_ap = NULL;
  timeout( rkrestart , 0 , PUDELAY );
}

rkrestart()
{
	register i, j;

  for ( i=0 ; i<NRK ; i++ )
	if ( rkvolmap[i]->rk_bufp )  {
		RKADDR->rkda = i<<13;
		for ( j=10 ; --j ; );
		if ( (RKADDR->rkds & (RWS|DRY)) != (RWS|DRY) )  {
			timeout( rkrestart , 0 , PURETRY );
			return;
		}
	}

  rkstart();
}
#endif
