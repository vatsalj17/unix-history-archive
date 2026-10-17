#
/*
 * RS03/04 disk driver
 */

#include "../hd/param.h"
#include "../hd/buf.h"
#include "../hd/user.h"

struct {
	int	hscs1, hswc, hdba, hsda;
	int	hscs2, hsds, hser, hsas;
	int	hsla, hsdb, hsmr, hsdt;
	int	hsbae, hscs3;
};

struct	devtab	hstab;
struct	buf	rhsbuf;

int	hs_addr, hs_cnt;

#define ERR	040000	/* hscs1 - composite error */

#define GO	01
#define RCLR	010
#define	DRY	0200	/* hsds - Drive Ready */

hsopen(dev)
{
}

hsclose(dev)
{
}

hsstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register mblks;

	bp = abp;
	if(bp->b_dev&010)
		mblks = 2048; /* RS04 */
	else	mblks = 1024; /* RS03 */
	if(bp->b_blkno >= mblks) {
		if (bp->b_blkno == mblks && bp->b_flags&B_READ)
			bp->b_resid = 512;
		else {
			bp->b_flags =| B_ERROR;
			bp->b_error = ENXIO;
		}
		iodone(bp);
		return;
	}
	bp->av_forw = 0;
	spl5();
	if (hstab.d_actf==0)
		hstab.d_actf = bp; else
		hstab.d_actl->av_forw = bp;
	hstab.d_actl = bp;
	if (hstab.d_active==0)
		hsstart();
	spl0();
}

hsstart()
{
	register struct buf *bp;
	register *rp;
	register addr;

	if ((bp = hstab.d_actf) == 0)
		return;
	hstab.d_active++;
	rp = hs_addr;
	addr = bp->b_blkno<<1;
	if ((bp->b_dev&010) == 0)
		addr =<< 1;
	rp->hscs2 = bp->b_dev & 07;
	rhstart(bp, &rp->hsda, addr, &rp->hsbae);
}

hsintr()
{
	register struct buf *bp;
	register *rp;

	if (hstab.d_active == 0)
		return;
	bp = hstab.d_actf;
	rp = hs_addr;
	hstab.d_active = 0;
	if(rp->hscs1 & ERR){	/* error bit */
		deverror(bp, rp->hscs2, 0);
		rp->hscs1 = RCLR|GO;
		if (++hstab.d_errcnt <= 10) {
			hsstart();
			return;
		}
		bp->b_flags =| B_ERROR;
	}
	hstab.d_errcnt = 0;
	hstab.d_actf = bp->av_forw;
	bp->b_resid = (-rp->hswc)<<1;
	iodone(bp);
	hsstart();
}

hsread(dev)
{
	if (physck((dev&010) ? 2048 : 1024, B_READ))
		physio(hsstrategy, &rhsbuf, dev, B_READ);
}

hswrite(dev)
{
	if (physck((dev&010) ? 2048 : 1024, B_WRITE))
		physio(hsstrategy, &rhsbuf, dev, B_WRITE);
}
