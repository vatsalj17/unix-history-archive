#
/*
**	Optimised Xylogics Phoenix 211 Disk Controller Driver
**
**	Bugs & comments to:	Piers Lauder
**				Dept. of Computer Science
**				Sydney University
*/


#include	"../defines.h"
#include	"../param.h"
#include	"../conf.h"
#include	"../buf.h"
#include	"../user.h"


#define	NDRV		1		/* Drives on controller */

/* (not	MANY_DRIVES)
#define	MANY_DRIVES			/* if NDRV > 1 (otherwise save some code)

/* (not	DIFFERENT_DRIVES)
#define	DIFFERENT_DRIVES		/* if physically different drives attched to controller */

/* (not	VOLUME_STATS)
#define	VOLUME_STATS			/* record disc usage by volume */

/* (not	DRIVE_STATS)
#define	DRIVE_STATS			/* record disc accesses by cylinder and queue size */

/*
** assignment of minor device bits
**
**	bits 030 are drives,
**	bits 007 are volumes.
*/
#define	NVOL		7		/* max allowable volumes per drive */

#define	INTLV		1		/* interleave factor for rotational optimisation */
#define	XYAGE		10		/* number of times i/o may be preempted */

#define	XYOPENPRI	10		/* sleep priority whilst waiting for drive ready */


#define	XYADDR		0164000		/* controller address */
#define	SPLXY		spl5		/* interrupt priority */

#include	"xy.h"			/* definitions of controller registers */

#include	"ok.h"			/* Okidata 3300 Disc Drive parameters */


/*
**	Structure to describe drive states and conditions
*/

struct	drive
{
	char		dr_state;	/* drive status */
	char		dr_qstate;	/* queue status */
	struct buf	*dr_qp;		/* head of i/o queue */
	int		dr_cyl;		/* current cylinder */
	char		dr_dir;		/* current direction of head sweep */
	char		dr_com;		/* last command to this drive */
	char		dr_errcnt;	/* count of errors on current i/o */
	char		dr_rtzcnt;	/* count of drive resets on current i/o */
	char		dr_id;		/* this drive number */
#	ifdef	DIFFERENT_DRIVES
	int		dr_trksecs;	/* sectors per track */
	int		dr_cylsecs;	/* sectors per cylinder */
#	endif	DIFFERENT_DRIVES
	unsigned	dr_errors[16];	/* drive errors per XYERR */
#	ifdef	DRIVE_STATS
	unsigned	dr_cylref[NOKCYL/8];	/* count of accesses to cylinder areas */
	unsigned	dr_qref[NBUF];	/* count of queue sizes */
	int		dr_qsize;	/* current queue size */
	unsigned	dr_rotopt;	/* count of rotational optimisations */
#	endif	DRIVE_STATS
}
	xydrv[NDRV]
#ifdef	DIFFERENT_DRIVES
{
  {	0,0,0,0,0,0,0,0,0,NOKSEC,NOKSEC*NOKHDS }
}
#endif	DIFFERENT_DRIVES
;


/** drive states **/

#define	DR_RDY		0		/* up and running */
#define	DR_UNR		1		/* unready */
#define	DR_ERR		2		/* hard error */


/** queue (i/o) states **/

#define	Q_EMPTY		0		/* guess */
#define	Q_WAIT		1		/* waiting for i/o */
#ifdef	MANY_DRIVES
#define	Q_SEEK		2		/* seeking */
#define	Q_RDY		3		/* on cylinder */
#else	MANY_DRIVES
#define	Q_RDY		Q_WAIT		/* do i/o immediately */
#endif	MANY_DRIVES
#define	Q_IO		4		/* i/o in progress */


/** sweep direction **/

#define	IN		0		/* increasing cylinder number */
#define	OUT		1		/* decreasing cylinder number */


/*
**	pointer to current drive doing i/o
*/

struct	drive	*xydp;


/*
**	Structure to map logical volumes onto drives
*/

struct	volume
{
	unsigned	v_blocks;	/* max. blocks this volume */
	int		v_cyloff;	/* 1st. cylinder address */
#	ifdef	VOLUME_STATS
	long		v_ref;		/* count of accesses to this volume */
#	endif	VOLUME_STATS
}
	xymap[NDRV][NVOL]
{
  {	/** Drive 1 -- Okidata 3300 **/
	 {	32256,   0	}	/* 0: 84 cylinders	*/
	,{	13824,  84	}	/* 1: 36		*/
	,{	 3840, 120	}	/* 2: 10		*/
	,{	14592, 130	}	/* 3: 38		*/
	,{	33408, 168	}	/* 4: 87		*/
	,{	32256, 255	}	/* 5: 84		*/
	/*     130176, 339		   1-1 mapping		*/
	,{      65536, 168	}	/* 6: 171 (max. volume)	*/
  }
};


/*
**	Error Recovery Strategy for read errors
*/

int	xyretrycm[]
{
	0			/* 3 in-place retrys */
	,02000			/* data late,  servo normal */
	,04000			/* data early, servo normal */
	,(SEEKINH|0400)		/* data normal, servo minus */	/* single sector only */
	,(SEEKINH|01000)	/* data normal, servo plus  */	/* single sector only */
	,(SEEKINH|02400)	/* data late,  servo minus  */	/* single sector only */
	,(SEEKINH|03000)	/* data late,  servo plus   */	/* single sector only */
	,(SEEKINH|04400)	/* data early, servo minus  */	/* single sector only */
	,(SEEKINH|05000)	/* data early, servo plus   */	/* single sector only */
};

#define	RECALTRYS	3	/* number of recalibrates to be attempted after RECAL error */


/*
**	Use b_scratch in the buffer header to hold cylinder address,
**	b_resid to hold i/o skip count,
**	av_forw to hold pointer to next buffer in i/o queue,
**	and av_back to hold unit_sector_head address.
*/

#define	b_next		av_forw		/* must be "struct buf *" */
#define	b_ush		av_back		/* 16 bits */
#define	b_age		b_resid		/* must be int */
#define	b_cyl		b_scratch	/* 16 bits */


/*
**	Raw buffer for physical i/o
*/

#ifdef	RAW_BUFFER_POOL
#define	XYRAWBUF	0
#else	RAW_BUFFER_POOL
struct	buf		xyrawbuf;
#define	XYRAWBUF	&xyrawbuf
#endif	RAW_BUFFER_POOL


/*
**	Device Table
*/

struct	devtab		xytab;




/*
**	Strategy routine
**
**	-- sort buffer into optimised i/o queue
*/

xystrategy( bp )
  register struct buf *bp;
{
	int			unit;
  {
	register unsigned	b;
	register struct volume	*vp;

	b = bp->b_dev.d_minor;
	unit = (b>>3)&7;
	b =& 7;
#	ifdef	MANY_DRIVES
	vp = &xymap[unit][b];
#	else	MANY_DRIVES
	vp = &xymap[0][b];
#	endif	MANY_DRIVES

	/*
	**	Check for validity of this request
	*/

	if ( unit >= NDRV || b >= NVOL || (b = bp->b_blkno) >= vp->v_blocks )
	{
bad:
		bp->b_flags =| B_ERROR;
		iodone( bp );
		return;
	}

	/*
	**	if raw i/o, check within limits
	*/

	if ( bp->b_flags & B_PHYS )
	{
		if ( (b+(-bp->b_wcount)/256) > vp->v_blocks )
			goto bad;
#		ifdef	UNIBUS_MAP
		mapalloc( bp );
#		endif	UNIBUS_MAP
	}

#	ifdef	VOLUME_STATS
	vp->v_ref++;
#	endif	VOLUME_STATS

	/*
	**	Calculate sector, cylinder, and disk address
	*/

#	ifdef	DIFFERENT_DRIVES
#	define	TRKSECS	trksecs
#	define	CYLSECS	cylsecs
#	else	DIFFERENT_DRIVES
#	define	TRKSECS	NOKSEC
#	define	CYLSECS	(NOKSEC*NOKHDS)
#	endif	DIFFERENT_DRIVES
	{
#		ifdef	DIFFERENT_DRIVES
		register	trksecs = xydrv[unit].dr_trksecs;
		register	cylsecs = xydrv[unit].dr_cylsecs;

#		endif	DIFFERENT_DRIVES
		bp->b_cyl = b / CYLSECS + vp->v_cyloff;
#		ifdef	MANY_DRIVES
		bp->b_ush = (unit << 12) | ((b % CYLSECS)/TRKSECS << 7) | (bp->b_sector = b % TRKSECS);
#		else	MANY_DRIVES
		bp->b_ush = ((b % CYLSECS)/TRKSECS << 7) | (bp->b_sector = b % TRKSECS);
#		endif	MANY_DRIVES
		bp->b_age = XYAGE;
	}
  }

  SPLXY();

  {
	register struct buf	*p1, *p2;
	int			d;

#	ifdef	MANY_DRIVES
	p2 = &xydrv[unit];
#	else	MANY_DRIVES
	p2 = xydrv;
#	endif	MANY_DRIVES

#	ifdef	DRIVE_STATS
	p2->dr_qref[p2->dr_qsize++]++;
	p2->dr_cylref[bp->b_cyl/8]++;
#	endif	DRIVE_STATS

	if ( (p1 = p2->dr_qp) == NULL )
	{
		/** Start up empty queue **/

		p2->dr_qp = bp;
		p2->dr_qstate = Q_WAIT;

		xystart();
	}
	else
	{
		/*
		**	Non-empty queue - determine where to place this request
		**	in the queue, taking into account current direction of
		**	head movement, and minimisation of head movement.
		*/

		d = p2->dr_dir;

		if ( p2 = p1->b_next )
			do
				if ( p2->b_age == 0 )
					p1 = p2;	/* skip this block overtaken too often */
			while
				( p2 = p2->b_next );

		for ( ; p2 = p1->b_next ; p1 = p2 )
		{
			if (	p1->b_cyl <= bp->b_cyl
			     && bp->b_cyl <= p2->b_cyl
			  ||	p1->b_cyl >= bp->b_cyl
			     && bp->b_cyl >= p2->b_cyl
			   )
			{
				for ( p1 = p2 ; (p2 = p1->b_next) && ( bp->b_cyl == p2->b_cyl ) ; p1 = p2 )
				{
					/** Cylinder match -- do rotational optimisation **/

#					ifdef	DRIVE_STATS
					xydrv[unit].dr_rotopt++;
#					endif	DRIVE_STATS

					if ( p2->b_sector > p1->b_sector )
					{
						if ( bp->b_sector > p1->b_sector + INTLV
						  && bp->b_sector < p2->b_sector - INTLV
						   )
							break;
					}
					else
						if ( bp->b_sector > p1->b_sector + INTLV
						  || bp->b_sector < p2->b_sector - INTLV
						   )
							break;
				}
				break;
			}
			else
				if ( d == IN )
				{
					if ( p2->b_cyl < p1->b_cyl )
						if ( bp->b_cyl > p1->b_cyl )
							break;
						else
							d = OUT;
				}
				else
					if ( p2->b_cyl > p1->b_cyl )
						if ( bp->b_cyl < p1->b_cyl )
							break;
						else
							d = IN;
		}

		p1->b_next = bp;

		if ( bp->b_next = p2 )
			do
				/** following blocks overtaken **/

				p2->b_age--;
			while
				( p2 = p2->b_next );
	}
  }

  spl0();
}




/*
#ifdef	MANY_DRIVES
**	Start seeks on all drives waiting that are not on cylinder
#endif	MANY_DRIVES
**	Start i/o on first drive ready on cylinder
*/

xystart()
{
	register struct drive	*dp = xydrv;
# ifdef	MANY_DRIVES
	register		x, i;
	static struct drive	*lastdrive xydrv;

  /*
  **	Start seeks on waiting queues
  */

  for ( ; dp < &xydrv[NDRV] ; dp++ )
	if ( (dp->dr_state == DR_RDY) && (dp->dr_qstate == Q_WAIT) )
		if ( dp->dr_cyl != dp->dr_qp->b_cyl )
		{
			/** Start this drive seeking **/

			xycommand( dp, XY_SEEK );
			dp->dr_qstate = Q_SEEK;
		}
		else
			dp->dr_qstate = Q_RDY;

  /*
  **	Attend to "seek done" status for other drives
  */

  if ( (x = XYSTAT) & ANYSKDN )
	for ( dp = xydrv, i = FSTSKDN ; dp < &xydrv[NDRV] ; i =<< 1, dp++ )
		if ( x & i )
		{
			dp->dr_qstate = Q_RDY;
			break;
		}

  /*
  **	"Round robin" poll of ready drives
  */

  for ( dp = lastdrive ; ((dp == &xydrv[NDRV-1]) ? (dp = xydrv) : ++dp) && (dp != lastdrive) ; )
# endif	MANY_DRIVES

	if ( (xydp == NULL) && (dp->dr_state == DR_RDY) && (dp->dr_qstate == Q_RDY) )
	{
		/** Start i/o on this drive **/

		xycommand( dp, (dp->dr_qp->b_flags & B_READ ? XY_READ : XY_WRITE) );
		dp->dr_qstate = Q_IO;
#		ifdef	MANY_DRIVES
		lastdrive = dp;
		break;
#		endif	MANY_DRIVES
	}
}




/*
**	Set up command in controller registers
*/

xycommand( dp, com )
  register struct drive *dp;
{
	register struct buf	*bp = dp->dr_qp;
  {
	register		x;

	if ( (x = com & XY_COM) == XY_READ || x == XY_WRITE )
		xydp = dp;	/** indicate controller busy on this i/o **/
	else
		xydp = NULL;

	dp->dr_com = x;
  }
  {
	register		*rp = &XYCYL;

	*rp = bp->b_cyl;				/* XYCYL  */
#	ifdef	MANY_DRIVES
	if ( xydp )
	{
#	endif	MANY_DRIVES
		*--rp = bp->b_wcount;			/* XYWCNT */
		*--rp = bp->b_addr;			/* XYCAR  */
#	ifdef	MANY_DRIVES
	}
	else
		--rp--;
#	endif	MANY_DRIVES
	*--rp = bp->b_ush;				/* XYUSH  */
	*--rp = (bp->b_xmem<<12)|com|INTEB|GO;		/* XYCSR  */
  }

	if ( bp->b_cyl > dp->dr_cyl )
		dp->dr_dir = IN;
	else
		if ( dp->dr_cyl > bp->b_cyl )
			dp->dr_dir = OUT;

	dp->dr_cyl = bp->b_cyl;
}




/*
**	Command completion interrupt handler
*/

xyintr()
{
	register struct drive	*dp;
	register struct buf	*bp;
	register unsigned	x;
	extern unsigned		xyerrors();

  if ( XYCSR > 0 )
  {
	/** Last command completed ok **/

	if ( dp = xydp )
	{
		/** I/O command completion **/

		bp = dp->dr_qp;

done:
		xydp = NULL;
		dp->dr_errcnt = 0;
		dp->dr_rtzcnt = 0;
		dp->dr_qstate = ( (dp->dr_qp = bp->b_next) != 0 );	/* Q_EMPTY or Q_WAIT */
#		ifdef	DRIVE_STATS
		dp->dr_qsize--;
#		endif	DRIVE_STATS

		xystart();

		bp->b_resid = 0;
		iodone( bp );
	}
	else
	{
		if ( (x = XYSTAT) & SEEKDON )
		{
			/** Seek complete **/

#			ifdef	MANY_DRIVES
			dp = &xydrv[x & SEEKID];
#			else	MANY_DRIVES
			dp = xydrv;
#			endif	MANY_DRIVES

			if ( dp->dr_state == DR_RDY )

				/** Normal seek complete **/

				dp->dr_qstate = Q_RDY;
			else
			{
				/** Drive error fixed or Drive come on line **/

				dp->dr_state = DR_RDY;
				if ( bp = dp->dr_qp )
					bp->b_flags =& ~B_ERROR;
			}
		}

		xystart();
	}
  }
  else
  {
	/*
	**	ERROR
	*/

#	ifdef	MANY_DRIVES
	dp = &xydrv[(XYUSH>>12)&3];
#	else	MANY_DRIVES
	dp = xydrv;
#	endif	MANY_DRIVES
	bp = dp->dr_qp;

	/*
	**	log errors and check for non-recoverable errors
	*/

	if ( (x = xyerrlog( dp->dr_errors )) == 0 )
	{
		/** Soft error recoverable **/

		/** CRC Error Correction ? **/

		if ( (x = dp->dr_errcnt++) < (((sizeof xyretrycm)/2)*3) )
		{
			/** In place retry from Error Recovery Strategy table **/

			if ( ((x = xyretrycm[x/3]) & SEEKINH)
			   && ((dp->dr_qstate != Q_IO) || ((-bp->b_wcount) > 256))
			   )
				goto error;	/* unsuitable strategy */

			xycommand( dp, dp->dr_com|x );
		}
		else
		{
			/** Retry Strategy exhausted -- recalibrate ? **/
error:
			if ( dp->dr_rtzcnt++ == 0 )
			{
				/** Recalibrate drive and try again **/

				dp->dr_errcnt = 0;
rtz:
				xycommand( dp, XY_RTZ );
				dp->dr_cyl = 0;
				dp->dr_qstate = Q_WAIT;
			}
			else
			{
				/** Too many errors, clear controller and give up on this i/o **/

				xyerrprt();	/* print out controller registers */

				XYCSR = XY_CLEAR|GO;

				bp->b_flags =| B_ERROR;
				goto done;
			}
		}
	}
	else
	{
		/*
		**	HARD error
		*/

		xyerrprt();	/* Print controller registers */

		bp->b_flags =| B_ERROR;

		if ( x & PROGERR )
			goto done;	/* Can't fix this one ! */

		if ( (dp->dr_com == XY_WRITE) && (x & REWRITE) )
		{
			/** Almost certainly a BUSERR **/
#			ifdef	RE_WRITE_FIX
			/** Must do a rewrite to avoid an unreadable sector **/

			register unsigned	y;

			x = bp->b_addr;	bp->b_addr = 0;
			y = bp->b_xmem;	bp->b_xmem = 0;

			xycommand( dp, XY_WRITE );

			bp->b_addr = x;
			bp->b_xmem = y;
#			else	RE_WRITE_FIX
			goto done;
#			endif	RE_WRITE_FIX
		}
		else
		{
			dp->dr_state = DR_ERR;

			if ( x & RECAL )
				if ( dp->dr_rtzcnt++ < RECALTRYS )
					/*
					** Attempt recalibration
					*/
					goto rtz;
				else
				{
					/** Drive has unsolvable problem **/
faultclear:
					xycommand( dp, XY_FLTCL );
					goto done;
				}
			else
				if ( x & DRVUNR )
				{
					/** Drive off-line **/

					extern char	*panicstring;

					dp->dr_state = DR_UNR;
					panicstring++;
					printf( "\nXY drive %d off line!\n", dp->dr_id );
					panicstring = 0;
				}
				else
					/*
					** Hard drive error
					*/
					if ( x & DRVFLT )
						goto faultclear;
					else
					{
						/** Clear controller **/

						XYCSR = XY_CLEAR|GO;
						goto done;
					}
		}
	}
  }
}




/*
**	Log errors and return any hard errors
*/

unsigned xyerrlog( ep )
  register unsigned *ep;
{
	register unsigned	e = XYERR;
	register unsigned	i = 1;

  do
	if ( e & i )
		if ( ++*ep == 0 )
			--*ep;
  while
	( ep++, i =<< 1 );

  return( e & ~RETRYER );
}




/*
**	Print out controller registers
*/

xyerrprt()
{
	register unsigned	*ep = XYADDR;

	printf( "\nXYERR=%o,ST=%o,CL=%o,WC=%o,BA=%o,DA=%o,CS=%o\n"
			 ,*ep,*ep++,*ep++,*ep++,*ep++,*ep++,*ep++	/* (stacked in reverse) */
	      );
}




/*
**	Physical I/O
*/

xyread( dev )
{
  physio( xystrategy, XYRAWBUF, dev, B_READ );
}

xywrite( dev )
{
  physio( xystrategy, XYRAWBUF, dev, B_WRITE );
}
