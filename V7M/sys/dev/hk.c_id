/*
 * UNIX/v7m RK06/7 disk driver
 * Fred Canter 6/19/81
 *
 * This driver supports, overlapped seek,
 * interleaved file systems, head offset positioning,
 * and full ECC recovery on block and raw I/O transfers.
 *
 * Thanks to Jerry Brenner for 95% of this driver !
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/seg.h"

/*
 * Define REGDUMP to get a dump of all the
 * RK06/7 device registers on an error.
 */
/*#define	REGDUMP	1	*/

#define HKEOFF	      1       /* if 1 then enable offset recovery */

/*
 *
 *	The following  structure defines the order of the Control
 *	and Status registers for the RK06/07
 *
 */
struct device
{
	int	hkcs1;	/* Control and Status register 1 */
	int	hkwc;	/* Word Count Register */
	caddr_t hkba;	/* Memory Address Register */
	int	hkda;	/* Track Sector Address Register */
	int	hkcs2;	/* Control And Status Register 2 */
	int	hkds;	/* Drive Status Register */
	int	hkerr;	/* Error Register */
	union{
		int	w; /* High byte is Attention Summary */
		char c[2]; /* Low is Offset Reg. */
	}hkas;
	int	hkdc;	/* Desired Cylinder Register */
	int	dum;	/* Dummy for unused location */
	int	hkdb;	/* Data Buffer */
	int	hkmr1;	/* Maintenance Register 1 */
	int	hkec1;	/* ECC Position Register */
	int	hkec2;	/* ECC Pattern Register */
	int	hkmr2;	/* Maint. Register 2 */
	int	hkmr3;	/* Maint. Register 3 */
};

#define	HKADDR	((struct device *)0177440)	/* RK6/711 address */
#define NHK	2	/* number of drives on system */
#define NSECT	22	/* number of sectors per pack */
#define NTRAC	3	/* number of tracks per cylinder */
#define NUMTRY 28	/* max retry count */

/*
 *
 *	The structure hk_sizes defines the logical separation of a disk pack
 *	into mountable file systems.
 *
 *	The file systems are named in numeric order.
 *
 *		the first entry in hk_sizes describes file system hk0
 *		the next hk1  , etc.
 *
 *	The nblocks entry specifies the number of blocks (512 bytes) in
 *	the file system.
 *
 *	The cyloff entry specifies the offset+1 from the physical beginning
 *	of the pack to the beginning of the file system in cylinders
 *
 */
struct	size
{
	daddr_t	nblocks;
	int	cyloff;
} hk_sizes[8] =
{
	9636,	0,	/* cyl   0-145, root on rk06/7		*/
	8910,	146,	/* cyl 146-280, swap on rk06/7		*/
	8514,	281,	/* cyl 281-409, rest of rk06 pack	*/
	26598,	411,	/* cyl 411-813, rest of rk07 pack	*/
	0,	0,	/* spare, for avoiding bad blocks	*/
	0,	0,	/* spare, for avoiding bad blocks	*/
	27060,	0,	/* cyl   0-409, all of rk06 pack	*/
	53724,	0,	/* cyl   0-813, all of rk07 back	*/
};

/*
 *
 * The following definitions specify the offset values
 * used during error recovery
 *
 */
#ifdef HKEOFF
#define P25	01	/* +25 Rk06, +12.5 Rk07 */
#define M25	0201	/* -25 RK06, -12.5 RK07 */
#define	P200	010	/* +200 RK06, +100 RK07 */
#define M200	0210	/* -200, RK06, -100 RK07 */
#define P400	020	/* +400  RK06 , +200  RK07 */
#define M400	0220	/* -400  RK06 , -200  RK07 */
#define P800	040	/* +800  RK06 , +400 RK07 */
#define M800	0240	/* -800  RK06 , -400  RK07 */
#define P1200	060	/* +1200  RK06 , +600  RK07 */
#define M1200	0260	/* -1200  RK06 , -600 Rk07 */
/*
 *
 *	The following array defines the order in which the offset values defined above
 *	are used during error recovery.
 *
 */
int	hk_offset[16] =
{
	P25, M25, P200, M200,
	P400, M400, P400, M400,
	P800, M800, P800, M800,
	P1200, M1200, P1200, M1200,
};
#endif

/*
*	Control and Status bit definitions for  hkcs1
*/
#define GO	01	/* GO bit */
#define SELECT	01	/* Select Function */
#define PAKACK	02	/* Pack Acknowledge Function */
#define DCLR	04	/* Drive Clear Function */
#define UNLOAD	06	/* Unload Heads Function */
#define STRSPN	010	/* Start Spindle Function */
#define RECAL	012	/* Recalibrate Function */
#define OFFSET	014	/* Offset Function */
#define SEEK	016	/* Seek Function */
#define RCOM	020	/* Read Command */
#define WCOM	022	/* Write Command */
#define RHDR	024	/* Read Header	*/
#define WHDR	026	/* Write Header */
#define WCHK	030	/* Write Check Function */
#define IEI	0100	/* Interrupt Inable bit */
#define CRDY	0200	/* Controller Ready bit */
#define SEL7	02000	/* Select RK07 bit */
#define CTO	04000	/* Controller Time Out bit */
#define CFMT	010000	/* Controller Format bit */
#define DTCPAR	020000	/* Drive to Controller Parity Error */
#define DINTR	040000	/* Drive Interrupt bit */
#define CCLR	0100000 /* Controller Clear bit */
#define CERR	0100000 /* Controller Error bit */

/*
*	Control and Status bit definitions for  hkcs2
*/
#define DLT	0100000 /* Data Late error */
#define WCE	040000	/* Write Check Error */
#define UPE	020000	/* Unibus Parity Error */
#define NED	010000	/* Nonexistent Drive error */
#define NEM	04000	/* Nonexistent Memory error */
#define PGE	02000	/* Programming error */
#define MDS	01000	/* Multiple Drive Select error */
#define UFE	0400	/* Unit Field Error */
#define SCLR	040	/* Subsystem Clear bit */
#define BAI	020	/* Bus Address Increment Inhibit bit */
#define RLS	010	/* Release bit	*/

/*
*	Control and Status bit definitions for  hkds
*/
#define SVAL	0100000 /* Status Valid bit */
#define CDA	040000	/* Current Drive Attention bit */
#define PIP	020000	/* Position In Progress bit */
#define WRL	04000	/* Write Lock bit */
#define DDT	0400	/* Disk Drive Type bit */
#define DRDY	0200	/* Drive Ready bit */
#define VV	0100	/* Volume Valid bit */
#define DROT	040	/* Drive Off Track Error */
#define SPLS	020	/* Speed Loss Error	*/
#define ACLO	010	/* Drive AC Low */
#define DOFST	04	/* Drive Offset bit */
#define DRA	01	/* Drive Available bit */

/*
*	Control and Status bit definitions for  hkerr
*/
#define DCK	0100000 /* Data Check error */
#define DUNS	040000	/* Drive Unsafe error */
#define OPI	020000	/* Operation Incomplete error */
#define DTE	010000	/* Drive Timing error */
#define DWLE	04000	/* Drive Write Lock error */
#define IDAE	02000	/* Invalid Disk Address Error */
#define COE	01000	/* Cylinder Overflow Error	*/
#define HRVC	0400	/* Header Vertical Redundance Check Error */
#define BSE	0200	/* Bad Sector Error */
#define ECH	0100	/* Error Correction Hard error */
#define DTYE	040	/* Drive Type error */
#define FMTE	020	/* Format Error */
#define CDPAR	010	/* Controller To Driver Parity Error */
#define NXF	04	/* Nonexecutable Function Error */
#define SKI	02	/* Seek Incomplete error */
#define ILF	01	/* Illegal Function Error */

struct
{
	int  drvtyp;	/* This contains the drive type */
	char mntflg;	/* This contains a mounted flag it is set to indicate*/
			/* that a drive type is known to the system */
	char recal;	/* recalibrate flag */
	int  ccyl;	/* stores drives current cylinder  */
	int  cs1;	/* Error reporting save locations */
	int  wc;
	int  ba;
	int  da;
	int  cs2;
	int  ds;
	int  err;
	int  as;
	int  dc;
	int  mr1;
	int  mr2;
	int  mr3;
}hk_r[NHK];

struct buf	hktab;	/* This is the buffer header used exclusivly by the RK06/07 for block I/O */
			/* the structure is the same as buf.h but useage is slightly different */

struct buf	rhkbuf; /* This buffer header is used by the RK06/07 for raw I/O. struct. same as hktab */

struct buf	hkutab[NHK]; /* Unit headers for overlapped seek driver */

#define trksec	av_back
#define cylin	b_resid
/*
 * Instrumentation (iostat) structures
 */

struct	ios	hk_ios[NHK];
#define DK_N	2
#define	DK_T	3	/* RK06/7 transfer rate indicator */

#define b_cylin b_resid

daddr_t dkblock();

/********   flag field defines for hk tables   **********/
#define HKBUSY	01	/* drive or controller is busy */
#define HKSEEK	02	/* drive is seeking */
#define HKOFFS	04	/* drive in offset mode */




/*
*
*	The following routine determines the drive type of the selected unit,
*	saves drive type in hk_r[].drvtyp,
*	and sets flag in hk_r[].mntflg
*
*/
hkmnt(dn)
{
	register int pri;

		hk_r[dn].drvtyp = 0;	/* set default drive type to RK06 */
		hk_ios[dn].dk_tr = DK_T;   /* (iostat) xfer rate indicator */
/*
 * Tell the instrumentation code in
 * the clock routine, where the iostat
 * structure for this controller is and
 * how many drives are on this controller.
 * Clock interrupts are locked out while
 * iostat pointers and counts are being set up !
 */
		pri = spl6();
		dk_iop[DK_N] = &hk_ios[0];
		dk_nd[DK_N] = NHK;
		splx(pri);
		hkcmd(dn, GO);	   /* select drive */
		if(HKADDR->hkcs1 & CERR && HKADDR->hkerr & DTYE)
		{
			hk_r[dn].drvtyp = SEL7; /* Drive type is RK07 */
			hkcmd(dn, DCLR|GO);	   /* clear error bits */
			hkcmd(dn, GO);	   /* get status */
		}
		if((HKADDR->hkcs1 & CERR) == 0)
			hk_r[dn].mntflg = 1;	/* set mnt flag and return */
}

/*
*
*	hkstrategy checks for overflow errors
*
*/
hkstrategy(bp)
register struct buf *bp;
{
	register struct buf *dp;
	register int unit;
	long sz, bn;
	int dn, dt;

	if(bp->b_flags & B_PHYS)
		mapalloc(bp);	/* phys I/O. see about allocating ubmap */

	unit = minor(bp->b_dev) & 077;	/* unit number from buffer header */
	dn = (unit >> 3) & 07;		/* get drive number	*/
	sz = bp->b_bcount;		/* size from buffer header */
	sz = (sz+511)>>9;		/* convert to number of blocks */

	/* if unit requested is greater than number available 
	* or
	* block number is < 0
	* or
	* block number +size is >= number of blocks in file system
	* or
	* mount flag is less than 0. Drive not available or no drive
	*
	* ERROR
	*
	* dkblock is an interleaved file system routine that returns
	* the actual block in the filesystem
	*/
	if(unit >= (NHK<<3)
		|| bp->b_blkno < 0
		|| (bn = dkblock(bp))+sz >= hk_sizes[unit&007].nblocks
		|| hk_r[dn].mntflg < 0)
	{
		u.u_error |= ENXIO;		/* zap the user process */
		bp->b_flags |= B_ERROR;		/* set ERROR flag */
		iodone(bp);	/* call iodone to return buffer to pool */
		return; 	/* bye bye */
	}

/*
*	no error so continue 
*/
	bp->b_cylin = bn/(NSECT*NTRAC) + hk_sizes[unit&07].cyloff;
	unit = dkunit(bp);	/* get the actual drive number
				   could be other than specified
				   with an interleaved file system */
	dp = &hkutab[unit];	/* get a pointer to the drives table */
	spl5();
	disksort(dp, bp);	/* sort the drives queue in ascending
				   positional order and load this
				   buffer on the drives queue	*/

/*	If drive is not active and no I/O pending
*	then start it up.
*/
	if(dp->b_active == 0 && hktab.b_active == 0)
	{
		hkustart(unit); 	/* start a drive seeking if needed */
		hkstart();		/* start I/O	*/
	}
	spl0();
}

hkustart(unit)
register unit;
{
	register struct buf *bp, *dp;
	daddr_t bn;
	int sn, tn, cn, dn;
	long	nblk;

	dn = unit;			/* get drive number	*/
	hk_ios[dn].dk_busy = 0;
	dp = &hkutab[dn];		/* grab pointer to unit table	*/
	if(( bp = dp->b_actf) == NULL)	/* nothing pending here so return */
		return;
	if(hk_r[dn].mntflg < 0)
	{	/* drive unloaded or not mountable */
		bp->b_flags |= B_ERROR; /* set error */
		dp->b_actf = bp->av_forw; /* get next buffer */
		iodone(bp);		/* return buffer */
		return; 		/* bye bye */
	}
	if(hk_r[dn].mntflg == 0)
	{
		hkmnt(dn);		/* make sure we know drive type */
		if(HKADDR->hkcs1 & CERR || (HKADDR->hkds & DRDY)== 0)
		{	/* error from mount. drive not available ? */
			hkerror(bp, dn, 0);	  /* count = 1 to save error */
			bp->b_flags |= B_ERROR; /* set error */
			hkerror(bp, dn, 1);	  /* fatal, log it */
			hkcmd(dn, IEI);	   /* clear controller error */
			hk_r[dn].mntflg = -1;	/* say bad news */
			hk_r[dn].ccyl = 0;	/* reset these */
			dp->b_actf = bp->av_forw; /* get next buffer */
			iodone(bp);		/* return buffer */
			return; 		/* bye bye */
		}
		hkcmd(dn, (PAKACK|GO));	       /* set volume valid */
		hk_r[dn].ccyl = 0;		/* set current cyl = 0 */
	}
	if(dp->b_active & HKSEEK || hktab.b_actf == NULL)
		goto done;
	bn = dkblock(bp);		/*get block number     */
	cn = bp->b_cylin;		/* get cylinder number	*/
	sn = bn%(NSECT*NTRAC);		/* calculate the */
	tn = sn/NSECT;			/* track number */
	sn = sn%NSECT;			/* get the sector number */

	if(hk_r[dn].ccyl != cn && hktab.b_actf != NULL)
	{		/* not on cylinder so seek */

		dp->b_active = HKSEEK|HKBUSY;	/* say drive is seeking */
		HKADDR->hkdc = cn;		/* desired cylinder */
		HKADDR->hkda = (tn <<8)| sn;	/* so no illegal addr */
		HKADDR->hkcs2 = dn;		/* select drive */
		hk_ios[dn].dk_busy++;		/* drive is active (seeking) */
		HKADDR->hkcs1 = (hk_r[dn].drvtyp|IEI|SEEK|GO);	/* do it */
		return;
	}
done:
	dp->b_active = HKBUSY;		/* say drive is active	*/
	dp->b_forw = NULL;		/* clear this forward link */
	if(hktab.b_actf == NULL)	/* if I/O table is 0 then */
		hktab.b_actf = dp;    /* load to I/O table (first in queue)*/
	else
		hktab.b_actl->b_forw = dp;	/* link to end of queue */
	hktab.b_actl = dp;			/* new end of queue */
}

hkstart()
{
	register struct buf *bp, *dp;
	register int unit;
	int com,cn,tn,sn,dn;
	daddr_t bn;

loop:
	if ((dp = hktab.b_actf) == NULL)	/* get a Unit table ptr */
	{
		HKADDR->hkcs1 = IEI;		/* nothing queued so make */
		return; 			/* sure IEI is set and bye */
	}
	if(dp->b_active & HKSEEK)
	{
		HKADDR->hkcs1 = IEI;
		return; 			/* drive is seeking. later */
	}
	if ((bp = dp->b_actf) == NULL)
	{					/* no buffer for this drive */
		hktab.b_actf = dp->b_forw;	/* get next drive from queue*/
		goto loop;			/* look some more */
	}
	hktab.b_active |= HKBUSY;	/* say that we're active */
	unit = minor(bp->b_dev)&077;	/* get the unit and file sys number */
	dn = dkunit(bp);		/* strip file sys #, get drive # */
	bn = dkblock(bp);		/* get the block number */
	cn =(bn/(NSECT*NTRAC))+hk_sizes[unit&07].cyloff;/* calc cylinder # */
	sn = bn%(NSECT*NTRAC);
	tn = sn/NSECT;			/* calculate track number */
	sn = sn%NSECT;			/* get the sector number */

#if HKEOFF
		/* try offset recovery. Offset only good on read */
	if((hktab.b_active & HKOFFS) == 0
		&& hktab.b_errcnt >= 16
		&& bp->b_flags & B_READ)
	{
		hktab.b_active = HKOFFS;
		hkcmd(dn, GO);
		HKADDR->hkas.w = hk_offset[hktab.b_errcnt &017];
		HKADDR->hkcs2 = dn;
		HKADDR->hkcs1 = (hk_r[dn].drvtyp|IEI|OFFSET|GO);
		return;
	}
	hktab.b_active &= ~HKOFFS;	/* clear offset flag */
#endif
	HKADDR->hkdc = cn;		/* load the desired cylinder */
	HKADDR->hkda = (tn<<8)|sn;	/* and the track , sector */
	HKADDR->hkba = bp->b_un.b_addr; /* get memory address */
	HKADDR->hkwc = -(bp->b_bcount>>1);/* comp and load the word count */
	com = ((bp->b_xmem&3)<<8)|IEI|GO; /* ext mem, IEI, and go bits */
	if(bp->b_flags & B_READ)
		com |= RCOM;		/* Read function */
	else
		com |= WCOM;		/* write function */
	com |= hk_r[dn].drvtyp; 	/* set drive type */
	HKADDR->hkcs2 = dn;		/* select drive number */
	HKADDR->hkcs1 = com;		/* and load it all into hkcs1 */
	hk_ios[dn].dk_busy++;		/* Instrumentation - disk active, */
	hk_ios[dn].dk_numb++;		/* count number of transfers, */
	unit = bp->b_bcount >> 6;	/* transfer size in 32 word chunks */
	hk_ios[dn].dk_wds += unit;	/* count total words transferred */
}

hkintr()
{
	register struct buf *bp, *dp;
	register int unit, ctr, dn;
	int seg, uba;
	int tpuis[2], tpbad[2], tppos, tpofst, bsp, pri;

	if((HKADDR->hkcs1 & CRDY) == 0) /* controller not ready. I/O active */
		return; 		/* wait for next interrupt */

	if(hktab.b_active)
	{
		dp = hktab.b_actf;	/* Doing I/O. get drive pointer */
		bp = dp->b_actf;	/* get buffer pointer */
		dn = dkunit(bp);	/* get drive number */
		if(hk_r[dn].recal || hktab.b_active & HKOFFS)
		{	/* recal in progress or offset active. see if done */
			hkcmd(dn, GO);   /* select the drive */
			if((HKADDR->hkds & DRDY)==0)
			{
				HKADDR->hkcs1 = IEI;
				return; 	/* recal still in prog */
			}
			hk_r[dn].recal = 0;	/* reset flag and */
			hktab.b_active &= ~HKBUSY;  /* retry I/O function */
			hkcmd(dn, GO);   /* select the drive */
		}
		if(HKADDR->hkcs1 & CERR)
		{
			if(hktab.b_errcnt == 0)
				hkerror(bp, dn, hktab.b_errcnt);

			/* if exceed retry count or Non-existent mem or */
			/* drive write lock error or cylinder overflow	*/
			/* or format error or bad sector error */
			/* or error during offset */
			/* then FATAL error */

			if(hktab.b_errcnt++ > NUMTRY
			   || HKADDR->hkcs2 & NEM
			   || HKADDR->hkerr & (DWLE|COE|FMTE|BSE)
			   || hktab.b_active & HKOFFS)
			{
				hktab.b_active |= HKBUSY; /* make sure set */
				bp->b_flags |= B_ERROR; /* Fatal error */
			}
			else if(HKADDR->hkds & DROT
				|| HKADDR->hkerr & (OPI|SKI))
			{				/* needs a recal   */
				hk_r[dn].recal++;	/* set recal flag */
				hkcmd(dn, (DCLR|GO));   /* clr drive */
				hkcmd(dn, (IEI|RECAL|GO));  /* issue recal */
				return; 		/* later */
			}
	

			else if((HKADDR->hkerr & (DCK|ECH))== DCK)
			{
				/* ECC error . recoverable */
			    pri = spl7();	  /* need lots of priority */
			    hkerror(bp, dn, 0);  /* make sure right registers */
			    tpuis[0] = UISA->r[0];  /* save User I PAR */
			    tpuis[1] = UISD->r[0];  /* save User I PDR */
			    uba = (HKADDR->hkcs1&01400)<<2; /* exmem */
			    uba += (((HKADDR->hkba>>1)&~0100000)>>5);
			    uba -= 8;
			    if(bp->b_flags&B_MAP)
			    {
				ctr = (uba >> 6) & 076;
				seg = UBMAP->r[ctr+1] << 10;
				seg |= (UBMAP->r[ctr] >> 6) & 01777;
				UISA->r[0] = seg + (uba & 0177);
			    } else
				UISA->r[0] = uba;
			    UISD->r[0] = 077406;	  /* Set User I PDR */
			    tppos = (HKADDR->hkec1-1)/16;   /* word addr */
			    tpofst = (HKADDR->hkec1-1)%16;  /* bit in word */
			    bsp = HKADDR->hkba & 077;	    /* 64 byte range*/
			    bsp += (tppos*2);
			    tpbad[0] = fuiword(bsp);	/* first bad word */
			    tpbad[1] = fuiword(bsp+2);	/* second bad word */
			    tpbad[0] ^= (HKADDR->hkec2<<tpofst); /* xor first word */

			    if(tpofst > 5)
			    {			/* must fix second word */
				    ctr = (HKADDR->hkec2 >> 1) & ~0100000;
				    if(tpofst < 15)
					    ctr = ctr >> (15 - tpofst);
				    tpbad[1] =^ ctr;	/* xor second word */
			    }
			    suiword(bsp,tpbad[0]);	/* set first word */
			    suiword((bsp+2),tpbad[1]);	/* set second word */
			    UISD->r[0] = tpuis[1];  /* restore User PDR */
			    UISA->r[0] = tpuis[0];  /* restore User PAR */
			    splx(pri);			/* back to old pri */
			    printf("%D ",bp->b_blkno+(HKADDR->hkwc -(-(bp->b_bcount>>1)))/256);
    			    prdev("ECC", bp->b_dev);
			    hktab.b_errcnt = 0;

			    /* now see if transfer finished. */
			    /* If not then reload xfer making sure */
			    /* the CCLR bit does not get set too */

			    if(HKADDR->hkwc != 0)
			    {
					HKADDR->hkcs1 = CCLR;
					while((HKADDR->hkcs1 & CRDY) == 0);
					HKADDR->hkwc = hk_r[dn].wc;
					HKADDR->hkba = hk_r[dn].ba;
					HKADDR->hkda = hk_r[dn].da;
					HKADDR->hkdc = hk_r[dn].dc;
					HKADDR->hkcs2 = dn;
				    HKADDR->hkcs1=(hk_r[dn].cs1&03576)|IEI|GO;
				    return;
			    }
			}
	
			else if(HKADDR->hkerr & ECH)
			{				/* soft error. retry */
				hktab.b_active &= ~HKBUSY;  /* retry I/O function */
				hkcmd(dn , (DCLR, GO)); /* clear drive's attn */
			}

			else if(HKADDR->hkcs2 & (UPE|DLT)
				|| HKADDR->hkerr & (HRVC|DTE)
				|| hkcomer(dn, bp))
			{				/* soft error. retry */
				hkcmd(dn , (DCLR, GO)); /* clear drive's attn */
				hktab.b_active &= ~HKBUSY;  /* retry I/O function */
			}

			else
			{
				/* must be fatal error */
				hktab.b_active |= HKBUSY;
				bp->b_flags |= B_ERROR;
			}
		}
		if(hktab.b_active & HKBUSY) /* need to return a buffer */
		{
			if(hktab.b_errcnt || bp->b_flags & B_ERROR)
				hkerror(bp, dn, hktab.b_errcnt);
			hktab.b_active = 0;		/* no I/O active */
			dp->b_active = 0;		/* no drive active */
			hktab.b_errcnt = 0;		/* clr I/O error count */
			dp->b_errcnt = 0;		/* clr drive error count */
			hktab.b_actf = dp->b_forw;	/* get next drive link */
			dp->b_actf = bp->av_forw;	/* get next I/O link */
			if(bp->b_flags & B_ERROR)
			    bp->b_resid = bp->b_bcount; /* nothing xfered */
			else
				bp->b_resid = 0;	/* xfer complete */
			iodone(bp);			/* return buffer */
#if HKEOFF
			if(hktab.b_errcnt >16 && bp->b_flags & B_READ)
			{
				hkcmd(dn, (DCLR|GO));	/* make sure attn is off */
				hkcmd(dn, OFFSET|GO);	/* return to zero */
			}
#endif
			hk_ios[dn].dk_busy = 0;	/* drive no longer active */
		}
		hkcmd(dn, (DCLR|GO));	/* make sure attn is off */
#if HKEOFF
		if(hktab.b_active & HKOFFS)
		{
			hkstart();
			return;
		}
#endif
	}
	hkattn();	/* go check attention lines */
}			/* and start any pending I/O */
	
hkattn()
{
	register struct buf *bp, *dp;
	register int dn, as;

	as = HKADDR->hkas.c[1]; 	/* get attn summary reg */
	for( dn = 0; dn < NHK; dn++)
	{
		if((as & (1 << dn)) == 0)
			continue;	/* find attn bit(s) */
		if(hk_r[dn].mntflg <= 0)
		{
			hkmnt(dn);
			if(hk_r[dn].mntflg > 0)
				hk_r[dn].mntflg = 0;
			hkcmd(dn, DCLR|GO);
			continue;
		}
		dp = &hkutab[dn];	/* get utable pointer */
		bp = dp->b_actf;	/* get active buffer pointer */
		hkcmd(dn , GO);	   /* select drive */
		while(HKADDR->hkds & PIP)
			hkcmd(dn, GO);
		if(HKADDR->hkcs1 & CERR)
		{
			if(dp->b_errcnt == 0)
				hkerror(bp, dn, dp->b_errcnt);

			if(dp->b_errcnt++ > NUMTRY
			     || (hkcomer(dn, bp)) == 0)
			{
				hkerror(bp, dn, dp->b_errcnt);
				if(dp->b_active){
					dp->b_actf = bp->av_forw;/* next link*/
					bp->b_flags |= B_ERROR; /* say bad */
					dp->b_errcnt = 0;/* not recoverable */
					iodone(bp);	      /* return buf */
				}
			}
			dp->b_active = 0;	/* drive not active */
			hk_r[dn].ccyl = 0;
		}

		if((HKADDR->hkds & DRDY) == 0)
		{	/* drive dropped off line */
			hk_r[dn].mntflg = -1;	/* not mounted */
			if(dp->b_active)
			{	/* something active */
				dp->b_active = 0;	/* say no more */
				bp->b_flags |= B_ERROR; /* error */
				dp->b_actf = bp->av_forw; /* get next buffer */
				iodone(bp);		/* return error */
			}
		}
		dp->b_active &= ~HKBUSY;	   /* make sure clear */
		hkcmd(dn, (DCLR|GO));	   /* clear drive attn */
	}
	for(dn = 0; dn < NHK; dn++)
	{
		dp = &hkutab[dn];	/* get utable pointer */
		if((dp->b_active & HKBUSY) == 0 && dp->b_actf)
			hkustart(dn);	/* if drive not busy and request
					   pending then start the drive */
	}
	if((hktab.b_active & HKBUSY) == 0)
		hkstart();		/* If no I/O active then start it */
}

hkcmd(dn, comm)
{
	int pri;


	while((HKADDR->hkcs1 & CRDY) == 0);	/* wait for ready */
	pri = spl5();				/* no interrupts */
	HKADDR->hkcs1 |= CCLR;			/* clear controller */
	while((HKADDR->hkcs1 & CRDY) == 0);	/* wait for contr ready */
	HKADDR->hkcs2 = dn;			/* load drive # */
	HKADDR->hkcs1 = (hk_r[dn].drvtyp|comm); /* load command */
	if((comm & IEI) == 0)
		while((HKADDR->hkcs1 & CRDY) == 0); /* need to wait here */
	splx(pri);
}

hkcomer(dn, bp)
struct buf *bp;
int dn;
{
	/** some errors are common to seek as well as i/o **/
	/** this routine checks for them.		  **/

	register struct buf *dp;
	register int soft, cnt;

	/* Check for Controller Type Errors first */

	if(HKADDR->hkcs1 & (CTO|DTCPAR) 	/* Controller timeout */
		|| HKADDR->hkcs2 & (UFE|PGE)	/* Drive to Cont. Parity */
		|| HKADDR->hkerr & CDPAR)	/* Cont. to Drive Parity */
						/* Unit Field Error */
						/* Programming Error */
		return(1);			/* are all soft errors */

	else if(HKADDR->hkcs2 & NED)		/* Non-Exist Drive */
		return(0);			/* return non-recoverable */

	else if(HKADDR->hkcs2 & MDS)		/* Multiple Drive Select */
	{					/* Have to clear the entire */
		HKADDR->hkcs2 = SCLR;		/* disk system to recover */
		while((HKADDR->hkcs1 & CRDY) == 0)
			;			/* wait for controller rdy */
		hktab.b_active = 0;		/* no I/O active */
		for(cnt = 0; cnt < NHK; cnt++)
		{
			dp = &hkutab[cnt];	/* no drive active */
			dp->b_active = 0;
		}
		return(0);			/* return Hard Error */
	}

	/* Now we can check for drive errors */

	else
		hkcmd(dn, GO);		   /* select drive */

	if(HKADDR->hkds & (ACLO|SPLS)		/* ACLO or Speed Loss */
	  || HKADDR->hkerr & (ILF|NXF|IDAE|DUNS)) /* Illegal Function */
						/* Nonexecutable Function */
						/* Invalid Disk Address */
		return(0);			/* Drive Unsafe are fatal */
	else
		return(1);		/* anything else is soft error?? */
}

hkerror(bp, dn, count)
struct buf *bp;
int dn, count;
{

	if(count == 0)
	{	/* first time through */
		/* save some registers */
		hk_r[dn].cs1 = HKADDR->hkcs1;
		hk_r[dn].wc = HKADDR->hkwc;
		hk_r[dn].ba = HKADDR->hkba;
		hk_r[dn].da = HKADDR->hkda;
		hk_r[dn].cs2 = HKADDR->hkcs2;
		hk_r[dn].ds = HKADDR->hkds;
		hk_r[dn].err = HKADDR->hkerr;
		hk_r[dn].as = HKADDR->hkas.w;
		hk_r[dn].dc = HKADDR->hkdc;
		hk_r[dn].mr1 = HKADDR->hkmr1;
		hk_r[dn].mr2 = HKADDR->hkmr2;
		hk_r[dn].mr3 = HKADDR->hkmr3;
	}
	else
#ifndef REGDUMP
		deverror(bp, hk_r[dn].cs2, hk_r[dn].err);
#else
	{
		if(bp->b_flags & B_ERROR || count >= NUMTRY)
			printf("\nfatal ");
		else
			printf("\nrecoverable ");
		if(bp)
			prdev("error", bp->b_dev);
		printf("Error Count = %d\n", count);
		printf("cs1 = %o\t   wc = %o\t	 ba = %o\n",hk_r[dn].cs1,hk_r[dn].wc,hk_r[dn].ba);
		printf(" da = %o\t  cs2 = %o\t	 ds = %o\n",hk_r[dn].da,hk_r[dn].cs2,hk_r[dn].ds);
		printf("err = %o\t  off = %o\t	 dc = %o\n",hk_r[dn].err,hk_r[dn].as,hk_r[dn].dc);
		printf("mr1 = %o\t  mr2 = %o\t	mr3 = %o\n",hk_r[dn].mr1,hk_r[dn].mr2,hk_r[dn].mr3);
	}
#endif
}

hkread(dev)
dev_t dev;
{
	physio(hkstrategy, &rhkbuf, dev, B_READ);
}

hkwrite(dev)
dev_t dev;
{
	physio(hkstrategy, &rhkbuf, dev, B_WRITE);
}
