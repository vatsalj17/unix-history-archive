/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1993 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */
/***********************************************************************
 *	Module: haiscsi.h
 *
 *	Constants and structures used to access SCSI devices through the
 *	SCSI Driver in a Host Adapter inspecific manner.
 *
 *	Copyright (c) 1993, Christopher Sean Hilton, All rights reserved.
 */

#ifndef __SYS_HAISCSI_H__
#define __SYS_HAISCSI_H__

#include <common/feature.h>
#include <common/ccompat.h>
#include <sys/types.h>
#include <sys/hdioctl.h>

#define SCSIMAJOR	13

#define MAXTID		7
#define MAXDEVS 	(MAXTID + 1)
#define MAXLUN		7
#define MAXUNITS	(MAXLUN + 1)

#define ST_GOOD 	0x00	/* Status Good. */
#define ST_CHKCOND	0x02	/* Check Condition */
#define ST_CONDMET	0x04	/* Condition Met */
#define ST_BUSY 	0x08	/* Busy */
#define ST_INTERM	0x10	/* Intermediate */
#define ST_INTCDMET	0x14	/* Intermediate Condtion Met */
#define ST_RESCONF	0x18	/* Reservation Conflict */
#define ST_COMTERM	0x22	/* Command Terminated */
#define ST_QFULL	0x28	/* Queue Full */

#define ST_TIMEOUT	0x0101	/* Command Timed out */
#define ST_USRABRT	0x0102	/* User pressed ^C */
#define ST_DRVABRT	0x0103	/* Command Aborted by driver */
#define ST_ABRT		ST_DRVABRT
#define ST_ABRTFAIL	0x0104	/* Abort Failed */
#define ST_DEVRST	0x0105	/* Device was reset */
#define ST_HATMOUT	0x0201	/* Host adapter Timed out command */
#define ST_INVDSRB	0x0301	/* Invalid SRB */
#define ST_PENDING	0xffff	/* Command Pending */

#define DMAREAD 	0x0001	/* Command Reads from SCSI device */
#define DMAWRITE	0x0002	/* Command Writes to SCSI device */

#define HAI_SENSELEN	18

#define GROUPMASK   0xe0
#define GROUP0      0x00        /* SCSI-1/2 */
#define GROUP1      0x20        /* SCSI-1/2 */
#define GROUP2      0x40        /* SCSI-2 */
#define GROUP5      0xa0        /* SCSI-1/2 */

#define PHYS_ADDR	0x0000		/* Physical Address		*/
#define KRNL_ADDR	0x0001		/* Kernel Address		*/
#define USER_ADDR	0x0002		/* User Address			*/
#define SYSGLBL_ADDR	0x0003		/* System Global address	*/
#define SGLIST_ADDR	0x0004		/* Scatter/Gather list		*/

#define WATCH_REQACK	0x0080		/* Watch req/ack on byte of xfer */
#define SPACE_MASK	~(WATCH_REQACK)

/***** Minor Device Number Bits *****/

#define SPECIAL 	0x80	/* Special Bit to flag boot block / Tape */
#define TIDMASK 	0x70
#define LUNMASK 	0x0c
#define PARTMASK	0x03
#define TAPE		0x01
#define REWIND		0x02

/* SCSI Command Descriptor Block */

typedef struct g0cmd_s *g0cmd_p;

typedef struct g0cmd_s {
	unsigned char	opcode; 	/* From opcode Table */
	unsigned char	lun_lba;	/* LUN and high part of LBA */
	unsigned char	lba_mid;	/* LBA Middle. */
	unsigned char	lba_low;	/* LBA Low. */
	unsigned char	xfr_len;	/* Transfer Length */
	unsigned char	control;	/* Control byte. */
} g0cmd_t;

typedef struct g1cmd_s *g1cmd_p;

#pragma align 1
typedef struct g1cmd_s {
	unsigned char	opcode; 	/* From opcode Table */
	unsigned char	lun;		/* LUN */
	unsigned long	lba __ALIGN (2);	/* LBA */
	unsigned char	pad1;		/* Reserved */
	unsigned short	xfr_len __ALIGN(1);    /* Transfer Length's MSB. */
	unsigned char	control;	/* Control byte. */
} g1cmd_t;
#pragma align


#define g2cmd_t g1cmd_t 		/* SCSI-2 Added Group 2 commands */
#define g2cmd_s g1cmd_s 		/* with the same size and layout as */
#define g2cmd_p g1cmd_p 		/* g1 commands. */

typedef struct g5cmd_s *g5cmd_p;

#pragma align 1
typedef struct g5cmd_s {
	unsigned char	opcode; 	/* From opcode Table */
	unsigned char	lun;		/* LUN */
	unsigned long	lba __ALIGN (2);	/* LBA's MSB */
	unsigned char	pad1[3];	/* Reserved */
	unsigned short	xfr_len __ALIGN (1);	/* Transfer Length */
	unsigned char	control;	/* Control byte. */
} g5cmd_t;
#pragma align

typedef union cdb_u *cdb_p;

typedef union cdb_u {
	g0cmd_t g0;
	g1cmd_t g1;
	g5cmd_t g5;
} cdb_t;

typedef struct sense_s *sense_p;

typedef struct sense_s {
	unsigned char	errorcode;	/* Error Code:	0x0? */
	unsigned char	lba_msb;	/* LSB's MS 5 Bits */
	unsigned char	lba_mid;	/*	 Middle 8 bits */
	unsigned char	lba_lsb;	/*	 LS 8 Bits */
} sense_t;

typedef struct extsense_s *extsense_p;

#pragma align 1
typedef struct extsense_s {
	unsigned char	errorcode;	/* Error Code (70H) */
	unsigned char	segmentnum; 	/* Number of current segment descriptor */
	unsigned char	sensekey;	/* Sense Key(See bit definitions too) */
	long		info __ALIGN (1);	/* Information MSB */
	unsigned char	addlen; 	/* Additional Sense Length */
	unsigned char	addbytes[1];	/* Additional Sense unsigned chars */
} extsense_t;
#pragma align

#if 	_KERNEL

#include <sys/buf.h>
#include <sys/poll.h>
#include <sys/io.h>

typedef struct haisgsegm_s *haisgsegm_p;

typedef struct haisgsegm_s {
    paddr_t	sgs_segstart;
    size_t	sgs_segsize;
} haisgsegm_t;

typedef struct bufaddr_s *bufaddr_p;

typedef struct bufaddr_s {
	int 		space;		/* Address space */
	union {
		paddr_t 	paddr;	/* Physical Address */
		caddr_t 	caddr;	/* Virtual Address */
		haisgsegm_p	sglist; /* Scatter/Gather list */
	} addr;
	size_t		size;		/* Size of buffer */
} bufaddr_t;

#define ba_virt		addr.caddr
#define ba_phys		addr.paddr
#define ba_sglist	addr.sglist

typedef struct srb_s *srb_p;	/* SCSI Request Block */

typedef struct srb_s {
	unsigned short	status; 	/* SCSI Status Byte */
	unsigned short	hastat; 	/* Host Adapter Status Byte */
	dev_t		dev;		/* Device number (major/minor) */
	unsigned char	target; 	/* Target ID */
	unsigned char	lun;		/* Logical Unit Number */
	unsigned short	tries;		/* Current tries */
	unsigned short	timeout;	/* Seconds til timeout */
	bufaddr_t	buf;		/* Buffer to use */
	unsigned short	xferdir;	/* Transfer Direction */
	void 		(*cleanup) __PROTO((srb_p));	/* Cleanup Function. */
	cdb_t		cdb;		/* Command to execute */
	char		sensebuf[HAI_SENSELEN];
} srb_t;


/* Host Adapter function table. */

typedef struct {
    int		hf_present;
    void 	(*hf_timer)		__PROTO((void));
    int 	(*hf_load) 		__PROTO((void));
    void	(*hf_unload)		__PROTO((void));
    int 	(*hf_startscsi) 	__PROTO((srb_p));
    void 	(*hf_abortscsi) 	__PROTO((srb_p));
    void 	(*hf_resetdevice) 	__PROTO((int));
} haft_t;

/***** Device Control Array *****/

typedef struct dca_s *dca_p;

typedef struct dca_s {
	int (*d_open)		__PROTO((dev_t, ...));
	void (*d_close) 	__PROTO((dev_t));
	void (*d_block)		__PROTO((buf_t *));
	void (*d_read)		__PROTO((dev_t, IO *));
	void (*d_write)		__PROTO((dev_t, IO *));
	void (*d_ioctl)		__PROTO((dev_t, int, char *));
	int (*d_load)		__PROTO((haft_t *, int));
	void (*d_unload)	__PROTO((int));
	int (*d_poll)		__PROTO((dev_t, int, int, void *));
} dca_t;

#ifdef HA_MODULE
extern dca_p mdca[MAXDEVS];
#endif

/***********************************************************************
 *	Host Adapter routines.
 *
 *	These must be defined by the host adapter module.  For each individual
 *	routine's functionality see the host adapter module aha154x.c.
 */

#define hatimer(ft)		(*((ft)->hf_timer))()
#define haload(ft)		(*((ft)->hf_load))()
#define haunload(ft)		(*((ft)->hf_unload))()
#define startscsi(ft, r)	(*((ft)->hf_startscsi))(r)
#define abortscsi(ft, r)	(*((ft)->hf_abortscsi))(r)
#define resetdevice(ft, id)	(*((ft)->hf_resetdevice))(id)

extern int	HAI_HAID;

#define bit(n)		(1 << (n))
#define tid(d)		(((d) & TIDMASK) >> 4)
#define lun(d)		(((d) & LUNMASK) >> 2)
#define partn(d)	(((d) & SPECIAL) ? 4 : ((d) & PARTMASK))

char *swapbytes		__PROTO((void *, size_t));
#define flip(o) 	swapbytes(&(o), sizeof(o))

int cpycdb		__PROTO((cdb_p, cdb_p));
void reqsense		__PROTO((haft_t *, srb_p));
void doscsi		__PROTO((haft_t *, srb_p, int, int, int, char []));
void printsense		__PROTO((dev_t, char [], extsense_p));
int printerror		__PROTO((srb_p, char []));
void dumpmem		__PROTO((char [], unsigned char [], size_t));
void haiioctl		__PROTO((srb_p, int, char []));
void hainonblk		__PROTO((buf_t *));
size_t ukbuftosgl	__PROTO((vaddr_t, size_t, haisgsegm_p, size_t));
size_t sysgbuftosgl	__PROTO((paddr_t, size_t, haisgsegm_p, size_t));
int addbuftosgl		__PROTO((paddr_t, size_t, haisgsegm_p, size_t));
int buftosglist		__PROTO((buf_t *, size_t, haisgsegm_p, size_t));
void loadbiosparms	__PROTO((int, int));
void haihdgeta		__PROTO((int, hdparm_t *, unsigned));
void haihdseta		__PROTO((int, hdparm_t *));

enum {
	HAI_154X_INDEX = 0,
	HAI_SS_INDEX = 1
};

#endif	/* _KERNEL */

#endif /* ! defined (__SYS_HAISCSI_H__) */
