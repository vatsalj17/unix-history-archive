/*
 *  RL disk driver
 *
 *	Modified Oct 1979 to run under level 6.  Should be improved
 *	someday for overlapped seeks and head travel optimisation
 *	(e.g. Children's Museum RK driver).
 *
 *				Kevin Hill (DCS)
 */

#include "../defines.h"
#include "../param.h"
#include "../buf.h"
#include "../conf.h"

#define NRL 	2		/* number of RL drives */

#define	NRLBLK	10240		/* blocks per disk */
#define	RLCYLSZ	10240		/* bytes per cylinder */
#define	RLSECSZ	256		/* bytes per sector */
#define	RLTRKSZ	20		/* blocks per track */

#define	RESET	013		/* drive reset during Get Status */
#define	STAT	03		/* DAR value during Get Status */
#define	GETSTAT	04		/* Get Status command */
#define	VC	01000		/* Volume Check from MP */
#define	WCOM	012		/* Write Data command */
#define	RCOM	014		/* Read Data command */
#define	SEEK	06		/* Seek command */
#define	SEEKHI	5		/* seek to higher cylinder nr */
#define	SEEKLO	1		/* seek to lower cylinder nr */
#define	RDHDR	010		/* Read Header command */
#define	IENABLE	0100		/* Enable Interrupts */
#define	CRDY	0200		/* Controller Ready */
#define	OPI	02000		/* Operation Incomplete */
#define	CRCERR	04000		/* CRC error */
#define	TIMOUT	010000
#define	NXM	020000		/* Non-Existent Memory */
#define	DE	040000		/* Drive Error */

struct device
{
	int rlcs;		/* Control Status Register */
	int rlba;		/* Bus Address Register */
	int rlda;		/* Disk Address Register */
	int rlmp;		/* Multipurpose Register */
};

#define RLADDR	0174400		/* Unibus address */

#ifdef	RAW_BUFFER_POOL
#define	rlbuf	0
#else
struct	buf	rrlbuf;
#define	rlbuf	&rrlbuf
#endif
struct	devtab	rltab;

struct 
{
	int	cn[4];		/* location of heads for each drive */
	int	dn;		/* drive number */
	int	com;		/* read or write command word */
	int	chn;		/* cylinder and head number */
	unsigned wleft;		/* words left to be transferred */
	unsigned wpart;		/* number of words transferred */
	int	sn;		/* sector number */
	union
	{
		int	w[2];
		long	l;
	} addr;			/* address of memory for transfer */
}
	rl = { -1, -1,-1, -1 };	/* initialise head locations to unknown */
				/* Note: max. 4 drives per controller */

rlstrategy(bp)
register struct buf *bp;
{
	if (bp->b_blkno >= NRLBLK || bp->b_dev.d_minor >= NRL)
	{
		bp->b_flags =| B_ERROR;
		iodone(bp);
		return;
	}
	bp->av_forw = NULL;
	spl5();
	if (rltab.d_actf == NULL)
		rltab.d_actf = bp;
	else
		rltab.d_actl->av_forw = bp;
	rltab.d_actl = bp;
	if (rltab.d_active == NULL)
		rlstart();
	spl0();
}

rlstart()
{
	register struct buf *bp;

	if ((bp = rltab.d_actf) == NULL)
		return;
	rltab.d_active++;
	rl.dn = bp->b_dev.d_minor;
	rl.chn = bp->b_blkno/RLTRKSZ;
	rl.sn = (bp->b_blkno % RLTRKSZ) << 1;	/* 256 bytes per sector */
	rl.wleft = bp->b_wcount;
	rl.addr.w[0] = bp->b_xmem & 3;
	rl.addr.w[1] = bp->b_addr;
	rl.com = (rl.dn << 8) | IENABLE;
	if (bp->b_flags & B_READ)
		rl.com =| RCOM;
	else
		rl.com =| WCOM;
	rlio();
}

rlintr()
{
	register struct buf *bp;
	register struct device *rp;
	register int status;

	rp = RLADDR;
	if (rltab.d_active == NULL)
	{
/*
		logstray(rp);
*/
		return;
	}
	bp = rltab.d_actf;
	if (rp->rlcs < 0)		/* error bit */
	{
		if (rp->rlcs & (OPI | CRCERR | TIMOUT | NXM))
		{
			if (rltab.d_errcnt > 2)
				printf("RL CS=%o BA=%o DA=%o MP=%o\n",
					rp->rlcs, rp->rlba, rp->rlda, rp->rlmp);
		}
		if (rp->rlcs & DE)
		{
			rp->rlda = STAT;
			rp->rlcs = (rl.dn << 8) | GETSTAT;
			while ((rp->rlcs & CRDY) == 0);
			status = rp->rlmp;
			if (rltab.d_errcnt > 2)
				printf("RL CS=%o BA=%o DA=%o MP=%o\n",
					rp->rlcs, rp->rlba, rp->rlda, rp->rlmp);
			rp->rlda = RESET;
			rp->rlcs = (rl.dn << 8) | GETSTAT;
			while ((rp->rlcs & CRDY) == 0);
			if (status & VC)
			{
				rlstart();
				return;
			}
		}
		if (++rltab.d_errcnt <= 10)
		{
			rl.cn[rl.dn] = -1;
			rlstart();
			return;
		}
		bp->b_flags =| B_ERROR;
		rl.wpart = rl.wleft;
	}

	if ((rl.wleft =- rl.wpart) > 0)
	{
		rl.addr.l =+ (rl.wpart << 1);
		rl.sn = 0;
		rl.chn++;
		rlio();
		return;
	}
	rltab.d_active = NULL;
	rltab.d_errcnt = 0;
	rltab.d_actf = bp->av_forw;
	iodone(bp);
	rlstart();
}

rlio()
{

	register struct device *rp;
	register dif;
	register int head;

	rp = RLADDR;
	head = rl.wpart>>5;
	if (rl.cn[rl.dn] < 0)
	{
		rp->rlcs = (rl.dn << 8) | RDHDR;
		while ((rp->rlcs & CRDY) == 0);
		rl.cn[rl.dn] = (rp->rlmp & 077700) >> 6;
	}
	dif =(rl.cn[rl.dn] >> 1) - (rl.chn >>1);
	head = (rl.chn & 1) << 4;
	if (dif < 0)
		rp->rlda = (-dif <<7) | SEEKHI | head;
	else
		rp->rlda = (dif << 7) | SEEKLO | head;
	rp->rlcs = (rl.dn << 8) | SEEK;
	rl.cn[rl.dn] = rl.chn;
	if (rl.wleft < (rl.wpart = RLCYLSZ - (rl.sn * RLSECSZ)))
		rl.wpart = rl.wleft;
	while ((rp->rlcs&CRDY) == 0);
	rp->rlda = (rl.chn << 6) | rl.sn;
	rp->rlba = rl.addr.w[1];
	rp->rlmp = -rl.wpart;
	rp->rlcs = rl.com | rl.addr.w[0] << 4;
}

rlread(dev)
{
	physio(rlstrategy, rlbuf, dev, B_READ);
}

rlwrite(dev)
{
	physio(rlstrategy, rlbuf, dev, B_WRITE);
}
