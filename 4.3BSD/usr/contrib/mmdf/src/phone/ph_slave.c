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
/*
 *  PHONENET SLAVE PROCESS
 *
 *  Handles telephone link & mail protocols, interfacing to MMDF for
 *  submission and pickup, via the mm_ package.
 *
 *  One argument is required:  The name of the channel the caller is
 *  claiming to come in from.  (It may be possible for one username to have
 *  access to mail for more than one channel, so the caller must indicate
 *  the channel of interest, for pickup.  However, Submit and/or Ch_PoBox
 *  will verify the claim.)
 *
 *  Sep 81  D. Crocker      allow reading channel name as first input line
 *  Nov 81  D. Crocker      commands case-insensitive
 *  Apr 82  D. Crocker      check chan name; have llog hdr show chan char
 *  Feb 84  D. Long	    made full channel name show in llog hdr
 *  Apr 84  D. Long         fixed phs_note call after pickup 
 */

#include <signal.h>
#include "ch.h"
#include "phs.h"

#define CTIMEOUT	180	/* 3 minute timeout on response to channel prompt */

extern char *dupfpath ();
extern LLog *logptr;

extern jmp_buf timerest;           /* return location saved for timeout  */
extern int     flgtrest;

char    ttyobuf[BUFSIZ];

LOCVAR struct rp_construct        /* replies                            */
	    rp_unknown =
{
    RP_PROT, 'U', 'n', 'k', 'n', 'o', 'w', 'n', ' ', 'p', 'r', 'o',
    't', 'o', 'c', 'o', 'l', ' ', 'c', 'o', 'm', 'm', 'a', 'n', 'd', '\0'
}          ,
	    rp_sbok =
{
    RP_OK, 's', 'e', 'n', 'd', ' ', 'o', 'k', '\0'
}          ,
	    rp_pkok =
{
    RP_OK, 'p', 'i', 'c', 'k', 'u', 'p', ' ', 'o', 'k', '\0'
};


/*   MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN      */

main (argc, argv)
int       argc;
char   *argv[];
{
    Chan *curchan;
    char chaname[64];

    umask(0);

    setbuf (stdout, ttyobuf);
    mmdf_init (argv[0]);

    siginit ();
    if (argv[0][0] == '-')
    {                             /* if login shell, ignore quit & int  */
	signal (SIGINT, SIG_IGN);
	signal (SIGQUIT, SIG_IGN);
    }

    if (setjmp (timerest) != 0)   /* other side took too long; give up    */
    {
	flgtrest = FALSE;
	err_abrt (RP_TIME, "Timeout");
    }
    else
	flgtrest = TRUE;

    if (argc == 1)                /* channel name not specified as arg  */
    {
	s_alarm (CTIMEOUT);
	printf ("channel: ");
	fflush (stdout);
	gets (chaname);           /* read it                            */
	s_alarm (0);
    }
    else                          /* use the argument                   */
	strcpy (chaname, argv[1]);

    ll_hdinit (logptr, chaname); /* which channel are we?    */

    if ((curchan = ch_nm2struct (chaname)) == (Chan *) NOTOK)
	err_abrt (RP_PARM, "Channel '%s' is unknown", chaname);

    ph_slave (curchan);
}

/* *****************  SERVER/SLAVE MODE  ****************************** */

ph_slave (curchan)                /* service requests from caller       */
    Chan *curchan;
{
    int       len;
    short     retval;
    char    linebuf[LINESIZE];
    register char *ptr;

    ch_llinit (curchan);
    if (rp_isbad (retval = ph_init (curchan)))
	err_abrt (retval, "Error with initialization");
				  /* set-up to service requests         */

    FOREVER
    {				  /* what does caller want to do?       */
	if (rp_isbad (retval = ph_rrec (linebuf, &len)))
	    err_abrt (retval, "Error return getting command");

#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "Command: \"%s\"", linebuf);
#endif

	for (ptr = linebuf; !isnull (*ptr); ptr++)
	    *ptr = uptolow (*ptr);

	if (len == 0 ||		  /* all done?                          */
		equal (linebuf, "end", 3) ||
		rp_gval (retval) == RP_DONE)
	    break;		  /* exit                               */

	if (equal (linebuf, "submit", 6))
	{			  /* gonna RECEIVE mail                 */
	    sl_submit (curchan);
	    continue;
	}
	if (equal (linebuf, "pickup", 6))
	{			  /* gonna SEND mail                    */
	    sl_pickup (curchan);
	    continue;
	}

	ph_wrply ((struct rp_bufstruct *) &rp_unknown,
					    rp_conlen (rp_unknown));
	err_abrt (rp_unknown.rp_cval, "%s:  '%s'",
			rp_unknown.rp_cline, linebuf);
    }
    ph_end (OK);
    mm_end (OK);
    exit (RP_DONE);
}
/**/

sl_submit (curchan)               /* dm_slave submission management     */
    Chan *curchan;
{
    short     retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "[ Submit ]");
#endif

    if (rp_isbad (retval = ph_wrply ((struct rp_bufstruct *) &rp_sbok,
			    rp_conlen (rp_sbok))))
		    err_abrt (retval, "Unable to ack submission request");

    phs_note (curchan, PHS_RESTRT);

    if (rp_isbad (retval = ph2mm_send (curchan)))
	err_abrt (retval, "ph2mm_send error");

    phs_note (curchan, PHS_REEND);
}

sl_pickup (curchan)               /* dm_slave pickup management         */
    Chan *curchan;
{
    short     retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "[ Pickup ]");
#endif

    if (rp_isbad (retval = ph_wrply ((struct rp_bufstruct *) &rp_pkok,
			    rp_conlen (rp_pkok))))
		    err_abrt (retval, "Unable to ack pickup request");

    if (rp_isbad (retval = mm2ph_send (curchan)))
	err_abrt (retval, "mm2ph_send error");
}

/* *************************  UTILITIES  ****************************** */

/* VARARGS2 */

err_abrt (code, fmt, b, c, d)
int       code;
char   *fmt,
       *b,
       *c,
       *d;
{
    if (fmt != 0)
	err_gen (code, fmt, b, c, d);

    ph_end (NOTOK);

    ll_log (logptr, LLOGFAT, "Ending [%s]", rp_valstr (code));

    ll_close (logptr);            /* put out "cycled" msg, if needed    */

#ifdef DEBUG
    if (rp_gbval (code) == RP_BTNO && rp_gcval (code) == RP_CCON)
	exit (code);		  /* connection-related temp error      */

    abort ();
#else
    exit (code);
#endif
}
/**/

/* VARARGS2 */

err_gen (code, fmt, b, c, d)      /* standard error processing          */
int       code;                     /* see err_abrt, for explanation      */
char   *fmt,
       *b,
       *c,
       *d;
{
    extern int    errno;
    char newfmt[LINESIZE];

    if (rp_isgood (code))	  /* it wasn't an error                 */
	return;

    printf (fmt, b, c, d);        /* the user what the problem is       */
    putchar ('\n');
    fflush (stdout);

    mm_end (NOTOK);
    switch (code)		  /* log the error?                     */
    {
	case RP_HUH: 		  /* not if it was a user error         */
	case RP_PARM: 
	case RP_USER: 
	    break;

	default: 
	    sprintf (newfmt, "%s%s", "err [ ABEND (%s) ] ", fmt);
	    ll_err (logptr, LLOGFAT, newfmt, rp_valstr (code), b, c, d);
    }
}
