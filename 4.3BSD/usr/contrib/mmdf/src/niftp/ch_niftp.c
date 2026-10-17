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
 *      Modified by Steve Kille from ch_local  July 1982
 *      Mar 83  Steve Kille     Upgrade for new mmdf
 *
 */
#include <signal.h>
#include "ch.h"
#include "phs.h"

extern LLog *logptr;
extern	Chan	*ch_nm2struct();

Chan    *chanptr;               /* Loaction for validating channel      */
/*
 *  CH_PNIFTP  --  Channel for transmission on NIFTP/ JNT mail
 *
 *  This channel produces a JNT Mail format file for each host
 *  and then calls the NIFTP to transmit the file.
 *
 */

/*      MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN     */

main (argc, argv)
int     argc;
char   *argv[];
{
    extern char *dupfpath ();
    short retval;

    mmdf_init (argv[0]);
    siginit ();
    signal (SIGINT, SIG_IGN);          /* always ignore interrupts         */



    if ((chanptr = ch_nm2struct (argv[0])) == (Chan *) NOTOK)
	err_abrt (RP_PARM, "unknown channel name '%s'", argv[0]);
				    /* find out who I am                  */
				    /* NIFTP channel can have several     */
				    /* names                              */

    retval = ch_niftp (argc, argv);
    ll_close (logptr);              /* clean log end, if cycled  */
    exit (retval);
}
/***************  (ch_) NIFTP  MAIL DELIVERY  ************************ */

ch_niftp (argc, argv)             /* send to NIFTP channel               */
int     argc;
char   *argv[];
{
    ch_llinit (chanptr);
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ch_niftp ()");
#endif

    if (rp_isbad (qu_init (argc, argv)))
	return (RP_NO);           /* problem setting-up for deliver     */
    if (rp_isbad (pn_init ()))
	return (RP_NO);

    phs_note (chanptr, PHS_CNSTRT);
    phs_note (chanptr, PHS_CNGOT);

    if (rp_isbad (qu2pn_send (argv[0])))
	return (RP_NO);           /* send the batch of outgoing mail    */

    qu_end (OK);                  /* done with Deliver function         */
    pn_end (OK);

    phs_end (chanptr, RP_OK);

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

    qu_end (NOTOK);
    pn_end (NOTOK);
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

