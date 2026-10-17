/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1993 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */

#ifndef __SYS_HDIOCTL_H__
#define	__SYS_HDIOCTL_H__

#include <common/feature.h>

/*
 * Ioctl support for hard disk devices.
 */

#define	HDIOC	('H' << 8)
#define	HDGETA	(HDIOC|1)	/* get drive attributes */
#define	HDSETA	(HDIOC|2)	/* set drive attributes */
#define HDGETIDEINFO (HDIOC|4)	/* get result of IDE info command */

/*
 * Configuration word (ii_config) meanings by bit number
 * 0:  ?
 * 1:  Hard sectored
 * 2:  Soft sectored
 * 3:  Not MFM encoded
 * 4:  Head Switch Time > 15 usec
 * 5:  Spindled Motor Control Option Implemented
 * 6:  Fixed Drive
 * 7:  ?
 * 8:  Disk Transfer Rate <= 5Mbs
 * 9:  Disk Transfer Rate > 5Mbs but <= 10 Mbs
 * 10: Disk Transfer Rate > 10Mbs
 * 11: Rotational Speed Tolerance > 0.5%
 * 12: Data Strobe Offset Option Available
 * 13: Track Offset Option Available
 * 14: Format Speed Tolerance Gap Required
 * 15: ?
 */

typedef struct ide_info {
  unsigned short ii_config;	/* Configuration */
  unsigned short ii_cyl;	/* Cylinders (default xlat mode) */
  unsigned short ii_reserved;	/* reserved */
  unsigned short ii_heads;	/* heads (default xlat mode */
  unsigned short ii_bpt;	/* bytes per track (unformatted) */
  unsigned short ii_bps;	/* bytes per sector (unformatted) */
  unsigned short ii_spt;	/* sectors per track (default xlat mode) */
  unsigned short ii_vendor1[3];	/* vendor unique data */
  unsigned short ii_serialnum[10]; /* serial number in ASCII */
  unsigned short ii_buffertype;	/* buffer type */
  unsigned short ii_buffersize;	/* buffer size in 512 byte sectors */
  unsigned short ii_eccbyteslong; /* ecc bytes for r/w long */
  unsigned short ii_firmrev[4];	/* firmware revision in ascii */
  unsigned short ii_modelnum[20]; /* model number in ascii */
  unsigned short ii_doublewordio; /* double word transfer flag */
  unsigned short ii_capabilities; /* capabilities */
  unsigned short ii_reserved2;	/* reserved */
  unsigned short ii_piomode;	/* PIO data xfer timing mode */
  unsigned short ii_dmamode;	/* DMA data xfer timing mode */
  unsigned short ii_reserved3[75]; /* reserved */
  unsigned short ii_vendor2[32]; /* vendor unique data */
  unsigned short ii_reserved4[96]; /* reserved */
} ide_info_t;


/*
 * Drive attributes
 * Note: all fields defined as bytes to prevent compiler arith probs.
 *	All multi-byte fields are stored low-byte first.
 * This struct is configured for binary compatibility with ROM data!
 */

typedef struct hdparm_s {
	unsigned char	ncyl[2];	/* number of cylinders */
	unsigned char	nhead;		/* number heads */
	unsigned char	rwccp[2];	/* reduced write curr cyl */
	unsigned char	wpcc[2];	/* write pre-compensation cyl */
	unsigned char	eccl;		/* max ecc data length */
	unsigned char	ctrl;		/* control byte */
	unsigned char	fill2[3];
	unsigned char	landc[2];	/* landing zone cylinder */
	unsigned char	nspt;		/* number of sectors per track */
	unsigned char	hdfill3;
} hdparm_t;


/* Macro for initializing drive parameter tables. */

#define _HDPARMS(cyl,hd,spt,ctl,pcomp)	{ \
	{ (cyl) & 0xFF, (cyl) >> 8 }, hd, { 0, 0 }, \
	{ (pcomp) & 0xFF, (pcomp) >> 8 }, 0, ctl, \
	{ 0, 0, 0 }, { 0 , 0 }, spt, 0 }


/* Convert from a 2-element unsigned char array to unsigned short. */
/* Copy number into a 2-element unsigned char array. */

#if	_I386

#define _CHAR2_TO_USHORT(c_array)	(* (unsigned short *) (c_array))
#define	_NUM_TO_CHAR2(c_array, num)	(* (unsigned short *) (c_array) = (num))

#else

#define _CHAR2_TO_USHORT(c_array)	\
	((unsigned short) ((c_array)[1] << 8) | (c_array)[0])
/#define _NUM_TO_CHAR2(c_array, num) \
	(((c_array)[0] = (num) & 0xFF), ((c_array)[1] = (num) >> 8))

#endif


#if	_KERNEL
#define N_ATDRV	2U			/* only two drives supported */
#endif

#endif /* ! defined (__SYS_HDIOCTL_H__) */
