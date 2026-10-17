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
/*                  SEND FROM PHONE TO LOCAL (SUBMIT)                   */

#include "ch.h"

extern struct ll_struct   *logptr;

mm2ph_send (curchan)                /* overall mngmt for batch of msgs    */
    Chan *curchan;                /* channel being picked up            */
{
    short     result;
    char    info[LINESIZE],
            sender[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mm2ph_send (%s)", curchan -> ch_name);
#endif

    if (rp_isbad (result = mm_pkinit (curchan -> ch_name)))
	return (result);
    if (rp_isbad (result = ph_sbinit ()))
	return (result);

    while (rp_gval (result = mm_rinit (info, sender)) == RP_OK)
    {
#ifdef DEBUG
	ll_log (logptr, LLOGPTR, "new message");
#endif
	if (rp_isbad (result = ph_winit (curchan -> ch_name, info, sender)))
	    return (result);      /* ready to process a message         */

	if (rp_isbad (result = m2p_admng ()))
	    return (result);      /* send the address list              */

	if (rp_isbad (result = m2p_txmng ()))
	    return (result);      /* send the message text              */
    }
#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "done w/msgs");
#endif

    mm_pkend ();                  /* done getting messages              */
    ph_sbend ();                  /* done sending messages              */

    switch (rp_gval (result))
    {
	case RP_DONE: 
	case RP_EOF: 
	    return (RP_OK);
    }
    return (result);
}
/**/

LOCFUN
	m2p_admng ()              /* send address list                  */
{
    struct rp_bufstruct thereply;
    int       len;
    short     result;
    char    host[LINESIZE];
    char    adr[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "doing adrs");
#endif

    FOREVER			  /* iterate thru list                    */
    {				  /* we already have and adr              */
#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "new addr");
#endif
	result = mm_radr (host, adr);
	if (rp_isbad (result))
	    return (result);      /* get address from Deliver           */
	if (rp_gval (result) == RP_DONE)
	    break;		  /* end of address list                */

	if (rp_isbad (result = ph_wadr (host, adr)))
	    return (result);      /* give to remote site                */

	if (rp_isbad (result = ph_rrply (&thereply, &len)))
	    return (result);      /* how did remote like it?            */

	switch (rp_gval (thereply.rp_val))
				  /* was address acceptable?            */
	{
	    case RP_AOK: 	  /* address ok, text not yet sent      */
		mm_wrply (&thereply, len);
		break;

	    case RP_NO: 	  /* remaining acceptible responses     */
		thereply.rp_val = RP_NDEL;
	    case RP_USER: 
	    case RP_NDEL: 
	    case RP_AGN: 
	    case RP_NOOP: 
		mm_wrply (&thereply, len);
		break;

	    default: 		  /* responses which force abort        */
		ll_log (logptr, LLOGFAT,
			"adr eof reply err (%s)'%s'",
			rp_valstr (thereply.rp_val), thereply.rp_line);
		if (rp_isbad (thereply.rp_val))
		    return (thereply.rp_val);
		return (RP_RPLY);
	}
    }
    return (ph_waend ());        /* tell remote of address list end    */
}
/**/

LOCFUN
	m2p_txmng ()              /* send message text                  */
{
    struct rp_bufstruct thereply;
    char    buffer[BUFSIZ];
    int     len;
    short     result;

#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "doing text");
#endif
    len = sizeof(buffer);
    while ((rp_gval (result = mm_rtxt (buffer, &len))) == RP_OK) {
	if (rp_isbad (result = ph_wtxt (buffer, len)))
	    return (result);
	len = sizeof(buffer);
    }
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "done sending");
#endif

    if (rp_isbad (result))
	return (result);	  /* problem with transmission          */
    if (rp_gval (result) != RP_DONE)
	return (RP_RPLY);	  /* protocol error in reply code       */
    if (rp_isbad (result = ph_wtend ()))
	return (result);	  /* flag end of message                */
    if (rp_isbad (result = ph_rrply (&thereply, &len)))
	return (result);	  /* problem getting reply?             */

    switch (rp_gval (thereply.rp_val))
				  /* was text acceptable?               */
    {
	case RP_OK: 
	    thereply.rp_val = RP_MOK;
	case RP_MOK: 		  /* text was accepted                  */
	    mm_wrply (&thereply, len);
	    break;

	case RP_NO: 		  /* remaining acceptible responses     */
	    thereply.rp_val = RP_NDEL;
	case RP_NDEL: 
	case RP_AGN: 
	case RP_NOOP: 
	    mm_wrply (&thereply, len);
	    thereply.rp_val = RP_OK;
	    break;                /* don't abort the process            */

	default: 		  /* responses which force abort        */
	    ll_log (logptr, LLOGFAT,
		    "text eof reply err (%s)'%s'",
		    rp_valstr (thereply.rp_val), thereply.rp_line);
	    if (rp_isbad (thereply.rp_val))
		return (thereply.rp_val);
	    return (RP_RPLY);
    }
    return (RP_OK);
}
