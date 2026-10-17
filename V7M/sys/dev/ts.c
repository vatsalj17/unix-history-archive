/*
 * UNIX/v7m TS11 - 1600 BPI tape driver
 *
 * Fred Canter 5/5/81
 *
 * Thanks to Jerry Brenner for most of this driver.
 *
 *******************************************************************
 *								   *
 * The TM11 and TS11 share the standard address (0172520) and      *
 * the standard vector (0224). This driver allows for the          *
 * coexistence of the TM11 and TS11 on the same system, by using   *
 * the minor device number to modify the TS11 CSR address.         *
 * The minor device number is added to the base address of 0172520 *
 * to form the actual TS11 address. If the TS11 is the only tape   *
 * on the system, it's hardware address is set to 0172520 and the  *
 * minor device number is zero. If the system also has a TM11 tape *
 * tape, the TS11 hardware address is set to 0172550 and the minor *
 * device number is set to six. The minor device number is set by  *
 * the makefile in /dev, i.e., "make ts0" for the TS11 at 0172520  *
 * or "make ts6" for the TS11 at 0172550.                          *
 * The same technigue is usde to boot the TS11 using the M9312     *
 * hardware bootstrap, i.e., "MS0" for TS11 at 0172520 and "MS6"   *
 * for TS11 at 0172550.      					   *
 *								   *
 *******************************************************************
 */


#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/file.h"
#include "../h/user.h"

int cmdpkt[5];	      /* command packet. extra location is mod 4 alignment */

struct	device
{
	int	tsdb;		/* TS data buffer and bus address reg */
	int	tssr;		/* TS status register */
};

struct	mespkt
{
	int	mshdr;		/* message packet header word */
	int	mssiz;		/* size of message returned */
	int	msresid;	/* remaining count register */
	int	mstsx0;		/* extended status reg 0 */
	int	mstsx1;		/* extended status reg 1 */
	int	mstsx2;		/* extneded status reg 2 */
	int	mstsx3;		/* extended status reg 3 */
};

struct compkt
{
	int	tscom;		/* command packet command word */
	int	tsba;		/* memory address */
	int	tsbae;		/* extended address */
	int	tswc;		/* byte count, record count, etc. */
};

struct	chrdat
{
	int	msbptr;		/* pointer to message buffer */
	int	msbae;		/* hi address (bits 16 & 17) */
	int	msbsiz;		/* size of message buffer */
	int	mschar;		/* characteristics word */
};

struct	chrdat	chrbuf;		/* characteristics buffer */
struct	mespkt	mesbuf;		/* message buffer */
struct	buf	tstab;
struct	buf	ctsbuf;
struct	buf	rtsbuf;


char	ts_flags;
int	ts_openf;
int	ts_stray;
daddr_t ts_blkno;
daddr_t ts_nxrec;

/*
 * DO NOT !!!!, change the following TS11 address assignment
 * without first reading the comments at the head of this driver.
 * If the TS11 is at address 0172520 or 0172550, then the
 * TSADDR definition should NOT be changed.
 * If the TS11 is at some other address, then set the TSADDR
 * to that address and make sure that the minor device
 * number is set to zero (/dev/makefile "make ts0").
 */
#define	TSADDR	((struct device *)0172520)

/*	The following 3 lines are necessary to align com buffer on mod 4 */
#define	t_temp	(&cmdpkt[1])	/* get address of cmdpkt +2 */
#define	t_temp1	(t_temp & 0177776)
#define	combuf	((struct compkt *)t_temp1)


	/* bit definitions for command word in ts command packet */

#define	ACK	0100000		/* acknowledge bit */
#define	CVC	040000		/* clear volume check */
#define	OPP	020000		/* opposite. reverse recovery */
#define	SWB	010000		/* swap bytes. for data xfer */
	/* bit definitions for Command mode field during read command */
#define	RNEXT	0		/* read next (forward) */
#define	RPREV	0400		/* read previous (reverse) */
#define	RRPRV	01000		/* reread previous (space rev, read two) */
#define	RRNXT	01400		/* reread next (space fwd, read rev) */
	/* bit definitions for Command mode field during write command */
#define	WNEXT	0		/* Write data next */
#define	WDRTY	01000		/* write data retry , space rev, erase, write data) */
	/* bit definitions for command mode field during position command */
#define	SPCFWD	0		/* space records forward */
#define	SPCREV	0400		/* space records reverse */
#define	SKTPF	01000		/* skip tape marks forward */
#define	SKTPR	01400		/* skip tape marks reverse */
#define	RWIND	02000		/* rewind */

	/* bit definitions for command mode field during format command */
#define	WEOF	0		/* write tape mark */
#define	ERAS	0400		/* erase */
#define	WEOFE	01000		/* write tape mark entry */
	/* bit definitions for command mode field during control command */
#define	MBREAL	0		/* message buffer release */
#define	REWUNL	0400		/* Rewind and unload */
#define	CLEAN	01000		/* clean */
	/* additional definitions */
#define	IEI	0200		/* interrupt enable bit */

	/* command code definitions */
#define	NOP	0
#define	RCOM	01		/* read command */
#define	WCHAR	04		/* write characteristics */
#define	WCOM	05		/* write */
#define	WSUSM	06		/* write subsystem memory */
#define	POSIT	010		/* position command */
#define	FORMT	011		/* Format command */
#define	CONTRL	012		/* Control command */
#define	INIT	013		/* initialize */
#define	GSTAT	017		/* get status immediate */


	/* definition of tssr bits */
#define	SC	0100000		/* special condition */
#define	UPE	040000		/* unibus parity error */
#define	SPE	020000		/* Serial bus parity error */
#define	RMR	010000		/* register modify refused */
#define	NXM	04000		/* non-existent memory */
#define	NBA	02000		/* Need Buffer address */
#define	SSR	0200		/* Sub-System ready */
#define	OFL	0100		/* off-line */

	/* fatal termination class codes */
#define	FTC	030		/* use this as a mask to get codes */
	/* code = 00	see error code byte in tsx3 */
	/* code = 01	I/O seq Crom or main Crom parity error */
	/* code = 10	u-processor Crom parity error,I/O silo parity */
	/*		serial bus parity, or other fatal */
	/* code = 11	A/C low. drive ac low */

	/* termination class codes */
#define	TCC	016		/* mask for termination class codes */
	/* code = 000	normal termination */
	/* code = 001	Attention condition 
	/* code = 010	Tape status alert 
	/* code = 011	Function reject 
	/* code = 100	Recoverable error - tape pos = 1 record down from
			start of function 
	/* code = 101	Recoverable error - tape has not moved
	/* code = 110	Unrecoverable error - tape position lost
	/* code = 111	Fatal controller error - see fatal class bits */

	/* definition of message buffer header word */
#define	MAKC	0100000		/* acknowledge from controller */
#define	MCCF	07400		/* mask for class code field */
	/* class codes are */
	/*	0 = ATTN, on or ofline
		1 = ATTN, microdiagnostic failure
		0 = FAIL, serial bus parity error
		1 = FAIL, WRT LOCK
		2 = FAIL, interlock or non-executable function
		3 = FAIL, microdiagnostic error
	*/

#define	MMSC	037	/* mask for message code field */
	/* message codes are
		020	= end
		021	= FAIL
		022	= ERROR
		023	= Attention

	/* definition of extended status reg 0 bits */
#define	TMK	0100000		/* Tape mark detected */
#define	RLS	040000		/* Record length short */
#define	LET	020000		/* Logical end of Tape */
#define	RLL	010000		/* Record length long */
#define	WLE	04000		/* Write lock error */
#define	NEF	02000		/* Non-executable function */
#define	ILC	01000		/* Illegal command */
#define	ILA	0400		/* Illegal address */
#define	MOT	0200		/* Capistan is moving */
#define	ONL	0100		/* On Line */
#define	IE	040		/* state of interrupt enable bit */
#define	VCK	020		/* Volume Check */
#define	PED	010		/* Phase encoded drive */
#define	WLK	04		/* Write locked */
#define	BOT	02		/* Tape at bot */
#define	EOT	01		/* Tape at eot */

	/* definitions of xstat1 */
#define	DLT	0100000		/* Data late error */
#define	COR	020000		/* Correctable data error */
#define CRS	010000		/* Crease detected */
#define	TIG	04000		/* Trash in the gap */
#define	DBF	02000		/* Deskew Buffer Fail */
#define	SCK	01000		/* Speed check */
#define	IPR	0200		/* Invalid preamble */
#define	SYN	0100		/* Synch Failure */
#define	IPO	040		/* invalid postamble */
#define	IED	020		/* invalid end data */
#define	POS	010		/* postamble short */
#define	POL	04		/* postamble long */
#define	UNC	02		/* Uncorrectable data error */
#define	MTE	01		/* multi track error */

	/* Definitions of XSTAT2 bits */
#define	OPM	0100000		/* operation in progress (tape moving) */
#define	SIP	040000		/* Silo Parity error */
#define	BPE	020000		/* Serial bus Parity error at drive */
#define	CAF	010000		/* Capstan Acceleration fail */
#define	WCF	02000		/* Write card error */
#define	DTP	0477		/* mask for Dead track bits */

	/*	bit definitions for XSTAT3 */
#define	LMX	0200		/* Limit exceeded (tension arms) */
#define	OPI	0100		/* operation incomplete */
#define	REV	040		/* current operation in reverse */
#define	CRF	020		/* capstan response fail */
#define	DCK	010		/* density check */
#define	NOI	04		/* no tape mark or preamble */
#define	LXS	02		/* limit exceeded manual recovery */
#define	RIB	01		/* Reverse into BOT */


#define	SIO	01
#define SSEEK	02
#define	SINIT	04
#define	SRETRY	010
#define	SCOM	020
#define	SOK	040
#define SERROR	0100
#define SREWND	0200

#define	S_WRITTEN	1

tsopen(dev, flag)
{
	register int ds;
	register struct device *tsadr;

		/* if unit > 7 or if already open
			then flag an error and return */
	tsadr = TSADDR + (dev&07);
	if (((minor(dev) & 0177) > 7)  || ts_openf > 0)
	{
		u.u_error = ENXIO;
		return;
	}

	if(tstab.b_active & SREWND)
	{	/* this is a delay loop to allow rewind completion	*/
		/* it allows for 2400 feet of tape plus a fudge factor	*/
		/******** this loop assumes lbolt every 4 seconds *******/

		for(ds = 0; ds < 125; ds++)
		{
			if(tsadr->tssr & SSR)
			{
				ds = 0; 	/* if ready then break	*/
				break;
			}
			else
				sleep(&lbolt, PZERO +1);
		}
		if(ds)
		{
			u.u_error = ENXIO;	/* timeout. byebye	*/
			return;
		}
		else
			tstab.b_active = 0;	/* rewind complete	*/
	}

		/* call init. if init returns 1 then failure.
			flag error and return */
	if(tsinit(tsadr, 0))
	{
		u.u_error = ENXIO;
		return;
	}

		/* some house keeping here */
	ts_blkno = 0;			/* set current block # to 0 */
	ts_nxrec = 1000000;		/* set max accessable block # */
	ts_flags = 0;			/* clear status flags	*/
	tstab.b_flags |= B_TAPE;	/* Say we are a tape so no write
						ahead */
	ds = tscommand(dev, NOP);	     /* Get drive status     */

		/* if drive is off-line or open for write(read/write)
			and write locked then fatal error	 */
	if(tsadr->tssr & OFL || flag && (ds&WLK))
	{
	       printf("Magtape %o offline or write locked\n",minor(dev)&0177);
		u.u_error = ENXIO;
	}
	if(u.u_error == 0)	/* if no error then say opened	*/
		ts_openf = 1;
	else
		ts_openf = 0;
}
tsclose(dev, flag)
{
	if(tstab.b_active & SREWND)
	{
		ts_openf = 0;		/* say closed	*/
		return;
	}
		/* if opened for write or read/write and was
			written then write two tape marks
			and back up one */
	if(flag == FWRITE || ((flag & FWRITE) && (ts_flags & S_WRITTEN)))
	{
		tscommand(dev, (FORMT|WEOF));
		tscommand(dev, (FORMT|WEOF));
		tscommand(dev, (SPCREV|POSIT));
	}

		/* if the no-rewind bit in minor is clear then rewind  */
	if ((minor(dev)&0200) ==0 )
	{
		tscommand(dev, (RWIND|POSIT));
	}
	ts_openf = 0;		/* say closed	*/
}
tscommand(dev, com)
{
	register struct buf *bp;

	bp = &ctsbuf;		/* set buffer pointer to command table	*/
	spl5(); 		/* grab some priority for this	*/
	while(bp->b_flags&B_BUSY)	/* if command table is busy	*/
	{
		bp->b_flags |= B_WANTED;	/* then say we want it	*/
		sleep((caddr_t)bp, PRIBIO);	/* and sleep around	*/
	}
	spl0(); 			/* have the buffer so drop pri	*/
	bp->b_dev = dev;		/* associate with this device	*/
	bp->b_resid = com;		/* load desired command here	*/
	bp->b_blkno = 0;		/* don't need a block number	*/
	bp->b_flags =  B_BUSY|B_READ;	/* mark command buffer busy	*/
	tsstrategy(bp); 		/* do it			*/
	iowait(bp);			/* wait until it's done 	*/
	if(bp->b_flags&B_WANTED)	/* if someone (me) wants it	*/
		wakeup((caddr_t)bp);	/* then say he can have it	*/
	bp->b_flags = 0;		/* say command buffer avail	*/
	return(bp->b_resid);		/* return drive status		*/
}
tsstrategy(bp)
{
	register daddr_t *p;

	if(bp->b_flags & B_PHYS)  /* if RAW I/O call unibus map allocate */
		mapalloc(bp);
	if(bp != &ctsbuf)		/* if not command buffer	*/
	{
		if(ts_openf < 0)
		{
			bp->b_flags |= B_ERROR; /* mark buffer bad	*/
			iodone(bp);
			return;
		}
		p = &ts_nxrec;		/* get pointer to max accessable
						block number	*/

		if(bp->b_blkno > *p)	/* if requested > max access	*/
		{
			bp->b_flags |= B_ERROR; /* thats a no-no	*/
			bp->b_error = ENXIO;	/* make sure it's known */
			iodone(bp);		/* return buffer	*/
			return; 		/* bye - bye		*/
		}

		if(bp->b_blkno == *p && bp->b_flags&B_READ)
		{
			/* if requested block = max accessable & read  */

			clrbuf(bp);		/* zero the buffer	*/
			bp->b_resid = bp->b_bcount;	/* didn't read	*/
			iodone(bp);		/* return the buffer	*/
			return; 		/* bye - bye		*/
		}

		if((bp->b_flags&B_READ) == 0)	/* if a write command	*/
		{
			*p = bp->b_blkno +1;	/* set max access to requested
							+ 1	*/
			ts_flags |= S_WRITTEN;	/* say it's been written */
		}
	}
	bp->av_forw = 0;		/* clear the available forward link */
	spl5(); 			/* grab some priority for this	*/

	if (tstab.b_actf == NULL)	/* if no forward link on the table */
		tstab.b_actf = bp;	/* then set this buffer as next */
	else
		tstab.b_actl->av_forw = bp;	/* else link this one to
							the end 	*/

	tstab.b_actl = bp;		/* set a reverse link in table	*/
	if (tstab.b_active == 0)	/* if not active then		*/
		tsstart();		/* start some activity		*/
	spl0();
}
tsstart()
{
	register struct buf *bp;
	register struct device *tsadr;
	register daddr_t blkno;
	int com;

    loop:
	if ((bp = tstab.b_actf) == NULL)
		return; 	/* no link so return	*/
	tsadr = TSADDR + (bp->b_dev & 07);

	blkno = ts_blkno;	/* get current block #	*/
	if(tsadr->tssr & OFL)
	{				/* driver off-line. bad news	*/
		printf("\nMagtape %o offline\n",minor(bp->b_dev)&0177);
		ts_openf = -1;		/* say this is fatal		*/
		bp->b_flags |= B_ERROR; /* mark buffer bad		*/
		goto next;		/* start the flush loop 	*/
	}
	if (bp == &ctsbuf)		/* if this is command table	*/
	{
		if(bp->b_resid == NOP)	/* No-op means get status	*/
		{
			bp->b_resid = mesbuf.mstsx0;	/* hope this is the latest */
			goto next;	/* try the next one		*/
		}
		tstab.b_active = SCOM;	/* say just general maint command */
		combuf->tswc = 1;	/* we only do one		*/
		combuf->tscom = (ACK|IEI|bp->b_resid);	/* load command */
		tsadr->tsdb = &combuf->tscom;	/* feed driver packet	*/
		return; 		/* later			*/
	}
	if(ts_openf < 0)
	{
		bp->b_error = ENXIO;	/* set fatal flag		*/
		bp->b_flags |= B_ERROR; /* mark buffer bad		*/
		goto next;		/* start the flush loop 	*/
	}
	if(bp->b_blkno > ts_nxrec)
	{
		bp->b_flags |= B_ERROR; /* block # out of range 	*/
		goto next;		/* start the flush loop 	*/
	}
	if(blkno != bp->b_blkno)	/* if requested != current	  */
	{
		tstab.b_active = SSEEK; /* say this is a seek		*/
		if(blkno < bp->b_blkno) /* we're not there yet so	*/
		{			/* space forward to it		*/
			combuf->tscom = (ACK|IEI|SPCFWD|POSIT);
			combuf->tsba = bp->b_blkno - blkno; /* load block */
		}
		else
		{			/* need to backup		*/
			combuf->tscom = (ACK|IEI|SPCREV|POSIT);
			combuf->tsba = blkno - bp->b_blkno; /* load block */
		}
		tsadr->tsdb = &combuf->tscom;	/* feed driver packet	*/
		return; 			/* bye - bye		*/
	}
	tstab.b_active = SIO;		/* say this is read or write	*/
	combuf->tsba = bp->b_un.b_addr; /* get low order address	*/
	combuf->tsbae = bp->b_xmem;	/* set extended bits		*/
	combuf->tswc = bp->b_bcount;	/* grab a positive byte count	*/
	combuf->tscom = (ACK|IEI);	/* set up acknowledge and IEI	*/
	if(bp->b_flags &B_READ)
		combuf->tscom |= (RNEXT|RCOM);	/* read next record	*/
	else
		combuf->tscom |= (WNEXT|WCOM);	/* write next record	*/
	tsadr->tsdb = &combuf->tscom;	/* feed packet to drive 	*/
	return;

	/* this loop is good for flushing buffers on fatal errors	*/
next:
	tstab.b_actf = bp->av_forw;	/* grab next avail forward	*/
	iodone(bp);			/* say current buffer done	*/
	goto loop;			/* try some more		*/
}
tsintr()
{
	register struct buf *bp;
	register int state;
	register struct device *tsadr;

	if((bp = tstab.b_actf) == 0)	/* grab a buffer pointer	*/
	{
		ts_stray++;		/* no pointer. stray interrupt	*/
		return;
	}
	tsadr = TSADDR + (bp->b_dev & 07);
	state = tstab.b_active; 	/* get a copy of status 	*/

	if(tsadr->tssr & SC)		/* Special Condition Set ??	*/
	{
			/* look at the Termination Class code	*/
		switch ((tsadr->tssr & TCC) >> 1)
		{
			case 07:	/* fatal sub-system error	*/
			case 06:	/* non-recoverable error	*/
				state = 0;	/* this means dead	*/
				break;	/* break out			*/

			case 05:	/* recoverable. no tape motion	*/
				++tstab.b_errcnt;
				state = SRETRY; /* do a straight retry	*/
				break;

			case 04:	/* recoverable. tape position +1 */
				if(++tstab.b_errcnt < 10)
				{
					if(bp->b_flags & B_READ)
						/* re-read previous	*/
					  combuf->tscom =(ACK|IEI|RRPRV|RCOM);
					else
						/* write data retry	*/
					   combuf->tscom=(ACK|IEI|WDRTY|WCOM);
					tsadr->tsdb = &combuf->tscom;
					return;
				}
				else
					break;

			case 03:	/* function reject		*/
					/* if VCK then someone diddled
						with the drive		*/
					/* this is fatal		*/
				state = 0;
				break;

			case 02:	/* Tape Status Alert		*/
				if(mesbuf.mstsx0 & TMK)
				{	/* End of file set max to this blk */
					ts_nxrec = bp->b_blkno;
					state = SOK;
					break;
				}
				else if(mesbuf.mstsx0 & EOT)
				{	/* End of tape set max to this blk */
					state = 0;	/* this is fatal */
					break;
				}
				else if(mesbuf.mstsx0&RLS)
				{	/* record length is short. do not
					have to treat this as fatal because
					the actual byte will be returned */

					state = SIO;	/* update block cnt */
					break;
				}
				else if(mesbuf.mstsx0&RLL)
				{
					state = SIO;
					bp->b_flags |= B_ERROR;
					break;
				}
				break;
			case 01:	/* Attention Condition		*/
				if(tsadr->tssr & OFL)
				{
					printf("\nMagtape %o offline\n",
						   minor(bp->b_dev)&0177);
					state = 0;	/* drive offline */
				}
				break;
			default:
				break;
		}
		if(tstab.b_errcnt > 4 || state == 0)
		{
			deverror(bp, tsadr->tssr, mesbuf.mstsx0);
			deverror(bp, mesbuf.mstsx1, mesbuf.mstsx2);
			deverror(bp, mesbuf.mstsx3, tstab.b_errcnt);
		}
		if(tstab.b_errcnt >= 10 || state == 0)
		{
			/* bad news. FATAL ERROR or No recovery */
			tstab.b_errcnt = 0;	/* clear error count	*/
			bp->b_flags |= B_ERROR; /* mark this buffer bad */
			ts_openf = -1;		/* say no more		*/
			if(state)
				tsinit(tsadr, 0);	/* init the drive. no rewind*/
			else
				tsinit(tsadr, 1);	/* init the drive. rewind */
			state = 0;		/* just to be sure	*/
		}
	}
	if((tstab.b_active & SREWND) == 0)
		tstab.b_active = 0;		/* say ok to do more	*/
	switch (state)
	{
		case SRETRY:			/* straight retry	*/
			break;
		case SIO:			/* update block # and	*/
		case SOK:			/* fall through SCOM	*/
			ts_blkno++;
		case SCOM:
			tstab.b_errcnt = 0;	/* no errors		*/
			tstab.b_actf = bp->av_forw;	/* next buffer	*/
			if(combuf->tscom & POSIT)	/* if position	*/
				bp->b_resid = 0;	/* this could be
								garbage */
			else
			       bp->b_resid = mesbuf.msresid;   /* get residual
								count	*/
			iodone(bp);			/* return buffer */
			break;
		case SSEEK:
			ts_blkno = bp->b_blkno; 	/* update blk no */
			break;
		default:
			break;
	}
	tsstart();		/* start any pending I/O		*/
}

tsinit(addr, ini)
struct device *addr;
int ini;
{
	int ds, cnt;
	register struct device *tsadr;

	tsadr = addr;
	if(ini)
	{
		tsadr->tssr = 0;		/* init with rewind	*/
		tstab.b_active = SREWND;
		return(0);
	}
	else
	{
		combuf->tscom = (ACK|CVC|INIT); /* init no rewind	*/
		tsadr->tsdb = &combuf->tscom;	/* feed drive packet	*/
		for(cnt = 0; cnt < 10000; cnt++)
		{				/* wait loop. hopefully */
			if(tsadr->tssr & SSR)	/* 3 - 5 msecs long	*/
				break;		/* have ready. break out*/
		}
	}
	if(cnt >= 10000)
	{					/* wait loop timeout	*/
		printf("TS11 init failed\n");
		deverror(&ctsbuf, tsadr->tssr, mesbuf.mstsx0);
		return(1);			/* bad stuff. say so	*/
	}
	chrbuf.msbptr = &mesbuf;		/* show where message
							buffer is	*/
	chrbuf.msbae = 0;			/* hope no extended addr */
	chrbuf.msbsiz = 016;			/* size of message buf	*/
	chrbuf.mschar = 0;			/* set characteristics	*/
	combuf->tscom = (ACK|CVC|WCHAR);	/* say this is write
							characteristics */
	combuf->tsba = &chrbuf; 		/* addr of chars buf	*/
	combuf->tsbae = 0;			/* hope no ext addr	*/
	combuf->tswc = 010;			/* size of chars buf	*/
	tsadr->tsdb = &combuf->tscom;		/* feed packet to drive */
	for(cnt = 0; cnt < 10000; cnt++)
	{
		if(tsadr->tssr & SSR)
			break;
	}
	if(cnt >= 10000)
	{
		printf("TS11 init failed\n");
		deverror(&ctsbuf, tsadr->tssr, mesbuf.mstsx0);
		return(1);			/* bad stuff. say so	*/
	}
	if(((tsadr->tssr & TCC) >>1) > 1)
	{
		printf("TS11 init failed\n");
		deverror(&ctsbuf, tsadr->tssr, mesbuf.mstsx0);
		return(1);			/* bad stuff. say so	*/
	}
	else
		return(0);			/* good init		*/
}
tsread(dev)
{
	tsphys(dev);
	physio(tsstrategy, &rtsbuf, dev, B_READ);
}
tswrite(dev)
{
	tsphys(dev);
	physio(tsstrategy, &rtsbuf, dev, B_WRITE);
}
tsphys(dev)
{
	daddr_t a;

	if((minor(dev) & 0177) < 8)	/* only if drive 0,		*/
					/* minor device 0 thru 7	*/
	{
		a = u.u_offset >>9;	/* grab the block offset	*/
		ts_blkno = a;		/* set current block		*/
		ts_nxrec = a+1; 	/* set next max block # 	*/
	}
}
