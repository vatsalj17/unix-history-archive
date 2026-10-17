/*
 *  haicd.c --	  Device module for CD-ROM.
 *
 *  Copyright (c) 1993, Christopher Sean Hilton - All rights reserved
 *
 * $Id$
 *
 * $Log$
 */

static char rcsid[] = "#(@) haicd.c $Revision$";

#define LOCAL		static
#define Register	register

#define _KERNEL	1

#include <errno.h>
#include <stddef.h>
#include <sys/cdrom.h>
#include <sys/cmn_err.h>
#include <sys/coherent.h>
#include <sys/file.h>
#include <sys/haiscsi.h>
#include <sys/inode.h>
#include <sys/sched.h>
#include <sys/stat.h>

/***********************************************************************
 *  SCSI Command Opcodes.
 */

#define INQUIRY		0x12
#define STARTSTOPUNIT	0x1b
#define	  IMMEDIATE	0x0001
#define	  STARTUNIT	0x0002
#define	  EJECT		0x0004
#define READCAPACITY	0x25
#define G1READ		0x28
#define READSUBCHNL	0x42
#define READTOC		0x43
#define PLAYMSF		0x47
#define PLAYTRKIND	0x48
#define	PAUSERESUME	0x4b

#pragma align 1

typedef struct readcap_s *readcap_p;

typedef struct readcap_s {
    unsigned long   blockcount;
    unsigned long   blocksize;
} readcap_t;

typedef struct msf_s *msf_p;

typedef struct msf_s {
    unsigned char   reserved,
                    minutes,
                    seconds,
                    frame;
} msf_t;

typedef struct toc_entry_s *toc_entry_p;

typedef struct toc_entry_s {
	unsigned char   reserved0;
	unsigned char   adrcontrol;
	unsigned char   tracknumber;
	unsigned char   reserved1;
	union {
		msf_t		msf;
		unsigned long   lba;
	} address;
} toc_entry_t;

typedef struct toc_data_s *toc_data_p;

typedef struct toc_data_s {
    unsigned short  datalength;
    unsigned char   firsttrack;
    unsigned char   lasttrack;
    toc_entry_t     toc_entries[100];
} toc_data_t;

typedef struct subchnlq_s *subchnlq_p;

typedef struct subchnlq_s {
	unsigned char	sq_reserved,
			sq_audiostatus;
	unsigned short	sq_datalength;
	unsigned char	sq_dataformat,
			sq_adrcontrol,
			sq_track,
			sq_index;
	union {
		msf_t		msf;		
		unsigned long 	lba;
	}		sq_absaddr;
	union {
		msf_t		msf;		
		unsigned long 	lba;
	}		sq_reladdr;
} subchnlq_t;

#pragma align

/***********************************************************************
 *  CD-ROM control structure.
 */

typedef struct cdrctrl_s *cdrctrl_p;

typedef struct cdrctrl_s {
    	haft_t		* haft;		/* Host adapter function table */
	short	   	inuse;		/* In use flag */
	srb_t	  	srb;		/* SCSI request block: this device */
	readcap_t	cdrsize;	/* Results of Read Capacity Command */
	toc_data_p	toclba;		/* Results of Read TOC Command (lba) */
	toc_data_p	tocmsf;		/* Results of Read TOC Command (msf) */
} cdrctrl_t;

cdrctrl_p cdrdevs[MAXDEVS] = {
	NULL,			   /* ID 0 */
	NULL,			   /* ID 1 */
	NULL,			   /* ID 2 */
	NULL,			   /* ID 3 */
	NULL,			   /* ID 4 */
	NULL,			   /* ID 5 */
	NULL,			   /* ID 6 */
	NULL			   /* ID 7 */
};

LOCAL void	cd_open		__PROTO((dev_t, int));
LOCAL void	cd_close	__PROTO((dev_t));
LOCAL void	cd_ioctl	__PROTO((dev_t, int, char *));
LOCAL int	cd_load		__PROTO((haft_t *, int));
LOCAL void	cd_read		__PROTO((dev_t, IO *));

static dca_t dca = {
	cd_open,	/* Open */
	cd_close,	/* Close */
	hainonblk,	/* Block */
	cd_read,	/* Read */
	NULL,		/* Write */
	cd_ioctl,	/* Ioctl */
	cd_load,	/* Load */
	NULL,		/* Unload */
	NULL		/* Poll */
};

dca_p cdrdca =&dca;

extern int ukcopy	__PROTO((vaddr_t, void *, size_t));
extern int kucopy	__PROTO((void *, vaddr_t, size_t));

/***********************************************************************
 *  cd_load() --	   Load entry point for CD-ROM drives.
 *  
 *  Allocate space for the control structure and the CD-ROM drive's
 *  buffer (very small buffer).
 */

#if __USE_PROTO__
LOCAL int cd_load(haft_t * haft, int id)
#else
LOCAL int
cd_load(haft, id)
haft_t * haft;
int	 id;
#endif
{
	cdrctrl_p c;

	if (!haft) {
	    cmn_err(CE_WARN,
		    "haicd: No host adapter function table - load failed");
	    return 0;
	}

	c = kalloc(sizeof(cdrctrl_t));
	if (!c) {
		cmn_err(CE_WARN, "CD-ROM Module load failed, no memory.");
		return 0;
	}
	memset(c, 0, sizeof(cdrctrl_t));
	c->haft = haft;
	c->srb.target = id;
	c->srb.lun = 0;
	cdrdevs[id] = c;
	
	cmn_err(CE_CONT, "%d: HAI CD-ROM device module: v1.9\n", id);
	return 1;
}   /* cd_load() */

/***********************************************************************
 *  checkmedia() --	 Check for new media in the drive.
 *  
 *  Use the Test Unit Read SCSI Command to check for new or any media
 *  in the drive.
 */
#if __USE_PROTO__
LOCAL int checkmedia(cdrctrl_p c)
#else
LOCAL int
checkmedia(c)
cdrctrl_p  c;
#endif
{
	register srb_p r = &(c->srb);
	
	r->buf.space = PHYS_ADDR | WATCH_REQACK;
	r->buf.ba_phys = NULL;
	r->buf.size = 0;
	r->xferdir = 0;
	r->timeout = 6;
	memset(&(r->cdb.g0), 0, sizeof(cdb_t));	/* Test Unit Ready */
	memset(r->sensebuf, 0, sizeof(r->sensebuf));
	doscsi(c->haft, r, 4, pritape, slpriSigCatch, "cdrchkmda");
	return (r->status == ST_GOOD);
}   /* checkmedia() */

/***********************************************************************
 *  iscdromdev() --	 Check the Inquiry bytes to make sure this is
 *					  a CD-ROM Drive.
 *  
 *  Get the first 16 bytes of inquiry information for this drive to
 *  make sure that the device out there on the bus is a CD-ROM Device.
 *  Used during initializaion.
 */

#if __USE_PROTO__
LOCAL int iscdromdev(cdrctrl_p c)
#else
LOCAL int
iscdromdev(c)
cdrctrl_p  c;
#endif
{
	register srb_p  r = &(c->srb);
	unsigned char   inqbuf[16];

	memset(inqbuf, 0, sizeof(inqbuf));
	r->buf.space = KRNL_ADDR | WATCH_REQACK;
	r->buf.ba_virt = (vaddr_t) inqbuf;
	r->xferdir = DMAREAD;
	memset(&(r->cdb), 0, sizeof(cdb_t));
	r->cdb.g0.opcode = INQUIRY;
	r->cdb.g0.xfr_len = r->buf.size = sizeof(inqbuf);
	doscsi(c->haft, r, 4, pritape, slpriSigCatch, "cdropen");
	return (r->status == ST_GOOD && (inqbuf[0] & 0x1f) == 0x05);
}   /* iscdromdev() */

/***********************************************************************
 *  readcapacity() --   Read the Capacity of the CD in the drive.
 *  
 *  Use the Read Capacity command to get the number of blocks on the
 *  CD ROM and the size of each block. This information can be accessed
 *  by user programs through the HAICD_READCAP I/O Control command.
 */

#if __USE_PROTO__
LOCAL int readcapacity(cdrctrl_p c)
#else
LOCAL int
readcapacity(c)
cdrctrl_p  c;
#endif
{
	register srb_p r = &(c->srb);

	r->buf.space = KRNL_ADDR | WATCH_REQACK;
	r->buf.ba_virt = (vaddr_t) &(c->cdrsize);
	r->buf.size = sizeof(c->cdrsize);
	memset(&(r->cdb), 0, sizeof(cdb_t));
	r->cdb.g1.opcode = READCAPACITY;
	doscsi(c->haft, r, 4, pritape, slpriSigCatch, "cdropen");
	if (r->status != ST_GOOD)
		return 0;
	else {
		flip(c->cdrsize.blockcount);
		flip(c->cdrsize.blocksize);
		return 1;
	}
}   /* readcapacity() */

/***********************************************************************
 *  readtoclba() --		Read the Disk's Table of Contents.
 *  
 *  Read the CD's TOC into a kalloced buffer.  The buffer only stays
 *  around while the Drive is openned and is allocated just large enough
 *  to hold the TOC info for this disk. This information is returned
 *  by the HAICD_READTOC I/O control command.
 */

#if __USE_PROTO__
LOCAL toc_data_p readtoclba(cdrctrl_p c)
#else
LOCAL toc_data_p
readtoclba(c)
cdrctrl_p  c;
#endif
{
	register srb_p		r = &(c->srb);
	register int		entries;
	static toc_data_t   	toclba;
	toc_data_p		p;
	toc_entry_p		q;

	r->buf.space = KRNL_ADDR | WATCH_REQACK;
	r->buf.ba_virt = (caddr_t) &toclba;
	memset(&(r->cdb), 0, sizeof(cdb_t));
	r->cdb.g1.opcode = READTOC;
	r->cdb.g1.xfr_len = r->buf.size = sizeof(toclba);
	flip(r->cdb.g1.xfr_len);
	doscsi(c->haft, r, 4, pritape, slpriSigCatch, "cdropen");
	if (r->status != ST_GOOD)
		return NULL;

	flip(toclba.datalength);
	if (!toclba.datalength) {
	    cmn_err(CE_WARN, "haicd: Bad or empty table of contents.");
	    return NULL;
	}
	if (!(p = kalloc(toclba.datalength)))
		return NULL;

	entries = (toclba.datalength - 2) / sizeof(toc_entry_t);
	q = toclba.toc_entries;
	while (entries-- > 0) {
		flip(q->address. lba);
		++q;		
	}
	memcpy(p, &toclba, toclba.datalength + sizeof(toclba.datalength));
	return (c->toclba = p);
}   /* readtoclba() */

/***********************************************************************
 *  readtocmsf() --		Read the Disk's Table of Contents.
 *  
 *  Read the CD's TOC into a kalloced buffer.  The buffer only stays
 *  around while the Drive is openned and is allocated just large enough
 *  to hold the TOC info for this disk. This information is returned
 *  by the HAICD_READTOC I/O control command.
 */

#if __USE_PROTO__
LOCAL toc_data_p readtocmsf(cdrctrl_p c)
#else
LOCAL toc_data_p
readtocmsf(c)
cdrctrl_p  c;
#endif
{
	register srb_p		r = &(c->srb);
	static toc_data_t   	tocmsf;
	toc_data_p		p;

	r->buf.space = KRNL_ADDR | WATCH_REQACK;
	r->buf.ba_virt = (caddr_t) &tocmsf;
	memset(&(r->cdb), 0, sizeof(cdb_t));
	r->cdb.g1.opcode = READTOC;
	r->cdb.g1.lun = 2;
	r->cdb.g1.xfr_len = r->buf.size = sizeof(tocmsf);
	flip(r->cdb.g1.xfr_len);
	doscsi(c->haft, r, 4, pritape, slpriSigCatch, "cdropen");
	if (r->status != ST_GOOD)
		return NULL;

	flip(tocmsf.datalength);
	if (!tocmsf.datalength) {
	    cmn_err(CE_WARN, "haicd: Bad or empty table of contents.");
	    return NULL;
	}
	if (!(p = kalloc(tocmsf.datalength)))
		return NULL;

	memcpy(p, &tocmsf, tocmsf.datalength + sizeof(tocmsf.datalength));
	return (c->tocmsf = p);
}   /* readtocmsf() */

/***********************************************************************
 *  cd_open() --	   Open entry point for CD-ROM drives.
 *  
 *  Check to make sure that there is a CD-ROM in the drive and if so
 *  read the TOC.
 */

#if __USE_PROTO__
LOCAL void cd_open(dev_t dev, int mode)
#else
LOCAL void
cd_open(dev, mode)
dev_t		dev;
int		mode;
#endif
{
	register cdrctrl_p  c = cdrdevs[tid(dev)];
	register srb_p	  r;
	int				 s;

	s = sphi();
	if (!c) {
		set_user_error (ENXIO);
		goto exit;		
	}

	if (c->inuse || mode != IPR) {
		set_user_error (EACCES);
		goto exit;		
	}

	c->inuse = 1;
	r = &(c->srb);
	r->dev = dev;
	r->target = tid(dev);
	r->lun = lun(dev);

	if (!checkmedia(c)) {
		if (printerror(r, "cdrom: <Door open - Check media>"))
			set_user_error (ENXIO);
		goto openfailed;
	}

	if (!iscdromdev(c)) {
		if (r->status == ST_GOOD) {
			cmn_err(CE_WARN,
				"(%d, %d) haicd: Device is not a CD-ROM",
				major(r->dev),
				minor(r->dev));
			set_user_error (ENXIO);
		}
		else {
			if (printerror(r, "cdrom: <Error on inquiry>"))
				set_user_error (EIO);
		}
		goto openfailed;
	}

	if (!readcapacity(c)) {
		if (printerror(r, "cdrom: <Error on read capacity>"))
			set_user_error (EIO);
		goto openfailed;
	}

	if (!readtoclba(c)) {
		if (printerror(r, "cdrom: <Error on read table of contents>"))
			set_user_error (EIO);
		goto openfailed;
	}

	if (!readtocmsf(c)) {
		if (printerror(r, "cdrom: <Error on read table of contents>"))
			set_user_error (EIO);
		goto openfailed;
	}

	/*******************************************************************
	 *  Success!
	 */
	 goto exit;

openfailed:
	c->inuse = 0;
	
exit:
	spl(s);
}   /* cd_open() */

/***********************************************************************
 *  cd_close() --	  Close routine for CD-ROM drive.
 */

#if __USE_PROTO__
LOCAL void cd_close(dev_t dev)
#else
LOCAL void cd_close(dev)
dev_t   dev;
#endif
{
	register cdrctrl_p c = cdrdevs[tid(dev)];

	if (!c)
		set_user_error (ENXIO);
	else {
		kfree(c->tocmsf);
		kfree(c->toclba);
		c->inuse = 0;
	}
}   /* cd_close() */

/***********************************************************************
 *  cd_read() --	   Read entry point for CD-ROM Devices.
 *  
 *  Read a block from the CD-ROM drive.
 */

#if __USE_PROTO__
LOCAL void cd_read(dev_t dev, IO *iop)
#else
LOCAL void
cd_read(dev, iop)
dev_t   dev;
IO	  *iop;
#endif
{
	register cdrctrl_p	c = cdrdevs[tid(dev)];
	register srb_p		r = &(c->srb);
	register g1cmd_p	g1 = &(r->cdb.g1);
	unsigned long		startblock,
				blockcount;

	if (!c) {
		set_user_error (EINVAL);
		return;
	}

	if (iop->io_seek % c->cdrsize.blocksize || iop->io_ioc % c->cdrsize.blocksize) {
		set_user_error (EINVAL);
		return;			
	}	

	startblock = iop->io_seek / c->cdrsize.blocksize;
	if (startblock > c->cdrsize.blockcount) {
		set_user_error (EINVAL);
		return;			
	}
	blockcount = iop->io_ioc / c->cdrsize.blocksize;
	if (startblock + blockcount > c->cdrsize.blockcount) {
		blockcount = c->cdrsize.blockcount - startblock;
		if (!blockcount) {
			iop->io_ioc -= blockcount * c->cdrsize.blocksize;
			return;
		}
	}
	memset(g1, 0, sizeof(cdb_t));
	g1->opcode = G1READ;
	g1->lun = (r->lun << 5);
	g1->lba = startblock;
	flip(g1->lba);
	g1->xfr_len = blockcount;
	flip(g1->xfr_len);

	r->target = tid(dev);
	r->lun = lun(dev);
	r->timeout = 4;
	switch (iop->io_seg) {
	case IOSYS:
		r->buf.space = KRNL_ADDR;
		r->buf.ba_virt = iop->io.vbase;
		break;
	case IOUSR:
		r->buf.space = USER_ADDR;
		r->buf.ba_virt = iop->io.vbase;
		break;
	case IOPHY:
		r->buf.space = PHYS_ADDR;
		r->buf.ba_phys = iop->io.pbase;
		break;
	}
	r->buf.size = (blockcount * c->cdrsize.blocksize);
	r->xferdir = DMAREAD;
	doscsi(c->haft, r, 1, pritape, slpriSigCatch, "cdrread");
	switch (r->status) {
	case ST_GOOD:
		iop->io_ioc -= r->buf.size;
		break;
	case ST_CHKCOND:
		printsense(r->dev, "Read failed", (extsense_p) r->sensebuf);
		set_user_error (EIO);
		break;
	case ST_USRABRT:
		break;
	default:
		cmn_err(CE_WARN,
			"(%d, %d) haicd: Read failed: status (0x%x)",
			major(r->dev),
			minor(r->dev),
			r->status);
		set_user_error (EIO);
		break;
	}
	return;
}   /* cd_read() */

#if __USE_PROTO__
LOCAL int cd_pauseresume(register cdrctrl_p c, int pauseflag)
#else
LOCAL int
cd_pauseresume(c, pauseflag)
register cdrctrl_p	c;
int			pauseflag;
#endif
{
	register srb_p	r = &(c->srb);
	register char	*g2 = (char *) &(r->cdb. g1);
	
	r->buf.space = PHYS_ADDR | WATCH_REQACK;
	r->buf.ba_phys = NULL;
	r->buf.size = 0;
	r->xferdir = 0;
	r->timeout = 6;
	memset(g2, 0, sizeof(cdb_t));
	g2[0] = PAUSERESUME;
	g2[8] = (pauseflag == 0) ? 0 : 1;
	doscsi(c->haft, r, 2, pritape, slpriSigCatch, "cdrioctl");

	return (r->status == ST_GOOD);
}	/* cd_pauseresume() */

#if __USE_PROTO__
LOCAL int cd_playmsf(register cdrctrl_p	c, caddr_t uvec)
#else
LOCAL int 
cd_playmsf(c, uvec)
register cdrctrl_p	c;
caddr_t			uvec;
#endif
{
	register srb_p		r = &(c->srb);
	register char		*g2 = (char *) &(r->cdb. g1);
	struct cdrom_msf	play_addr;

	if (!ukcopy(uvec, &play_addr, sizeof(struct cdrom_msf)))
		return 0;

	r->buf.space = PHYS_ADDR | WATCH_REQACK;
	r->buf.ba_phys = NULL;
	r->buf.size = 0;
	r->xferdir = 0;
	r->timeout = 6;
	memset(g2, 0, sizeof(cdb_t));
	g2[0] = PLAYMSF;
	g2[3] = play_addr. cdmsf_min0;
	g2[4] = play_addr. cdmsf_sec0;
	g2[5] = play_addr. cdmsf_frame0;
	g2[6] = play_addr. cdmsf_min1;
	g2[7] = play_addr. cdmsf_sec1;
	g2[8] = play_addr. cdmsf_frame1;
	doscsi(c->haft, r, 2, pritape, slpriSigCatch, "cdrioctl");
	if (r->status != ST_GOOD) {
		set_user_error(EIO);
		return 0;
        }
	else
	        return 1;
}	/* cd_playmsf() */

#if __USE_PROTO__
LOCAL int cd_playtrkind(register cdrctrl_p c, struct cdrom_ti *cdti)
#else
LOCAL int cd_playtrkind(c, cdti)
register cdrctrl_p	c;
struct cdrom_ti		*cdti;
#endif
{
	register srb_p		r = &(c->srb);
	register char		*g2 = (char *) &(r->cdb. g1);

	r->buf.space = PHYS_ADDR | WATCH_REQACK;
	r->buf.ba_phys = NULL;
	r->buf.size = 0;
	r->xferdir = 0;
	r->timeout = 6;
	memset(g2, 0, sizeof(cdb_t));
	g2[0] = PLAYTRKIND;
	g2[4] = cdti->cdti_trk0;
	g2[5] = cdti->cdti_ind0;
	g2[7] = cdti->cdti_trk1;
	g2[8] = cdti->cdti_ind1;
	doscsi(c->haft, r, 2, pritape, slpriSigCatch, "cdrioctl");
	if (r->status != ST_GOOD) {
		set_user_error(EIO);
		return 0;
	}
	else
 	        return 1;
}	/* cd_playtrkind() */

#if __USE_PROTO__
LOCAL int cd_startstop(register cdrctrl_p c, int opflags)
#else
LOCAL int cd_startstop(c, opflags)
register cdrctrl_p	c;
int			opflags;
#endif
{
	register srb_p		r = &(c->srb);
	register g0cmd_p	g0 = &(r->cdb. g0);
	
	r->buf.space = PHYS_ADDR | WATCH_REQACK;
	r->buf.ba_phys = NULL;
	r->buf.size = 0;
	r->xferdir = 0;
	r->timeout = 6;
	memset(g0, 0, sizeof(cdb_t));
	g0->opcode = STARTSTOPUNIT;
	if (opflags & IMMEDIATE)
		g0->lun_lba |= 1;
	if (opflags & STARTUNIT)
		g0->xfr_len |= 1;

	/***************************************************************
         *  Because of the way I read the table of contents into memory
         *  and hold it until the drive is closed, Do not implement the
         *  Eject command right now. This prevents the bug of someone
         *  openning up the Drive with one CD and then changing the
         *  media via an eject command. This does nothing to prevent
         *  the user from using the front panel eject button to do
         *  the same thing.
         */

#if	0
	if (opflags & EJECT)
		g0->xfr_len |= 2;
#endif
	doscsi(c->haft, r, 2, pritape, slpriSigCatch, "cdrioctl");

	return (r->status == ST_GOOD);
}	/* cd_startstop() */

#if __USE_PROTO__
LOCAL int cd_readtocentry(register cdrctrl_p c, struct cdrom_tocentry *cdte)
#else
LOCAL int cd_readtocentry(c, cdte)
register cdrctrl_p	c;
struct cdrom_tocentry	*cdte;
#endif
{
	int			entries;
	toc_entry_p 		lba,
				msf;

	entries = (c->toclba->datalength - 2) / 8;
        lba = c->toclba->toc_entries;
        msf = c->tocmsf->toc_entries;
        while (entries--) {
                if (lba->tracknumber == cdte->cdte_track) {
                        cdte->cdte_adr = (lba->adrcontrol >> 4) & 0x0f;
                        cdte->cdte_ctrl = (lba->adrcontrol & 0x0f);
                        if (cdte->cdte_format == CDROM_LBA) {
                                cdte->cdte_addr.lba = lba->address.lba;
                        }
                        else if (cdte->cdte_format == CDROM_MSF) {
                                cdte->cdte_addr.msf.minute = msf->address.msf.minutes;
                                cdte->cdte_addr.msf.second = msf->address.msf.seconds;
                                cdte->cdte_addr.msf.frame = msf->address.msf.frame;
                        }
                        else {
                                set_user_error(ENXIO);
                                return 0;
                        }
                        return 1;
                }
                ++lba;
                ++msf;
        }
        set_user_error(ENXIO);
        return 0;
}	/* cd_readtocentry() */

#if __USE_PROTO__
LOCAL int cd_subchnl(register cdrctrl_p	c, struct cdrom_subchnl *cdsc)
#else
LOCAL int cd_subchnl(c, cdsc)
register cdrctrl_p	c;
struct cdrom_subchnl	*cdsc;
#endif
{
	register srb_p	r = &(c->srb);
	register char	*g2 = (char *) &(r->cdb. g1);
	subchnlq_t	sq;

	r->buf.space = KRNL_ADDR | WATCH_REQACK;
	r->buf.ba_virt = (caddr_t) &sq;
	r->buf.size = sizeof(subchnlq_t);
	memset(g2, 0, sizeof(cdb_t));
	g2[0] = READSUBCHNL;
	if (cdsc->cdsc_format == CDROM_MSF)
		g2[1] |= 0x02;	/* Specify M/S/F format */
	g2[2] = 0x40;		/* Specify Subchannel Q data required */
	g2[3] = 1;		/* Subchannel Q data format (CD-ROM position) */
	g2[7] = 0x00;
	g2[8] = sizeof(subchnlq_t);
	doscsi(c->haft, r, 4, pritape, slpriSigCatch, "cdropen");
	if (r->status != ST_GOOD) {
		set_user_error(EIO);
		return 0;
	}

	cdsc->cdsc_audiostatus = sq. sq_audiostatus;
	cdsc->cdsc_adr = (sq.sq_adrcontrol >> 4) & 0x0f;
	cdsc->cdsc_ctrl = (sq.sq_adrcontrol & 0x0f);
	cdsc->cdsc_trk = sq. sq_track;
	cdsc->cdsc_ind = sq. sq_index;
	if (cdsc->cdsc_format == CDROM_MSF) {
		cdsc->cdsc_absaddr.msf.minute = sq.sq_absaddr.msf.minutes;
		cdsc->cdsc_absaddr.msf.second = sq.sq_absaddr.msf.seconds;
		cdsc->cdsc_absaddr.msf.frame = sq.sq_absaddr.msf.frame;

		cdsc->cdsc_reladdr.msf.minute = sq.sq_reladdr.msf.minutes;
		cdsc->cdsc_reladdr.msf.second = sq.sq_reladdr.msf.seconds;
		cdsc->cdsc_reladdr.msf.frame = sq.sq_reladdr.msf.frame;
	}
	else {
		cdsc->cdsc_absaddr.lba = sq. sq_absaddr. lba;
		flip(cdsc->cdsc_absaddr.lba);
		cdsc->cdsc_reladdr.lba = sq. sq_reladdr. lba;
		flip(cdsc->cdsc_reladdr.lba);
	}
	return 1;
}	/* cd_subchnl() */
 
/***********************************************************************
 *  cd_ioctl() --	  I/O Control routine for CD-ROM drives.
 */

#if __USE_PROTO__
LOCAL void cd_ioctl(dev_t dev, int cmd, char *vec)
#else
LOCAL void
cd_ioctl(dev, cmd, vec)
dev_t   dev;
int	cmd;
char	*vec;
#endif
{
	register cdrctrl_p 	c = cdrdevs[tid(dev)];
	struct cdrom_ti		cdti;
	struct cdrom_tochdr 	cdth;
	struct cdrom_tocentry 	cdte;
	struct cdrom_subchnl	cdsc;
	
	if (!c) {
		set_user_error (ENXIO);
		return;
	}

	switch (cmd)	 {
	case CDROMPAUSE:	/* pause */
		if (!cd_pauseresume(c, 0))
			set_user_error(EIO);
		return;
		
	case CDROMRESUME:	/* resume */
		if (!cd_pauseresume(c, 1))
			set_user_error(EIO);
		return;
		
	case CDROMPLAYMSF:	/* play audio */
		cd_playmsf(c, vec);
		return;

	case CDROMPLAYTRKIND:	/* play track */
		if (!ukcopy(vec, &cdti, sizeof(struct cdrom_ti)))
			return;
		cd_playtrkind(c, &cdti);
		return;
	case CDROMREADTOCHDR:	/* read the TOC header */
		cdth.cdth_trk0 = c->toclba->firsttrack;
		cdth.cdth_trk1 = c->toclba->lasttrack;
		kucopy(&cdth, vec, sizeof(struct cdrom_tochdr));
		return;
		
	case CDROMREADTOCENTRY:	/* read a TOC entry */
		if (!ukcopy(vec, &cdte, sizeof(struct cdrom_tocentry)))
			return;

		if (!cd_readtocentry(c, &cdte))
			return;
			
		kucopy(&cdte, vec, sizeof(struct cdrom_tocentry));
	    	return;

	case CDROMSTOP:		/* stop the drive motor */
		if (!cd_startstop(c, 0))
			set_user_error(EIO);
		return;

	case CDROMSTART:	/* turn the motor on */
		if (!cd_startstop(c, STARTUNIT))
			set_user_error(EIO);
		return;

	case CDROMEJECT:	/* eject CD-ROM media */
		if (!cd_startstop(c, EJECT))
			set_user_error(EIO);
		return;

	case CDROMVOLCTRL:	/* volume control */
		return;

	case CDROMSUBCHNL:	/* read sub-channel data */
		if (!ukcopy(vec, &cdsc, sizeof(struct cdrom_subchnl)))
			return;

		if (!cd_subchnl(c, &cdsc))
			return;

		kucopy(&cdsc, vec, sizeof(struct cdrom_subchnl));
		return;

	case CDROMREADMODE1:	/* read type-1 data */
		return;

	case CDROMREADMODE2:	/* read type-2 data */
	default:
		set_user_error(ENXIO);
		return;
	}
}   /* cd_ioctl() */

/* End of file */
