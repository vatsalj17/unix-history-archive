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
/*                  SEND FROM NIFTP TO SUBMIT                           */
/*                                                                      */
/*      Adapted from ph2mm_send.c                                       */
/*                                                                      */
/*      Steve Kille     August 1982                                     */
/*                                                                      */
/*      Note that for now, this module only handles one message at      */
/*      a time.  The overhead associated with Scanning a directory      */
/*      is not worthwhile.  Eventually the NIFTP may pass pointers      */
/*      too multiple files                                              */

/*
 * Mar 83  Steve Kille  Adapt for new MMDF
 *                      Changes to remove MMDF dependancies freom NIFTP
 */

#include "ch.h"

extern struct ll_struct   *logptr;

extern Chan *curchan;

LOCVAR int qn_gotone;

qn2mm_send (fname, TSname)
				/* Send message(s) to submit            */
    char fname[];               /* name of message file                 */
    char TSname[];              /* TS name (calling address) of source  */
				/* host - to be translated to host name */
{
    short     result;
    char    info[LINESIZE];
    char    sender[LINESIZE];
    struct rp_bufstruct thereply;
    int len;

#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "qn2mm_send");
#endif

    if (rp_isbad (result = qn_pkinit ()))
	return (result);

    if (rp_isbad (result = mm_sbinit ()))
	return (result);


    if (rp_gval (result = qn_rinit (fname, TSname, info, sender)) != RP_OK){
	if(rp_gval(result) != RP_FIO)
	    qn_rtend (sender, fname, RP_NO);	/* this deletes the message */
	return(result);
    }
    /* 
     * For now we only go through this loop
     * once - multiple submissions later perhaps (but I doubt it)
     */

    strcat (info, "v");
    if (rp_isbad (result = mm_winit (curchan -> ch_name, info, sender)))
	return (result);      /* ready to process a message         */

    if (rp_isbad (result = mm_rrply (&thereply, &len)))
	return (result);      /* how did remote like it?            */

    switch (rp_gbval (thereply.rp_val))
    {			  /* was source acceptable?            */
	case RP_BNO:
    	case RP_BTNO:
	    return (thereply.rp_val);
    }

    if (rp_isbad (result = qn2mm_admng ()))
    {
	qn_rtend (sender, fname, result);
	return (result);
    }

    if (!qn_gotone)		/* stop here if no good addresses     */
				/* send the address list              */
    {
	if (sender [0] == '\0')
	{
	    ll_log (logptr, LLOGTMP,  "Filewith no valid addresseses and no return adr");
	    qn_rtend (sender, fname, RP_NDEL);
	    return (RP_NDEL);
	}
	return (qn_rtend (sender,  fname, RP_OK));
    }

				/* send the message text              */
    if (rp_isbad (result = qn2mm_txmng (sender, fname)))
    {
	qn_rtend (sender, fname, result);
	return (result);
    }

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "done with submission");
#endif

    if (rp_gval (result) == RP_DONE)
	return (RP_OK);
    return (result);
}
/**/

LOCFUN
	qn2mm_admng ()             /* send address list                  */
{
    struct rp_bufstruct thereply;
    int       len;
    short     result;
    char    host[LINESIZE];
    char    adr[LINESIZE];

    qn_gotone = FALSE;
    FOREVER                       /* iterate thru list                    */
    {                             /* we already have and adr              */
	result = qn_radr (host, adr);
	if (rp_isbad (result))
	    return (result);      /* get address from NIFTP file        */
	if (rp_gval (result) == RP_DONE)
	    break;                /* end of address list                */

#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "'%s'@'%s'", adr, host);
#endif

	if (rp_isbad (result = mm_wadr (host, adr)))
	    return (result);      /* give to remote site                */

	if (rp_isbad (result = mm_rrply (&thereply, &len)))
	    return (result);      /* how did remote like it?            */

	switch (rp_gval (thereply.rp_val))
				  /* was address acceptable?            */
	{
	    case RP_AOK:          /* address ok, text not yet sent      */
		qn_wrply (&thereply, adr, host);
		qn_gotone = TRUE;
		break;

	    case RP_NO:           /* remaining acceptible responses     */
		thereply.rp_val = RP_NDEL;
	    case RP_USER:         /* unknown user                       */
	    case RP_NDEL:
	    case RP_AGN:
	    case RP_NOOP:
	    case RP_NS:
		qn_wrply (&thereply, adr, host);
		break;

	    default:              /* responses which force abort        */
		qn_wrply (&thereply, adr, host);
		ll_log (logptr, LLOGFAT,
			"reply err (%s)'%s'",
			rp_valstr (thereply.rp_val), thereply.rp_line);
		if (rp_isbad (thereply.rp_val))
		    return (thereply.rp_val);
		return (RP_RPLY);
	}
    }
    if (rp_isbad (result = qn_raend ()))
	return (result);        /* read end of address list            */
				 /* sets fp to beginning of text       */


    return (mm_waend ());        /* tell remote of address list end    */
}
/**/

LOCFUN
	qn2mm_txmng (sender, fname) /* send message text                  */
   char *sender;                /* Who sent the message - for error     */
				/* replies to go to                     */
    char        *fname;         /* Name of file containing message      */
				/* needed to unlink message             */
{
    struct rp_bufstruct thereply;
    int     len;
    short   result;
    char    buffer[BUFSIZ];

#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "doing text");
#endif
    len = sizeof(buffer);
    while ((rp_gval (result = qn_rtxt (buffer, &len))) == RP_OK)
    {
	if (rp_isbad (result = mm_wtxt (buffer, len)))
	    return (result);
	len = sizeof(buffer);
    }
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "done sending");
#endif

    if (rp_isbad (result))
	return (result);          /* problem with transmission          */
    if (rp_gval (result) != RP_DONE)
	return (RP_RPLY);         /* protocol error in reply code       */

    if (rp_isbad (result = mm_wtend ()))
	return (result);          /* flag end of message                */

    if (rp_isbad (result = mm_rrply (&thereply, &len)))
	return (result);          /* problem getting reply?             */
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "got text reply");
#endif

    switch (rp_gval (thereply.rp_val))
    {                             /* was text acceptable?               */
	case RP_OK:
	case RP_MOK:              /* text was accepted                  */
	    break;

	default:                  /* responses which force abort        */
	    ll_log (logptr, LLOGFAT, "reply err (%s)'%s'",
			rp_valstr (thereply.rp_val), thereply.rp_line);
	    return (thereply.rp_val);
    }

				/* sent it out, so clean up read        */
    if (rp_isbad (result = qn_rtend (sender, fname, RP_OK)))
	return (result);          /* flag end of message                */

    return (RP_OK);
}
