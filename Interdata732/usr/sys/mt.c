#

/*
 * Interdata 800 bpi mag tape driver
 *
 * This is a 'first attempt' at a driver, and should be thrown
 * away and rewritten.  The major known problems are:
 *	- it probably won't work for multiple transports
 *	- attempting a backspace from loadpoint will hang up the controller
 *	  (this can be cleared manually by a reset/forward/reset/online
 *	  sequence)
 *	- error checking, particularly for end-of-tape, is insufficient
 *
 */

#include "param.h"
#include "conf.h"
#include "user.h"
#include "buf.h"
#include "selch.h"

/* Configuration */

#define	NTAPES	1

#define NSELCH	0	/* connected to selch 0 */

char	mtaddr[] {
	0xc5
};

#define	NMTERR	8	/* number of error retries */

int	mtio(), mtscintr();
struct tape {
	int	m_status;		/* status of transport */
	int	m_blkno;		/* current block position in file */
	int	m_lastrec;		/* next record # at end of file */
} tape[NTAPES];

#define	ISOPEN		01
#define	ISDU		02
#define	ISWRITING	04
#define	ISBOT		010

struct devtab	mtab;

#define	SCOM	1
#define	SSFOR	2
#define	SSREV	3
#define	SIO	4
#define	SBOT	5

struct selchq	mtselchq {
	&mtio,
	&mtscintr,
	0
};

struct buf	rmtbuf, cmtbuf;
int	mtead;
/*
 * Tape controller commands
 */
#define	ENABLE	0x40
#define	CLEAR	0x20
#define	READ	0x21
#define	WRITE	0x22
#define	WF	0x30
#define	RW	0x38
#define	FF	0x23
#define	BF	0x13
#define	BR	0x11

#define	OPEN	0xff

/*
 * Tape status
 */
#define	ERR	0x80
#define	EOF	0x40
#define	EOT	0x20
#define	NMTN	0x10
#define	BSY	0x04
#define	DU	0x01

mtopen(dev, flag)
{
	register struct tape *mt;
	register unit;

	if ((unit=dev.d_minor&03) >= NTAPES
	  || (mt = &tape[unit])->m_status & ISOPEN) {
		u.u_error = ENXIO;
		return;
	}
	mt->m_status = mt->m_blkno = 0;
	mt->m_lastrec = 1000000;

	mtcommand(dev, OPEN);
	if (mt->m_status & ISDU)
		u.u_error = ENXIO;

	if (u.u_error == 0)
		mt->m_status =| ISOPEN;
}

mtclose(dev, flag)
{
	register struct tape *mt;
	register writing;

	mt = &tape[dev.d_minor&03];
	writing = 0;

	if (mt->m_status&ISWRITING) {
		writing++;
		mtcommand(dev, WF);
		mtcommand(dev, WF);
	}
	if (dev.d_minor & 04) {
		if (writing)
			mtcommand(dev, BR);
	} else
		mtcommand(dev, RW);
	mt->m_status = 0;
}

mtcommand(dev, command)
{
	register struct buf *bp;

	trace(0100<<16, "mtcommand", command);
	bp = &cmtbuf;
	spl(5);
	while (bp->b_flags & B_BUSY) {
		bp->b_flags =| B_WANTED;
		sleep(bp, PRIBIO);
	}
	bp->b_flags =| B_BUSY | B_READ;
	spl(0);

	bp->b_dev = dev;
	bp->b_blkno = command;
	mtstrategy(bp);
	iowait(bp);

	if (bp->b_flags & B_WANTED)
		wakeup(bp);
	bp->b_flags = 0;
}

mtstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register struct tape *mt;
	register int *p;

	if ((bp = abp) != &cmtbuf) {
		mt = &tape[bp->b_dev.d_minor&03];
		p = &mt->m_lastrec;
		if (bp->b_blkno > *p) {
			bp->b_flags =| B_ERROR;
			iodone(bp);
			return;
		}
		if (bp->b_blkno == *p && (bp->b_flags&B_READ)) {
			bp->b_resid = bp->b_bcount;
			clrbuf(bp);
			iodone(bp);
			return;
		}
		if ((bp->b_flags&B_READ) == 0)
			*p = bp->b_blkno + 1;
	}

	bp->av_forw = 0;
	spl(5);
	if (mtab.d_actf)
		mtab.d_actl->av_forw = bp;
	else
		mtab.d_actf = bp;
	mtab.d_actl = bp;
	if (mtab.d_active == 0)
		mtstart();
	spl(0);
}

mtstart()
{
	register struct tape *mt;
	register struct buf *bp;

	while (bp = mtab.d_actf) {
		mt = &tape[bp->b_dev.d_minor&03];
		if (bp != &cmtbuf && (mt->m_status&ISDU)) {
			bp->b_flags =| B_ERROR;
			mtab.d_actf = bp->av_forw;
			iodone(bp);
			continue;
		}
		mt->m_status =& ~ISWRITING;
		if (bp == &cmtbuf)
			mtab.d_active = SCOM;
		else if (bp->b_blkno == mt->m_blkno) {
			mtab.d_active = SIO;
			if ((bp->b_flags&B_READ) == 0) {
				mt->m_status =| ISWRITING;
				/*
				 * According to the Interdata tape manual,
				 * when writing the first block it is necessary
				 * to write a file mark first and backspace
				 * over it.  If this is not done, the block
				 * is sometimes not written correctly.
				 */
				if (mt->m_status&ISBOT)
					mtab.d_active = SBOT;
			}
		}
		else if (bp->b_blkno > mt->m_blkno)
			mtab.d_active = SSFOR;
		else
			mtab.d_active = SSREV;
		selchreq(NSELCH, &mtselchq);
		return;
	}
}

mtio()
{
	register struct buf *bp;
	register addr;
	register stat;

	trace(0100<<16, "mtio", mtab.d_active);
	if ((bp = mtab.d_actf) == 0)
		return;

	addr = mtaddr[bp->b_dev.d_minor&03];

	switch(mtab.d_active) {

	case SCOM:
		if (bp->b_blkno == OPEN) {
			register struct tape *mt;

			mt = &tape[bp->b_dev.d_minor&03];
			oc(addr, CLEAR);
			oc(addr, ENABLE);
			if ((stat = ss(addr))&DU)
				mt->m_status =| ISDU;
			if (stat&EOT)
				mt->m_status =| ISBOT;
			mtab.d_active = 0;
		} else
			oc(addr, bp->b_blkno);
		trace(0200<<16, "mtoc", bp->b_blkno);
		trace(0200<<16, "status", ss(addr));
		mtab.d_actf = bp->av_forw;
		iodone(bp);
		selchfree(NSELCH);
		return;

	case SSREV:
		oc(addr, BR);
		trace(0200<<16, "mtoc", BR);
		trace(0200<<16, "status", ss(addr));
		return;

	case SBOT:
		oc(addr, WF);
		trace(0200<<16, "mtoc", WF);
		trace(0200<<16, "status", ss(addr));
		return;

	case SSFOR:
	case SIO:
		oc(selchaddr[NSELCH], STOP);
		wdh(selchaddr[NSELCH], bp->b_addr);
		wdh(selchaddr[NSELCH], bp->b_addr + bp->b_bcount - 1);

		trace(0200<<16, "mtrw", bp->b_addr);
		if ((bp->b_flags&B_READ) || mtab.d_active == SSFOR) {
			oc(addr, READ);
			oc(selchaddr[NSELCH], READ_GO);
		} else {
			oc(addr, WRITE);
			oc(selchaddr[NSELCH], GO);
		}
	}
}

mtscintr(dev, stat)
{
	oc(selchaddr[NSELCH], STOP);
	mtead = (rdh(selchaddr[NSELCH])+1) & ~01;
}

mtintr(dev, stat)
{
	register struct buf *bp;
	register struct tape *mt;
	register op;

	trace(0200<<16, "interrupt", dev);
	trace(0200<<16, "status", stat);

	/*
	 * If NMTN status is not set, wait for another
	 * interrupt. NMTN should be the last bit to set.
	 */
	if ((stat&NMTN) == 0)
		return;

	op = mtab.d_active;
	mtab.d_active = 0;
	if ((bp = mtab.d_actf) == 0)
		return;
	if (op == SCOM) {
		mtstart();
		return;
	}

	selchfree(NSELCH);
	mt = &tape[bp->b_dev.d_minor&03];
	mt->m_status =& ~(ISBOT|ISDU);
	if (stat&DU) {
		mt->m_status =| ISDU;
		mtab.d_actf = bp->av_forw;
		bp->b_flags =| B_ERROR;
		iodone(bp);
		mtstart();
		return;
	}

	if (op == SSREV)
		mt->m_blkno--;
	else
		mt->m_blkno++;

	if ((stat&ERR) && op == SIO) {
		deverror(bp, stat, dev);
		if (++mtab.d_errcnt < NMTERR) {
			mtab.d_active = SSREV;
			selchreq(NSELCH, &mtselchq);
			return;
		}
		bp->b_flags =| B_ERROR;
	}

	if (op == SIO || (stat&(EOF|EOT)) && op == SSFOR) {
		mtab.d_errcnt = 0;
		mtab.d_actf = bp->av_forw;
		bp->b_resid = stat&(EOF|EOT) ? bp->b_bcount :
			bp->b_bcount - (mtead - bp->b_addr);
		iodone(bp);
	}

	mtstart();
}

/*
 * Raw magtape interface
 */
mtread(dev)
{
	mtseek(dev);
	physio(&mtstrategy, &rmtbuf, dev, B_READ);
}

mtwrite(dev)
{
	mtseek(dev);
	physio(&mtstrategy, &rmtbuf, dev, B_WRITE);
}

/*
 * Kludge to ignore seeks on raw mag tape by making block no. look right
 */
mtseek(dev)
{
	register struct tape *mt;

	mt = &tape[dev.d_minor&03];
	mt->m_lastrec = (mt->m_blkno = lshift(u.u_offset, -9)) + 1;
}

/*
 * Stty call is used to issue commands to raw mag tape
 * First word is command function; other two must be 0
 */

char mtcmds[8] {		/* command functions */
	FF,			/* 0 - forward space file */
	BF,			/* 1 - back space file */
	0,			/* 2 */
	WF,			/* 3 - write file mark */
	READ,			/* 4 - forward space record
				 *	(since there is no FR command, we simply
				 *	do a read without starting up the selch
				 *	and ignore the overrun error)
				 */
	BR,			/* 5 - back space record */
	0,			/* 6 */
	RW			/* 7 - rewind */
};

mtsgtty(dev, v)
int *v;
{
	register fn, cmd;
	register int	*ap;

	/* Ignore gtty */
	if (v)
		return;

	ap = u.u_arg;
	if ((fn = *ap) < 0 || fn > 7 || !(cmd = mtcmds[fn])
	    || ap[1] || ap[2]) {
		u.u_error = ENXIO;
		return;
	}
	mtcommand(dev, cmd);
}
