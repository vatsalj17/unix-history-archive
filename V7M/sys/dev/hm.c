/*
 * UNIX/v7m - RM02/3, RP04/5/6, & ML11 disk driver.
 * Disks on second RH controller (176300)
 * Fred Canter 9/4/81
 *
 * This is a combination driver, it supports any mix
 * of RM02/3, RP04/5/6, & ML11 disks, up to eight
 * drives on the same controller.
 * If support for the ML11 drive type is not
 * required, it may be removed by redefining the control
 * parameter for the ML11 (ML) to zero.
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/seg.h"

/*
 * The following parameter controls the inclusion
 * of code to support the ML11 disk drive.
 */

#define	ML	1	/* ML11 */

struct	device
{
	union {
		int	w;
		char	c[2];
	} hmcs1;		/* Control and Status register 1 */
	int	hmwc;		/* Word count register */
	caddr_t	hmba;		/* UNIBUS address register */
	int	hmda;		/* Desired address register */
	union {
		int	w;
		char	c[2];
	} hmcs2;		/* Control and Status register 2 */
	int	hmds;		/* Drive Status */
	int	hmer1;		/* Error register 1 */
/*		mler	*/	/* Error register */
	int	hmas;		/* Attention Summary */
	int	hmla;		/* Look ahead */
/*		mlpa	*/	/* Prom address register */
	int	hmdb;		/* Data buffer */
	int	hmmr;		/* Maintenance register */
/*		rmmr1	*/	/* Maintenance register 1 */
	int	hmdt;		/* Drive type */
	int	hmsn;		/* Serial number */
	int	hmof;		/* Offset register */
/*		mle1	*/	/* ECC CRC word 1 */
	int	hmdc;		/* Desired Cylinder address register */
/*		mle2	*/	/* ECC CRC word 2 */
	int	hmcc;		/* Current Cylinder */
/*		mld1	*/	/* Data diagnostic 1 register */
/*		rmhr	*/	/* Holding register */
	int	hmer2;		/* Error register 2 */
/*		mld2	*/	/* Data diagnostic 2 register */
/*		rmmr2	*/	/* Maintenance register 2 */
	int	hmer3;		/* Error register 3 */
/*		mlee	*/	/* ECC error register */
/*		rmer2	*/	/* Error register 2 */
	int	hmec1;		/* Burst error bit position */
/*		mlel	*/	/* ECC error location register */
	int	hmec2;		/* Burst error bit pattern */
/*		mlpd	*/	/* Prom data register */
	int	hmbae;		/* 11/70 bus extension */
	int	hmcs3;		/* 11/70 control & status 3 */
};

#define	HMADDR	((struct device *)0176300)
#define	NUNIT	2
/*
 * Instrumentation (iostat) structures
 */
struct	ios	hm_ios[NUNIT];
#define	DK_N	1

/*
 * Drive type definitions
 */
#define	RP04	020
#define	RP05	021
#define	RP06	022
#define	RM03	024
#define	RM02	025
#define	ML11	0110
/*
 * Disk geometry definitions
 */
#define NRMSEC	32
#define NRMTRK	5
#define NRPSEC	22
#define NRPTRK	19
#define	SDIST	2
#define	RDIST	6

/*
 * The number of elements in the hm_sizes[]
 * array has been changed from 8 to 24.
 * The first 8 elements describe the disk partitions
 * for the RP04/5/6 and the second 8 elements
 * specify the RM02/3 disk partitions,
 * the last 8 elements are the size of each ML11
 * unit in blocks.
 * The size of the ML11 unit is determined by reading
 * the ML maintenance register the first time the drive
 * is accessed.
 * The last cylinder of the disk pack is not used, 
 * so that a bad block replacement strategy may be
 * implemented later.
 */

struct	size
{
	daddr_t	nblocks;
	int	cyloff;
} hm_sizes[24] =
{
	9614,	0,	/* cyl   0- 22, root file system on rp04/5/6	*/
	8778,	23,	/* cyl  23- 43, swap area on rp04/5/6		*/
	8778,	44,	/* cyl  44- 64, /sys (system sources) rp04/5/6	*/
	144210,	65,	/* cyl  65-409, /usr (user + sources) rp04/5	*/
	313082,	65,	/* cyl  65-813, /usr (user + sources) rp06	*/
	168454,	411,	/* cyl 411-813, user file system on rp06	*/
	171380,	0,	/* cyl   0-409, user - all of rp04/5 pack	*/
	340252,	0,	/* cyl   0-813, user - all of rp06 pack		*/
	9600,	0,	/* cyl   0- 59, root file system on rm02/3	*/
	8800,	60,	/* cyl  60-114, swap area on rm02/3		*/
	8000,	115,	/* cyl 115-164, /sys (system sources) rm02/3	*/
	105120,	165,	/* cyl 165-821, /usr (user + sources) rm02/3	*/
	0,	0,	/* spare for avoiding bad blocks		*/
	0,	0,	/* spare for avoiding bad blocks		*/
	0,	0,	/* spare for avoiding bad blocks		*/
	131520,	0,	/* cyl   0-821, user - all of rm02/3 pack	*/
	0,	0,	/* ML11  unit	0	*/
	0,	0,	/*		1	*/
	0,	0,	/*		2	*/
	0,	0,	/*		3	*/
	0,	0,	/*		4	*/
	0,	0,	/*		5	*/
	0,	0,	/*		6	*/
	0,	0,	/*		7	*/
};

#define	P400	020
#define	M400	0220
#define	P800	040
#define	M800	0240
#define	P1200	060
#define	M1200	0260
int	hm_offset[16] =
{
	P400, M400, P400, M400,
	P800, M800, P800, M800,
	P1200, M1200, P1200, M1200,
	0, 0, 0, 0,
};

struct	buf	hmtab;
struct	buf	rhmbuf;
struct	buf	hmutab[NUNIT];
char	hm_dt[NUNIT];	/* drive type */

#define	GO	01
#define	PRESET	020
#define	RTC	016
#define	OFFSET	014
#define	SEARCH	030
#define	RECAL	06
#define DCLR	010
#define	WCOM	060
#define	RCOM	070

#define	IE	0100
#define	PIP	020000
#define	DRY	0200
#define	ERR	040000
#define	TRE	040000
#define	DCK	0100000
#define	WLE	04000
#define	ECH	0100
#define VV	0100
#define	DPR	0400
#define	MOL	010000
#define FMT22	010000

#define	b_cylin	b_resid

daddr_t dkblock();

hmstrategy(bp)
register struct buf *bp;
{
	register struct buf *dp;
	register unit;
	long sz, bn;
	int nbpc, dn, dt, fs, pri;

	if((bp->b_flags&B_PHYS) && !rh70hm)
		mapalloc(bp);
	unit = minor(bp->b_dev) & 077;
	dn = (unit >> 3) & 07;	/* get drive number */
	if(hm_dt[dn] == 0) {	/* get drive type, if not known */
		spl5();
		HMADDR->hmcs2.c[0] = dn;
		hm_dt[dn] = HMADDR->hmdt & 0377;
#if ML
/*
 * Find the size of the ML11 unit by reading the
 * number of array modules from the ML maintenance register.
 * There are 512 blocks per array unless the array type
 * bit is set, in which case there are 2048 blocks per array.
 */
		if(hm_dt[dn] == ML11) {
			hm_sizes[dn+16].nblocks = (HMADDR->hmmr >> 2) & 037000;
			if(HMADDR->hmmr & 02000)
				hm_sizes[dn+16].nblocks <<= 2;
/*
 * Check the ML11 transfer rate.
 * 2mb is too fast for any pdp11
 * 1mb allowed on pdp11/70 only
 * .5mb & .25mb ok on any processor.
 */
			fs = HMADDR->hmmr & 01400;
			if((fs == 0) || ((fs == 0400) && !rh70hm)) {
				printf("\nML11 xfer rate error\n");
				goto bad;
				}
			hm_ios[dn].dk_tr = ((fs >> 8) & 3) + 4;
			}
		spl0();
#endif
		if(hm_dt[dn] != ML11) {
			hm_ios[dn].dk_tr = 1;
			if(hm_dt[dn] == RM03)
				hm_ios[dn].dk_tr = 2;
			}
		pri = spl6();
		dk_iop[DK_N] = &hm_ios[0];
		dk_nd[DK_N] = NUNIT;
		splx(pri);
		}
/*
 * Set up the index into the sizes table (dt)
 * and the number of blocks per cylinder (nbpc).
 */
#if ML
	if(hm_dt[dn] == ML11)
		dt = 16;
	else
#endif
	if((hm_dt[dn] == RM02) || (hm_dt[dn] == RM03)) {
		nbpc = NRMSEC*NRMTRK;
		dt = 8;
		}
	else
	if(hm_dt[dn] <= RP06) {
		nbpc = NRPSEC*NRPTRK;
		dt = 0;
		}
	else
	goto bad;
	sz = bp->b_bcount;
	sz = (sz+511) >> 9;
	fs = unit & 07;		/* For RP & RM, fs is partition */
	if(dt == 16)		/* For ML, fs is unit number */
		fs = dn;
#if ML
	if((hm_dt[dn] == ML11) && (unit&7))
		goto bad;	/* ML11 is not partitioned */
#endif
	if (unit >= (NUNIT<<3) ||
	    bp->b_blkno < 0 ||
	    (bn = dkblock(bp))+sz > hm_sizes[fs+dt].nblocks) {
	bad:
		u.u_error = ENXIO;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
#if ML
	if(dt == 16)
		bp->b_cylin = 0;	/* fake out dsort for ML11 */
	else
#endif
	bp->b_cylin = bn/nbpc + hm_sizes[fs+dt].cyloff;
	unit = dkunit(bp);
	dp = &hmutab[unit];
	spl5();
	disksort(dp, bp);
	if (dp->b_active == 0) {
		hmustart(unit);
		if(hmtab.b_active == 0)
			hmstart();
	}
	spl0();
}

hmustart(unit)
register unit;
{
	register struct buf *bp, *dp;
	daddr_t bn;
	int sn, cn, csn;
	int nsect, ntrac;

	HMADDR->hmcs2.w = unit;
	HMADDR->hmcs1.c[0] = IE;
	HMADDR->hmas = 1<<unit;

	if(unit >= NUNIT)
		return;
	if(hm_dt[unit] <= RP06) {
		nsect = NRPSEC;
		ntrac = NRPTRK;
	} else {
		nsect = NRMTRK;
		ntrac = NRMTRK;
		}
	hm_ios[unit].dk_busy = 0;
	dp = &hmutab[unit];
	if((bp=dp->b_actf) == NULL)
		return;
	if((HMADDR->hmds & VV) == 0) {
		HMADDR->hmcs1.c[0] = IE|PRESET|GO;
#if ML
		if(hm_dt[unit] != ML11)
#endif
			HMADDR->hmof = FMT22;
	}
	if(dp->b_active)
		goto done;
#if ML
	if(hm_dt[unit] == ML11) {
		sn = dkblock(bp);
		dp->b_active++;
		goto mlsrch;
		}
#endif
	dp->b_active++;
	if ((HMADDR->hmds & (DPR|MOL)) != (DPR|MOL))
		goto done;

	bn = dkblock(bp);
	cn = bp->b_cylin;
	sn = bn%(nsect*ntrac);
	sn = (sn+nsect-SDIST)%nsect;

	if(HMADDR->hmcc != cn)
		goto search;
	csn = (HMADDR->hmla>>6) - sn + SDIST - 1;
	if(csn < 0)
		csn += nsect;
	if(csn > nsect-RDIST)
		goto done;

search:
	HMADDR->hmdc = cn;
mlsrch:
	HMADDR->hmda = sn;
	hm_ios[unit].dk_busy++;
	HMADDR->hmcs1.c[0] = IE|SEARCH|GO;
	return;

done:
	dp->b_forw = NULL;
	if(hmtab.b_actf == NULL)
		hmtab.b_actf = dp; else
		hmtab.b_actl->b_forw = dp;
	hmtab.b_actl = dp;
}

hmstart()
{
	register struct buf *bp, *dp;
	register unit;
	daddr_t bn;
	int dn, sn, tn, cn, dt;
	int nsect, ntrac;

loop:
	if ((dp = hmtab.b_actf) == NULL)
		return;
	if ((bp = dp->b_actf) == NULL) {
		hmtab.b_actf = dp->b_forw;
		goto loop;
	}
	hmtab.b_active++;
	unit = minor(bp->b_dev) & 077;
	dn = dkunit(bp);
	spl5();
	HMADDR->hmcs2.w = dn;
	if(hm_dt[dn] <= RP06) {
		dt = 0;
		nsect = NRPSEC;
		ntrac = NRPTRK;
	} else {
		dt = 8;
		nsect = NRMSEC;
		ntrac = NRMTRK;
		}
	bn = dkblock(bp);
	if(hm_dt[dn] != ML11) {
		cn = bn/(nsect*ntrac) + hm_sizes[(unit&07)+dt].cyloff;
		sn = bn%(nsect*ntrac);
		tn = sn/nsect;
		sn = sn%nsect;
		}

	if ((HMADDR->hmds & (DPR|MOL)) != (DPR|MOL)) {
		hmtab.b_active = 0;
		hmtab.b_errcnt = 0;
		dp->b_actf = bp->av_forw;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		spl0();
		goto loop;
	}
	if((hmtab.b_errcnt >= 16) && (hm_dt[dn] != ML11)) {
		HMADDR->hmof = hm_offset[hmtab.b_errcnt & 017] | FMT22;
		HMADDR->hmcs1.w = OFFSET|GO;
		while(HMADDR->hmds & PIP)
			;
	}
	if(hm_dt[dn] == ML11)
		HMADDR->hmda = bn;
	else {
		HMADDR->hmdc = cn;
		HMADDR->hmda = (tn << 8) + sn;
	}
	HMADDR->hmba = bp->b_un.b_addr;
	if(rh70hm)
		HMADDR->hmbae = bp->b_xmem;
	HMADDR->hmwc = -(bp->b_bcount>>1);
	unit = ((bp->b_xmem&3) << 8) | IE | GO;
	if(bp->b_flags & B_READ)
		unit |= RCOM; else
		unit |= WCOM;
	HMADDR->hmcs1.w = unit;

	hm_ios[dn].dk_busy++;		/* Instrumentation - disk active, */
	hm_ios[dn].dk_numb++;		/* count number of transfers, */
	unit = bp->b_bcount >> 6;	/* transfer size in 32 word chunks, */
	hm_ios[dn].dk_wds += unit;	/* count total words transferred */
	spl0();
}

hmintr()
{
	register struct buf *bp, *dp;
	register unit;
	register seg, uba;
	int as;
	int tpuis[2], tpbad[2], tppos, tpofst;
	int bsp;
	int pri, ctr, dt

	as = HMADDR->hmas & 0377;
	if(hmtab.b_active) {
		dp = hmtab.b_actf;
		bp = dp->b_actf;
		unit = dkunit(bp);
		HMADDR->hmcs2.c[0] = unit;
		hm_ios[unit].dk_busy = 0;
		if (HMADDR->hmcs1.w & TRE) {		/* error bit */
			while((HMADDR->hmds & DRY) == 0)
				;
#if ML
			if((hm_dt[unit] == ML11) && (hmtab.b_errcnt == 0))
				hmtab.b_errcnt = 18;
#endif
			if(++hmtab.b_errcnt > 28 || HMADDR->hmer1&WLE)
				bp->b_flags |= B_ERROR; else
				hmtab.b_active = 0;
			if(hmtab.b_errcnt > 27)
				deverror(bp, HMADDR->hmcs2.w, HMADDR->hmer1);
/* Start of ECC code, please excuse the improper indentation */
	if(((HMADDR->hmer1&(DCK|ECH)) == DCK) && (hm_dt[unit] != ML11))
	{
		pri = spl7();
		tpuis[0] = UISA->r[0];	/* save User I space PAR */
		tpuis[1] = UISD->r[0];	/* save User I space PDR */
		if(rh70hm)
			uba = (HMADDR->hmbae << 10) & 0176000;
		else
			uba = (HMADDR->hmcs1.w & 01400) << 2;
		uba += (((HMADDR->hmba >> 1) & ~0100000) >> 5);
		uba -= 8;
		if(bp->b_flags & B_MAP) {
			ctr = ((uba >> 6) & 076);
			seg = UBMAP->r[ctr+1] << 10;
			seg |= (UBMAP->r[ctr] >> 6) & 01777;
			UISA->r[0] = seg + (uba & 0177);
		} else
			UISA->r[0] = uba;
		UISD->r[0] = 077406;	/* Set User I space PDR */
		tppos = (HMADDR->hmec1 - 1)/16;	/* word addr of error burst */
		tpofst = (HMADDR->hmec1 - 1)%16;	/* bit in word */
		bsp = HMADDR->hmba & 077;
		bsp += (tppos*2);
		tpbad[0] = fuiword(bsp);
		tpbad[1] = fuiword(bsp+2);
		tpbad[0] ^= (HMADDR->hmec2 << tpofst); /* xor first word */
		if(tpofst > 5) {	/* must fix second word */
			ctr = (HMADDR->hmec2 >> 1) & ~0100000;
			if(tpofst < 15)
				ctr = ctr >> (15 - tpofst);
			tpbad[1] ^= ctr;	/* xor second word */
			}
		suiword(bsp,tpbad[0]);
		suiword((bsp+2),tpbad[1]);
		UISD->r[0] = tpuis[1];	/* restore User PDR */
		UISA->r[0] = tpuis[0];	/* restore User PAR */
		splx(pri);
		/* now see if transfer finished. */
		/* If not then reset;track,sector,etc. */
		if(HMADDR->hmwc != 0)
			{
			tpbad[0] = bp->b_bcount >> 1;
			tpbad[1] = -(HMADDR->hmwc);
			bp->b_blkno =+ (tpbad[0] - tpbad[1]) / 256;
			bp->b_bcount = -(HMADDR->hmwc) << 1;
			bp->b_un.b_addr = HMADDR->hmba;
			if(rh70hm)
				bp->b_xmem = HMADDR->hmbae & 077;
			else
				bp->b_xmem = (HMADDR->hmcs1.w & 01400) >> 8;
			printf("%D ", (bp->b_blkno - 1));
			prdev("ECC", bp->b_dev);
			HMADDR->hmcs1.w = TRE|DCLR|GO;
			hmstart();	/* Continue transfer */
			return;
			}
		hmtab.b_active++;
		printf("%D ", bp->b_blkno);
		prdev("ECC", bp->b_dev);
	}
/* End of ECC code					*/
			HMADDR->hmcs1.w = TRE|IE|DCLR|GO;
			if(((hmtab.b_errcnt&07) == 4) && (hm_dt[unit] != ML11)) {
				HMADDR->hmcs1.w = RECAL|IE|GO;
				while(HMADDR->hmds & PIP)
					;
			}
		}
		if(hmtab.b_active) {
			if((hmtab.b_errcnt) && (hm_dt[unit] != ML11)) {
				HMADDR->hmcs1.w = RTC|GO;
				while(HMADDR->hmds & PIP)
					;
			}
			hmtab.b_active = 0;
			hmtab.b_errcnt = 0;
			hmtab.b_actf = dp->b_forw;
			dp->b_active = 0;
			dp->b_errcnt = 0;
			dp->b_actf = bp->av_forw;
			bp->b_resid = -(HMADDR->hmwc<<1);
			iodone(bp);
			HMADDR->hmcs1.w = IE;
			if(dp->b_actf)
				hmustart(unit);
		}
		as &= ~(1<<unit);
	} else {
		if(as == 0)
			HMADDR->hmcs1.w = IE;
		HMADDR->hmcs1.c[1] = TRE>>8;
	}
	for(unit=0; unit<NUNIT; unit++)
		if(as & (1<<unit))
			hmustart(unit);
	hmstart();
}

hmread(dev)
{

	physio(hmstrategy, &rhmbuf, dev, B_READ);
}

hmwrite(dev)
{

	physio(hmstrategy, &rhmbuf, dev, B_WRITE);
}
