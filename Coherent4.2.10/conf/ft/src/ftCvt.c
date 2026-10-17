/*
 * ftCvt.c
 *
 * Conversion routines between logical volume location and
 * either physical tape coordinates or diskette pseudo coordinates.
 */

/*
 * ----------------------------------------------------------------------
 * Includes.
 */
#include	<sys/coherent.h>
#include	<sys/cmn_err.h>
#include	<common/_tricks.h>

#include	<sys/ft.h>

/* Real (tape) and pseudo (diskette) parameters for each cartridge format. */
struct FtParm {
	/* Tape parameters. */
	uint	segPerTrk;
	uint	tracks;

	/* FDC pseudo parameters. */
	uint	blkPerTrk;
	uint	heads;
	uint	cyls;
};

/* The following is a read-only table.	*/

static struct FtParm	ftParm[] = {
/* segments/track, tracks, blocks/pseudotrack, heads, cylinders	*/
	{ 150, 28, 128, 7, 150 },		/* QIC-80, 307.5'	*/
	{ 100, 28, 128, 5, 150 },		/* QIC-80, 205'		*/
	{ 102, 20, 128, 2, 255 },		/* QIC-40, 307.5'	*/
	{  68, 20, 128, 2, 170 }		/* QIC-40, 205'		*/
};

static struct FtParm	ftP;

static int	formatSet;
static int	ftSegsPerVol;
static int	ftBlksPerVol;

/************************************************************************
 * ftCBlocksLeft
 *
 * Return number of blocks to end of present tape track.
 ***********************************************************************/

#if	__USE_PROTO__
int ftCBlocksLeft (struct ftTapeLocn * tlp)
#else
int
ftCBlocksLeft (tlp)
struct ftTapeLocn * tlp;
#endif
{
	int retval;

	retval = (FT_BLK_PER_SEG - tlp->ftTLblock)
	  + (ftP.segPerTrk - (tlp->ftTLsegment + 1))
	  * FT_BLK_PER_SEG;

#if FT_TRACE
	cmn_err (CE_NOTE,
	  "!ftCBlocksLeft : %d blocks left on track %d",
	  retval, tlp->ftTLtrack);
#endif

	return retval;
}

/************************************************************************
 * ftCBlksPerVol
 *
 * Return number of blocks per tape.
 *
 * Return -1 if no valid format is set.
 ***********************************************************************/

#if	__USE_PROTO__
int ftCBlksPerVol (void)
#else
int
ftCBlksPerVol ()
#endif
{
	if (!formatSet) {
#if FT_TRACE
		cmn_err(CE_WARN, "ftCBlksPerVol: unknown ft format");
#endif
		return -1;
	}

	return ftBlksPerVol;
}

/************************************************************************
 * ftCDLInc
 *
 * Increment by one a position given in fdc pseudocoordinates.
 *
 * Return -1 if format unknown or increment takes us past end of volume.
 ***********************************************************************/

#if	__USE_PROTO__
int ftCDLInc (struct ftDiskLocn * dlp)
#else
int
ftCDLInc (dlp)
struct ftDiskLocn * dlp;
#endif
{
	if (!formatSet) {
#if FT_TRACE
		cmn_err(CE_WARN, "ftCDLInc: unknown ft format");
#endif
		return -1;
	}

	dlp->ftDLsector++;
	if (dlp->ftDLsector > ftP.blkPerTrk) {

		/* Went over to next pseudotrack.  Sectors start at 1, not 0. */
		dlp->ftDLsector = 1;
		dlp->ftDLcylinder++;

		if (dlp->ftDLcylinder >= ftP.cyls) {

			/* Went over to next cylinder. */
			dlp->ftDLcylinder = 0;
			dlp->ftDLhead++;

			if (dlp->ftDLhead >= ftP.heads) {

				/* Went too far!  Past end of tape. */
#if FT_TRACE
				cmn_err (CE_WARN,
				  "ftCDLInc: Increment past EOT");
#endif
				return -1;
			}
		}
	}

	return 0;
}

/************************************************************************
 * ftCLBtoDL
 *
 * Given a 0-based logical block number, calculate diskette pseudocoordinates.
 *
 * Return 0 on success.
 * Return -1 if block number out of range or tape format is not known.
 ***********************************************************************/

#if	__USE_PROTO__
int ftCLBtoDL	(int logicalBlk, struct ftDiskLocn * diskLoc)
#else
int
ftCLBtoDL (logicalBlk, diskLoc)
int			logicalBlk;
struct ftDiskLocn	* diskLoc;
#endif
{
	if (!formatSet) {
#if FT_TRACE
		cmn_err(CE_WARN, "ftCLBtoDL: unknown ft format");
#endif
		return -1;
	}

	if (logicalBlk < 0 || logicalBlk >= ftBlksPerVol) {
#if FT_TRACE
		cmn_err(CE_WARN, "ftCLBtoDL: block # %d invalid",
		  logicalBlk);
#endif
		return -1;
	}

	diskLoc->ftDLsector = (logicalBlk % ftP.blkPerTrk) + 1;
	logicalBlk /= ftP.blkPerTrk;

	diskLoc->ftDLcylinder = logicalBlk % ftP.cyls;
	logicalBlk /= ftP.cyls;

	diskLoc->ftDLhead = logicalBlk;

	diskLoc->ftDLsecsize = 3;	/* N=3 for 1024 bytes/block */

	return 0;
}

/************************************************************************
 * ftCLStoTL
 *
 * Given a 0-based logical segment number, calculate track and physical
 * segment number.
 *
 * Return 0 on success.
 * Return -1 if segment number out of range or tape format is not known.
 ***********************************************************************/

#if	__USE_PROTO__
int ftCLStoTL	(int logicalSeg, struct ftTapeLocn * tapeLoc)
#else
int
ftCLStoTL (logicalSeg, tapeLoc)
int			logicalSeg;
struct ftTapeLocn	* tapeLoc;
#endif
{
	if (!formatSet) {
#if FT_TRACE
		cmn_err(CE_WARN, "ftCLStoTL: unknown ft format");
#endif
		return -1;
	}

	if (logicalSeg < 0 || logicalSeg >= ftSegsPerVol) {
#if FT_TRACE
		cmn_err(CE_WARN, "ftCLStoTL: segment # %d invalid",
		  logicalSeg);
#endif
		return -1;
	}

	tapeLoc->ftTLtrack = logicalSeg / ftP.segPerTrk;
	tapeLoc->ftTLsegment = logicalSeg % ftP.segPerTrk;
	tapeLoc->ftTLblock = 0;

	return 0;
}

/************************************************************************
 * ftCSectoTL
 *
 * Given a 0-based logical sector number, calculate track, physical
 * segment number, and block (0..31) within segment.
 *
 * Return 0 on success.
 * Return -1 if sector number out of range or tape format is not known.
 ***********************************************************************/

#if	__USE_PROTO__
int ftCSectoTL	(int sector, struct ftTapeLocn * tapeLoc)
#else
int
ftCSectoTL (sector, tapeLoc)
int			sector;
struct ftTapeLocn	* tapeLoc;
#endif
{
	int	segment;

	if (!formatSet) {
#if FT_TRACE
		cmn_err(CE_WARN, "ftCSectoTL: unknown ft format");
#endif
		return -1;
	}

	if (sector < 0 || sector >= ftBlksPerVol) {
#if FT_TRACE
		cmn_err(CE_WARN, "ftCSectoTL: sector # %d invalid",
		  sector);
#endif
		return -1;
	}

	segment = sector / FT_BLK_PER_SEG;

	tapeLoc->ftTLblock = sector % FT_BLK_PER_SEG;
	tapeLoc->ftTLsegment = segment % ftP.segPerTrk;
	tapeLoc->ftTLtrack = segment / ftP.segPerTrk;

	return 0;
}

/************************************************************************
 * ftCDLtoSec
 *
 * Given a set of diskette pseudo coordinates, calculate the logical
 * sector number.
 *
 * Return 0 on success.
 * Return -1 if coordinates out of range or tape format is not known.
 ***********************************************************************/

#if	__USE_PROTO__
int ftCDLtoSec	(struct ftDiskLocn * diskLoc, int * logSector)
#else
int
ftCDLtoSec (diskLoc, logSector)
struct ftDiskLocn	* diskLoc;
int			* logSector;
#endif
{
	int	lsn;	/* logical *sector* number, as in QIC standards */

	if (!formatSet) {
#if FT_TRACE
		cmn_err(CE_WARN, "ftCDLtoSec: unknown ft format");
#endif
		return -1;
	}

#if FT_TRACE
	if (diskLoc->ftDLhead >= ftP.heads) {
		cmn_err(CE_WARN, "ftCDLtoSec: C=%d/H=%d/R=%d:H invalid",
		  diskLoc->ftDLcylinder, diskLoc->ftDLhead, diskLoc->ftDLsector);
	}

	if (diskLoc->ftDLcylinder >= ftP.cyls) {
		cmn_err(CE_WARN, "ftCDLtoSec: C=%d/H=%d/R=%d:C invalid",
		  diskLoc->ftDLcylinder, diskLoc->ftDLhead, diskLoc->ftDLsector);
	}

	if (diskLoc->ftDLsector - 1 >= ftP.blkPerTrk) {
		cmn_err(CE_WARN, "ftCDLtoSec: C=%d/H=%d/R=%d:R invalid",
		  diskLoc->ftDLcylinder, diskLoc->ftDLhead, diskLoc->ftDLsector);
	}
#endif

	/* Sort applying Horner's rule here... */

	lsn = diskLoc->ftDLhead;
	lsn *= ftP.cyls;
	lsn += diskLoc->ftDLcylinder;
	lsn *= ftP.blkPerTrk;
	lsn += diskLoc->ftDLsector - 1;

	* logSector = lsn;

	return 0;
}

/************************************************************************
 * ftCDLtoLS
 *
 * Given a set of diskette pseudo coordinates, calculate the logical
 * segment number.
 *
 * Return 0 on success.
 * Return -1 if coordinates out of range or tape format is not known.
 ***********************************************************************/

#if	__USE_PROTO__
int ftCDLtoLS	(struct ftDiskLocn * diskLoc, int * logicalSeg)
#else
int
ftCDLtoLS (diskLoc, logicalSeg)
struct ftDiskLocn	* diskLoc;
int			* logicalSeg;
#endif
{
	int	lsn;	/* logical *sector* number, as in QIC standards */

	if (ftCDLtoSec (diskLoc, &lsn))
		return -1;

	* logicalSeg = lsn / FT_BLK_PER_SEG;

	return 0;
}

/************************************************************************
 * ftCvtSetFmt
 *
 * Initialize conversion routines for a specific QIC-80 or QIC-40 format.
 *
 * Return 0 on success.
 * Return -1 if format number is invalid.
 ***********************************************************************/

#if	__USE_PROTO__
int ftCvtSetFmt	(int format)
#else
int
ftCvtSetFmt (format)
int	format;
#endif
{
	if (format < 0 || format >= __ARRAY_LENGTH(ftParm)) {
#if FT_TRACE
		cmn_err(CE_WARN, "ftCvtSetFmt: format # %d invalid", format);
#endif
		formatSet = 0;
		return -1;
	}
	formatSet = 1;
	ftP = ftParm [format];

	ftSegsPerVol = ftP.segPerTrk * ftP.tracks;
	ftBlksPerVol = ftSegsPerVol * FT_BLK_PER_SEG;

	return 0;
}

/************************************************************************
 * ftCvtSetFtParms
 *
 * Initialize conversion routines for a specific QIC-80 or QIC-40 format.
 * Like ftCvtSetFmt, except get individual parameter values (e.g. from
 * tape header segment), and use default values for any zero arguments.
 ***********************************************************************/

#if	__USE_PROTO__
void ftCvtSetFtParms (unsigned int spt, unsigned int tracks,
  unsigned int heads, unsigned int cyls)
#else
void
ftCvtSetFtParms (spt, tracks, heads, cyls)
unsigned int spt;
unsigned int tracks;
unsigned int heads;
unsigned int cyls;
#endif
{
	formatSet = 1;

	if (spt > 0)
		ftP.segPerTrk = spt;

	if (tracks > 0)
		ftP.tracks = tracks;

	if (heads > 0)
		ftP.heads = heads;

	if (cyls > 0)
		ftP.cyls = cyls;

	ftSegsPerVol = ftP.segPerTrk * ftP.tracks;
	ftBlksPerVol = ftSegsPerVol * FT_BLK_PER_SEG;
}

/************************************************************************
 * ftCvtUnsetFmt
 *
 * Set tape format invalid (e.g., new tape).
 ***********************************************************************/

#if	__USE_PROTO__
void ftCvtUnsetFmt	(void)
#else
void
ftCvtUnsetFmt ()
#endif
{
	formatSet = 0;
}
