#include "util.h"
#include "mmdf.h"

/*
 *   Copyright University College London - 1984
 *
 *   MMDF channel mapping to the EAN X.400 system
 *
 *   Steve Kille        November 1984
 */

#include <signal.h>
#include "ch.h";
#include "phs.h"

extern Chan     *ch_nm2struct();
extern LLog	*logptr;
Chan    *chanptr;


/*      MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN     */

main (argc, argv)
int     argc;
char   *argv[];
{
    extern char *dupfpath ();
    short retval;
    int   realid,
	  effecid;

    mmdf_init (argv[0]);
    siginit ();
    signal (SIGINT, SIG_IGN);   /* always ignore interrupts             */

    if ((chanptr = ch_nm2struct (argv[0])) == (Chan *) NOTOK)
    {
	ll_log (logptr, LLOGTMP, "qu2en_send (%s) unknown channel", argv[0]);
	exit (NOTOK);
    }

    getwho (&realid, &effecid);
    if (effecid != 0)              /* MUST run as superuser              */
	err_abrt (RP_BHST, "not running as superuser");

    ch_llinit(chanptr);
    retval = ch_ean (argc, argv);
    ll_close (logptr);              /* clean log end, if cycled  */
    exit (retval);
}
/***************  (ch_) EAN MAIL DELIVERY  ************************ */

ch_ean (argc, argv)
int     argc;
char   *argv[];
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ch_ean ()");
#endif

    if (rp_isbad (qu_init (argc, argv)))
	return (RP_NO);           /* problem setting-up for deliver     */
    phs_note (chanptr, PHS_CNSTRT);         /* make a timestamp */
    phs_note (chanptr, PHS_CNGOT);         /* make a timestamp */

    if (rp_isbad (qu2en_send ()))
	return (RP_NO);           /* send the batch of outgoing mail    */

    qu_end (OK);                  /* done with Deliver function         */
    phs_end  (chanptr, RP_OK);     /* note end of session */

    return (RP_OK);               /* NORMAL RETURN                      */
}


/* VARARGS2 */
err_abrt (code, fmt, b, c, d)     /* terminate ourself                  */
short     code;
char    *fmt, *b, *c, *d;
{
    char linebuf[LINESIZE];

    qu_end (NOTOK);
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
