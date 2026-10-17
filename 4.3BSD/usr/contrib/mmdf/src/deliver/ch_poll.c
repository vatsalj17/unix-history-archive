/*
 *     MULTI-CHANNEL MEMO DISTRIBUTION FACILITY  (MMDF)
 *     
 *
 *     Copyright (C) 1979,1980,1981  University of Delaware
 *     
 *     Department of Electrical Engineering
 *     University of Delaware
 *     Newark, Delaware  19711
 *
 *     Phone:  (302) 738-1163
 *     
 *     
 *     This program module was developed as part of the University
 *     of Delaware's Multi-Channel Memo Distribution Facility (MMDF).
 *     
 *     Acquisition, use, and distribution of this module and its listings
 *     are subject restricted to the terms of a license agreement.
 *     Documents describing systems using this module must cite its source.
 *
 *     The above statements must be retained with all copies of this
 *     program and may not be removed without the consent of the
 *     University of Delaware.
 *     
 *
 *     version  -1    David H. Crocker    March   1979
 *     version   0    David H. Crocker    April   1980
 *     version  v7    David H. Crocker    May     1981
 *     version   1    David H. Crocker    October 1981
 *
 */
/* do any polling for mail to pickup                                    */

/*    Apr 82  Steve Bellovin	tbldfldir -> logdfldir, for timings files 
 */

#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include <sys/stat.h>
#include "nexec.h"
#include "phs.h"

extern struct ll_struct *logptr;
extern char *logdfldir;
extern char *chndfldir;
extern int *regfdary;
extern int numfds;

ch_poll (checklist)               /* poll hosts for pickup mail         */
    Chan *checklist[];            /* chans which may be processed       */
{
    extern time_t time ();
    time_t  thetime;              /* current time                       */
    register Chan **chanptr;

#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "ch_poll");
#endif

    time (&thetime);
    thetime = thetime / (time_t) 900;
				  /* turn it into quarter-hours         */
    for (chanptr = checklist; *chanptr != 0; chanptr++)
	if ((*chanptr) -> ch_poltime != NOPOLL)
	    if (pol_chk (thetime, *chanptr))
		pol_doit (*chanptr);
}
/**/

LOCFUN
	pol_chk (ptime, pchan)    /* channel due for polling?           */
    time_t    ptime;
    register Chan *pchan;
{
    long    dstrt,                /* when a delivery was last started   */
	    ddone,                /*  "   "     "     "    "  finished  */
	    pstrt,                /*  "   " pickup    "    "  started   */
	    pdone;                /*  "   "     "     "    "  finished  */

    if (pchan -> ch_access & DLVRDID)
	return (FALSE);           /* channel been done, this wakeup     */
    if (! (pchan -> ch_access & CH_PICK))
	return (FALSE);           /* pickup not allowed                 */
    if (pchan -> ch_poltime == NOPOLL)
	return (FALSE);           /* polling not allowed                */

    if (pchan -> ch_poltime == ALLPOLL)
	return (TRUE);            /* poll every wakeup                  */

    dstrt = phs_get (pchan, PHS_WRSTRT);
    if (dstrt > 0L)
	dstrt /= 900L;

    ddone = phs_get (pchan, PHS_WREND);
    if (ddone > 0L)
	ddone /= 900L;

    pstrt = phs_get (pchan, PHS_RESTRT);
    if (pstrt > 0L)
	pstrt /= 900L;

    pdone = phs_get (pchan, PHS_REEND);
    if (pdone > 0L)
	pdone /= 900L;


#ifdef DEBUG
    ll_log (logptr, LLOGFTR,
	    "%s(%d) did=%o, dstrt=%ld, ddone=%ld, pstrt=%ld, pdone=%ld, cur=%ld",
	    pchan -> ch_name, pchan -> ch_poltime,
	    pchan -> ch_access&DLVRDID, dstrt, ddone, pstrt, pdone, ptime);
#endif

    ptime -= pchan -> ch_poltime;

    if (dstrt > ddone)		/* outbound mail still pending		*/
	return (FALSE);
    if (pstrt > pdone)          /* inbound mail probably still pending  */
	return (TRUE);
    if (ddone > ptime || pdone > ptime)
	return (FALSE);           /* still within delay window          */

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "performing pickup");
#endif

    return (TRUE);
}

LOCFUN
	pol_doit (thechan)
    register Chan *thechan;
{
    char temppath[128];
    short tmp;

    ll_log (logptr, LLOGGEN, "pickup %s", thechan -> ch_name);
    printx ("forcing pickup of %s... ", thechan -> ch_name);
    fflush (stdout);

    regfdary[0] = 0;
    regfdary[1] = 1;
    for (tmp = 2; tmp < numfds; regfdary[tmp++] = NOTOK);

/*  run the channel program, in polling mode.  collect its return value.
 *  if there is a really serious error, force a logging message, otherwise,
 *  treat it more casually.
 */
    getfpath (thechan -> ch_ppath, chndfldir, temppath);

    if ((tmp = nexecl (FRKEXEC, FRKWAIT, regfdary, temppath,
		thechan -> ch_name, (domsg) ? "-w" : "-p", (char *)0)) < OK)
	ll_err (logptr, (rp_gbval (tmp) == RP_BNO) ? LLOGTMP : LLOGBTR,
		    "(%s) during %s pickup", rp_valstr (tmp),
		    thechan -> ch_name);

    printx ("\n");
}

