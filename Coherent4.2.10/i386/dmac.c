/* $Header: /ker/i386/RCS/dmac.c,v 2.4 93/10/29 00:56:43 nigel Exp Locker: nigel $ */
/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or persuant to the license agreement is unlawful.
 *
 *	COHERENT Version 2.3.37
 *	Copyright (c) 1982, 1983, 1984.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
/*
 * Program the 8237 DMA controller on the system board.
 * $Log:	dmac.c,v $
 * Revision 2.4  93/10/29  00:56:43  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.3  93/08/19  03:40:01  nigel
 * Nigel's R83
 */

#include <sys/types.h>

#define	_KERNEL		1

#include <kernel/reg.h>
#include <sys/dmac.h>

/*
 * Identify the dma segment of a physical address.
 */

#if 0

/* This variant explains part of hcp's problems with COH ddk dma. */

#define	dmaseg(p)	((p) & ~ (NBPC - 1))

#else

/* A "dma segment" is a range of addresses within consecutive 64k physical
   boundaries. */

#define	dmaseg(p)	((p) & ~ (0xffff))
#endif

/*
 * This table maps channel
 * numbers into DMA page address
 * register ports. The wiring of the
 * RA lines on the 74LS670 is a bit
 * strange.
 */

static	int	dmaport[8] = {
	DMAPAGE + 7,			/* 0 (Free) */
	DMAPAGE + 3,			/* 1 (Free) */
	DMAPAGE + 1,			/* 2 (Floppy) */
	DMAPAGE + 2,			/* 3 (Free) */
	DMAPAGE + 15,			/* 4 (Cascade) */
	DMAPAGE + 11,			/* 5 */
	DMAPAGE + 9,			/* 6 */
	DMAPAGE + 10			/* 7 */
};


/*
 * Program the channel of the
 * 8237 DMA controller specified by
 * "chan". The "paddr" is a 20 bit
 * physical address. The "count" is
 * the byte count. The "wflag" is
 * true if this is a write, from the
 * point of view of the device.
 * True return if the mapping can be
 * set up, given the 64K limits.
 * The "count" is predecremented, so
 * that backplane "+T/C" is issued at
 * the expected point in time.
 */

int
dmaon(chan, paddr, count, wflag)
register int	chan;
paddr_t		paddr;
unsigned	count;
int		wflag;
{
	int	port;
	int	s;

	/*
	 * Change from 0 based transfer count to -1 based.
	 */

	count --;

	/*
	 * Select byte/word transfer.
	 * Channels 0-4 use byte transfer.
	 * Channels 5-7 use word transfers, with low 17 addr bits right shifted.
	 */

	if (chan >= 5) {
		count >>= 1;
		paddr >>= 1;
		paddr += paddr & 0xFF0000L;
	}

	/*
	 * Check for DMA straddle.
	 */

	if (dmaseg (paddr) != dmaseg (paddr + count))
		return 0;

	s = sphi ();

	/*
	 * Select DMA controller.
	 */

	if (chan < 4) {
		port = DMA;

		/*
		 * Program for dma read/write operation.
		 */
		if (wflag != 0)
			outb (port + SETMODE * 1, (chan & 3) | RDMEM);
		else
			outb (port + SETMODE * 1, (chan & 3) | WRMEM);
		outb(port + CLEARFL * 1, 0);
	} else {
		port = SDMA;

		/*
		 * Program for dma read/write operation.
		 */

		if (wflag != 0)
			outb (SDMA + SETMODE * 2, (chan & 3) | RDMEM);
		else
			outb (SDMA + SETMODE * 2, (chan & 3) | WRMEM);
		outb (SDMA + CLEARFL * 2, 0);
	}

	/*
	 * Select memory bank.
	 */

	outb (dmaport [chan], (int) (paddr >> 16));

	if (chan < 4)
		port += (chan & 3) << 1;
	else
		port += (chan & 3) << 2;

	/*
	 * Program memory offset in bank.
	 */

	outb (port, (int) paddr >> 0);
	outb (port, (int) paddr >> 8);

	port ++;
	if (chan >= 4)
		port ++;

	/*
	 * Program transfer count.
	 */

	outb (port, count >> 0);
	outb (port, count >> 8);

	spl (s);
	return 1;
}


/*
 * dmago(chan) - initiate dma transfer
 */

void
dmago(chan)
int chan;
{
	/*
	 * Enable dma transfers.
	 */
	if (chan < 4)
		outb (DMA + SETMASK * 1, (chan & 3) | MASKOFF);
	else
		outb (SDMA + SETMASK * 2, (chan & 3) | MASKOFF);
}


/*
 * dmaoff(chan) - turn dma channel off, return residual count
 */

int
dmaoff(chan)
int chan;
{
	int port;
	int count;
	int s;

	/*
	 * Disable DMA transfers.
	 * Obtain the -1 based residual count.
	 */

	s = sphi ();
	if (chan < 4) {
		outb (DMA + SETMASK * 1, (chan & 3) | MASKON);
		port = DMA + ((chan & 3) << 1) + 1;
	} else {
		outb (SDMA + SETMASK * 2, (chan & 3) | MASKON);
		port = SDMA + ((chan & 3) << 2) + 2;
	}
	count = inb (port);
	count += inb (port) << 8;
	spl (s);

	/* Convert residual from -1 based to 0 based. */
	count ++;

	/* Don't report a count of zero as a count of 65536! */
	count &= 0xffff;

	/*
	 * Convert residual from word based to byte based.
	 */

	if (chan >= 5)
		count <<= 1;

	/*
	 * Return residual count in bytes.
	 */

	return count;
}
