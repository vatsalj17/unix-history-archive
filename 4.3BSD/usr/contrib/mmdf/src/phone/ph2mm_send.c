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
 *     Mar 84   D. Long         added RP_PARM in p2m_txmsg() for "no
 *                              addressees" return
 *     Apr 84   D. Long         added RP_PARM & flush in p2m_admng() for
 *                              "illegal return address" return
 *                              moved RP_PARM in p2m_txmsg to return RP_NDEL
 */
/*                  SEND FROM PHONE TO LOCAL (SUBMIT)                   */

#include "ch.h"
#include "phs.h"

extern Chan *curchan;
extern LLog *logptr;

LOCVAR long msglen;
LOCVAR int numadrs;

ph2mm_send (chan)             /* overall mngmt for batch of msgs    */
    Chan *chan;               /* who to say is doing relaying       */
{
    struct rp_bufstruct thereply;
    int       len;
    short     result;
    char    info[LINESIZE],
            sender[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "ph2mm_send");
#endif

    if (rp_isbad (result = ph_pkinit ()))
    {
	printx ("remote site refuses to send mail... ");
	fflush (stdout);
	return (result);
    }
    if (rp_isbad (result = mm_sbinit ()))
	return (result);

    printx ("starting to pickup mail... ");
    fflush (stdout);

    while (rp_gval (result = ph_rinit (info, sender)) == RP_OK)
    {
	strcat (info, "v");
	if (rp_isbad (result = mm_winit (chan -> ch_name, info, sender)))
	    return (result);      /* ready to process a message         */

	if (rp_isbad (result = mm_rrply (&thereply, &len)))
	    return (result);      /* how did remote like it?            */

	switch (rp_gbval (thereply.rp_val))
	{			  /* was source acceptable?            */
	    case RP_BNO:
    	    case RP_BTNO:
		return (thereply.rp_val);
	}

	if (rp_isbad (result = p2m_admng ()))
	    return (result);      /* send the address list              */

	if (rp_isbad (result = p2m_txmng ()))
	    return (result);      /* send the message text              */

	phs_msg (chan, numadrs, msglen);
    }

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "done with submission");
#endif
    if (rp_isbad (ph_pkend ()))   /* done getting messages              */
	printx ("bad ending for incoming submission");
    printx ("done\n");
    fflush (stdout);

    mm_sbend ();                  /* done sending messages              */

    if (rp_gval (result) == RP_DONE)
	return (RP_OK);
    return (result);
}
/**/

LOCFUN
	p2m_admng ()              /* get & process address list         */
{
    struct rp_bufstruct thereply;
    int       len;
    short     result;
    int       flush = FALSE;
    char    host[LINESIZE];
    char    adr[LINESIZE];

    for (numadrs = 0; ; )         /* iterate through list from remote   */
    {				  
	result = ph_radr (host, adr);
	if (rp_isbad (result))
	    return (result);      /* get address from remote            */
	if (rp_gval (result) == RP_DONE)
	    break;		  /* end of address list                */

#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "'%s'@'%s'", adr, host);
#endif

	if (!flush)
	{
    	    if (rp_isbad (result = mm_wadr (host, adr)))
                return (result);      /* give to local submit           */

	    if (rp_isbad (result = mm_rrply (&thereply, &len)))
	        return (result);      /* how did submit like it?        */
        }
       switch (rp_gval (thereply.rp_val))
        			  /* was address acceptable?            */
       {
	    case RP_AOK: 	  /* address ok, text not yet sent      */
		ph_wrply (&thereply, len);
		numadrs++;
		break;

	    case RP_PARM:         /* bad return address--reject all     */
	        flush = TRUE;     /* "to" addresses                     */
	    case RP_NO: 	  /* remaining acceptible responses     */
		thereply.rp_val = RP_NDEL;
	    case RP_USER: 	  /* unknown user                       */
	    case RP_NDEL: 
	    case RP_AGN: 
	    case RP_NOOP: 
		ph_wrply (&thereply, len);
		break;

	    default: 		  /* responses which force abort        */
		ll_log (logptr, LLOGFAT,
			"reply err (%s)'%s'",
			rp_valstr (thereply.rp_val), thereply.rp_line);
		if (rp_isbad (thereply.rp_val))
		    return (thereply.rp_val);
		return (RP_RPLY);
	}
    }
    return (mm_waend ());        /* tell submit of address list end     */
}
/**/

LOCFUN
	p2m_txmng ()              /* send message text                  */
{
    static short  msgcount;
    struct rp_bufstruct thereply;
    int     len;
    short   result;
    char    buffer[BUFSIZ];

#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "doing text");
#endif
    printx ("%d", ++msgcount);    /* keep it pretty                     */
    fflush (stdout);
    msglen = 0;

    len = sizeof(buffer);
    while ((rp_gval (result = ph_rtxt (buffer, &len))) == RP_OK)
    {
	printx (".");		  /* to show watcher we're working      */
	fflush (stdout);
	msglen += len;
	if (rp_isbad (result = mm_wtxt (buffer, len)))
	    return (result);
	len = sizeof(buffer);
    }
    printx (" ");		  /* keep it pretty                     */
    fflush (stdout);
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "done sending");
#endif

    if (rp_isbad (result))
	return (result);	  /* problem with transmission          */
    if (rp_gval (result) != RP_DONE)
	return (RP_RPLY);	  /* protocol error in reply code       */

    if (rp_isbad (result = ph_rtend ()))
	return (result);	  /* flag end of message                */

    if (rp_isbad (result = mm_wtend ()))
	return (result);	  /* flag end of message                */

    if (rp_isbad (result = mm_rrply (&thereply, &len)))
	return (result);	  /* problem getting reply?             */
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "got text reply");
#endif

    switch (rp_gval (thereply.rp_val))
    {                             /* was text acceptable?               */
	case RP_OK: 
	    thereply.rp_val = RP_MOK;
	case RP_MOK: 		  /* text was accepted                  */
	    ph_wrply (&thereply, len);
	    break;

	case RP_NO: 		  /* remaining acceptible responses     */
	case RP_PARM:
	    thereply.rp_val = RP_NDEL;
	case RP_NDEL: 
	case RP_AGN: 
	case RP_NOOP: 
	    ph_wrply (&thereply, len);
	    thereply.rp_val = RP_OK;
	    break;                /* don't abort the process            */

	default: 		  /* responses which force abort        */
		ll_log (logptr, LLOGFAT,
			"reply err (%s)'%s'",
			rp_valstr (thereply.rp_val), thereply.rp_line);
	    if (rp_isbad (thereply.rp_val))
		return (thereply.rp_val);
	    return (RP_RPLY);
    }

    return (RP_OK);
}
