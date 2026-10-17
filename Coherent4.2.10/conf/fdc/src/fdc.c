/* $Header: /ker/io.386/RCS/fdc.c,v 2.5 93/08/19 10:38:44 nigel Exp Locker: nigel $ */
/*
 * Support 765-style controller for diskette and floppy tape
 *
 * $Log:	fdc.c,v $
 * Revision 2.5  93/08/19  10:38:44  nigel
 * r83 ioctl (), corefile, new headers
 * 
 * Revision 2.4  93/08/19  04:02:19  nigel
 * Nigel's R83
 */

#define _KERNEL 1

#include <sys/errno.h>
#include <sys/cred.h>
#include <sys/stat.h>
#include <sys/coherent.h>

#include <kernel/trace.h>

#include <coh/i386lib.h>
#include <coh/md.h>
#include <coh/misc.h>
#include <coh/putchar.h>

#include <common/ccompat.h>

#include <sys/uproc.h>
#include <sys/buf.h>
#include <sys/con.h>
#include <sys/devices.h>
#include <sys/dmac.h>
#include <sys/fdc765.h>
#include <sys/cmn_err.h>


/* Number of ticks to busy-wait the kernel before timeout awaiting RQM. */
#define FDC_RQM_WAIT	2

int fdcTblk;
int OUTB_DB = 0;

enum {
	FL_TIMING = 1,
	FT_TIMING = 2
};

enum {
	FL_INTR = 1,
	FT_INTR = 2
};

void	fdcReset();

void		fdcRate();
static int	fdcWaitRQM();

static int	setFdcIntr();
static void	setFdcTiming();


/*
 * Two patchable pointers, for enabling diskette and/or tape device control.
 */

extern	CON * flCon;		/* Initialized in fl/Space.c */
extern	CON * ftCon;		/* Initialized in ft/Space.c */

/* Global struct "fdc" passes FDC status to diskette and tape drivers. */
struct FDC fdc;

void	(*flIntr)();
void	(*ftIntr)();

static int	fdcIntOwner;
static int	fdcTiming;

/******* FOR DEBUG PURPOSES ONLY ************/
#if __USE_PROTO__
void outbDb (int addr, int data)
#else
void
outbDb(addr, data)
int addr, data;
#endif
{
	static int oldAddr, oldData;
	int s = sphi();

	if (OUTB_DB) {
		outb(addr, data);
		if (addr != oldAddr) {
			cmn_err (CE_CONT, "!OUT[%x,%x]", addr, data);
			oldAddr = addr;
			oldData = data;
		} else if (data != oldData) {
			cmn_err (CE_CONT, "!/%x", data);
			oldData = data;
		} else
			putchar('=');
	} else
		outb(addr, data);
	spl(s);
}

/***************************************************************************/
/*
 * First part of fdc module.
 *
 * Kernel interface.
 */

/*
 * The load routine asks the
 * switches how many drives are present
 * in the machine, and sets up the field
 * in the floppy database. It also grabs
 * the level 6 interrupt vector.
 */

#if __USE_PROTO__
static void fdcload(void)
#else
static void
fdcload()
#endif
{
	register int	s;

	if (flCon == NULL && ftCon == NULL) {
		cmn_err (CE_WARN, "fdc has no target devices");
		return;
	}

	/*
	 * Ensure DMA channel 2 is turned off.
	 * The Computerland ROM does not disable DMA channel after autoboot
	 * from hard disk.  The Western Digital controller board appears to
	 * send a dma burst when the floppy controller chip is reset.
	 */
	dmaoff(DMA_CH2);

	if (flCon)
		(* flCon->c_load) ();
	if (ftCon)
		(* ftCon->c_load) ();

	/*
	 * Initialize the floppy disk controller (if we
	 * have any floppy drives).
	 */

	s = sphi();
	fdcReset ();
	spl(s);
}

/*
 * Release resources.
 */

#if __USE_PROTO__
static void fdcunload(void)
#else
static void
fdcunload()
#endif
{
	if (flCon == NULL && ftCon == NULL)
		return;
	
	if (flCon)
		(*flCon->c_uload) ();
	if (ftCon)
		(*ftCon->c_uload) ();

	/*
	 * Cancel periodic (1 second) invocation.
	 */
	drvl[FL_MAJOR].d_time = 0;
	fdcTiming = 0;

	/*
	 * Turn motors off.
	 */
	outbDb(FDCDOR, DORNMR);		/* Leave interrupts disabled. */

	/*
	 * Clear interrupt vector.
	 */
	clrivec(6);
}

#if __USE_PROTO__
static void fdcopen(o_dev_t dev, int mode, int flags, __cred_t * credp)
#else
static void
fdcopen (dev, mode, flags, credp)
o_dev_t		dev;
int		mode;
int		flags;
cred_t	      *	credp;
#endif
{
	if (FDC_DISKETTE (dev)) {
		if (flCon)
			(* flCon->c_open) (dev, mode, flags, credp);
		else
			SET_U_ERROR (ENXIO, "fdcopen()-no floppy");
	} else if (FDC_TAPE (dev)) {
		if (ftCon)
			(* ftCon->c_open) (dev, mode, flags, credp);
		else
			SET_U_ERROR (ENXIO, "fdcopen()-no tape");
	} else
		SET_U_ERROR (ENXIO, "fdcopen()-no device");
}

#if __USE_PROTO__
static void fdcclose(o_dev_t dev, int mode, int flags, __cred_t * credp)
#else
static void
fdcclose(dev, mode, flags, credp)
o_dev_t		dev;
int		mode;
int		flags;
cred_t	      *	credp;
#endif
{
	if (FDC_DISKETTE (dev)) {
		if (flCon)
			(* flCon->c_close) (dev, mode, flags, credp);
		else
			SET_U_ERROR (ENXIO, "fdcclose()-no floppy");
	} else if (FDC_TAPE (dev)) {
		if (ftCon)
			(* ftCon->c_close) (dev, mode, flags, credp);
		else
			SET_U_ERROR (ENXIO, "fdcclose()-no tape");
	} else
		SET_U_ERROR (ENXIO, "fdcclose()-no device");
}

#if __USE_PROTO__
static void fdcread(o_dev_t dev, IO * iop, __cred_t * credp)
#else
static void
fdcread(dev, iop, credp)
o_dev_t		dev;
IO	      *	iop;
cred_t	      *	credp;
#endif
{
	if (FDC_DISKETTE (dev)) {
		if (flCon)
			(* flCon->c_read) (dev, iop, credp);
		else
			SET_U_ERROR (ENXIO, "fdcread()-no floppy");
	} else if (FDC_TAPE (dev)) {
		if (ftCon)
			(* ftCon->c_read) (dev, iop, credp);
		else
			SET_U_ERROR (ENXIO, "fdcread()-no tape");
	} else
		SET_U_ERROR(ENXIO, "fdcread()-no device");
}

#if __USE_PROTO__
static void fdcwrite(o_dev_t dev, IO * iop, __cred_t * credp)
#else
static void
fdcwrite(dev, iop, credp)
o_dev_t		dev;
IO	      *	iop;
cred_t	      *	credp;
#endif
{
	if (FDC_DISKETTE (dev)) {
		if (flCon)
			(* flCon->c_write) (dev, iop, credp);
		else
			SET_U_ERROR (ENXIO, "fdcwrite()-no floppy");
	} else if (FDC_TAPE (dev)) {
		if (ftCon)
			(* ftCon->c_write) (dev, iop, credp);
		else
			SET_U_ERROR (ENXIO, "fdcwrite()-no tape");
	} else
		SET_U_ERROR (ENXIO, "fdcwrite()-no device");
}


#if __USE_PROTO__
static void fdcioctl(o_dev_t dev, int com, __VOID__ * par, int mode,
  __cred_t * credp, int * rvalp)
#else
static void
fdcioctl (dev, com, par, mode, credp, rvalp)
o_dev_t		dev;
int		com;
char	      *	par;
int		mode;
cred_t	      *	credp;
int	      *	rvalp;
#endif
{
	if (FDC_DISKETTE (dev)) {
		if (flCon)
			(* flCon->c_ioctl) (dev, com, par, mode, credp, rvalp);
		else
			SET_U_ERROR (ENXIO, "fdcioctl()-no floppy");
	} else if (FDC_TAPE(dev)) {
		if (ftCon)
			(* ftCon->c_ioctl) (dev, com, par, mode, credp, rvalp);
		else
			SET_U_ERROR (ENXIO, "fdcioctl()-no tape");
	} else
		SET_U_ERROR (ENXIO, "fdcioctl()-no device");
}

/* Can't set u.u_error inside block routine. */
static void
fdcblock (bp)
BUF	      *	bp;
{
	if (FDC_DISKETTE (bp->b_dev)) {
		if (flCon)
			(* flCon->c_block) (bp);
	} else if (FDC_TAPE (bp->b_dev))
		if (ftCon)
			(* ftCon->c_block) (bp);
}


/***************************************************************************/
/*
 * Second part of fdc module.
 *
 * Hardware interface.
 */

/*
 * Get status (if any) from last command
 */
void
fdcCmdStatus()
{
	register int	b;
	register int	n = 0;		/* # of status bytes read */
	register int	s;

	s = sphi();

	/*
	 * Read all the status bytes the controller will give us.
	 */

	for (;;) {
		b = fdcGet();
		if (b == -1)
			break;

		if (n < FDC_NUM_CMD_STAT)
			fdc.fdc_cmdstat[n++] = b;
	}

	fdc.fdc_ncmdstat = n;
	spl(s);
}

/*
 * fdcDrvSelect()
 *
 * Select the drive indicated by "drive" (0..3).
 * If "motorOn" is nonzero, turn the motor on as well.
 */
void
fdcDrvSelect(drive, motorOn)
int drive, motorOn;
{
	unsigned char motorBits = 0;

	if (drive >= 4) {
		cmn_err (CE_WARN, "fdcDrvSelect(**drive**=%d,%d)",
		  drive, motorOn);
		return;
	}

	/* If needed, generate motor on bit for current selected drive. */
	if (motorOn)
		motorBits = 0x10 << drive;

	outbDb(FDCDOR, DORNMR | DORIEN | drive | motorBits);
	fdcSense();				/* Just in case --- */
}

/*
 * Send Sense Drive Status command to FDC.
 */
void
fdcDrvStatus(drive, head)
int drive, head;
{
	fdcPut(FDC_CMD_SDRV);
	fdcPut(drive | (head << 2));
}

/*
 * Read NEC data register, doing needed handshake.
 * Return -1 in case of timeout before RQM or no data available.
 */
int
fdcGet()
{
	int ret = -1;

	/* Wait for RQM, then expect DIO true. */
	if (busyWait(fdcWaitRQM, FDC_RQM_WAIT) && (inb(FDCMSR) & MSRDIO))
		ret = inb(FDCDAT);

	return ret;
}

/*
 * Get Interrupt status
 */
void
fdcIntStatus()
{
	register int	b;
	register int	n = 0;		/* # of status bytes read */
	register int	s;

	s = sphi();

	/*
	 * Issue a sense interrupt command and stash result.
	 */
	fdcPut(FDC_CMD_SINT);

	n = 0;
	for (;;) {
		b = fdcGet();
		if (b == -1)
			break;

		if (n < FDC_NUM_INT_STAT)
			fdc.fdc_intstat[n++] = b;
	}
	fdc.fdc_nintstat = n;
	spl(s);
}

/*
 * Since the FDC is run in DMA mode, possible interrupt causes are:
 *
 * 1 Result phase of: Read Data, Read Track, Read ID, Read Deleted Data,
 *   Write Data, Format Cylinder, Write Deleted Data, Scan.
 *
 * 2 Ready line of diskette drive changes state.
 *
 * 3 End of Seek or Recalibrate.
 *
 * In case 1, the interrupt is cleared by read/write data to FDC.
 * In cases 2 and 3, a Sense Interrupt is needed to clear the interrupt
 * and determine its cause.
 *
 * The following comment is obsolete, but possibly of interest:
 ***********************************************
 * The interrupt routine gets all
 * the status bytes the controller chip
 * will give it, then issues a sense interrupt
 * status command (which is necessary for a seek
 * to complete!) and throws all of the status
 * bytes away.
 ***********************************************
 */

void
fdcintr ()
{
	int s;

	s = sphi ();

	/* Invalidate previously stored interrupt and command status. */
	fdc.fdc_nintstat = 0;
	fdc.fdc_ncmdstat = 0;

	/* Vector to diskette or tape device interrupt handler. */
	if ((fdcIntOwner & FL_INTR) && flIntr)
		(* flIntr)();
	else if ((fdcIntOwner & FT_INTR) && ftIntr)
		(* ftIntr)();
	else /* Just clear the interrupt status. */
		fdcSense ();
	spl (s);
}


/*
 * Send a command byte to the NEC chip, first waiting until the chip
 * says that it is ready.
 * Return 0 if able to send, or -1 if timed out or FDC had wrong I/O
 * direction.
 */
int
fdcPut(cmd)
int cmd;
{
	int ret = -1;

	/* Wait for RQM, then expect DIO false. */
	if (busyWait(fdcWaitRQM, FDC_RQM_WAIT)) {
		if ((inb(FDCMSR) & MSRDIO) == 0) {
			outbDb(FDCDAT, cmd);
			ret = 0;
		}
	}

	return ret;
}

/*
 * Send a command string, given as byte array and length.
 * Return the number of bytes sent.
 */
int
fdcPutStr(cmdStr, cmdLen)
unsigned char * cmdStr;
unsigned int cmdLen;
{
	int bytesSent;

	for (bytesSent = 0; bytesSent < cmdLen; bytesSent++) {
		if (fdcPut(*cmdStr++)) {
			cmn_err (CE_WARN, "fdcPutStr: only sent %d bytes of %d",
			  bytesSent, cmdLen);
			break;
		}
	}
	return bytesSent;
}

/*
 * Set transfer rate (FDC_RATE_250K/300K/500K/1MEG)
 */
void
fdcRate(rate)
int rate;
{
	outbDb(FDCRATE, rate);
}

/*
 * Given drive # (0..3), and head (0..1), send Read ID command to the FDC.
 */
void
fdcReadID(drive, head)
int drive;
int head;
{
	fdcPut(FDC_CMD_RDID);
	fdcPut(drive | (head << 2));
}

/*
 * Given drive # (0..3), send a Recalibrate command to the FDC.
 *
 * Sense interrupt - fdcIntStatus() - *must* be done when the
 * ensuing IRQ happens.
 */
void
fdcRecal(drive)
int drive;
{
	fdcPut(FDC_CMD_RCAL);
	fdcPut(drive);
}

/*
 * Given drive # (0..3), head (0..1), and cylinder (0..255),
 * send a seek command to the FDC.
 *
 * Sense interrupt - fdcIntStatus() - *must* be done when the
 * ensuing IRQ happens.
 */
void
fdcSeek(drive, head, cyl)
int drive, head, cyl;
{
	fdcPut(FDC_CMD_SEEK);
	fdcPut(drive | (head << 2));
	fdcPut(cyl);
}

/*
 * fdcSense() issues Sense Drive Status and Sense Interrupt Status,
 * saving information from the FDC into global struct "fdc".
 *
 * It is called in response to FDC interrupts.
 */
void
fdcSense()
{
	fdcCmdStatus();			/* Get command status. */
	fdcIntStatus();			/* Get int status, just in case. */
}

/*
 * Send Specify command and data bytes to FDC.
 * Always specify DMA mode (ND bit = 0).
 */
void
fdcSpecify(srt, hut, hlt)
int srt, hut, hlt;
{
	fdcPut(FDC_CMD_SPEC);
	fdcPut((srt << 4) | hut);
	fdcPut(hlt << 1);
}

/*
 * Dissassemble the floppy error status for user reference.
 */
void
fdcStatus()
{
	/* If using floppy tape, show block number, not drive & head. */
	if (fdcIntOwner & FT_INTR)
		cmn_err (CE_CONT,
		  "WARNING : FDC tape block=%d (u=%d h=%u)",
		  fdcTblk,
		  fdc.fdc_cmdstat[0] & 3, (fdc.fdc_cmdstat[0] & 4) >> 2);
	else
		cmn_err (CE_CONT, "WARNING : FDC unit=%d  head=%u",
		  fdc.fdc_cmdstat[0] & 3, (fdc.fdc_cmdstat[0] & 4) >> 2);

	/*
	 * Report on ST0 bits.
	 */
	if (fdc.fdc_ncmdstat >= 1) {
		if (fdc.fdc_cmdstat[0] & ST0_NR)
			cmn_err (CE_CONT, " <Not Ready>");

		if (fdc.fdc_cmdstat[0] & ST0_EC)
			cmn_err (CE_CONT, " <Equipment Check>");
	}

	/*
	 * Report on ST1 bits.
	 */
	if (fdc.fdc_ncmdstat >= 2) {
		if (fdc.fdc_cmdstat[1] & ST1_MA)
			cmn_err (CE_CONT, " <Missing Address Mark>");

		if (fdc.fdc_cmdstat[1] & ST1_NW)
			cmn_err (CE_CONT, " <Write Protected>");

		if (fdc.fdc_cmdstat[1] & ST1_ND)
			cmn_err (CE_CONT, " <No Data>");

		if (fdc.fdc_cmdstat[1] & ST1_OR)
			cmn_err (CE_CONT, " <Overrun>");

		if (fdc.fdc_cmdstat[1] & ST1_DE)
			cmn_err (CE_CONT, " <Data Error>");

		if (fdc.fdc_cmdstat[1] & ST1_EN)
			cmn_err (CE_CONT, " <End of Cyl>");
	}

	/*
	 * Report on ST2 bits.
	 */
	if (fdc.fdc_ncmdstat >= 3) {
		if (fdc.fdc_cmdstat[2] & ST2_MD)
			cmn_err (CE_CONT, " <Missing Data Address Mark>");

		if (fdc.fdc_cmdstat[2] & ST2_BC)
			cmn_err (CE_CONT, " <Bad Cylinder>");

		if (fdc.fdc_cmdstat[2] & ST2_WC)
			cmn_err (CE_CONT, " <Wrong Cylinder>");

		if (fdc.fdc_cmdstat[2] & ST2_DD)
			cmn_err (CE_CONT, " <Bad Data CRC>");

		if (fdc.fdc_cmdstat[2] & ST2_CM)
			cmn_err (CE_CONT, " <Data Deleted>");
	}

	cmn_err (CE_CONT, "\n");
}

/*
 * Wait for FDC Main Status Register to assert Request for Master.
 * This function is designed to be called by busyWait().
 * It returns nonzero if RQM is asserted, 0 if not.
 */
static int
fdcWaitRQM()
{
	return (inb(FDCMSR) & MSRRQM);
}

/*
 * If sw is nonzero, start the timer for diskette;
 * else, stop the timer.
 */
void
setFlTimer(sw)
int sw;
{
	setFdcTiming(sw, FL_TIMING);
}

/*
 * If sw is nonzero, start the timer for floppy tape;
 * else, stop the timer.
 */
void
setFtTimer(sw)
int sw;
{
	setFdcTiming(sw, FT_TIMING);
}

static void
setFdcTiming(sw, mask)
int sw, mask;
{
	int s = sphi();

	/*
	 * Only do something if current request changes status of
	 * timing for the device.
	 */
	if (sw && (fdcTiming & mask) == 0) {
		fdcTiming |= mask;
		drvl[FL_MAJOR].d_time = 1;
		goto setFdcTimingDone;
	}
	if (sw  == 0 && (fdcTiming & mask)) {
		fdcTiming &= ~mask;
		if (fdcTiming == 0)
			drvl[FL_MAJOR].d_time = 0;
		goto setFdcTimingDone;
	}

setFdcTimingDone:
	spl(s);
	return;
}

#if __USE_PROTO__
static void fdctimeout (o_dev_t dev)
#else
static void
fdctimeout (dev)
o_dev_t		dev;
#endif
{
	if ((fdcTiming & FL_TIMING) && flCon)
		(* flCon->c_timer) (dev);
	if ((fdcTiming & FT_TIMING) && ftCon)
		(* ftCon->c_timer) (dev);
}


/*
 * Reset the fdc.
 * Sets drive select bits to 00, motor on bits to 0000.
 */

void
fdcReset()
{
	/* "Not Reset FDC" must remain low for at least 3.5 usec */
	outbDb (FDCDOR, 0);
	busyWait2 (NULL, 4);
	outbDb (FDCDOR, DORNMR | DORIEN);
}


/*
 * Reset the fdc.
 * Maintain drive select and motor enable (as specified) during the reset.
 */

void
fdcResetSel(drive, motorOn)
int drive, motorOn;
{
	unsigned char motorBits = 0;
	unsigned char outByte;

	/* If needed, generate motor on bit for current selected drive. */
	if (motorOn)
		motorBits = 0x10 << drive;

	/*
	 * Send drive select, motor on if needed, interrupt enable.
	 * The "not reset" bit is zero, which is the point of this routine.
	 */
	outByte = motorBits | drive | DORIEN;
	outbDb(FDCDOR, outByte);

	/* "Not Reset FDC" must remain low for at least 3.5 usec */
	busyWait2(NULL, 4);

	outByte |= DORNMR;
	outbDb(FDCDOR, outByte);
}

/*
 * If sw is nonzero, try to seize the fdc interrupt for the diskette;
 * else, try to release it.
 *
 * Return 1 on success, 0 on failure.
 */
int
setFlIntr(sw)
int sw;
{
	return setFdcIntr(sw, FL_INTR);
}

/*
 * If sw is nonzero, try to seize the fdc interrupt for floppy tape;
 * else, try to release it.
 *
 * Return 1 on success, 0 on failure.
 */
int
setFtIntr(sw)
int sw;
{
	return setFdcIntr(sw, FT_INTR);
}

static int
setFdcIntr(sw, mask)
int sw, mask;
{
	int ret;

	/*
	 * if attaching
	 *   if fdc interrupt is free
	 *	attach as requested
	 *   else
	 *      return failure
	 * else
	 *   if fdc requesting device now owns fdc interrupt
	 *	detach as requested
	 *   else
	 *      return failure
	 */

	if (sw) {
		if (fdcIntOwner == 0) {
			fdcIntOwner = mask;
			ret = 1;
		} else if (fdcIntOwner == mask) {
			ret = 1;
		} else {
			ret = 0;
		}
	} else {
		if (fdcIntOwner == mask) {
			fdcIntOwner = 0;
			ret = 1;
		} else {
			ret = 0;
		}
	}

	return ret;
}


CON	fdccon	= {
	DFBLK | DFCHR,			/* Flags */
	FL_MAJOR,			/* Major index */
	fdcopen,			/* Open */
	fdcclose,			/* Close */
	fdcblock,			/* Block */
	fdcread,			/* Read */
	fdcwrite,			/* Write */
	fdcioctl,			/* Ioctl */
	NULL,				/* Powerfail */
	fdctimeout,			/* Timeout */
	fdcload,			/* Load */
	fdcunload,			/* Unload */
	NULL				/* Poll */
};
