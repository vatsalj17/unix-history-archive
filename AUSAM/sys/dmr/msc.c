#
/*
 *	Optimised Microcomputer Systems 1100 driver
 *	used with Ampex DM 9100 disc drives.
 *	Dated 8 - 3 - 78.
 *
 *	When instructed correctly (via the ifdefs) this driver will attempt
 *	to queue io requests by drive, cylinder and sector, and endeavour
 *	to overlap seeks with data transfers for more than one drive.
 *	Method is very similar to Childrens Museum RK driver.
 *
 *	Some simple error correction and recovery is attempted, but nothing
 *	fancy. You have been WARNED.
 *
 *	Bugs and comments to	Peter Ivanov
 *				Dept. Computer Science,
 *				University of New South Wales.
 */

#include	"../defines.h"
#include	"../param.h"
#include	"../buf.h"
#include	"../conf.h"
#include	"../seg.h"
#include	"../user.h"

/*	global defines	*/
#define	SWEEP	/* enable checking to force max head travel */
/*#define	NO_SEEK		/* to stop pre-seeking */
/*#define	VOLUME_STATS	/* define to enable stats collection per volume */
/*#define	DRIVE_STATS	/* define to enable stats collection per drive and cylinder */
#define		IO_STATS	/* count sectors read & written */
/*#define	NO_OFFLINEQ	/* stop qing of requests for offlined drives */
/*#define	DROP_OFFLINEQ	/* delete drive request q when offlined */
/*#define	RETRIES_ONLY	/* don't print error messages for first soft error */
/*#define	LAST_CYL	/* to accumulate the info about the last seek distance on drive */

/*	configuration details	*/
#define	MSADDR	0176700	/* address of device */
#define	NDRV	2	/* the number of real 9100 drives on controller */
#define	MSCYLS	411	/* 411 cylinders per pack */
#define	MSTRK	19	/* number of tracks per cyl (platters) */
#define	MSSEC	21	/* number of sectors per track */
#define	MSBYTE	512	/* number of bytes per sector */
#define	TVOL	(sizeof ms_sizes/sizeof ms_sizes[0])	/* total number of volumes */
#define	MSAGE	100	/* number of times this block may be pre-empted for io
				   before io is forced	*/
#define	INTLV	5	/* interleaving factor for rp04 type drives */
#define	MSSPL	spl5	/* priority of ms disc */

/*	definition of device registers	*/
struct	{
	int	msccra;	/* command completion register A */
	int	msccrb;	/* command completion register B */
	int	mscsr;	/* control and status register */
	int	msctra;	/* command transmission register A */
	int	msctrb;	/* command transmission register B */
/*	int	filler;	/* dummy filler	*/
/*	int	mscsr;	/* second occurrence of control and status reg */
	};

/*	structure of an incore disc command descriptor - one per drive	*/
struct	msdesc
	{
	int	 ds_id;		/* command identifier - drive number for us */
	int	 ds_stat;	/* status word */
	char	 ds_nexthi;	/* hi portion of link to next descriptor */
	char	 ds_sfc;	/* system fault code */
	unsigned ds_nextlo;	/* lo portion of link to next descriptor */
	int	 ds_discop;	/* disc operation code */
	int	 ds_drive;	/* disc drive number */
	int	 ds_cylinder;	/* disc cylinder number */
	int	 ds_head;	/* disc head number (track) */
	int	 ds_sector;	/* disc sector number */
	unsigned ds_wcount;	/* word count */
	unsigned ds_memlo;	/* lo part of memory address */
	unsigned ds_memhi;	/* hi part of memory address - watch bit 6! */
	unsigned ds_rid1;	/* part one of record id on "readid"s */
	unsigned ds_rid2;	/* part two of record id on "readid"s */
/*
 *	it is possible for the controller to return many more words than this
 *	up to 41 infact but the command which causes this will NEVER be used.
 */
	};

/*
 *	The following maps logical volumes of various sizes to locations
 *	on various drives. This must be altered to add extra drives.
 *	PLEASE GET THEM CORRECT FIRST TIME.
 */

struct	{
	unsigned	nblocks;
	int	cyloff;
	int	drive;
#	ifdef	VOLUME_STATS
	long	refcount;	/* count of accesses to this volume */
#	endif
} ms_sizes[] {
/*	 0:	  0->163	(164cyls)	drive 0	*/ {	65436l,	  0,	0, }	/* system and source */
/*	 1:	164->213	( 50cyls)	drive 0	*/ ,{	19950l,	164,	0, }	/* users and swap */
/*	 2:	214->245	( 32cyls)	drive 0	*/ ,{	12768l,	214,	0, }	/* system disc  */
/*	 3:	246->409	( 164cyls)	drive 0	*/ ,{	65436l,	246,	0, }	/* system users */
/*	 4:	410->411	( 1cyls)	drive 0	*/ ,{	399l,	410,	0, }	/* MSC DIAGNOSTIC AREA */
/*	 5:	  0->163	(164cyls)	drive 1	*/ ,{	65436l,	  0,	1, }	/* secondary system and swap */
/*	 6:	164->213	( 50cyls)	drive 1	*/ ,{	19950l,	164,	1, }	/* user disk 1 */
/*	 7:	214->245	( 32cyls)	drive 1	*/ ,{	12768l,	214,	1, }	/* tmp disc  */
/*	 8:	246->409	( 164cyls)	drive 1	*/ ,{	65436l,	246,	1, }	/* user disc 2 */
/*	 9:	410->411	( 1cyls)	drive 1	*/ ,{	399l,	410,	1, }	/* MSC DIAGNOSTIC AREA */
};

#	ifdef	IO_STATS
	long	msrsec;		/* sectors read */
	long	mswsec;		/* sectors writ */
#	endif
/*
 *	structure of a disc queue
 */

struct msq
	{
	struct	buf	*ms_bufp;	/* pointer to first buffer in queue for this drive */
	struct	msdesc	ms_desc;	/* descriptor for this drive */
	int	ms_flags;		/* flags for qs */
#ifdef	SWEEP
	int	ms_lcyl;		/* last cylinder accessed on this drive */
	char	ms_dirf;		/* current direction of head movement */
#endif
#ifdef	DRIVE_STATS
	char	ms_nreq;		/* number of requests in queue - stats */
	char	ms_rmax;			/* high water mark for q */
	unsigned	ms_cyls[MSCYLS];	/* accesses per cylinder */
#endif
	} ms_q[NDRV];

/*	definition of io error retry strategy	*/
int	retry[]
	{
	0,	/* no variance */
	0,	/* no variance */
	0,	/* no variance */
	1,	/* early data strobe */
	2,	/* late data strobe */
	3,	/* positive cylinder offset, normal data strobe */
	4,	/* negative cylinder offset, normal data strobe */
	5,	/* positive cylinder offset, early data strobe */
	6,	/* negative cylinder offset, early data strobe */
	7,	/* positive cylinder offset, late data strobe */
	8	/* negative cylinder offset, late data strobe */
	};

/*	and so the max number of retries may be defined as	*/
#define	MSERRMAX	(sizeof retry/2)

/*
 *	We use a few buffer variables for other purposes here, ie:
 *		b_scratch	to hold unit/cylinder address of this request
 *		b_resid		to hold track/sector address of this request
 *		av_back		to hold request age count
 */

#define	ms_cyl		b_scratch	/* at least an int */
#define	ms_sector	b_resid.lobyte
#define	ms_track	b_resid.hibyte
#define	ms_age		av_back.integ

struct	devtab	mstab;

#ifndef	RAW_BUFFER_POOL
struct	buf	msbuf;
#endif

/*	some handy masks	*/
#define	BYTE	0377
#define	LSD	07	/* least significant octal digit */
#define	UNIT	016000	/* the position of unit no. in ms_cyl */
#define	CYL	0777	/* the desired/present cylinder bits */

/*	some equally handy numbers	*/
#define	UP	1	/* the disc cyls are moving up */
#define	DOWN	0	/* the disc cyld are moving down */
#define	SBYTE	8	/* amount by which to shift for a byte */
#define	SUNIT	10	/* amount by which to shift unit nos. */

/*	some flags		*/
#define	MS_IDLE		0	/* drive is quiescent */
#define	MS_SEEK		1	/* seeking flag for q header */
#define	MS_READY	2	/* ready for io flag for q header */
#define	MS_ERROR	3	/* doing error recovery */
#define	MS_IO		4	/* actually doing io */
#define	MS_RECAL	5	/* actually doing a recal */

/* ccra bits */
#define DSKPON	0100000		/* disk drive powered on */
#define CMDQERR	0010000		/* command queue error */
#define CMDNXM	0004000		/* nxm during command descriptor fetch */
#define CMDPAR	0002000		/* memory parity during descriptor fetch */

/* csr bits */
#define RDYCC	0100000		/* ready from command completion */
#define RESET	0002000		/* reset controller */
#define CCACK	0001000		/* command completion acknowledge */
#define CARDY	0000400		/* command address ready */
#define RDYCT	0000200		/* ready for command transmission */
#define CTIE	0000100		/* command transmission interrupt enable */
#define CCIE	0000040		/* command completion interrupt enable */
#define PWRCYC	0000020		/* controller power cycle */
#define CTLRON	0000010		/* controller power is on */
#define DMAON	0000004		/* direct memory access in progress */
#define BOOT	0000001		/* load bootstrap into location 0 */

/* status bits */
#define RETRY	0100000		/* retry occurred due to data miss */
#define DATAERR 0040000		/* uncorrectable data error (read) */
#define ECCERR	0020000		/* ecc correction made */
#define IDERR	0010000		/* error in sector ID field */
#define ADDRERR 0004000		/* bad address in sector ID field */
#define WRTPRTC	0002000		/* write protected sector */
#define ALTSECT	0001000		/* alternate sector flag */
#define DATMISS 0000400		/* data miss (hard) */
#define SEEKERR 0000200		/* illegal seek or seek incomplete */
#define DRVNRDY 0000100		/* drive not ready */
#define DRVFALT 0000020		/* drive fault or select lock */
#define NXMERR	0000010		/* transfer referenced non-existant memory */
#define PARERR	0000004		/* memory parity error during transfer */
#define	SYSFLT	0000002		/* system fault */
#define PRTCDRV 0000001		/* write protected drive */

/* a few opcodes */
#define	DSTATE	02		/* obtain drive status */
#define RECAL	04		/* recalibrate drive */
#define SEEK	05		/* seek specified cylinder */
#define WRITE	06		/* write data */
#define READ	020		/* read data */

char	ms_onlinef;
#ifdef	UNIBUS_MAP & MAPPED_BUFFERS
int	*ms_map, ms_offset;	/* UNIBUS MAP register and address */
#endif	UNIBUS_MAP & MAPPED_BUFFERS
long	ms_transf;	/* words transferred (for error rate calculations) */
unsigned	ms_softerrs;


msopen()
{
	register int	n;
	register struct msq	*qp;
	register struct msdesc	*msd;

#ifdef	MALLOC_UMAP
	extern int umap[];
#endif	MALLOC_UMAP

	/*
	 *	if there are no active drives,
	 *		then clear controller
	 *		initialise
	 */

#ifdef	UNIBUS_MAP & MAPPED_BUFFERS
	if( !ms_map)
	{
		if( (n = malloc( umap, 1)) == NULL)
		{
			u.u_error = EIO;
			return;
		}
		ms_map = &(UBMAP->r[ (--n) << 1]);
		*ms_map = ms_q;
		*(ms_map + 1) = 0;
		ms_offset = n;
	}
#endif	UNIBUS_MAP & MAPPED_BUFFERS
	MSSPL();

	if( !ms_onlinef)
	{
		MSADDR->mscsr = RESET;
#ifndef	UNIBUS_MAP & MAPPED_BUFFERS
		MSADDR->msctra = 0;
#endif	UNIBUS_MAP & MAPPED_BUFFERS
		MSADDR->mscsr = CCIE;
		ms_onlinef = 0377;
		for(n=0; n<NDRV; n++)
		{
			/* initialise the relevant memory places */
			msd = &(qp = &ms_q[n])->ms_desc;
			qp->ms_flags = MS_IDLE;
			msd->ds_nexthi=0;
			msd->ds_nextlo=0;
			msd->ds_id=n;
			msd->ds_drive=n;
		}
	}

	spl0();
}

msstrategy(bp)
register struct buf	*bp;
{
	register unsigned	p1, p2;
	register int	f;

	bp->b_error = 0;		/* so that error retrys are correct */
	p1 = bp->b_dev.d_minor & BYTE;

/*
 *	Validate the request
 */

	if(
		(p1 >= TVOL)
	    || ((p2 = &ms_sizes[p1])->nblocks <= bp->b_blkno)
#ifdef	NO_OFFLINEQ | DROP_OFFLINEQ
	    || ( !(ms_onlinef & (1<< (p2->drive))))
#endif
	  )
	{
		bp->b_flags =| B_ERROR;
		iodone(bp);
		return;
	}

#ifdef	_1170
	if(bp->b_flags & B_PHYS)
		mapalloc(bp);
#endif
#	ifdef	VOLUME_STATS

	p2->refcount++;
#	endif
#	ifdef	IO_STATS
	if( bp->b_flags & B_READ )
		msrsec =- bp->b_wcount.hibyte;
	else
		mswsec =- bp->b_wcount.hibyte;
#	endif

	bp->av_forw = 0;
	bp->ms_age = MSAGE;
	bp->ms_cyl = p2->cyloff | ((f = p2->drive & LSD) << SUNIT);

	p1 = bp->b_blkno;
	bp->ms_sector = lrem(p1,MSSEC); /* sector */
	p1 = ldiv(p1,MSSEC); /* trk and cyl */
	bp->ms_track = p1 % MSTRK;
	bp->ms_cyl =+ p1/MSTRK;
	p2 = &ms_q[f];

/*
 *	Now "ms_cyl" contains "drive and cylinder"
 *	"ms_track" contains the head number,
 *	and "ms_sector" contains the sector.
 */

#ifdef	DRIVE_STATS
	p2->ms_cyls[bp->ms_cyl & CYL]++;
	p2->ms_nreq++;
#endif

	MSSPL();
	if ((p1 = p2->ms_bufp) == NULL)
	{
		/* this queue is empty */
		p2->ms_bufp = bp;
		msstart();
	}
	  else
	{
		/*
		 * the queue is not empty, so place in queue so as to
		 * minimise head movement.
		 */

#ifdef	SWEEP
		f = p2->ms_dirf;
#endif

		p2 = p1->av_forw;
		while(p2)
		{
			/* skip any overtaken blocks */
			if( !(p2->ms_age))
				p1 = p2;
			p2 = p2->av_forw;
		}

		for(; p2 = p1->av_forw; p1 = p2)

			if (	p1->ms_cyl <= bp->ms_cyl
			     && bp->ms_cyl <= p2->ms_cyl
			     || p1->ms_cyl >= bp->ms_cyl
			     && bp->ms_cyl >= p2->ms_cyl
			 ){
				while (bp->ms_cyl == p2->ms_cyl)
				{
					/*
					 * for a cylinder match, do the
					 * rotational optimisation.
					 * INTLV should be set to an optimal
					 * value, thought to be around 5
					 * and should NOT be altered if the discs
					 * are reformatted or the free list
					 * reordered...... good luck.
					 */
					if(p2->ms_sector > p1->ms_sector)
					{
						if(bp->ms_sector > p1->ms_sector + INTLV
						&& bp->ms_sector < p2->ms_sector - INTLV)
							goto out;
					}
					  else
						if(bp->ms_sector > p1->ms_sector + INTLV
						|| bp->ms_sector < p2->ms_sector - INTLV)
							goto out;
					p1 = p2;
					if( !(p2 = p1->av_forw)) goto out;
				}
				break;
			}

#ifdef SWEEP
			  else
				if (f == UP)
				{
					if(p2->ms_cyl < p1->ms_cyl)
						if(bp->ms_cyl > p1->ms_cyl)
							goto out;
						else
							f = DOWN;
				}
				  else
				{
					if(p2->ms_cyl > p1->ms_cyl)
						if(bp->ms_cyl < p1->ms_cyl)
							goto out;
						else
							f = UP;
				}
#endif

	out:
		bp->av_forw = p2;
		p1->av_forw = bp;

		while(p2)
		{
			/* count down overtaken blocks */
			if( p2->ms_age)
				p2->ms_age--;
			p2 = p2->av_forw;
		}
	}

	spl0();

}

/*
 *	msstart() goes through all the queues and sets everybody
 *	seeking that should be.
 */
msstart()
{
	/* called at MSSPL or greater */

	register struct buf *bp;
	register struct msq *qp;
	register int	n;
	int	ioflg;

	static int	drv, ioage;

#ifndef	NO_SEEK

	for(n=0; n<NDRV; n++)
		if((bp=(qp= &ms_q[n])->ms_bufp) && (ms_onlinef & (1<<n)))
		{
			/*
			 *	for all active drives
			 */
			if(qp->ms_flags == MS_IDLE)
			{
				/*
				 *	which are seekable, start seeking
				 */

				if(qp->ms_lcyl == bp->ms_cyl)
					qp->ms_flags = MS_READY;
				else
				{
#ifdef	SWEEP
					if(bp->ms_cyl > qp->ms_lcyl)
						qp->ms_dirf = UP;
					else
						qp->ms_dirf = DOWN;
#endif	SWEEP
					mscstart(bp, qp, SEEK);
					qp->ms_flags = MS_SEEK;
				}
			}
		}
#endif

	/*
	 *	start a retry with recalibrate
	 */

	for(n=0; n<NDRV; n++)
		if((bp = (qp = &ms_q[n])->ms_bufp) && (ms_onlinef & (1<<n)))
		{
			/*
			 *	for all active drives
			 */
			if( qp->ms_flags == MS_ERROR)
			{
				/*
				 *	which are recalibratable
				 */

				mscstart(bp, qp, RECAL);
				qp->ms_flags = MS_RECAL;
				qp->ms_lcyl = 0;
			}
		}

	/*
	 * try to start an io
	 */
	ioflg = 0;
	do
	{
		for(n = 0; n < NDRV; n++)
		{
			/*
			 *	for the next io-able drive in a circular
			 *	lookup, start an io if waiting
			 */

			if((bp = (qp = &ms_q[drv])->ms_bufp) &&
#ifndef	NO_SEEK
				(qp->ms_flags  ==  MS_READY) &&
#endif
				(ms_onlinef & (1<<drv)))
			{
				ioflg++;
				if(ioage <= MSAGE)
				{
					if(bp->b_flags & B_READ)
						mscstart(bp, qp, READ);
					  else
						mscstart(bp, qp, WRITE);
					qp->ms_flags = MS_IO;
					ioage++;
					return;
				}
			}
			if( ++drv >= NDRV) drv = 0;
			ioage = 0;
		}
	}
	while( ioflg);		/* Ioage caused us to miss an I/O */

}


msintr()
{
	/* called at MSSPL or greater */

	register struct msdesc	*msd;
	register struct msq	*qp;
	register struct buf	*bp;
	int	n;

	/*
	 *	Here we have three possibilities, these being
	 *		1) Normal command complete
	 *		2) Disc powered up
	 *		3) A command (NXM, PAR, QERR)
	 */

	if(MSADDR->msccra & (CMDQERR | CMDNXM | CMDPAR))
	{
		/* absolute failure, cannot recover */
		printf("MSERR FATAL CCRA ERROR\n");
		MSADDR->mscsr =| CCACK;
		return;
	}
	/*
	 *	now have three further possibilities, namely
	 *		1) drive powered up
	 *		2) error in completed command
	 *		3) command complete
	 */
	if(MSADDR->msccra & DSKPON)
	{
		/* a drive has powered up */
		printf("DRIVE POWERED ON, CCRB=%o\n", MSADDR->msccrb);
		ms_onlinef =| MSADDR->msccrb & BYTE;
		MSADDR->mscsr =| CCACK;
	}
	  else
	{
		/* command complete, with or without errors */
		bp = (qp = &ms_q[n = MSADDR->msccrb & BYTE])->ms_bufp;
		msd = &qp->ms_desc;
		MSADDR->mscsr =| CCACK;
		if(msd->ds_stat & ~(RETRY | ECCERR))
		{
			/* an error!!! */
			if(msd->ds_stat & (PRTCDRV|WRTPRTC|ALTSECT|NXMERR|PARERR))
			{
				/*
				 * operation cannot continue.
				 * NXMERR or PARERR are pretty nasty
				 * but we mustn't hang up driver.
				 */
				printf("MSERR UNRECOVERABLE\n");
/*debugging*/	printf("DS_WC=%d, DS_ML=%d, DS_MH=%d\n",msd->ds_wcount,msd->ds_memlo,msd->ds_memhi);
/*debugging*/	printf("BP_WC=%d, BP_ML=%d, BP_MH=%d\n",bp->b_wcount, bp->b_addr, bp->b_xmem);
				mserr(qp, bp, 1);
				bp->b_flags =| B_ERROR;
				msdone(qp, bp);
			}
			  else
			{
				/* error but can continue */
				mserr(qp, bp, 0);	/* only give error if already tried twice */
				if(msd->ds_stat & (ADDRERR | SEEKERR | DRVNRDY
					| SYSFLT | DRVFALT))
				{
					/* retry and recalibrate */
					qp->ms_flags = MS_ERROR;
				}
				 else
				{
					/* data error, retry by positioning */
					qp->ms_flags = MS_READY;
				}
				if(errcnt(bp))
				{
					msdone(qp, bp);
					if( (msd->ds_stat & DRVNRDY) ||
						(msd->ds_sfc == 1) )
					{
						ms_onlinef =& ~(1<<n);
#ifdef	DROP_OFFLINEQ
						while(bp=qp->ms_bufp)
						{
							bp->b_flags =| B_ERROR;
#ifdef	DRIVE_STATS
							if(qp->ms_nreq-- > qp->ms_rmax)
								qp->ms_rmax = qp->ms_nreq+1;
#endif
							qp->ms_bufp = bp->av_forw;
							iodone(bp);
						}
#endif
					}
				}
			}
		}
		  else
		{
			/* command complete */
			switch (qp->ms_flags)
			{
		    case MS_SEEK:	/* seek complete */
				qp->ms_flags = MS_READY;
				break;
		    case MS_RECAL:	/* error recovery complete */
				qp->ms_flags = MS_IDLE;
				break;
		    case MS_IO:		/* IO complete */
#ifdef	SWEEP
				qp->ms_lcyl = bp->ms_cyl;
#endif
				msdone(qp, bp);
				break;
		    default:		/* can NEVER happen */
				printf("MSERR \"FUCK\"\n");
				mserr(qp, bp, 1);
				break;
			}
		}
	}

	msstart();

}

errcnt(bp)
register struct buf	*bp;
{
	if( ++bp->b_error >= MSERRMAX)
	{
		bp->b_flags =| B_ERROR;
		return(1);
	}
	return(0);
}

msdone(qp, bp)
register struct msq	*qp;
register struct buf	*bp;
{
	qp->ms_flags = MS_IDLE;
	qp->ms_bufp = bp->av_forw;
#ifdef	DRIVE_STATS
	if(qp->ms_nreq-- > qp->ms_rmax)
		qp->ms_rmax = qp->ms_nreq+1;
#endif
	bp->b_resid = -qp->ms_desc.ds_wcount;
	iodone(bp);
}

mserr(qp, bp, flag)
register struct msq	*qp;
struct buf		*bp;
int			flag;
{
#ifdef	RETRIES_ONLY
	if(bp->b_error > 0 /* second try at least */ || flag)
#endif	RETRIES_ONLY
	{
		printf("MS %o,CRA=%o,SR=%o,ST=%o,FC=%o,OP=%o,CYL=%o,HD=%o,SEC=%o,RETRYS=%o\n",
			qp->ms_desc.ds_drive,
			MSADDR->msccra,
			MSADDR->mscsr,
			qp->ms_desc.ds_stat,
			qp->ms_desc.ds_sfc,
			qp->ms_desc.ds_discop,
			qp->ms_desc.ds_cylinder,
			qp->ms_desc.ds_head,
			qp->ms_desc.ds_sector,
			bp->b_error);
	}
	if((++ms_softerrs & 017)  == 0)
	{
		printf("Error rate %dx1,000 words per error\n",
			longdiv(((ms_transf + (1<<7))>>8), (ms_softerrs<<2)));
			/* really times 1024, but this is close enough */
	}
#ifdef LAST_CYL
	{
		int	diff;
		printf("lastcyl=%o, currcyl=%o, distance=",
			qp->ms_lcyl & CYL,
			bp->ms_cyl & CYL);
		if( (diff = (bp->ms_cyl & CYL) - (qp->ms_lcyl & CYL)) < 0)
			printf("-%o\n", -diff);
		else
			printf("+%o\n", diff);
	}
#endif LAST_CYL
}

/*
 *	Physical IO:
 *	truncate transfers at the ends of logical volumes
 */

unsigned	msresidue;

msread(dev)
int	dev;
{
	register unsigned resid;

	if(msphys(dev))
	{
		resid = msresidue;
#ifndef	RAW_BUFFER_POOL
		physio(msstrategy, &msbuf, dev, B_READ);
#else	RAW_BUFFER_POOL
		physio(msstrategy,      0, dev, B_READ);
#endif	RAW_BUFFER_POOL
		u.u_count =+ resid;
	}
}

mswrite(dev)
int	dev;
{
	register unsigned resid;

	if(msphys(dev))
	{
		resid = msresidue;
#ifndef	RAW_BUFFER_POOL
		physio(msstrategy, &msbuf, dev, B_WRITE);
#else	RAW_BUFFER_POOL
		physio(msstrategy,      0, dev, B_WRITE);
#endif	RAW_BUFFER_POOL
		u.u_count =+ resid;
	}
}

msphys(dev)
int	dev;
{
	register unsigned a, b, c;

	b = u.u_offset >> 9;
	if( ((c = dev.d_minor & BYTE) >= TVOL) ||
		((a = ms_sizes[c].nblocks) <= b) )
	{
		u.u_error = ENXIO;
		return(0);
	}
	c = u.u_count;
	if(ldiv(c+511, 512) + b > a)
		c = (a - b) << 9;
	msresidue = u.u_count - c;
	u.u_count = c;
	return(1);
}

#ifdef	MSPOWERFAIL
mspowerf(rdev)
{
	/*
	 *	Msc-Ampex disc power fail recovery routine
	 *
	 *	It is ASSUMED that
	 *
	 *	. a power fail has occured and the cpu did a reset
	 *	. mem management registers have been restored
	 *	. as have unibus map regs as have
	 *	. the other vital ones.
	 */

	register struct buf	*bp;
	register struct msq	*qp;
	register	d;

	long	n;

	MSADDR->hpcs2=CLR;
#ifndef	NO_SEEK
	for(d=0; d< NDRV; d++)
		ms_q[n].ms_flags = MS_IDLE;
#endif
	if(rdev)
	{
		for(d=0; d<NDRV; d++)
			if(ms_onlinef & (1<<d))
			{
				/*
				 *	this drive was on when it died
				 */

				MSADDR->hpcs2 = d;
				for(;;)
				{
					if( (MSADDR->hpcs1 & CRDY) && (MSADDR->hpds & MOL) && (MSADDR->hpds & DRY) )
					{
						msdinit(d);
						break;
					}
					for(n=2000000; n>0; n--);
					printf("MSERR PWR FAIL RECVRY WAITING DRIVE %o\n", d);
				}
			}
	}
	  else
	{
		ms_onlinef = 0;
	}

	msstart();

	printf("MSERR PWR FAIL RECVRD\n");
}

#endif

/*****************************************************************************************/
/*****************************************************************************************/
/*****************************************************************************************/
/*****************************************************************************************/

/*
 *	Start routine for MSC controller
 */

mscstart(bp, qp, com)
register struct buf	*bp;
register struct msq	*qp;
int	com;
{
	register char *msd;

	/*
	 *	ensure that RDYCT is up before doing it
	 */

	while( !(MSADDR->mscsr & RDYCT));
	msd = &qp->ms_desc;
	if(com & (READ | WRITE))
	{
		msd->ds_memlo = bp->b_addr;
		msd->ds_memhi = bp->b_xmem & 03;
		msd->ds_wcount = -bp->b_wcount;
		ms_transf =- bp->b_wcount;
		msd->ds_sector = bp->ms_sector;
		msd->ds_head = bp->ms_track;
	}
	msd->ds_cylinder = bp->ms_cyl & CYL;

	if(com == READ)
		com =| retry[bp->b_error];
	msd->ds_discop = com;

#ifndef	MAPPED_BUFFERS & UNIBUS_MAP
	MSADDR->msctrb = msd;
#else	MAPPED_BUFFERS & UNIBUS_MAP
	MSADDR->msctrb = msd + (ms_offset << 13) - *ms_map;
	MSADDR->msctra = ms_offset >> 3;
#endif	MAPPED_BUFFERS & UNIBUS_MAP


	MSADDR->mscsr = CCIE | CARDY;

}
