#
/*
 * TM tape driver
 * minor device classes:
 * bits 0,1: slave select
 * bit 2 off: rewind on close; on: position after first TM
 */

#include "../hd/param.h"
#include "../hd/buf.h"
#include "../hd/user.h"

struct {
	int tmer, tmcs, tmbc, tmba;
	int tmdb, tmrd;
};

struct	devtab	tmtab;
struct	buf	rtmbuf;
char	t_openf[4], *t_blkno[4], *t_nxrec[4];

int	tm_addr;

#define	RCOM	03
#define	WCOM	05
#define	WEOF	07
#define	SFORW	011
#define	SREV	013
#define	WIRG	015
#define	REW	017
#define	DENS	060000		/* 9-channel */
#define	IENABLE	0100
#define GSD	010000
#define	HARD	0102200	/* ILC, EOT, NXM */
#define	EOF	0040000
#define	RLE	01000

#define	SIO	1
#define	SSFOR	2
#define	SSREV	3
#define SCOM	4

tmopen(dev, flag)
{
	register unit;

	unit = dev&03;
	if (t_openf[unit])
		u.u_error = ENXIO;
	else {
		t_openf[unit]++;
		t_blkno[unit] = 0;
		t_nxrec[unit] = 65535;
	}
}

tmclose(dev, flag)
{
	register unit;
	register struct buf *bp;
	register struct devtab *dp;

	unit = dev&03;
	if (flag) {
		tcommand(unit, WEOF);
		tcommand(unit, WEOF);
	}
	if (dev&04) {
		if (flag)
			tcommand(unit, SREV); else
			if (t_blkno[unit] < t_nxrec[unit])
				tcommand(unit, SFORW);
	} else
		tcommand(unit, REW);
	dp = &tmtab;
	for (bp=dp->b_forw; bp!=dp; bp=bp->b_forw)
		if ((bp->b_dev&03)==unit)
			bp->b_dev =| 0100;
	t_openf[unit] = 0;
}

tcommand(unit, com)
{
	register *rp;

	rp = tm_addr;
	spl5();
	while(tmtab.d_active)
		sleep(&tmtab, -1);
	tmtab.d_active = SCOM;
	rp->tmbc = 0;
	rp->tmcs = IENABLE|DENS|com | (unit<<8);
	spl0();
}

tmstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register char **p;

	bp = abp;
	p = &t_nxrec[bp->b_dev&03];
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
	bp->av_forw = 0;
	spl5();
	if (tmtab.d_actf==0)
		tmtab.d_actf = bp;
	else
		tmtab.d_actl->av_forw = bp;
	tmtab.d_actl = bp;
	if (tmtab.d_active==0)
		tmstart();
	spl0();
}

tmstart()
{
	register struct buf *bp;
	register com, *rp;
	int unit;
	char *blkno;

    loop:
	if ((bp = tmtab.d_actf) == 0) {
		tmtab.d_active = 0;
		wakeup(&tmtab);
		return;
	}
	unit = bp->b_dev&03;
	blkno = t_blkno[unit];
	if (t_openf[unit] < 0 || bp->b_blkno > t_nxrec[unit]) {
		bp->b_flags =| B_ERROR;
		tmtab.d_actf = bp->av_forw;
		iodone(bp);
		goto loop;
	}
	com = (unit<<8) | ((bp->b_xmem & 03) << 4) | IENABLE|DENS;
	rp = tm_addr;
	if (blkno != bp->b_blkno) {
		if (bp->b_blkno > blkno) {
			com =| SFORW;
			tmtab.d_active = SSFOR;
			rp->tmbc = blkno - bp->b_blkno;
		} else {
			com =| SREV;
			tmtab.d_active = SSREV;
			rp->tmbc = bp->b_blkno - blkno;
		}
		rp->tmcs = com;
		return;
	}
	tmtab.d_active = SIO;
	rp->tmbc = bp->b_wcount << 1;
	rp->tmba = bp->b_addr;		/* core address */
	rp->tmcs = com | ((bp->b_flags&B_READ)? RCOM:
	    ((tmtab.d_errcnt)? WIRG: WCOM));
}

tmintr()
{
	register struct buf *bp;
	register unit, *rp;
	int	state;

	if ((state = tmtab.d_active)==0)
		return;
	rp = tm_addr;
	if (state != SCOM) {
	bp = tmtab.d_actf;
	unit = bp->b_dev&03;
	if (rp->tmcs < 0) {		/* error bit */
		while(rp->tmrd & GSD) ; /* wait for gap shutdown */
		if ((rp->tmer&(HARD|EOF))==0 && state==SIO) {
			if (bp != &rtmbuf || (rp->tmer&RLE)==0)
			if (++tmtab.d_errcnt < 10) {
				t_blkno[unit]++;
				tmstart();
				return;
			} else
				bp->b_flags =| B_ERROR;
		} else
		if (rp->tmer&EOF) {
			if (state==SIO) {
				rp->tmbc = bp->b_wcount<<1;
				t_nxrec[unit] = bp->b_blkno;
			} else {
				t_nxrec[unit] = bp->b_blkno+(state==SSREV?
					-rp->tmbc:rp->tmbc-1);
				t_blkno[unit] = bp->b_blkno+(state==SSREV?
					-rp->tmbc:rp->tmbc);
				tmstart();
				return;
			}
		} else {
			bp->b_flags =| B_ERROR;
			if (bp != &rtmbuf)
				t_openf[unit] = -1;
		}
	}
	if (state == SIO) {
		tmtab.d_errcnt = 0;
		t_blkno[unit]++;
		tmtab.d_actf = bp->av_forw;
		bp->b_resid = -rp->tmbc;
		iodone(bp);
	} else
		t_blkno[unit] = bp->b_blkno;
	}
	tmstart();
}

tmread(dev)
{
	tmphys(dev);
	physio(tmstrategy, &rtmbuf, dev, B_READ);
}

tmwrite(dev)
{
	tmphys(dev);
	physio(tmstrategy, &rtmbuf, dev, B_WRITE);
}

tmphys(dev)
{
	register unit, a;

	unit = dev&03;
	a = lshift(u.u_offset, -9);
	t_blkno[unit] = a;
	t_nxrec[unit] = ++a;
}
