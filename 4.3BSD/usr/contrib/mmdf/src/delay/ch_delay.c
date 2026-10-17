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

/*                  DELAYED PROCESSING CHANNEL                          */
/*                  hacked up from the list channel Phil UCL Nov85      */

#include <signal.h>
#include "phs.h"
#include "ch.h"

extern char *dupfpath();
extern LLog *logptr;

Chan *chanptr;
char obuf[BUFSIZ];           /* Buffer for status printfs */

/*      MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN     */

main (argc, argv)
int       argc;
char   *argv[];
{
    short retval;

    mmdf_init (argv[0]);
    setbuf( stdout, obuf );

#ifdef RUNALON
    logptr -> ll_fd = 1;
    ll_init (logptr);
#endif

    siginit ();
    signal (SIGINT, SIG_IGN);     /* always ignore interrupts             */

    if ((chanptr = ch_nm2struct (argv[0])) == (Chan *) NOTOK)
    {
	ll_log (logptr, LLOGTMP, "delayproc (%s) unknown channel", argv[0]);
	exit (RP_PARM);
    }
    retval = ch_delay (argc, argv);
    ll_close (logptr);
    exit (retval);
}
/***************  (ch_) DELAYED SUBMISSION DELIVERY  ***************** */

ch_delay (argc, argv)              /* send to internet                   */
int       argc;
char   *argv[];
{
    ch_llinit (chanptr);
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ch_delay()");
#endif

    if (rp_isbad (qu_init (argc, argv)))
	return (RP_NO);           /* problem setting-up for deliver     */

    phs_note (chanptr, PHS_WRSTRT);

    if (rp_isbad (qu2ds_send ()))
	return (RP_NO);           /* send the batch of outgoing mail    */

    phs_note (chanptr, PHS_WREND);

    qu_end (OK);                  /* done with Deliver function         */

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
#ifdef DEBUG
    char linebuf[LINESIZE];
#endif

    qu_end (NOTOK);

#ifdef DEBUG
    if (rp_gbval (code) == RP_BNO || logptr -> ll_level >= LLOGBTR)
    {                         /* don't worry about minor stuff      */
	sprintf (linebuf, "%s%s", "err [ ABEND (%s) ]\t", fmt);
	ll_log (logptr, LLOGFAT, linebuf, rp_valstr (code), b, c, d);
	abort ();
    }
#endif
    ll_close (logptr);            /* in case of cycling, close neatly   */

    exit (code);
}
