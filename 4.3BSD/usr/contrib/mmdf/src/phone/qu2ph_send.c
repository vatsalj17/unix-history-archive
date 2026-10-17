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
/*  CH_PHONE:  deliver-to-phone transmission                            */
/*                                                                      */

/*  Jun 81  D. Crocker      Add d2p_nadrs mechanism to send no msg text
 *                          when there are not valid addresses
 *  Jul 81  D. Crocker      Change state variable values, for more robust
 *                          state diagrams, to avoid hang with Deliver
 */


#include "ch.h"
#include "chan.h"
#include "phs.h"
#include "ap.h"

extern struct ll_struct  *logptr;
extern Chan *curchan;
extern char *strdup();
extern char *ap_p2s();
extern int  ap_outtype;
extern char *supportaddr;
extern char *mmdflogin;

/*#define RUNALON   */

LOCVAR int numadrs;                    /* number of valid addresses          */
LOCVAR char *sender = (char *)NULL;    /* return address for message         */
LOCVAR char *adr    = (char *)NULL;    /* recipient address                  */
LOCVAR struct rp_construct
			    rp_hend  =
{
    RP_NOOP, 'e', 'n', 'd', ' ', 'o', 'f', ' ', 'h', 'o', 's', 't', ' ',
    'i', 'g', 'n', 'o', 'r', 'e', 'd', '\0'
};

char ph_state = SND_RINIT;  /* state of processing current msg    */


qu2ph_send (chname)                 /* overall mngmt for batch of msgs    */
    char chname[];                /* name of channel we are             */
{
    short     result;
    char    info[LINESIZE],
	    sendbuf[ADDRSIZE];

    if (rp_isbad (result = qu_pkinit ()))
	return (result);

    if (rp_isbad (result = ph_sbinit ()))
    {
	printx ("remote site refuses to receive mail... ");
	return (result);
    }

    ph_state = SND_ABORT;
    for(;;){
	AP_ptr  loctree, domtree, routree, sendtree;
	AP_ptr  ap_s2tree(), ap_normalize();

	result = qu_rinit (info, sendbuf, curchan->ch_apout);
	if(rp_gval(result) == RP_NS){
	    qu_rend();
	    continue;
	}
	if(rp_gval(result) == RP_DONE)
	    break;

	if (rp_gval(result) == RP_FIO)          /* Can't open message file */
	    continue;
	else if (rp_gval(result) != RP_OK)      /* Some other error */
	    break;

	if (sender != (char *)NULL){
	    free (sender);
	    sender = NULL;
	}

	if ( sendbuf[0] == '\0' ||
	    (sendtree = ap_s2tree( sendbuf )) == (AP_ptr) NOTOK) {
		printx("return address unparseable, using Orphanage\n");
		fflush(stdout);
		if ((sendtree = ap_s2tree( supportaddr )) == (AP_ptr) NOTOK) {
		    printx("Orphanage unparseable, using MMDF\n");
		    fflush(stdout);
		} else
		if ((sendtree = ap_s2tree( mmdflogin )) == (AP_ptr) NOTOK) {
		    result = RP_PARM;
		    qu_rend();
		    goto bugout;
		}
	}
	ap_outtype = curchan -> ch_apout;
	sendtree = ap_normalize (curchan -> ch_lname,
			curchan  -> ch_ldomain, sendtree, curchan);
	if(sendtree == (AP_ptr)MAYBE){
	    result = RP_NS;
	    qu_rend();
	    goto bugout;
	}
	ap_t2parts (sendtree, (AP_ptr *)0, (AP_ptr *)0, &loctree,
			&domtree, &routree);
	sender = ap_p2s ((AP_ptr)0, (AP_ptr)0, loctree, domtree, routree);
	if(sender == (char *)MAYBE){
	    result = RP_NS;
	    qu_rend();
	    goto bugout;
	}
	ap_sqdelete( sendtree, (AP_ptr) 0 );
	ap_free( sendtree );

	ph_state = SND_RDADR;
	if (rp_isbad (result = ph_winit (chname, info, sender)))
	    return (result);      /* ready to process a message         */

	if (rp_isbad (result = q2p_admng ()))
	    return (result);      /* send the address list              */

	if (rp_gval (result) != RP_DONE)
	    break;                /* catch protocol errors              */

	if (rp_isbad (result = q2p_txmng ()))
	    return (result);      /* send the message text              */

	qu_rend();
    }
    qu_rend();

    if (rp_gval (result) != RP_DONE)
    {
	ll_log (logptr, LLOGTMP, "not DONE (%s)", rp_valstr (result));
	return (RP_RPLY);         /* catch protocol errors              */
    }

bugout:
    ph_state = SND_ABORT;
    qu_pkend ();                  /* done getting messages              */
    if (rp_isbad (result = ph_sbend ()))
				  /* done sending messages              */
	printx ("bad ending for outgoing submission... ");

    return (result);
}
/**/

LOCFUN
	q2p_admng ()              /* send address list                  */
{
    struct rp_bufstruct thereply;
    short     result;
    int       len;
    char    host[ADDRSIZE];
    char    adrbuf[ADDRSIZE];

    AP_ptr  loctree, domtree, routree, adrtree;
    AP_ptr  ap_s2tree(), ap_normalize();

    for (numadrs = 0; ; )         /* reset for each new messages        */
    {                             /* we already have and adr            */
	ph_state = SND_ABORT;
	result = qu_radr (host, adrbuf);
	if (rp_isbad (result))
	    return (result);      /* get address from Deliver           */

	if (rp_gval (result) == RP_HOK)
	{                         /* no-op the sub-list indication      */
	    qu_wrply ((struct rp_bufstruct *) &rp_hend, rp_conlen (rp_hend));
	    continue;
	}

	if (rp_gval (result) == RP_DONE)
	    break;                /* end of address list                */

	if (adr != (char *)NULL)
		free (adr);

	if ( (adrtree = ap_s2tree(adrbuf)) == (AP_ptr) NOTOK)  {
	    adr = strdup( adrbuf );
	} else {
	    ap_outtype = curchan -> ch_apout;
	    adrtree = ap_normalize (curchan -> ch_lname,
				curchan  -> ch_ldomain, adrtree, curchan);
	    if(adrtree == (AP_ptr)MAYBE){
		result = RP_NS;
		goto bugout;
	    }
	    ap_t2parts (adrtree, (AP_ptr *)0, (AP_ptr *)0, &loctree,
			&domtree, &routree);
	    adr = ap_p2s ((AP_ptr)0, (AP_ptr)0, loctree, domtree, routree);
	    if(adr == (char *)MAYBE){
		result = RP_NS;
		goto bugout;
	    }
	    ap_sqdelete( adrtree, (AP_ptr) 0 );
	    ap_free( adrtree );
	}

	ph_state = SND_ARPLY;
	if (rp_isbad (result = ph_wadr (host, adr)))
	    return (result);      /* give to remote site                */

	if (rp_isbad (result = ph_rrply (&thereply, &len)))
	    return (result);      /* how did remote like it?            */

bugout:
	switch (rp_gval (thereply.rp_val))
	{                         /* was address acceptable?            */
	    case RP_AOK:          /* address ok, text not yet sent      */
		numadrs++;
		ph_state = SND_ABORT;
		qu_wrply (&thereply, len);
		break;

	    case RP_NO:           /* remaining acceptible responses     */
		thereply.rp_val = RP_NDEL;
	    case RP_NS:
	    case RP_USER:
	    case RP_NDEL:
	    case RP_AGN:
	    case RP_NOOP:
		ph_state = SND_ABORT;
		qu_wrply (&thereply, len);
		break;

	    default:              /* responses which force abort        */
		if (rp_isbad (thereply.rp_val))
		    return (thereply.rp_val);
		return (RP_RPLY);
	}
    }
    ph_state = SND_TRPLY;
    return (ph_waend ());        /* tell remote of address list end    */
}
/**/

LOCFUN
	q2p_txmng ()              /* send message text                  */
{
    struct rp_bufstruct thereply;
    int     len;
    short   result;
    char    buffer[BUFSIZ];
    static char faktxt[] = "   ";       /* to keep dial happy   */

/*  the main portion handles normal transmission, when there have been
 *  some addresses accepted.  when no addresses have been accepted, then
 *  the text will be ignored by the receiving side.  due to a problem
 *  with the dial package, you must send some text.  this is handled
 *  in the 'else' condition.
 */
    printx ("\tsending text...:");
    fflush (stdout);
    if (numadrs > 0)            /* really need to send the text       */
    {
	len = sizeof(buffer);
	for (qu_rtinit (0L);
		(rp_gval (result = qu_rtxt (buffer, &len))) == RP_OK; )
	{
	    printx (".");         /* to show watcher we're working      */
	    fflush (stdout);
	    if (rp_isbad (result = ph_wtxt (buffer, len)))
		return (result);                         
	    if (rp_gval (result) != RP_OK)
		return (RP_RPLY);
	    len = sizeof(buffer);
	}
	printx (" ");             /* keep it pretty                     */
	fflush (stdout);

	if (rp_isbad (result))
	    return (result);
	if (rp_gval (result) != RP_DONE)
	    return (RP_RPLY);     /* didn't get it all across?          */
    }
    else                          /* send some text to keep dial happy  */
    {
	if (rp_isbad (result = ph_wtxt (faktxt, (sizeof faktxt) - 1)))
	    return (result);
	if (rp_gval (result) != RP_OK)
	    return (RP_RPLY);
    }


    if (rp_isbad (result = ph_wtend ()))
	return (result);          /* flag end of message                */

    if (rp_isbad (result = ph_rrply (&thereply, &len)))
	return (result);          /* problem getting reply?             */

    switch (rp_gval (thereply.rp_val))
    {                             /* was text acceptable?               */
	case RP_OK: 
	    thereply.rp_val = RP_MOK;
	case RP_MOK:              /* text was accepted                  */
	    qu_wrply (&thereply, len);
	    break;

	case RP_NO:               /* remaining acceptible responses     */
	    thereply.rp_val = RP_NDEL;
	case RP_NDEL: 
	case RP_AGN: 
	case RP_NOOP: 
	    qu_wrply (&thereply, len);
	    thereply.rp_val = RP_OK;
	    break;                /* don't abort process                */

	default:                  /* responses which force abort        */
	    if (rp_isbad (thereply.rp_val))
		return (thereply.rp_val);
	    return (RP_RPLY);
    }
    return (thereply.rp_val);     /* just quote remote                  */
}
