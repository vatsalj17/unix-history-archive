/*
 *	MULTI-CHANNEL MEMO DISTRIBUTION FACILITY  (MMDF)
 *
 *	Department of Electrical Engineering
 *	University of Delaware
 *	Newark, Delaware  19711
 *
 *
 *	Program Channel: Take message and feed a request to a program
 *
 *
 *	C H _ P R O G . C
 *	=================
 *
 *	main program (qu2pr_send does the interesting work)
 *
 *	J.B.D.Pardoe
 *	University of Cambridge Computer Laboratory
 *	October 1985
 *	
 *	based on the UUCP channel by Doug Kingston (US Army Ballistics 
 *	Research Lab, Aberdeen, Maryland: <dpk@brl>)
 *
 */

#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include "phs.h"
#include <signal.h>


extern struct ll_struct chanlog;
extern char logdfldir[];
struct ll_struct  *logptr = &chanlog;

Chan *chan;


/*
 * main
 * ====
 */
main (argc, argv)
    short   argc;
    char   *argv[];
{
    char *dupfpath ();
    short retval;

    mmdf_init (argv[0]);
    siginit ();
    signal (SIGINT, SIG_IGN); 

    if ((chan = ch_nm2struct (argv[0])) == (Chan *) NOTOK)
	err_abrt (RP_PARM, "unknown channel name '%s'", argv[0]);

    retval = ch_prog (argc, argv);
    ll_close (logptr);

#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "exit value is %d.", retval);
#endif

    exit (retval);
}

/*
 * ch_prog
 * =======
 */
ch_prog (argc, argv)
    short   argc;
    char   *argv[];
{
    ch_llinit (chan);
#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "ch_prog ()");
#endif

    if (rp_isbad (qu_init (argc, argv)))
	return (RP_NO);

    lowerfy (chan->ch_lname);

    phs_note (chan, PHS_CNSTRT);
    phs_note (chan, PHS_CNGOT);

    if (rp_isbad (qu2pr_send ()))
	return (RP_NO);		  

    qu_end (OK);
    phs_end  (chan, RP_OK);

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "ch_prog: Normal Return!");
#endif
    return (OK);
}


err_abrt (code, f, a0, a1, a2)
    short code;
    char  *f, *a0, *a1, *a2;
{
    char buf [LINESIZE];

    qu_end (NOTOK);

#ifdef DEBUG
    if (rp_isbad (code)) {
	if (rp_gbval (code) == RP_BNO || logptr->ll_level >= LLOGTMP) {
	    /* don't worry about minor stuff */
	    sprintf (buf, "%s%s", "err [ ABEND (%s) ]\t", f);
	    ll_log (logptr, LLOGFAT, buf, rp_valstr (code), a0, a1, a2);
	    abort (code);
	}
    }
#endif DEBUG

    ll_close (logptr);           /* in case of cycling, close neatly   */
    exit (code);
}
