/*
 * ftMotion.c
 *
 * State information and device control for floppy tape motion and position.
 *
 */

/*
 * ----------------------------------------------------------------------
 * Includes.
 */
#include	<sys/coherent.h>
#include	<sys/cmn_err.h>
#include	<sys/param.h>		/* HZ */

#include	<sys/ft.h>

/*
 * State Information.
 */

static int	ftMovement = FT_STOPPED;
static int	ftTrackValid;
static int	ftPhySegValid;

static struct	ftTapeLocn ftTL;

/************************************************************************
 * ftMSetLocB
 *
 * Update state information on tape position, given block number.
 ***********************************************************************/

#if	__USE_PROTO__
void ftMSetLocB (int blockno)
#else
void
ftMSetLocB (blockno)
int blockno;
#endif
{
	ftTrackValid = 0;
	ftPhySegValid = 0;

	/*
	 * Don't update if incoming coordinates are bogus.
	 * Conversion routine will report the error.
	 */
	if (ftCSectoTL(blockno, & ftTL))
		return;

	ftTrackValid = 1;
	ftPhySegValid = 1;

	return;
}

/************************************************************************
 * ftMSetLoc
 *
 * Update state information on tape position.
 ***********************************************************************/

#if	__USE_PROTO__
void ftMSetLoc (struct ftDiskLocn * ftDLp)
#else
void
ftMSetLoc (ftDLp)
struct ftDiskLocn * ftDLp;
#endif
{
	int	sector;

	ftTrackValid = 0;
	ftPhySegValid = 0;

	/*
	 * Don't update if incoming coordinates are bogus.
	 * Conversion routine will report the error.
	 */
	if (ftCDLtoSec(ftDLp, & sector))
		return;

	if (ftCSectoTL(sector, & ftTL))
		return;

	ftTrackValid = 1;
	ftPhySegValid = 1;

	return;
}

/************************************************************************
 * ftMNewTape
 *
 * Invalidate state information (new tape).
 ***********************************************************************/
#if	__USE_PROTO__
void ftMNewTape (void)
#else
void
ftMNewTape()
#endif
{
	ftTrackValid = 0;
	ftPhySegValid = 0;
	ftMovement = FT_STOPPED;

#if FT_TRACE
	cmn_err (CE_NOTE, "!ftMNewTape");
#endif
}

/************************************************************************
 * ftMGetMtn
 *
 * Get value of tape motion state variable.
 ***********************************************************************/
#if	__USE_PROTO__
int ftMGetMtn (void)
#else
int
ftMGetMtn()
#endif
{
	return ftMovement;
}

/************************************************************************
 * ftMSetMtn
 *
 * Set tape motion state variable.
 ***********************************************************************/
#if	__USE_PROTO__
void ftMSetMtn (int motion)
#else
void
ftMSetMtn(motion)
int motion;
#endif
{
	ftMovement = motion;
}

/************************************************************************
 * ftMReadIDs
 *
 * Given a desired logical sector number, perform continuous Read ID
 * operations until next sector on tape is in the same segment as the
 * desired sector, and before the desired sector.
 *
 * Assume tape is already moving forward at playback speed.
 *
 * Return 0 on success, -1 on failure.
 ***********************************************************************/
#if	__USE_PROTO__
static int ftMReadIDs (int desiredSector)
#else
static int
ftMReadIDs(desiredSector)
int desiredSector;
#endif
{
	int			segmentDiff;
	int			sectorDiff;
	int			currentSector;
	int			readIDTries;
	struct ftDiskLocn	diskLoc;

#if FT_TRACE
	cmn_err (CE_NOTE, "!ftMReadIDs (%d)", desiredSector);
#endif

	readIDTries = 0;

	for (;;) {

		if (ftReadID(& diskLoc))
			return -1;

		ftCDLtoSec(& diskLoc, & currentSector);
#if FT_TRACE
		cmn_err (CE_NOTE, "!at %d", currentSector);
#endif

#if 1
		segmentDiff = currentSector / FT_BLK_PER_SEG
		  - desiredSector / FT_BLK_PER_SEG;

		sectorDiff = currentSector - desiredSector;

		/* One before the target segment.  See if we can quit here. */
		if (segmentDiff == -1)
			break;

		/* Target segment reached. */
		if (segmentDiff == 0 && sectorDiff < 0)
			break;

		/* It is possible for continuous Read ID's to
		 * overshoot.  Retry a reasonable number of times. */

		if (sectorDiff >= 0) {
#if FT_TRACE
			cmn_err (CE_WARN, "FT : overshot sector %d",
			  desiredSector);
#endif
			readIDTries++;
			if (readIDTries < FT_READ_ID_TRIES) {
				if (ftPauseTape()) {
#if FT_TRACE
					cmn_err(CE_WARN, "ftReadIDs: pause failed");
#endif
					return -1;
				}
				ftFwdTape();
			} else {
#if FT_TRACE
				cmn_err(CE_WARN, "ftMPosTape: overshoot (0)");
#endif
				return -1;
			}
			break;
		}
#else
		segmentDiff = (currentSector + 1) / FT_BLK_PER_SEG
		  - desiredSector / FT_BLK_PER_SEG;

		sectorDiff = (currentSector + 1) - desiredSector;

		/* One before the target segment.  See if we can quit here. */
		if (segmentDiff == -1)
			break;

		/* Target segment reached. */
		if (segmentDiff == 0 && sectorDiff <= 0)
			break;

		/* It is possible for continuous Read ID's to
		 * overshoot.  Retry a reasonable number of times. */

		if (sectorDiff > 0) {
#if FT_TRACE
			cmn_err (CE_WARN, "FT : overshot sector %d",
			  desiredSector);
#endif
			readIDTries++;
			if (readIDTries < FT_READ_ID_TRIES) {
				if (ftPauseTape()) {
#if FT_TRACE
					cmn_err(CE_WARN, "ftReadIDs: pause failed");
#endif
					return -1;
				}
				ftFwdTape();
			} else {
#if FT_TRACE
				cmn_err(CE_WARN, "ftMPosTape: overshoot (0)");
#endif
				return -1;
			}
			break;
		}
#endif
	}
	return 0;
}

/************************************************************************
 * ftMPosTape
 *
 * Given a 0-based logical sector number, get tape positioned and moving
 * for a read or write of the specified sector.
 *
 * The tape is properly positioned if the read/write head is over the
 * beginning of the segment containing the desired sector.  At this
 * point, a read/write command issued to the FDC will skip over sectors
 * until the right one is reached, or return meaningful failure status.
 *
 * Return 0 on success, -1 on failure.
 ***********************************************************************/
#if	__USE_PROTO__
int ftMPosTape (int sector)
#else
int
ftMPosTape(sector)
int	sector;
#endif
{
	struct ftTapeLocn	desired;
	struct ftDiskLocn	diskLoc;
	int			skipTries, readIDTries;
	int			inPlace, IDRead;
	int			hereSec;
	struct ftTapeLocn	hereTL;

#if FT_TRACE
	cmn_err (CE_NOTE, "!ftMPosTape (%d)", sector);
#endif

	/* Convert logical sector to tape track, physical segment, and block. */
	if (ftCSectoTL(sector, & desired))
		return -1;

#if FT_TRACE
	cmn_err (CE_NOTE, "!want track=%d seg=%d block=%d",
	  desired.ftTLtrack, desired.ftTLsegment, desired.ftTLblock);
#endif

	/*
	 * 	If current track is not the desired track
	 * 		Seek head to desired track.
	 * 		Wait for drive ready.
	 */

	if ((! ftTrackValid)
	  || (desired.ftTLtrack != ftTL.ftTLtrack)) {
		if(ftSeekTape(desired.ftTLtrack)) {
#if FT_TRACE
			cmn_err(CE_WARN, "ftMPosTape: seek track %d failed",
			  ftTL.ftTLtrack);
#endif
			return -1;
		}
		ftTL.ftTLtrack = desired.ftTLtrack;
		ftTrackValid = 1;
	}


	/*
	 *	If trying to read close to start of physical track
	 *		If even track
	 *			Rewind tape.
	 *		Else (odd track)
	 *			Full forward tape.
	 *		Wait for drive ready.
	 *		Logical forward.
	 *		Update current state and position.
	 *		Return success.
	 */
	if (desired.ftTLsegment <= FT_CUSHION) {

		if ((ftTL.ftTLtrack & 1) == 0) {

			/* Even track. */
			if (ftRewindTape()) {
#if FT_TRACE
				cmn_err(CE_WARN,
				  "ftMPosTape: rewind track %d failed",
				  ftTL.ftTLtrack);
#endif
				return -1;
			}
		} else {

			/* Odd track. */
			if (ftWindTape()) {
#if FT_TRACE
				cmn_err(CE_WARN,
				  "ftMPosTape: wind track %d failed",
				  ftTL.ftTLtrack);
#endif
				return -1;
			}
		}

		ftTL.ftTLsegment = 0;
		ftPhySegValid = 1;

		ftFwdTape ();

		/* If segment > 0 wanted, do read ID's till we get there. */

		if (desired.ftTLsegment > 0) {
			if (ftMReadIDs (sector))
				return -1;
		}

		return 0;
	}

	/*
	 * 	Desired segment is not at start of track.
	 *
	 *	Pause (backs up a couple segments).
	 *	Wait for drive ready.
	 *	While not at desired position
	 *		Logical forward.
	 *		Read Segment ID.
	 *		If current segment is ahead of desired segment
	 *			Skip back some number of segments.
	 *			Wait for drive ready.
	 *		Else if current segment is behind desired segment
	 *			Skip forward some number of segments.
	 *			Wait for drive ready.
	 *		Else
	 *			Tape is at desired position.
	 *	Endwhile.
	 *	Update current state and position.
	 *	Return success.
	 */

	/*
	 * Pausing the tape backs it up a couple segments.
	 * This will have an added benefit of getting us away from
	 * the end of a track, if we happen to be there.
	 */
	if (ftPauseTape()) {
#if FT_TRACE
		cmn_err(CE_WARN, "ftMPosTape: pause failed");
#endif
		return -1;
	}

	for (skipTries = 0, inPlace = 0;
	  skipTries < FT_SKIP_TRIES && ! inPlace; skipTries++) {

		int skipCount;

		if (ftMovement == FT_STOPPED)
			ftFwdTape();

		for (readIDTries = 0, IDRead = 0;
		  readIDTries < FT_READ_ID_TRIES && ! IDRead; readIDTries++) {
			if (ftReadID(& diskLoc) == 0)
				IDRead = 1;
		}

		if (! IDRead) {
#if FT_TRACE
			cmn_err(CE_WARN, "ftMPosTape: read ID failed");
#endif
			return -1;
		}

		/* hereSec is the current tape position (logical sector). */
		ftCDLtoSec(& diskLoc, & hereSec);

#if FT_TRACE
		cmn_err (CE_NOTE, "!block %d", hereSec);
#endif

		/* Want track number for present position. */
		if (ftCSectoTL(hereSec, & hereTL))
			return -1;

#if FT_TRACE
		cmn_err (CE_NOTE, "!track %d", hereTL.ftTLtrack);
#endif

		/* Are we on the right track? */
		if (hereTL.ftTLtrack != desired.ftTLtrack) {

			/* We are on the wrong track. */
			ftStopTape ();
			if(ftSeekTape(desired.ftTLtrack)) {
#if FT_TRACE
				cmn_err(CE_WARN,
				  "ftMPosTape: seek (2) track %d failed",
				  ftTL.ftTLtrack);
#endif
				return -1;
			}
			ftTL.ftTLtrack = desired.ftTLtrack;
			ftTrackValid = 1;
			continue;
		}

		if (hereSec >= sector) {

			/* We are on or past the desired sector.  Back up. */
			skipCount =
			  (hereSec - sector)/FT_BLK_PER_SEG + FT_CUSHION/2;

			ftSkipBack(skipCount);

		} else if (hereSec < sector - FT_CUSHION * FT_BLK_PER_SEG) {

			/* We are before the desired segment.  Go forward. */
			skipCount =
			  (sector - hereSec)/FT_BLK_PER_SEG - FT_CUSHION/2;

			ftSkipFwd(skipCount);

		} else {

			/*
			 * We are between sector - FT_CUSHION * FT_BLK_PER_SEC
			 * and sector - 1, inclusive.  Close enough.
			 */
			inPlace = 1;

		}
	}

	if (inPlace) {

		if (ftMovement == FT_STOPPED)
			ftFwdTape();

		/* Read ID's till just before start of target segment. */

		if (ftMReadIDs (sector))
			return -1;

		return 0;

	} else {

#if FT_TRACE
		cmn_err(CE_WARN, "ftMPosTape: skip to sector %d failed",
		  sector);
#endif
		return -1;
	}
}
