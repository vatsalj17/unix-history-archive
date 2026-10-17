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
/*#define RUNALON   */

/*                  Handle telephone-based i/o                          */

/*  Jul, 80 Dave Crocker    remove ph_d_started setting from ph_init    */
/*  Feb, 82 D. Long	    improved ll_log hdr field for wider chan name */
/*  Mar, 84 D. Long         added use of per-channel phone transcript */

#include "d_returns.h"
#include "ch.h"
#include "phs.h"

extern time_t time ();
extern Chan *curchan;
extern struct ll_struct   *logptr,
			   ph_log;
extern char *logdfldir;
extern char *tbldfldir;
extern char *def_trn;             /* straight transcript of char i/o    */

LOCVAR char ph_d_started;         /* called dial package yet?           */
LOCVAR char *ph_trn;		  /* filled-out version of phone trn file */
LOCVAR char *ph_script;           /* filled-out version of dialing script */

/* ********  (ph_)  PHONE MAIL MASTER INIT/END SUB-MODULE  ********** */

ph_init ()              /* call remote, to do mail            */
{
    extern char *dupfpath ();
    short     retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_init ()");
#endif
    if (ph_d_started)             /* tried before?                      */
    {
	ph_end (NOTOK);           /* make sure old channel gone         */
	sleep (10);               /* just for the hell of it            */
	ll_log (logptr, LLOGTMP, "[ REDIAL ]");
	printx ("re-");
    }
    else
    {
/*	ll_hdinit (&ph_log, " D");   /* header unique for send             */
	ll_hdinit (&ph_log, logptr -> ll_hdr);   /* init header for send    */
/*	ph_log.ll_hdr[0] = logptr -> ll_hdr[0];  */
	ph_log.ll_hdr[0] = 'D';  /* unique letter for send */
	ph_log.ll_file = dupfpath (ph_log.ll_file, logdfldir);
				   /* fill-out path to log files        */

	ph_script = dupfpath (curchan -> ch_script, tbldfldir);

	if (curchan -> ch_trans == (char *) DEFTRANS)
	    ph_trn = dupfpath (def_trn, logdfldir);
	else
	    ph_trn = dupfpath (curchan -> ch_trans, logdfldir);

	ll_log (logptr, LLOGGEN, "dialing");
    }
    printx ("dialing... ");
    fflush (stdout);
#ifndef RUNALON
    ph_d_started = TRUE;
    phs_note (curchan, PHS_CNSTRT);
				  /* note WHICH phone channel           */
    if ((retval = d_masconn (ph_script, &ph_log, TRUE, ph_trn,
			    (ph_log.ll_level >= LLOGBTR) ? TRUE : FALSE))
		< D_OK)
    {
	printx ("couldn't connect...\r\n\t");
	retval = ph_d2rpval (retval);
				  /* map return to rp.h value           */
	ll_log (logptr, LLOGTMP,
		    "unable to connect (%s)", rp_valstr (retval));
	return (retval);
    }
#endif

    printx ("\007connected...\r\n\t");
    fflush (stdout);
    phs_note (curchan, PHS_CNGOT);
    return (RP_OK);
}
/**/

ph_end (type)                     /* done with mail process             */
short     type;                     /* clean / dirty ending               */
{

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_end ()");
#endif

    if (ph_d_started)             /* is phone channel even active?      */
    {
	ph_d_started = FALSE;
	if (type == OK)           /* try clean good end                 */
	{
	    if (rp_isgood (ph_wrec ("end", 3)))
	    {                     /* should also look for the reply     */
		d_masdrop (0, TRUE);
				  /* finish the script                  */
		phs_end(curchan, RP_OK);
		return;
	    }
	}
#ifndef RUNALON
	else                      /* clean bad end                      */
	    d_masdrop (0, 0);     /* don't finish the script            */
#endif

	phs_end(curchan, RP_NO);
	return;
    }
}
/**/

ph_sbinit ()                      /* ready to submit to remote site     */
{
    struct rp_bufstruct thereply;
    int       len;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_sbinit ()");
#endif
    if (rp_isbad (ph_wrec ("submit", 6)) ||
	    rp_isbad (ph_rrply ((struct rp_bufstruct  *) & thereply,
		                                           &len)) ||
	                                            rp_isbad (thereply.rp_val))
    {
						    ll_log (logptr, LLOGFAT, "can't submit");
	return (RP_NO);
    }
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "starting to submit");
#endif
    return (RP_OK);
}

ph_sbend ()                       /* done with submission               */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_sbend ()");
#endif

    if (rp_isbad (ph_wrec ((char *) 0, 0)))
    {
	ll_log (logptr, LLOGTMP, "bad sbend()");
	return (RP_NO);
    }
    return (RP_OK);
}
/**/

ph_pkinit ()                      /* initialize remote pickup            */
{
    struct rp_bufstruct thereply;
    int       len;

#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "ph_pkinit ()");
#endif
    if (rp_isbad (ph_wrec ("pickup", 6)) ||
	    rp_isbad (ph_rrply ((struct rp_bufstruct  *) & thereply,
		                                           &len)) ||
	                                            rp_isbad (thereply.rp_val))
    {
						    ll_log (logptr, LLOGFAT, "can't pickup");
	return (RP_NO);
    }
    return (RP_OK);
}

ph_pkend ()                       /* done with pickup                   */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_pkend ()");
#endif
    return (RP_OK);
}
