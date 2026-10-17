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
/*
 *      Modified by Steve Kille from ch_local  August 1982
 *
 */
#include <signal.h>
#include "ch.h"

extern LLog *logptr;

Chan    *curchan;               /* Loaction for validating channel      */

/*
 *  CH_QNIFTP  --  Channel for reception of NIFTP/ JNT mail
 *
 *  This module is made to look like a channel although it is not really
 *
 *  The module is called by the NIFTP, but does not communicate
 *  with it except for the final exit value.  The module interacts
 *  with submit, and passes messages to it.
 *
 *  The major advantage of looking like a channel is that messages
 *  can pretent to submit that they have come in over a channel.
 *  The facilities of submit can then be used for management and
 *  other things (e.g. stamping Via: fields).
 *
 *  It should be called by the NIFTP with an execl of the form
 *  execl( path, name, niftp-queue, fname, TSname) where the parameters are
 *  character strings.  fname is the file containing the JNT mail
 *  format message, and TSname is the calling address of the host.
 *  MMDF will try to identify this as a real host.
 *  niftp-queue is the NIFTP queue name.  This is mapped into an
 *  MMDF channel name
 *
 *
 *
 */

/*      MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN     */

main (argc, argv)
int     argc;
char   *argv[];
{
    extern char *dupfpath ();
    short retval;

    umask(0);
    mmdf_init (argv[0]);
    siginit ();

    retval = ni_niftp (argc, argv);

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "Exiting with value (%s)", rp_valstr (retval));
#endif

    ll_close (logptr);              /* clean log end, if cycled  */
    exit (retval);
}
/***************  (ch_) NIFTP  MAIL DELIVERY  ************************ */

ni_niftp (argc, argv)             /* send to NIFTP channel               */
int     argc;
char   *argv[];
{
    short       result;
    ll_log (logptr, LLOGFST, "ni_niftp (q='%s', f='%s', TS='%s')",
		argv[1], argv[2], argv[3]);

    if (rp_isbad (result = qn_init (argv[1])))
	return (result);           /* problem setting-up NIFTP reception */
				   /* This checks channel validity       */
				   /* based on NIFTP queue name          */
    if (rp_isbad (result = mm_init ()))
	return (result);         /* problem initialising submit          */

    if (rp_isbad (result = qn2mm_send (argv[2], argv[3])))
	return (result);           /* transfer the mail to submit        */

    qn_end (OK);                  /* done with Deliver function         */
    mm_end (OK);

    return (RP_OK);               /* NORMAL RETURN                      */
}

/**/

/* VARARGS2 */

err_abrt (code, fmt, b, c, d)     /* terminate ourself                  */
short     code;
char    fmt[],
	b[],
	c[],
	d[];
{
    char linebuf[LINESIZE];

    qn_end (NOTOK);
    mm_end (NOTOK);
    if (rp_isbad (code))
    {
#ifdef DEBUG
	if (rp_gbval (code) == RP_BNO || logptr -> ll_level >= LLOGBTR)
	{                         /* don't worry about minor stuff      */
	    sprintf (linebuf, "%s%s", "[ABEND:  %s]\t", fmt);
	    ll_log (logptr, LLOGFAT, linebuf, rp_valstr (code), b, c, d);
	    abort ();
	}
#endif
    }
    ll_close (logptr);           /* in case of cycling, close neatly   */
    exit (code);
}


