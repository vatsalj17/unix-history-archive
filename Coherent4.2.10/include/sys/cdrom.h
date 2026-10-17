/*
 * /usr/include/sys/cdrom.h
 */

#ifndef __SYS_CDROM_H__
#define __SYS_CDROM_H__


#include <sys/types.h>

/*
 * CD-ROM IOCTL commands common to all drives
 */

#define CDBASE			('c' << 8)
#define	CDROMPAUSE		(CDBASE | 0x01)	/* pause */
#define	CDROMRESUME		(CDBASE | 0x02)	/* resume */
#define	CDROMPLAYMSF		(CDBASE | 0x03)	/* play audio */
#define	CDROMPLAYTRKIND		(CDBASE | 0x04) /* play track */
#define	CDROMREADTOCHDR		(CDBASE | 0x05) /* read the TOC header */
#define	CDROMREADTOCENTRY	(CDBASE | 0x06) /* read a TOC entry */
#define	CDROMSTOP		(CDBASE | 0x07) /* stop the drive motor */
#define	CDROMSTART		(CDBASE | 0x08) /* turn the motor on */
#define	CDROMEJECT		(CDBASE | 0x09) /* eject CD-ROM media */
#define	CDROMVOLCTRL		(CDBASE | 0x0a) /* volume control */
#define	CDROMSUBCHNL		(CDBASE | 0x0b) /* read sub-channel data */
#define	CDROMREADMODE1		(CDBASE | 0x0c) /* read type-1 data */
#define	CDROMREADMODE2		(CDBASE | 0x0d) /* read type-2 data */

/*
 * CDROM IOCTL structures common to all drives
 */

struct cdrom_msf {
	uchar_t	cdmsf_min0;		/* start minute */
	uchar_t	cdmsf_sec0;		/* start second */
	uchar_t	cdmsf_frame0;		/* start frame */
	uchar_t	cdmsf_min1;		/* end minute */
	uchar_t	cdmsf_sec1;		/* end second */
	uchar_t	cdmsf_frame1;		/* end frame */
};

struct cdrom_ti {
	uchar_t	cdti_trk0;		/* start track */
	uchar_t	cdti_ind0;		/* start index */
	uchar_t	cdti_trk1;		/* end track */
	uchar_t	cdti_ind1;		/* end index */
};

struct cdrom_tochdr {
	uchar_t	cdth_trk0;		/* start track */
	uchar_t	cdth_trk1;		/* end track */
};

struct cdrom_tocentry {
	uchar_t	cdte_track;
	uchar_t	cdte_adr:4;
	uchar_t	cdte_ctrl:4;
	uchar_t	cdte_format;
	union {
		struct {
			uchar_t	minute;
			uchar_t	second;
			uchar_t	frame;
		} msf;
		int	lba;
	} cdte_addr;
	uchar_t	cdte_datamode;
};

/*
 * CD-ROM address types (cdrom_tocentry.cdte_format)
 */

#define	CDROM_LBA	0x01
#define	CDROM_MSF	0x02

/*
 * bit to tell whether track is data or audio
 */

#define	CDROM_DATA_TRACK	0x04

/*
 * The leadout track is always 0xAA, regardless of # of tracks on disc
 */

#define	CDROM_LEADOUT	0xAA

struct cdrom_subchnl {
	uchar_t	cdsc_format;
	uchar_t	cdsc_audiostatus;
	uchar_t	cdsc_adr:4;
	uchar_t	cdsc_ctrl:4;
	uchar_t	cdsc_trk;
	uchar_t	cdsc_ind;
	union {
		struct {
			uchar_t	minute;
			uchar_t	second;
			uchar_t	frame;
		} msf;
		int	lba;
	} cdsc_absaddr;
	union {
		struct {
			uchar_t	minute;
			uchar_t	second;
			uchar_t	frame;
		} msf;
		int	lba;
	} cdsc_reladdr;
};

/*
 * return value from READ SUBCHANNEL DATA
 */

#define	CDROM_AUDIO_INVALID	0x00	/* audio status not supported */
#define	CDROM_AUDIO_PLAY	0x11	/* audio play operation in progress */
#define	CDROM_AUDIO_PAUSED	0x12	/* audio play operation paused */
#define	CDROM_AUDIO_COMPLETED	0x13	/* audio play successfully completed */
#define	CDROM_AUDIO_ERROR	0x14	/* audio play stopped due to error */
#define	CDROM_AUDIO_NO_STATUS	0x15	/* no current audio status to return */

struct cdrom_volctrl {
	uchar_t	channel0;
	uchar_t	channel1;
	uchar_t	channel2;
	uchar_t	channel3;
};

struct cdrom_read {
	int	cdread_lba;
	caddr_t	cdread_bufaddr;
	int	cdread_buflen;
};

#define	CDROM_MODE1_SIZE	2048
#define	CDROM_MODE2_SIZE	2336

#endif

