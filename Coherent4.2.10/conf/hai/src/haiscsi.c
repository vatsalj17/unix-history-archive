/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1994 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */
/***********************************************************************
 *  Module: haiscsi.c
 *
 *  This is the interface between the Coherent kernel, the host
 *  adapter module and the SCSI device modules. It's just a simple
 *  dispatcher that determines which routine to call based upon the
 *  calling device's Target ID.  The target ID should be set in bits
 *  4-6 of the device's minor number.  
 *
 *  Copyright (c) 1993, Christopher Sean Hilton, All Rights Reserved.
 *
 *  Last Modified: Wed Jun 15 11:17:00 1994 by [chris]
 */

/***********************************************************************
 * $Id$
 *
 * $Log$
 */

static char rcsid[] = "@(#) haiscsi.c $Revision$";
 
#include <sys/coherent.h>

#include <stddef.h>

#include <errno.h>
#include <sys/buf.h>
#include <sys/cmn_err.h>
#include <sys/con.h>
#include <sys/haiioctl.h>
#include <sys/haiscsi.h>
#include <sys/io.h>
#include <sys/sched.h>
#include <sys/stat.h>
#include <sys/uproc.h>

#include <coh/proc.h>

#define LOCAL	static
#if __GNUC__
#define Register
#else
#define Register register
#endif

/* Configurable variables - see /etc/conf/hai/Space.c. */
extern int HAI_DISK;
extern int HAI_TAPE;
extern int HAI_CDROM;

/*
 * HAI_HAINDEX is the configurable variable to select the host adapter that
 * This variable is the index into the master host adapter table that is
 * set up in the scsi_load routine.
 *
 * 0 = Adaptec 154x.
 * 1 = Seagate/Future Domain 8-bit.
 */

extern int HAI_HAINDEX;		/* Host adapter index */

/*
 * Device function tables.
 */

extern dca_p    sddca;		/* Fixed disk control routines */
extern dca_p    ctdca;		/* Cartridge tape control routines */

#if _HAICD_OK
extern dca_p    cdrdca;		/* CD-ROM control routines */
#endif

dca_p  mdca [MAXDEVS];		/* Initialized by setup_mdca(). */

/*
 * Host adapter function tables. If you add a host adapter, the
 * function table pointer goes here.
 * HAI_HAINDEX refers to the type of host adapter. To add 
 * host adapters add an extern pointer to its function table here
 * and then append that pointers address to the end of this table.
 */

extern haft_t	* _154x_haftp;	/* Adaptec AHA-154x and compatibles */
extern haft_t	* ss_haftp;	/* Future Domain TMC-950/9C50 based hosts */

static haft_t	* haftp;	/* Host adapter function table */


static char *errstr[] = {
	"No sense",
	"Recovered error",
	"Not ready",
	"Medium error",
	"Hardware error",
	"Illegal request",
	"Unit attention",
	"Data protect",
	"Blank check",
	"Vendor unique error",
	"Copy Aborted",
	"Aborted command",
	"Equal",
	"Volume overflow",
	"Miscompare",
	"Reserved"
};

/* char iofailmsg[] = "%s: status(0x%x)"; */

/***********************************************************************
 *  int scsi_open(dev_t dev, int mode)
 *
 *  Open a device on the SCSI bus at target ID: tid(dev). This is
 *  Accomplished by calling the open routine at mdca[tid(dev)]->d_open
 */

#if __USE_PROTO__
LOCAL void scsi_open(Register dev_t dev, unsigned mode)
#else
LOCAL void 
scsi_open(dev, mode)
Register dev_t   dev;
unsigned mode;
#endif
{
	Register dca_p d = mdca[tid(dev)];

	if (!haftp->hf_present || !d)
		set_user_error (EINVAL);
	else
		(* d->d_open) (dev, mode);
}   /* scsi_open() */

/***********************************************************************
 *  void scsi_close()
 *
 *  Close entry point for all devices at major index SCSIMAJOR.
 */

#if __USE_PROTO__
LOCAL void scsi_close(Register dev_t dev)
#else
LOCAL void 
scsi_close(dev)
Register dev_t dev;
#endif
{
	Register dca_p d = mdca[tid(dev)];

	if (!haftp->hf_present || !d)
		set_user_error (EINVAL);
	else
		(* d->d_close)(dev);
}   /* scsi_close() */

/***********************************************************************
 *  void scsi_block()
 *
 *  Block Entry point.
 */

#if __USE_PROTO__
LOCAL void scsi_block(Register BUF *bp)
#else
LOCAL void scsi_block(bp)
Register BUF *bp;
#endif
{
	Register dca_p d = mdca[tid(bp->b_dev)];

	if (!haftp->hf_present || !d) {
		bp->b_resid = bp->b_count;
		bp->b_flag |= BFERR;
		bdone(bp);
	}
	else
		(*(d->d_block))(bp);
}   /* scsi_block() */

/***********************************************************************
 *  void scsi_read()
 *
 *  Read entry point.
 */

#if __USE_PROTO__
LOCAL void scsi_read(Register dev_t dev, Register IO *iop)
#else
LOCAL void scsi_read(dev, iop)
Register dev_t  dev;
Register IO  	*iop;
#endif
{
	Register dca_p d = mdca [tid(dev)];

	if (!haftp->hf_present || !d)
		set_user_error (EINVAL);
	else
		(* d->d_read)(dev, iop);
}   /* scsi_read() */

/***********************************************************************
 *  int scsi_write()
 *
 *  Write entry point.
 */

#if __USE_PROTO__
LOCAL void scsi_write(Register dev_t dev, IO  *iop)
#else
LOCAL void scsi_write(dev, iop)
Register dev_t  dev;
IO  *iop;
#endif
{
	Register dca_p d = mdca[tid(dev)];
	if (!haftp->hf_present || !d)
		set_user_error (EINVAL);
	else
		(* d->d_write)(dev, iop);
}   /* scsi_write() */

/***********************************************************************
 *  scsi_ioctl()
 *
 *  IO Control entry point.
 */

#if __USE_PROTO__
LOCAL void scsi_ioctl(Register dev_t dev, int cmd, char	vec[])
#else
LOCAL void scsi_ioctl(dev, cmd, vec)
Register dev_t  dev;
int		cmd;
char		vec[];
#endif
{
	Register dca_p d = mdca[tid(dev)];

	if (!haftp->hf_present || !d)
		set_user_error (EINVAL);
	else
		(*d->d_ioctl)(dev, cmd, vec);
}   /* scsi_ioctl() */

/*
 * scsi_timer() -- Timeout service entry point.
 *
 * Called once per second at the request of the host adapter driver. 
 * This routine just downcalls the timer routine for the host adapter.
 * or turns off timer processing if there isn't a timer routine.
 */

#if __USE_PROTO__
LOCAL void scsi_timer(void)
#else
LOCAL void 
scsi_timer()
#endif
{
    if (haftp->hf_timer)
	(*(haftp->hf_timer))();
    else
	drvl[SCSIMAJOR]. d_time = 0;
}

/***********************************************************************
 * setup_mdca
 *
 * Load mdca table based on globals HAI_DISK and HAI_TAPE.
 */

#if __USE_PROTO__
LOCAL void setup_mdca(void)
#else
LOCAL void 
setup_mdca()
#endif
{
	int	id,
		mask;

	for (id = 0; id < 8; id ++) {
		mask = 1 << id;
		if (HAI_DISK & mask)
			mdca[id] = sddca;
		if (HAI_TAPE & mask)
			mdca[id] = ctdca;
#if _HAICD_OK
		if (HAI_CDROM & mask)
			mdca[id] = cdrdca;
#endif
	}
}

/***********************************************************************
 *  scsi_load()
 *
 *  Load Entry point.
 */

#if __USE_PROTO__
LOCAL int scsi_load(void)
#else
LOCAL int 
scsi_load()
#endif
{
	Register int	id;
	Register dca_p  d;

	cmn_err(CE_CONT, "Host Adapter Independent SCSI Driver v1.9\n");

	/* Determine which host adapter is in use. */
	switch (HAI_HAINDEX) {
	case HAI_154X_INDEX:
		haftp = _154x_haftp;
		break;
	case HAI_SS_INDEX:
		haftp = ss_haftp;
		break;
	default:
		cmn_err (CE_WARN, "haiscsi - unrecognized host adapter type");
		return 0;
	}

	/* Make sure the selected host adapter has a function table. */
	if (!haftp) {
	    	cmn_err(CE_WARN,
		  "Host adapter configuration invalid. Initialization failed.");
		return 0;
        }

	haload(haftp);
	if (!haftp->hf_present) {
		cmn_err(CE_WARN, "Host Adapter Initialization failed.");
		return 0;
        }

	setup_mdca();

	for (id = 0; id < MAXDEVS; ++id) {
		if ((d = mdca[id]) && d->d_load) {
			if (!(*(d->d_load))(haftp, id)) {
				cmn_err (CE_WARN,
				  "SCSI target device at id %d did not load.",
				  id);
			}
		}
	}
	return 1;
}   /* scsi_load() */

/***********************************************************************
 *  scsi_unload()
 *
 *  SCSI unload routine.
 */

#if __USE_PROTO__
LOCAL int scsi_unload(void)
#else
LOCAL int 
scsi_unload()
#endif
{
	Register int	id;
	Register dca_p  d;

	for (id = 0; id < MAXDEVS; ++id)
		if ((d = mdca[id]) && d->d_unload)
			(*(d->d_unload))(id);

	return 1;
}   /* scsi_unload() */

/***********************************************************************
 *  Utility Routines
 */

/***********************************************************************
 *  char *swapbytes()
 *
 *  Swap bytes in an object from big to little endian or vice versa.
 */

#if __USE_PROTO__
char *swapbytes(void *mem, size_t size)
#else
char *swapbytes(mem, size)
char	*mem;
size_t  size;
#endif
{
	Register char *p = mem;
	Register char *q = p + size - 1;

	while (q > p) {
		*p ^= *q;
		*q ^= *p;
		*p ^= *q;
		p++;
		q--;
	}
	return mem;
}   /* swapbytes() */

/***********************************************************************
 *  cpycdb()
 *
 *  Copy a SCSI Command/Data Block. Return the number of bytes copied.
 */

#if __USE_PROTO__
int cpycdb(Register cdb_p dst, Register cdb_p src)
#else
int cpycdb(dst, src)
Register cdb_p dst;
Register cdb_p src;
#endif
{
		switch (src->g0.opcode & GROUPMASK) {
		case GROUP0:
			memcpy(dst, src, sizeof(g0cmd_t));
			return sizeof(g0cmd_t);
		case GROUP1:
		case GROUP2:
			memcpy(dst, src, sizeof(g1cmd_t));
			return sizeof(g1cmd_t);
		case GROUP5:
			memcpy(dst, src, sizeof(g5cmd_t));
			return sizeof(g5cmd_t);
		default:
			return 0;
	}
}   /* cpycdb() */

/***********************************************************************
 *  dumpmem()
 *
 *  Dump memory from (p) for (s) bytes.
 */

static char hexchars[] = "0123456789abcdef";
static char linebuf[] = "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
	"| ................\n";

#if __USE_PROTO__
void dumpmem(char m[], unsigned char *p, size_t	s)
#else
void
dumpmem(m, p, s)
char		m[];
unsigned char *	p;
size_t		s;
#endif
{
	Register int i;
	char *l;

	if (m)
		cmn_err(CE_CONT, "!%s", m);
	cmn_err(CE_CONT, "! (0x%x)\n", (unsigned) p);
	memset(linebuf, ' ', sizeof(linebuf) - 2);
	linebuf[48] = '|';
	l = linebuf;
	for (i = 0; i < s; ++i, ++p) {
		*l++ = hexchars[(*p >> 4) & 0x0f];
		*l++ = hexchars[*p & 0x0f];
		*l++ = ' ';
		linebuf[50 + (i & 15)] = (*p >= ' ' && *p <= '~') ? *p : '.';
		if ((i & 15) == 15) {
			cmn_err(CE_CONT, "!%s", linebuf);
			memset(linebuf, ' ', sizeof(linebuf) - 2);
			linebuf[48] = '|';
			l = linebuf;
		}
	}
	if ((s & 15) != 0)
		cmn_err(CE_CONT, "!%s", linebuf);
}   /* dumpmem() */

/***********************************************************************
 *  scsidone()
 *
 *  Wake up processes sleeping on SRB (r).
 */

#if __USE_PROTO__
LOCAL void scsidone(Register srb_p r)
#else
LOCAL void
scsidone(r)
Register srb_p	r;
#endif
{
	wakeup((char *)(&(r->status)));
}

/***********************************************************************
 *  reqsense()
 *
 *  Issue a request sense command device loaded into the target/lun
 *  fields of the given srb r.  Uses v_sleep().
 */

#if __USE_PROTO__
void reqsense(haft_t * haft, Register srb_p r)
#else
void
reqsense(haft, r)
haft_t * haft;
Register srb_p r;
#endif
{
	int		s;
	unsigned short  status;
	unsigned short  tries;
	unsigned short  timeout;
	bufaddr_t	buf;
	unsigned short  xferdir;
	void 		(*cleanup) __PROTO((srb_p));
	cdb_t		cdb;

	if (r->status == ST_CHKCOND) {
		status = ST_CHKCOND;
		tries = r->tries;
		timeout = r->timeout;
		memcpy(&buf, &(r->buf), sizeof(bufaddr_t));
		xferdir = r->xferdir;
		cleanup = r->cleanup;
		memcpy(&cdb, &(r->cdb), sizeof(cdb_t));

		r->timeout = 4;
		r->tries = 10;
		r->buf.space = KRNL_ADDR;
		r->buf.addr.caddr = (caddr_t) r->sensebuf;
		r->buf.size = sizeof(r->sensebuf);
		r->xferdir = DMAREAD;
		r->cleanup = scsidone;
		memset(&(r->cdb), 0, sizeof(cdb_t));
		r->cdb.g0.opcode = 0x03;
		r->cdb.g0.lun_lba = (r->lun << 5);
		r->cdb.g0.xfr_len = r->buf.size;
		s = sphi();
		startscsi(haft, r);
		while (r->status == ST_PENDING) {
			if (x_sleep((char *)(&(r->status)), pritape,
			  slpriSigCatch, "reqsense")) {
				set_user_error (EINTR);
				status = ST_USRABRT;
				break;
			}
		}
		spl(s);

		r->tries = tries;
		r->timeout = timeout;
		memcpy(&(r->buf), &buf, sizeof(bufaddr_t));
		r->xferdir = xferdir;
		r->cleanup = cleanup;
		memcpy(&(r->cdb), &(cdb), sizeof(cdb_t));
		r->status = status;
	}
}   /* reqsense() */

/***********************************************************************
 *  doscsi()
 *
 *  An alternative to startscsi() which handles everything including
 *  any request sense commands necessary if the command failed. All
 *  information is returned in given srb. Note:  you can only use
 *  this routine when the u structure for a process is available (from
 *  an open, close, read, write, or ioctl routine). Since this calls
 *  sleep it will screw things up something fierce if you call it from
 *  a load, unload, block, timer or interrupt routine. Also note
 *  that some host adapters do the sense part this automatically.
 */
#if __USE_PROTO__
void doscsi(haft_t * haft,
	    srb_p r, 
	    int retrylimit,
	    int schedPri, 
	    int sleepPri,
	    char reason[])
#else
void
doscsi(haft, r, retrylimit, schedPri, sleepPri, reason)
haft_t *	haft;
srb_p		r;
int		retrylimit;
int		schedPri;
int		sleepPri;
char		reason[];
#endif
{
	int	 s;

	r->cleanup = scsidone;
	for (r->tries = 0; r->tries < retrylimit; ) {
		if (startscsi(haft, r)) {
			s = sphi();
			while (r->status == ST_PENDING) {
				if (x_sleep ((char *)(& r->status), schedPri,
				  sleepPri, reason)) {
					abortscsi(haft, r);
					r->status = ST_USRABRT;
					set_user_error (EINTR);
				}
			}
			spl(s);
			if (r->status == ST_GOOD
			  || r->status == ST_USRABRT
			  || r->status == ST_DRVABRT)
				return;

			if (r->status == ST_CHKCOND)
				reqsense(haft, r);
		}
		else
			r->status = ST_TIMEOUT;
	}
}   /* doscsi() */

/***********************************************************************
 *  printsense()
 *
 *  Print out the results in the given sense buffer. This is done
 *  in English, almost.
 */

#if __USE_PROTO__
void printsense(Register dev_t dev, Register char msg[], Register extsense_p e)
#else
void
printsense(dev, msg, e)
Register dev_t	  dev;
Register char	   *msg;
Register extsense_p e;
#endif
{
	long info;
	if ((e->errorcode & 0x70) != 0x70)
		cmn_err(CE_WARN,
			"(%d ,%d) %s: Bad sensekey",
			major(dev),
			minor(dev),
			msg);
	else {
		if ((e->errorcode & 0x80) != 0x80)
			cmn_err(CE_WARN,
				"(%d, %d) %s: %s - key: (0x%x)",
				major(dev),
				minor(dev),
				msg,
				errstr[e->sensekey & 0x0f],
				(e->sensekey & 0xe0));
		else {
			info = (long) e->info;
			flip(info);
			cmn_err(CE_WARN,
				"(%d, %d) %s: %s - addr: %d key: (0x%x)",
				major(dev),
				minor(dev),
				msg,
				errstr[e->sensekey & 0x0f],
				info,
				(e->sensekey & 0xe0));
		}
	}
}   /* printsense() */

/***********************************************************************
 *  printerror()
 *  
 *  Print an error after command completion. Be silent if the command
 *  was aborted by the user.
 */

#if __USE_PROTO__
int printerror(Register srb_p r, Register char msg[])
#else
int
printerror(r, msg)
Register srb_p r;
Register char *msg;
#endif
{
	if (r->status == ST_USRABRT)
		return 0;
	else {
		if (r->status != ST_CHKCOND)
			cmn_err(CE_WARN,
				"(%d, %d) %s: status(0x%x)",
				major(r->dev),
				minor(r->dev),
				msg,
				r->status);
		else
			printsense(r->dev, msg, (extsense_p) r->sensebuf);
		return 1;
	}
}   /* printerror() */

/***********************************************************************
 *  haiioctl()  --	  I/O Controls common to all devices.
 *  
 *  This function provides I/O Control functions common to all SCSI
 *  devices. The chain of operation should be as follows:
 *  
 *  You:
 *	  1)  Make sure that the device is in an appropriate state to
 *		  perform the I/O Control. (It might not be a good idea to
 *		  format the disk drive with the root partition).
 *  
 *	  2)  Call haiioctl() with your srb and cmd from I/O Control.
 */

#if __USE_PROTO__
void haiioctl(Register srb_p r, /* Device's srb */
	      Register int cmd,	/* Command to do */
	      Register char vec[]) /* Additional information (if needed) */
#else
void
haiioctl(r, cmd, vec)
Register srb_p  r;		/* Device's srb */
Register int	cmd;		/* Command to do */
Register char   *vec;		/* Additional information (if needed) */
#endif
{
	haiusercdb_t h;
	
	switch (cmd) {
	case HAIINQUIRY:
	case HAIMDSNS0:
	case HAIMDSLCT0:
	case HAIMDSNS2:
	case HAIMDSLCT2:
		set_user_error (ENXIO);
		return;

	case HAIUSERCDB:
		if (!ukcopy(vec, &h, sizeof(haiusercdb_t)))
			return;
		r->buf.space = USER_ADDR;
		r->buf.addr.caddr = vec + sizeof(haiusercdb_t);
		r->buf.size = h.buflen;
		r->xferdir = h.xferdir;
		r->timeout = h.timeout;
		memcpy(&(r->cdb), &(h.cdb), sizeof(cdb_t));
		doscsi(haftp, r, 1, pritty, slpriSigCatch, "haiioctl");
		putusd((((haiusercdb_p) vec)->status), r->status);
		putusd((((haiusercdb_p) vec)->hastat), r->hastat);
		kucopy(r->sensebuf, ((haiusercdb_p) vec)->sensebuf,
		  HAI_SENSELEN);
		if (r->status != ST_GOOD)
			set_user_error (ENXIO);
		return;
	
	default:
		set_user_error (ENXIO);
		break;
	}
}   /* haiioctl() */

/***********************************************************************
 *  hainonblk()	 --  Block entry point for devices that shouldn't
 *					  have block entry points.
 *  
 *  Since this is a multiplexing driver and some devices behind it
 *  will have block entry points and some shouldn't. ALL do.
 */

#if __USE_PROTO__
void hainonblk(BUF *bp)
#else
void
hainonblk(bp)
BUF	*bp;
#endif
{
	bp->b_flag |= BFERR;
	bdone (bp);
}   /* hainonblk() */

CON scsicon = {
	DFBLK | DFCHR,
	SCSIMAJOR,
	scsi_open,		/* Open entry point */
	scsi_close,		/* Close entry point */
	scsi_block,		/* Block entry point. */
	scsi_read,		/* Read Entry point */
	scsi_write,		/* write entry point */
	scsi_ioctl,		/* IO control entry point */
	NULL,			/* No powerfail entry (yet?) */
	scsi_timer,		/* timeout entry point */
	scsi_load,		/* Load entry point */
	scsi_unload,		/* Unload entry point */
	NULL			/* No poll entry yet either. */
};
/* End of file: haiscsi.c */
