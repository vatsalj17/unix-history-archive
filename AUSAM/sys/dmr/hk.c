#
/*
 */ 

/*
 * RK06/07 disk driver
 *
 * This driver has been tested on a working RK06 for a short time.
 * It does not attempt ECC error correction and is probably
 * deficient in general in the case of errors and when packs
 * are dismounted. Code for multiple drives has not been tested.
 *
 *					ian johnstone
 *					AGSM 1978
 */ 

#include "../param.h"
#include "../buf.h"
#include "../conf.h"
#include "../user.h"

struct
{
	int hkcs1;	/* Control and Status register 1 */ 
	int hkwc;	/* Word count register */ 
	int hkba;	/* UNIBUS address register */ 
	int hkda;	/* Desired address register */ 
	int hkcs2;	/* Control and Status register 2*/ 
	int hkds;	/* Drive Status */ 
	int hker;	/* Error register */ 
	int hkas;	/* Attention Summary */ 
	int hkdc;	/* Desired cylinder */ 
	int hk00;	/* Unused */ 
	int hkdb;	/* Data buffer */ 
	int hkmr1;	/* Maintenance register 1 */ 
	int hkecps;	/* ECC position register */ 
	int hkecpt;	/* ECC pattern register */ 
	int hkmr2;	/* Maintenance register 2 */ 
	int hkmr3;	/* Maintenance register 3 */ 
};

#define	HKADDR	0177440
#define	NHK	8

#define	HKSIZE	27126

struct devtab hktab;
struct buf hkbuf;

char hk_openf;

#define	GO	01	/* Drive Commands */
#define	ACK	02
#define	RECAL	012
#define RCLR	04
#define	WCOM	022
#define	RCOM	020

#define	IENABLE	0100	/* hkcs1 - interrupt enable */
#define	READY	0200	/* hkds - drive ready */
#define	PIP	020000	/* hkds - Positioning Operation in Progress */
#define	ERR	0100000	/* hkcs1 - composite error */

#define	DU	040000	/* hker - Drive Unsafe	*/
#define	DTE	010000  /* hker - Drive Timing Error	*/
#define	OPI	020000  /* hker - Operation Incomplete	*/
			/* Error Correction Code errors */ 
#define DCK	0100000	/* hker - Data Check error */
#define ECH	0100    /* hker - ECC hard error */

#define CLR	040	/* hkcs2 - Controller Clear */

/*
 * Use av_back to save track+sector,
 * b_resid for cylinder.
 */ 

#define	trksec	av_back
#define	cylin	b_resid

hkopen(dev)
{
	register int dbit;

	dbit = dev.d_minor&0377;
	if(dbit >= NHK)
	{
		u.u_error = ENXIO;
		return;
	}
	dbit = 1 << dbit;

	if((hk_openf & dbit) == 0)
	{
		if(hk_openf == 0)
			HKADDR->hkcs2 = CLR;
		hk_openf =| dbit;
		HKADDR->hkcs2 = dev.d_minor;
		HKADDR->hkcs1 = ACK|GO;
		while(HKADDR->hkcs1&READY == 0);
		HKADDR->hkcs2 = dev.d_minor;
		HKADDR->hkcs1 = RCLR|GO;
	}
}

hkstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register char *p1, *p2;

	bp = abp;
	if(bp->b_dev.d_minor >= NHK || bp->b_blkno >= HKSIZE)
	{
		bp->b_flags =| B_ERROR;
		iodone(bp);
		return;
	}
	bp->av_forw = 0;
	p1 = bp->b_blkno;
	p2 = lrem(p1, 22);
	p1 = ldiv(p1, 22);
	bp->trksec = (p1%3)<<8|p2;
	bp->cylin = p1/3;
	spl5();
	if((p1 = hktab.d_actf) == 0)
		hktab.d_actf = bp;
	else
	{
		for(; p2 = p1->av_forw; p1 = p2)
		{
			if(p1->cylin <= bp->cylin && bp->cylin < p2->cylin || p1->cylin >= bp->cylin && bp->cylin >p2-> cylin)
				break;
		}
		bp->av_forw = p2;
		p1->av_forw = bp;
	}
	if(hktab.d_active == 0)
		hkstart();
	spl0();
}

hkstart()
{
	register struct buf *bp;

	if((bp = hktab.d_actf) == 0)
		return;
	hktab.d_active++;
	HKADDR->hkcs2 = bp->b_dev.d_minor;	/* select drive */
	HKADDR->hkdc = bp->cylin;		/* desired cylinder */
	HKADDR->hkda = bp->trksec;		/* track & sector */
	HKADDR->hkba = bp->b_addr;		/* buffer address */
	HKADDR->hkwc = bp->b_wcount;		/* word count */
	HKADDR->hkcs1 = IENABLE | GO |
		((bp->b_xmem & 03) << 8) |
		(bp->b_flags&B_READ ? RCOM : WCOM);
}

hkintr()
{
	register struct buf *bp;

	if(hktab.d_active == 0)
		return;
	bp = hktab.d_actf;
	hktab.d_active = 0;
	if(HKADDR->hkcs1&ERR)
	{	/* error bit */ 
		printf("HKERR er=%o cs1=%o cs2=%o ds=%o da=%o dc=%o\n",
			HKADDR->hker,HKADDR->hkcs1,HKADDR->hkcs2,HKADDR->hkds,HKADDR->hkda,HKADDR->hkdc);
		if(HKADDR->hker&(DU|DTE|OPI))
		{
			HKADDR->hkcs1 = ERR;	/* reset error */
			HKADDR->hkcs2 = bp->b_dev.d_minor;
			HKADDR->hkcs1 = RCLR|GO;
			while(HKADDR->hkcs1&READY == 0);
			HKADDR->hkcs2 = bp->b_dev.d_minor;
			HKADDR->hkcs1 = RECAL|GO;
			while(HKADDR->hkcs1&READY == 0);
		}
		HKADDR->hkcs1 = ERR;	/* reset error */
		HKADDR->hkcs2 = bp->b_dev.d_minor;
		HKADDR->hkcs1 = RCLR|GO;
		while(HKADDR->hkcs1&READY == 0);
		if(++hktab.d_errcnt <= 10)
		{
			hkstart();
			return;
		}
		bp->b_flags =| B_ERROR;
	}
	hktab.d_errcnt = 0;
	hktab.d_actf = bp->av_forw;
	bp->b_resid = HKADDR->hkwc;
	iodone(bp);
	hkstart();
}

hkread(dev)
{

	if(hkphys(dev))
		physio(hkstrategy, &hkbuf, dev, B_READ);
}

hkwrite(dev)
{

	if(hkphys(dev))
		physio(hkstrategy, &hkbuf, dev, B_WRITE);
}

hkphys(dev)
{
	register c;

	c = lshift(u.u_offset, -9);
	c =+ ldiv(u.u_count+511, 512);
	if(c > HKSIZE)
	{
		u.u_error = ENXIO;
		return(0);
	}
	return(1);
}
