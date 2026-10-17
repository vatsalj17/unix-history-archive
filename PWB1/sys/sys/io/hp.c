#
/*
 * RJP04, RWP04 driver
 */

#include "../hd/param.h"
#include "../hd/buf.h"
#include "../hd/user.h"

struct {
	int	hpcs1, hpwc, hpba, hpda;	
	int	hpcs2, hpds, hper1, hpas;	
	int	hpla, hpdb, hpmr, hpdt;	
	int	hpsn, hpof, hpdc, hpcc;	
	int	hper2, hper3, hpec1, hpec2;	
	int	hpbae, hpcs3;	
};

int	hp_addr, hp_cnt;

struct {
	unsigned nblocks;
	int	cyloff;
} hp_sizes[8];

struct	devtab	hptab;
struct	devtab	hputab[8];
struct	buf	hpbuf;
char	hpstatus[8];
int	hpsioc[8], hpsios[8];

#define	GO	01
#define	UNLOAD	02
#define RECAL	06
#define DCLR	010
#define	RELEASE	012
#define	PRESET	020
#define	SEARCH	030

#define	ERR	040000	/* hpds - Error */
#define PIP	020000
#define	MOL	010000	/* hpds - Medium online */
#define	DRY	0200	/* hpds - drive ready */
#define VV	0100	/* hpds - volume valid */
#define SC	0100000	/* hpcs1 - Special condition */
#define	TRE	040000	/* hpcs1 - transfer error */
#define DVA	04000	/* hpcs1 - drive available */
#define	IE	0100	/* hpcs1 - interrupt enable */
#define	WLE	04000	/* hper1 - Write lock error */
#define FMT22	010000	/* hpof - 16 bit /word format */
#define	ECI	04000	/* hpof - ecc inhibit */
/*
 * Use av_back to save track+sector,
 * b_resid for cylinder.
 */

#define	trksec	av_back
#define	cylin	b_resid

hpopen(dev)
{
	if (((dev>>3)&037)>hp_cnt)
		u.u_error = ENXIO;
}

hpclose(dev)
{
}

hpstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register char *p1, *p2;
	struct devtab *dp;
	int	unit;

	bp = abp;
	p1 = &hp_sizes[minor(bp->b_dev)&07];
	unit = minor(bp->b_dev)>>3;
	if (hpstatus[unit] || bp->b_blkno >= p1->nblocks) {
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
	p2 = lrem(p1, 22);
	p1 = ldiv(p1, 22);
	bp->trksec = (p1%19)<<8 | p2;
	bp->cylin =+ p1/19;
	spl5();
	dp = &hputab[unit];
	hpsioc[unit]++;
	if ((p1 = dp->d_actf)==0)
		dp->d_actf = bp;
	else {
		for (; p2 = p1->av_forw; p1 = p2) {
			if (p1->cylin <= bp->cylin
			 && bp->cylin <  p2->cylin
			 || p1->cylin >= bp->cylin
			 && bp->cylin >  p2->cylin) 
				break;
		}
		bp->av_forw = p2;
		p1->av_forw = bp;
	}
	if (dp->d_active==0) {
		hpustart(unit);
		if (hptab.d_active==0)
			hpstart();
	}
	spl0();
}

hpustart(unit)
{
	register struct buf *bp;
	register struct devtab *dp;
	register *rp;
	int	search;

	dp = &hputab[unit];
	rp = hp_addr;
	rp->hpcs2 = unit;
	rp->hpcs1.lobyte = IE;
	rp->hpas = 1<<unit;
	if ((rp->hpcs1&DVA)==0) {
	/* either NED or dual-port not avail */
		goto abort;
	}
	if (rp->hpds&ERR) {
		printf("RP04 drive %d errors: %o %o %o\n",
			unit, rp->hper1, rp->hper2, rp->hper3);
		rp->hpcs1.lobyte = IE|DCLR|GO;
		if (rp->hpds&ERR || ++dp->d_errcnt > 16)
			goto abort;
	}
	if ((rp->hpds&MOL)==0)
		goto abort;
	hpstatus[unit] = 0;
	if ((bp = dp->d_actf)==0)
		return;
	if ((rp->hpds&VV) == 0) {
		rp->hpcs1.lobyte = IE|PRESET|GO;
		rp->hpof = FMT22|ECI;
	}
	dp->d_active++;
	rp->hpdc = bp->cylin;
	search = bp->trksec.lobyte-(rp->hpla>>6)-1;
	if (search<0) search =+ 22;
	if ((bp->cylin!=rp->hpcc || search>6) &&
	    dp->d_active<3) {
		search = bp->trksec;
		search.lobyte =- 4;
		if (search.lobyte<0) search.lobyte =+ 22;
		hpsios[unit]++;
		rp->hpda = search;
		rp->hpcs1.lobyte = IE|SEARCH|GO;
	} else {
		dp->b_forw = 0;
		if (hptab.d_actf == 0)
			hptab.d_actf = dp; else
			hptab.d_actl->b_forw = dp;
		hptab.d_actl = dp;
	}
	return;
    abort:
	rp->hpas = 1<<unit;
	if (hpstatus[unit])
		return;
	hpstatus[unit] = 1;
	while(bp = dp->d_actf) {
		bp->b_flags =| B_ERROR;
		dp->d_actf = bp->av_forw;
		iodone(bp);
	}
	dp->d_active = 0;
	dp->d_errcnt = 0;
	printf("RP04 drive %d offline\n",unit);
}

hpstart()
{
	register struct buf *bp;
	register struct devtab *dp;
	register *rp;

    loop:
	if ((dp = hptab.d_actf) == 0)
		return;
	if ((bp = dp->d_actf) == 0) {
		hptab.d_actf = dp->b_forw;
		goto loop;
	}
	rp = hp_addr;
	rp->hpcs2 = minor(bp->b_dev) >> 3;
	hptab.d_active++;
	rp->hpdc = bp->cylin;
	rhstart(bp, &rp->hpda, bp->trksec, &rp->hpbae);
}

hpintr()
{
	register struct buf *bp;
	register unit;
	register *rp;
	struct devtab *dp;
	int as;

	rp = hp_addr;
	as = rp->hpas&0377;
	if (hptab.d_active) {	/* data transfer underway */
		dp = hptab.d_actf;
		bp = dp->d_actf;
		unit = minor(bp->b_dev)>>3;
		rp->hpcs2.lobyte = unit;
		if (rp->hpcs1 & TRE) {		/* error bit */
			while ((rp->hpds&DRY)==0);
			if (++hptab.d_errcnt>16 || rp->hper1&WLE)
				bp->b_flags =| B_ERROR; else
				hptab.d_active = 0;
			if (hptab.d_errcnt > 3)
				deverror(bp, rp->hper1, rp->hpcs2);
			rp->hpcs1 = TRE|IE|DCLR|GO;
			if ((hptab.d_errcnt&07)==04) {	/* 4,12 */
				rp->hpcs1 = IE|RECAL|GO;
				while ((rp->hpds&PIP));
			}
		}
		if (hptab.d_active) {
			hptab.d_active = 0;
			hptab.d_errcnt = 0;
			hptab.d_actf = dp->b_forw;
			dp->d_active = 0;
			dp->d_errcnt = 0;
			dp->d_actf = bp->av_forw;
			bp->b_resid = (-rp->hpwc)<<1;
			iodone(bp);
			rp->hpcs1 = IE|RELEASE|GO;
			if (dp->d_actf)
				hpustart(unit);
		}
		as =& ~(1<<unit);
	} else {
		if (as==0)
			rp->hpcs1 = IE;
		rp->hpcs1.hibyte = TRE>>8;
	}
	for (unit=0; as; unit++)
		if (as&(1<<unit)) {
			as =& ~(1<<unit);
			hpustart(unit);
		}
	hpstart();
}

hpread(dev)
{
	if (physck(hp_sizes[dev&07].nblocks, B_READ))
		physio(hpstrategy, &hpbuf, dev, B_READ);
}

hpwrite(dev)
{
	if (physck(hp_sizes[dev&07].nblocks, B_WRITE))
		physio(hpstrategy, &hpbuf, dev, B_WRITE);
}
