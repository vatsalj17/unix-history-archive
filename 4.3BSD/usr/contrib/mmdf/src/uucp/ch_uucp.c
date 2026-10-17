#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include "phs.h"
#include <signal.h>

/*
 *			C H _ U U C P . C
 *
 *		Take message and feed a request to UUX
 *
 *  qu2uu_send does the interesting work.  This interface was developed
 *  for MMDF by Doug Kingston at the US Army Ballistics Research Lab,
 *  Aberdeen, Maryland.    <dpk@brl>
 *
 *		    Original Version 21 Oct 81
 *
 *			Revision History
 *			================
 *
 */

/*
 *     MULTI-CHANNEL MEMO DISTRIBUTION FACILITY  (MMDF)
 *     
 *
 *     Department of Electrical Engineering
 *     University of Delaware
 *     Newark, Delaware  19711
 *
 *     Phone:  (302) 738-1163
 *     
 *
 */

extern char *dupfpath ();
extern LLog *logptr;

Chan	*curchan;	/* what channel am I?           */

/*      MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN     */

main (argc, argv)
short     argc;
char   *argv[];
{
    short retval;

    mmdf_init (argv[0]);
    siginit ();
    signal (SIGINT, SIG_IGN);	  /* always ignore interrupts             */

    if ((curchan = ch_nm2struct (argv[0])) == (Chan *) NOTOK)
	err_abrt (RP_PARM, "unknown channel name '%s'", argv[0]);

    retval = ch_uucp (argc, argv);
    ll_close (logptr);
#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "exit value is %d.", retval);
#endif
    exit (retval);
}
/***************  (ch_) UUCP MAIL DELIVERY  ************************ */

ch_uucp (argc, argv)		  /* send to remote machine via UUCP   */
short     argc;
char   *argv[];
{
    ch_llinit (curchan);
#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "ch_uucp ()");
#endif

    if (rp_isbad (qu_init (argc, argv)))
	return (RP_NO);		  /* problem setting-up for deliver     */

    if (rp_isbad (uu_init ()))
	return (RP_NO);		  /* problem with uucp startup */

    phs_note (curchan, PHS_CNSTRT);     /* make a timestamp */
    phs_note (curchan, PHS_CNGOT);      /* make a timestamp */
    if (rp_isbad (qu2uu_send ()))
	return (RP_NO);		  /* send the batch of outgoing mail    */

    qu_end (OK);		  /* done with Deliver function         */
    phs_end  (curchan, RP_OK);    /* note end of session */

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "ch_uucp: Normal Return!");
#endif
    return (OK);		  /* NORMAL RETURN                      */
}

/**/
/*VARARGS2*/
err_abrt (code, fmt, b, c, d)     /* terminate ourself                  */
short     code;
char	*fmt, *b, *c, *d;
{
    char linebuf[LINESIZE];

    qu_end (NOTOK);
    if (rp_isbad (code))
    {
#ifdef DEBUG
	if (rp_gbval (code) == RP_BNO || logptr -> ll_level >= LLOGTMP)
	{                         /* don't worry about minor stuff      */
	    sprintf (linebuf, "%s%s", "err [ ABEND (%s) ]\t", fmt);
	    ll_log (logptr, LLOGFAT, linebuf, rp_valstr (code), b, c, d);
	}
#endif
    }
    ll_close (logptr);           /* in case of cycling, close neatly   */
    exit (code);
}

/**/
/*ARGSUSED*/
LOCFUN
	uu_init()
{
#ifdef notdef
#define	UU_CARGS	8
    char    *argv[UU_CARGS];
    int    argc;
#endif notdef

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "uu_init ()");
#endif
#ifdef notdef
    if (isstr(curchan->ch_confstr)) {
	argc = sstr2arg(curchan->ch_confstr, UU_CARGS, argv, " \t");
	while (argc < UU_CARGS) {
	    if (prefix("naddrs=", argv[argc], 7)) {
		if ((uu_naddrs = atoi(argv[argc]+7)) < 1)
		    uu_naddrs = 1;
#ifdef DEBUG
		ll_log (logptr, LLOGFTR, "uu_naddrs now %d", uu_naddrs);
	    } else {
		ll_log (logptr, LLOGTMP, "unknown uucp confstr '%s'", argv[argc]);
#endif
	    }
	    argc++;
	}
    }
#endif notdef
    return (RP_OK);
}
