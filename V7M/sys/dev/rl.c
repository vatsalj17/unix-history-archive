/*
 * UNIX/v7m RL01/2 disk driver
 *
 * This driver is based on the standard v7 RL driver.
 *
 * 7/80 - Modified by Armando P. Stettner to support both
 *	RL01 and RL02 drives on the same controller.
 *
 * 5/81 - Modified by Fred Canter to improve drive type
 *	identification and disk block number overflow checking.
 *
 * 9/81 - Modified by Fred Canter to include the new
 *	iostat code.
 *
 * Fred Canter 9/4/81
 */

#include "../h/param.h"
#include "../h/buf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/systm.h"

#define	BLKRL1	10240		/* Number of UNIX blocks for an RL01 drive */
#define BLKRL2	20480		/* Number of UNIX blocks for an RL02 drive */
#define RLCYLSZ 10240		/* bytes per cylinder */
#define RLSECSZ 256		/* bytes per sector */

#define RESET 013
#define	RL02TYP	0200	/* drive type bit */
#define STAT 03
#define GETSTAT 04
#define WCOM 012
#define RCOM 014
#define SEEK 06
#define SEEKHI 5
#define SEEKLO 1
#define RDHDR 010
#define IENABLE 0100
#define CRDY 0200
#define OPI 02000
#define CRCERR 04000
#define TIMOUT 010000
#define NXM 020000
#define DE  040000

struct device
{
	int	rlcs;	/* RL control & status register */
	int	rlba;	/* RL bus addess register */
	int	rlda;	/* RL disk address register */
	int	rlmp;	/* RL multipurpose register */
};

#define RLADDR	((struct device *)0174400)
#define	NRL	4	/* maximum number of RL drives, DO NOT CHANGE ! */
/*
 * Instrumentation (iostat) structures
 */

#define	DK_N	5
#define	DK_T	9	/* RL xfer rate indicator */
struct	ios	rl_ios[NRL];
struct	buf	rrlbuf;
struct	buf	rltab;

struct 
{
	int	cn[4];		/* location of heads for each drive */
	int	type[4];	/* RL disk drive type indicator */
				/* -1 = non existent drive */
				/* BLKRL1 = RL01 */
				/* BLKRL2 = RL02 */
	char	openf;		/* RL open flag, 1 = open not completed */
	int	dn;		/* drive number */
	int	com;		/* read or write command word */
	int	chn;		/* cylinder and head number */
	unsigned int	bleft;	/* bytes left to be transferred */
	unsigned int	bpart;	/* number of bytes transferred */
	int	sn;		/* sector number */
	union {
		int	w[2];
		long	l;
	} addr;			/* address of memory for transfer */

/*
 * Initialize the RL structure as follows:
 *
 * cn[] = all heads, location unknown
 * type[] = all drives, non existent
 * openf = RL open not completed yet
 */

}	rl = {-1,-1,-1,-1, -1,-1,-1,-1, 1} ;

rlstrategy(bp)
register struct buf *bp;
{
	register struct device *rp ;
	register int ctr;
	int dn, nblks;
	long sz;

	rp = RLADDR ;

	if(bp->b_flags&B_PHYS)
		mapalloc(bp);

/*
 * If the RL open has not been completed,
 * then get the status of all possible drives
 * as follows:
 *
 * 1.	Execute a get status with reset up to 8 times
 *	to get a valid status from the drive.
 *
 * 2.	If an OPI error is detected then the drive
 *	is non-existent (NED).
 *
 * 3.	If a vaild status cannot be obtained after 8 attempts,
 *	then print the "can't get status" message and
 *	mark the drive non-existent.
 *
 * 4.	If a valid status is obtained, then use the drive
 *	type to set rl.type to the number of blocks for that
 *	type drive.
 *	10240 for RL01 or 20480 for RL02
 *
 * The above sequence only occurs on the first access
 * of the RL disk driver. The drive status obtained is
 * retained until the system in rebooted.
 * Packs may be mounted and dismounted,
 * HOWEVER the disk unit number select plugs may
 * NOT be changed without rebooting the system.
 *
 ****************************************************************
 *								*
 * For some unknown reason the RL02 does not return a valid	*
 * status the first time a GET STATUS request is issued for	*
 * the drive, in fact it can take up to three or more		*
 * GET STATUS requests to obtain a valid drive status.		*
 * This is why the GET STATUS is repeated eight times		*
 * in step one above.						*
 *								*
 ****************************************************************
 */

	if(rl.openf)
		{
		rl.openf = 0;
		nblks = 0;
		for (dn=0; dn<NRL; dn++)
			{
			for(ctr=0; ctr<8; ctr++)
				{
				rp->rlda = RESET;
				rp->rlcs = (dn << 8) | GETSTAT;
				while ((rp->rlcs & CRDY) == 0) ;
				if(rp->rlcs & OPI)
					break;	/* NED */
				if((rp->rlmp & 0157400) == 0)
					break;	/* valid status */
				}
			if(rp->rlcs & OPI)
				continue;	/* NED */
			if(ctr >= 8)
				{
			printf("\nCan't get status of RL unit %d\n", dn);
				continue;
				}
			if(rp->rlmp & RL02TYP)
				rl.type[dn] = BLKRL2;	/* RL02 */
			else
				rl.type[dn] = BLKRL1;	/* RL01 */
			rl_ios[dn].dk_tr = DK_T;
			nblks++;
			}
		ctr = spl6();
		dk_iop[DK_N] = &rl_ios[0];
		dk_nd[DK_N] = nblks;
		splx(ctr);
		}
	dn = minor(bp->b_dev);
	if((dn >= NRL) || (rl.type[dn] < 0))
		goto bad;
	nblks = rl.type[dn];	/* get # of blocks on this drive type */
	sz = bp->b_bcount;
	sz = (sz+511) >> 9;
	if(bp->b_blkno < 0)
		goto bad;
	if((bp->b_blkno + sz) > nblks)
		goto bad;
	if(bp->b_blkno >= nblks) {
		if(bp->b_blkno == nblks && bp->b_flags&B_READ)
			bp->b_resid = bp->b_bcount;
		else {
	bad:
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
		}
		iodone(bp);
		return;
	}
	bp->av_forw = NULL;
	spl5();
	if(rltab.b_actf == NULL)
		rltab.b_actf = bp;
	else
		rltab.b_actl->av_forw = bp;
	rltab.b_actl = bp;
	if(rltab.b_active == NULL)
		rlstart();
	spl0();
}

rlstart()
{

	register struct buf *bp;

	if ((bp = rltab.b_actf) == NULL)
		return;
	rltab.b_active++;
	rl.dn = minor(bp->b_dev);
	rl.chn = bp->b_blkno/20;
	rl.sn = (bp->b_blkno%20) << 1;
	rl.bleft = bp->b_bcount;
	rl.addr.w[0] = bp->b_xmem & 3;
	rl.addr.w[1] = (int)bp->b_un.b_addr;
	rl.com = (rl.dn << 8) | IENABLE;
	if (bp->b_flags & B_READ)
		rl.com |= RCOM;
	else
		rl.com |= WCOM;
	rl_ios[rl.dn].dk_busy++;
	rl_ios[rl.dn].dk_numb++;
	rl_ios[rl.dn].dk_wds += (bp->b_bcount >> 6);
	rlio();
}

rlintr()
{
	register struct buf *bp;
	register struct device *rp;
	register int status;

	rp = RLADDR;
	if (rltab.b_active == NULL)
		return;
	bp = rltab.b_actf;
	rl_ios[rl.dn].dk_busy = 0;
	if (rp->rlcs < 0) {		/* error bit */
		if (rp->rlcs & 036000) {
			if(rltab.b_errcnt > 2)
				{
				printf("rlcs, rlda  ") ;
				deverror(bp, rp->rlcs, rp->rlda);
				}
		}
		if (rp->rlcs & 040000) {
			rp->rlda = STAT;
			rp->rlcs = (rl.dn << 8) | GETSTAT;
			while ((rp->rlcs & CRDY) == 0)
				;
			status = rp->rlmp;
			if(rltab.b_errcnt > 2)
				{
				printf("rlmp, rlda  ") ;
				deverror(bp, status, rp->rlda);
				}
			rp->rlda = RESET;
			rp->rlcs = (rl.dn << 8) | GETSTAT;
			while ((rp->rlcs & CRDY) == 0)
				;
			if(status & 01000) {
				rlstart();
				return;
			}
		}
		if (++rltab.b_errcnt <= 10) {
			rl.cn[rl.dn] = -1;
			rlstart();
			return;
		}
		else {
			bp->b_flags |= B_ERROR;
			rl.bpart = rl.bleft;
		}
	}

	if ((rl.bleft -= rl.bpart) > 0) {
		rl.addr.l += rl.bpart;
		rl.sn=0;
		rl.chn++;
		rlio();
		return;
	}
	rltab.b_active = NULL;
	rltab.b_errcnt = 0;
	rltab.b_actf = bp->av_forw;
	bp->b_resid = 0;
	iodone(bp);
	rlstart();
}

rlio()
{

	register struct device *rp;
	register dif;
	register int head;

	rp = RLADDR;
	/*
	 * When the device is first touched, find where the heads are and
	 * what type of drive it is.
	 * rl.cn[] is initialized as -1 and this indicates that we have not touched 
	 * this drive before.
	 */
	if (rl.cn[rl.dn] < 0) {
		/* find where the heads are */
		rp->rlcs = (rl.dn << 8) | RDHDR;
		while ((rp->rlcs&CRDY) == 0)
			;
		rl.cn[rl.dn] = (rp->rlmp >> 6) & 01777;
	}
	dif =(rl.cn[rl.dn] >> 1) - (rl.chn >>1);
	head = (rl.chn & 1) << 4;
	if (dif < 0)
		rp->rlda = (-dif <<7) | SEEKHI | head;
	else
		rp->rlda = (dif << 7) | SEEKLO | head;
	rp->rlcs = (rl.dn << 8) | SEEK;
	rl.cn[rl.dn] = rl.chn;
	if (rl.bleft < (rl.bpart = RLCYLSZ - (rl.sn * RLSECSZ)))
		rl.bpart = rl.bleft;
	while ((rp->rlcs&CRDY) == 0)
		;
	rp->rlda = (rl.chn << 6) | rl.sn;
	rp->rlba = rl.addr.w[1];
	rp->rlmp = -(rl.bpart >> 1);
	rp->rlcs = rl.com | rl.addr.w[0] << 4;
}

rlread(dev)
{

	physio(rlstrategy, &rrlbuf, dev, B_READ);
}

rlwrite(dev)
{

	physio(rlstrategy, &rrlbuf, dev, B_WRITE);
}
