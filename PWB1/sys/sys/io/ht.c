#
/*
 * TJU16, TWU16 driver
 * Handles one TM02 controller, up to 4 TU16 slave transports
 * minor device classes:
 * bits 0,1: slave select
 * bit 2 off: rewind on close; on: position after first TM
 * bit 3 off: 800 bpi; on: 1600 bpi
 */

#include "../hd/param.h"
#include "../hd/buf.h"
#include "../hd/user.h"

#define NUNIT 4

struct {
	int	htcs1, htwc, htba, htfc;
	int	htcs2, htds, hter, htas;
	int	htck, htdb, htmr, htdt;
	int	htsn, httc, htbae, htcs3;
};

struct	devtab	httab;
struct	buf	rhtbuf, chtbuf;

char	h_openf[NUNIT];
int	h_den[NUNIT];
char	*h_blkno[NUNIT], *h_nxrec[NUNIT];

int	ht_addr;

#define	GO	01
#define	NOP	0
#define	WEOF	026
#define	SFORW	030
#define	SREV	032
#define	ERASE	024
#define	REW	06
#define	DCLR	010
#define P800	01300		/* 800 + pdp11 mode */
#define	P1600	02300		/* 1600 + pdp11 mode */
#define	IENABLE	0100
#define	RDY	0200
#define	TM	04
#define	DRY	0200
#define EOT	02000
#define CS	02000
#define COR	0100000
#define PES	040
#define WRL	04000
#define MOL	010000
#define ERR	040000
#define FCE	01000
#define	TRE	040000
#define HARD	064023	/* UNS|OPI|NEF|FMT|RMR|ILR|ILF */

#define	SIO	1
#define	SSFOR	2
#define SSREV	3
#define SRETRY	4
#define SCOM	5
#define SOK	6

htopen(dev, flag)
{
	register unit, ds;

	unit = dev&03;
	if (unit >= NUNIT || h_openf[unit]) {
		u.u_error = ENXIO;
		return;
	}
	h_den[unit] = (dev&010 ? P1600 : P800)|unit;
	h_blkno[unit] = 0;
	h_nxrec[unit] = -1;
	ds = hcommand(unit, NOP);
	if ((ds&MOL)==0 || (flag && (ds&WRL)))
		u.u_error = ENXIO;
	if (u.u_error==0)
		h_openf[unit]++;
}

htclose(dev, flag)
{
	register unit;
	register struct buf *bp;
	register struct devtab *dp;

	unit = dev&03;
	if (flag) {
		hcommand(unit, WEOF);
		hcommand(unit, WEOF);
	}
	if (dev&04) {
		if (flag)
			hcommand(unit, SREV); else
			{
				hcommand(unit, NOP);
				if (h_blkno[unit] < h_nxrec[unit])
					hcommand(unit, SFORW);
			}
	} else
		hcommand(unit, REW);
	dp = &httab;
	for (bp=dp->b_forw; bp!=dp; bp=bp->b_forw)
		if ((bp->b_dev&03)==unit)
			bp->b_dev =| 0100;
	h_openf[unit] = 0;
}

hcommand(unit, com)
{
	register struct buf *bp;
	register *rp;

	bp = &chtbuf;
	rp = ht_addr;
	spl5();
	while(bp->b_flags&B_BUSY) {
		bp->b_flags =| B_WANTED;
		sleep(bp, PRIBIO);
	}
	spl0();
	bp->b_dev = unit;
	bp->b_resid = com;
	bp->b_blkno = 0;
	bp->b_flags = B_BUSY|B_READ;
	htstrategy(bp);
	iowait(bp);
	if(bp->b_flags&B_WANTED)
		wakeup(bp);
	bp->b_flags = 0;
	return(bp->b_resid);
}

htstrategy(bp)
register struct buf *bp;
{
	register char **p;

	if (bp != &chtbuf) {
		p = &h_nxrec[bp->b_dev&03];
		if (bp->b_blkno > *p) {
			bp->b_flags =| B_ERROR;
			bp->b_error = ENXIO;
			iodone(bp);
			return;
		}
		if (bp->b_blkno == *p && bp->b_flags&B_READ) {
			bp->b_resid = 512;
			clrbuf(bp);
			iodone(bp);
			return;
		}
		if ((bp->b_flags&B_READ)==0)
			*p = bp->b_blkno + 1;
	}
	bp->av_forw = 0;
	spl5();
	if (httab.d_actf==0)
		httab.d_actf = bp;
	else
		httab.d_actl->av_forw = bp;
	httab.d_actl = bp;
	if (httab.d_active==0)
		htstart();
	spl0();
}

htstart()
{
	register struct buf *bp;
	register int unit;
	register *rp;
	char *blkno;

	rp = ht_addr;
    loop:
	if ((bp = httab.d_actf) == 0)
		return;
	unit = bp->b_dev&03;
	rp->htcs2 = 0;
	if((rp->httc&03777)!=h_den[unit])
		rp->httc = h_den[unit];
	blkno = h_blkno[unit];
	if (bp == &chtbuf) {
		if (bp->b_resid==NOP) {
			bp->b_resid = rp->htds;
			goto next;
		}
		httab.d_active = SCOM;
		rp->htfc = 0;
		rp->htcs1 = bp->b_resid|IENABLE|GO;
		return;
	}
	if (h_openf[unit] < 0 || bp->b_blkno > h_nxrec[unit])
		goto abort;
	if (blkno == bp->b_blkno) {
		httab.d_active = SIO;
		rhstart(bp, &rp->htfc, bp->b_wcount<<1, &rp->htbae);
	} else {
		if (blkno < bp->b_blkno) {
			httab.d_active = SSFOR;
			rp->htfc = blkno - bp->b_blkno;
			rp->htcs1 = SFORW|IENABLE|GO;
		} else {
			httab.d_active = SSREV;
			rp->htfc = bp->b_blkno - blkno;
			rp->htcs1 = SREV|IENABLE|GO;
		}
	}
	return;
    abort:
	bp->b_flags =| B_ERROR;
    next:
	httab.d_actf = bp->av_forw;
	iodone(bp);
	goto loop;
}

htintr()
{
	register struct buf *bp;
	register int unit;
	register *rp;
	int	err, state;

	if ((bp = httab.d_actf)==0)
		return;
	rp = ht_addr;
	unit = bp->b_dev&03;
	state = httab.d_active;
	httab.d_active = 0;
	if (rp->htcs1&TRE) {
		err = rp->hter;
		if (rp->htcs2.hibyte>0 || err&HARD)
			state = 0;
		if (bp == &rhtbuf)
			err =& ~FCE;
		if ((bp->b_flags&B_READ) && (rp->htds&PES))
			err =& ~(CS|COR);
		if (rp->htds&EOT || (rp->htds&MOL)==0)
			h_openf[unit] = -1;
		else if (rp->htds&TM) {
			rp->htwc = bp->b_wcount;
			h_nxrec[unit] = bp->b_blkno;
			state = SOK;
		}
		else if (state && err == 0)
			state = SOK;
		if (httab.d_errcnt > 4)
			deverror(bp, rp->hter, rp->htcs2);
		rp->htcs1 = TRE|DCLR|GO;
		if (state==SIO && ++httab.d_errcnt < 10) {
			httab.d_active = SRETRY;
			h_blkno[unit]++;
			rp->htfc = -1;
			rp->htcs1 = SREV|IENABLE|GO;
			return;
		}
		if (state!=SOK) {
			bp->b_flags =| B_ERROR;
			state = SIO;
		}
	} else if (rp->htcs1 < 0) {	/* SC */
		if (rp->htds&ERR)
			rp->htcs1 = DCLR|GO;
	}
	switch(state) {
	case SIO:
	case SOK:
		h_blkno[unit]++;
	case SCOM:
		httab.d_errcnt = 0;
		httab.d_actf = bp->av_forw;
		iodone(bp);
		bp->b_resid = (-rp->htwc)<<1;
		break;
	case SRETRY:
		if((bp->b_flags&B_READ)==0) {
			httab.d_active = SSFOR;
			rp->htcs1 = ERASE|IENABLE|GO;
			return;
		}
	case SSFOR:
	case SSREV:
		if (rp->htds&TM) {
			h_nxrec[unit] = bp->b_blkno+(state==SSREV?
				-rp->htfc:rp->htfc-1);
			h_blkno[unit] = bp->b_blkno+(state==SSREV?
				-rp->htfc:rp->htfc);
		} else
			h_blkno[unit] = bp->b_blkno;
		break;
	default:
		return;
	}
	htstart();
}

htread(dev)
{
	htphys(dev);
	physio(htstrategy, &rhtbuf, dev, B_READ);
}

htwrite(dev)
{
	htphys(dev);
	physio(htstrategy, &rhtbuf, dev, B_WRITE);
}

htphys(dev)
{
	register unit, a;

	unit = dev&03;
	a = lshift(u.u_offset, -9);
	h_blkno[unit] = a;
	h_nxrec[unit] = ++a;
}
