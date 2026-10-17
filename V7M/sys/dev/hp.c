/*
 * UNIX/v7m - RM02/3, RP04/5/6, & ML11 disk driver.
 * Disks on first RH controller (176700)
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
	} hpcs1;		/* Control and Status register 1 */
	int	hpwc;		/* Word count register */
	caddr_t	hpba;		/* UNIBUS address register */
	int	hpda;		/* Desired address register */
	union {
		int	w;
		char	c[2];
	} hpcs2;		/* Control and Status register 2 */
	int	hpds;		/* Drive Status */
	int	hper1;		/* Error register 1 */
/*		mler	*/	/* Error register */
	int	hpas;		/* Attention Summary */
	int	hpla;		/* Look ahead */
/*		mlpa	*/	/* Prom address register */
	int	hpdb;		/* Data buffer */
	int	hpmr;		/* Maintenance register */
/*		rmmr1	*/	/* Maintenance register 1 */
	int	hpdt;		/* Drive type */
	int	hpsn;		/* Serial number */
	int	hpof;		/* Offset register */
/*		mle1	*/	/* ECC CRC word 1 */
	int	hpdc;		/* Desired Cylinder address register */
/*		mle2	*/	/* ECC CRC word 2 */
	int	hpcc;		/* Current Cylinder */
/*		mld1	*/	/* Data diagnostic 1 register */
/*		rmhr	*/	/* Holding register */
	int	hper2;		/* Error register 2 */
/*		mld2	*/	/* Data diagnostic 2 register */
/*		rmmr2	*/	/* Maintenance register 2 */
	int	hper3;		/* Error register 3 */
/*		mlee	*/	/* ECC error register */
/*		rmer2	*/	/* Error register 2 */
	int	hpec1;		/* Burst error bit position */
/*		mlel	*/	/* ECC error location register */
	int	hpec2;		/* Burst error bit pattern */
/*		mlpd	*/	/* Prom data register */
	int	hpbae;		/* 11/70 bus extension */
	int	hpcs3;		/* 11/70 control & status 3 */
};

#define	HPADDR	((struct device *)0176700)
#define	NUNIT	2
/*
 * Instrumentation (iostat) structures
 */
struct	ios	hp_ios[NUNIT];
#define	DK_N	0

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
 * The number of elements in the hp_sizes[]
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
} hp_sizes[24] =
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
int	hp_offset[16] =
{
	P400, M400, P400, M400,
	P800, M800, P800, M800,
	P1200, M1200, P1200, M1200,
	0, 0, 0, 0,
};

struct	buf	hptab;
struct	buf	rhpbuf;
struct	buf	hputab[NUNIT];
char	hp_dt[NUNIT];	/* drive type */

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

hpstrategy(bp)
register struct buf *bp;
{
	register struct buf *dp;
	register unit;
	long sz, bn;
	int nbpc, dn, dt, fs, pri;

	if((bp->b_flags&B_PHYS) && !rh70hp)
		mapalloc(bp);
	unit = minor(bp->b_dev) & 077;
	dn = (unit >> 3) & 07;	/* get drive number */
	if(hp_dt[dn] == 0) {	/* get drive type, if not known */
		spl5();
		HPADDR->hpcs2.c[0] = dn;
		hp_dt[dn] = HPADDR->hpdt & 0377;
#if ML
/*
 * Find the size of the ML11 unit by reading the
 * number of array modules from the ML maintenance register.
 * There are 512 blocks per array unless the array type
 * bit is set, in which case there are 2048 blocks per array.
 */
		if(hp_dt[dn] == ML11) {
			hp_sizes[dn+16].nblocks = (HPADDR->hpmr >> 2) & 037000;
			if(HPADDR->hpmr & 02000)
				hp_sizes[dn+16].nblocks <<= 2;
/*
 * Check the ML11 transfer rate.
 * 2mb is too fast for any pdp11
 * 1mb allowed on pdp11/70 only
 * .5mb & .25mb ok on any processor.
 */
			fs = HPADDR->hpmr & 01400;
			if((fs == 0) || ((fs == 0400) && !rh70hp)) {
				printf("\nML11 xfer rate error\n");
				goto bad;
				}
			hp_ios[dn].dk_tr = ((fs >> 8) & 3) + 4;
			}
		spl0();
#endif
		if(hp_dt[dn] != ML11) {
			hp_ios[dn].dk_tr = 1;
			if(hp_dt[dn] == RM03)
				hp_ios[dn].dk_tr = 2;
			}
		pri = spl6();
		dk_iop[DK_N] = &hp_ios[0];
		dk_nd[DK_N] = NUNIT;
		splx(pri);
		}
/*
 * Set up the index into the sizes table (dt)
 * and the number of blocks per cylinder (nbpc).
 */
#if ML
	if(hp_dt[dn] == ML11)
		dt = 16;
	else
#endif
	if((hp_dt[dn] == RM02) || (hp_dt[dn] == RM03)) {
		nbpc = NRMSEC*NRMTRK;
		dt = 8;
		}
	else
	if(hp_dt[dn] <= RP06) {
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
	if((hp_dt[dn] == ML11) && (unit&7))
		goto bad;	/* ML11 is not partitioned */
#endif
	if (unit >= (NUNIT<<3) ||
	    bp->b_blkno < 0 ||
	    (bn = dkblock(bp))+sz > hp_sizes[fs+dt].nblocks) {
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
	bp->b_cylin = bn/nbpc + hp_sizes[fs+dt].cyloff;
	unit = dkunit(bp);
	dp = &hputab[unit];
	spl5();
	disksort(dp, bp);
	if (dp->b_active == 0) {
		hpustart(unit);
		if(hptab.b_active == 0)
			hpstart();
	}
	spl0();
}

hpustart(unit)
register unit;
{
	register struct buf *bp, *dp;
	daddr_t bn;
	int sn, cn, csn;
	int nsect, ntrac;

	HPADDR->hpcs2.w = unit;
	HPADDR->hpcs1.c[0] = IE;
	HPADDR->hpas = 1<<unit;

	if(unit >= NUNIT)
		return;
	if(hp_dt[unit] <= RP06) {
		nsect = NRPSEC;
		ntrac = NRPTRK;
	} else {
		nsect = NRMTRK;
		ntrac = NRMTRK;
		}
	hp_ios[unit].dk_busy = 0;
	dp = &hputab[unit];
	if((bp=dp->b_actf) == NULL)
		return;
	if((HPADDR->hpds & VV) == 0) {
		HPADDR->hpcs1.c[0] = IE|PRESET|GO;
#if ML
		if(hp_dt[unit] != ML11)
#endif
			HPADDR->hpof = FMT22;
	}
	if(dp->b_active)
		goto done;
#if ML
	if(hp_dt[unit] == ML11) {
		sn = dkblock(bp);
		dp->b_active++;
		goto mlsrch;
		}
#endif
	dp->b_active++;
	if ((HPADDR->hpds & (DPR|MOL)) != (DPR|MOL))
		goto done;

	bn = dkblock(bp);
	cn = bp->b_cylin;
	sn = bn%(nsect*ntrac);
	sn = (sn+nsect-SDIST)%nsect;

	if(HPADDR->hpcc != cn)
		goto search;
	csn = (HPADDR->hpla>>6) - sn + SDIST - 1;
	if(csn < 0)
		csn += nsect;
	if(csn > nsect-RDIST)
		goto done;

search:
	HPADDR->hpdc = cn;
mlsrch:
	HPADDR->hpda = sn;
	hp_ios[unit].dk_busy++;
	HPADDR->hpcs1.c[0] = IE|SEARCH|GO;
	return;

done:
	dp->b_forw = NULL;
	if(hptab.b_actf == NULL)
		hptab.b_actf = dp; else
		hptab.b_actl->b_forw = dp;
	hptab.b_actl = dp;
}

hpstart()
{
	register struct buf *bp, *dp;
	register unit;
	daddr_t bn;
	int dn, sn, tn, cn, dt;
	int nsect, ntrac;

loop:
	if ((dp = hptab.b_actf) == NULL)
		return;
	if ((bp = dp->b_actf) == NULL) {
		hptab.b_actf = dp->b_forw;
		goto loop;
	}
	hptab.b_active++;
	unit = minor(bp->b_dev) & 077;
	dn = dkunit(bp);
	spl5();
	HPADDR->hpcs2.w = dn;
	if(hp_dt[dn] <= RP06) {
		dt = 0;
		nsect = NRPSEC;
		ntrac = NRPTRK;
	} else {
		dt = 8;
		nsect = NRMSEC;
		ntrac = NRMTRK;
		}
	bn = dkblock(bp);
	if(hp_dt[dn] != ML11) {
		cn = bn/(nsect*ntrac) + hp_sizes[(unit&07)+dt].cyloff;
		sn = bn%(nsect*ntrac);
		tn = sn/nsect;
		sn = sn%nsect;
		}

	if ((HPADDR->hpds & (DPR|MOL)) != (DPR|MOL)) {
		hptab.b_active = 0;
		hptab.b_errcnt = 0;
		dp->b_actf = bp->av_forw;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		spl0();
		goto loop;
	}
	if((hptab.b_errcnt >= 16) && (hp_dt[dn] != ML11)) {
		HPADDR->hpof = hp_offset[hptab.b_errcnt & 017] | FMT22;
		HPADDR->hpcs1.w = OFFSET|GO;
		while(HPADDR->hpds & PIP)
			;
	}
	if(hp_dt[dn] == ML11)
		HPADDR->hpda = bn;
	else {
		HPADDR->hpdc = cn;
		HPADDR->hpda = (tn << 8) + sn;
	}
	HPADDR->hpba = bp->b_un.b_addr;
	if(rh70hp)
		HPADDR->hpbae = bp->b_xmem;
	HPADDR->hpwc = -(bp->b_bcount>>1);
	unit = ((bp->b_xmem&3) << 8) | IE | GO;
	if(bp->b_flags & B_READ)
		unit |= RCOM; else
		unit |= WCOM;
	HPADDR->hpcs1.w = unit;

	hp_ios[dn].dk_busy++;		/* Instrumentation - disk active, */
	hp_ios[dn].dk_numb++;		/* count number of transfers, */
	unit = bp->b_bcount >> 6;	/* transfer size in 32 word chunks, */
	hp_ios[dn].dk_wds += unit;	/* count total words transferred */
	spl0();
}

hpintr()
{
	register struct buf *bp, *dp;
	register unit;
	register seg, uba;
	int as;
	int tpuis[2], tpbad[2], tppos, tpofst;
	int bsp;
	int pri, ctr, dt

	as = HPADDR->hpas & 0377;
	if(hptab.b_active) {
		dp = hptab.b_actf;
		bp = dp->b_actf;
		unit = dkunit(bp);
		HPADDR->hpcs2.c[0] = unit;
		hp_ios[unit].dk_busy = 0;
		if (HPADDR->hpcs1.w & TRE) {		/* error bit */
			while((HPADDR->hpds & DRY) == 0)
				;
#if ML
			if((hp_dt[unit] == ML11) && (hptab.b_errcnt == 0))
				hptab.b_errcnt = 18;
#endif
			if(++hptab.b_errcnt > 28 || HPADDR->hper1&WLE)
				bp->b_flags |= B_ERROR; else
				hptab.b_active = 0;
			if(hptab.b_errcnt > 27)
				deverror(bp, HPADDR->hpcs2.w, HPADDR->hper1);
/* Start of ECC code, please excuse the improper indentation */
	if(((HPADDR->hper1&(DCK|ECH)) == DCK) && (hp_dt[unit] != ML11))
	{
		pri = spl7();
		tpuis[0] = UISA->r[0];	/* save User I space PAR */
		tpuis[1] = UISD->r[0];	/* save User I space PDR */
		if(rh70hp)
			uba = (HPADDR->hpbae << 10) & 0176000;
		else
			uba = (HPADDR->hpcs1.w & 01400) << 2;
		uba += (((HPADDR->hpba >> 1) & ~0100000) >> 5);
		uba -= 8;
		if(bp->b_flags & B_MAP) {
			ctr = ((uba >> 6) & 076);
			seg = UBMAP->r[ctr+1] << 10;
			seg |= (UBMAP->r[ctr] >> 6) & 01777;
			UISA->r[0] = seg + (uba & 0177);
		} else
			UISA->r[0] = uba;
		UISD->r[0] = 077406;	/* Set User I space PDR */
		tppos = (HPADDR->hpec1 - 1)/16;	/* word addr of error burst */
		tpofst = (HPADDR->hpec1 - 1)%16;	/* bit in word */
		bsp = HPADDR->hpba & 077;
		bsp += (tppos*2);
		tpbad[0] = fuiword(bsp);
		tpbad[1] = fuiword(bsp+2);
		tpbad[0] ^= (HPADDR->hpec2 << tpofst); /* xor first word */
		if(tpofst > 5) {	/* must fix second word */
			ctr = (HPADDR->hpec2 >> 1) & ~0100000;
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
		if(HPADDR->hpwc != 0)
			{
			tpbad[0] = bp->b_bcount >> 1;
			tpbad[1] = -(HPADDR->hpwc);
			bp->b_blkno =+ (tpbad[0] - tpbad[1]) / 256;
			bp->b_bcount = -(HPADDR->hpwc) << 1;
			bp->b_un.b_addr = HPADDR->hpba;
			if(rh70hp)
				bp->b_xmem = HPADDR->hpbae & 077;
			else
				bp->b_xmem = (HPADDR->hpcs1.w & 01400) >> 8;
			printf("%D ", (bp->b_blkno - 1));
			prdev("ECC", bp->b_dev);
			HPADDR->hpcs1.w = TRE|DCLR|GO;
			hpstart();	/* Continue transfer */
			return;
			}
		hptab.b_active++;
		printf("%D ", bp->b_blkno);
		prdev("ECC", bp->b_dev);
	}
/* End of ECC code					*/
			HPADDR->hpcs1.w = TRE|IE|DCLR|GO;
			if(((hptab.b_errcnt&07) == 4) && (hp_dt[unit] != ML11)) {
				HPADDR->hpcs1.w = RECAL|IE|GO;
				while(HPADDR->hpds & PIP)
					;
			}
		}
		if(hptab.b_active) {
			if((hptab.b_errcnt) && (hp_dt[unit] != ML11)) {
				HPADDR->hpcs1.w = RTC|GO;
				while(HPADDR->hpds & PIP)
					;
			}
			hptab.b_active = 0;
			hptab.b_errcnt = 0;
			hptab.b_actf = dp->b_forw;
			dp->b_active = 0;
			dp->b_errcnt = 0;
			dp->b_actf = bp->av_forw;
			bp->b_resid = -(HPADDR->hpwc<<1);
			iodone(bp);
			HPADDR->hpcs1.w = IE;
			if(dp->b_actf)
				hpustart(unit);
		}
		as &= ~(1<<unit);
	} else {
		if(as == 0)
			HPADDR->hpcs1.w = IE;
		HPADDR->hpcs1.c[1] = TRE>>8;
	}
	for(unit=0; unit<NUNIT; unit++)
		if(as & (1<<unit))
			hpustart(unit);
	hpstart();
}

hpread(dev)
{

	physio(hpstrategy, &rhpbuf, dev, B_READ);
}

hpwrite(dev)
{

	physio(hpstrategy, &rhpbuf, dev, B_WRITE);
}
