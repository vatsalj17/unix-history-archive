#include "util.h"
#include "mmdf.h"

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
/*  CH_IO:  Channel invocation & communication                          */

/*  Jun 80 Dave Crocker     fix ch_close fildes setting to -1
 *  Nov 80 Dave Crocker     convert to ch_struct pointer from ch_code
 *  Jul 81 Dave Crocker     ch_rdrec a little cleaner on error handling
 */

/*#define RUNALON                 /* run without invoking actual channels */

#include "ch.h"
#include <signal.h>

extern char pgm_bakgrnd;
extern struct ll_struct   *logptr;
extern char *chndfldir;

Chan    *ch_curchan;               /* currently active channel program   */


/* **************  (ch_) WRITE MAIL TO AND ADDRESS  ***************** */


ch_winit (ret, msg)    /* initial parameters for a message   */
char *ret, *msg;           /* name of message                    */
{
    RP_Buf rply;
    int len;
    int retval;
    char linebuf[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "ch_winit (%s, %s)", ret, msg);
#endif

    arg2lstr (0, LINESIZE, linebuf, "msg", msg, ret, (char *)0);

    if (rp_isgood (retval = ch_wrec (linebuf)))
	if (rp_isgood (retval = ch_rrply (&rply, &len)))
	{
	    retval = rply.rp_val;
	    if (rp_isbad (retval))
		printx ("Problem with queue: %s\n", rply.rp_line);
	}
    return (retval);
}
/**/

ch_wadr (host, adr)             /* send channel an address              */
    char host[],
	 adr[];
{
    char linebuf[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "ch_wadr (%s, %s)", host, adr);
#endif

    arg2lstr (0, LINESIZE, linebuf, "addr", host, adr, (char *)0);

    return (ch_wrec (linebuf));
}

ch_whend ()                     /* end of sub-list for this host        */
{
    RP_Buf rply;
    int len,
	retval;

    if (rp_isgood (retval = ch_wrec ("hend")))
	if (rp_isgood (retval = ch_rrply (&rply, &len)))
	{
	    retval = rply.rp_val;
	    if (rp_isbad (retval) && rp_gval (retval) != RP_NOOP)
		printx ("Problem host ending: %s\n", rply.rp_line);
	}
#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "ch_hend (%s)", rp_valstr (retval));
#endif
    return (retval);
}

ch_waend ()                     /* end of list for channel              */
{
    RP_Buf rply;
    int len,
	retval;

    if (rp_isgood (retval = ch_wrec ("aend")))
	if (rp_isgood (retval = ch_rrply (&rply, &len)))
	{
	    retval = rply.rp_val;
	    if (rp_isbad (retval))
		printx ("Problem address ending: %s\n", rply.rp_line);
	}
#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "ch_waend (%s)", rp_valstr (retval));
#endif
    return (retval);
}

