/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1994 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */
/***********************************************************************
 * Module: haisd.c
 *
 * Unix device driver functions for accessing SCSI hard drives as
 * block devices. Conforms to Mark Williams Coherent definition of
 * the Unix Device Driver interface.
 *
 * Copyright (c) 1993, Christopher Sean Hilton. All rights reserved.
 *
 * Last Modified: Wed Jul 13 16:49:30 1994 by [chris]
 *
 * This code assumes BSIZE == (1 << 9).
 *
 * $Id$
 *
 * $Log$
 */

#include <stddef.h>
#include <sys/coherent.h>
#include <sys/buf.h>
#include <sys/cmn_err.h>
#include <sys/fdisk.h>
#include <sys/sched.h>
#include <sys/stat.h>
#include <sys/haiscsi.h>

#include <errno.h>

#define INQBUFSZ	64
#define RETRYLIMIT      7

#define LOCAL	static

extern unsigned lbolt;          /* System timer */
extern size_t haisd_maxreq;	/* Max requests for look ahead */

typedef enum {
    SD_INIT = -1,               /* Driver hasn't been initialized */
    SD_IDLE = 0,		/* Driver is Idle */
    SD_STARTSCMD,		/* Start SCSI command */
    SD_SCMDWAIT,		/* Wait for SCSI command to finish */
    SD_SCMDFINISH,		/* SCSI Command has finished */
    SD_RECOVER,			/* Recover from failed command */
    SD_RSTARTSCMD,		/* Start a command for recovery */
    SD_RSCMDWAIT,		/* Wait for a recovery command */
    SD_RSCMDFINISH		/* Finish up a recovery command */
} sdstate_t;

typedef struct partlim_s *partlim_p;

typedef struct partlim_s {
	unsigned long	p_base;		/* base of the partition (blocks) */
	unsigned long	p_size;		/* size of the partition (blocks) */
} partlim_t;

#if __ALLOW_STATS

#define HDGETPERF 	(HDIOC|13)	/* Get performance stats */
#define HDCLRPERF	(HDIOC|14)	/* Clear Performance stats */
#define HDGETPERFCLR	(HDIOC|15)	/* Get perf stats and clear */ 

typedef struct hdstats_s {
	unsigned 	hd_reqcnt;	/* Number of request */
	unsigned	hd_blkcnt;	/* Number of blocks read */
	unsigned	hd_time;	/* Time to complete these request */
	unsigned	hd_errcnt;	/* Number of errors generated */
} hdstats_t;
#endif

typedef struct sdctrl_s *sdctrl_p;

typedef struct sdctrl_s {
    sdstate_t	c_state;	/* Driver state */
    haft_t *	c_haft;		/* Host adapter function table */
    buf_t	*c_actf,	/* First buffer in driver work queue */
    		*c_actl;	/* Last buffer in driver work queue */
    int		c_tries;	/* Number of tries on this command */
    int         c_lastclose;	/* Last close on this device */
    int		c_rmb;		/* Removable media flag */
    daddr_t     c_blkno;        /* Block number of last I/O op */
    size_t      c_blkcnt;       /* Count of blocks on last I/O op */
    size_t	c_reqcnt;	/* Number of requests served by this buffer */
    unsigned    c_starttime;    /* Start time on current request */
    size_t	c_sglistsize;	/* Scatter/gather list size */
    haisgsegm_t	c_sglist[16];	/* Scatter/gather list elements */
    partlim_t   c_plim[5];	/* Partition table entries */
    srb_t	c_srb;		/* SCSI request block for all commands */
#if __ALLOW_STATS
    hdstats_t	c_stats;	/* Statistics */
#endif
} sdctrl_t;

static sdctrl_p sddevs[MAXDEVS] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/*
 * Make a table of all the command that we will use. This is more
 * self documenting and allows us to have a last operation string
 * so we can print out better error messages when things don't work.
 */

typedef struct scmd_s *scmd_p;

typedef struct scmd_s {
    unsigned char sc_opcode;	/* Opcode */
    char *sc_description;	/* String description */
} scmd_t;

scmd_t	TEST_UNIT_READY = { 0x00, "Test unit ready" },
	REZERO_UNIT 	= { 0x01, "Rezero unit" },
 	REQUEST_SENSE	= { 0x03, "Request Sense" },
	INQUIRY		= { 0x12, "Inquiry" },
	MODE_SENSE	= { 0x1a, "Mode Sense" },
	READ_CAPACITY	= { 0x25, "Read Capacity" },
	G1READ		= { 0x28, "Read" },
    	G1WRITE		= { 0x2a, "Write" },
	G1SEEK		= { 0x2b, "Seek" };

static char *lastscsicmd;

#define set_opcode(o, cmd)	\
{ \
    lastscsicmd = (cmd).sc_description; \
    o = (cmd).sc_opcode; \
}

/*
 * And now a table of error strings for printing out sense conditions.
 */

static char *sensestr[] = {
    "False check condition (Firmware fault)",
    "Recovered error",
    "Device not ready error",
    "Medium error",
    "Hardware error",
    "Illegal request condition (Driver/Firmware fault)",
    "Unit attention condition",
    "Data protect error",
    "Blank check error",
    "Vendor unique error",
    "Copy aborted ",
    "Aborted command",
    "Equal",
    "Volume overflow",
    "Miscompare",
    "Reserved"
};

/*
 * Function prototypes.
 */

LOCAL int sdload 	__PROTO((haft_t *, register int));
LOCAL void sdunload 	__PROTO((register int));
LOCAL int sdopen 	__PROTO((dev_t, ...));
LOCAL void sdclose 	__PROTO((dev_t));
LOCAL void sdblock 	__PROTO((register buf_t *));
LOCAL void sdread 	__PROTO((dev_t, IO *));
LOCAL void sdwrite 	__PROTO((dev_t, IO *));
LOCAL void sdioctl 	__PROTO((dev_t, int, char *));

LOCAL void sdmachine 	__PROTO((register sdctrl_p));

/*
 * Kernel functions
 */

int busyWait	__PROTO((int (*)(void), unsigned));
int busyWait2	__PROTO((int (*)(void), unsigned));
int fdisk	__PROTO((dev_t, struct fdisk_s []));
unsigned kucopy	__PROTO((void *, void *, unsigned));
unsigned ukcopy	__PROTO((void *, void *, unsigned));

static dca_t dca = {
    sdopen, 		/* Open */
    sdclose,		/* Close */
    sdblock,		/* Block */
    sdread, 		/* Read */
    sdwrite,		/* Write */
    sdioctl,		/* Ioctl */
    sdload, 		/* Load */
    sdunload,		/* Unload */ 	
    NULL		/* Poll */ 
};

dca_p sddca = &dca;

#define partindex(d)	((((d) & (SPECIAL | PARTMASK)) == SPECIAL) \
			 ? 0 : ((d) & PARTMASK) + 1)
#define blkdev_base(p, d)       (p[partindex(d)]. p_base)
#define blkdev_size(p, d)       (p[partindex(d)]. p_size)
#define set_state(ctrl_p, newstate)   ((ctrl_p)->c_state = (newstate))

#define	DEBUG_QTRACE 	0

#if	DEBUG_QTRACE
#define qtrace(x, y, c)	_chirp((c), 2 * (80 * (y) + (x)))
#else
#define	qtrace(x, y, c)
#endif

/***********************************************************************
 *  srbpending()
 *  
 *  Repeatedly check the status in sp_srb for pending.  Return 0 if
 *  still pending, one if not. This function is intended to be used
 *  with busyWait. Use as follows:
 *  
 *      set p_srb to point to your srb.
 *      set up your srb and start the command.
 *      call busyWait with the timeout that you would like and srbpending.
 */

static srb_p	p_srb = NULL;

#if __USE_PROTO__
LOCAL int srbpending(void)
#else
LOCAL int
srbpending()
#endif
{
    return (p_srb->status != ST_PENDING); 
}   /* srbpending() */

/***********************************************************************
 *  timedscsi()
 *  
 *  For lack of a better name... This does the command within an srb
 *  or times out within the given number of clock ticks. This does
 *  not use sleep so it's safe to use anywhere however it does
 *  use busyWait so it's probably best left for short commands or the
 *  load/init routine.
 */

#if __USE_PROTO__
LOCAL int timedscsi(haft_t * haft, register srb_p r, unsigned clockticks)
#else
LOCAL int timedscsi(haft, r, clockticks)
haft_t * 	haft;
srb_p		r;
unsigned	clockticks;
#endif
{
    int rval;
    int s;

    s = sphi();
    p_srb = r;
    if (!startscsi(haft, r)) {
	cmn_err(CE_WARN, 
                "(%d,0x%x) haisd: could not start scsi command", 
                major(r->dev), minor(r->dev));
	rval = -1;
    } else
	rval = busyWait(srbpending, clockticks);
    p_srb = NULL;
    spl(s);
    return rval;
}   /* timedscsi() */

#if __USE_PROTO__
LOCAL void timedreqsense(haft_t * haft, register srb_p r, unsigned clockticks)
#else
LOCAL void
timedreqsense(haft, r, clockticks)
haft_t *   haft;
srb_p       r;
unsigned    clockticks;
#endif
{
    r->buf. space = KRNL_ADDR | WATCH_REQACK;
    r->buf. ba_virt = (caddr_t) r->sensebuf;
    r->buf. size = sizeof(r->sensebuf);

    r->cleanup = NULL;

    memset(&(r->cdb), 0, sizeof(cdb_t));
    set_opcode(r->cdb. g0. opcode, REQUEST_SENSE);
    r->cdb. g0. xfr_len = sizeof(r->sensebuf);
    timedscsi(haft, r, clockticks);
}   /* timedreqsense() */

#if __ALLOW_STATS
/***********************************************************************
 * sdclrstat() 
 *  
 *  Clear out the statistics buffer.
 */

#if __USE_PROTO__
LOCAL void sdclrstat(hdstats_t *st)
#else
LOCAL void
sdclrstat(st)
hdstats_t *st;
#endif

{
    int s;

    s = sphi();
    if (st)
        memset(st, 0, sizeof(hdstats_t));
    spl(s);
}
#endif

/***********************************************************************
 * sdload()
 *
 * Start up a DASD device at (id).
 *
 * 1)	Make sure that it's a disk drive and that we can support it.
 * 2)	Get its size and blocksize to make sure that we can use it.
 * 3)	Set up a control structure for it.
 */

#if __USE_PROTO__
LOCAL int sdload(haft_t * haft, register int id)
#else
LOCAL int
sdload(haft, id)
haft_t * haft;
register int id;
#endif
{
    register sdctrl_p c;
    register srb_p r;
    int tries;
    char inqbuf[INQBUFSZ];
    unsigned long diskcap[2];
    int i;

#if HDB
cmn_err (CE_CONT, "haisd loading\n");
#endif
    if (!haft) {
	cmn_err(CE_WARN,
		"haisd: Empty host adapter function table - load failed.");
	return 0;
    }

    c = kalloc(sizeof(sdctrl_t));
    if (!c) {
        cmn_err(CE_WARN, "out of memory in sdload(): ");
        return 0;
    }

    memset(c, 0, sizeof(sdctrl_t));
    c->c_state = SD_INIT;
    c->c_haft = haft;
    /* Request Sense to clear reset condition. */
    
    r = &(c->c_srb);
    r->dev = makedev(SCSIMAJOR, SPECIAL | (r->target << 4));
    r->target = id;
    r->lun = 0;
    r->timeout = 0;
    r->cleanup = NULL;
    r->xferdir = DMAREAD;
    
    timedreqsense(c->c_haft, r, 300);
    if (r->status != ST_GOOD) {
        cmn_err(CE_WARN, "Request sense failed: status (0x%x)\n", r->status);
        kfree(c);
        return 0;
    }
#if HDB
cmn_err (CE_CONT, "request sense succeeded\n");
#endif
    
    /* Inquiry to make sure that this is a disk drive */
    r->buf.space = KRNL_ADDR | WATCH_REQACK;
    r->buf.ba_virt= (caddr_t) inqbuf;
    r->buf.size = sizeof(inqbuf);

    memset(&(r->cdb), 0, sizeof(cdb_t));
    set_opcode(r->cdb. g0. opcode, INQUIRY);
    r->cdb.g0.xfr_len = sizeof(inqbuf);
    timedscsi(c->c_haft, r, 300);
    
    if (r->status != ST_GOOD) {
        cmn_err(CE_WARN, "Inquiry failed status: (0x%x)\n", r->status);
        kfree(c);
        return 0;
    }
#if HDB
cmn_err (CE_CONT, "inquiry succeeded\n");
#endif

    if (inqbuf[0] != 0) {
        cmn_err(CE_WARN,
	  "Device type byte: (0x%x) - not direct access device\n", inqbuf[0]);
        kfree(c);
        return 0;
    }
#if HDB
cmn_err (CE_CONT, "DASD detected\n");
#endif
    
    /* Get Capacity to set up the drive for use */
    for (tries = 3; tries > 0; --tries) {
        r->buf.space = KRNL_ADDR | WATCH_REQACK;
        r->buf.ba_virt = (caddr_t) diskcap;
        r->buf.size = sizeof(diskcap);
        diskcap[0] = diskcap[1] = 0;
        memset(&r->cdb, 0, sizeof(cdb_t));
        set_opcode(r->cdb. g1. opcode, READ_CAPACITY);
    
        timedscsi(c->c_haft, r, 500);
        if (r->status == ST_GOOD)
            break;
        else if (r->status == ST_CHKCOND) {
            timedreqsense(c->c_haft, r, 300);
        }
        else {
            resetdevice(c->c_haft, r->target);
            busyWait(NULL, 500);
        }
    }
    
    if (r->status != ST_GOOD) {
        cmn_err(CE_WARN, "Get Capacity Failed: 0x%x\n", r->status);
        kfree(c);
        return 0;
    }
#if HDB
cmn_err (CE_CONT, "got capacity\n");
#endif
    flip(diskcap[0]);
    flip(diskcap[1]);
    if (diskcap[1] != BSIZE) {
        cmn_err(CE_WARN,
		"Invalid Block Size %ld Reformat with %d Bytes/Block\n",
		diskcap[1],
		BSIZE);
        kfree(c);
        return 0;
    }
    
    inqbuf[36] = '\0';

    /* Suppress weird control characters in inqbuf. */
    for (i = 8; i < 36; i++)
        if (inqbuf [i] < 0x20)
            inqbuf [i] = ' ';
    
    cmn_err(CE_CONT,
	    "%d: %s %d MB\n",
	    id,
	    (inqbuf + 8),
	    (diskcap[0] + bit(10)) >> 11);
    c->c_state = SD_IDLE;
    c->c_plim[0].p_base = 0;
    c->c_plim[0].p_size = diskcap[0];
    c->c_actf = c->c_actl = NULL;
    c->c_rmb = ((inqbuf[1] & 0x80) == 0x80);
    c->c_sglistsize = sizeof(c->c_sglist) / sizeof(haisgsegm_t);

    sddevs[id] = c;
    sddevs[id]->c_state = SD_IDLE;
    return 1;
}

/***********************************************************************
 * sdunload()
 *
 * Unload routine. Right now unused so ifdefed out.
 */

#if __USE_PROTO__
LOCAL void sdunload(register int id)
#else
LOCAL void
sdunload(id)
register int id;
#endif
{
    if (sddevs[id]) {
	kfree(sddevs[id]);
	sddevs[id] = NULL;
    }
}	/* sdunload() */

/***********************************************************************
 * loadptable()
 *
 * Read the master boot record from the Fixed disk and set the block
 * limits on the individual partition devices. Wouldn't it be nice if
 * there were more than four partition slots available?!
 */

#if __USE_PROTO__
LOCAL int loadptable(register dev_t dev)
#else
LOCAL int
loadptable(dev)
register dev_t	 dev;
#endif
{
    struct fdisk_s	fp[4];
    register sdctrl_p	c;
    register int	i;
    
    if (!partindex(dev))
	return 0;
    
    if (fdisk(makedev(major(dev), (minor(dev) & ~PARTMASK) | SPECIAL), fp)) {
	for (c = sddevs[tid(dev)], i = 1; i < 5; ++i) {
	    c->c_plim[i].p_base = fp[i-1].p_base;
	    c->c_plim[i].p_size = fp[i-1].p_size;
	}
	return 1;
    }
    else {
	cmn_err(CE_WARN, "Partition table load failed\n");
	return -1;
    }
}	/* loadptable() */

/***********************************************************************
 *  checkmedia() --     Check for new media in the drive.
 *  
 *  Use the Test Unit Read SCSI Command to check for new or any media
 *  in the drive.
 */

#if __USE_PROTO__
LOCAL int checkmedia(register sdctrl_p c)
#else
LOCAL int
checkmedia(c)
sdctrl_p  c;
#endif
{
    srb_p r = &(c->c_srb);
    
    r->buf. space = PHYS_ADDR | WATCH_REQACK;
    r->buf. ba_phys = NULL;
    r->buf. size = 0;
    r->xferdir = 0;
    r->timeout = 2;
    memset(&(r->cdb. g0), 0, sizeof(cdb_t));    /* Test Unit Ready */
    set_opcode(r->cdb. g0. opcode, TEST_UNIT_READY);
    memset(r->sensebuf, 0, sizeof(r->sensebuf));
    doscsi(c->c_haft, r, 4, pritape, slpriSigCatch, "sdchkmda");
    return (r->status == ST_GOOD);
}   /* checkmedia() */

/***********************************************************************
 * sdopen()
 *
 * Open Entry point for SCSI DASD devices.
 */
#if __USE_PROTO__
LOCAL int sdopen(dev_t dev, ...)
#else
LOCAL int
sdopen(dev /*, mode */)
dev_t	dev;
/* int	   mode; */
#endif
{
    register sdctrl_p c;
    
    c = sddevs[tid(dev)];
    
    if (!c) {
	set_user_error(ENXIO);
	return -1;
    }
    
    if (loadptable(dev) == -1) {
	set_user_error (ENXIO);
	return -1;
    }
    
    if (c->c_rmb && !checkmedia(c)) {
	cmn_err(CE_NOTE,
                "(%d,%d) <Door open - Check media>",
                major(c->c_srb. dev), 
		minor(c->c_srb. dev));
	set_user_error (ENXIO);
	return -1;
    }
    ++c->c_lastclose;
    return 0;
}	/* sdopen() */

/***********************************************************************
 * sdclose()
 *
 * Close the SCSI DASD device at dev.
 */

#if __USE_PROTO__
LOCAL void sdclose(dev_t dev)
#else
LOCAL void
sdclose(dev)
dev_t	dev;
#endif
{
    register sdctrl_p c;
    
    c = sddevs[tid(dev)];
    if (!c)
	set_user_error (ENXIO);
    else if (c->c_lastclose)
	--c->c_lastclose;
}	/* sdclose() */

#if __USE_PROTO__
LOCAL void sdcleanup(register srb_p r)
#else
LOCAL void
sdcleanup(r)
register srb_p r;
#endif
{
    sdmachine(sddevs[r->target]);
}

#if __USE_PROTO__
LOCAL void sddone(register sdctrl_p c, unsigned xfrsize, int errno)
#else
LOCAL void
sddone(c, xfrsize, errno)
register sdctrl_p c;
unsigned xfrsize;
int errno;
#endif
{
    register buf_t *bp;
    int s;

    /*
     * This can be put in a loop to move the actual bytes read out of
     * a look ahead buffer and into the buffer cache to support multi-
     * sector transfers. The trick would be to put a while or for loop
     * around the whole thing. The caller would have to make two calls
     * to this function, one before recovery is attempted with errno 
     * cleared and another after recovery is attempted with errno set.
     * Calling this function will return all the buffers that finished
     * the kernel.
     */

    bp = c->c_actf;
    if (!bp)
	return;

    bp->b_resid -= xfrsize;
    if (bp->b_resid > 0 && xfrsize > 0 && !errno)
	return;
    
    if (errno)
	bioerror(bp, errno);
    
    s = sphi();
    c->c_actf = bp->b_actf;
    bp->b_actf = bp->b_actl = NULL;
    spl(s);
    bdone(bp);
}

#if __USE_PROTO__
LOCAL srb_p sdmksrb(sdctrl_p c, srb_p r)
#else
LOCAL srb_p sdmksrb(c, r)
sdctrl_p c;
srb_p r;
#endif
{
    buf_t *bp;
    size_t blkno;
    size_t maxblk;
    size_t seccnt;
    size_t reqcnt;
    size_t blkcnt;
    int segcount;
    size_t segindex;
    g1cmd_p g1;

    /*
     * Search through the request queue for the first block that needs
     * work to be done. 99.44% of the time this will be the first block
     * in the chain but safe programming dicates that we cover for an
     * error somewhere else and minimize the resulting damage at every
     * stage.
     */

    while ((bp = c->c_actf) != NULL) {
	/*
	 * Get the starting block number and generate the maximum
	 * I/O size. The maximum possible I/O size is the number 
	 * of blocks from the starting block to the end of the device.
	 */

	blkno = bp->b_bno + (bp->b_count - bp->b_resid) / BSIZE;
	maxblk = blkdev_size(c->c_plim, bp->b_dev) - blkno;

	/*
	 * Check to be sure that there is something to do here. In
	 * particular the last I/O request could have been shortened
	 * because it straddled the End of the device. In that case
	 * the block should be returned to the kernel as is.
	 */

	if (maxblk > 0)
	    break;
	else
	    sddone(c, 0, (bp->b_req == BREAD) ? 0 : ENXIO);
    } /* while */

    /*
     * We could have come out of that with no work to do. If so then
     * return the sentinel value.
     */

    if (!bp)
	return NULL;

    /* 
     * Now look ahead through the work list to see if we can optimize
     * our communications with the host adapter. e.g. Send one command
     * to serve several kernel request. Most of the time we probably
     * won't be able to.
     */

    seccnt = (bp->b_resid / BSIZE);
    if (seccnt > maxblk)
	seccnt = maxblk;
    blkcnt = seccnt;
    reqcnt = 1;

    memset(c->c_sglist, 0, sizeof(c->c_sglist));
    c->c_sglist[0]. sgs_segstart = NULL;
    c->c_sglist[0]. sgs_segsize = 0;
    segindex = 0;
    segcount = addbuftosgl(bp->b_paddr + bp->b_count - bp->b_resid,
			   (seccnt * BSIZE),
			   c->c_sglist,
			   c->c_sglistsize);
    if (segcount != -1) {
	segindex += segcount;

	while (reqcnt < haisd_maxreq &&
	       blkcnt < maxblk &&
	       bp->b_actf != NULL && 
               bp->b_actf->b_bno == blkno + blkcnt &&
	       bp->b_dev == bp->b_actf->b_dev &&
	       bp->b_req == bp->b_actf->b_req) {

	    bp = bp->b_actf;
	    if (bp->b_resid != bp->b_count)
		break;
	    
	    seccnt = (bp->b_resid / BSIZE);
	    if (seccnt + blkcnt > maxblk)
		seccnt = maxblk - blkcnt;	
	    segcount = addbuftosgl(bp->b_paddr,
				   (seccnt * BSIZE),
				   c->c_sglist + segindex,
				   c->c_sglistsize - segindex);

	    if (segcount == -1)
		break;

	    segindex += segcount;
	    blkcnt += seccnt;
	    ++reqcnt;
	} /* while */
    } /* if */
    else {
	cmn_err(CE_CONT, "[Couldn't start Scatter/Gather list]\n");
    }
    
    bp = c->c_actf;
    c->c_blkno = blkno + blkdev_base(c->c_plim, bp->b_dev);
    c->c_blkcnt = blkcnt;
    c->c_reqcnt = reqcnt;

    r->dev = bp->b_dev;
    r->target = tid(r->dev);
    r->lun = lun(r->dev);
    
    if (reqcnt == 1) {
	r->buf.space = SYSGLBL_ADDR;
	r->buf.ba_phys = bp->b_paddr + bp->b_count - bp->b_resid;
	r->buf.size = bp->b_resid;
    } /* if */
    else {
	r->buf.space = SGLIST_ADDR;
	r->buf.ba_sglist = c->c_sglist;
	r->buf.size = segindex + 1;
    } /* else */
    r->cleanup = &sdcleanup;
    
    g1 = &(r->cdb. g1);
    memset(g1, 0, sizeof(g1cmd_t));

    if (bp->b_req == BREAD) {
	r->xferdir = DMAREAD;
	set_opcode(g1->opcode, G1READ);
    }
    else {
	r->xferdir = DMAWRITE;
	set_opcode(g1->opcode, G1WRITE);
    }
    g1->lba = c->c_blkno;
    flip(g1->lba);
    g1->xfr_len = c->c_blkcnt;
    flip(g1->xfr_len);

    return r;
} /* sdmksrb() */

#if __USE_PROTO__
LOCAL void sdmscheckout(sdctrl_p c, size_t xfrsize)
#else
LOCAL void
sdmscheckout(c, xfrsize)
sdctrl_p c;
size_t xfrsize;
#endif
     
{
    buf_t *bp;
    size_t bufsize;

    /*
     * Setup the variables. The data in the cache is valid for
     * (cachesize) bytes.
     */

    bp = c->c_actf;
    while (xfrsize > 0) {
	if (!bp)
	    cmn_err(CE_WARN, "haisd: Internal error: ran out of buffers.");

	/*
	 * Get number of bytes to transfer, always the minimum of the 
	 * Number of bytes in the cache or the number of bytes requested.
	 */

	bufsize = bp->b_resid;
	if (bufsize > xfrsize)
	    bufsize = xfrsize;
	
	/*
	 * Update the block and give it back to the kernel if it's done.
 	 */

        sddone(c, bufsize, 0);
	
	/*
	 * Update the cachestart and buffer pointers and go back 
	 * for another pass. 
	 */
	 
	bp = c->c_actf;
	xfrsize -= bufsize;
    } /* while */
} /* sdmscheckout() */

/*
 * int sderror()
 *
 * Print a message and return 1 if the last SCSI command had a hard error.
 * Return a 0 if no error or if drive corrected the error. 
 *
 * Side effects: if xfrsz is non-null set it to the number of blocks that
 * where effected on the last I/O instruction.
 */

#if __USE_PROTO__
LOCAL int sderror(register sdctrl_p c, size_t *xfrsz)
#else
LOCAL int
sderror(c, xfrsz)
register sdctrl_p c;
size_t *xfrsz;
#endif
{
    register srb_p      r = &(c->c_srb);
    extsense_p		e;
    unsigned long	info;

    /*
     * First check the status in the srb. If this is set to ST_GOOD then
     * Look no further.
     */

    if (r->status == ST_GOOD) {
	if (xfrsz)
	    *xfrsz = c->c_blkcnt;
        return 0;
    }

    /*
     * When Status is not good assume the worst and handle corrected
     * Errors as a special case.
     */

    if (xfrsz)
	*xfrsz = 0;

    /*
     * When status is ST_CHKCOND do a Request Sense command to get more
     * information.
     */

    if (r->status == ST_CHKCOND) {
	e = (extsense_p) r->sensebuf;	/* Assume reqsense will work. */
	timedreqsense(c->c_haft, r, 300);

	/*
         * If the host adapter is hung then the request sense will
	 * fail too so we need to check the status again here. If the 
	 * request sense worked then we've got all we need in the sense
	 * buffer so now we just have to decode it.
	 */

	if (r->status == ST_GOOD && (e->errorcode & 0xf0) == 0x70) {

	    /*
	     * Check to see if the info bytes are valid. They usually
	     * will be for I/O commands. I'm not sure on the recovery
	     * commands. If they are good then print out an informative
	     * Message.
	     */

	    if (e->errorcode & 0x80) {
		info = e->info;
		flip(info);
		cmn_err(CE_WARN,
			"haisd: (%d, 0x%x) %s on %s at block %ld.",
			major(r->dev),
			minor(r->dev),
			sensestr[e->sensekey & 0x0f],
			lastscsicmd,
			info - blkdev_base(c->c_plim, r->dev));

		/*
		 * Set xfrsz from info. This still may be wrong (recovered
		 * errors or false check conditions)
		 */

		if (xfrsz)
		    *xfrsz = info - c->c_blkno;
	    } else {
		cmn_err(CE_WARN,
			"haisd: (%d, 0x%x) %s on %s.",
			major(r->dev),
			minor(r->dev),
			sensestr[e->sensekey & 0x0f],
			lastscsicmd);
	    }

	    /*
	     * If this is a recovered error or a false check condition.
	     * just assume we got it all and just return success. The Seagate
	     * Manual says that the info bytes will be set to the "first" 
	     * offending block if multiple blocks offend. This could
	     * be a problem if other drives do this differently or if multiple
	     * blocks get recovered errors.
	     */
	      
	    if ((e->sensekey & 0x0f) <= 1) {
		if (xfrsz)
		    *xfrsz = c->c_blkcnt;
		return 0;
	    } else
		return 1;
	} else {
	    cmn_err(CE_WARN, 
		"haisd: (%d, 0x%x) %s failed: status check = 0x%x",
		    major(r->dev), minor(r->dev), lastscsicmd, r->status);
	}
    } else
	cmn_err(CE_WARN, 
		"haisd: (%d, 0x%x) %s failed: status 0x%x.",
		major(r->dev), minor(r->dev), lastscsicmd, r->status);
    return 1;
}

#if __USE_PROTO__
LOCAL int sdrecover(register sdctrl_p c)
#else
LOCAL int
sdrecover(c)
register sdctrl_p c;
#endif
{
    register srb_p r = &(c->c_srb);
    g0cmd_p g0 = &(r->cdb. g0);
    g1cmd_p g1 = &(r->cdb. g1);

    memset(&(r->cdb), 0, sizeof(r->cdb));
    switch (++(c->c_tries)) {
    case 1:			/* Test unit ready. */
	set_opcode(g0->opcode, TEST_UNIT_READY);
	break;
    case 3:			/* Seek to (target - 1). */
	set_opcode(g1->opcode, G1SEEK);
	g1->lba = c->c_blkno - 1;
	flip(g1->lba);
	break;
    case 5:			/* Seek to (target + 1). */
	set_opcode(g1->opcode, G1SEEK);
	g1->lba = c->c_blkno + 1;
	flip(g1->lba);
	break;
    case 2:			/* Rezero unit. */
    case 4:			/* Rezero unit. */
    case 6:			/* Rezero unit. */
	set_opcode(g0->opcode, REZERO_UNIT);
	break;
    default:
	cmn_err(CE_WARN, 
		"haisd: (%d, 0x%x) %s failed at block %ld resetting device.",
		major(r->dev), minor(r->dev), lastscsicmd,
		c->c_blkno - blkdev_base(c->c_plim, r->dev));
	cmn_err(CE_CONT,
		"...haisd: Expect unit attention on next operation.\n");
	resetdevice(c->c_haft, r->target);
        return 0;
    }
    return 1;
}

#if __USE_PROTO__
LOCAL void sdmachine(register sdctrl_p c)
#else
LOCAL void 
sdmachine(c)
register sdctrl_p c;
#endif
{
    int 	done;
    int		erf;
    size_t	xfrsz;
    unsigned    reqtime;

    for (done = 0 ; !done ; ) {
	switch (c->c_state) {
	case SD_INIT:
	    cmn_err(CE_PANIC, "haisd: Improper state in sdmachine().");
	    break;

	case SD_IDLE:
	    if (!c->c_actf)
		done = 1;
	    else if (sdmksrb(c, &(c->c_srb)))
		set_state(c, SD_STARTSCMD);
	    break;

	case SD_STARTSCMD:
	    set_state(c, SD_SCMDWAIT);
	    c->c_starttime = lbolt;
	    startscsi(c->c_haft, &(c->c_srb));
	    set_state(c, SD_SCMDFINISH);
            break;

        case SD_SCMDWAIT:
	    return;

        case SD_SCMDFINISH:

            /*
             * Status is still pending so the transfer hasn't finished 
             * yet. We can leave here and pick up when the transfer completes.
	     */

	    if (c->c_srb. status == ST_PENDING) 
		done = 1;
	    
	    /* 
	     * Transfer is complete. Check for an error and get the number of
	     * blocks that actually transfered. Handle partial transfers under
	     * the error code. Take note that sdpost properly handles partial
	     * block transfers.
	     */

	    else {
		erf = sderror(c, &xfrsz);
		if (c->c_reqcnt > 1)
		    sdmscheckout(c, xfrsz * BSIZE);
		else
		    sddone(c, xfrsz * BSIZE, 0);

		reqtime = lbolt - c->c_starttime;
#if __ALLOW_STATS
		c->c_stats. hd_reqcnt++;
		c->c_stats. hd_blkcnt += xfrsz;
                c->c_stats. hd_time += (reqtime > 0) ? reqtime : -reqtime;
                c->c_stats. hd_errcnt += (erf != 0);
#endif
		
		set_state(c, (erf) ? SD_RECOVER : SD_IDLE);
	    }
            break;

        case SD_RECOVER:
	    if (!sdrecover(c)) {
		sddone(c, 0, EIO);
		set_state(c, SD_IDLE);
	    }
	    else
		set_state(c, SD_RSTARTSCMD);
            break;

	case SD_RSTARTSCMD:
	    set_state(c, SD_RSCMDWAIT);
	    startscsi(c->c_haft, &(c->c_srb));
	    set_state(c, SD_RSCMDFINISH);
            break;

        case SD_RSCMDWAIT:
	    return;

        case SD_RSCMDFINISH:
	    if (c->c_srb. status == ST_PENDING)
		done = 1;
	    else if (sderror(c, NULL)) {
		c->c_tries = RETRYLIMIT;
		set_state(c, SD_RECOVER);
	    }
	    else
		set_state(c, SD_IDLE);
            break;
        }
    }
}

#if __USE_PROTO__
LOCAL void sdblock(register buf_t *bp)
#else
LOCAL void
sdblock(bp)
register buf_t *bp;
#endif
{
    register sdctrl_p c;
    int s;
    
    if (!bp) {
	cmn_err(CE_WARN, "haisd: Null buffer address.");
	return;
    }
    
    bp->b_resid = bp->b_count;
    bp->b_actf = bp->b_actl = NULL;
    
    if (major(bp->b_dev) != SCSIMAJOR) {
	bioerror(bp, ENXIO);
	bdone(bp);
	return;
    }
    
    c = sddevs[tid(bp->b_dev)];
    if ((bp->b_resid % BSIZE) != 0 ||
	bp->b_bno > blkdev_size(c->c_plim, bp->b_dev)) {
	bioerror(bp, ENXIO);
	bdone(bp);
	return;
    }
    
    s = sphi();
    if (!c->c_actf)
	c->c_actf = c->c_actl = bp;
    else {
	c->c_actl->b_actf = bp;
	c->c_actl = bp;
    }
    spl(s);
    while (c->c_actf != NULL && c->c_state == SD_IDLE)
	sdmachine(c);
}

/***********************************************************************
 * sdread()
 *
 * Read entry point for SCSI DASD devices.
 */

#if __USE_PROTO__
LOCAL void sdread(dev_t dev, IO *iop)
#else
LOCAL void
sdread(dev, iop)
dev_t	dev;
IO	*iop;
#endif
{
    ioreq (NULL, iop, dev, BREAD, BFIOC | BFBLK | BFRAW);
}

/***********************************************************************
 * sdwrite()
 *
 * Write entry point for SCSI DASD devices.
 */
#if __USE_PROTO__
LOCAL void sdwrite(dev_t dev, IO *iop)
#else
LOCAL void
sdwrite(dev, iop)
dev_t	dev;
IO	*iop;
#endif
{
    ioreq (NULL, iop, dev, BWRITE, BFIOC | BFRAW);
}

/***********************************************************************
 * sdioctl()
 *
 * I/O Control for DASD's right now. The options here are for the
 * self configuring SCSI kernel.
 */

#if __USE_PROTO__
LOCAL void sdioctl(register dev_t dev, register int cmd, char *vec)
#else
LOCAL void
sdioctl(dev, cmd, vec)
register dev_t	dev;
register int	cmd;
char		*vec;
#endif
{
    sdctrl_p	c = sddevs[tid(dev)];
    hdparm_t	hdp;
    
    switch (cmd) {
    case HDGETA:
	if ((dev & SPECIAL) == 0)
	    set_user_error(ENXIO);
	else {
	    haihdgeta(tid(dev), &hdp, c->c_plim[0]. p_size);
	    kucopy(&hdp, vec, sizeof(hdparm_t));
	}
	break;
	
    case HDSETA:
	if ((dev & SPECIAL) == 0)
	    set_user_error(ENXIO);
	else {
	    if (ukcopy(vec, &hdp, sizeof(hdparm_t)))
		haihdseta(tid(dev), &hdp);
	}
	break;

#if __ALLOW_STATS
    case HDGETPERF:
        kucopy(&(c->c_stats), vec, sizeof(hdstats_t));
        break;

    case HDGETPERFCLR:
        kucopy(&(c->c_stats), vec, sizeof(hdstats_t));
        sdclrstat(&(c->c_stats));
        break;

    case HDCLRPERF:
        sdclrstat(&(c->c_stats));
        break;
#endif
	
    default:
	set_user_error (ENXIO);
	break;
    }
}	/* sdioctl() */

/* End of file */
