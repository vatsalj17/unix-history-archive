#
/*
 * RP11/RP03 disk driver
 */

#include "../hd/param.h"
#include "../hd/buf.h"
#include "../hd/user.h"

struct {
	int	rpds, rper, rpcs, rpwc;
	int	rpba, rpca, rpda;
};

int	rp_addr, rp_cnt;

struct {
	char	*nblocks;
	int	cyloff;
} rp_sizes[8];

struct	devtab	rptab;
struct	buf	rrpbuf;

#define	GO	01
#define	RESET	0
#define	HSEEK	014

#define	IENABLE	0100
#define	READY	0200

#define	SUFU	01000
#define	SUSU	02000
#define	SUSI	04000
#define	HNF	010000

/*
 * Use av_back to save track+sector,
 * b_resid for cylinder.
 */

#define	trksec	av_back
#define	cylin	b_resid


rpopen(dev)
{
	if (dev >= (rp_cnt<<3))
		u.u_error = ENXIO;
}

rpclose(dev)
{
}

rpstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register char *p1, *p2;

	bp = abp;
	p1 = &rp_sizes[minor(bp->b_dev)&07];
	if (bp->b_blkno >= p1->nblocks) {
		if (bp->b_blkno == p1->nblocks && bp->b_flags&B_READ)
			bp->b_resid = 512;
		else {
			bp->b_flags =| B_ERROR;
			bp->b_error = ENXIO;
		}
		iodone(bp);
		return;
	}
	bp->av_forw = 0;
	bp->cylin = p1->cyloff;
	p1 = bp->b_blkno;
	p2 = lrem(p1, 10);
	p1 = ldiv(p1, 10);
	bp->trksec = (p1%20)<<8 | p2;
	bp->cylin =+ p1/20;
	spl5();
	if ((p1 = rptab.d_actf)==0) {
		rptab.d_actf = bp;
		rptab.d_actl = bp;
	} else {
		for (p1 = rptab.d_actl; p2 = p1->av_forw; p1 = p2) {
			if (p1->cylin <= bp->cylin
			 && bp->cylin <  p2->cylin
			 || p1->cylin >= bp->cylin
			 && bp->cylin >  p2->cylin) 
				break;
		}
		bp->av_forw = p2;
		p1->av_forw = bp;
	}
	if (rptab.d_active==0)
		rpstart();
	spl0();
}

rpstart()
{
	register struct buf *bp;
	register *rp;

	if ((bp = rptab.d_actf) == 0)
		return;
	rptab.d_active++;
	rp = rp_addr;
	rp->rpda = bp->trksec;
	devstart(bp, &rp->rpca, bp->cylin, minor(bp->b_dev)>>3);
}

rpintr()
{
	register struct buf *bp;
	register ctr, *rp;

	if (rptab.d_active == 0)
		return;
	bp = rptab.d_actf;
	rp = rp_addr;
	rptab.d_active = 0;
	if (rp->rpcs < 0) {		/* error bit */
		deverror(bp, rp->rper, rp->rpds);
		if(rp->rpds & (SUFU|SUSI|HNF)) {
			rp->rpcs.lobyte = HSEEK|GO;
			ctr = 0;
			while ((rp->rpds&SUSU) && --ctr);
		}
		rp->rpcs = RESET|GO;
		ctr = 0;
		while ((rp->rpcs&READY) == 0 && --ctr);
		if (++rptab.d_errcnt <= 10) {
			rpstart();
			return;
		}
		bp->b_flags =| B_ERROR;
	}
	rptab.d_errcnt = 0;
	rptab.d_actf = bp->av_forw;
	bp->b_resid = (-rp->rpwc)<<1;
	iodone(bp);
	if(bp=rptab.d_actf) while(bp->av_forw) bp = bp->av_forw;
	rptab.d_actl = bp;
	rpstart();
}

rpread(dev)
{
	if (physck(rp_sizes[dev&07].nblocks, B_READ))
		physio(rpstrategy, &rrpbuf, dev, B_READ);
}

rpwrite(dev)
{
	if (physck(rp_sizes[dev&07].nblocks, B_WRITE))
		physio(rpstrategy, &rrpbuf, dev, B_WRITE);
}

