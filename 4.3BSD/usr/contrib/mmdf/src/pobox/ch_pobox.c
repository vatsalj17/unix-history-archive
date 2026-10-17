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
/*                      SEND ON PICKUP REQUEST                          */

#include <signal.h>
#include "ch.h"
#include "phs.h"

extern LLog *logptr;
extern char po_state;	/* state of processing current msg    */

LOCVAR char ibuf[BUFSIZ],         /* input buffer for fd 0              */
	    obuf[BUFSIZ];         /* output buffer for fd 1             */

/*      MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN     */

main (argc, argv)
int     argc;
char   *argv[];
{
    extern char *dupfpath ();
    int retval;

    setbuf (stdin, ibuf);
    setbuf (stdout, obuf);
    mmdf_init (argv[0]);
    siginit ();
    signal (SIGINT, SIG_IGN);      /* always ignore interrupts             */

    retval = ch_pobox (argc, argv);

    ll_close (logptr);

    exit (retval);
}
/*****************  (ch_) LOCAL MAIL DELIVERY  ********************** */

ch_pobox (argc, argv)		  /* send to local machine               */
int     argc;
char   *argv[];
{
    Chan *curchan;
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ch_pobox ()");
#endif

    if ((curchan = ch_nm2struct (argv[0])) == (Chan *) NOTOK)
	err_abrt (RP_PARM, "unknown channel name '%s'", argv[0]);
				  /* find out who I am                  */

    ch_llinit (curchan);
    if (rp_isbad (qu_init (argc, argv)))
	return (RP_NO);		  /* problem setting-up for deliver     */
    if (rp_isbad (po_init ()))
	return (RP_NO);

    phs_note (curchan, PHS_WRSTRT);

    if (rp_isbad (qu2po_send (argv[0])))
    {                             /* send the batch of outgoing mail    */
	qu_fakrply (po_state);   /* problem, but hide from Deliver     */
	po_end (NOTOK);
    }
    else
    {
	po_end (OK);
	phs_note (curchan, PHS_WREND);
    }

    qu_end (OK);                  /* done with Deliver function         */
    return (RP_OK);		  /* NORMAL RETURN                      */
}

/**/

/* VARARGS2 */

err_abrt (code, fmt, b, c, d)     /* terminate ourself                  */
int     code;
char    fmt[],
        b[],
        c[],
        d[];
{
#ifdef DEBUG
    char linebuf[LINESIZE];
#endif

    qu_end (NOTOK);
    po_end (NOTOK);
    if (rp_isbad (code))
    {
#ifdef DEBUG
	if (rp_gbval (code) == RP_BNO || logptr -> ll_level >= LLOGBTR)
	{                         /* don't worry about minor stuff      */
	    sprintf (linebuf, "%s%s", "err [ ABEND (%s) ]\t", fmt);
	    ll_log (logptr, LLOGFAT, linebuf, rp_valstr (code), b, c, d);
	    abort ();
	}
#endif
    }
    ll_close (logptr);           /* in case of cycling, close neatly   */
    exit (code);
}
