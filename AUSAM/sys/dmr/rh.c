#define	AGSM
/*
 *	Optimised RHP04, RJP04, RM02 and RM03 driver
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
 *
 *	RM02 features added by		Craig McGregor	Architecture
 */

#include	"../defines.h"
#include	"../param.h"
#include	"../buf.h"
#include	"../conf.h"
#include	"../user.h"
#include	"../seg.h"

/* (no PRESEEK)
#define	PRESEEK		/* enable overlapped seeks on multiple drives */
#define	SWEEP		/* enable checking to force max head travel */
/* (no RJP04)
#define	RJP04		/* define for RP04's on the unibus,
			   ONLY will work for non 11/70 because of rhstart.
			   undefine for RP04's on RH70	*/
/* (RHP04)
#define	RM02		/* define if RM02 or RM03 drive */
			/* if RM03 - define RJP04 */
/* (no ECC)
#define	ECC		/* enable data check correction */
/*
#define	RHSTATS		/* define to enable stats collection */
/* (no NO_OFFLINEQ)
#define	NO_OFFLINEQ	/* stop qing of requests for offlined drives */
/* (no DROP_OFFLINEQ)
#define	DROP_OFFLINEQ	/* delete drive request q when offlined */

struct
{
	int	rhcs1;	/* Control and Status register 1 */
	int	rhwc;	/* Word Count register */
	int	rhba;	/* Bus address register */
	int	rhda;	/* Desired disc address - sector/track */
	int	rhcs2;	/* Control and Status register 2 */
	int	rhds;	/* Drive status register */
	int	rher1;	/* Error register 1 */
	int	rhas;	/* Attention summary register */
	int	rhla;	/* Disc position look-ahead register */
	int	rhdb;	/* Data buffer */
	int	rhmr;	/* Maintenance register */
	int	rhdt;	/* Drive type encoding */
	int	rhsn;	/* Drive serial number */
	int	rhof;	/* Offset register - contains fmt16 bit */
	int	rhdc;	/* Desired cylinder address register */
	int	rhcc;	/* Current cylinder address register */
	int	rher2;	/* Error register 2 */
	int	rher3;	/* Error register 3 */
	int	rhpos;	/* Burst error bit position */
	int	rhpat;	/* Burst error bit pattern */
#ifndef	RJP04
	int	rhbae;	/* 11/70 bus extension register */
	int	rhcs3;	/* Control and Status register 3 */
#endif	RJP04
};

#ifndef	RM02
/* rp04 characteristics */
#define	NCYLS	411	/* cylinders per pack */
#define	NTRKS	 19	/* tracks per cylinder */
#define	NSECS	 22	/* sectors per track */

/* rp04 names */
#define	RHOPEN		hpopen
#define	RHSTRATEGY	hpstrategy
#define	RHSTART		hpstart
#define	RHINTR		hpintr
#define	RHREGS		hpregs
#define	RHECC		hpecc
#define	RHREAD		hpread
#define	RHWRITE		hpwrite
#define	RHPHYS		hpphys
#define	RHDINIT		hpdinit
#define	RHPOWF		hppowf
#define	RH_SIZES	hp_sizes
#define	RHQ		hpq
#define	RHINTLV		hpintlv
#define	RHINTST		hpintst
#define	RHTAB		hptab
#define	RHBUF		hpbuf
#else	RM02
/* rm02 characteristics */
#define	NCYLS	823	/* cylinders per pack */
#define	NTRKS	  5	/* tracks per cylinder */
#define	NSECS	 32	/* sectors per track */

/* rm02 names */
#define	RHOPEN		rmopen
#define	RHSTRATEGY	rmstrategy
#define	RHSTART		rmstart
#define	RHINTR		rmintr
#define	RHREGS		rmregs
#define	RHECC		rmecc
#define	RHREAD		rmread
#define	RHWRITE		rmwrite
#define	RHPHYS		rmphys
#define	RHDINIT		rmdinit
#define	RHPOWF		rmpowf
#define	RH_SIZES	rm_sizes
#define	RHQ		rmq
#define	RHINTLV		rmintlv
#define	RHINTST		rmintst
#define	RHTAB		rmtab
#define	RHBUF		rmbuf
#endif	RM02

#define	RHADDR	0176700
#define	MAXDRV	  8	/* the number of drives that can be attached */
#define	TVOL	(sizeof RH_SIZES/sizeof RH_SIZES[0])	/* total number of volumes */
#define	RHAGE	10	/* number of times this block may be pre-empted for io
					   before io is forced	*/
#define	RHSPL	spl5	/* priority of rh disc */

#ifdef	AGSM
#define	NDRV	  2	/* the number of real rp04/rm02 drives on controller */
#define	PRESEEK
#endif	AGSM
#ifdef	ARCH
#define	NDRV	  1	/* the number of real rp04/rm02 drives on controller */
#endif	ARCH

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
RH_SIZES[]
{
#ifdef	AGSM
/* 00:   0->155 (156cyls) drive 0 */	65208,   0, 0,
/*     user1:  ADMIN + LIBRARY    */
/* 01: 156->219 (64cyls) drive 0  */	26752, 156, 0,
/*     spare or 'swap' or '/tmp'  */
/* 02: 220->254 (35cyls) drive 0  */	14630, 220, 0,
/*     system disk                */
/* 03: 255->410 (156cyls) drive 0 */	65208, 255, 0,
/*     user0: STAFF + STUDENTS    */
/* 10:   0->155 (156cyls) drive 1 */	65208,   0, 1,
/*     srce:  SYSTEM SOURCE       */
/* 11: 156->219 (64cyls) drive 1  */	26752, 156, 1,
/*     /tmp + 'swap'              */
/* 12: 220->254 (35cyls) drive 1  */	14630, 220, 1,
/*     user2: non-agsm users      */
/* 13: 255->410 (156cyls) drive 1 */	65208, 255, 1,
/*     spare                      */
#endif	AGSM
#ifdef	ARCH
/* 00:   0->324 (325cyls) drive 0 */	52000,   0, 0,
/*     user1: spare + small users */
/* 01: 325->426 (102cyls) drive 0 */	16320, 325, 0,
/*     'swap' or '/tmp'           */
/* 02: 427->497 (71cyls) drive 0 */	11360, 427, 0,
/*     system disk                */
/* 03: 498->822 (325cyls) drive 0 */	52000, 498, 0,
/*     user0: users               */
#endif	ARCH
};

/*
 *	structure of an rh disc queue
 */

struct rhq
{
	char	rhq_flags;		/* flags for qs */
					/* assumed zero - RHOFF==0 */
#define	RHOFF		00	/* drive offline */
#ifdef	PRESEEK
#define	RHSEEK		01	/* seeking flag for q header */
#endif	PRESEEK
#define	RHREADY		02	/* ready for io flag for q header */
#define	RHBUSY		03	/* data transfer in progress */
#define	RHRECAL		04	/* recalibrate in progress */
#define	RHIDLE		05	/* nought doing */

#ifdef	SWEEP
	char	rhq_dirf;		/* current direction of head movement */
#define	UP	1		/* the disc cyls are moving up */
#define	DOWN	0		/* the disc cyls are moving down */

#endif	SWEEP
	struct	buf	*rhq_bufp;	/* pointer to first buffer in queue for this drive */
#ifdef	RHSTATS
	char	rhq_nreq;		/* number of requests in queue - stats */
	char	rhq_rmax;		/* high water mark for q */
	unsigned rh_cyls[NCYLS];	/* accesses per cylinder */
#endif	RHSTATS
} RHQ[NDRV];

#ifdef	RHSTATS
/*
 *	gather statistics to decide on optimal interleaving factor.
 *	When starting an io accumulate in RHINTLV the count of
 *	each segment to go to io transfer start.
 */
long	RHINTLV[NSECS*4];	/* divide disk into NSECS*4 segments */
#endif	RHSTATS

int	intlv	1;	/* interleaving factor for queueing io on same cylinder */
int	RHINTST	2;	/* DEBUG try this much look-ahead */


/*
 *	We use a few buffer variables for other purposes here, ie:
 *		b_scratch	to hold unit/cylinder address of this request
 *		b_resid		to hold track/sector address of this request
 *		av_back		to hold request age count
 */

#define	rh_cyl		b_scratch	/* at least an int */
#define	rh_sector	b_resid.lobyte
#define	rh_track	b_resid.hibyte
#define	rh_trksec	b_resid
#define	rh_age		av_back

struct	devtab	RHTAB;

#ifndef	RAW_BUFFER_POOL
struct	buf	RHBUF;
#endif	RAW_BUFFER_POOL

/* various drive commands utilised herein */

#define	GO		 01
#define	PRESET		020	/* read-in-preset */
#define	RECAL		 06	/* recalibrate */
#define	DCLR		010	/* drive clear */
#define	SEEK		 04	/* seek */
#define	SEARCH		030	/* search */
#define	UNLOAD		 02	/* unload pack */

/*	rhds bits	*/

#define	DRY	   0200		/* drive ready */
#define	VV	   0100		/* volume valid */
#define	MOL	 010000		/* medium on line */
#define	ERR	 040000		/* the OR of error bits */
#define	ATA	0100000		/* attention bit */

/*	rhcs1 bits	*/

#define	IE	   0100		/* interrupt enable bit */
#define	RDY	   0200		/* controller ready */
#define	TRE	 040000		/* the OR of error bits */

/*	rher1 bits	*/

#define	ECH	 000100		/* hard ecc error */
#define	AOE	 001000		/* end of drive error */
#define	WLE	 004000		/* Write protect error */
#define	DTE	 010000		/* drive timing error */
#define	OPI	 020000		/* operation incomplete */
#define	UNS	 040000		/* drive unsafe */
#define	DCK	0100000		/* data check error for RHPs and RJPs (DEC) */

/*	rhcs2 bits	*/

#define	CLR	040		/* controller clear */

/*	rhof bits	*/

#define	FMT16	 010000		/* 16bit format (as opposed to 18-bit format) (DEC) */




RHOPEN(dev)
{
	register int	n;

	/*
	**	If the requested drive is offline -> error
	**	Initialize it if necessary. Error if this fails.
	*/
	if ((n = dev.d_minor) >= TVOL)
	{
		u.u_error = EOPENFAIL;
		return;
	}
	n = RH_SIZES[n].drive;
	RHSPL();
	if (RHQ[n].rhq_flags == RHOFF)
	{
		RHDINIT(n);
		if (RHQ[n].rhq_flags == RHOFF)
		{
			u.u_error = EOPENFAIL;
		}
	}
	spl0();
}

RHSTRATEGY(bp)
register struct buf	*bp;
{
	register unsigned	p1, p2;
	int dirf;

	p1 = bp->b_dev.d_minor;

	/*
	 *	Validate the request
	 */

	p2 = &RH_SIZES[p1];
#ifdef	NO_OFFLINEQ | DROP_OFFLINEQ
	if (p2->nblocks <= bp->b_blkno || RHQ[p2->drive] == RHOFF)
#else	NO_OFFLINEQ | DROP_OFFLINEQ
	if (p2->nblocks <= bp->b_blkno)
#endif	NO_OFFLINEQ | DROP_OFFLINEQ
	{
		bp->b_flags =| B_ERROR;
		iodone(bp);
		return;
	}

#ifdef	_1170 & RJP04
	if (bp->b_flags & B_PHYS)
	{
		mapalloc(bp);
	}
#endif	_1170 & RJP04

	bp->av_forw = 0;
	bp->rh_age = RHAGE;
	bp->b_error = 0;	/* clear error count */
	bp->rh_cyl = p2->cyloff;

	p1 = bp->b_blkno;
	bp->rh_sector = lrem(p1,NSECS);	/* sector */
	p1 = ldiv(p1,NSECS); /* trk and cyl */
	bp->rh_track = p1 % NTRKS;
	bp->rh_cyl =+ p1/NTRKS;
	p2 = &RHQ[p2->drive];

/*
 *	Now "rh_cyl" contains "cylinder"
 *	and "rh_trksec" contains the "track and sector"
 */

#ifdef	RHSTATS
	p2->rh_cyls[bp->rh_cyl]++;
	p2->rhq_nreq++;
	if (p2->rhq_nreq > p2->rhq_rmax) p2->rhq_rmax = p2->rhq_nreq;
#endif	RHSTATS

	RHSPL();
	if ((p1 = p2->rhq_bufp) == NULL)
	{
		/* this queue is empty */
		p2->rhq_bufp = bp;
		RHSTART();
	}
	else
	{
		/*
		 * the queue is not empty, so place in queue so as to
		 * minimise head movement.
		 */
#ifdef	SWEEP
		dirf = p2->rhq_dirf;
#endif	SWEEP

		p2 = p1->av_forw;
		while (p2)
		{
			/* skip any overtaken blocks */
			if (!(p2->rh_age))
				p1 = p2;
			p2 = p2->av_forw;
		}

		for (; p2 = p1->av_forw; p1 = p2)
		{
			if (p1->rh_cyl<=bp->rh_cyl && bp->rh_cyl<=p2->rh_cyl
			 || p1->rh_cyl>=bp->rh_cyl && bp->rh_cyl>=p2->rh_cyl)
			{
				while (bp->rh_cyl == p2->rh_cyl)
				{
					/*
					** for a cylinder match, do the
					** rotational optimisation.
					** intlv is presumably the optimal value.
					** SEE RHSTATS as to how this could be done
					*/
					if (p2->rh_sector > p1->rh_sector)
					{
						if (bp->rh_sector > p1->rh_sector + intlv
						&& bp->rh_sector < p2->rh_sector - intlv)
							break;
					}
					else
					{
						if (bp->rh_sector > p1->rh_sector + intlv
						|| bp->rh_sector < p2->rh_sector - intlv)
							break;
					}
					p1 = p2;
					if (!(p2 = p1->av_forw)) break;
				}
				break;
			}

#ifdef SWEEP
			else
			{
				if (dirf == UP)
				{
					if (p2->rh_cyl < p1->rh_cyl)
						if (bp->rh_cyl > p1->rh_cyl)
							break;
						else
							dirf = DOWN;
				}
				else
				{
					if (p2->rh_cyl > p1->rh_cyl)
						if (bp->rh_cyl < p1->rh_cyl)
							break;
						else
							dirf = UP;
				}
			}
#endif	SWEEP
		}

		bp->av_forw = p2;
		p1->av_forw = bp;

		while (p2)
		{
			/* count down overtaken blocks */
			p2->rh_age--;
			p2 = p2->av_forw;
		}
	}

	spl0();
}

/*
 *	start seeks as required and possibly one data transfer.
 */
RHSTART()
{
	/* called at RHSPL or greater */

	register struct buf *bp;
	register struct rhq *qp;
	register int	n;

	static int	drv, ioage;	/* we assume that these are zero initially */


#ifdef	PRESEEK
	for (n = NDRV; --n >= 0;)
	{
		qp = &RHQ[n];
		if ((bp = qp->rhq_bufp) && (qp->rhq_flags == RHIDLE))
		{
			/*
			**	for all available drives start seeking
			*/
			RHADDR->rhcs2 = n;
			if (RHADDR->rhcc == bp->rh_cyl)
			{
				qp->rhq_flags = RHREADY;
			}
			else
			{
				int xx;
#ifdef	SWEEP
				if (bp->rh_cyl > RHADDR->rhcc)
					qp->rhq_dirf = UP;
				else if (bp->rh_cyl < RHADDR->rhcc)
					qp->rhq_dirf = DOWN;
#endif	SWEEP
				xx = bp->rh_sector - RHINTST;
				if (xx < 0) xx =+ 22;
				xx.hibyte = bp->rh_track;
				RHADDR->rhda = xx;
				RHADDR->rhdc = bp->rh_cyl;
				qp->rhq_flags = RHSEEK;
				RHADDR->rhcs1.lobyte = IE | SEARCH | GO;
			}
		}
	}
#endif	PRESEEK
	/*
	 *	check if possible to start an io
	 */
	for (n = NDRV; --n >= 0;)
		if (RHQ[n].rhq_flags == RHBUSY) return;

	if (RHADDR->rhcs1 & RDY)	/* ensure controller available */
	{
		/*
		**	try to start an IO
		*/
		n = NDRV;
		do
		{
			qp = &RHQ[drv];
			if ((bp = qp->rhq_bufp) && qp->rhq_flags == RHREADY)
			{
#ifdef	RHSTATS
				int a;

#endif	RHSTATS
				qp->rhq_flags = RHBUSY;
				RHADDR->rhcs2 = drv;
				RHADDR->rhdc = bp->rh_cyl;
#ifdef	RHSTATS
				a = bp->rh_sector * 4;
				a =- RHADDR->rhla >> 4;
				if (a <= 3) a =+ NSECS*4;
				RHINTLV[a-4]++;
#endif	RHSTATS
#ifdef	RJP04
				rhstart(bp, &RHADDR->rhda, bp->rh_trksec);
#else	RJP04
				rhstart(bp, &RHADDR->rhda, bp->rh_trksec, &RHADDR->rhbae);
#endif	RJP04
				if (--ioage <= 0)
				{
					ioage = RHAGE;
					if (++drv >= NDRV)
						drv = 0;
				}
				return;
			}
			if (++drv >= NDRV) drv=0;
			ioage = RHAGE;
		} while (--n > 0);
	}

	if (!(RHADDR->rhcs1 & IE))
		for (n=0; n<NDRV; n++)
		{
			/*
			 *	if the IE bit has not been set,
			 *	then nothing is happening and nothing
			 *	was started, so find a drive which will accept
			 *	a command and set the IE bit with a nop
			 */

			RHADDR->rhcs2 = n;
			if (RHADDR->rhds & DRY)
			{
				RHADDR->rhcs1 = IE;
				return;
			}
		}

}


RHINTR()
{
	/* called at RHSPL or greater */

	register int	n;
	register struct rhq *qp;
	register struct buf *bp;

	/*
	**	An error has occured and/or a data transfer has completed.
	*/


	for (n=0; n<NDRV; n++)
	{
		RHADDR->rhcs2 = n;	/* select drive */
		qp = &RHQ[n];
		if ((RHADDR->rhds & ATA) || qp->rhq_flags == RHBUSY)
		{
			bp = qp->rhq_bufp;
			if ((RHADDR->rhds & MOL) && !(RHADDR->rhds & VV))
			{
				/* drive has come online */
#ifndef	RM02
				printf("HP drive %d turned on\n", n);
#else	RM02
				printf("RM drive %d turned on\n", n);
#endif	RM02
				RHDINIT(n);
			}
			else if (!(RHADDR->rhds & MOL))
			{
				/* drive down - disable and flag */
#ifndef	RM02
				printf("HP drive %d turned off\n", n);
#else	RM02
				printf("RM drive %d turned off\n", n);
#endif	RM02
				qp->rhq_flags = RHOFF;
#ifdef	DROP_OFFLINEQ
		dropq:
				while (bp)
				{
					bp->b_flags =| B_ERROR;
					iodone(bp);
					bp = bp->av_forw;
				}
				qp->rhq_bufp = 0;
#endif	DROP_OFFLINEQ
			}
			else if (RHADDR->rhds & ERR)
			{
				/* a drive error */
				int	er1;	/* saves the error register */

				RHREGS(qp);
				er1 = RHADDR->rher1;
				RHADDR->rhcs1.lobyte = IE | DCLR | GO;
				while (!(RHADDR->rhds & DRY));
				if (er1 & (UNS | DTE | OPI))
				{
					if (RHADDR->rher1 & UNS)
					{
						/* Still unsafe, unload for safety */
						qp->rhq_flags = RHOFF;
						RHADDR->rhcs1.lobyte = IE | UNLOAD | GO;
#ifndef	RM02
						printf("HP drive %d UNSAFE\n", n);
#else	RM02
						printf("RM drive %d UNSAFE\n", n);
#endif	RM02
						printf("\nSTOP and re-START drive to recover\n\n");
#ifdef	DROP_OFFLINEQ
						goto dropq;
#endif	DROP_OFFLINEQ
					}
					else
					{
						/* Okay now, recal and go */
						qp->rhq_flags = RHRECAL;
						RHADDR->rhcs1.lobyte = IE | RECAL | GO;
					}
				}
#ifdef	ECC
				else if ((er1 & DCK) && !(er1 & ECH))
				{
					if (RHECC())
						continue; /* IO resumed */
					else
						RHADDR->rhcs1 = TRE | IE;
						goto trcmplt; /* IO done */
				}
#endif	ECC
				switch(qp->rhq_flags)
				{
			    case RHOFF:
			    case RHREADY:
			    case RHIDLE:
					break;
			    case RHBUSY:
					RHADDR->rhcs1 = TRE | IE;
					if (er1 & AOE)
					{
						/* This might occur if RH_SIZES wrong */
						RHADDR->rhcs1 = TRE | IE;
						goto trcmplt;
					}
					else if (er1 & WLE)
					{
						/* Drive is wrt protected - give up */
						bp->b_flags =| B_ERROR;
						goto unlink;
					}
#ifdef	PRESEEK
			    case RHSEEK:
					qp->rhq_flags = RHIDLE;
#else	PRESEEK
					qp->rhq_flags = RHREADY;
#endif	PRESEEK
					goto errec;
			    case RHRECAL:
					if (qp->rhq_bufp)
						goto errec;
				}
			}
			else
			{
				/*
				 * Operation complete. Transfer or seek/recalibrate
				 */
				if (qp->rhq_flags == RHBUSY)
				{
					if (RHADDR->rhcs1 & RDY)
					{
						if (RHADDR->rhcs1 & TRE)
						{
							RHREGS(qp);
							RHADDR->rhcs1 = TRE | IE;
				errec:
							if (++bp->b_error >= 10)
							{
								bp->b_flags =| B_ERROR;
								goto unlink;
							}
						}
						else
						{
							/* Transfer complete SUCCESS! */
				trcmplt:
							bp->b_resid = RHADDR->rhwc;
				unlink:
							qp->rhq_bufp = bp->av_forw;
							iodone(bp);
						}
					}
					else
					{
						continue;
					}
				}
#ifdef	PRESEEK
				if (qp->rhq_flags == RHSEEK)
					qp->rhq_flags = RHREADY;
				else if (qp->rhq_flags != RHRECAL)
					qp->rhq_flags = RHIDLE;
#else	PRESEEK
				if (qp->rhq_flags != RHRECAL)
					qp->rhq_flags = RHREADY;
#endif	PRESEEK
			}
	clear:
			RHADDR->rhas = 1 << n;
		}
	}
	RHSTART();

}

RHREGS(qp)
struct rhq	*qp;
{
	static struct rherrmsgs
	{
			char *str;	/* identify which register */
			int  *reg;	/* address of device register */
	}
	rherrmsgs[]
	{
			"ER1", &RHADDR->rher1,
			"CS1", &RHADDR->rhcs1,
			"DS",  &RHADDR->rhds,
			"CS2", &RHADDR->rhcs2,
			"WC",  &RHADDR->rhwc,
			"BA",  &RHADDR->rhba,
			"DC",  &RHADDR->rhdc,
			"DA",  &RHADDR->rhda,
			"AS",  &RHADDR->rhas,
			0
	};
	register struct rherrmsgs *p = rherrmsgs;

#ifndef	RM02
	printf("HPERR\t");
#else	RM02
	printf("RMERR\t");
#endif	RM02
	do
	{
			printf("%s=%o, ", p->str, *(p->reg));
			p++;
	} while (p->str);
	printf("FLAGS=%o\n",qp->rhq_flags);
}

/*
 *	Physical IO:
 *	truncate transfers at the ends of logical volumes
 */

RHREAD(dev)
int	dev;
{
	RHPHYS(dev, B_READ);
}

RHWRITE(dev)
int	dev;
{
	RHPHYS(dev, B_WRITE);
}

RHPHYS(dev, flag)
int	dev;
int	flag;	/* B_READ or B_WRITE */
{
	register unsigned a, b, c;

	a = RH_SIZES[dev.d_minor & 07].nblocks;
	b = u.u_offset >> 9;
	if (b >= a)
	{
		u.u_error = ENXIO;
		return;
	}
	c = u.u_count;
	if (ldiv(c+511, 512) + b > a)
		c = (a - b) << 9;
	a = u.u_count - c;
	u.u_count = c;
#ifdef	RAW_BUFFER_POOL
	physio(&RHSTRATEGY,      0, dev, flag);
#else	RAW_BUFFER_POOL
	physio(&RHSTRATEGY, &RHBUF, dev, flag);
#endif	RAW_BUFFER_POOL
	u.u_count =+ a;
}

RHDINIT(n)
register n;
{
	/* called at RHSPL or greater */

	RHADDR->rhcs2 = n;
	if (RHADDR->rhds & MOL)	/* this may generate a NED error */
				/* RHINTR() will handle it */
	{
		RHADDR->rhcs1.lobyte = IE | DCLR | GO;
		/*
		**	possibly some risk in these while loops here and
		**	in previous code in that if the drive is maybe
		**	MOL & !(DRY) ever....... not very likely and you
		**	would be buggered if it was.....
		*/
		while (!(RHADDR->rhds & DRY));
		RHADDR->rhcs1.lobyte = IE | PRESET | GO;
		while (!(RHADDR->rhds & DRY));
		RHADDR->rhof = FMT16;
#ifdef	PRESEEK
		RHQ[n].rhq_flags = RHIDLE;
#else	PRESEEK
		RHQ[n].rhq_flags = RHREADY;
#endif	PRESEEK
	}
}

#ifdef	POWER_FAIL
RHPOWF(rdev)
{
	/*
	 *	rh disc power fail recovery routine
	 *
	 *	It is ASSUMED that
	 *
	 *	. a power fail has occured and the cpu did a reset
	 *	. mem management registers have been restored
	 *	. as have unibus map regs as have
	 *	. the other vital ones.
	 */

	register struct buf	*bp;
	register struct rhq	*qp;
	register	d;

	long	n;

	RHADDR->rhcs2=CLR;
	for (d=0; d< NDRV; d++)
		RHQ[n].rhq_flags = RHOFF;
	if (rdev)
	{
		for (d=0; d<NDRV; d++)
			{
				/*
				 *	this drive was on when it died
				 */

				RHADDR->rhcs2 = d;
				for (;;)
				{
					if ((RHADDR->rhcs1 & RDY) && (RHADDR->rhds & MOL) && (RHADDR->rhds & DRY))
					{
						RHDINIT(d);
						break;
					}
					for (n=2000000; n>0; n--);
#ifndef	RM02
					printf("HPERR PWR FAIL RECVRY WAITING DRIVE %o\n", d);
#else	RM02
					printf("RMERR PWR FAIL RECVRY WAITING DRIVE %o\n", d);
#endif	RM02
				}
			}
	}
	RHSTART();

}
#endif	POWER_FAIL
#ifdef	ARCH
	help();
#endif	ARCH
