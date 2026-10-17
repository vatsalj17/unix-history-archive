/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1994 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */
/***********************************************************************
 * Module: haict.c
 *
 * Unix device driver functions for accessing SCSI tape drives as
 * character devices.	Conforms to Mark Williams Coherent definition
 * of the Unix Device Driver interface for Coherent v4.2.
 *
 * The philosophy of this driver is to support basic functions on
 * the tape drive (read, write, retension, rewind, skip, etc). There
 * are more features out there for all the SCSI tape drives out there
 * than I know what to do with.  I leave custom support for these
 * drives to the people who have them.  To this end this drive will
 * blindly follow whatever information it can get using Mode Sense
 * and Read Block Limits CDB's.  These tests are done at open time.
 * An application can change the operation of the driver by applying
 * the mode select command through I/O Control mechanism.
 *
 * Copyright (c) 1993, Christopher Sean Hilton, All Rights Reserved.
 *
 * Last Modified: Fri Jun 17 12:05:35 1994 by [kroot]
 *
 * $Id: haict.c,v 2.4 93/08/19 04:02:38 nigel Exp Locker: nigel $
 *
 * $Log:		haict.c,v $
 * Revision 2.4  93/08/19  04:02:38  nigel
 * Nigel's R83
 */

#include <errno.h>
#include <stddef.h>
#include <sys/coherent.h>
#include <sys/cmn_err.h>
#include <sys/buf.h>
#include <sys/inode.h>
#include <sys/stat.h>
#include <sys/sched.h>
#include <sys/mtioctl.h>
#include <sys/tape.h>
#include <sys/file.h>

#include <sys/haiscsi.h>
#include <sys/haiioctl.h>

#define LOCAL	static
#if __GNUC__
#define Register
#else
#define Register	register
#endif

#define REWINDTAPE	0x01
#define	  IMMEDIATE	0x0010
#define REQSENSE	0x03
#define READBLKLMT	0x05
#define READ		0x08
#define WRITE		0x0a
#define WRITEFM 	0x10
#define SPACE		0x11
#define   SKIP_BLK	0x00
#define   SKIP_FM 	0x01
#define   SKIP_SEQFM	0x02
#define   SKIP_EOT	0x03
#define MODESELECT	0x15
#define ERASE		0x19
#define   ERASE_BLOCK	0x0000
#define   ERASE_TAPE	0x0001
#define MODESENSE	0x1a
#define LOAD		0x1b
#define   RETENSION	0x0020

#define CTDIRTY 	0x0001
#define CTCLOSING	0x0002

#define CTILI		0x0020		/* Sensekey's Illegal Length Indicator */
#define CTEOM		0x0040		/* Sensekey's End OF Media bit */
#define CTFILEMARK	0x0080		/* Sensekey's Filemark bit */
#define	CTSKMASK	(CTILI | CTEOM | CTFILEMARK)
#define CTRDMD		0x0100		/* we are reading from the tape */
#define CTWRTMD 	0x0200		/* we are writing to the tape */

/*
 * There wasn't much of a difference in speed between 32 and 40 block
 * in my experiance so save as much kalloc memory as possible.
 */

#define STDCACHESZ	(8 * BSIZE)	/* 32 Block Cache for each device */
#define TDCCACHESZ      (10 * BSIZE)    /* Little bigger on the tandberg */

#ifndef HAICTVERBOSE
#define HAICTVERBOSE	0x0001		/* Switch console messages on/off */
#endif

typedef enum {
	CTIDLE = 0,
	CTINIT,
	CTFBRD,
	CTVBRD,
	CTFBWRT,
	CTVBWRT,
	CTLASTWRT,
	CTSENSE,
	CTWRITEFM,
	CTSPACE,
	CTREWIND,
	CTERASE,
	CTLOADRETEN,
	CTIOCTL
} ctstate_t;

/* Block Descriptors in the mode sense command. */

typedef struct blkdscr_s *blkdscr_p;

typedef struct blkdscr_s {
	union {
		unsigned char	mediatype;
		unsigned long	totalblocks;
	} mt;
	union {
		unsigned char reserved;
		unsigned long blocksize;
	} rb;
} blkdscr_t;

typedef struct blklim_s *blklim_p;

typedef struct blklim_s {
	unsigned		blmax;		/* Maximum size for Reads/Writes */
	unsigned short	blmin;		/* Minimum size for Reads/Writes */
} blklim_t;

typedef struct ctctrl_s *ctctrl_p;

typedef struct ctctrl_s {
	char		*cache, 	/* Transfer Cache */
			*start; 	/* Start of data in cache */
	haft_t *	haft;		/* Host adapter functions */
	size_t		cachesize,	/* Size of cache */
			avail;		/* bytes availaible in cache */
	ctstate_t	state;
	unsigned	block,		/* Block size of device */
			blmax;		/* Block limits maximum */
	unsigned short	blmin,		/* Block Limits minimum */
			flags,		/* Flags from device */
			inuse;		/* In Use flag */
	srb_t		srb;		/* SCSI Request block for transfers */
} ctctrl_t;

LOCAL int ctinit	__PROTO((haft_t *, int));
LOCAL void ctopen	__PROTO((dev_t, int));
LOCAL void ctclose	__PROTO((dev_t));
LOCAL void ctread	__PROTO((dev_t, IO *));
LOCAL void ctwrite	__PROTO((dev_t, IO *));
LOCAL void ctioctl	__PROTO((dev_t, int, char *));
LOCAL int fillcache	__PROTO((ctctrl_p));
LOCAL int flushcache	__PROTO((ctctrl_p));

#define min(a, b)			((a) < (b) ? (a) : (b))

static dca_t dca = {
	ctopen, 		/* Open */
	ctclose,		/* Close */
	hainonblk,		/* No Block point here but don't just drop Buffers */
	ctread, 		/* Read */
	ctwrite,		/* Write */
	ctioctl,		/* Ioctl */
	ctinit, 		/* Load */
	NULL,			/* Unload */
	NULL			/* Poll */
};

dca_p		ctdca = &dca;

static ctctrl_p ctdevs[MAXDEVS];

extern int	haict_tdcbug;
extern size_t	haict_cache;

/***********************************************************************
 * Utility functions.		
 */

#define ctvmsg(l, cmd)		{ if (HAICTVERBOSE & (l)) { (cmd); } }
#define ctsleepPri(ctrl, value)	\
	(((ctrl)->flags & CTCLOSING) ? slpriNoSig : (value))
#define tandberg(id)	((bit(id) & haict_tdcbug) != 0)

/***********************************************************************
 * ctbusywait()
 *
 * Wait for the tape drive state to return to idle. This is easy
 * for two reasons: 1) With no block entry point its safe to sleep
 * at any time. 2) We shouldn't really need this anyhow. This is
 * unneccessary because without a block routine and with only one
 * process able to open the tape drive at a time the state of the
 * tape drive driver is well defined. So, why is it here you ask?
 * because one day some user might fork a process that owns the tape
 * drive. This would cause 40 days and nights worth of rain etc.
 * Now all that will happen is both processes will be able to write/read
 * from the tape drive and the data that they get will be complete
 * garbage. However, the kernel will not break.
 */

#if __USE_PROTO__
LOCAL int ctbusywait(Register ctctrl_p c, Register ctstate_t newstate)
#else
LOCAL int
ctbusywait(c, newstate)
Register ctctrl_p	c;
Register ctstate_t	newstate;
#endif
{
	Register int	s;
	int 			retval;

	s = sphi();
	retval = 1;
	while (c->state != CTIDLE)
		if (x_sleep(& c->srb.status, pritape, slpriSigCatch, "ctbsywt")) {
			set_user_error (EINTR);
			retval = 0;
			break;
		}
	c->state = newstate;
	spl(s);
	return retval;
}	/* ctbusywait() */

/***********************************************************************
 * loadtape()
 *
 * Move the tape to the load point.
 */

#if __USE_PROTO__
LOCAL int loadtape(ctctrl_p c, int opt)
#else
LOCAL int
loadtape(c, opt)
ctctrl_p	c;
int		opt;
#endif
{
	Register srb_p		r = & c->srb;
	Register g0cmd_p	g0 = & r->cdb.g0;

	if (!ctbusywait(c, CTLOADRETEN))
		return 0;

	r->buf.space = PHYS_ADDR | WATCH_REQACK;
	r->buf.ba_phys = NULL;
	r->buf.size = 0;
	r->xferdir = 0;
	r->timeout = 300;
	memset(g0, 0, sizeof(cdb_t));
	g0->opcode = LOAD;
	g0->xfr_len = 1;		/* Move tape to load point.*/
	if (opt & IMMEDIATE)
		g0->lun_lba |= 1;
	if (opt & RETENSION)
		g0->xfr_len |= 2;
	doscsi(c->haft, r, 4, pritape, slpriSigCatch, "loadtape");
	if (r->status != ST_GOOD && printerror(r, "Load failed"))
		set_user_error (EIO);

	c->state = CTIDLE;
	c->flags &= ~(CTFILEMARK | CTEOM);
	return (r->status == ST_GOOD);
}	/* loadtape() */

/***********************************************************************
 * writefm()
 *
 * Write Filemarks on the tape.
 */

#if __USE_PROTO__
LOCAL void writefm(ctctrl_p c, int count)
#else
LOCAL void
writefm(c, count)
ctctrl_p	c;
int		count;
#endif
{
	Register srb_p		r = &(c->srb);
	Register g0cmd_p	g0 = &(r->cdb.g0);

	if (!ctbusywait(c, CTWRITEFM))
		return;

	r->buf.space = PHYS_ADDR | WATCH_REQACK;
	r->buf.ba_phys = NULL;
	r->buf.size = 0;
	r->xferdir = 0;
	r->timeout = 40;
	g0->opcode = WRITEFM;
	g0->lun_lba = (r->lun << 5);
	g0->lba_mid = ((unsigned char *) &count)[2];
	g0->lba_low = ((unsigned char *) &count)[1];
	g0->xfr_len = ((unsigned char *) &count)[0];
	g0->control = 0;
	doscsi(c->haft, r, 4, pritape, ctsleepPri(c, slpriSigCatch), "writefm");
	if (r->status != ST_GOOD && printerror(r, "Write filemarks failed"))
		set_user_error (EIO);
	c->state = CTIDLE;
}	/* writefm() */

/***********************************************************************
 * space()
 *
 * Space over blocks/filemarks/etc.
 */

#if __USE_PROTO__
LOCAL void space(ctctrl_p c, int count, int object)
#else
LOCAL void
space(c, count, object)
ctctrl_p	c;
int		count;
int		object;
#endif
{
	Register srb_p		r = &(c->srb);
	Register g0cmd_p	g0 = &(r->cdb.g0);

	if (!ctbusywait(c, CTSPACE))
		return;

	r->buf.space = PHYS_ADDR | WATCH_REQACK;
	r->buf.ba_phys = NULL;
	r->buf.size = 0;
	r->xferdir = 0;
	r->timeout = 300;
	g0->opcode = SPACE;
	g0->lun_lba = (r->lun << 5) | (object & 3);
	g0->lba_mid = ((unsigned char *) &count)[2];
	g0->lba_low = ((unsigned char *) &count)[1];
	g0->xfr_len = ((unsigned char *) &count)[0];
	g0->control = 0;
	doscsi(c->haft, r, 2, pritape, slpriSigCatch, "space");
	if (r->status != ST_GOOD && printerror(r, "Space failed"))
		set_user_error (EIO);
	c->state = CTIDLE;
}	/* space() */

/***********************************************************************
 * rewind()
 *
 * Rewind the tape drive back to the load point.
 */

#if __USE_PROTO__
LOCAL void rewind(ctctrl_p c, int wait)
#else
LOCAL void
rewind(c, wait)
ctctrl_p	c;
int		wait;
#endif
{
	Register srb_p		r = &(c->srb);
	Register g0cmd_p	g0 = &(r->cdb.g0);

	if (!ctbusywait(c, CTREWIND))
		return;

	r->buf.space = PHYS_ADDR | WATCH_REQACK;
	r->buf.ba_phys = NULL;
	r->buf.size = 0;
	r->timeout = 300;
	r->xferdir = 0;
	memset(g0, 0, sizeof(cdb_t));
	g0->opcode = REWINDTAPE;
	if (!wait)
		g0->lun_lba = (r->lun << 5) | 1;
	doscsi(c->haft, r, 2, pritape, ctsleepPri(c, slpriSigCatch), "rewind");
	if (r->status != ST_GOOD && printerror(r, "Rewind failed"))
		set_user_error (EIO);
	c->flags = 0;
	c->state = CTIDLE;
}	/* rewind() */

#if __USE_PROTO__
LOCAL void erase(ctctrl_p c, int to_eot)
#else
LOCAL void
erase(c, to_eot)
ctctrl_p	c;
int		to_eot;
#endif
{
	Register srb_p		r = &(c->srb);
	Register g0cmd_p	g0 = &(r->cdb.g0);

	if (!ctbusywait(c, CTERASE))
		return;

	r->buf.space = PHYS_ADDR | WATCH_REQACK;
	r->buf.ba_phys = NULL;
	r->buf.size = 0;
	r->timeout = 300;
	r->xferdir = 0;
	memset(g0, 0, sizeof(cdb_t));
	g0->opcode = ERASE;
	g0->lun_lba = (r->lun << 5);
	if (to_eot)
		g0->lun_lba |= 1;
	doscsi(c->haft, r, 2, pritape, slpriSigCatch, "erase");
	if (r->status != ST_GOOD && printerror(r, "Erase failed"))
		set_user_error (EIO);
	if (to_eot)
		c->flags &= ~(CTFILEMARK | CTEOM | CTILI | CTDIRTY);
	c->state = CTIDLE;
}	/* erase() */

/***********************************************************************
 * Device Driver Entry Point routines.		                       *
 ***********************************************************************/

/***********************************************************************
 * ctinit()
 *
 * Initialize the tape device at (id). This doesn't do anything,
 * not even verify that the drive is there because it could be powered
 * off.
 */

#if __USE_PROTO__
LOCAL int ctinit(haft_t * haft, Register int id)
#else
LOCAL int
ctinit(haft, id)
haft_t * 	haft;
Register int	id;
#endif
{
    Register ctctrl_p c = kalloc(sizeof(ctctrl_t));

    if (!haft) {
	cmn_err(CE_WARN, "haict: No host adapter function table.");
	return 0;
    }
    
    if (!c) {
	cmn_err(CE_WARN, "haict: Could not allocate control structure.");
	return 0;
    }
    
    cmn_err(CE_CONT, "%d: HAI SCSI Tape Module v1.9\n", id);
    memset(c, 0, sizeof(ctctrl_t));
    c->haft = haft;
    c->inuse = 0;

    /*
     * Now set up the cache. One of two types either a really big one
     * in physical memory. Or a smaller one in kalloc memory allocated 
     * at open time. Assume the latter.
     */

    c->cache = NULL;
    if (haict_cache) {		/* Configured for physical memory cache */
	c->cachesize = haict_cache;
	c->cache = (char *) getPhysMem(haict_cache);
    }

    if (!c->cache) {
	if (haict_cache)
	    haict_cache = 0;

	c->cachesize = (tandberg(id)) ? TDCCACHESZ : STDCACHESZ;
    }
    c->start = NULL;
    c->srb.target = id;
    c->srb.lun = 0;
    c->state = CTIDLE;
    ctdevs[id] = c;
    return 1;
}

#if __USE_PROTO__
LOCAL void ctopen(dev_t dev, int mode)
#else
LOCAL void
ctopen(dev, mode)
dev_t		dev;
int		mode;
#endif
{
    Register ctctrl_p	c = ctdevs[tid(dev)];
    Register srb_p	r = &(c->srb);
    Register g0cmd_p	g0 = &(r->cdb.g0);
    int 		rblerf;	/* read block limits error flag */
    int 		s;
    char		buf[64];
    blkdscr_p		bd = (blkdscr_p) (buf + 4);
    blklim_p		bl = (blklim_p) (buf);
    
    if (!c) {
	set_user_error (ENXIO);
	return;
    }
    if ((mode != IPR) && (mode != IPW)) {
	set_user_error (EINVAL);
	return;
    }
    
    s = sphi();
    if (c->inuse) {
	set_user_error (EBUSY);
	goto done;
    }
    
    c->inuse = 1;
    c->state = CTINIT;
    r->dev = dev;	   /* Save the rewind bit for close.*/
    
    /***************************************************************
     *  Media_check: Make sure that there is a tape in the drive.
     *  The test unit ready command returns whether or not the
     *  tape drive is has a tape and is ready. We have to retry
     *  this command several times because a bus_device_resets
     *  or tape change is reported as a failed test_unit_ready
     *  followed by a successful one.
     */
    
    r->buf.space = PHYS_ADDR | WATCH_REQACK;
    r->buf.ba_phys = NULL;
    r->buf.size = 0;
    r->timeout = 2;
    r->xferdir = 0;
    memset(g0, 0, sizeof(cdb_t));		/* Test Unit Ready */
    memset(r->sensebuf, 0, sizeof(r->sensebuf));
    doscsi(c->haft, r, 4, pritape, slpriSigCatch, "ctopen");
    
    /***************************************************************
     * If the command fails there probably wasn't a tape in the
     * drive.
     */
    
    if (r->status != ST_GOOD) { 	/* Is there a tape in the drive? */
	if (r->status != ST_USRABRT) {
	    /* Otherwise assume no tape.*/
	    set_user_error (ENXIO);
	    devmsg (r->dev, "Tape drive not ready.");
	}
	goto openfailed;
    }
    else {
	/*******************************************************
	 *  Do a load command on the tandberg tape drives when
	 *  the drive is busy retensioning. This blocks the
	 *  applications from sending too many commands to
	 *  the drive on startup.
	 */
	
	if (tandberg(tid(dev)) && r->sensebuf[0] == 0x70 && r->sensebuf[2] == 0x06) {
            r->buf.space = PHYS_ADDR | WATCH_REQACK;
            r->buf.ba_phys = NULL;
            r->buf.size = 0;
	    r->xferdir = 0;
	    r->timeout = 300;
	    memset(g0, 0, sizeof(cdb_t));
	    g0->opcode = LOAD;
	    g0->xfr_len = 1;		/* Move tape to load point.*/
	    doscsi(c->haft, r, 4, pritape, slpriSigCatch, "ctopen");
	    if (r->status != ST_GOOD) {
		if (printerror(r, "Load failed - TDC3600 not ready"))
		  set_user_error (ENXIO);
		goto openfailed;
	    }
	}
    }
    
    ctvmsg(0x0100, (devmsg(r->dev, "Read block limits.")));
    
    /***************************************************************
     *  Tandberg's Read block limits command is broken.
     */
    
    if (tandberg(tid(dev))) {
	c->blmin = c->blmax = 512;
	rblerf = 0;
    }
    else {
	/*******************************************************
	 *  Do a read block limits to find out what the drive
	 *  is capable of. We need either Read block limits
	 *  or Mode Sense to work here. If we cannot get either
	 *  then we have problems.
	 */
	
	r->buf.space = KRNL_ADDR | WATCH_REQACK;
	r->buf.addr.caddr = (caddr_t) bl;
	r->buf.size = sizeof(blklim_t);
	r->xferdir = DMAREAD;
	r->timeout = 2;
	memset(g0, 0, sizeof(cdb_t));
	g0->opcode = READBLKLMT;
	g0->xfr_len = 6;
	doscsi(c->haft, r, 3, pritape, slpriSigCatch, "ctopen");
        
	if (rblerf = (r->status != ST_GOOD)) {
	    ctvmsg(0x0010, (printerror(r, "Read Block LImits")));
	    c->blmax = c->blmin = 0;
	}
	else {
	    flip(bl->blmax);        /* SCSI to INTEL order */
	    flip(bl->blmin);        /* Ditto */
	    c->blmax = (bl->blmax & 0x00ffffff);
	    c->blmin = bl->blmin;
	}
    }
    
    /***************************************************************
     *  Now for the mode sense. This should return at least one
     *  block descriptor which we can use to figure out the buffer
     *  size that the tape drive is using. Most of the streaming
     *  tape drives don't support variable mode operation. I don't
     *  know about the DATs
     */
    
    r->buf.space = KRNL_ADDR | WATCH_REQACK;
    r->buf.addr.caddr = (caddr_t) buf;
    r->buf.size = sizeof(buf);
    r->xferdir = DMAREAD;
    r->timeout = 2;
    memset(g0, 0, sizeof(cdb_t));
    g0->opcode = MODESENSE;
    g0->xfr_len = sizeof(buf);
    doscsi(c->haft, r, 3, pritape, slpriSigCatch, "ctopen");
    if (r->status != ST_GOOD) {
	if (printerror(r, "Mode sense failed"))
	    set_user_error (EIO);
	goto openfailed;
    }
    
    /***********************************************************************
     * If tape drive opened in write mode make sure the tape is not write
     * protected now.
     */
    
    if (mode == IPW && (buf[2] & 0x80) != 0) {
	devmsg(dev, "Tape is write protected");
	set_user_error (ENXIO);
	goto openfailed;
    }
 
    /***************************************************************
     *  According to SCSI-1 the first media descriptor is the default
     *  we will use this one. SCSI-2 is much clearer on this.
     */
    
    if (buf[3]) {	/* If mode sense returned any media descriptors */
	bd->rb.blocksize &= 0xffffff00;
	flip(bd->rb.blocksize);
	c->block = bd->rb.blocksize;
	if (c->block) {
	    if (haict_cache)
		c->cachesize = haict_cache;
	    else
		c->cachesize = (tandberg(tid(dev))) ? TDCCACHESZ : STDCACHESZ;

	    if (c->cachesize % c->block)
		c->cachesize -= (c->cachesize % c->block);
	}
    }
    else {
	devmsg(r->dev, "No media descriptors: Contact Mark Williams Tech support");
	set_user_error (ENXIO);
	goto openfailed;
    }
    ctvmsg(0x0010, devmsg(dev, "Blocksize: %d bytes.", c->block));
    
    /***********************************************************************
     * One last check:  If we aren't using block mode (!c->block)
     * and we didn't get any block limits then we cannot support this
     * drive.
     */
    
    if (!c->block && rblerf) {
	devmsg(r->dev, "<No block limits on variable mode tape drive>");
	devmsg(r->dev, "<Contact Mark Williams Tech Support>");
	set_user_error (ENXIO);
	goto openfailed;
    }
    
    c->flags = (c->flags & (CTDIRTY | CTEOM)) |
	       ((mode == IPR) ? CTRDMD : CTWRTMD);
    if (c->block) {
	if (!haict_cache) {
	    c->cache = kalloc(c->cachesize);
	    if (!c->cache) {
		devmsg(dev, "Could not allocate tape cache");
		set_user_error (ENOMEM);
		goto openfailed;
	    }
	}
	c->avail = (c->flags & CTRDMD) ? 0 : c->cachesize;
	c->start = c->cache;
    }
    c->state = CTIDLE;
    goto done;
    
 openfailed:
    c->state = CTIDLE;
    c->inuse = 0;
    
 done:
    spl(s);
}	/* ctopen() */

/***********************************************************************
 * ctclose()
 *
 * Close the SCSI Device at (dev).
 */


#if __USE_PROTO__
LOCAL void ctclose(Register dev_t dev)
#else
LOCAL void
ctclose(dev)
Register dev_t	dev;
#endif
{
    Register ctctrl_p	c = ctdevs[tid(dev)];
    Register srb_p	r = &(c->srb);
    int 		s;
    
    if (!c) {
	set_user_error (ENXIO);
	return;
    }
    
    s = sphi();
    c->flags |= CTCLOSING;
    if (c->block && (c->flags & CTDIRTY)) {
	if (ctbusywait(c, CTLASTWRT))
	    flushcache(c);
	c->state = CTIDLE;
    }
    spl(s);
    if (!haict_cache && c->cache) {
	kfree(c->cache);
	c->cache = c->start = NULL;
	c->avail = 0;
    }
    
    if (c->flags & CTDIRTY)
	writefm(c, 1);
    
    if (r->dev & REWIND) {
	if (c->flags & CTDIRTY)
	    writefm(c, 1);
	rewind(c, 0);
    }
    
    c->inuse = 0;
    return;
}	/* ctclose() */

/***********************************************************************
 * fillcache() --	Read from the tape into the cache (really?)
 *
 * return 0 and set u.u_error on any errors.
 */

#if __USE_LOCAL__
LOCAL int fillcache(Register ctctrl_p c)
#else
LOCAL int
fillcache(c)
Register ctctrl_p	c;
#endif
{
	Register srb_p		r = (&c->srb);
	Register g0cmd_p	g0 = &(r->cdb.g0);
	size_t				blocks;
	extsense_p			e;
	int 				info;
	int 				retval = 0;

	r->buf.space = KRNL_ADDR;
	r->buf.addr.caddr = (caddr_t) c->cache;
	r->buf.size = c->cachesize;
	r->xferdir = DMAREAD;
	r->timeout = 30;
	r->tries = 0;
	g0->opcode = READ;
	g0->lun_lba = (r->lun << 5) | 1;
	blocks = c->cachesize / c->block;
	g0->lba_mid = ((unsigned char *) &blocks)[2];
	g0->lba_low = ((unsigned char *) &blocks)[1];
	g0->xfr_len = ((unsigned char *) &blocks)[0];
	g0->control = 0;
	doscsi(c->haft, r, 1, pritape, slpriSigCatch, "ctblkrd");
	switch (r->status) {
	case ST_GOOD:
		c->start = c->cache;
		c->avail = r->buf.size;
		retval = 1;
		break;
	case ST_CHKCOND:
		e = (extsense_p) r->sensebuf;
		if ((e->errorcode & 0x70) == 0x70) {
			info = 0;
			if (e->errorcode & 0x80) {
				info = e->info;
				flip(info);
			}
			if (e->sensekey & (CTFILEMARK | CTEOM)) {
				c->flags |= (e->sensekey & (CTFILEMARK | CTEOM));
				c->start = c->cache;
				c->avail = c->cachesize - (info * c->block);
				retval = 1;
				break;
			}
		}
		printsense(r->dev, "Read failed", (extsense_p) r->sensebuf);
		set_user_error (EIO);
		retval = 0;
		break;
	case ST_USRABRT:
		set_user_error (EINTR);
		c->start = c->cache;
		c->avail = 0;
		retval = 0;
		break;
	default:
		devmsg(r->dev, "Read failed: status (0x%x)", r->status);
		set_user_error (EIO);
		retval = 0;
		break;
	}
	return retval;
}	/* fillcache() */

/***********************************************************************
 * ctfbrd()	--	Fixed block read handler. Reads from the tape
 * 				drive through the cache when the tape drive is
 * 				in fixed block mode.
 */

#if __USE_PROTO__
LOCAL void ctfbrd(Register ctctrl_p c, Register IO *iop)
#else
LOCAL void
ctfbrd(c, iop)
Register ctctrl_p	c;
Register IO 		*iop;
#endif
{
    Register size_t reqcount,	/* Total bytes transfered toward request */
                    xfrsize;	/* Current transfer size */
    size_t          total,	/* System global memory total transfer size */
    		    size;	/* System global memory current transfer size */
    
    if (!ctbusywait(c, CTFBRD))
	return;
    reqcount = 0;
    while (iop->io_ioc) {
	xfrsize = min(c->avail, iop->io_ioc);
	if (xfrsize > 0) {
	    switch (iop->io_seg) {
	    case IOSYS:
		memcpy(iop->io.vbase + reqcount, c->start, xfrsize);
		break;
	    case IOUSR:
		kucopy(c->start, iop->io.vbase + reqcount, xfrsize);
		break;
	    case IOPHY:
		total = 0;
		while (total < xfrsize) {
		    size = min(xfrsize - total, NBPC);
		    xpcopy(c->start + total,
			   iop->io.pbase + reqcount + total,
			   size, SEL_386_KD | SEG_VIRT);
		    total += size;
		}
		break;
	    }
	    c->start += xfrsize;
	    c->avail -= xfrsize;
	    reqcount += xfrsize;
	    iop->io_ioc -= xfrsize;
	}
	if (iop->io_ioc) {
	    if (c->flags & CTFILEMARK) {
		c->flags &= ~CTFILEMARK;
		break;
	    }
	    
	    if (c->flags & CTEOM) {
		set_user_error (EIO);
		break;
	    }
	    
	    if (!fillcache(c))
		break;
	}
    }	/* while */
    c->state = CTIDLE;
}	/* ctfbrd() */

/***********************************************************************
 * ctvbrd()	--	Variable block read entry point.
 */

#if __USE_PROTO__
LOCAL void ctvbrd(Register ctctrl_p c, IO *iop)
#else
LOCAL void
ctvbrd(c, iop)
Register ctctrl_p c;
IO *iop;
#endif
{
    Register srb_p r = &(c->srb);
    Register g0cmd_p g0 = &(r->cdb.g0);
    size_t xfrsize;
    extsense_p e;
    int info;
    
    if (!ctbusywait(c, CTVBRD))
	return;
    
    if (c->flags & CTEOM) {
	set_user_error (EIO);
	return;
    }
    if (c->flags & CTFILEMARK) {
	c->flags &= ~CTFILEMARK;
	return;
    }
    switch (iop->io_seg) {
    case IOSYS:
	r->buf.space = KRNL_ADDR;
	r->buf.addr.caddr = iop->io.vbase;
	break;
    case IOUSR:
	r->buf.space = USER_ADDR;
	r->buf.addr.caddr = iop->io.vbase;
	break;
    case IOPHY:
	r->buf.space = PHYS_ADDR;
	r->buf.addr.paddr = iop->io.pbase;
	break;
    }
    r->buf.size = xfrsize = iop->io_ioc;
    r->xferdir = DMAREAD;
    r->timeout = 30;
    g0->opcode = READ;
    g0->lun_lba = (r->lun << 5);
    g0->lba_mid = ((unsigned char *) &xfrsize)[2];
    g0->lba_low = ((unsigned char *) &xfrsize)[1];
    g0->xfr_len = ((unsigned char *) &xfrsize)[0];
    g0->control = 0;
    doscsi(c->haft, r, 1, pritape, slpriSigCatch, "ctvbrd");
    switch (r->status) {
    case ST_GOOD:
	iop->io_ioc -= r->buf.size;
	break;
    case ST_CHKCOND:
	e = (extsense_p) r->sensebuf;
	if ((e->errorcode & 0x70) == 0x70) {
	    info = 0;
	    if (e->errorcode & 0x80) {
		info = (long) e->info;
		flip(info);
	    }
	    if (e->sensekey & (CTFILEMARK | CTEOM)) {
		c->flags |= (e->sensekey & (CTFILEMARK | CTEOM));
		break;
	    }
	    else if (e->sensekey & CTILI) {
		devmsg(r->dev,
		       "Read failed buffer size %d blocksize %d",
		       xfrsize,
		       xfrsize - info);
		if (info > 0)
		    iop->io_ioc -= (xfrsize - info);
		else
		    set_user_error (EIO);
		break;
	    }
	}
	printsense(r->dev, "Read failed", (extsense_p) r->sensebuf);
	set_user_error (EIO);
	break;
    case ST_USRABRT:
	break;
    default:
	devmsg(r->dev, "Read failed: status (0x%x)", r->status);
	set_user_error (EIO);
	break;
    }
    c->state = CTIDLE;
}	/* ctvbrd() */

/***********************************************************************
 * ctread()	--	OS Read entry point.
 */

#if __USE_PROTO__
LOCAL void ctread(dev_t dev, Register IO *iop)
#else
LOCAL void
ctread(dev, iop)
dev_t dev;
IO *iop;
#endif
{
	Register ctctrl_p	c = ctdevs[tid(dev)];

	if (!c) {
		set_user_error (EINVAL);
		return;
	}

	if (c->block)
		ctfbrd(c, iop);
	else
		ctvbrd(c, iop);
}	/* ctread() */

/***********************************************************************
 * flushcache()	--	flush the data in the cache to the tape.
 *
 * returns 0 and sets u.u_error on failure else returns 1.
 */

#if __USE_LOCAL__
LOCAL int flushcache(Register ctctrl_p c)
#else
LOCAL int
flushcache(c)
Register ctctrl_p	c;
#endif
{
	Register srb_p		r = (&c->srb);
	Register g0cmd_p	g0 = &(r->cdb.g0);
	size_t				xfrsize;
	extsense_p			e;
	int 				info;
	int 				retval = 0;

	if (c->avail >= c->cachesize)
		return 1;

	r->buf.space = KRNL_ADDR;
	r->buf.addr.caddr = (caddr_t) c->cache;
	r->buf.size = xfrsize = c->cachesize - c->avail;
	r->xferdir = DMAWRITE;
	r->timeout = 30;
	r->tries = 0;
	g0->opcode = WRITE;
	g0->lun_lba = (r->lun << 5);
	if (c->block) {
		g0->lun_lba |= 1;
		xfrsize = (xfrsize + c->block - 1) / c->block;
	}
	g0->lba_mid = ((unsigned char *) &xfrsize)[2];
	g0->lba_low = ((unsigned char *) &xfrsize)[1];
	g0->xfr_len = ((unsigned char *) &xfrsize)[0];
	g0->control = 0;
	doscsi(c->haft, r, 1, pritape, ctsleepPri(c, slpriSigCatch), "ctblkwrt");
	switch (r->status) {
	case ST_GOOD:
		c->start = c->cache;
		c->avail = c->cachesize;
		retval = 1;
		break;
	case ST_CHKCOND:
		e = (extsense_p) r->sensebuf;
		if ((e->errorcode & 0x70) == 0x70) {
			info = 0;
			if (e->errorcode & 0x80) {
				info = e->info;
				flip(info);
			}
			if (e->sensekey & CTEOM) {
				c->flags |= CTEOM;
				devmsg(r->dev, "End of tape on block write");
			}
		}
		printsense(r->dev, "Read failed", (extsense_p) r->sensebuf);
		set_user_error (EIO);
		retval = 0;
		break;
	case ST_USRABRT:
		retval = 0;
		break;
	default:
		devmsg(r->dev, "Read failed: status (0x%x)", r->status);
		set_user_error (EIO);
		retval = 0;
		break;
	}
	return retval;
}	/* flushcache() */

/***********************************************************************
 * ctfbwrt()	--	Fixed block write.	This should be fast because
 * 				it uses the tapes drives optimum setting and it
 * 				goes through a cache.
 */

#if __USE_PROTO__
LOCAL void ctfbwrt(Register ctctrl_p c, Register IO *iop)
#else
LOCAL void
ctfbwrt(c, iop)
Register ctctrl_p	c;
Register IO 		*iop;
#endif
{
    Register size_t reqcount,	/* Total bytes transfered */
                    xfrsize;	/* Current transfer size */
    size_t    	    total,	/* System global memory total transfer size */
                    size;	/* System global memory current transfer size */
  
    if (!ctbusywait(c, CTFBWRT))
	return;
    
    reqcount = 0;
    while (iop->io_ioc) {
	xfrsize = min(c->avail, iop->io_ioc);
	if (xfrsize) {
	    switch (iop->io_seg) {
	    case IOSYS:
		memcpy(c->start, iop->io.vbase + reqcount, xfrsize);
		break;
	    case IOUSR:
		ukcopy(iop->io.vbase + reqcount, c->start, xfrsize);
		break;
	    case IOPHY:
		total = 0;
		while (total < xfrsize) {
		    size = min(xfrsize - total, NBPC);
		    pxcopy(iop->io.pbase + reqcount + total,
			   c->start + total,
			   size, SEL_386_KD | SEG_VIRT);
		    total += size;
		}
		break;
	    }
	    c->start += xfrsize;
	    c->avail -= xfrsize;
	    reqcount += xfrsize;
	    iop->io_ioc -= xfrsize;
	}
	if (iop->io_ioc) {
	    if (!flushcache(c))
		break;
	}
    }	/* while */
    c->state = CTIDLE;
}	/* ctfbwrt() */

/***********************************************************************
 * ctvbwrt()	--	Variable block writes.
 */

#if __USE_PROTO__
LOCAL void ctvbwrt(Register ctctrl_p c, Register IO *iop)
#else
LOCAL void
ctvbwrt(c, iop)
Register ctctrl_p	c;
Register IO 		*iop;
#endif
{
	Register srb_p		r = &(c->srb);
	Register g0cmd_p	g0 = &(r->cdb.g0);
	size_t				xfrsize;
	extsense_p			e;
	int 				info;

	if (!ctbusywait(c, CTVBWRT))
		return;

	if (c->blmax && iop->io_ioc > c->blmax) {
		devmsg(r->dev, "Tape Error: maximum read/write size is %d bytes.", c->blmax);
		set_user_error (EIO);
		return;
	}
	switch (iop->io_seg) {
	case IOSYS:
		r->buf.space = KRNL_ADDR;
		r->buf.addr.caddr = iop->io.vbase;
		break;
	case IOUSR:
		r->buf.space = USER_ADDR;
		r->buf.addr.caddr = iop->io.vbase;
		break;
	case IOPHY:
		r->buf.space = PHYS_ADDR;
		r->buf.addr.paddr = iop->io.pbase;
		break;
	}
	xfrsize = min(iop->io_ioc, c->blmin);
	r->buf.size = xfrsize;
	r->xferdir = DMAWRITE;
	r->timeout = 30;
	g0->opcode = WRITE;
	g0->lun_lba = (r->lun << 5);
	g0->lba_mid = ((unsigned char *) &xfrsize)[2];
	g0->lba_low = ((unsigned char *) &xfrsize)[1];
	g0->xfr_len = ((unsigned char *) &xfrsize)[0];
	g0->control = 0;
	doscsi(c->haft, r, 1, pritape, slpriSigCatch, "ctvbwrt");
	switch (r->status) {
	case ST_GOOD:
		iop->io_ioc -= r->buf.size;
		break;
	case ST_CHKCOND:
		e = (extsense_p) r->sensebuf;
		if ((e->errorcode & 0x70) == 0x70) {
			info = 0;
			if (e->errorcode & 0x80) {
				info = (long) e->info;
				flip(info);
			}
			if (e->sensekey & CTEOM) {
				c->flags |= CTEOM;
				devmsg(r->dev, "End of tape");
			}
		}
		printsense(r->dev, "Write failed", (extsense_p) r->sensebuf);
		set_user_error (EIO);
		break;
	case ST_USRABRT:
		break;
	default:
		devmsg(r->dev, "Read failed: status (0x%x)", r->status);
		set_user_error (EIO);
		break;
	}
	c->state = CTIDLE;
}	/* ctvbwrt() */

/***********************************************************************
 * ctwrite()	-- Write entry point for tape drive.
 */

#if __USE_PROTO__
LOCAL void ctwrite(Register dev_t dev, Register IO *iop)
#else
LOCAL void
ctwrite(dev, iop)
Register dev_t	dev;
Register IO 	*iop;
#endif
{
	Register ctctrl_p c = ctdevs[tid(dev)];

	if (!c) {
		set_user_error (ENXIO);
		return;
	}

	c->flags |= CTDIRTY;
	if (c->block)
		ctfbwrt(c, iop);
	else
		ctvbwrt(c, iop);
}	/* ctwrite() */

/***********************************************************************
 * ctioctl()
 *
 * I/O Control Entry point for Cartridge tape devices.
 *
 * This function had been modified to allow applications level programs
 * to select modes and features for the tape drive.As stated above,
 * the philosophy of this driver is to provide least common denominator
 * support for all tape drives. I know that you spend big bucks to
 * get that (insert your favorite drive brand/model).	If I decide
 * to support everything out there on the market then I won't be able
 * to write network drivers, serial drivers, etc. So if you need
 * to do something to the tape drive to make it work (mode sense/select)
 * you can do it through this ctioctl as an applications program.
 */

#if __USE_PROTO__
LOCAL void ctioctl(dev_t dev, Register int cmd, char *vec)
#else
LOCAL void
ctioctl(dev, cmd, vec)
dev_t		dev;
Register int	cmd;
char		*vec;
#endif
{
    Register ctctrl_p	c = ctdevs[tid(dev)];
    Register srb_p		r = &(c->srb);
    int 				s;
    
    if (!c) {
	set_user_error (EINVAL);
	return;
    }
    
    switch (cmd) {
    case T_RST:
	resetdevice(c->haft, tid(dev));
	break;
    case MTREWIND:		/* Rewind */
    case T_RWD:
	if (c->flags & CTDIRTY)
	    writefm(c, 1);

	rewind(c, 1);
	break;
    case MTWEOF:		/* Write end of file mark */
    case T_WRFILEM:
	writefm(c, 1);
	break;
    case MTRSKIP:		/* Record skip */
	space(c, 1, SKIP_BLK);
	break;
    case MTFSKIP:		/* File skip */
    case T_SFF:
	space(c, 1, SKIP_FM);
	break;
    case MTTENSE:		/* Tension tape */
    case T_RETENSION:
	if (c->flags & CTDIRTY)
	    writefm(c, 1);
	loadtape(c, RETENSION);
	break;
    case MTERASE:		/* Erase tape */
    case T_ERASE:
	erase(c, ERASE_TAPE);
	break;
    case MTDEC: 		/* DEC mode */
    case MTIBM: 		/* IBM mode */
    case MT800: 		/* 800 bpi */
    case MT1600:		/* 1600 bpi */
    case MT6250:		/* 6250 bpi */
	return;
    default:
	if (!ctbusywait(c, CTIOCTL))
	    return;
	s = sphi();
	haiioctl(r, cmd, vec);
	c->state = CTIDLE;
	spl(s);
	break;
    }
} /* ctioctl() */
