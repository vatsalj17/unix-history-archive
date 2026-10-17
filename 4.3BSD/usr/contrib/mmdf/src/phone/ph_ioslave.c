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

/*  May 82  D. Crocker  add chan.log rmsg/wmsg stats, like ch_phone has
 *  Mar 84  D. Long     add use of per-channel phone transcripts
 */

#include "ch.h"
#include "phs.h"
#include "d_returns.h"

extern struct ll_struct    *logptr,
			    ph_log;
extern char *logdfldir;
extern char *def_trn;

LOCVAR Chan *ph_curchan;
LOCVAR char *ph_trn;

/* *********  (ph_)  PHONE MAIL SLAVE INIT/END SUB-MODULE  ************ */

ph_init (curchan)                 /* we are slave: initialize channel   */
    Chan *curchan;
{
    extern char *dupfpath ();
    short retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_init (%s)", curchan -> ch_name);
#endif

    ph_curchan = curchan;
    ph_log.ll_hdr = logptr -> ll_hdr;
    ph_log.ll_file = dupfpath (ph_log.ll_file, logdfldir);
				  /* fill-out path to log file         */
    if (curchan -> ch_trans == (char *) DEFTRANS)
       ph_trn = dupfpath (def_trn, logdfldir);
    else
       ph_trn = dupfpath (curchan -> ch_trans, logdfldir);
    phs_note (curchan, PHS_CNSTRT);
#ifndef RUNALON
    if ((retval = d_slvconn (&ph_log, TRUE, ph_trn,
			(ph_log.ll_level >= LLOGBTR) ? TRUE : FALSE,
			0, DL_RCVSZ, DL_XMTSZ) < D_OK))
    {				  /* standard i/o port, max buf sizes   */
	ll_log (logptr, LLOGTMP, "dial package problem with startup");
	return (ph_d2rpval (retval));
    }
#endif

    phs_note (curchan, PHS_CNGOT);
    return (RP_OK);
}

/* ARGSUSED */

ph_end (type)                     /* say goodbye to the caller          */
short     type;               
{

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_end ()");
#endif
#ifndef RUNALON
    d_slvdrop (0);		  /* always just drop the line          */
#endif

    phs_end (ph_curchan, type ? RP_NO : RP_OK);
    ph_curchan = OK;
    return (RP_OK);
}

/**/

ph_sbinit ()                      /* ready to submit to remote site     */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_sbinit ()");
#endif
    return (RP_OK);
}

ph_sbend ()                       /* done with submission               */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_sbend ()");
#endif

    if (rp_isbad (ph_wrec ((char *)0, 0)))
    {
	ll_log (logptr, LLOGTMP, "bad submit ending");
	return (RP_NO);
    }
    return (RP_OK);
}

ph_pkinit ()                      /* initialize remote pickup            */
{
#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "ph_pkinit ()");
#endif
    return (RP_OK);
}

ph_pkend ()                       /* done with pickup                   */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_pkend ()");
#endif
    return (RP_OK);
}
