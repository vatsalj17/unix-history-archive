/*
 *	Optimised RHP04 and RJP04 driver
 *
 *	When instructed correctly (via the ifdefs) this driver will attempt
 *	to queue io requests by drive, cylinder and sector, and endeavour
 *	to overlap seeks with data transfers for more than one drive.
 *	Method is very similar to Childrens Museum RK driver.
 *
 *	Some error correction and recovery is attempted.
 *	You have been WARNED.
 *
 *	Based on the novel by Peter Ivanov	UNSWDCS
 *
 *	Debugged and recoded by		Ian Johnstone	AGSM
 *				and
 *					Chris Maltby	DCS.
 */

#include	"../defines.h"
#include	"../param.h"
#include	"../buf.h"
#include	"../conf.h"
#include	"../user.h"
#include	"../seg.h"

#define	PRESEEK		/* enable overlapped seeks on multiple drives */
#define	SWEEP		/* enable checking to force max head travel */
#define	RJP04		/* define for RP04's on the unibus,
			   ONLY will work for non 11/70 because of rhstart.
			   undefine for RP04's on RH70	*/
/* ( no HPECC )
#define	HPECC		/* enable data check correction */
/*
#define	HPSTATS		/* define to enable stats collection */
/* ( no NO_OFFLINEQ )
#define	NO_OFFLINEQ	/* stop qing of requests for offlined drives */
/* ( no DROP_OFFLINEQ )
#define	DROP_OFFLINEQ	/* delete drive request q when offlined */

struct
{
	int	hpcs1;	/* Control and Status register 1 */
	int	hpwc;	/* Word Count register */
	int	hpba;	/* Bus address register */
	int	hpda;	/* Desired disc address - sector/track */
	int	hpcs2;	/* Control and Status register 2 */
	int	hpds;	/* Drive status register */
	int	hper1;	/* Error register 1 */
	int	hpas;	/* Attention summary register */
	int	hpla;	/* Disc position look-ahead register */
	int	hpdb;	/* Data buffer */
	int	hpmr;	/* Maintenance register */
	int	hpdt;	/* Drive type encoding */
	int	hpsn;	/* Drive serial number */
	int	hpof;	/* Offset register - contains fm22 bit */
	int	hpdc;	/* Desired cylinder address register */
	int	hpcc;	/* Current cylinder address register */
	int	hper2;	/* Error register 2 */
	int	hper3;	/* Error register 3 */
	int	hppos;	/* Burst error bit position */
	int	hppat;	/* Burst error bit pattern */
#ifndef	RJP04
	int	hpbae;	/* 11/70 bus extension register */
	int	hpcs3;	/* Control and Status register 3 */
#endif	RJP04
};

#define	HPADDR	0176700
#define	NDRV	  1	/* the number of real rp04 drives on controller */
#define	MAXDRV	  8	/* the number of drives that can be attached */
#define	NCYLS	411	/* cylinders per pack */
#define	NTRKS	 19	/* tracks per cylinder */
#define	NSECS	 22	/* sectors per track */
#define	TVOL	(sizeof hp_sizes/sizeof hp_sizes[0])	/* total number of volumes */
#define	HPAGE	10	/* number of times this block may be pre-empted for io
					   before io is forced	*/
#define	HPSPL	spl5	/* priority of hp disc */

/*
 *	The following maps logical volumes of various sizes to locations
 *	on various drives. This must be altered to add extra drives.
 *	PLEASE GET THEM CORRECT FIRST TIME.
 */

struct
{
	unsigned	nblocks;
	int		cyloff;
	int		drive;
}
hp_sizes[]
{
/* 00:   0->155 (156cyls) drive 0 */	65208,   0, 0,
/* 01: 156->219 ( 64cyls) drive 0 */	26752, 156, 0,
/* 02: 220->254 ( 35cyls) drive 0 */	14630, 220, 0,
/* 03: 255->410 (156cyls) drive 0 */	65208, 255, 0,
};

/*
 *	structure of an hp disc queue
 */

struct hpq
{
	char	hpq_flags;		/* flags for qs */
					/* assumed zero - HPOFF==0 */
#define	HPOFF		00	/* drive offline */
#ifdef	PRESEEK
#define	HPSEEK		01	/* seeking flag for q header */
#endif	PRESEEK
#define	HPREADY		02	/* ready for io flag for q header */
#define	HPBUSY		03	/* data transfer in progress */
#define	HPRECAL		04	/* recalibrate in progress */
#define	HPIDLE		05	/* nought doing */

#ifdef	SWEEP
	char	hpq_dirf;		/* current direction of head movement */
#define	UP	1		/* the disc cyls are moving up */
#define	DOWN	0		/* the disc cyls are moving down */

#endif	SWEEP
	struct	buf	*hpq_bufp;	/* pointer to first buffer in queue for this drive */
#ifdef	HPSTATS
	char	hpq_nreq;		/* number of requests in queue - stats */
	char	hpq_rmax;			/* high water mark for q */
	unsigned hp_cyls[NCYLS];	/* accesses per cylinder */
#endif	HPSTATS
} hpq[NDRV];

#ifdef	HPSTATS
/*
 *	gather statistics to decide on optimal interleaving factor.
 *	When starting an io accumulate in hpintlv the count of
 *	each segment to go to io transfer start.
 */
long hpintlv[88];	/* divide disk into 88 segments */
#endif	HPSTATS

int intlv 1;	/* interleaving factor for queueing io on same cylinder */
int hpintst 2;	/* DEBUG try this much look-ahead */


/*
 *	We use a few buffer variables for other purposes here, ie:
 *		b_scratch	to hold unit/cylinder address of this request
 *		b_resid		to hold track/sector address of this request
 *		av_back		to hold request age count
 */

#define	hp_cyl		b_scratch	/* at least an int */
#define	hp_sector	b_resid.lobyte
#define	hp_track	b_resid.hibyte
#define	hp_trksec	b_resid
#define	hp_age		av_back

struct	devtab	hptab;

#ifndef	RAW_BUFFER_POOL
struct	buf	hpbuf;
#endif	RAW_BUFFER_POOL

/* various drive commands utilised herein */

#define	GO		 01
#define	PRESET		020	/* read-in-preset */
#define	RECAL		 06	/* recalibrate */
#define	DCLR		010	/* drive clear */
#define	SEEK		 04	/* seek */
#define	SEARCH		030	/* search */
#define	UNLOAD		 02	/* unload pack */

/*	hpds bits	*/

#define	DRY	   0200		/* drive ready */
#define	VV	   0100		/* volume valid */
#define	MOL	 010000		/* medium on line */
#define	ERR	 040000		/* the OR of error bits */
#define	ATA	0100000		/* attention bit */

/*	hpcs1 bits	*/

#define	IE	   0100		/* interrupt enable bit */
#define	RDY	   0200		/* controller ready */
#define	TRE	 040000		/* the OR of error bits */

/*	hper1 bits	*/

#define	ECH	 000100		/* hard ecc error */
#define	AOE	 001000		/* end of drive error */
#define	WLE	 004000		/* Write protect error */
#define	DTE	 010000		/* drive timing error */
#define	OPI	 020000		/* operation incomplete */
#define	UNS	 040000		/* drive unsafe */
#define	DCK	0100000		/* data check error for RHPs and RJPs (DEC) */

/*	hpcs2 bits	*/

#define	CLR	040		/* controller clear */

/*	hpof bits	*/

#define	FMT22	 010000		/* 16bit format for RHPs and RJPs (DEC) */




hpopen(dev)
{
	register int	n;

	/*
	**	If the requested drive is offline -> error
	**	Initialize it if necessary. Error if this fails.
	*/
	if((n = dev.d_minor) >= TVOL)
	{
		u.u_error = EOPENFAIL;
		return;
	}
	n = hp_sizes[n].drive;
	HPSPL();
	if( hpq[n].hpq_flags == HPOFF)
	{
		hpdinit(n);
		if(hpq[n].hpq_flags == HPOFF)
		{
			u.u_error = EOPENFAIL;
		}
	}
	spl0();
}

hpstrategy(bp)
register struct buf	*bp;
{
	register unsigned	p1, p2;
	int dirf;

	p1 = bp->b_dev.d_minor;

	/*
	 *	Validate the request
	 */

	p2 = &hp_sizes[p1];
#ifdef	NO_OFFLINEQ | DROP_OFFLINEQ
	if( p2->nblocks <= bp->b_blkno || hpq[p2->drive] == HPOFF )
#else
	if( p2->nblocks <= bp->b_blkno )
#endif	NO_OFFLINEQ | DROP_OFFLINEQ
	{
		bp->b_flags =| B_ERROR;
		iodone(bp);
		return;
	}

#ifdef	_1170 & RJP04
	if(bp->b_flags & B_PHYS)
	{
		mapalloc(bp);
	}
#endif	_1170 & RJP04

	bp->av_forw = 0;
	bp->hp_age = HPAGE;
	bp->b_error = 0;	/* clear error count */
	bp->hp_cyl = p2->cyloff;

	p1 = bp->b_blkno;
	bp->hp_sector = lrem(p1,NSECS);	/* sector */
	p1 = ldiv(p1,NSECS); /* trk and cyl */
	bp->hp_track = p1 % NTRKS;
	bp->hp_cyl =+ p1/NTRKS;
	p2 = &hpq[p2->drive];

/*
 *	Now "hp_cyl" contains "cylinder"
 *	and "hp_trksec" contains the "track and sector"
 */

#ifdef	HPSTATS
	p2->hp_cyls[bp->hp_cyl]++;
	p2->hpq_nreq++;
	if( p2->hpq_nreq > p2->hpq_rmax ) p2->hpq_rmax = p2->hpq_nreq;
#endif	HPSTATS

	HPSPL();
	if ((p1 = p2->hpq_bufp) == NULL)
	{
		/* this queue is empty */
		p2->hpq_bufp = bp;
		hpstart();
	}
	else
	{
		/*
		 * the queue is not empty, so place in queue so as to
		 * minimise head movement.
		 */
#ifdef	SWEEP
		dirf = p2->hpq_dirf;
#endif	SWEEP

		p2 = p1->av_forw;
		while(p2)
		{
			/* skip any overtaken blocks */
			if( !(p2->hp_age) )
				p1 = p2;
			p2 = p2->av_forw;
		}

		for( ; p2 = p1->av_forw; p1 = p2)
		{
			if( p1->hp_cyl<=bp->hp_cyl && bp->hp_cyl<=p2->hp_cyl
			 || p1->hp_cyl>=bp->hp_cyl && bp->hp_cyl>=p2->hp_cyl )
			{
				while (bp->hp_cyl == p2->hp_cyl)
				{
					/*
					** for a cylinder match, do the
					** rotational optimisation.
					** intlv is presumably the optimal value.
					** SEE HPSTATS as to how this could be done
					*/
					if(p2->hp_sector > p1->hp_sector)
					{
						if(bp->hp_sector > p1->hp_sector + intlv
						&& bp->hp_sector < p2->hp_sector - intlv)
							break;
					}
					else
					{
						if(bp->hp_sector > p1->hp_sector + intlv
						|| bp->hp_sector < p2->hp_sector - intlv)
							break;
					}
					p1 = p2;
					if( !(p2 = p1->av_forw) ) break;
				}
				break;
			}

#ifdef SWEEP
			else
			{
				if (dirf == UP)
				{
					if(p2->hp_cyl < p1->hp_cyl)
						if(bp->hp_cyl > p1->hp_cyl)
							break;
						else
							dirf = DOWN;
				}
				else
				{
					if(p2->hp_cyl > p1->hp_cyl)
						if(bp->hp_cyl < p1->hp_cyl)
							break;
						else
							dirf = UP;
				}
			}
#endif	SWEEP
		}

		bp->av_forw = p2;
		p1->av_forw = bp;

		while(p2)
		{
			/* count down overtaken blocks */
			p2->hp_age--;
			p2 = p2->av_forw;
		}
	}

	spl0();
}

/*
 *	start seeks as required and possibly one data transfer.
 */
hpstart()
{
	/* called at HPSPL or greater */

	register struct buf *bp;
	register struct hpq *qp;
	register int	n;

	static int	drv, ioage;	/* we assume that these are zero initially */


#ifdef	PRESEEK
	for( n = NDRV; --n >= 0;)
	{
		qp = &hpq[n];
		if( (bp = qp->hpq_bufp) && (qp->hpq_flags == HPIDLE) )
		{
			/*
			**	for all available drives start seeking
			*/
			HPADDR->hpcs2 = n;
			if(HPADDR->hpcc == bp->hp_cyl)
			{
				qp->hpq_flags = HPREADY;
			}
			else
			{
				int xx;
#ifdef	SWEEP
				if(bp->hp_cyl > HPADDR->hpcc)
					qp->hpq_dirf = UP;
				else if(bp->hp_cyl < HPADDR->hpcc)
					qp->hpq_dirf = DOWN;
#endif	SWEEP
				xx = bp->hp_sector - hpintst;
				if( xx < 0 ) xx =+ 22;
				xx.hibyte = bp->hp_track;
				HPADDR->hpda = xx;
				HPADDR->hpdc = bp->hp_cyl;
				qp->hpq_flags = HPSEEK;
				HPADDR->hpcs1.lobyte = IE | SEARCH | GO;
			}
		}
	}
#endif	PRESEEK
	/*
	 *	check if possible to start an io
	 */
	for( n = NDRV; --n >= 0;)
		if(hpq[n].hpq_flags == HPBUSY ) return;

	if(HPADDR->hpcs1 & RDY)	/* ensure controller available */
	{
		/*
		**	try to start an IO
		*/
		n = NDRV;
		do
		{
			qp = &hpq[drv];
			if( (bp = qp->hpq_bufp) && qp->hpq_flags == HPREADY)
			{
#ifdef	HPSTATS
				int a;

#endif	HPSTATS
				qp->hpq_flags = HPBUSY;
				HPADDR->hpcs2 = drv;
				HPADDR->hpdc = bp->hp_cyl;
#ifdef	HPSTATS
				a = bp->hp_sector * 4;
				a =- HPADDR->hpla >> 4;
				if( a <= 3 ) a =+ 88;
				hpintlv[a-4]++;
#endif	HPSTATS
#ifdef	RJP04
				rhstart(bp, &HPADDR->hpda, bp->hp_trksec);
#else
				rhstart(bp, &HPADDR->hpda, bp->hp_trksec, &HPADDR->hpbae);
#endif	RJP04
				if( --ioage <= 0)
				{
					ioage = HPAGE;
					if( ++drv >= NDRV)
						drv = 0;
				}
				return;
			}
			if( ++drv >= NDRV) drv=0;
			ioage = HPAGE;
		} while (--n > 0);
	}

	if( !(HPADDR->hpcs1 & IE))
		for(n=0; n<NDRV; n++)
		{
			/*
			 *	if the IE bit has not been set,
			 *	then nothing is happening and nothing
			 *	was started, so find a drive which will accept
			 *	a command and set the IE bit with a nop
			 */

			HPADDR->hpcs2 = n;
			if(HPADDR->hpds & DRY)
			{
				HPADDR->hpcs1 = IE;
				return;
			}
		}

}


hpintr()
{
	/* called at HPSPL or greater */

	register int	n;
	register struct hpq *qp;
	register struct buf *bp;

	/*
	**	An error has occured and/or a data transfer has completed.
	*/


	for(n=0; n<NDRV; n++)
	{
		HPADDR->hpcs2 = n;	/* select drive */
		qp = &hpq[n];
		if( (HPADDR->hpds & ATA) || qp->hpq_flags == HPBUSY)
		{
			bp = qp->hpq_bufp;
			if( (HPADDR->hpds & MOL) && !(HPADDR->hpds & VV) )
			{
				/* drive has come online */
				printf("HP drive %d turned on\n", n);
				hpdinit(n);
			}
			else if( !(HPADDR->hpds & MOL) )
			{
				/* drive down - disable and flag */
				printf("HP drive %d turned off\n", n);
				qp->hpq_flags = HPOFF;
#ifdef	DROP_OFFLINEQ
		dropq:
				while(bp)
				{
					bp->b_flags =| B_ERROR;
					iodone(bp);
					bp = bp->av_forw;
				}
				qp->hpq_bufp = 0;
#endif	DROP_OFFLINEQ
			}
			else if( HPADDR->hpds & ERR)
			{
				/* a drive error */
				int	er1;	/* saves the error register */

				hpregs(qp);
				er1 = HPADDR->hper1;
				HPADDR->hpcs1.lobyte = IE | DCLR | GO;
				while( !(HPADDR->hpds & DRY));
				if( er1 & (UNS | DTE | OPI) )
				{
					if( HPADDR->hper1 & UNS)
					{
						/* Still unsafe, unload for safety */
						qp->hpq_flags = HPOFF;
						HPADDR->hpcs1.lobyte = IE | UNLOAD | GO;
						printf("HP drive %d UNSAFE\n", n);
						printf("\nSTOP and re-START drive to recover\n\n");
#ifdef	DROP_OFFLINEQ
						goto dropq;
#endif	DROP_OFFLINEQ
					}
					else
					{
						/* Okay now, recal and go */
						qp->hpq_flags = HPRECAL;
						HPADDR->hpcs1.lobyte = IE | RECAL | GO;
					}
				}
#ifdef	HPECC
				else if( (er1 & DCK) && !(er1 & ECH))
				{
					if( hpecc() )
						continue; /* IO resumed */
					else
						HPADDR->hpcs1 = TRE | IE;
						goto trcmplt; /* IO done */
				}
#endif	HPECC
				switch(qp->hpq_flags)
				{
			    case HPOFF:
			    case HPREADY:
			    case HPIDLE:
					break;
			    case HPBUSY:
					HPADDR->hpcs1 = TRE | IE;
					if( er1 & AOE )
					{
						/* This might occur if hp_sizes wrong */
						HPADDR->hpcs1 = TRE | IE;
						goto trcmplt;
					}
					else if( er1 & WLE )
					{
						/* Drive is wrt protected - give up */
						bp->b_flags =| B_ERROR;
						goto unlink;
					}
#ifdef	PRESEEK
			    case HPSEEK:
					qp->hpq_flags = HPIDLE;
#else
					qp->hpq_flags = HPREADY;
#endif	PRESEEK
					goto errec;
			    case HPRECAL:
					if( qp->hpq_bufp )
						goto errec;
				}
			}
			else
			{
				/*
				 * Operation complete. Transfer or seek/recalibrate
				 */
				if( qp->hpq_flags == HPBUSY)
				{
					if(HPADDR->hpcs1 & RDY)
					{
						if( HPADDR->hpcs1 & TRE )
						{
							hpregs(qp);
							HPADDR->hpcs1 = TRE | IE;
				errec:
							if( ++bp->b_error >= 10 )
							{
								bp->b_flags =| B_ERROR;
								goto unlink;
							}
						}
						else
						{
							/* Transfer complete SUCCESS! */
				trcmplt:
							bp->b_resid = HPADDR->hpwc;
				unlink:
							qp->hpq_bufp = bp->av_forw;
							iodone(bp);
						}
					}
					else
					{
						continue;
					}
				}
#ifdef	PRESEEK
				if( qp->hpq_flags == HPSEEK )
					qp->hpq_flags = HPREADY;
				else if(qp->hpq_flags != HPRECAL)
					qp->hpq_flags = HPIDLE;
#else
				if(qp->hpq_flags != HPRECAL)
					qp->hpq_flags = HPREADY;
#endif	PRESEEK
			}
	clear:
			HPADDR->hpas = 1 << n;
		}
	}
	hpstart();

}

hpregs(qp)
struct hpq	*qp;
{
	static struct hperrmsgs
	{
			char *str;	/* identify which register */
			int  *reg;	/* address of device register */
	}
	hperrmsgs[]
	{
			"ER1", &HPADDR->hper1,
			"CS1", &HPADDR->hpcs1,
			"DS",  &HPADDR->hpds,
			"CS2", &HPADDR->hpcs2,
			"WC",  &HPADDR->hpwc,
			"BA",  &HPADDR->hpba,
			"DC",  &HPADDR->hpdc,
			"DA",  &HPADDR->hpda,
			"AS",  &HPADDR->hpas,
			0
	};
	register struct hperrmsgs *p = hperrmsgs;

	printf("HPERR\t");
	do
	{
			printf("%s=%o, ", p->str, *(p->reg) );
			p++;
	} while( p->str );
	printf("FLAGS=%o\n",qp->hpq_flags);
}

/*
 *	Physical IO:
 *	truncate transfers at the ends of logical volumes
 */

hpread(dev)
int	dev;
{
	hpphys( dev , B_READ );
}

hpwrite(dev)
int	dev;
{
	hpphys( dev , B_WRITE );
}

hpphys( dev , flag )
int	dev;
int	flag;	/* B_READ or B_WRITE */
{
	register unsigned a, b, c;

	a = hp_sizes[dev.d_minor & 07].nblocks;
	b = u.u_offset >> 9;
	if(b >= a)
	{
		u.u_error = ENXIO;
		return;
	}
	c = u.u_count;
	if(ldiv(c+511, 512) + b > a)
		c = (a - b) << 9;
	a = u.u_count - c;
	u.u_count = c;
#ifdef	RAW_BUFFER_POOL
	physio(hpstrategy,      0, dev, flag);
#else
	physio(hpstrategy, &hpbuf, dev, flag);
#endif	RAW_BUFFER_POOL
	u.u_count =+ a;
}

hpdinit(n)
register n;
{
	/* called at HPSPL or greater */

	HPADDR->hpcs2 = n;
	if(HPADDR->hpds & MOL)	/* this may generate a NED error */
				/* hpintr will handle it */
	{
		HPADDR->hpcs1.lobyte = IE | DCLR | GO;
		/*
		**	possibly some risk in these while loops here and
		**	in previous code in that if the drive is maybe
		**	MOL & !(DRY) ever....... not very likely and you
		**	would be buggered if it was.....
		*/
		while( !(HPADDR->hpds & DRY) );
		HPADDR->hpcs1.lobyte = IE | PRESET | GO;
		while( !(HPADDR->hpds & DRY) );
		HPADDR->hpof = FMT22;
#ifdef	PRESEEK
		hpq[n].hpq_flags = HPIDLE;
#else
		hpq[n].hpq_flags = HPREADY;
#endif	PRESEEK
	}
}

#ifdef	POWER_FAIL
hppowf(rdev)
{
	/*
	 *	hp disc power fail recovery routine
	 *
	 *	It is ASSUMED that
	 *
	 *	. a power fail has occured and the cpu did a reset
	 *	. mem management registers have been restored
	 *	. as have unibus map regs as have
	 *	. the other vital ones.
	 */

	register struct buf	*bp;
	register struct hpq	*qp;
	register	d;

	long	n;

	HPADDR->hpcs2=CLR;
	for(d=0; d< NDRV; d++)
		hpq[n].hpq_flags = HPOFF;
	if(rdev)
	{
		for(d=0; d<NDRV; d++)
			{
				/*
				 *	this drive was on when it died
				 */

				HPADDR->hpcs2 = d;
				for(;;)
				{
					if( (HPADDR->hpcs1 & RDY) && (HPADDR->hpds & MOL) && (HPADDR->hpds & DRY) )
					{
						hpdinit(d);
						break;
					}
					for(n=2000000; n>0; n--);
					printf("HPERR PWR FAIL RECVRY WAITING DRIVE %o\n", d);
				}
			}
	}
	hpstart();

}
#endif	POWER_FAIL
