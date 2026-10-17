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
/*  CH_PHONE:  Telephone channel transmission                           */
/*                                                                      */
/*  May, 80 Dave Crocker    fix timerest declaration                    */
/*  Jul, 80 Dave Crocker    remove ph_end from ch_phone for(;;)         */
/*  Aug, 80 Dave Crocker    ch_phone, move retry ok to AFTER sending    */

/*  Jun 81  D. Crocker      Add d2p_nadrs mechanism to send no msg text
 *                          when there are not valid addresses
 *  Jul 81  D. Crocker      Change state variable values, for more robust
 *                          state diagrams, to avoid hang with Deliver
 *  Jun 82  D. Crocker      Add chmod, after close(creat())
 */

#include <signal.h>
#include "ch.h"
#include "phs.h"

extern char *dupfpath();
extern LLog *logptr;

extern char ph_state;              /* state of processing current msg    */
extern jmp_buf  timerest;
extern int      flgtrest;

Chan *curchan;         /* what channel am I?                 */

/*    MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN     */

main (argc, argv)
int     argc;
char   *argv[];
{
    short retval;

    mmdf_init (argv[0]);
    siginit ();
    signal (SIGINT, SIG_IGN);          /* always ignore interrupts           */

    if ((curchan = ch_nm2struct (argv[0])) == (Chan *) NOTOK)
	err_abrt (RP_PARM, "unknown channel name '%s'", argv[0]);
				  /* find out who I am                  */

    if (argc < 2)
	err_abrt (RP_PARM, "ch_phone (%s) arg count", curchan -> ch_name);

    ch_llinit (curchan);
    ll_hdinit (logptr, argv[0]);   /* which channel are we?              */
    if (argc <= 2)
    {
	logptr -> ll_hdr[0] = 'P'; /* note that this is poll mode       */

	if (argv[1][0] != '-')
	    err_abrt (RP_PARM, "ill-formed argument '%s'", argv[1]);
	if (argv[1][1] == 'w')
	    domsg = TRUE;
    }

    ll_log (logptr, LLOGPTR, "%s", argv[0]);

    retval = ch_phone (argc, argv);

    ll_log (logptr, LLOGGEN, "norm end");
    ll_close (logptr);           /* in case of cycling, close neatly   */
    exit (retval);
}
/*************  (ch_) PHONE-BASED MAIL USER/SENDER  ***************** */

ch_phone (argc, argv)		  /* send on phone-type chan            */
int     argc;
char   *argv[];
{
    short     result;
    short     triesleft;            /* number of re-dials we may do       */
    char    maysend;		  /* have messages to send out          */

/*  NOTE:   Besides deliverying mail out through the telephone, this
 *          procedure manages retrieval of mail from the remote host.  To
 *          keep track of its transaction state, it time-stamps several
 *          flag-files.  The time-stamping is used (by Deliver) to decide
 *          whether to force a pickup, when there hasn't been any outgoing
 *          mail for a long time.
 */

    if (argc > 2)		  /* we running under Deliver?          */
    {
	maysend = TRUE;
	if (rp_isbad (qu_init (argc, argv)))
	    return (RP_NO);       /* problem setting-up for deliver     */
    }
    else
	maysend = FALSE;

    for (triesleft = DL_TRIES; triesleft > 0; triesleft--)
    {
	if (setjmp (timerest) != 0)
	{
	    flgtrest = FALSE;
	    break;                /* no retry on dialup/send error      */
	}
	else
	    flgtrest = TRUE;

	if (rp_isbad (result = ph_init ()))
	{                         /* problem dialing foreign site       */
	    if (result != RP_AGN)
		break;            /* isn't worth retrying               */
	    continue;             /* retrying ok                        */
	}

	if (maysend && (curchan -> ch_access & CH_SEND))
	{                         /* TRANSMISSION: SEND MESSAGES        */
	    phs_note (curchan, PHS_WRSTRT);

	    if (rp_isbad (qu2ph_send (argv[0])))
		break;            /* send outgoing.  no retry           */
	    maysend = FALSE;      /* done with sending                  */

	    phs_note (curchan, PHS_WREND);
	    qu_end (OK);          /* done with Deliver function         */
	}

	if (!curchan -> ch_access & CH_PICK)
	    break;                /* pickup not allowed for channel     */

	if (setjmp (timerest) != 0)
	{
	    flgtrest = FALSE;
	    continue;             /* TIMEOUT during pickup; retry ok    */
	}
	else
	    flgtrest = TRUE;

	ll_log (logptr, LLOGPTR, "pickup");
	phs_note (curchan, PHS_RESTRT);

	result = ph2mm_send (curchan);
				  /* RECEPTION: PICKUP MESSAGES         */
	if (rp_gbval (result) == RP_BNO)
	    break;		  /* retrying won't help                */
	if (rp_gbval (result) == RP_BTNO)
	    continue;		  /* maybe retrying will help           */

	triesleft = 0;		  /* nothing to re-try for              */
	phs_note (curchan, PHS_REEND);
	ll_log (logptr, LLOGPTR, "normal hangup");
	ph_end (OK);              /* say goodbye to the nice slave      */
	return (RP_OK);           /* NORMAL RETURN                      */
    }				  /* BELOW THIS is error handling       */

    ll_log (logptr, LLOGPTR, "error hangup");
    ph_end (NOTOK);
    if (maysend)		  /* gave up during attempt to send?    */
	qu_fakrply (ph_state);   /* just pass DEADs back to Deliver      */

    err_abrt (RP_DHST, "Remote site not available");
    /* NOTREACHED */
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
    ph_end (NOTOK);
    if (rp_isbad (code))
	if (rp_gbval (code) == RP_BNO || logptr -> ll_level >= LLOGBTR)
	{                         /* don't worry about minor stuff      */
	    sprintf (linebuf, "%s%s", "err [ ABEND (%s) ]\t", fmt);
	    ll_log (logptr, LLOGFAT, linebuf, rp_valstr (code), b, c, d);
				/* disable error reporting */
#ifdef DEBUG
	    sigabort (fmt);
#endif
	}
    ll_close (logptr);           /* in case of cycling, close neatly   */
    exit (code);
}
