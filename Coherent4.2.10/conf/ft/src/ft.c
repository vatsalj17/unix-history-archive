/*
 * Floppy tape device control.
 * Requires 765 controller module fdc.c
 *
 * FDC = floppy disk controller, NEC upd765 or compatible.
 *
 * This driver can handle only one floppy tape drive at a time!
 *
 * To do:
 *   ioctl: rewind, retension, wind to end of data.
 *   ioctl: format.
 *   ioctl: set offset to simulate a previous no rewind on close.
 *
 *   Dynamic buffer allocation.
 *   Sane response to signals received.
 *   FDC data rate control based on tape drive status.
 */

/*
 * Here is the protocol for QIC report commands, I think:
 * 
 * Send QIC report command to FDC (this will be an FDC seek command).
 * FDC interrupts when step pulses are sent.
 * Send Sense Interrupt Status to FDC.  (Clears interrupt line.)
 * Read interrupt status from FDC.
 * - Get ACK from tape drive.  Want Track 0 true.  Variable latency.
 * Do
 *   Send Sense Drive Status to FDC.
 *   Read drive status from FDC, including Track 0 bit.
 * Until Track 0 true.
 * - Get data bits for the report command.
 * For number-of-data-bits-in-report times
 *   Send QIC Report Next Bit command to FDC.
 *   FDC interrupts when step pulses are sent.
 *   Send Sense Interrupt Status to FDC.
 *   Read interrupt status from FDC.
 *   Send Sense Drive Status to FDC.
 *   Read drive status from FDC, including Track 0 bit.
 *   Save Track 0 bit value into report data.
 * End for
 * - Get final Track 0 true from tape drive.
 * Send QIC Report Next Bit command to FDC.
 * FDC interrupts when step pulses are sent.
 * Send Sense Interrupt Status to FDC.
 * Read interrupt status from FDC.
 * Send Sense Drive Status to FDC.
 * Read drive status from FDC, including Track 0 bit.
 * Track 0 bit must be true.
 */

/*
 * ----------------------------------------------------------------------
 * Includes.
 */
#include	<sys/coherent.h>
#include	<coh/defer.h>
#include	<coh/i386lib.h>
#include	<coh/misc.h>
#include	<coh/proc.h>
#include	<coh/timeout.h>

#include	<kernel/v_types.h>

#include	<errno.h>
#include	<sys/buf.h>
#include	<sys/cmn_err.h>
#include	<sys/con.h>
#include	<sys/devices.h>
#include	<sys/fdc765.h>
#include	<sys/fdioctl.h>
#include	<sys/file.h>
#include	<sys/mmu.h>
#include	<sys/seg.h>
#include	<sys/stat.h>
#include	<sys/tape.h>

#include	<sys/ft.h>
#include	<sys/ftioctl.h>
#include	<sys/ftx.h>

#define FT_MAX_PAGES	1024	/* Max # of pages allowed in buffer pool. */
#define FT_MAX_NBUF	127	/* NBUF patch value is limited to this.	*/
#define FT_SIGNATURE	0xAA55AA55	/* Start of header segment.	*/

#define FT_BLK_PER_SEG	32
#define FT_PAGES_PER_SEG	(FT_SEG_SIZE / NBPC)

#define FT_ACK_TRIES	2000	/* max # of tries for ACK to QIC rpt cmd*/
#define FT_CAL_TRIES	2	/* max # of tries for seek load point	*/

#define FT_OPEN_ERR(msg)	{devmsg(dev,msg);goto badFtOpen;}
#define FT_READ_ERR(msg)	{devmsg(dev,msg);goto badFtRead;}
#define FT_WRITE_ERR(msg)	{devmsg(dev,msg);goto badFtWrite;}

/*
 * CMS Jumbo 120 drive locks up permanently (til next power cycle) unless
 * each QIC-117 command is followed by a nominal delay of at least 2.5 msec.
 * For all commands which leave the tape stopped on completion,
 * we enlarge this considerably (and still see occasional lockups!).
 */
#define FT_MOTION_TICKS 20

#define FT_DRIVE(unit)		(unit)

/* QIC-117 says Motor On line from FDC is ignored by the tape unit. */
#define FT_MOTOR(unit)		FDC_MOTOR_OFF

/*
 * Tunable values.
 *
 * FT_NBUF = number of 32 Kbyte segment buffers allocated.  In the
 *   present memory model, this buffer space cannot be used for
 *   anything else, and is part of the "PhysMem" pool reserved at
 *   startup.  The buffer set is attached at ftBigBuf during ftload().
 * FT_SOFT_SELECT = 0 for hard select, 1 for A/M/S protocol, 2 for CMS.
 *   If soft select is used, the unit bits in the minor number are
 *   ignored and all commands to the FDC use unit zero.
 *   A/M/S = Archive/Conner/Mountain/Summit.
 *   CMS = Colorado Memory Systems.
 * FT_AUTO_CONFIG - tries to guess which type of tape drive is installed
 *   by getting status from each possibility in turn.  The testing is
 *   done in the driver load routine.
 */

extern int	FT_NBUF;
extern int	FT_SOFT_SELECT;
extern int	FT_AUTO_CONFIG;

/*
 * FT_CUSHION = number of segments we try to have between current
 *   physical position of tape when stopped and the next segment to
 *   be accessed.  The cushion allows time for the tape to come up to
 *   speed.
 */
int		FT_CUSHION = 8;	/* # of segs passed during start/stop	*/

/* Data rate is 500 Kbits/sec.  This will probably not change. */
int		ftrate = 0;

/*
 * DMA store.
 *   ftHeaderSeg is the virtual address of a 1 Kbyte aligned virtually
 *     contiguous buffer area 32 Kbytes in length, for the tape header info.
 *   ftBigBuf is the virtual address of a 1 Kbyte aligned, virtually contiguous
 *     buffer area, FT_NBUF segments in length.
 *   ftNumBlks [i] is the number of valid blocks in segment buffer i, during
 *     reads.  At the end of ftReadSegments (), valid blocks are packed
 *     toward the start of the segment.
 *   ftNumBytes [i] is the number of valid bytes in segment buffer i, during
 *     writes.  At the start of ftWriteSegments (), blocks are unpacked
 *     within the segment according to the bad block table.
 */

static unsigned char	* ftPool;
static unsigned char	* ftPooltop;
static __paddr_t	ftPoolPaddr [FT_MAX_PAGES];

static unsigned char	* ftHeaderSeg;
static unsigned char	* ftBigBuf;

extern struct FtSegErr	ftSegErr [];
extern unsigned char	ftNumBlks [];

extern unsigned int	ftNumBytes [];

/* Parameters for FDC Specify Command */
int FT_RATE = FDC_RATE_500K;
int FT_SRT_2 = 0xE;
int FT_SRT_3 = 0xD;
int FT_HUT = 0xF;
int FT_HLT = 0x1;

/* Things that depend on the data rate. */

struct FtRateParameters {
	int	srt;	/* FDC Stepping Rate */
	int	ftRate;
	int	fdcRate;
};

struct FtRateParameters ftRateParms [] = {
	{ 0xe, FT_DATA_RATE_500K, FDC_RATE_500K },
	{ 0xf, FT_DATA_RATE_250K, FDC_RATE_250K },
	{ 0xd, FT_DATA_RATE_1MEG, FDC_RATE_1MEG },
};

struct FT	ft;

/*********************
**********************
**
** Debug Area.
**
**********************
**********************/

#if FT_TRACE

/* Simulation of read/write errors and bad blocks in the bitmap. */

int	fakeBBTbitmap;
int	fakeBBTsegment;
int	fakeIOerrblk;

extern unsigned long __bolt;

static	char	*qicErr[] = {
	"NULL err",
	"command received while drive not ready",
	"cartridge not present or removed",
	"motor speed error (not within 1%)",
	"motor speed fault (jammed, or gross speed error)",
	"cartridge write protected",
	"undefined or reserved command code",
	"illegal track address specified for seek",
	"illegal command in report subcontext",
	"illegal entry into a diagnostic mode",
	"broken tape detected (based on hole sensor)",
	"warning - read gain setting error",
	"command received while error status pending (obsolete)",
	"command received while new cartridge pending",
	"command illegal or undefined in primary mode",
	"command illegal or undefined in format mode",
	"command illegal or undefined in verify mode",
	"logical forward not at logical BOT in format mode",
	"logical EOT before all segments generated",
	"command illegal when cartridge not referenced",
	"self-diagnostic failed (cannot be cleared)",
	"warning EEPROM not initialized, defaults set",
	"EEPROM corrupt or hardware failure",
	"motion timeout error",
	"data segment too long - logical forward or pause",
	"transmit overrun (obsolete)",
	"power on reset occurred",
	"software reset occurred",
	"diagnostic mode 1 error",
	"diagnostic mode 2 error",
	"command received during noninterruptible process",
	"rate selection error",
	"illegal command while in high speed mode",
	"illegal seek segment value"
};

static	char	*qicStat[] = {
	"drive ready or idle",
	"error detected",
	"cartridge present",
	"cartridge write protected",
	"new cartridge",
	"cartridge referenced",
	"at physical BOT",
	"at physical EOT"
};

static	char	*qicCmd[] = {
	"NULL cmd",
	"soft reset",
	"report next bit",
	"pause",
	"micro step pause",
	"alternate command timeout",
	"report drive status",
	"report error code",
	"report drive configuration",
	"report ROM version",
	"logical forward",
	"physical reverse",
	"physical forward",
	"seek head to track",
	"seek load point",
	"enter format mode",
	"write reference burst",
	"enter verify mode",
	"stop tape",
	"reserved (19)",
	"A/M/S soft select 2",
	"micro step head up",
	"micro step head down",
	"A/M/S soft select 1",
	"A/M/S soft deselect",
	"skip n segments reverse",
	"skip n segments forward",
	"select rate",
	"enter diag mode 1",
	"enter diag mode 2",
	"enter primary mode",
	"reserved (31)",
	"report vendor ID",
	"report tape status",
	"skip n segments extended reverse",
	"skip n segments extended forward",
	"calibrate tape length",
	"report format segments",
	"set N format segments",
	"reserved (39)",
	"reserved (40)",
	"reserved (41)",
	"reserved (42)",
	"reserved (43)",
	"reserved (44)",
	"reserved (45)",
	"CMS soft select",
	"CMS soft deselect",
};

/* print command as [command] */
#if	__USE_PROTO__
static void ftDbPrintCmd (unsigned int cmd)
#else
static void
ftDbPrintCmd(cmd)
unsigned int cmd;
#endif
{
	if (cmd >= 1 && cmd < sizeof(qicCmd)/sizeof(qicCmd[0])) {
		if (cmd != QIC_CMD_RNB)
			cmn_err (CE_CONT, "![%s] %ld\n", qicCmd[cmd], __bolt);
	} else
		cmn_err (CE_CONT, "![%x]\n", cmd);
}

/* print 2-byte error status as <error-code,command> */
#if	__USE_PROTO__
static void ftDbPrintErr (unsigned int errword)
#else
static void
ftDbPrintErr(errword)
unsigned int errword;
#endif
{
	unsigned int lo, hi;

	lo = errword & 0xff;
	hi = (errword >> 8) & 0xff;

	if (lo >= 1 && lo < sizeof(qicErr)/sizeof(qicErr[0]))
		cmn_err (CE_CONT, "!<%s,", qicErr[lo]);
	else
		cmn_err (CE_CONT, "!<%x,", lo);

	if (hi >= 1 && hi < sizeof(qicCmd)/sizeof(qicCmd[0]))
		cmn_err (CE_CONT, "!%s>\n", qicCmd[hi]);
	else
		cmn_err (CE_CONT, "!%x>\n", hi);
}

/* print tape status as { status string,... } */
#if	__USE_PROTO__
void ftDbPrintStat (unsigned int stat)
#else
void
ftDbPrintStat(stat)
unsigned int stat;
#endif
{
	int i, pcount;

	cmn_err (CE_CONT, "!{ ");
	pcount = 0;
	for (i = 0; i < 8; i++) {
		if (stat & (1 << i)) {
			if (pcount > 0 && (pcount % 2) == 0)
				cmn_err (CE_CONT, "!\n");
			cmn_err (CE_CONT, "!%s, ", qicStat[i]);
			pcount++;
		}
	}
	cmn_err (CE_CONT, "!}\n");
}

#endif

/*********************
**********************
**
** Level 0 Support routines.
** (These don't call other routines in this module.)
**
**********************
**********************/

/************************************************************************
 * ftBufVtop
 *
 * Compute physical address for a buffer pool virtual address.
 * Return 0 on success, 1 in case address out of range.
 ***********************************************************************/

#if	__USE_PROTO__
int ftBufVtop (__caddr_t vadd, __paddr_t * paddp)
#else
int
ftBufVtop (vadd, paddp)
__caddr_t vadd;
__paddr_t * paddp;
#endif
{
	int pageIndex;

	if ((unsigned int)vadd < (unsigned int)ftPool
	  || (unsigned int)vadd >= (unsigned int)ftPooltop) {
		cmn_err (CE_WARN, "FT : vtop (%x), addr outside range %x-%x",
		  (int)vadd, (int)ftPool, (int)ftPooltop);
		return 1;
	}

	pageIndex = ((int)vadd - (int)ftPool) / NBPC;
	* paddp = ftPoolPaddr [pageIndex] | ((unsigned int)vadd % NBPC);

	return 0;
}

/************************************************************************
 * ftAlloc
 *
 * Seize pages for the tape buffer.
 * In case of failure, leave ftBigBuf at its initial value of 0 (NULL).
 ***********************************************************************/

#if	__USE_PROTO__
void ftAlloc (void)
#else
void
ftAlloc ()
#endif
{
	__caddr_t	devVaddr;
	int		devSeg;
	int		s;
	int		pagesNeeded;
	int		pte;
	int		i;
	__paddr_t	tmpPaddr;
	__caddr_t	pageTableVaddr;
	int		* ptable;
	int		usablePages;

	/*
	 * This technique limits us to 4 MBytes of buffer pool, or
	 * 127 tape segment buffers in the pool plus one more for
	 * the header segment.
	 */

	if (FT_NBUF > FT_MAX_NBUF)
		FT_NBUF = FT_MAX_NBUF;

	/* Expand paging services to allow for this buffer area. */

	pagesNeeded = (FT_NBUF + 1) * FT_PAGES_PER_SEG;

	/*
	 * See if there are enough pages to do the allocation.
	 * Allow about 64 extra since the system will need some room to run.
	 * Since we are allocating segment buffers, total number of pages
	 * we can allocate must be a multiple of 32 KBytes.
	 */

	usablePages = allocno () - 32;
	usablePages -= (usablePages % FT_PAGES_PER_SEG);

	if (usablePages < pagesNeeded) {
		if (usablePages < 4) {
			cmn_err (CE_WARN,
			  "FT : Not enough RAM for buffers");
			return;
		}

		cmn_err (CE_NOTE,
		  "FT : Reducing number of buffers from %d to %ld",
		  FT_NBUF, usablePages - FT_PAGES_PER_SEG);

		FT_NBUF = usablePages - FT_PAGES_PER_SEG;
	}

	/* Compute the virtual address where device buffer memory starts. */

	devVaddr = DEVICE_SEG_VADDR(FL_MAJOR);

	/* Compute the segment number of the 4 MByte-aligned virtual segment. */

	devSeg = btosrd ((int)devVaddr);

	s = sphi ();

	/* Grab a page to map the buffer area for this device. */

	tmpPaddr = (__paddr_t)(ctob (* -- sysmem.pfree));
	pageTableVaddr = __PTOV(tmpPaddr);

	pte = (int)(tmpPaddr) | SEG_RW;
	ptable0_v [devSeg] = pte;
	
	/* Grab and map the specified number of pages for the device buffer. */

	ptable = (int *)pageTableVaddr;

	for (i = 0; i < pagesNeeded; i++) {
		tmpPaddr = (__paddr_t)(ctob (* -- sysmem.pfree));
		ftPoolPaddr [i] = tmpPaddr;
		* ptable ++ = ((int) tmpPaddr) | SEG_RW;
	}

	spl (s);
	mmuupd ();

	ftPool = (unsigned char *)devVaddr;
	ftPooltop = ftPool + pagesNeeded * NBPC;

	cmn_err (CE_NOTE, "!ftAlloc () - using %d pages at virt address %x",
	  pagesNeeded, (int)devVaddr);

	return;
}

/*************************************************************************
 * ftAutoConfig
 *
 * Return 0 on success, nonzero on failure.
 ************************************************************************/

/* Here is the list of things to try, in order of decreasing likelihood. */
struct PossibleCfg {
	unsigned char drive;
	unsigned char softsel;
} pcfg [] = {
	{ 0, 2 },	/* CMS soft select */
	{ 0, 1 },	/* A/C/M/S soft select */
	{ 1, 0 },	/* Unit 1 hard select */
	{ 2, 0 },	/* Unit 2 hard select */
	{ 3, 0 },	/* Unit 3 hard select */
	{ 1, 2 },	/* CMS soft select, unit 1 */
	{ 2, 2 },	/* CMS soft select, unit 2 */
	{ 3, 2 },	/* CMS soft select, unit 3 */
	{ 0, 0 },	/* Unit 0 hard select */
};

#if	__USE_PROTO__
static int ftAutoConfig (void)
#else
static int
ftAutoConfig ()
#endif
{
	int		found;
	int		index;
	int		retval;
	int		drvStatus;

	/* If can't get FDC interrupt, fail now. */
	if (!setFtIntr(1)) {
#if FT_TRACE
		cmn_err (CE_WARN, "FT : Auto Config : Can't get IRQ.");
#endif
		return 1;
	}

	/* Run through configuration possibilities. */
	for (index = 0, found = 0;
	  !found && index < __ARRAY_LENGTH(pcfg); index++) {

		/* Try a select. */
		ftSelect(pcfg [index].drive, pcfg [index].softsel);

		/* Select worked if we get valid status. */
		drvStatus = ftStsWthErr();
#if FT_TRACE
		cmn_err (CE_NOTE, "!ftAutoConfig index=%d stat=%x",
		  index, drvStatus);
#endif

		if (drvStatus != -1) {
#if FT_TRACE
			cmn_err (CE_NOTE, "!ftAutoConfig found at index=%d",
			  index);
#endif
			found = 1;
			ft.ft_drv = pcfg [index].drive;
			ft.ft_softsel = pcfg [index].softsel;
		}
	}

	/* If we succeeded at soft select, deselect now. */
	if (found) {
		switch (ft.ft_softsel) {
		case 0:
			break;
		case 1:
			/* A/C/M/S soft select disable. */
			ftCmd(QIC_CMD_SS_OFF);
			break;
		case 2:
			/* CMS soft select disable. */
			ftCmd(QIC_CMD_CSS_OFF);
			break;
		}

		/* Successful return status. */
		retval = 0;
	} else
		/* Unsuccessful return status. */
		retval = 1;

	setFtIntr(0);

	return retval;
}

/************************************************************************
 * ftCmdSend
 *
 * Given a QIC-117 command number, cause that number of step pulses
 * to be sent from the FDC by faking a seek command.
 ***********************************************************************/

#if	__USE_PROTO__
static void ftCmdSend (int cmd)
#else
static void
ftCmdSend(cmd)
int cmd;
#endif
{
	/* ncn = new cylinder #. */
	unsigned char ncn;
	
	/*
	 * Will fake a seek command.
	 * Figure out whether to simulate seek to lower or higher
	 * cylinder number.
	 */
	if (ft.ft_pcn >= cmd)
		ncn = ft.ft_pcn - cmd;
	else
		ncn = ft.ft_pcn + cmd;

	fdcSeek(ft.ft_drv, 0, ncn);
}

#define FT_DATA_BLK_PER_SEG	29
#define FT_PAGES_PER_SEG	(FT_SEG_SIZE / NBPC)

/* Byte offsets within the header segment. */

#define FT_FIRST_DATA_SEG	10
#define FT_LAST_DATA_SEG	12
#define FT_START_BBT			(FT_BLK_SIZE * 2)
#define FT_END_BBT			(FT_BLK_SIZE * 29)

/************************************************************************
 * ftBBT
 *
 * Look up a logical segment number in the bad block table.
 *
 * Return 0 if successful, -1 if no valid header or if bad segment number.
 ***********************************************************************/

#if	__USE_PROTO__
int ftBBT (int segment, unsigned int * bbtValp)
#else
int
ftBBT (segment, bbtValp)
int segment;
unsigned int * bbtValp;
#endif
{
	unsigned int	* bbt;

	if (!ft.ft_hdrOk) {
#if FT_TRACE
		cmn_err (CE_WARN, "FT ftBBT(%d,x) : no BBT available", segment);
#endif
		return -1;
	}

	if (segment < 0 || segment > ft.ft_lastDataSeg) {
#if FT_TRACE
		cmn_err (CE_WARN, "FT ftBBT(%d,x) : invalid segment", segment);
#endif
		return -1;
	}

#if FT_TRACE
	if (segment == fakeBBTsegment) {

		* bbtValp = fakeBBTbitmap;

	} else {

		bbt = (unsigned int *)(ftHeaderSeg + FT_START_BBT);
		* bbtValp = bbt [segment];

	}
#else
	bbt = (unsigned int *)(ftHeaderSeg + FT_START_BBT);
	* bbtValp = bbt [segment];
#endif

	return 0;
}

/************************************************************************
 * ftAdjSeek
 *
 * Adjust a seek address to account for the offset of data on tape
 * (after any initial bad segments and header segments) and for blocks
 * voided by the bad block map.  Return a logical segment number, and
 * a byte offset into that segment.  Assumes good blocks are already
 * packed from the start of the segment.
 *
 * Return 0 on success, -1 if seek offset is negative or if attempting
 * to seek when header segment, which contains the bad block map, is not
 * available, -2 if attempting to seek past data area on tape.
 ***********************************************************************/

#if	__USE_PROTO__
static int ftAdjSeek (int seekIn, int * segOutp, int * offsetp)
#else
static int
ftAdjSeek(seekIn, segOutp, offsetp)
int seekIn;
int * segOutp;
int * offsetp;
#endif
{
	int		found;
	int		byteOffset;
	int		segment;
	unsigned int	map;
	int		goodBlocks;
	int		goodBytes;

	/* The following are valid if ft.ft_seekAdj is nonzero. */
	static int	prevSeekIn;
	static int	prevSegment;

	/* Offset all seek operations to account for no rewind on close. */
	seekIn += ft.ft_segbase;

	if (seekIn < 0) {
#if FT_TRACE
		cmn_err (CE_WARN, "FT : seek to %d, before BOT", seekIn);
#endif
		return -1;
	}

	if (!ft.ft_hdrOk) {
#if FT_TRACE
		cmn_err (CE_WARN, "FT : seek to %d, no BBT available", seekIn);
#endif
		return -1;
	}

	found = 0;

	/*
	 * Convert Bad Block Table byte offsets in header to integer
	 * addresses.
	 *
	 * Since this is a sequential calculation, work from previous
	 * results whenever possible.
	 */

	if (ft.ft_seekAdj && seekIn >= prevSeekIn) {
		segment = prevSegment;
		byteOffset = prevSeekIn;
	} else {
		segment = ft.ft_firstDataSeg;
		byteOffset = 0;
	}

	while (ftBBT (segment, & map) == 0) {

		/*
		 * Step through bad block bitmaps, one at a time.
		 *
		 * Use bad block bitmap to determine # of good
		 * blocks in this segment.
		 */

		goodBlocks = FT_DATA_BLK_PER_SEG;
		while (map != 0) {
			goodBlocks--;
			if (goodBlocks <= 0)
				break;
			/* Clear a 1 bit in the bitmap. */
			map &= (map - 1);
		}

		goodBytes = goodBlocks * FT_BLK_SIZE;

		/*
		 * When adjustment is done, save values that correspond to
		 * start of current segment, for next time.
		 */
		if (seekIn < byteOffset + goodBytes) {

			prevSeekIn = byteOffset;
			prevSegment = segment;

			ft.ft_seekAdj = 1;
			found = 1;

			break;
		}

		/* Update for next iteration. */

		byteOffset += goodBytes;
		segment++;
		if (segment > ft.ft_lastDataSeg) {
#if FT_TRACE
			cmn_err (CE_WARN, "FT : seek to %d, past EOT", seekIn);
#endif
			return -2;
		}
	}

	if (!found) {
#if FT_TRACE
		cmn_err (CE_WARN, "FT : seek to %d, past BBT", seekIn);
#endif
		return -1;
	}

	* segOutp = segment;
	* offsetp = seekIn - byteOffset;

	return 0;
}

#if 0
/************************************************************************
 * ftRecal
 *
 * Send Recalibrate command to FDC and wait for it to finish.
 * Return 0 if normal operation, -1 if signaled before recal complete.
 ***********************************************************************/

#if	__USE_PROTO__
static int ftRecal (void)
#else
static int
ftRecal()
#endif
{
	ft.ft_wakeMeUp = 1;
	fdcRecal(ft.ft_drv);
	if (x_sleep ((char *)(&ft.ft_wakeMeUp), pritape,
	  slpriSigCatch, "ftRecal"))
		return -1;
	else
		return 0;
}
#endif

/************************************************************************
 * ftRptBegin
 *
 * Initialize ft state information in preparation for QIC report command.
 *
 * Argument "bitCount" is the total number of bits expected, including
 * initial ACK and final TRUE.  It is 1 more than the number of
 * Report Next Bit Commands that will be issued.
 ***********************************************************************/

#if	__USE_PROTO__
static void ftRptBegin (int bitCount)
#else
static void
ftRptBegin(bitCount)
int bitCount;
#endif
{
	ft.ft_bitsNeeded = bitCount;
	ft.ft_bitsRcvd = 0;
	ft.ft_report = 0;
	ft.ft_ackNeeded = (bitCount) ? 1 : 0;
}

/************************************************************************
 * ftRptUpdate
 *
 * Acquire another bit for QIC report command.
 * Last bit is discarded.  Other bits are accumulated,
 * least significant bit first, into ft_report.
 ***********************************************************************/

#if	__USE_PROTO__
static void ftRptUpdate (int bit)
#else
static void
ftRptUpdate(bit)
int bit;
#endif
{
	ft.ft_bitsNeeded--;
	ft.ft_bitsRcvd++;

	if (ft.ft_bitsNeeded == 0) {
		if (bit != 1) {
#if FT_TRACE
			cmn_err (CE_WARN, "!Missing final TRUE ");
#endif
			ft.ft_ftruMissed = 1;
		}
	} else {
		ft.ft_report |= (bit << (ft.ft_bitsRcvd - 1));
	}
}

/***********************************************************************
 * ftTimeout
 *
 * Called from timeout () if FDC takes too long to do an operation.
 ***********************************************************************/

#if	__USE_PROTO__
static int ftTimeout (int __NOTUSED(to_type))
#else
static int
ftTimeout (to_type)
int	to_type;
#endif
{
	ft.ft_timedOut = 1;
	wakeup (& ft.ft_wakeMeUp);
	return 0;
}

/************************************************************************
 * ftWait
 *
 * Wait the specified number of ticks.
 * Return 0 if full wait occurs, -1 if signaled.
 * Uses sleep, so may not be used in timeout/interrupt/load/unload, etc.
 ***********************************************************************/

#if	__USE_PROTO__
int ftWait (unsigned int ticks)
#else
int
ftWait(ticks)
unsigned int ticks;
#endif
{

	timeout (& ft.ft_tim, ticks, (__TIMED_FN_PTR)wakeup,
	  (int)(& ft.ft_wakeMeUp));
	return x_sleep ((char *)(& ft.ft_wakeMeUp), pritape,
	  slpriSigCatch, "ftWait");
}

/*********************
**********************
**
** Level 1 Support routines.
**
**********************
**********************/

/************************************************************************
 * ftCmd
 *
 * Given a QIC-117 command number, send the command.
 * If report bits are expected in response, initialize the bit counter.
 * Then sleep until the commmand is done and report bits are gathered.
 ***********************************************************************/

#if	__USE_PROTO__
int ftCmd (int cmd)
#else
int
ftCmd(cmd)
int cmd;
#endif
{
	int	bitsNeeded;

#if FT_TRACE
	ftDbPrintCmd(cmd);
#endif

	/* Will sleep until command done and report bits are in. */
	ft.ft_wakeMeUp = 1;

	/*
	 * The following commands expect report bits from the tape drive.
	 * After receiving the QIC command (and subsequent delay),
	 * the drive sends a leading ACK bit (always 1).  This bit is
	 * not counted in the numbers below.
	 * Subseqeunt bits are sent in response to Report Next Bit,
	 * least significant bit first, then a trailing 1.
	 * The value of bitsNeeded is either one more than the number of
	 * data bits in the report, or zero.

	 */
	switch(cmd) {
	case QIC_CMD_STS:
	case QIC_CMD_DRVCN:
	case QIC_CMD_ROMVN:
	case QIC_CMD_TPSTAT:
		bitsNeeded = 9;
		break;
	case QIC_CMD_ECD:
	case QIC_CMD_VNDID:
		bitsNeeded = 17;
		break;
	default:
		bitsNeeded = 0;
	}
	ftRptBegin(bitsNeeded);
	ft.ft_ackMissed = 0;
	ft.ft_ftruMissed = 0;

	ftCmdSend(cmd);

	/* Do not allow signals during QIC-117 protocol - tape drive hangs. */
	timeout (& ft.ft_tim, 50, ftTimeout, NULL);
	if (x_sleep ((char *)(&ft.ft_wakeMeUp), pritape,
	  slpriNoSig, "ftCmd")) {

#if FT_TRACE
		cmn_err (CE_WARN, "!ftCmd: sig to cmd %d", cmd);
#endif
		/* Signal woke us prematurely. */
		return -1;

	} else if (ft.ft_ackMissed) {

#if FT_TRACE
		cmn_err (CE_WARN, "!ftCmd: Ack missed");
#endif
		ft.ft_ackMissed = 0;
		return -1;

	} else if (ft.ft_ftruMissed) {

#if FT_TRACE
		cmn_err (CE_WARN, "!ftCmd: Final true missed");
#endif
		ft.ft_ftruMissed = 0;
		return -1;
	} else
		return 0;
}

/************************************************************************
 * ftCmdArg
 *
 * Like ftCmd, except this routine sends command arguments.
 * No setup for report next bit.
 * Different debug print handling.
 * QIC-117 says add 2 to each argument byte.
 ***********************************************************************/

#if	__USE_PROTO__
static int ftCmdArg (int arg)
#else
static int
ftCmdArg(arg)
int arg;
#endif
{
	int	bitsNeeded;

#if FT_TRACE
	cmn_err (CE_CONT, "!(%x)\n", arg);
#endif

	/* Will sleep until stepper pulses sent. */
	ft.ft_wakeMeUp = 1;

	bitsNeeded = 0;
	ftRptBegin(bitsNeeded);
	ft.ft_ackMissed = 0;

	ftCmdSend (arg + 2);

	/* Do not allow signals during QIC-117 protocol - tape drive hangs. */
	timeout (& ft.ft_tim, 50, ftTimeout, NULL);
	if (x_sleep ((char *)(&ft.ft_wakeMeUp), pritape,
	  slpriNoSig, "ftArg")) {

		/* Signal woke us prematurely. */
#if FT_TRACE
		cmn_err (CE_WARN, "ftCmdArg: sig to arg %d", arg);
#endif

		return -1;
	} else
		return 0;
}

/************************************************************************
 * ftIrqHandler
 *
 * Interrupt handler.
 ***********************************************************************/
#if	__USE_PROTO__
static void ftIrqHandler (void)
#else
static void
ftIrqHandler()
#endif
{
	int i, bit;

	/*
	 * Need to get FDC status from result phase - fdcCmdStatus -
	 * or clear interrupt - fdcIntStatus - that may have been
	 * generated by diskette change or seek/recal complete.
	 */
	if (FDC_BUSY()) {
		fdcCmdStatus();
	} else {
		fdcIntStatus();
/* WARNING - should squawk if wrong number of status bytes. */
		ft.ft_pcn = fdc.fdc_intstat[1];
	}

	/* If the FDC-block-transfer in progress, route IRQ handler there. */
	if (ftx.ftx_IRQpending) {
		ftx.ftx_IRQpending = 0;
		ftxIntr ();
	}

	/* If ACK needed, try several times to get it. */
	if (ft.ft_ackNeeded) {

	for (i = 0; i < FT_ACK_TRIES; i++) {

			fdcDrvStatus(ft.ft_drv, FDC_HEAD_0);
			fdcCmdStatus();
			bit = (fdc.fdc_cmdstat[0] & ST3_T0) ? 1 : 0;

#if 0
			cmn_err (CE_CONT, bit ? "!|" : "!o");
#endif

			if (bit) {	
				ft.ft_ackNeeded = 0;
				break;
			}

			/* Wait about 20 usec. then try again. */
			busyWait2 (NULL, 20);
		}

		/*
		 * Tape Drive did not send ACK in response to QIC
		 * Report command.  Probably the command is not
		 * supported by this drive, or there is no drive
		 * present.  Set ackMissed flag and clean up.
		 */
		if (ft.ft_ackNeeded) {
			ft.ft_ackNeeded = 0;
			ft.ft_ackMissed = 1;
			ft.ft_bitsNeeded = 0;
			if (ft.ft_wakeMeUp) {
				ft.ft_wakeMeUp = 0;
				wakeup(&ft.ft_wakeMeUp);
			}
		}
	} else if (ft.ft_bitsNeeded) {

		/* Report bits are needed.  Get ST3. */
		fdcDrvStatus(ft.ft_drv, FDC_HEAD_0);
		fdcCmdStatus();

		/* Get next report bit by checking Track Zero bit in ST3 */
		if (fdc.fdc_ncmdstat == 1) {
			int bit;

			bit = (fdc.fdc_cmdstat[0] & ST3_T0) ? 1 : 0;
			ftRptUpdate(bit);
#if FT_TRACE
		} else {
			cmn_err (CE_WARN, "!ftIrqHandler: rnb status bad ");
#endif
		}
	}

	/*
	 * If more report bits will be needed
	 *   Send request for next bit.
	 * Else
	 *   See if original requestor needs wakeup, etc.
	 */
	if (ft.ft_bitsNeeded) {
#if 1
		itimeout ((__tfuncp_t) ftCmdSend, (__VOID__ *) QIC_CMD_RNB,
		  2, pltimeout);
#else
		ftCmdSend(QIC_CMD_RNB);
#endif
	} else {
		if (ft.ft_wakeMeUp) {
			ft.ft_wakeMeUp = 0;
			wakeup(&ft.ft_wakeMeUp);
		}
	}
}

/************************************************************************
 * ftResetFDC
 *
 * Reset the FDC and wait for the resulting interrupt.
 * Reset is done keeping the current unit selected.
 ***********************************************************************/

#if	__USE_PROTO__
static void ftResetFDC (unsigned int drive)
#else
static void
ftResetFDC(drive)
unsigned int drive;
#endif
{
	/*
	 * Since FDC reset generates an interrupt, we need to tell the
	 * interrupt handler there will be *no* report bits incoming.
	 */
	ftRptBegin(0);

	ft.ft_pcn = 0;
	ft.ft_wakeMeUp = 1;
	fdcResetSel(FT_DRIVE(drive), FT_MOTOR(drive));
	x_sleep ((char *)(&ft.ft_wakeMeUp), pritape,
	  slpriSigCatch, "ftRstFDC");
}

/************************************************************************
 * ftDataRate
 *
 * Set data rate at tape drive.
 * Return 0 if success, -1 if timed out before sensing drive ready.
 ***********************************************************************/
#if	__USE_PROTO__
int ftDataRate(int rate)
#else
int
ftDataRate(rate)
int rate;
#endif
{
	ftCmd (QIC_CMD_RATE);
	ftCmdArg (rate);
	return ftReadyWait (10);
}

#if 0
/************************************************************************
 * ftSelRate
 *
 * Sel Tape Data Rate
 * Return 0 on success, nonzero on failure.
 ***********************************************************************/

#if	__USE_PROTO__
int ftSelRate (int rate)
#else
int
ftSelRate(rate)
int rate;
#endif
{
	/*
	 * rate 0=250 Kbits/sec:
	 *   SRT=0xF  HUT=0xF  HLT=0x1
	 *   fdcRate(FDC_RATE_250K);
	 *   ftDataRate(FT_DATA_RATE_250K);
	 *
	 * rate 1=500 Kbits/sec:
	 *   SRT=0xE  HUT=0xF  HLT=0x1
	 *   fdcRate(FDC_RATE_500K);
	 *   ftDataRate(FT_DATA_RATE_500K);
	 *
	 * rate 2=1000 Kbits/sec:
	 *   SRT=0xD  HUT=0xF  HLT=0x1
	 *   fdcRate(FDC_RATE_1MEG);
	 *   ftDataRate(FT_DATA_RATE_1MEG);
	 *
	 * fdcSpecify(srt, hut, hlt);
	 */
	int ft_rate, fdc_rate, srt, hut, hlt;
	char * rateName;

	switch (rate) {
	case 0:
		rateName = "250K";
		ft_rate = FT_DATA_RATE_250K;
		fdc_rate = FDC_RATE_250K;
		srt = 0xF;  hut = 0xF;  hlt=1;
		break;
	case 1:
		rateName = "500K";
		ft_rate = FT_DATA_RATE_500K;
		fdc_rate = FDC_RATE_500K;
		srt = 0xE;  hut = 0xF;  hlt=1;
		break;
	case 2:
		rateName = "1MEG";
		ft_rate = FT_DATA_RATE_1MEG;
		fdc_rate = FDC_RATE_1MEG;
		srt = 0xD;  hut = 0xF;  hlt=1;
		break;
	default:
		cmn_err (CE_WARN,
		  "ftSelRate : bad rate constant : %d (not 0, 1, 2)", rate);
		return 1;
	}

	/* If slow or fast rates ng., fall back to middle rate (500K). */

	if (ftDataRate (ft_rate)) {
		cmn_err (CE_WARN, "ftSelRate : can't set rate to %s", rateName);
		fdcRate(FDC_RATE_500K);
		fdcSpecify(0xE, 0xF, 1);
		return 1;
	} else {
		fdcRate(fdc_rate);
		fdcSpecify(srt, hut, hlt);
		cmn_err (CE_NOTE, "ft : rate set to %s", rateName);
		return 0;
	}
}
#endif

/************************************************************************
 * ftSelect
 *
 * Select tape unit.
 ***********************************************************************/

#if	__USE_PROTO__
void ftSelect (unsigned char drive, unsigned char softsel)
#else
void
ftSelect(drive, softsel)
unsigned char drive;
unsigned char softsel;
#endif
{
	fdcRate (ftRateParms [ftrate].fdcRate);
	fdcDrvSelect(FT_DRIVE(drive), FT_MOTOR(drive));

	/* Reset FDC and Initialize pseudo cylinder number for QIC commands. */
	ftResetFDC(drive);

	fdcSpecify(ftRateParms [ftrate].srt, FT_HUT, FT_HLT);

	switch (softsel) {
	case 0:
		break;
	case 1:
		/* A/C/M/S soft select enable. */
		ftCmd(77);
		ftWait(2);
		ftCmd(QIC_CMD_SS1);
		ftWait(2);
		ftCmd(QIC_CMD_SS2);
		ftWait(2);
		break;
	case 2:
		/* CMS soft select enable. */
		ftCmd(77);
		ftWait(2);
		ftCmd(QIC_CMD_CSS_ON);
		ftWait(2);
		ftCmdArg (drive);
		ftWait(2);
		break;
	}

	return;
}

/***********************************************************************
 * ftWaitWithTimeout
 *
 * Clear timeout flag.
 * Start timeout timer.
 * Sleep.
 * If timeout did not occur.
 *   Turn off timeout timer.
 * Print warning message if operation timed out or was signaled.
 *
 * Arguments:
 *   timeout interval, in clock ticks
 *   sleep reason string
 *
 * Return status:
 *	0: normal wakeup	FT_RESULT_OK
 *	1: timeout		FT_RESULT_TIMED_OUT
 *	2: signal received	FT_RESULT_SIGNALED
 ***********************************************************************/

#if	__USE_PROTO__
enum FtWtRslt ftWaitWithTimeout (int ticks, char * reason)
#else
enum FtWtRslt
ftWaitWithTimeout (ticks, reason)
int	ticks;
char *	reason;
#endif
{
	int	sleepval;
	int	retval;

	ft.ft_timedOut = 0;
	timeout (& ft.ft_tim, ticks, ftTimeout, NULL);
	sleepval = x_sleep ((char *)(&ft.ft_wakeMeUp), pritape,
	  slpriSigCatch, reason);
	timeout (& ft.ft_tim, 0, NULL, NULL);

	if (sleepval) {
#if FT_TRACE
		cmn_err (CE_WARN, "FT %s: Signaled", reason);
#endif
		retval = FT_RESULT_SIGNALED;
	} else if (ft.ft_timedOut) {
#if FT_TRACE
		cmn_err (CE_WARN, "FT %s: Timeout", reason);
#endif
		retval = FT_RESULT_TIMED_OUT;
	} else
		retval = FT_RESULT_OK;

	return retval;
}

/*********************
**********************
**
** Level 2 Support routines.
**
**********************
**********************/

#if 0
/************************************************************************
 * ftGetInfo
 *
 * Issue several QIC report commands - all but Report Drive Status
 * and Report Error Code.
 ***********************************************************************/

#if	__USE_PROTO__
static void ftGetInfo(o_dev_t dev)
#else
static void
ftGetInfo(dev)
o_dev_t dev;
#endif
{
	if (ftCmd(QIC_CMD_DRVCN))
		devmsg(dev, "Drive configuration unavailable");
	else
		devmsg(dev, "Drive Configuration = %x", ft.ft_report);

	if (ftCmd(QIC_CMD_ROMVN))
		devmsg(dev, "ROM version unavailable");
	else
		devmsg(dev, "Rom Version = 0x%x", ft.ft_report);

	if (ftCmd(QIC_CMD_VNDID))
		devmsg(dev, "Vendor ID unavailable");
	else
		devmsg(dev, "Vendor ID = 0x%x", ft.ft_report);

	if (ftCmd(QIC_CMD_TPSTAT))
		devmsg(dev, "Tape status unavailable");
	else
		devmsg(dev, "Tape Status = 0x%x", ft.ft_report);
}
#endif

/***********************************************************************
 * ftReadyWait
 *
 * Keep checking drive status every second until drive is ready or
 * the specified number of seconds has elapsed.
 * Return 0 if drive ready, -1 if timeout or signal or protocol failure.
 ***********************************************************************/

#if	__USE_PROTO__
int ftReadyWait (unsigned int seconds)
#else
int
ftReadyWait(seconds)
unsigned int seconds;
#endif
{
	uint i;
	int retval = -1;
	int drvStatus;

	for (i = 0; i < seconds; i++) {

		drvStatus = ftStsWthErr();
		if (drvStatus == -1) {
			continue;
		}

		/* Drive ready?  Change return value to show success. */
		if (drvStatus & QIC_STAT_RDY) {
			retval = 0;
			break;
		}

		/* Signal arrived while waiting a second? */
		if (ftWait(HZ))
			break;
	}

	return retval;
}

/*********************
**********************
**
** Global Support routines.
**
**********************
**********************/


/***********************************************************************
 * ftReadID
 *
 * Send FDC Read ID command and store results at pointer supplied.
 * Return 0 on success, -1 on failure (signal).
 *
 * Of principal interest is fdc.fdc_cmdstat[5] - present block number.
 ***********************************************************************/

#if	__USE_PROTO__
int ftReadID (struct ftDiskLocn * diskLoc)
#else
int
ftReadID (diskLoc)
struct ftDiskLocn * diskLoc;
#endif
{
	ft.ft_wakeMeUp = 1;
	fdcReadID (ft.ft_drv, 0);

	/* Wait for interrupt when read is done. */
	if (ftWaitWithTimeout (FT_XFER_SECS * HZ, "ftReadID")) {
		ftSelect(ft.ft_drv, ft.ft_softsel);
		return -1;
	}

	if (fdc.fdc_ncmdstat != 7) {
#if FT_TRACE
		cmn_err (CE_WARN, "ftReadID: bad command status: %d",
		  fdc.fdc_ncmdstat);
#endif
		return -1;
	} else {
		diskLoc->ftDLcylinder = fdc.fdc_cmdstat[3];
		diskLoc->ftDLhead = fdc.fdc_cmdstat[4];
		diskLoc->ftDLsector = fdc.fdc_cmdstat[5];
		diskLoc->ftDLsecsize = fdc.fdc_cmdstat[6];

		ftMSetLoc (diskLoc);
		return 0;
	}
}

/************************************************************************
 * ftSkipBack()
 *
 * Skip segments in logical backward direction.
 * Return 0 on success, -1 on failure.
 ***********************************************************************/

#if	__USE_PROTO__
int ftSkipBack(int segCount)
#else
int
ftSkipBack(segCount)
int segCount;
#endif
{
	int result;

	/* If possible, use short (non-extended) form of skip command. */
	if ((segCount & ~0xFF) == 0) {
		if (ftCmd(QIC_CMD_SKPB))
			return -1;
		ftCmdArg (segCount & 0xF);
		ftCmdArg (segCount >> 4);
	} else {
		if (ftCmd(QIC_CMD_SKPBX))
			return -1;
		ftCmdArg (segCount & 0xF);
		ftCmdArg ((segCount >> 4) & 0xF);
		ftCmdArg ((segCount >> 8) & 0xF);
	}

	ftMSetMtn (FT_WINDING);
	ftWait(FT_MOTION_TICKS);
	result = ftReadyWait(FT_SKIP_SECS);
	ftMSetMtn (FT_STOPPED);
	return result;
}

/************************************************************************
 * ftSkipFwd()
 *
 * Skip segments in logical forward direction.
 * Return 0 on success, -1 on failure.
 ***********************************************************************/

#if	__USE_PROTO__
int ftSkipFwd(int segCount)
#else
int
ftSkipFwd(segCount)
int segCount;
#endif
{
	int result;

	/* If possible, use short (non-extended) form of skip command. */
	if ((segCount & ~0xFF) == 0) {
		if (ftCmd(QIC_CMD_SKPF))
			return -1;
		ftCmdArg (segCount & 0xF);
		ftCmdArg (segCount >> 4);
	} else {
		if (ftCmd(QIC_CMD_SKPFX))
			return -1;
		ftCmdArg (segCount & 0xF);
		ftCmdArg ((segCount >> 4) & 0xF);
		ftCmdArg ((segCount >> 8) & 0xF);
	}

	ftMSetMtn (FT_WINDING);
	ftWait(FT_MOTION_TICKS);
	result = ftReadyWait(FT_SKIP_SECS);
	ftMSetMtn (FT_STOPPED);
	return result;
}

/************************************************************************
 * ftFwdTape
 *
 * Send QIC forward tape command.
 * Return 0 if success, -1 if ACK missed or signal interrupted.
 ***********************************************************************/
#if	__USE_PROTO__
int ftFwdTape(void)
#else
int
ftFwdTape()
#endif
{
	int retval;

	ftMSetMtn (FT_MOVING);
	retval = ftCmd(QIC_CMD_FWD);
	return retval;
}

#if 0
/************************************************************************
 * ftMicroPauseTape
 *
 * Send QIC micro step pause tape command, then sleep til drive ready.
 * Return 0 if success, -1 if timed out before sensing drive ready.
 ***********************************************************************/
#if	__USE_PROTO__
static int ftMicroPauseTape(void)
#else
static int
ftMicroPauseTape()
#endif
{
	/* Micro Step Pause, wait for ready. */
	ftCmd(QIC_CMD_MPAUS);
	ftWait(FT_MOTION_TICKS);
	ftMSetMtn (FT_STOPPED);
	return ftReadyWait(FT_PAUSE_SECS);
}
#endif

/************************************************************************
 * ftPauseTape
 *
 * Send QIC pause tape command, then sleep til drive ready.
 * Return 0 if success, -1 if timed out before sensing drive ready.
 ***********************************************************************/
#if	__USE_PROTO__
int ftPauseTape(void)
#else
int
ftPauseTape()
#endif
{
	/* Pause, wait for ready. */
	ftCmd(QIC_CMD_PAUS);
	ftWait(FT_MOTION_TICKS);
	ftMSetMtn (FT_STOPPED);
	return ftReadyWait(FT_PAUSE_SECS);
}

/************************************************************************
 * ftRewindTape
 *
 * Send QIC rewind (BOT) tape command, then sleep til drive ready.
 * Return 0 if success, -1 if timed out before sensing drive ready.
 ***********************************************************************/
#if	__USE_PROTO__
int ftRewindTape(void)
#else
int
ftRewindTape()
#endif
{
	/* Rewind, wait for ready. */
	ftCmd(QIC_CMD_BOT);
	ftWait(FT_MOTION_TICKS);
	ftMSetMtn (FT_STOPPED);
	return ftReadyWait(FT_WIND_SECS);
}

/************************************************************************
 * ftSeekTape
 *
 * Send QIC seek tape command, then sleep til drive ready.
 * Return 0 if success, -1 if timed out before sensing drive ready.
 ***********************************************************************/
#if	__USE_PROTO__
int ftSeekTape(int track)
#else
int
ftSeekTape(track)
int track;
#endif
{
	ftWait(3);
	ftCmd(QIC_CMD_SEEK);
	ftCmdArg (track);
	ftWait(2);
	return ftReadyWait(FT_SEEK_SECS);
}

/************************************************************************
 * ftStopTape
 *
 * Send QIC stop tape command, then sleep til drive ready.
 * Return 0 if success, -1 if timed out before sensing drive ready.
 ***********************************************************************/
#if	__USE_PROTO__
int ftStopTape(void)
#else
int
ftStopTape()
#endif
{
	/* Stop, wait for ready. */
	ftCmd(QIC_CMD_STOP);
	ftWait(FT_MOTION_TICKS);
	ftMSetMtn (FT_STOPPED);
	return ftReadyWait(FT_STOP_SECS);
}

/************************************************************************
 * ftStsWthErr
 *
 * Do Report Drive Status.
 * If error detected, do Report Error Code.
 * Uses sleep, so may not be used in timeout/interrupt/load/unload, etc.
 * Return status if ok, -1 on protocol failure.
 ***********************************************************************/

#if	__USE_PROTO__
int ftStsWthErr(void)
#else
int
ftStsWthErr()
#endif
{
	int status;

	if (ftCmd(QIC_CMD_STS)) {
#if FT_TRACE
		cmn_err (CE_WARN, "!ftStsWthErr : ftCmd failed");
#endif
		return -1;
	}

	status = ft.ft_report;
	ft.ft_qicstat = ft.ft_report;

#if FT_TRACE
	ftDbPrintStat(status);
#endif

	/* If error status detected, get the error code and print in English. */

	if ((status & QIC_STAT_ERR) && (status & QIC_STAT_RDY)) {
		if (ftCmd(QIC_CMD_ECD)) {
#if FT_TRACE
			cmn_err (CE_WARN,
			  "ftStsWthErr stat %x, can't get err code", status);
#endif
			return status;
		}
#if FT_TRACE
		ft.ft_errstat = ft.ft_report;
		ftDbPrintErr (ft.ft_report);
#endif
	}
	return status;
}

/************************************************************************
 * ftWindTape
 *
 * Send QIC wind (EOT) tape command, then sleep til drive ready.
 * Return 0 if success, -1 if timed out before sensing drive ready.
 ***********************************************************************/
#if	__USE_PROTO__
int ftWindTape(void)
#else
int
ftWindTape()
#endif
{
	/* Wind, wait for ready. */
	ftCmd(QIC_CMD_EOT);
	ftWait(FT_MOTION_TICKS);
	ftMSetMtn (FT_STOPPED);
	return ftReadyWait(FT_WIND_SECS);
}

/*********************
**********************
**
** FDC level i/o.
**
**********************
**********************/

/************************************************************************
 * ftLogIOerr
 *
 * Given error table pointer, segment index, and block index,
 * attempt to log an I/O error.
 *
 * Return 0 on success, 1 on failure.
 ***********************************************************************/
#if __USE_PROTO__
static int ftLogIOerr (struct FtSegErr * ftSegErrp,
  int segIndex, int errBlock)
#else
static int
ftLogIOerr (ftSegErrp, segIndex, errBlock)
struct FtSegErr * ftSegErrp;
int segIndex;
int errBlock;
#endif
{
	int retval = 1;

#if FT_TRACE
cmn_err (CE_NOTE, "!ftLogIOerr (%x, %d, %d)",
  (int)ftSegErrp, segIndex, errBlock);
#endif

	if (ftSegErrp) {

		unsigned char * errCountp;

		errCountp = & (ftSegErrp [segIndex].se_errCount);

#if FT_TRACE
cmn_err (CE_NOTE, "!ftLogIOerr : old error count is %d", (int)(* errCountp));
#endif

		/*
		 * segIndex is segment number we are on, relative
		 * to the first segment requested on this call
		 * to ftReadSegments () - it is used to access
		 * the i/o error table for each segment.
		 */

		if (* errCountp < FT_NUM_ERR_BLK) {

			/* Log the error. */

			ftSegErrp [segIndex].se_errBlock [* errCountp] =
			  errBlock;

#if FT_TRACE
cmn_err (CE_NOTE, "!ftLogIOerr : error # %d at block %d",
  (int)(* errCountp), errBlock);
#endif

			(* errCountp) ++;

			/* Return success. */

			retval = 0;

		} else {

			/* Too many errors in this segment. Fail. */

#if FT_TRACE
cmn_err (CE_NOTE, "!ftLogIOerr : too many errors for segment");
#endif

		}
	} else {

		/* No provision for error logging.  Fail. */

#if FT_TRACE
cmn_err (CE_NOTE, "!ftLogIOerr : error table not in use");
#endif

	}

	return retval;
}

/*********************
**********************
**
** Segment level i/o routines.
**
**********************
**********************/

/************************************************************************
 * ftReadSegments
 *
 * Arguments:
 *   Starting segment number.
 *   Number of segments.
 *   Virtual address of a virtually contiguous buffer region.
 *   Switch to tell us whether to check Bad Block Table or not.
 *   Pointer to error struct table, or null.
 *   Pointer to block count table, or null.
 *
 * This routine calls ftXferFDC () and provides these upper-level
 * functions for the latter:
 *
 *   Check for requests for 0 blocks, and do not pass these on to
 *   ftXferFDC ().
 *
 *   Check for requests to read past end of tape, and truncate them
 *   before calling ftXferFDC ().
 *
 *   If a request will go past the end of a tape track, split it 
 *   into multiple calls to ftXferFDC ().
 *
 *   If any request to ftXferFDC () does not finish, repeat the
 *   attempt.  As long as at least one new block is read with each
 *   call, keep going.  If a single block cannot be read after several
 *   attempts, log the error into the i/o error table.  If no error
 *   table is provided or the error limit for a segment is exceeded, fail.
 *
 *   After tape access is completed, pack all good blocks toward the
 *   beginning of each segment (i.e., skip "holes" left due to observance
 *   of the bad block map) then apply ecc.  If ecc fails, abort the read.
 *
 *   Return number of segments read.
 ***********************************************************************/
#if __USE_PROTO__
int ftReadSegments (int logSegment, int numSegments, caddr_t readBuf,
  int bbtSw, struct FtSegErr * ftSegErrp, unsigned char * ftnbp)
#else
int
ftReadSegments (logSegment, numSegments, readBuf, bbtSw, ftSegErrp, ftnbp)
int logSegment;
int numSegments;
caddr_t readBuf;
int bbtSw;
struct FtSegErr * ftSegErrp;
unsigned char * ftnbp;
#endif
{

/*
 * WARNING - don't alter the value of logSegment - a couple places below
 * count on using the value passed into this function!!!
 */

	int	totBlksRead;
	int	totSegsRead;
	int	excessSegments;
	int	retries;
	int	logBlock;
	int	numBlocks;
	int	segIndex;
	struct ftDiskLocn dl;
	caddr_t	readBufOrig = readBuf;  /* Save buffer start addr for later. */

#if FT_TRACE
cmn_err (CE_NOTE,
  "!ftReadSegments (logSeg=%d, numSeg=%d, rbuf=%x, bbtSw=%d, erp=%x, nbp=%x)",
  logSegment, numSegments, (int) readBuf, bbtSw, (int)ftSegErrp, (int)ftnbp);
#endif

	totBlksRead = 0;
	totSegsRead = 0;

	/* Return now if no work to be done. */

	if (numSegments <= 0)
		goto endReadSegs;

	/* Compute how many, if any, segments were requested past end of tape. */

	excessSegments = logSegment + numSegments
	  - (ftCBlksPerVol () / FT_BLK_PER_SEG);

	/* Nothing to do if entire request is beyond EOT. */

	if (excessSegments >= numSegments)
		goto endReadSegs;

	/* Keep any request from going off the end of the tape. */

	if (excessSegments > 0) {
#if FT_TRACE
		cmn_err (CE_NOTE, "!ftReadSegments : %d blocks past EOT",
		  excessSegments);
#endif
		numSegments -= excessSegments;
	}

	/* Convert request for segments into request for blocks. */

	logBlock = logSegment * FT_BLK_PER_SEG;
	numBlocks = numSegments * FT_BLK_PER_SEG;

	/* Clear out IO error stats.  This is overkill, but helps debug. */

	if (ftSegErrp) {
		for (segIndex = 0; segIndex < numSegments; segIndex++) {
			int i;

			ftSegErrp [segIndex].se_errCount = 0;
			for (i = 0; i < FT_NUM_ERR_BLK; i++)
				ftSegErrp [segIndex].se_errBlock [i] = 0;
		}
	}

	/* Driver loop for low-level read. */

	retries = 0;
	while (numBlocks > 0) {
		int blocksRequested = numBlocks;
		int blocksRead = 0;
		int bytesRead = 0;
		struct ftTapeLocn tl;
		int excessBlocks;

		/* Compute FDC pseudocoordinates for the starting sector. */

		if (ftCLBtoDL (logBlock, & dl))
			break;

#if FT_TRACE
cmn_err (CE_NOTE,
  "!ftReadSegments : Start from head %d, cyl %d, sector %d",
  dl.ftDLhead, dl.ftDLcylinder, dl.ftDLsector);
#endif

		/* Compute tape coordinates for the starting sector. */

		if (ftCSectoTL (logBlock, & tl))
			break;

#if FT_TRACE
cmn_err (CE_NOTE,
  "!ftReadSegments : Start from track %d, segment %d, block %d",
  tl.ftTLtrack, tl.ftTLsegment, tl.ftTLblock);
#endif

		/*
		 * A single call to ftXferFDC should not try to run
		 * past the end of a tape track.
		 * Compute the number of blocks remaining on the present track.
		 * Limit number of blocks requested to at most this number.
		 */
		excessBlocks = numBlocks - ftCBlocksLeft (&tl);

		if (excessBlocks > 0)
			blocksRequested -= excessBlocks;

		bytesRead = ftXferFDC (logBlock, blocksRequested,
		  readBuf, &dl, bbtSw, FT_READ, retries);

		/* Don't accept partial blocks read. */

		bytesRead -= bytesRead % FT_BLK_SIZE;
		blocksRead = bytesRead / FT_BLK_SIZE;

		/*
		 * If at least one block made it in
		 *   Reset the retry count.
		 *   Update i/o parameters for next call to ftXferFDC ().
		 * Else
		 *   Increment retry count.
		 *   If count too high, give up on the read.
		 *
		 * In order to avoid wasting time on read-ahead,
		 * if at least one complete segment has been read at the
		 * time and i/o error occurs, stop accessing the FDC and
		 * return data to the user.
		 */

		if (blocksRead > 0) {

			retries = 0;
			numBlocks -= blocksRead;
			logBlock += blocksRead;
			readBuf += bytesRead;
			totBlksRead += blocksRead;
			totSegsRead = totBlksRead / FT_BLK_PER_SEG;

			if (blocksRead != blocksRequested) {

				/* Some blocks were not read.  I/O error. */

				if (totSegsRead > 0)
					break;
				retries++;
			}

		} else {

			/* No blocks were read.  I/O error. */

			if (totSegsRead > 0)
				break;
			retries++;

			if (retries >= FT_READ_BLK_RETRIES) {
				if (ftLogIOerr (ftSegErrp, totSegsRead,
				  logBlock % FT_BLK_PER_SEG)) {

					/* Log Error failed.  Fail. */
					break;

				} else {

					/* Move on to next block. */

					retries = 0;
					numBlocks --;
					logBlock ++;
					readBuf += FT_BLK_SIZE;
					totBlksRead ++;
					totSegsRead =
					  totBlksRead / FT_BLK_PER_SEG;
				}
			}
		}
	}

	/*
	 * For each segment
	 *   Set good block count to 32.
	 *   Get bad block map, if any.
	 *   If bad block map is nonempty
	 *     Set gap to 0 blocks.
	 *     For each bit in bad block map
	 *       If bit is 1 (bad block)
	 *	   Subtract 1 from good block count.
	 *         If there is an error table
	 *           For each block logged in error table
	 *             If error block number > current block number
	 *               Subtract 1 from error block number.
	 *             Else if error block number == current block number
	 *               Report error - this should not happen
	 *       Else bit is 0 (good block)
	 *         If gap > 0
	 *           Move current block "gap" blocks toward start of segment
	 *  If there is an error table
	 *    Fetch error count and blocks
	 *  Else
	 *    Use null values
	 *  If ecc fails
	 *    Display error and end the call.
	 */
#if FT_TRACE
cmn_err (CE_NOTE, "!ftReadSegments : %d buffers to process.", totSegsRead);
#endif

	for (segIndex = 0; segIndex < totSegsRead; segIndex++) {
		int	goodBlocks;
		unsigned int	badBitMap;
		int	gap;
		int	bitIndex;
		int	errBlkIndex;
		int	eccResult;
		struct FtSegErr * ftsep;
		caddr_t x;

		if (ftSegErrp)
			ftsep = ftSegErrp + segIndex;
		else
			ftsep = 0;

		goodBlocks = FT_BLK_PER_SEG;
		if (bbtSw == FT_USE_BBT) {

			if (ftBBT (logSegment + segIndex, & badBitMap))
				badBitMap = 0;

			if (badBitMap) {
				gap = 0;
				for (bitIndex = 0; bitIndex < FT_BLK_PER_SEG;
				  bitIndex++) {
					if (badBitMap & (1 << bitIndex)) {
#if FT_TRACE
cmn_err (CE_NOTE, "!ftReadSegments : "
  "Buffer %d, segment %d, block %d is bad in BBT",
  segIndex, logSegment + segIndex, bitIndex);
#endif
						goodBlocks --;
						gap++;
						if (ftsep) {
for (errBlkIndex = 0; errBlkIndex < ftsep->se_errCount; errBlkIndex++) {

	unsigned char  * errBlockp = ftsep->se_errBlock + errBlkIndex;

	if (* errBlockp > bitIndex) {

#if FT_TRACE
cmn_err (CE_CONT, "!I/O error at block %d becomes error at block %d.\n",
  * errBlockp, (* errBlockp) - 1);
#endif

		(* errBlockp) --;

	} else if (* errBlockp == bitIndex) {

		/*
		 * This is an error because we should not have tried to
		 * read a block marked bad in the BBT, so there should be
		 * no I/O error at that block.
		 */

#if FT_TRACE
		cmn_err (CE_WARN,
		  "FT : Squeeze error, block %d, segment %d, segIndex %d",
		  bitIndex, logSegment + segIndex, segIndex);
#endif
	}
}
						}
					} else { /* Good block. */

						if (gap) {

#if FT_TRACE
cmn_err (CE_CONT, "!Moving block %d to position %d to allow for BBT\n",
  bitIndex, bitIndex - gap);
#endif

/* x is the address of the current block in the buffer pool. */

x = readBufOrig + (segIndex * FT_SEG_SIZE) + (bitIndex * FT_BLK_SIZE);

/* Move the current block closer to the start of the segment. */
memcpy (x - gap * FT_BLK_SIZE, x, FT_BLK_SIZE);
						}
					}
				}
			}

		}

		/* x is address of current segment in buffer pool. */

		x = readBufOrig + (segIndex * FT_SEG_SIZE);

		if (ftsep) {
			eccResult = decodeEcc ((unsigned char *)x, goodBlocks,
			  ftsep->se_errCount, ftsep->se_errBlock [0],
			  ftsep->se_errBlock [1], ftsep->se_errBlock [2]);
		} else {
			eccResult = decodeEcc ((unsigned char *)x, goodBlocks,
			  0, 0, 0, 0);
		}

#if FT_TRACE
		if (eccResult == 0) {

			/* Only display warning if no data was read. */
			if (totSegsRead == 0)
				cmn_err (CE_WARN,
				  "FT : ECC failure at segment %d",
				  logSegment + segIndex);
			else
				cmn_err (CE_NOTE,
				  "!FT : ECC failure at segment %d",
				  logSegment + segIndex);
		}
#endif

		/* Update count of good blocks in the buffer pool. */
		if (ftnbp)
			ftnbp [segIndex] = goodBlocks - FT_NUM_ECC_BLKS;

	} /* End for loop over segments read. */


endReadSegs:

#if FT_TRACE
	cmn_err (CE_NOTE, "!ftReadSegments : %d segments read", totSegsRead);
#endif
	return totSegsRead;
}

/************************************************************************
 * ftWriteSegments
 *
 * Arguments:
 *   Starting segment number.
 *   Number of segments.
 *   Virtual address of a virtually contiguous buffer region.
 *   Switch to tell us whether to check Bad Block Table or not.
 *   Pointer to byte count table, or null.
 *
 * This routine calls ftXferFDC () and provides these upper-level
 * functions for the latter:
 *
 *   Check for requests for 0 blocks, and do not pass these on to
 *   ftXferFDC ().
 *
 *   Check for requests to read past end of tape, and truncate them
 *   before calling ftXferFDC ().
 *
 *   If a request will go past the end of a tape track, split it 
 *   into multiple calls to ftXferFDC ().
 *
 *   If any request to ftXferFDC () does not finish, repeat the
 *   attempt.  As long as at least one new block is read with each
 *   call, keep going.  If a single block cannot be read after several
 *   attempts, log the error into the i/o error table.  If no error
 *   table is provided or the error limit for a segment is exceeded, fail.
 *
 *   After tape access is completed, pack all good blocks toward the
 *   beginning of each segment (i.e., skip "holes" left due to observance
 *   of the bad block map) then apply ecc.  If ecc fails, abort the read.
 *
 *   Return number of segments read.

For all segment buffers
  If data count does not match the number of blocks that may be written to this
  segment according to the Bad Block Table
    Display warning
      Segment x, buffer has %d bytes, bbt allows %d, writing %d
  Generate ECC blocks.
  Unpack data blocks to allow for Bad Block Table.
End for all segment buffers.

Compute number of segments to pass to FDC in a single request, the smaller of:
  Number of segments remaining on current track
  Number of segments stored in buffer pool.

Write blocks to FDC, using Bad Block Table.
  
 ***********************************************************************/
#if __USE_PROTO__
int ftWriteSegments (int logSegment, int numSegments, caddr_t writeBuf,
  int bbtSw, unsigned int * ftnbyp)
#else
int
ftWriteSegments (logSegment, numSegments, writeBuf, bbtSw, ftnbyp)
int logSegment;
int numSegments;
caddr_t writeBuf;
int bbtSw;
unsigned int * ftnbyp;
#endif
{
	int		totSegsWritten;
	int		totBlksWritten;
	int		excessSegments;
	int		segIndex;
	int		numBlocks;
	int		logBlock;
	int		retries;

	struct ftDiskLocn dl;

	totSegsWritten = 0;
	totBlksWritten = 0;

#if FT_TRACE
cmn_err (CE_NOTE,
  "!ftWriteSegments (%d, %d, %x, %d, %x)",
  logSegment, numSegments, (int) writeBuf, bbtSw, (int)ftnbyp);
#endif

	/* Return now if no work to be done. */

	if (numSegments <= 0)
		goto endWriteSegs;

	/* Compute how many, if any, segments were requested past end of tape. */

	excessSegments = logSegment + numSegments
	  - (ftCBlksPerVol () / FT_BLK_PER_SEG);

	/* Nothing to do if entire request is beyond EOT. */

	if (excessSegments >= numSegments)
		goto endWriteSegs;

	/* Keep any request from going off the end of the tape. */

	if (excessSegments > 0) {
#if FT_TRACE
		cmn_err (CE_NOTE, "!ftWriteSegments : %d blocks past EOT",
		  excessSegments);
#endif
		numSegments -= excessSegments;
	}

	/* Loop over all segment buffers that have data to go out. */

	for (segIndex = 0; segIndex < numSegments; segIndex++) {

		int	goodBlocks;
		int	writeBytes;
		unsigned int	bitMap;
		unsigned char	nb;
		int	gap;

		/* Calculate how many bytes we may write for the segment. */

		gap = 0;
		goodBlocks = FT_DATA_BLK_PER_SEG;

		if (bbtSw == FT_USE_BBT) {
			if (ftBBT (logSegment + segIndex, & bitMap))
				bitMap = 0;
		} else
			bitMap = 0;

#if FT_TRACE
cmn_err (CE_NOTE, "!ftWriteSegments : segment %d, buffer %d, bitmap is %x",
  logSegment + segIndex, segIndex, bitMap);
#endif

		if (bbtSw == FT_USE_BBT) {

			unsigned int tempMap;

			tempMap = bitMap;

			while (tempMap != 0) {
				goodBlocks--;
				gap++;
				if (goodBlocks <= 0)
					break;
				/* Clear a 1 bit in the bitmap. */
				tempMap &= (tempMap - 1);
			}
		}

#if FT_TRACE
cmn_err (CE_NOTE, "!segment has %d good blocks", goodBlocks);
#endif

		writeBytes = goodBlocks * FT_BLK_SIZE;

#if FT_TRACE
		if (writeBytes != ftnbyp [segIndex]) {
			cmn_err (CE_NOTE,
"!ftWriteSegments : segment %d, buffer has %d bytes, BBT allows %d",
			  logSegment + segIndex, ftnbyp [segIndex], writeBytes);
		}
#endif

		if (writeBytes <= 0)
			continue;

		/* Generate 3 ECC blocks per segment buffer. */

		nb = goodBlocks + FT_NUM_ECC_BLKS;
		encodeEcc ((unsigned char *)
		  (writeBuf + (segIndex * FT_SEG_SIZE)), nb);

#if FT_TRACE
cmn_err (CE_NOTE, "!encodeEcc (%x, %d)",
  (int)(writeBuf + (segIndex * FT_SEG_SIZE)), nb);
#endif

		/* Unpack blocks within segment according to BBT. */

		if (bitMap != 0) {

			int	srcBlk;
			int	destBlk;
			caddr_t	src;
			caddr_t	dest;

			srcBlk = goodBlocks + FT_NUM_ECC_BLKS - 1;
			for (destBlk = FT_BLK_PER_SEG - 1;
			  destBlk >= 0 && srcBlk != destBlk; destBlk--) {

				if (bitMap & (1 << destBlk)) {

#if FT_TRACE
cmn_err (CE_NOTE, "!Leaving gap at block %d", destBlk);
#endif

				} else {

src = writeBuf + (segIndex * FT_SEG_SIZE) + (srcBlk * FT_BLK_SIZE);
dest = writeBuf + (segIndex * FT_SEG_SIZE) + (destBlk * FT_BLK_SIZE);

					/* Move a block closer to the end of
					 * the segment. */

					memcpy (dest, src, FT_BLK_SIZE);

#if FT_TRACE
cmn_err (CE_NOTE, "!Moving block %d to position %d", srcBlk, destBlk);
#endif
					srcBlk --;
				}
			}
		}

	} /* End for loop over all segments to be written. */

	/* Convert request for segments into request for blocks. */

	logBlock = logSegment * FT_BLK_PER_SEG;
	numBlocks = numSegments * FT_BLK_PER_SEG;

	/* Driver loop for low-level write. */

	retries = 0;
	while (numBlocks > 0) {

		int blocksRequested = numBlocks;
		int blocksWritten = 0;
		int bytesWritten = 0;
		struct ftTapeLocn tl;
		int excessBlocks;

		/* Compute FDC pseudocoordinates for the starting sector. */

		if (ftCLBtoDL (logBlock, & dl))
			break;

#if FT_TRACE
cmn_err (CE_NOTE,
  "!ftWriteSegments : Start from head %d, cyl %d, sector %d",
  dl.ftDLhead, dl.ftDLcylinder, dl.ftDLsector);
#endif

		/* Compute tape coordinates for the starting sector. */

		if (ftCSectoTL (logBlock, & tl))
			break;

#if FT_TRACE
cmn_err (CE_NOTE,
  "!ftWriteSegments : Start from track %d, segment %d, block %d",
  tl.ftTLtrack, tl.ftTLsegment, tl.ftTLblock);
#endif

		/*
		 * A single call to ftXferFDC should not try to run
		 * past the end of a tape track.
		 * Compute the number of blocks remaining on the present track.
		 * Limit number of blocks requested to at most this number.
		 */
		excessBlocks = numBlocks - ftCBlocksLeft (&tl);

		if (excessBlocks > 0)
			blocksRequested -= excessBlocks;

		bytesWritten = ftXferFDC (logBlock, blocksRequested,
		  writeBuf, &dl, bbtSw, FT_WRITE, retries);

		/* Don't accept partial blocks written. */

		bytesWritten -= bytesWritten % FT_BLK_SIZE;
		blocksWritten = bytesWritten / FT_BLK_SIZE;

		/*
		 * If at least one block made it in
		 *   Reset the retry count.
		 *   Update i/o parameters for next call to ftXferFDC ().
		 * Else
		 *   Increment retry count.
		 *   If count too high, give up on the write.
		 */

		if (blocksWritten > 0) {
			retries = 0;
			numBlocks -= blocksWritten;
			logBlock += blocksWritten;
			writeBuf += bytesWritten;
			totBlksWritten += blocksWritten;
			totSegsWritten = totBlksWritten / FT_BLK_PER_SEG;

			if (blocksWritten != blocksRequested)
				retries++;

		} else {
			retries++;
			if (retries >= FT_WRITE_BLK_RETRIES) {

#if FT_TRACE
cmn_err (CE_WARN, "FT : ftWriteSegments : block %d, too many retries",
  logBlock);
#endif

				break;
			}
		}
	}

endWriteSegs:

#if FT_TRACE
	cmn_err (CE_NOTE,
	  "!ftWriteSegments : %d Segments Written", totSegsWritten);
#endif

	return totSegsWritten;
}


/************************************************************************
 * ftGetHeaderSeg
 *
 * Try to read the header info from the first valid segment into the
 * buffer area reserved for that purpose.  This loads the bad block
 * map for us.  Don't do anything if there is already a valid header.
 *
 * Return 0 if success, -1 on failure.
 ***********************************************************************/
#if	__USE_PROTO__
static int ftGetHeaderSeg (void)
#else
static int
ftGetHeaderSeg ()
#endif
{
	int		trySeg;

	if (ft.ft_hdrOk)
		return 0;

	for (trySeg = 0; trySeg < FT_GET_HEADER_TRIES; trySeg++) {

		if (ftReadSegments (trySeg, 1, (caddr_t)ftHeaderSeg,
		  FT_DONT_USE_BBT, 0, 0) == 1) {

			/* Read segment succeeded.  Check for valid signature. */
			if (*(int *)ftHeaderSeg == FT_SIGNATURE) {
				ft.ft_hdrOk = 1;
				break;
			}
#if FT_TRACE
			cmn_err (CE_WARN, "FT : signature is %x, want %x",
			  *(int *)ftHeaderSeg, FT_SIGNATURE);
		} else {
			cmn_err (CE_WARN, "FT : read segment %d failed", trySeg);
#endif
		}
	}
	if (ft.ft_hdrOk) {

		unsigned int spt;
		unsigned int tracks;
		unsigned int heads;
		unsigned int cyls;
		unsigned int blkPerPseudoTrack;

		/*
		 * Sanity test.  *All* valid tape formats have 128 sectors
		 * per floppy pseudotrack.
		 */

		blkPerPseudoTrack = ftHeaderSeg [29];
		if (blkPerPseudoTrack != FT_SEC_PER_PTRK) {
#if FT_TRACE
			cmn_err (CE_WARN,
			  "FT : unlikely max sector # %d, want %d",
			  blkPerPseudoTrack, FT_SEC_PER_PTRK);
#endif
			return -1;
		}

		spt = *(short *)(ftHeaderSeg + 24);
		tracks = ftHeaderSeg [26];
		heads = ftHeaderSeg [27] + 1;
		cyls = ftHeaderSeg [28] + 1;

		/* Valid signature seen.  Set tape parameters. */

		ftCvtSetFtParms (spt, tracks, heads, cyls);

#if FT_TRACE
cmn_err (CE_NOTE,
  "!ftGetHeaderSeg : %d segs per track, %d tracks, %d heads, %d cyls",
  spt, tracks, heads, cyls);
#endif

		/* Make note of data range on tape. */

		ft.ft_firstDataSeg = *(unsigned short *)(ftHeaderSeg + FT_FIRST_DATA_SEG);
		ft.ft_lastDataSeg = *(unsigned short *)(ftHeaderSeg + FT_LAST_DATA_SEG);

#if FT_TRACE
cmn_err (CE_NOTE, "!ftGetHeaderSeg : Data segment range: %d to %d",
  ft.ft_firstDataSeg, ft.ft_lastDataSeg);
#endif

		return 0;
	} else
		return -1;
}

/*********************
**********************
**
** CON struct routines.
**
**********************
**********************/

/************************************************************************
 *	ftblock
 *
 * Tape is not a block device, but we need a block entry point since
 * the same driver controls diskette access.
 ***********************************************************************/
#if	__USE_PROTO__
static void ftblock (buf_t * bp)
#else
static void
ftblock (bp)
buf_t * bp;
#endif
{
	set_user_error (EIO);
	bp->b_flag |= BFERR;
	bdone (bp);
}

/************************************************************************
 * ftclose
 *
 ***********************************************************************/
#if	__USE_PROTO__
static void ftclose(o_dev_t dev,
  int __NOTUSED(_mode), int __NOTUSED(_flags), __cred_t * __NOTUSED(_credp))
#else
static void
ftclose (dev)
o_dev_t dev;
#endif
{
#if FT_TRACE
	cmn_err (CE_NOTE, "!ftclose");
#endif

	/* Write any data still in buffers. */

	if (ft.ft_write && ft.ft_numSegs > 0) {

		int	segsWritten;

		segsWritten = ftWriteSegments (ft.ft_segment, ft.ft_numSegs,
		  (caddr_t)ftBigBuf, FT_USE_BBT, ftNumBytes);

		if (segsWritten != ft.ft_numSegs) {
#if FT_TRACE
			cmn_err (CE_WARN,
"FT : Only wrote %d of %d segments, starting at segment %d",
			  segsWritten, ft.ft_numSegs, ft.ft_segment);
#endif
		}
		ft.ft_numSegs = 0;
	}

	if (ft.ft_norwdcl) {

		/*
		 * No rewind on close.
		 * Save number of next unused segment for next open.
		 */

		ft.ft_segbase += ft.ft_lastseek;

#if 0
		/* Align final offset to 1 KByte block boundary. */
		{
			int phySeg;
			int byteOffset;
			int pad;

			/* Leave off on a block boundary. */
			pad = ft.ft_segbase % FT_BLK_SIZE;
			if (pad > 0)
				ft.ft_segbase += (FT_BLK_SIZE - pad);
		}
#endif
		ftStopTape ();

	} else {
		/* Rewind on close. */
		ftRewindTape();
		ft.ft_segbase = 0;
	}

#if FT_TRACE
cmn_err (CE_NOTE, "!ftclose : segbase = %ld", ft.ft_segbase);
#endif

	switch (ft.ft_softsel) {
	case 0:
		break;
	case 1:
		/* A/C/M/S soft select disable. */
		ftCmd(QIC_CMD_SS_OFF);
		break;
	case 2:
		/* CMS soft select disable. */
		ftCmd(47);
		break;
	}

	setFtIntr(0);
	ft.ft_open = 0;
}

/*************************************************************************
 *	ftioctl
 *	Handle tape drive & controller commands like erase, rewind,
 *	retension, read filemark, and write filemark
 ************************************************************************/

#if	__USE_PROTO__
static void ftioctl(o_dev_t dev, int com, __VOID__ * vec,
  int __NOTUSED (_mode), __cred_t * __NOTUSED( _credp), int *__NOTUSED(_rvalp))
#else
static void
ftioctl (dev, com, vec)
o_dev_t dev;
int com;
__VOID__ * vec;
#endif
{
	int			result;

	switch (com) {

	case FT_GET_HEADER:
		{
			struct FtHdrBuf	fthb;
			unsigned int	bytesToCopy;

			/* Get header buffer descriptor from user. */

			if (copyin((caddr_t) vec, (caddr_t)(& fthb),
			  sizeof(fthb)))
				set_user_error (EIO);

			/* If no valid tape header yet, get one now. */

			else if (ftGetHeaderSeg ())
				set_user_error (EIO);

			/* Copy up to 29 Kbytes of tape header to user. */

			else {
				bytesToCopy = FT_BLK_SIZE
				  * (FT_BLK_PER_SEG - FT_NUM_ECC_BLKS);
				if (bytesToCopy > fthb.bufLen)
					bytesToCopy = fthb.bufLen;
				if (copyout((caddr_t) ftHeaderSeg,
				  (caddr_t)(fthb.buffer), (size_t)bytesToCopy))
					set_user_error (EIO);
			}
		}
		break;

	case FT_SET_HEADER:
		{
			struct FtHdrBuf	fthb;
			unsigned int	bytesToCopy;

			/* Get header buffer descriptor from user. */

			if (copyin((caddr_t) vec, (caddr_t)(& fthb),
			  sizeof(fthb)))
				set_user_error (EIO);

			/* If no valid tape header yet, get one now. */

			else if (ftGetHeaderSeg ())
				set_user_error (EIO);

			/* Copy up to 29 Kbytes of tape header from user. */

			else {
				bytesToCopy =
				  FT_DATA_BLK_PER_SEG * FT_BLK_SIZE;

				if (bytesToCopy > fthb.bufLen)
					bytesToCopy = fthb.bufLen;

				if (copyin((caddr_t)(fthb.buffer),
				  (caddr_t) ftHeaderSeg,
				  (size_t)bytesToCopy)) {
					set_user_error (EIO);
					return;
				}

				encodeEcc (ftHeaderSeg, FT_BLK_PER_SEG);

				result = ftWriteSegments (0, 1,
				  (caddr_t)ftHeaderSeg,
				  FT_DONT_USE_BBT, & bytesToCopy);

				if (result != 1) {
#if FT_TRACE
					cmn_err (CE_WARN,
					  "FT : Failed to write to segment 0");
#endif
					return;
				}

				result = ftWriteSegments (0, 1,
				  (caddr_t)ftHeaderSeg,
				  FT_DONT_USE_BBT, & bytesToCopy);

				if (result != 1) {
#if FT_TRACE
					cmn_err (CE_WARN,
					  "FT : Failed to write to segment 1");
#endif
					return;
				}

			}
		}
		break;

	case FT_GET_OFFSET:
	case T_TELL:
		if (copyout((caddr_t) & ft.ft_segbase,
		  (caddr_t)vec, sizeof(int)))
			set_user_error (EIO);
		break;

	case FT_SET_OFFSET:
	case T_SEEK:
		ft.ft_segbase = (int)vec;
		break;

	case T_RDSTAT:
		{
			/* save stack space */
			static struct TapeStat ts;

			/* load up the local status struct */

			/* Type of tape device is FLOPPY TAPE.*/
			ts.tape_stat_type = TAPE_STAT_FT;

			/* QIC-117 Status (8 bits, or -1). */
			ts.tsu.ftstat.qicstat = ftStsWthErr();

			/* QIC-117 Error Status (16 bits, or -1). */
			if (ft.ft_qicstat & QIC_STAT_ERR)
				ts.tsu.ftstat.qicerrstat = ft.ft_errstat;
			else
				ts.tsu.ftstat.qicerrstat = -1;

			/* QIC-117 Drive Configuration (8 bits, or -1). */
			if (ftCmd(QIC_CMD_DRVCN))
				ts.tsu.ftstat.drvcn = -1;
			else
				ts.tsu.ftstat.drvcn = ft.ft_report;

			/* QIC-117 ROM Version (8 bits, or -1). */
			if (ftCmd(QIC_CMD_ROMVN))
				ts.tsu.ftstat.romvn = -1;
			else
				ts.tsu.ftstat.romvn = ft.ft_report;

			/* QIC-117 Vendor ID (16 bits, or -1). */
			if (ftCmd(QIC_CMD_VNDID))
				ts.tsu.ftstat.vndid = -1;
			else
				ts.tsu.ftstat.vndid = ft.ft_report;

			/* QIC-117 Tape Status (8 bits, or -1). */
			if (ftCmd(QIC_CMD_TPSTAT))
				ts.tsu.ftstat.tpstat = -1;
			else
				ts.tsu.ftstat.tpstat = ft.ft_report;

			/* Copy status struct into user space. */

			if (copyout ((caddr_t) &ts, (caddr_t)vec, sizeof(ts)))
				set_user_error (EIO);
		}
		break;

	case T_RWD:
		ftRewindTape();

		/* Reset seek pointer - overrides no-rewind-on-close */
		ft.ft_segbase = 0;
		break;

	case T_RETENSION:
		ftRewindTape();
		ftWait(2 * HZ);
		ftWindTape();
		ftWait(2 * HZ);
		ftRewindTape();
		ftWait(2 * HZ);

		/* Reset seek pointer - overrides no-rewind-on-close */
		ft.ft_segbase = 0;
		break;

	case FT_READ_SEGMENTS:
		{
			int	logBlock;
			int	numBlocks;
			caddr_t	inbuf;

			copyin((caddr_t) vec, (caddr_t)(& logBlock), 4);
			copyin((caddr_t) vec + 4, (caddr_t)(& numBlocks), 4);

			inbuf = (caddr_t)ftBigBuf;

#if FT_TRACE
cmn_err(CE_NOTE, "!ftioctl : Read Segments ioctl : start=%d  num blocks=%d  buffer=%x",
  logBlock, numBlocks, (int) inbuf);
#endif
			
			result = ftReadSegments (logBlock, numBlocks,
			  (caddr_t)inbuf, 0, 0, 0);
		}

		if (result == 0)
			set_user_error (EIO);
		copyout ((caddr_t) (& result), ((caddr_t)(vec)) + 8, 4);
		break;

	case FT_FIND_SEG:
		result = ftMPosTape ((int) vec);
		if (result)
			set_user_error (EIO);
		break;

	case FT_PRINT_STAT:
#if FT_TRACE
		ftDbPrintStat (ftStsWthErr ());
#endif
		break;

	case FT_SKIP_TAPE:
		if ((int)vec >= 0)
			result = ftSkipFwd((int)vec);
		else
			result = ftSkipBack((int)vec);
		if (result)
			set_user_error (EIO);
		break;

	case FT_STOP_TAPE:
		ftStopTape();
		break;

	case FT_SPECIFY:
		if (ftDataRate((int) vec))
			set_user_error (EIO);
		break;

#if FT_TRACE
	case FT_FAKE_BBT_MAP:
		fakeBBTbitmap = (int)vec;
cmn_err (CE_NOTE, "FT : ftioctl : fakeBBTbitmap = %x", (int)vec);
		break;

	case FT_FAKE_BBT_SEG:
		fakeBBTsegment = (int)vec;
cmn_err (CE_NOTE, "FT : ftioctl : fakeBBTsegment = %d", (int)vec);
		break;

	case FT_FAKE_IOERR:
		fakeIOerrblk = (int)vec;
cmn_err (CE_NOTE, "FT : ftioctl : fakeIOerrlbk = %d", (int)vec);
		break;
#endif

	default:
#if FT_TRACE
		cmn_err (CE_WARN, "!ftioctl bad command %d", com);
#endif
		set_user_error (EINVAL);
	}
}

/*************************************************************************
 *	ftload
 ************************************************************************/

#if	__USE_PROTO__
static void ftload (void)
#else
static void
ftload ()
#endif
{
	if (ftPool) {

		ftHeaderSeg = ftPool;
		ftBigBuf = ftHeaderSeg + FT_SEG_SIZE;

#if FT_TRACE
	} else {
		cmn_err (CE_WARN,
		  "FT : Could not allocate %d KBytes of buffer pool.",
		  (FT_NBUF + 1) * FT_BLK_PER_SEG);
#endif
	}

	/* Got buffers ok.  Attach interrupt handler to fdc module. */
	ftIntr = ftIrqHandler;

	/* Fire up Reed-Solomon tables. */
	initEcc ();

	/* We have to do this - it was in the paperwork MWC signed in
	 * order to get source code from CMS and Conner. */
	cmn_err (CE_CONT,
"The COHERENT floppy tape driver was developed using source code copyrighted\n"
"by Colorado Memory Systems, Inc. and by Conner Peripherals, Inc.\n");

}

/************************************************************************
 * ftopen
 *
 ***********************************************************************/
#if	__USE_PROTO__
static void ftopen(o_dev_t dev, int mode,
  int __NOTUSED(_flags), __cred_t * __NOTUSED(_credp))
#else
static void
ftopen (dev, mode)
o_dev_t dev;
int mode;
#endif
{
	int drvStatus;
	int fmt;
	static int driveAutoconfigured = 0;

#if FT_TRACE
	/* Debug setup. */

	fakeBBTbitmap = 0;
	fakeBBTsegment = -1;
	fakeIOerrblk = -1;
#endif

	/* Couldn't allocate buffer area. */
	if (ftBigBuf == NULL) {
		devmsg (dev, "no buffers ");
		set_user_error (EIO);
		return;
	}

	/* Can't append to tape (yet?). */
	if ((mode & IPAPPEND) != 0) {
		devmsg (dev, "can't append ");
		set_user_error (EINVAL);
		return;
	}

	/* Open for read or write, not both. */
	if ((mode & IPR) != 0 && (mode & IPW) != 0) {
		devmsg (dev, "can't open for both read and write ");
		set_user_error (EINVAL);
		return;
	}

	/* Only one open at a time. */
	if (ft.ft_open) {
		devmsg (dev, "only one ftopen at a time ");
		set_user_error (EBUSY);
		return;
	}

	/* Try to determine soft-select v. unit #, etc., if asked to do so. */
	if (FT_AUTO_CONFIG && !driveAutoconfigured) {

#if FT_TRACE
		cmn_err (CE_NOTE, "!ftopen : trying autoconfig.");
#endif

		if (ftAutoConfig ()) {
#if FT_TRACE
			cmn_err (CE_WARN,
			  "FT : Can't initialize QIC-80/QIC-40 tape drive.");
#endif
			set_user_error (EIO);
			return;
		}
		driveAutoconfigured = 1;
	}

	ft.ft_open = 1;

	/* Get fdc unit from device minor number. */
	if (!FT_AUTO_CONFIG) {

		ft.ft_softsel = FT_SOFT_SELECT;

#if FT_TRACE
		cmn_err (CE_NOTE, "!ftopen : soft select mode %d.",
		  ft.ft_softsel);
#endif
		ft.ft_drv = FT_UNIT(dev);
	}

	/* Get rewind-on-close status from minor number. */
	ft.ft_norwdcl = FT_NORWDCL(dev);

#if FT_TRACE
cmn_err (CE_NOTE, "!ftopen : %s on close",
  ft.ft_norwdcl ? "No rewind" : "Rewind");
#endif

	/* Need to seize IRQ from fdc - won't be able to if diskette in use. */
	if (!setFtIntr(1))
		FT_OPEN_ERR("Floppy Disk Controller is Busy");

	/* Save read/write mode of this open. */
	ft.ft_write = (mode & IPW) ? 1 : 0;

	/* Can't write to write-protected cartridge. */
	if (ft.ft_write && (ft.ft_qicstat & QIC_STAT_WPROT))
		FT_OPEN_ERR("Cartridge Write Protected");

	/* Select tape drive. */
	ftSelect(ft.ft_drv, ft.ft_softsel);

	/* Get initial status. */
	drvStatus = ftStsWthErr();
#if FT_TRACE
	cmn_err (CE_NOTE, "!ftopen initial stat=%x", drvStatus);
#endif

	if (drvStatus == -1) {

		if (ftCmd(QIC_CMD_RST))
			FT_OPEN_ERR("Soft Reset Failed ");

		/* Need to wait a second after QIC software reset. */
		ftWait(HZ);

		/* Re-Select tape drive (reset clears soft select). */
		ftSelect(ft.ft_drv, ft.ft_softsel);

		drvStatus = ftStsWthErr();
#if FT_TRACE
		cmn_err (CE_NOTE, "!ftopen retry stat=%x", drvStatus);
#endif

		if (drvStatus == -1)
			FT_OPEN_ERR("Get Drive Status Failed ");
	}

	/* Can't issue most commands if new cartridge is detected. */

	if (ft.ft_qicstat & QIC_STAT_NEWCT) {

		/* Invalidate saved header segment. */
		ft.ft_hdrOk = 0;

		/* Invalidate referenced status. */
		ft.ft_refOk = 0;

		/* New Cart blows no rewind on close. */
		if (ft.ft_norwdcl && ft.ft_segbase > 0) {
#if FT_TRACE
			cmn_err (CE_NOTE,
			  "!ftopen : segbase reset from %ld to zero",
			  ft.ft_segbase);
#endif
			ft.ft_segbase = 0;
		}

		/* Get Error Code clears New Cartridge flag. */
		ftCmd (QIC_CMD_ECD);
		ftReadyWait(FT_RDY_SECS);
	}

	/* If writing, ignore previous buffer contents. */
	if (ft.ft_write) {
		ft.ft_segment = 0;
		ft.ft_numSegs = 0;
	}

	/* On startup or new cart, discard state info. */
	if (!ft.ft_hdrOk) {
	
		/* Invalidate buffer pool contents. */
		ft.ft_segment = 0;
		ft.ft_numSegs = 0;

		/* Invalidate saved seek adjustment values. */
		ft.ft_seekAdj = 0;

		/* Get default format from minor number. */
		if (FT_Q80_FORMAT(dev))
			if (FT_LONGTAPE(dev))
				fmt = FT_Q80_307;
			else
				fmt = FT_Q80_205;
		else
			if (FT_LONGTAPE(dev))
				fmt = FT_Q40_307;
			else
				fmt = FT_Q40_205;

		/* Set default format.  Will be overridden by header segment. */
		ftCvtSetFmt(0);

		/* Initialize tape motion control. */
		ftMNewTape();
	}


	/* Do soft reset if there is an error state. */

	if (ft.ft_qicstat & QIC_STAT_ERR) {

		if (ftCmd(QIC_CMD_RST))
			FT_OPEN_ERR("Soft Reset Failed ");

		/* Need to wait a second after QIC software reset. */
		ftWait(HZ);

		/* Re-Select tape drive (reset clears soft select). */
		ftSelect(ft.ft_drv, ft.ft_softsel);
	}

	/* Wait for drive to be ready, or give up. */
	if(ftReadyWait(FT_RDY_SECS))
		FT_OPEN_ERR("Tape Drive Not Ready");

	/* Need Cartridge Present to be true. */
	if ((ft.ft_qicstat & QIC_STAT_PRSNT) == 0)
		FT_OPEN_ERR("No Cartridge");

	/* Put drive select bits into read/write command. */
	ftxSetDrive (ft.ft_drv);
	
	/* Reset local seek pointer. */
	ft.ft_lastseek = 0;

	return;

	/* Error exit.  Try to clean up as much as possible.*/
badFtOpen:

	set_user_error (EIO);
	ft.ft_open = 0;
	fdcSpecify(FT_SRT_3, FT_HUT, FT_HLT);

	/* Turn off tape motor and soft select. */
	fdcDrvSelect(0, FDC_MOTOR_OFF);
	switch (ft.ft_softsel) {
	case 0:
		break;
	case 1:
		/* A/C/M/S soft select disable. */
		ftCmd(QIC_CMD_SS_OFF);
		break;
	case 2:
		/* CMS soft select disable. */
		ftCmd(47);
		break;
	}

	setFtIntr(0);
	return;
}

/************************************************************************
 * ftread
 *
 * while more segments are to be read
 *   read as many segments as possible into buffer area
 *   if no segments were read
 *     end read prematurely
 *   copy segments from buffer area into user space (iowrite)
 *
 ***********************************************************************/
#if	__USE_PROTO__
static void ftread(o_dev_t dev, IO * iop, __cred_t * __NOTUSED(_credp))
#else
static void
ftread(dev, iop)
o_dev_t dev;
IO * iop;
#endif
{
	int	phySeg;		/* Physical segment number. */
	int	offset;		/* Byte offset within segment. */
	int	ft_seek;

#if FT_TRACE
	cmn_err (CE_NOTE, "!ftread");
#endif

	/* Get reference burst, or fail. */

	if (ftGetRefBurst ())
		FT_READ_ERR("Get Reference Burst Failed ");

	/* Get header segment, or fail. */

	if (ftGetHeaderSeg ())
		FT_READ_ERR("Get Header Segment Failed ");

#if FT_TRACE
	cmn_err (CE_NOTE, "!ftread : Read offset=%d, len=%d.",
	  (int)iop->io_seek, iop->io_ioc);
#endif

	/* Read as much as possible at a time. */

/*
Get a read request, header segment is ok...

Convert seek address - get segment and byte offset.

While there is more data wanted (ioc > 0)
  Copy Buffer Data
  Compute number of segments to ask for, whichever is fewer:
    (Number of segments from start of request to end of track)
    [First one is checked in ftReadSegments.]
    Number of segments that will fit in buffer pool
    Number of segments from start of request to end of data area
  If this number is 0
    Break from loop.
  Ask for the segments.
  If got 0 segments back
    Break from loop.
Endwhile.

Copy Buffer Data

If seek segment is within segments represented by buffer pool
  Set current buffer to the one containing the seek segment.
  While there is another buffer and there are still bytes to transfer:
    Number of bytes to copy is minimum of:
      Bytes in segment after seek byte offset
      Number of bytes in i/o request
    If number of bytes to copy > 0
      Do the copy (iowrite).
      Add # of bytes copied to seek address.
      Convert new seek address - get segment and byte offset.
      (iowrite () has already updated number of bytes in i/o request)
    Advance to next segment buffer.
  Endwhile.
Endif.
*/
	/*
	 * Make a local copy of the user's seek pointer -
	 * driver will update the local copy; file system code outside
	 * the driver maintains the original.
	 */
	ft_seek = iop->io_seek;

	/* Adjust seek address for header segment, bad blocks. */

	if (ftAdjSeek (ft_seek, & phySeg, & offset)) {
		/* Function that just failed will display error. */
#if FT_TRACE
		cmn_err (CE_WARN, "FT : Read Seek Adjust Failure");
#endif
		FT_READ_ERR("FT : Read : Initial seek adjust failed");
	}

	while (iop->io_ioc) {

		int	segIndex;	/* Index into segment buffer pool. */
		int	segsWanted;
		int	segsRead;

		/* Copy Buffer Data. */

#if FT_TRACE
cmn_err (CE_NOTE, "!ftread : Copy buf data, seek %d, phySeg %d, offset %d",
  ft_seek, phySeg, offset);

cmn_err (CE_NOTE, "!ftread : buffer has %d segments starting with %d",
  ft.ft_numSegs, ft.ft_segment);
#endif

		for (segIndex = phySeg - ft.ft_segment;
		  segIndex >= 0 && segIndex < ft.ft_numSegs && iop->io_ioc;
		  segIndex++) {

			int	bytesToCopy;
			int	bytesLeftInSeg;

			bytesLeftInSeg = ftNumBlks [segIndex] * FT_BLK_SIZE
			  - offset;

#if FT_TRACE
cmn_err (CE_NOTE, "!ftread : %d bytes left in segment buffer %d",
  bytesLeftInSeg, segIndex);
#endif

			bytesToCopy = iop->io_ioc;
			if (bytesToCopy > bytesLeftInSeg)
				bytesToCopy = bytesLeftInSeg;

			if (bytesToCopy > 0) {

#if FT_TRACE
cmn_err (CE_NOTE, "!ftread : iowrite(x,%x,%d)",
  (int)(ftBigBuf + (segIndex * FT_SEG_SIZE) + offset), bytesToCopy);
#endif

				iowrite (iop, (char *)(ftBigBuf
				  + (segIndex * FT_SEG_SIZE) + offset),
				  (size_t)bytesToCopy);

				ft_seek += bytesToCopy;
				ft.ft_lastseek = ft_seek;

				if (ftAdjSeek (ft_seek, & phySeg, & offset)) {
#if FT_TRACE
cmn_err (CE_WARN, "ftread : Adj 2 failure");
#endif
					goto badFtRead;
				}

				/* If all wanted data has been moved, quit. */

				if (iop->io_ioc == 0)
					goto okFtRead;

#if FT_TRACE
cmn_err (CE_NOTE, "!ftread : New seek ptr, seek %d, phySeg %d, offset %d",
  ft_seek, phySeg, offset);
#endif

			}

		} /* Endwhile there is another valid segment buffer. */

		/* Calculate how many segments to ask for. */

		segsWanted = (ft.ft_lastDataSeg + 1) - phySeg;
		if (segsWanted > FT_NBUF)
			segsWanted = FT_NBUF;

#if FT_TRACE
cmn_err (CE_NOTE, "!ftread : Want %d segments from tape", segsWanted);
#endif

		if (segsWanted <= 0)
			break;

		segsRead = ftReadSegments (phySeg, segsWanted,
		  (caddr_t)ftBigBuf, FT_USE_BBT,
		  ftSegErr, ftNumBlks);

		if (segsRead == 0)
			break;

		ft.ft_segment = phySeg;
		ft.ft_numSegs = segsRead;

	} /* Endwhile more characters requested. */

okFtRead:
	return;

badFtRead:
	set_user_error (EIO);
	return;
}

/************************************************************************
 * ftunload
 *
 ***********************************************************************/

#if	__USE_PROTO__
static void ftunload (void)
#else
static void
ftunload ()
#endif
{
}

/************************************************************************
 * ftwrite
 *
 ***********************************************************************/
#if	__USE_PROTO__
static void ftwrite(o_dev_t dev, IO * iop, __cred_t * __NOTUSED(_credp))
#else
static void
ftwrite(dev, iop)
o_dev_t dev;
IO * iop;
#endif
{
	int	ft_seek;

	/* Two booleans to allow setting ENOSPC if entire write is past EOT. */
	int	pastEot = 0;
	int	dataWritten = 0;

#if FT_TRACE
	cmn_err (CE_NOTE, "!ftwrite");
#endif

	/* Get reference burst, or fail. */

	if (ftGetRefBurst ())
		FT_WRITE_ERR("Get Reference Burst Failed ");

	/* Get header segment, or fail. */

	if (ftGetHeaderSeg ())
		FT_WRITE_ERR("Get Header Segment Failed ");

#if FT_TRACE
	cmn_err (CE_NOTE, "!ftwrite : Write offset=%d, len=%d.",
	  (int)iop->io_seek, iop->io_ioc);
#endif

	ft_seek = iop->io_seek;

	while (iop->io_ioc > 0) {

		int	phySeg;		/* Physical segment number. */
		int	byteOffset;	/* Byte offset within segment. */
		int	segIndex;	/* Index into segment buffer pool. */
		int	segsWritten;
		int	segsRead;
		unsigned int map;
		int	goodBlocks;
		int	writeBytes;
		int	doFlush;
		int	result;

		/* Adjust seek address for header and bad blocks. */

		result = ftAdjSeek (ft_seek, & phySeg, & byteOffset);

		if (result != 0) {
#if FT_TRACE
			cmn_err (CE_WARN, "FT : Write Seek Adjust Failure");
#endif
			if (result == -2)
				pastEot = 1;

			break;
		}

#if FT_TRACE
cmn_err (CE_NOTE, "!ftwrite : phySeg=%d  byteOffset=%d", phySeg, byteOffset);

cmn_err (CE_NOTE, "!ftwrite : buffer has %d segments starting with %d",
  ft.ft_numSegs, ft.ft_segment);
#endif

		/* Can we continue with a current write buffer? */

		segIndex = phySeg - ft.ft_segment;
		doFlush = 1;

		/* Do not flush if the buffer pool is empty. */

		if (ft.ft_numSegs == 0) {
			doFlush = 0;
			ft.ft_segment = phySeg;
			segIndex = 0;
		  	ftNumBytes [segIndex] = 0;

#if FT_TRACE
cmn_err (CE_NOTE, "!no doFlush 1");
#endif

		/* Do not flush if writing into or just after data in a
		 * currently used buffer. */

		} else if (phySeg >= ft.ft_segment
		  && phySeg < ft.ft_segment + ft.ft_numSegs
		  && byteOffset <= ftNumBytes [segIndex]) {
			doFlush = 0;
		  	ftNumBytes [segIndex] = byteOffset;

#if FT_TRACE
cmn_err (CE_NOTE, "!no doFlush 2");
#endif

		/* Do not flush if we can start on another buffer,
		 * contiguous with previous writes. */

		} else if (ft.ft_numSegs < FT_NBUF
		  && phySeg == ft.ft_segment + ft.ft_numSegs) {
			doFlush = 0;

#if FT_TRACE
cmn_err (CE_NOTE, "!no doFlush 3");
#endif
		  	ftNumBytes [segIndex] = 0;
		}

		/* Is it time to flush the write buffer pool? */

		if (doFlush) {

			segsWritten = ftWriteSegments (ft.ft_segment,
			  ft.ft_numSegs, (caddr_t)ftBigBuf, FT_USE_BBT,
			  ftNumBytes);

			if (segsWritten != ft.ft_numSegs) {
#if FT_TRACE
				cmn_err (CE_WARN,
"FT : Only wrote %d of %d segments, starting at segment %d",
				  segsWritten, ft.ft_numSegs, ft.ft_segment);
#endif
					break;
			}

			ft.ft_segment = phySeg;
			ft.ft_numSegs = 0;
			segIndex = 0;
		  	ftNumBytes [segIndex] = 0;
		}

#if FT_TRACE
cmn_err (CE_NOTE, "!ftwrite : starting to write into buffer %d", segIndex);
#endif

		/* Is preread of a segment needed? */

		if (byteOffset > ftNumBytes [segIndex]) {

			/* Check - buffer should be empty! */
#if FT_TRACE
			if (ftNumBytes [segIndex] > 0)
				cmn_err (CE_WARN,
  "FT : ftwrite : segment %d, buffer %d, write offset %d, buffer has %d",
  phySeg, segIndex, byteOffset, ftNumBytes [segIndex]);
#endif

#if FT_TRACE
cmn_err (CE_NOTE,
  "!ftwrite : Preread of Segment %d for %d bytes.", phySeg, byteOffset);
#endif

			segsRead = ftReadSegments (phySeg, 1,
			  (caddr_t)(ftBigBuf + FT_SEG_SIZE * segIndex),
			  FT_USE_BBT, ftSegErr, ftNumBlks);

			if (segsRead != 1) {
#if FT_TRACE
				cmn_err (CE_WARN,
"FT : Read segment %d (before write at offset %d) failed", phySeg, byteOffset);
#endif
				break;
			}
			ftNumBytes [segIndex] = byteOffset;

		}


		/* At this point, buffer at segIndex represents phySeg. */

		/* Calculate how many bytes we may write into this buffer. */

		ftBBT (phySeg, & map);
		goodBlocks = FT_DATA_BLK_PER_SEG;

#if FT_TRACE
cmn_err (CE_NOTE,
  "!ftwrite : segment %d, buffer %d, has bitmap %x.", phySeg, segIndex, map);
#endif

		while (map != 0) {
			goodBlocks--;
			if (goodBlocks <= 0)
				break;
			/* Clear a 1 bit in the bitmap. */
			map &= (map - 1);
		}

#if FT_TRACE
cmn_err (CE_NOTE, "!ftwrite : %d writable data blocks.", goodBlocks);
#endif

		writeBytes = goodBlocks * FT_BLK_SIZE - byteOffset;

		if (writeBytes > iop->io_ioc)
			writeBytes = iop->io_ioc;

		if (writeBytes > 0) {
			ioread (iop,
			  (char *)(ftBigBuf + (segIndex * FT_SEG_SIZE))
			  + byteOffset, writeBytes);
			dataWritten = 1;

#if FT_TRACE
cmn_err (CE_NOTE, "!ftwrite : ioread (%x, %x, %d).",
  (int)iop, (int)(ftBigBuf + (segIndex * FT_SEG_SIZE)), writeBytes);
#endif

			ft_seek += writeBytes;
			ft.ft_lastseek = ft_seek;
			ftNumBytes [segIndex] += writeBytes;
			if (ft.ft_numSegs <= segIndex)
				ft.ft_numSegs = segIndex + 1;
		}
	}

	/* Set ENOSPC if full write is past EOT. */

	if (pastEot && !dataWritten)
		set_user_error (ENOSPC);

	return;

badFtWrite:
	set_user_error (EIO);
	return;

}

CON	ftcon = {
	DFCHR,				/* Flags */
	FL_MAJOR,			/* Major index */
	ftopen,				/* Open */
	ftclose,			/* Close */
	ftblock,			/* Block */
	ftread,				/* Read */
	ftwrite,			/* Write */
	ftioctl,			/* Ioctl */
	NULL,				/* Powerfail */
	NULL,				/* Timeout */
	ftload,				/* Load */
	ftunload,			/* Unload */
	NULL				/* Poll */
};
