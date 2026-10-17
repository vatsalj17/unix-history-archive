/*
 * File:	ftGRB.c
 *
 * Purpose:	Find Reference Burst.
 *
 * Revised:	Sun May 29 08:39:26 1994 CDT
 */

/*
 * ----------------------------------------------------------------------
 * Includes.
 */
#include <sys/coherent.h>

#include <sys/cmn_err.h>
#include <common/ccompat.h>
#include <common/xdebug.h>

#include <sys/ft.h>

/*
 * ----------------------------------------------------------------------
 * Definitions.
 *	Constants.
 *	Macros with argument lists.
 *	Typedefs.
 *	Enums.
 */

/*
 * ----------------------------------------------------------------------
 * Functions.
 *	Import Functions.
 *	Export Functions.
 *	Local Functions.
 */

/*
 * ----------------------------------------------------------------------
 * Global Data.
 *	Import Variables.
 *	Export Variables.
 *	Local Variables.
 */

/*
 * ----------------------------------------------------------------------
 * Code.
 */

/************************************************************************
 * ftGetRefBurst
 *
 * Locate reference burst on the current tape.
 * Failure may mean unformatted tape, or I/O error.
 * Return 0 on success, -1 on failure.
 ***********************************************************************/
#if __USE_PROTO__
int ftGetRefBurst (void)
#else
int
ftGetRefBurst ()
#endif
{
	int	drvStatus;
	char	* msg;

	if (ft.ft_refOk)
		return 0;

	drvStatus = ftStsWthErr();
	if (drvStatus == -1) {
		msg = "FT:GRB: Get Drive Status Failed ";
		goto badGRB;
	}

	/* Has reference burst been seen? */

	if ((drvStatus & QIC_STAT_REFD) == 0) {

		/*
		 * If not, look for reference burst now.
		 * If this fails, it's probably due to unformatted tape.
		 */

		ftCmd(QIC_CMD_CAL);
		if (ftReadyWait(FT_CAL_SECS)) {
			msg = "FT:GRB: No reference burst - probably unformatted tape";
			goto badGRB;
		}

		/* Again, has reference burst been seen? */

		drvStatus = ftStsWthErr ();
		if (drvStatus == -1) {
			msg = "FT:GRB: get drive status failed ";
			goto badGRB;
		}

		if ((drvStatus & QIC_STAT_REFD) == 0) {
			msg = "FT:GRB: seek load point failed";
			goto badGRB;
		}

		/* Initialize position information. */
		ftMNewTape ();
	}

	ft.ft_refOk = 1;
	return 0;

badGRB:

#if FT_TRACE
	cmn_err (CE_WARN, "FT : GRB : %s", msg);
#endif

	return -1;
}
