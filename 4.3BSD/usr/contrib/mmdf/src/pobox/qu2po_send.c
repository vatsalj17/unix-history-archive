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

/*              SEND FROM DELIVER OVER TO CALLER (PASSIVE)              */

#include "ch.h"
#include "chan.h"
#include <pwd.h>
#include "ap.h"

extern struct ll_struct    *logptr;

extern char *mmdflogin;
extern char *supportaddr;

extern char *strdup();
extern char *ap_p2s();

extern int ap_outtype;

LOCVAR char *sender = (char *)NULL;    /* return address for message         */
LOCVAR char *adr    = (char *)NULL;    /* recipient address                  */
LOCVAR struct rp_construct
			    rp_hend  =
{
    RP_NOOP, 'e', 'n', 'd', ' ', 'o', 'f', ' ', 'h', 'o', 's', 't', ' ',
    'i', 'g', 'n', 'o', 'r', 'e', 'd', '\0'
};
LOCVAR struct rp_construct
			    rp_skip =
{
    RP_NOOP, 's', 'k', 'i', 'p', ' ', 't', 'h', 'i', 's', ' ',
    'a', 'd', 'd', 'r', 'e', 's', 's', '\0'
};

LOCVAR Chan *q2p_chan;            /* what channel am I?                 */

LOCVAR int    q2p_uid;            /* user id of caller                  */

LOCVAR unsigned short q2p_nadrs;  /* number of valid addresses          */

LOCVAR char *q2p_info;            /* initialization info for message    */

LOCVAR char q2p_username[USERSIZE + 1];
				  /* name of caller                     */
LOCVAR char q2p_out;              /* at least one adr sent              */
char po_state = SND_RINIT;/* state of processing current msg    */
/**/

qu2po_send (chname)               /* overall mngmt for batch of msgs    */
    char chname[];                /* name of channel we are             */
{
    extern char *strdup ();
    short     result;
    char    info[LINESIZE],
	    sendbuf[ADDRSIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu2po_send");
#endif

    q2p_gcinfo ();                /* get info on the caller             */

    q2p_chan = ch_nm2struct (chname);
    if (q2p_chan == (Chan *) NOTOK)
	err_abrt (RP_PARM, "unknown channel name '%s'", chname);
				  /* find out who I am                  */

    if (q2p_chan -> ch_login != 0)  /* only one login may pickup chan     */
    {
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "%s =?= %s",
		    q2p_username, q2p_chan -> ch_login);
#endif
	if (!lexequ (q2p_username, q2p_chan -> ch_login) &&
	    !lexequ (q2p_username, mmdflogin))
	{
	    ll_log (logptr, LLOGFAT,
		    "'%s' not authorized to pickup for %s (%s)",
		    q2p_username, q2p_chan -> ch_table -> tb_name, 
		    q2p_chan -> ch_name);
	    return (RP_USER);     /* caller not authorized to pickup    */
	}
    }

    if (rp_isbad (result = qu_pkinit ()))
	return (result);

    if (rp_isbad (result = po_sbinit ()))
	return (result);

    po_state = SND_ABORT;
    for(;;){
	AP_ptr  loctree, domtree, routree, sendtree;
	AP_ptr  ap_s2tree(), ap_normalize();

	result = qu_rinit (info, sendbuf, q2p_chan->ch_apout);
	if(rp_gval(result) == RP_NS){
	    qu_rend();
	    continue;
	}
	else if(rp_gval(result) == RP_DONE)
	    break;

	if (rp_gval(result) == RP_FIO)          /* Can't open message file */
		continue;
	else if (rp_gval(result) != RP_OK)      /* Some other error */
		break;

	q2p_info = strdup (info); /* not sent until an address goes out */

	if (sender != (char *)NULL){
		free (sender);
		sender = NULL;
	}

	if ( sendbuf[0] == '\0' ||
	    (sendtree = ap_s2tree( sendbuf )) == (AP_ptr) NOTOK) {
		printx("return address unparseable, using Orphanage\n");
		fflush(stdout);
		sender = strdup (supportaddr);
	} else {
		ap_outtype = q2p_chan -> ch_apout;
		sendtree = ap_normalize (q2p_chan -> ch_lname,
			       q2p_chan  -> ch_ldomain, sendtree, q2p_chan);
		if(sendtree == (AP_ptr)MAYBE)
		    return(RP_NS);
		ap_t2parts (sendtree, (AP_ptr *)0, (AP_ptr *)0, &loctree, &domtree, &routree);
		sender = ap_p2s ((AP_ptr)0, (AP_ptr)0, loctree, domtree, routree);
		if(sender == (char *)MAYBE)
			return(RP_NS);
		ap_sqdelete( sendtree, (AP_ptr) 0 );
		ap_free( sendtree );
	}

	po_state = SND_RDADR;
	if (rp_isbad (result = q2p_admng ()))
	    return (result);      /* send the address list              */

	if (rp_gval (result) != RP_DONE)
	    break;                /* catch protocol errors              */

	result = q2p_txmng ();
	free (q2p_info);          /* don't have to zero                 */
	if (rp_isbad (result))
	    return (result);      /* send the message text              */
    }

    if (rp_gval (result) != RP_DONE)
    {
	ll_log (logptr, LLOGTMP, "not DONE (%s)", rp_valstr (result));
	return (RP_RPLY);         /* catch protocol errors              */
    }

    po_state = SND_ABORT;
    qu_pkend ();                  /* done getting messages              */
    return (po_sbend ());         /* done sending messages              */
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


#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "q2p_admng");
#endif

    q2p_nadrs = 0;                /* reset for each new messages        */
    q2p_out = FALSE;
    FOREVER                       /* iterate thru list                  */
    {                             /* we already have and adr            */
	po_state = SND_ABORT;
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

	po_state = SND_ARPLY;
	if (!q2p_mayadr (host))
	{                         /* not authorized to go out           */
	    qu_wrply ((struct rp_bufstruct *) &rp_skip, rp_conlen (rp_skip));
	    continue;
	}

	if (!q2p_out)             /* first one to go out                */
	{
	    q2p_out = TRUE;
	    if (rp_isbad (result =
			    po_winit ((char *) 0, q2p_info, sender)))
		return (result);
	}

	if (adr != (char *)NULL)
		free (adr);

	if ( (adrtree = ap_s2tree(adrbuf)) == (AP_ptr) NOTOK)  {
		adr = strdup( adrbuf );
	} else {
		ap_outtype = q2p_chan -> ch_apout;
		adrtree = ap_normalize (q2p_chan -> ch_lname,
				q2p_chan  -> ch_ldomain, adrtree, q2p_chan);
		if(adrtree == (AP_ptr)MAYBE)
		    return(RP_NS);
		ap_t2parts (adrtree, (AP_ptr *)0, (AP_ptr *)0, &loctree, &domtree, &routree);
		adr = ap_p2s ((AP_ptr)0, (AP_ptr)0, loctree, domtree, routree);
		if(adr == (char *)MAYBE)
		    return(RP_NS);
		ap_sqdelete( adrtree, (AP_ptr) 0 );
		ap_free( adrtree );
	  }

	if (rp_isbad (result = po_wadr (host, adr)))
	    return (result);      /* give to remote site                */

	if (rp_isbad (result = po_rrply (&thereply, &len)))
	    return (result);      /* how did remote like it?            */

	switch (rp_gval (thereply.rp_val))
	{                         /* was address acceptable?            */
	    case RP_AOK:          /* address ok, text not yet sent      */
		q2p_nadrs++;
		po_state = SND_ABORT;
		qu_wrply (&thereply, len);
		break;

	    case RP_NO:           /* remaining acceptible responses     */
		thereply.rp_val = RP_NDEL;
	    case RP_USER: 
	    case RP_NDEL: 
	    case RP_AGN: 
	    case RP_NOOP: 
		po_state = SND_ABORT;
		qu_wrply (&thereply, len);
		break;

	    default:              /* responses which force abort        */
		if (rp_isbad (thereply.rp_val))
		    return (thereply.rp_val);
		return (RP_RPLY);
	}
    }
    po_state = SND_TRPLY;
    if (q2p_out)                  /* at least one went out              */
	return (po_waend ());    /* tell remote of address list end    */
    else
	return (RP_DONE);
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
 *  the text will be ignored by the receiving side.
 */
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "q2p_txmng");
#endif

    if (!q2p_out)
    {                             /* no addresses tried                 */
	ll_log (logptr, LLOGGEN, "no addrs sent");
	qu_wrply ((struct rp_bufstruct *) &rp_skip, rp_conlen (rp_skip));
	return (RP_OK);
    }

    if (q2p_nadrs == 0)           /* no valid addresses                 */
    {                             /* send some text to keep things happy*/
	if (rp_isbad (result = po_wtxt (faktxt, (sizeof faktxt) - 1)))
	    return (result);
	if (rp_gval (result) != RP_OK)
	    return (RP_RPLY);
    }
    else
    {
	qu_rtinit (0L);           /* get ready to read text             */
    	len = sizeof(buffer);
	while ((rp_gval (result = qu_rtxt (buffer, &len))) == RP_OK)
	{
	    if (rp_isbad (result = po_wtxt (buffer, len)))
		return (result);
	    if (rp_gval (result) != RP_OK)
		return (RP_RPLY);
	    len = sizeof(buffer);
	}

	if (rp_isbad (result))
	    return (result);
	if (rp_gval (result) != RP_DONE)
	    return (RP_RPLY);     /* didn't get it all across?          */
    }


    if (rp_isbad (result = po_wtend ()))
	return (result);          /* flag end of message                */

    if (rp_isbad (result = po_rrply (&thereply, &len)))
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
	    break;

	default:                  /* responses which force abort        */
	    if (rp_isbad (thereply.rp_val))
		return (thereply.rp_val);
	    return (RP_RPLY);
    }
    return (thereply.rp_val);     /* just quote remote                  */
}
/**/

LOCFUN
	q2p_gcinfo ()             /* who is the caller?                 */
{
    struct passwd *getpwuid ();
    int   effecid;
    register struct passwd  *pwdptr;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "q2p_gcinfo ()");
#endif

    getwho (&q2p_uid, &effecid);
    pwdptr = getpwuid (q2p_uid);

    strcpy (q2p_username, pwdptr -> pw_name);
}
/**/

LOCFUN
	q2p_mayadr (host)         /* may caller get this message?       */
char    host[];                   /* destination host                   */
{
    char    adrline[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "q2p_mayadr");
#endif

/* PICKUP POLICY:   Who may pickup messages?
 *
 *  Two situations will be most common:  pickup net & relay
 *
 *  A "pickup net" is a network of callers who each will have
 *  mail to pickup.  Each caller is a single recipient (machine).
 *  Hence, a "host" equates to a single receiver (machine).  The
 *  hostname, therefore, must equal the name (unix login name) of
 *  the caller.
 *
 *  A "relay" is a single caller, retrieving mail for a set of
 *  hosts.  Here, the hostname has nothing to do with the
 *  caller's id and is part of the address that is passed on.
 *  (For local, arpanet, and pickup-net mail, the hostname part
 *  of the address is not passed on to the recipient.)  In this
 *  case, the id of the authorized caller is hard-wired into a
 *  variable.
 *
 *  The authorization test, for relaying, is done in qu2ph_send's
 *  startup.  The test for pickup-net is done in this routine.
 */

    if (q2p_chan -> ch_login == 0)
    {                             /* login => host, not chan            */
	if (tb_k2val (q2p_chan -> ch_table, TRUE, host, adrline)!=OK)
	{
	    ll_log (logptr, LLOGTMP, "pobox host '%s' not in table!", host);
	    return (FALSE);       /* map name -> address                  */
	}

#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "%s =?= %s", adrline, q2p_username);
#endif

	if (!lexequ (adrline, q2p_username))
	    return (FALSE);       /* "address" == this user's id?         */
    }

    return (TRUE);                /* OK to send                           */
}
