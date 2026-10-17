/*
 * UNIX/v7m - ML11 solid state disk driver.
 * Fred Canter 9/8/81
 *
 * This driver does not support mixed drive types,
 * it requires that the ML11 be on a separate
 * RH11 or RH70 controller and that there be only
 * ML11 drives on that controller.
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/seg.h"

struct	device
{
	union {
		int	w;
		char	c[2];
	} mlcs1;		/* Control and Status register 1 */
	int	mlwc;		/* Word count register */
	caddr_t	mlba;		/* UNIBUS address register */
	int	mlda;		/* Desired address register */
	union {
		int	w;
		char	c[2];
	} mlcs2;		/* Control and Status register 2 */
	int	mlds;		/* Drive Status */
	int	mler;		/* Error register */
	int	mlas;		/* Attention Summary */
	int	mlpa;		/* Prom address register */
	int	mldb;		/* Data buffer */
	int	mlmr;		/* Maintenance register */
	int	mldt;		/* Drive type */
	int	mlsn;		/* Serial number */
	int	mle1;		/* ECC CRC word 1 */
	int	mle2;		/* ECC CRC word 2 */
	int	mld1;		/* Data diagnostic 1 register */
	int	mld2;		/* Data diagnostic 2 register */
	int	mlee;		/* ECC error register */
	int	mlel;		/* ECC error location register */
	int	mlpd;		/* Prom data register */
	int	mlbae;		/* RH70 bus address extension */
	int	mlcs3;		/* RH70 control & status register 3 */
};

#define	MLADDR	((struct device *)0176400)
#define	NUNIT	1

/*
 * Instrumentation (iostat) structures
 */
struct	ios	ml_ios[NUNIT];
#define	DK_N	3
#define	DK_T	4
char	ml_opn;

/*
 * Drive type definition
 */
#define	ML11	0110

/*
 * ml_sizes[] contains the size of each ML11
 * unit in blocks. The size is set dynamicaly
 * by reading the number of arrays from the ML maintenance
 * register. The array type bit the the ML maintenacne
 * register is used to adjust the number of blocks per array
 * from 512 (16k chips) to 2048 (64k chips).
 */

char	*ml_sizes[8] =
{
	0,	/* ML11   unit	0	*/
	0,	/*		1	*/
	0,	/*		2	*/
	0,	/*		3	*/
	0,	/*		4	*/
	0,	/*		5	*/
	0,	/*		6	*/
	0,	/*		7	*/
};

struct	buf	mltab;
struct	buf	rmlbuf;

#define	GO	01
#define	PRESET	020
#define DCLR	010
#define	WCOM	060
#define	RCOM	070

#define	IE	0100
#define	DRY	0200
#define	ERR	040000
#define	TRE	040000
#define	DCK	0100000
#define	ECH	0100
#define VV	0100
#define	DPR	0400
#define	MOL	010000

mlstrategy(bp)
register struct buf *bp;
{
	register struct buf *dp;
	register unit;
	int tr;
	long sz, bn;

	if((bp->b_flags&B_PHYS) && !rh70ml)
		mapalloc(bp);
	unit = minor(bp->b_dev) & 07;
	if(unit >= NUNIT)
		goto bad;
	spl5();
	if((MLADDR->mldt & 0377) != ML11)
		goto bad;
	MLADDR->mlcs2.c[0] = unit;
/*
 * Find the size of the ML11 unit by reading the
 * number of array modules from the ML maintenance register.
 * There are 512 blocks per array unless the array type
 * bit is set, in which case there are 2048 blocks per array.
 */
	ml_sizes[unit] = (MLADDR->mlmr >> 2) & 037000;
	if(MLADDR->mlmr & 02000)
		ml_sizes[unit] <<= 2;
/*
 * Check the ML11 transfer rate.
 * 2mb is too fast for any pdp11
 * 1mb allowed on pdp11/70 only
 * .5mb & .25mb ok on any processor
 */
	tr = MLADDR->mlmr & 01400;
	if((tr == 0) || ((tr == 0400) && !rh70ml)) {
		printf("\nML11 xfer rate error\n");
		goto bad;
		}
	sz = bp->b_bcount;
	sz = (sz+511) >> 9;
	if (bp->b_blkno < 0 ||
	    (bp->b_blkno + sz) > ml_sizes[unit]) {
	bad:
		spl0();
		u.u_error = ENXIO;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
	if(ml_opn == 0) {
		ml_opn++;
		tr = spl6();
		dk_iop[DK_N] = &ml_ios[0];
		dk_nd[DK_N] = NUNIT;
		splx(tr);
		}
	bp->av_forw = NULL;
	dp = &mltab;
	if(dp->b_actf == NULL)
		dp->b_actf = bp;
	else
		dp->b_actl->av_forw = bp;
	dp->b_actl = bp;
	if(dp->b_active == NULL)
		mlstart();
	spl0();
}

mlstart()
{
	register struct buf *bp;
	register unit, com;

	if ((bp = mltab.b_actf) == NULL)
		return;
	mltab.b_active++;
	unit = minor(bp->b_dev) & 07;
	spl5();
	MLADDR->mlcs2.w = unit;
	if((MLADDR->mlds & VV) == 0)
		MLADDR->mlcs1.w = PRESET|GO;
	ml_ios[unit].dk_tr = ((MLADDR->mlmr >> 8) & 03 ) + DK_T;
	if ((MLADDR->mlds & (DPR|MOL)) != (DPR|MOL)) {
		mltab.b_active = 0;
		mltab.b_errcnt = 0;
		bp->b_actf = bp->av_forw;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		spl0();
		return;
	}
	MLADDR->mlda = bp->b_blkno;
	MLADDR->mlba = bp->b_un.b_addr;
	if(rh70ml)
		MLADDR->mlbae = bp->b_xmem;
	MLADDR->mlwc = -(bp->b_bcount>>1);
	com = ((bp->b_xmem&3) << 8) | IE | GO;
	if(bp->b_flags & B_READ)
		com |= RCOM; else
		com |= WCOM;
	MLADDR->mlcs1.w = com;
	ml_ios[unit].dk_busy++;		/* drive active */
	ml_ios[unit].dk_numb++;		/* count xfer's */
	ml_ios[unit].dk_wds += (bp->b_bcount >> 6);	/* count words xfer'd */
	spl0();
}

mlintr()
{
	register struct buf *bp;
	register unit;
	int as;
	int ctr;

	if(mltab.b_active = NULL)
		return;
	as = MLADDR->mlas & 0377;
	bp = mltab.b_actf;
	unit = minor(bp->b_dev) & 7;
	ml_ios[unit].dk_busy = 0;	/* drive no longer active */
	as &= ~(1<<unit);
	MLADDR->mlas = as;
	MLADDR->mlcs2.c[0] = unit;
	if (MLADDR->mlcs1.w & TRE) {		/* error bit */
		ctr = 0;
		while(((MLADDR->mlds & DRY) == 0) && --ctr) ;
		if(mltab.b_errcnt == 0)
			deverror(bp, MLADDR->mlcs2.w, MLADDR->mler);
		MLADDR->mlcs1.w = DCLR|GO;
		if(++mltab.b_errcnt <= 10) {
			mlstart();
			return;
			}
		bp->b_flags |= B_ERROR;
	}
	mltab.b_errcnt = 0;
	mltab.b_actf = bp->av_forw;
	bp->b_resid = 0;
	iodone(bp);
	mlstart();
}

mlread(dev)
{

	physio(mlstrategy, &rmlbuf, dev, B_READ);
}

mlwrite(dev)
{

	physio(mlstrategy, &rmlbuf, dev, B_WRITE);
}
