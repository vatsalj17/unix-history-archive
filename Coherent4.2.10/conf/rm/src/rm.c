/* $Header: /ker/io.386/RCS/rm.c,v 2.3 93/08/19 04:03:10 nigel Exp Locker: nigel $ */
/* (-lgl
 * 	COHERENT Device Driver Kit version 1.2.0
 * 	Copyright (c) 1982, 1991 by Mark Williams Company.
 * 	All rights reserved. May not be copied without permission.
 -lgl) */
/*
 * Block or character device RAM disk driver.
 * $Log:	rm.c,v $
 * Revision 2.3  93/08/19  04:03:10  nigel
 * Nigel's R83
 */

#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <sys/coherent.h>
#include <kernel/trace.h>
#include <sys/uproc.h>
#include <sys/buf.h>
#include <sys/seg.h>
#include <sys/con.h>

/*
 * Patchable variables for 386.
 * Starting addresses in kernel data and size for each ram disk.
 */

int	RAM0 = 0x88000000;
int	RAMSIZE = 0x800000;
#define RAM1	(RAM0 + RAMSIZE)


/*
 * Minor number encoding: dsssssss
 * d       drive number (0 or 1)
 * sssssss allocation size: 0 to free, 1-127 allocsize (n * ASIZE * BSIZE bytes)
 */

#define	rm_drive(dev)	(minor(dev) >> 7)
#define	rm_asize(dev)	(minor(dev) & 0x7F)
#define	ASIZE		128	/* allocation chunk size in blocks (64KB) */
#define NUM_RM		2	/* number of ram disks */
				/* - tied to dev encoding (see above) */
#define	RMMAJ		8	/* major # for driver */

typedef struct rm {
	off_t	rm_size;	/* Size in allocation chunks */
	SR	rm_sr;
	int	rm_nopen;	/* Open count to avoid blowups */
} RM;

static	RM	rm [NUM_RM];

/*
 * Load.
 */

static void
rmload ()
{
}


/*
 * Unload.
 * Release the allocated buffers.
 */

static void
rmuload ()
{
	int i;

	for (i = 0; i < NUM_RM; i ++) {
		if (rm [i].rm_size != 0) {
			unload (& rm [i].rm_sr);
			sfree (rm [i].rm_sr.sr_segp);
		}
	}
}


/*
 * Open.
 * Allocate on the first call.
 * Increment the open count.
 */

static void
rmopen (dev, mode)
dev_t dev;
int mode;
{
	RM * rmp;
	off_t asize, osize;

	rmp = & rm [rm_drive (dev)];
	asize = rm_asize (dev);
	osize = rmp->rm_size;

	/* Fail on read before creation or bogus size. */
	if ((mode == IPR && osize == 0) ||
	    (asize != 0 && osize != 0 && asize != osize) ||
	    (asize == 0 && osize == 0)) {
		SET_U_ERROR ( ENXIO,
			"can not read ram disk yet or bogus size" );
		return;
	}

	if (ASIZE * BSIZE * asize > RAMSIZE) {
		SET_U_ERROR ( ENOMEM, "Ram disk too big" );
		return;
	}

	/*
	 * Allocate as required.
	 * Ignore case asize == 0 && osize!= 0, handled by rmclose ().
	 * If asize!= 0 && asize == osize, just bump the open count.
	 */
	if (asize != 0 && osize == 0) {
		rmp->rm_sr.sr_segp =
		    salloc ((off_t)ASIZE * BSIZE * asize, SFSYST | SFNSWP | SFNCLR);
		if (rmp->rm_sr.sr_segp == NULL) {
			SET_U_ERROR ( ENOMEM,
				"can not allocate segment for ram disk" );
			return;
		}

		rmp->rm_size = asize;
		rmp->rm_sr.sr_base = rm_drive (dev)== 0 ? RAM0 : RAM1;
		rmp->rm_sr.sr_size = rmp->rm_sr.sr_segp->s_size;

		doload (& rmp->rm_sr);
	}
	rmp->rm_nopen ++;
}


/*
 * Close.
 * Decrement the open count.
 * Release the allocated buffer if minor number is 0.
 */

static void
rmclose (dev)
dev_t dev;
{
	RM * rmp;
	off_t asize, osize;

	rmp = rm + rm_drive (dev);
	asize = rm_asize (dev);
	osize = rmp->rm_size;

	if (osize == 0 || (asize != 0 && asize != osize) ||
	    rmp->rm_nopen == 0) {
		set_user_error (ENXIO);
		return;
	}

	rmp->rm_nopen --;

	if (asize == 0) {
		if (rmp->rm_nopen != 0) {
			set_user_error (EBUSY);
			return;
		}
		unload (& rmp->rm_sr);
		sfree (rmp->rm_sr.sr_segp);
		rmp->rm_size = 0;
	}
}


static void
rmblock (bp)
BUF * bp;
{
	caddr_t base;
	off_t asize, osize;
	dev_t dev;
	RM * rmp;

	dev = bp->b_dev;
	rmp = rm + rm_drive (dev);
	asize = rm_asize (dev);
	osize = rmp->rm_size;
	if (osize == 0 || asize != osize)
		set_user_error (ENXIO);
	else if (bp->b_bno >= asize * ASIZE)
		set_user_error (EIO);
	else {
		base = rmp->rm_sr.sr_base + (paddr_t)bp->b_bno * BSIZE;

		if (bp->b_req == BREAD)
			dmaout (bp->b_count, bp->b_paddr, base);
		else
			dmain (bp->b_count, bp->b_paddr, base);
	}
	bdone (bp);
}


/*
 * The read routine calls the common raw I / O processing code,
 * using a static buffer header in the driver.
 */

static void
rmread (dev, iop)
dev_t dev;
IO * iop;
{
	ioreq (NULL, iop, dev, BREAD, BFIOC | BFRAW);
}


/*
 * The write routine is just like the read routine,
 * except that the function code is write instead of read.
 */

static void
rmwrite (dev, iop)
dev_t dev;
IO * iop;
{
	ioreq (NULL, iop, dev, BWRITE, BFIOC | BFRAW);
}


CON rmcon = {
	DFBLK | DFCHR,
	RMMAJ,
	rmopen,			/* Open */
	rmclose,		/* Close */
	rmblock,		/* Block */
	rmread,			/* Read */
	rmwrite,		/* Write */
	NULL,
	NULL,
	NULL,
	rmload,			/* Load */
	rmuload			/* Unload */
};

