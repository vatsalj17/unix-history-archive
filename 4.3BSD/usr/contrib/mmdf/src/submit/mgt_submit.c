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
#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include <pwd.h>
#include "cnvtdate.h"
#include "ap.h"
#include "dm.h"
#include "msg.h"
#include "hdr.h"
#include "ns.h"

/*  enforce management policies on message submission   */

/*  currently, we are only concerned about author authenticity and
 *  format conformance.
 *
 *  Jun 82  D. Crocker      add adruid, for proc/file address protection
 *  Jul 82  D. Crocker      add mgt_hops, checking for looping
 *                          fix use of From, over Sender, for return addr
 *                          add use of Reply-to, over From, fro return addr
 *  Jul 82  A. Chang (suny) mgt_author require mbox==username
 */

extern Chan **ch_tbsrch;           /* complete list of channels          */
extern FILE *mq_mffp;
extern char *ch_dflnam;            /* default channel name             */

extern struct passwd *getpwmid ();
extern struct ll_struct *logptr;
extern char *index ();
extern char *multcat();
extern Chan *ch_nm2struct();
extern Domain *dm_v2route();

extern long tx_msize;             /* char count of message text         */

extern short pro_vfadr;

extern int    effecid,
	    userid,
	    adruid,
	    maxhops,
	    mgt_addid,
	    dlv_flgs;

extern char *adr_orgspec;         /* original mailbox specification     */
extern char *adr_fulldmn;	  /* used for the delay channel 	*/
extern char *blt();
extern char *compress ();

short       lnk_nadrs;            /* number of addressees for message   */

extern char *mq_munique,          /* name of the message                */
	    *prm_dupval (),
	    *strdup (),
	    *locmachine,
	    *locname,
	    *locdomain,
	    nlnultrm[],           /* termination break-set for ut_stdin */
	    *mailid,
	    *username;

char *mgt_txsrc = NULL;           /* some text for Source-Info:         */
char *mgt_return = NULL;          /* return address                     */
int	mgt_gotid;		  /* have seen a message-id: field	*/
int     mgt_inalias;              /* is address in alias file           */

short   mgt_s2return = FALSE;     /* get return from Sender             */

char	mgt_dlname[] = "delay";	  /* name of the delay channel		*/
int     mgt_nodelay = FALSE;      /* don't use delay channel if true    */
int     mgt_amdelay = FALSE;      /* I really am the delay channel      */

LOCVAR struct ch_hstchan
{
    Chan *mgt_achan;              /* channel relay coming in on         */
    char *mgt_ahost;              /* official host name on channel      */
}   mgt_vchan;                    /* caller is coming in from chan x    */

LOCVAR Chan *mgt_chdfl;
LOCVAR short   mgt_trust,        /* pass on author authentication?     */
#define NRMFROM 0                 /*    normal checking                 */
#define FLGFROM 1                 /*    flag as unauthenticated         */
#define OKFROM  2                 /*    ignore validity                 */
	mgt_rt2sndr,              /* if a return mail message is needed */
				  /*    send it to submittor            */
	mgt_rtgot,                /* got the return address             */
	mgt_remail,               /* this was remailed                  */
	mgt_okauthor,             /* an author field was acceptable     */
	mgt_fmcnt,                /* number of from fields seen         */
	mgt_sncnt,                /* number of sender fields seen       */

	mgt_doloc,                /* send "immediate" channels, if any  */
	mgt_donet,                /* send "network" channels, if any    */
	mgt_loops,                /* number of times through this site  */
	mgt_hops;                 /* number of hops message has gone    */
/**/

mgt_init ()
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mgt_init ()");
#endif
    if (ch_dflnam[0] != '\0')
	if ((mgt_chdfl = ch_nm2struct (ch_dflnam)) == (Chan *) NOTOK)
	    err_abrt (RP_PARM, "'%s' invalid default channel name", ch_dflnam);

    return (RP_OK);
}

mgt_minit ()                      /* initialize for new message         */
{
    register Chan **chanptr;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mgt_minit ()");
#endif

    if (mgt_txsrc)                /* if previously set                  */
	free (mgt_txsrc);
    if (mgt_vchan.mgt_ahost)      /*  "     "       "                   */
	free (mgt_vchan.mgt_ahost);
    if (mgt_return)
	free (mgt_return);

    mgt_vchan.mgt_achan =  (Chan *) 0;
    mgt_vchan.mgt_ahost =  (char *) 0;
    mgt_txsrc = (char *) 0;
    mgt_return = (char *) 0;

    mgt_trust = NRMFROM;

    mgt_remail =
	mgt_rt2sndr =
	mgt_s2return =
	mgt_rtgot =
	mgt_okauthor = FALSE;
    mgt_fmcnt =
	mgt_sncnt =
	mgt_gotid =
	mgt_loops =
	mgt_hops = 0;

    for (chanptr = ch_tbsrch; *chanptr != 0; chanptr++)
	(*chanptr) -> ch_access &= ~DLVRDID;
				  /* reset "used" bit, for channel      */
    return (RP_OK);
}
/**/

char *
	mgt_parm (theparm)
register char *theparm;
{
    char *ptr;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mgt_parm ()");
#endif
    switch (*theparm)
    {
	case '-':
	    break;

	case 'h':                 /* hostname for relay source          */
	    theparm = prm_dupval (++theparm, &mgt_vchan.mgt_ahost);
	    break;                /* just save the info, for now        */

	case 'i':                 /* relaying Indirectly                */
	    theparm = prm_dupval (++theparm, &ptr);
	    mgt_forward (ptr);
	    free (ptr);
	    break;                /* certify legality of relaying       */

	case 'l':                 /* send local now, if have any        */
	    mgt_doloc = TRUE;
	    break;

	case 'n':                 /* send net now, if have any          */
	    mgt_donet = TRUE;
	    break;

	case 'r':                 /* get RETURN from login info         */
	    mgt_rt2sndr = TRUE;   /*   and not explicit specification   */
	    break;

	case 's':                 /* get RETURN from Sender info        */
	    mgt_s2return = TRUE;
	    break;

	case 't':                 /* TRUST me; don't authenticate       */
	    mgt_trust = OKFROM;
	    break;

	case 'u':                 /* (close to 't'); no authentication  */
	    mgt_trust = FLGFROM;
	    break;

	case 'd':                 /* don't use delay channel            */
	    mgt_nodelay = TRUE;
	    break;

	case 'j':                 /* I really am the delay channel      */
	    mgt_amdelay = TRUE;
	    mgt_nodelay = TRUE;
	    break;

	case 'k':
	    theparm = prm_dupval (++theparm, &ptr);
#ifdef NAMESERVER
	    ns_settimeo(atoi(ptr));
#endif
	    free(ptr);
	    break;

	default:
	    return ((char *) NOTOK);    /* not a management parameter   */
    }

    return (theparm);
}
/**/

mgt_pend ()                       /* end of parameters                  */
{
    AP_ptr rtntree;
    AP_ptr local, domain, route;
    char rtnbuf[ADDRSIZE];
    register short len;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mgt_pend ()");
#endif

    mgt_source();

    if (mgt_rt2sndr)              /* Use sender for return address      */
    {
	mgt_return = multcat(mailid, "@", locname, ".", locdomain, (char *)0);
	mgt_rtgot = TRUE;
    }
    else {                         /* submittor specifying return        */
	if (!mgt_s2return)         /* take msg directly                  */
	{
	    register char *cp;

	    if ((len = ut_stdin (rtnbuf, LINESIZE, nlnultrm, NOTOK)) <= 0)
		err_abrt (RP_LIO, "error copying return address");

	    /* Suck up leading white space */
	    for (cp = rtnbuf; isspace(*cp); cp++);

	    if (*cp == '\n')
		*cp = NULL;
	    if (*cp == NULL)
		/*   Special Case:  Null return address --> Quiet flag */
		dlv_flgs |= ADR_NORET;
	    else if (len > (sizeof(rtnbuf) - 1))
		err_msg (RP_LIO, "return address too long");
	    else if ((rtntree = ap_s2tree (cp)) == (AP_ptr) NOTOK)
		err_msg (RP_PARM, "Problem with return address '%s'", rtnbuf);
	    else {
		rtntree = ap_normalize (mgt_vchan.mgt_ahost, (char *) 0,
							rtntree, (Chan *)0);
		if(rtntree == (AP_ptr)MAYBE)
		    err_msg (RP_NS, "Nameserver timeout for '%s'", rtnbuf);
		ap_t2parts(rtntree, (AP_ptr *)0, (AP_ptr *)0, &local,
							&domain, &route);
		mgt_return = ap_p2s( (AP_ptr) 0, (AP_ptr) 0, local, domain,
								route);
		ap_sqdelete (rtntree, (AP_ptr) 0);
		ap_free (rtntree);   /* delete full string                 */
		if(mgt_return == (char *)MAYBE)
		    err_msg (RP_NS, "Nameserver timeout for '%s'", rtnbuf);
		mgt_rtgot = TRUE;
	    }
	}
    }

    if (mgt_vchan.mgt_achan == (Chan *) 0)
	auth_init (username, (mgt_trust == NRMFROM));
    else
	auth_init ((mgt_return ? mgt_return : ""), (mgt_trust == NRMFROM));

    if(pro_vfadr)
	pro_reply (RP_OK, "Sender is '%s'", mgt_return ? mgt_return : "");
#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "mgt_return: '%s'", mgt_return ? mgt_return : "");
#endif
}
/**/

#define AOKNUM 50

mgt_aok (thechan, hostr, mbox, parm) /* is address acceptable for sending  */
Chan *thechan;                    /* internal chan name/code            */
char *hostr,                      /* official name of host              */
     *mbox,                       /* name of mailbox                    */
     *parm;                       /* local-mailbox parameter            */
{
    register struct passwd *pwdptr;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mgt_aok (%s, %s, %s, %s)", thechan->ch_name,
		hostr, mbox, parm);
#endif

/* if address maps to file or process, login uid must meet the following
 * restrictions.
 */

    if (thechan == mgt_chdfl)    /* "local" address              */
    {
	switch (parm[0]) 
	{
	case '/':           /* file destination                 */
	case '|':           /* process destination              */
	    if ((pwdptr = getpwmid (mbox)) == (struct passwd *) NULL)
	    {
		ll_err (logptr, LLOGTMP,
			    "user '%s' not in passwd file", mbox);
		return (FALSE); /* can't deliver to non-persons */
	    }
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "caller uid (%d), parm uid (%d)",
			adruid, pwdptr -> pw_uid);
#endif
	    if (!mgt_inalias)
				/* only accept from alias file    */
				/* unless 'insecure'              */
#ifdef INSECURE
		/*
		 *  Only do this code if you are willing to take the
		 *  implied security risk.  I do not trust this (DPK).
		 *
		 *  Must be ((your own login id or privelidged user)
		 *  and locally originated) (SEK - note de Morgan!!)
		 */
		if (((adruid != pwdptr -> pw_uid) && (adruid != 0)) ||
			(mgt_vchan.mgt_achan != (Chan *) 0))
#endif
			return (FALSE);
	}
    }

/* is this address subject to immediate transmission, if requested?
 * the check, here, is more for convenience than for enforcement,
 * since Deliver will have to redo the check.  this is just to
 * avoid calling Deliver unnecessarily.
 */

    if (! (thechan -> ch_access & (DLVRPSV | DLVRBAK)))
    {                             /* an active, foreground-able chan?   */
	if ((thechan -> ch_access & DLVRIMM) || mgt_donet)
	    thechan -> ch_access |= DLVRDID;  /* channel may be run now */
    }

    if (mgt_vchan.mgt_achan == (Chan *) 0)
	return (auth_user (mgt_chdfl -> ch_lname, hostr,
						mgt_chdfl, thechan));
    else
	return (auth_user (mgt_vchan.mgt_ahost, hostr,
					mgt_vchan.mgt_achan, thechan));
				/* apply control policies relating to */
				/* channel access                     */
}

mgt_aend ()
{                                 /* record some stats                  */
    char sizstr[11];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mgt_aend ()");
#endif

    if (lnk_nadrs <= 0)
	err_msg (RP_NDEL, "No valid addresses");

    sprintf (sizstr, "%ld", tx_msize);
				  /* ll_log can't handle longs          */
    if (mgt_vchan.mgt_achan == 0) /* local submission                   */
	ll_log (logptr, LLOGBST, "lin %s (%d, %s) %s %s",
		mq_munique, lnk_nadrs, sizstr, mgt_chdfl->ch_queue, mailid);
    else
    {                             /* claiming to be a relay             */
	if (!mgt_s2return &&       /* hack past possible bad parsing     */
	    mgt_return != NULL && !isnull (*mgt_return) &&
	    !index(mgt_return, '@'))/* add host if necessary if not given */
	{
	    char *cp;

	    cp = multcat(mgt_return, "@", mgt_vchan.mgt_ahost, (char *)0);
	    free (mgt_return);
	    mgt_return = cp;
	}

	ll_log (logptr, LLOGBST, "rin %s (%d, %s) %s %s %s",
		mq_munique, lnk_nadrs, sizstr,
		mgt_vchan.mgt_achan -> ch_queue, mgt_vchan.mgt_ahost,
		mgt_return);

    }
    auth_end ();
    return (RP_OK);
}

/**/

mgt_source ()                     /* ready to process headers */
{
    char linebuf[LINESIZE];

    if (mgt_vchan.mgt_achan != 0)
    {
	if (mgt_vchan.mgt_ahost == 0)
	{                         /* no source host named               */
	    if (mgt_vchan.mgt_achan -> ch_host == NORELAY)
	    {                     /* not a single relay for the chan    */
		if(mgt_amdelay && PRIV_USER(userid))
			mgt_vchan.mgt_ahost = strdup(adr_fulldmn);
		else	/* cheat here should do an err_abrt with RP_NS ?? */
		    if (tb_k2val (mgt_vchan.mgt_achan -> ch_table, TRUE,
			 	username, linebuf) != OK)
				  /* derive it from login name          */
			err_abrt(RP_PARM,"'%s' not an authorized %s (%s) relay",
				username,
				mgt_vchan.mgt_achan -> ch_table -> tb_name,
				username, mgt_vchan.mgt_achan -> ch_name);
		else 
		    mgt_vchan.mgt_ahost = strdup (linebuf);
	    }
	    else			/* take name from ch_tai structure    */
		mgt_vchan.mgt_ahost = strdup (mgt_vchan.mgt_achan -> ch_host);
	}
	else if (!PRIV_USER(userid))	/* trying to name host explicitly     */
		err_abrt (RP_PARM,
			    "only root, mmdf and daemons may name source host");
    }
}

mgt_hinit ()                     /* ready to process headers */
{
    if (mgt_vchan.mgt_achan != 0)
	mgt_via();
}

/*
 *  Authentication of "Resent" messages is quit a bit looser than
 *  authentication of original messages.  This is due to the fact
 *  that a message might be resent several times, each time gaining
 *  a new set of Resent- headers.  The last set wins.   (DPK, 17 July 84)
 */
mgt_hdr (name, contents, hdr_state)
    char *name,
	contents[];
    int hdr_state;
{
    int ind,
	argc;
    char *argv[25];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mgt_hdr ()");
#endif

    if (prefix ("Resent-", name))
    {
	name += 7;
	goto doremail;
    }
    if (prefix ("Remailed-", name))
    {
	name += 9;
	goto doremail;
    }
    if (prefix ("Redistributed-", name))
    {
	name += 14;
doremail:
	if (!mgt_remail)
	{
	    mgt_sncnt = mgt_fmcnt = 0;
	    mgt_remail = TRUE;
	}
	if (lexequ ("Sender", name))
	    goto dosn;
	if (lexequ ("From", name))
	    goto dofm;
	if (lexequ ("Reply-to", name))
	    goto dorplyto;
    }

    if (lexequ ("Sender", name))
    {
	if (!mgt_remail)
	{
dosn:
	    mgt_sncnt++;
	    if (mgt_trust == NRMFROM)
		mgt_okauthor = mgt_author (contents);

	    if (mgt_s2return) {   /* use Sender field for return addr   */
		compress (contents, contents);
		mgt_return = strdup(contents);
		mgt_rtgot = TRUE;
	    }

	}
    }
    else
    if (lexequ ("From", name))
    {
	if (!mgt_remail)
	{
dofm:
	    mgt_fmcnt++;

	    /* Sender overrides; contents of From     */
	    if (mgt_trust == NRMFROM)
		if (mgt_remail || mgt_sncnt == 0)
		    mgt_okauthor = mgt_author (contents);

	    if (mgt_s2return && !mgt_rt2sndr && !mgt_rtgot) {
		compress (contents, contents);
		mgt_return = strdup(contents);
	    }                     /* get return from From, if no Sender */
	}
    }
    else
    if (lexequ ("Reply-To", name))
    {
	if (!mgt_remail)
	{
dorplyto:
	    if (!mgt_s2return && !mgt_rt2sndr && !mgt_rtgot)
	    {
		compress (contents, contents);
		mgt_return = strdup(contents);;
		mgt_rtgot = TRUE;  /* From doesn't set it, but Reply-to does */
	    }
	}
    }
    else
    if (lexequ ("Via", name))   /* old rfc733 pseudo-standard   */
    {
	/*
	 * Only inc mgt_hops if this is the first line of the trace
	 */
	if (hdr_state != HDR_MOR && ++mgt_hops > maxhops)
	{
	    if (mgt_vchan.mgt_achan != (Chan *) 0)
		ll_log (logptr, LLOGTMP, "reject on hop count from: %s %s %s",
			mgt_vchan.mgt_achan -> ch_name, mgt_vchan.mgt_ahost,
			mgt_return);
	    else
		ll_log (logptr, LLOGTMP, "reject last hop:  %s", contents);
	    err_msg (RP_NDEL, "Message has travelled too many hops");
	}

	argc = str2arg (contents, 25, argv, (char *) 0);
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "from '%s'; contents '%s'",
	    mgt_vchan.mgt_ahost, argv[0]);
#endif

	if (mgt_vchan.mgt_achan != (Chan *) 0 &&
	    lexequ (mgt_vchan.mgt_ahost, argv[0]) &&
	    ++mgt_loops > 2)
	{
	    ll_log (logptr, LLOGTMP, "reject on loop from:  %s %s %s",
			mgt_vchan.mgt_achan -> ch_name, mgt_vchan.mgt_ahost,
			mgt_return);
	    err_msg (RP_NDEL,
			"Message has been through this site (%s) already",
			locname);
	}
    }
    if (lexequ ("Received", name)) /* RFC822 standard   */
    {
	/*
	 * Only inc mgt_hops if this is the first line of the trace
	 */
	if (hdr_state != HDR_MOR && ++mgt_hops > maxhops)
	{
	    if (mgt_vchan.mgt_achan != (Chan *) 0)
		ll_log (logptr, LLOGTMP, "reject on hop count from: %s %s %s",
			mgt_vchan.mgt_achan -> ch_name, mgt_vchan.mgt_ahost,
			mgt_return);
	    else
		ll_log (logptr, LLOGTMP, "reject last hop:  %s", contents);
	    err_msg (RP_NDEL, "Message has travelled too many hops");
	}

	argc = str2arg (contents, 25, argv, (char *) 0);
	for (ind = 0; ind < argc - 2; ind += 2)
	    if (lexequ (argv[ind], "by"))
		break;          /* found the field naming the receiver */

	if (ind++ < argc)
	{                       /* compare our name with 'received' name */
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "from '%s'; contents '%s'",
		    mgt_vchan.mgt_ahost, argv[ind]);
#endif

	    if (mgt_vchan.mgt_achan != (Chan *) 0 &&
		    lexequ (mgt_vchan.mgt_achan -> ch_lname, argv[ind]) &&
		    ++mgt_loops > 2)
	    {
		ll_log (logptr, LLOGTMP, "reject on loop from:  %s %s %s",
			mgt_vchan.mgt_achan -> ch_name,
			mgt_vchan.mgt_achan -> ch_lname,
			mgt_return);
		err_msg (RP_NDEL,
			"Message has been through this site (%s) too many times",
			mgt_vchan.mgt_achan -> ch_lname);
	    }
	}
    }
    if (lexequ ("Message-Id", name))
    {
	mgt_gotid++;
    }
    return (RP_OK);     /* no management policies, for this one         */
}
/**/

mgt_hend ()
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mgt_hend ()");
    ll_log (logptr, LLOGFTR, "uid=%d,effec=%d, fmcnt=%d,okauth=%d",
				userid, effecid, mgt_fmcnt, mgt_okauthor);
#endif

    if (!mgt_remail) {
	if (mgt_sncnt == 0 && mgt_fmcnt > 1 && mgt_trust == NRMFROM)
	    err_msg (RP_PARM, "Too many From: lines");
	if (mgt_sncnt > 1 && mgt_trust == NRMFROM)
	    err_msg (RP_PARM, "Too many Sender: lines");
    }

    if (mgt_trust == OKFROM)     /* pure trust requested               */
    {
	if (!PRIV_USER(userid) && mgt_vchan.mgt_achan == 0)
				  /* trust root, mmdf, and relays      */
	    mgt_trust = FLGFROM; /* otherwise note, but honor          */
    }

    if (mgt_addid && !mgt_gotid)
	mgt_messageid ();

    if (mgt_trust == FLGFROM || mgt_txsrc != 0)
	mgt_srcinfo ();           /* this must follow the above "if"    */

    if (mgt_trust != NRMFROM)
	return (RP_OK);           /* not checking for these errors      */

    if (!mgt_okauthor)
	err_msg (RP_PARM, "No valid author specification present");

    return (RP_OK);
}
/**/

LOCFUN
	mgt_forward (channame)    /* certify source as relay site       */
    char channame[];
{                                 /* map username to "host" on chan     */
    register Chan *thechan;       /* chan claiming to come from         */

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mgt_forward (%s)", channame);
#endif

    if ((thechan = ch_nm2struct (channame)) == (Chan *) NOTOK)
	err_abrt (RP_PARM, "Channel '%s' does not exist", channame);

    mgt_vchan.mgt_achan = thechan;
}

/**/

LOCFUN
	mgt_author (contents)     /* does component contain only the    */
    char contents[];
{                                 /* address of the actual sender?      */
    struct passwd *pwdptr;
    AP_ptr  locptr,               /* local & domain parts               */
	    dmnptr,
	    aptr;
    int retval = TRUE;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mgt_author(%s)", contents);
#endif

    /* do not normalize.  evaluate as is */
    if ((aptr = ap_s2tree (contents)) == (AP_ptr) NOTOK
	|| ap_t2parts (aptr, (AP_ptr *) 0, (AP_ptr *) 0,
		&locptr, &dmnptr, (AP_ptr *) 0) != (AP_ptr) OK) {
	retval = FALSE;         /* only allowed one address in the field */
	goto cleanup;
    }

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "checking: %s, uid %d, %s", locptr -> ap_obvalue,
		userid, dmnptr ? dmnptr -> ap_obvalue : "No Domain");
#endif
    if ((pwdptr = getpwmid (locptr -> ap_obvalue)) == (struct passwd *)NULL
      || pwdptr -> pw_uid != userid)
      retval = FALSE;
    /*
     *  If a domain was specified, make sure it was the proper one.
     */
    if (retval == TRUE && dmnptr != (AP_ptr) 0) {
	Dmn_route dmnroute;
	char tmpbuf[LINESIZE];

#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "mgt_author, v2route(%s)",dmnptr->ap_obvalue);
#endif
	switch ((int)dm_v2route (dmnptr -> ap_obvalue, tmpbuf, &dmnroute)) {
	case MAYBE:
	    retval = FALSE;
	    break;
	case OK:                /* Can't happen ??  -DPK- */
	    retval = TRUE;
	    break;
	default:
#ifdef DEBUG
	    ll_log (logptr, LLOGBTR, "mgt_author, h2chan(%s)", dmnroute.dm_argv[0]);
#endif
	    if (ch_h2chan (dmnroute.dm_argv[0], 1) != OK)
		retval = FALSE;
	    break;
	case NOTOK:
#ifdef DEBUG
	    ll_log (logptr, LLOGBTR, "mgt_author, h2chan(%s)", dmnptr->ap_obvalue);
#endif
	    if (ch_h2chan (dmnptr -> ap_obvalue, 1) != OK)
		retval = FALSE;
	    break;
	}
    }

cleanup:
    if (aptr && aptr != (AP_ptr)NOTOK) {
	ap_sqdelete (aptr, (AP_ptr) 0);
	ap_free (aptr);
    }

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mgt_author, returns %d", retval);
#endif
    return (retval);
}
/**/

LOCFUN
	mgt_messageid ()            /* add Message-ID: */
{
    char buf[32];
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mgt_messageid ()");
#endif

    if (isstr(locmachine))
	fprintf(mq_mffp, "Message-ID:  <%s.%s@%s.%s.%s>\n",
	    cnvtdate(TIMCOM, buf), &mq_munique[4], locmachine,
	    locname, locdomain);
    else
	fprintf(mq_mffp, "Message-ID:  <%s.%s@%s.%s>\n",
	    cnvtdate(TIMCOM, buf), &mq_munique[4], locname, locdomain);
}

LOCFUN
	mgt_srcinfo ()            /* add Source-Info field, maybe       */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mgt_srcinfo ()");
#endif

    fputs ("Source-Info:  ", mq_mffp);
    if (mgt_trust == FLGFROM)
    {                             /* note the lack of authentication    */
	fputs ("From (or Sender) name not authenticated.\n", mq_mffp);
	if (mgt_txsrc != 0)       /* indent second line of text         */
	    fputs ("     ", mq_mffp);
    }
    if (mgt_txsrc != 0)           /* user wants to say something        */
    {
	fputs (mgt_txsrc, mq_mffp);
	if (mgt_txsrc[strlen (mgt_txsrc) - 1] != '\n')
	    putc ('\n', mq_mffp);
    }
}

/**/

LOCFUN
	mgt_via ()                /* note fact of relaying              */
{
    extern char *cnvtdate ();
    char    thedate[LINESIZE];
    int len;
#ifdef VIATRACE
    char    *p;
#endif

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mgt_via ()");
#endif

/* Received: by hostname.net; date                                      */
/*  e.g.:                                                               */
/* Received: by Rand-Unix.ArpaNet; 21 Jan 79 12:40 EDT                  */

    if (mgt_vchan.mgt_achan -> ch_login != NOLOGIN)
				  /* single login authorized to relay   */
	if (!PRIV_USER(userid))	  /* caller is not special id           */
	    if (!lexequ (mgt_vchan.mgt_achan -> ch_login, username))
				  /* caller not the authorized id       */
		err_abrt (RP_USER, "%s not authorized to submit mail as %s",
			    username, mgt_vchan.mgt_achan -> ch_name);

    cnvtdate (TIMSHRT, thedate);  /* net name & short date/time         */

/*  if mail always goes through a relay, for this channel, then the
 *  host/chan combination is a constant and source reference is not
 *  useful; only the receiver will be listed.  The channel name is
 *  used to specify the transfer link.
 */

#ifdef VIATRACE
				/* needed for standard UK sites */
				/* note reversed domain ordering*/
    len = mgt_rcv (0, "Via:");
    if (mgt_vchan.mgt_achan -> ch_host == NORELAY)
	p = ap_dmflip (mgt_vchan.mgt_ahost);
    else
	p  = ap_dmflip (mgt_vchan.mgt_achan -> ch_host);
    len = mgt_rcv (len, " %s;", p);
    free (p);
#else VIATRACE

    len = mgt_rcv (0, "Received:");

    if(mgt_amdelay != TRUE){    /* special string for Delay channel */
	if (mgt_vchan.mgt_achan -> ch_host == NORELAY)
	    len = mgt_rcv (len, "from %s", mgt_vchan.mgt_ahost);
	else
	    len = mgt_rcv (len, "from %s", mgt_vchan.mgt_achan -> ch_host);
    }

#ifndef UCL
    if (isstr (locmachine))
	len = mgt_rcv (len, "by %s.%s.%s", locmachine,
		mgt_vchan.mgt_achan -> ch_lname,
		mgt_vchan.mgt_achan -> ch_ldomain);
    else
	len = mgt_rcv (len, "by %s.%s",
		mgt_vchan.mgt_achan -> ch_lname,
		mgt_vchan.mgt_achan -> ch_ldomain);
    if (adr_orgspec != (char *) 0)
	len = mgt_rcv (len, "for %s", adr_orgspec);
    len = mgt_rcv (len, "id %s;", &mq_munique[4]);
			/* skip the "msg." preface */
#else UCL
    if (isstr (locmachine))
	len = mgt_rcv (len, "by %s.%s.%s ", locmachine, locname, locdomain);
    else
	len = mgt_rcv (len, "by %s.%s ", locname, locdomain);
			/* SEK use local rather than channel pref names? */
			/* structure ch_show to suit tase               */
    if(mgt_amdelay == TRUE)
	len = mgt_rcv (len, " with Delay channel");
    else
	len = mgt_rcv (len, " %s", mgt_vchan.mgt_achan -> ch_show);
    len = mgt_rcv (len, " id %s;", &mq_munique[4]);
			/* skip the "msg." preface */
#endif UCL

#endif VIATRACE
    (void) mgt_rcv (len, "%s\n", thedate);
}

/* VARARGS2 */

LOCVAR
mgt_rcv (curlen, fmt, val1, val2, val3)
    int curlen;
    char *fmt, *val1, *val2, *val3;
{
    char linebuf[LINESIZE];
    int len;

    sprintf (linebuf, fmt, val1, val2, val3);
    len = strlen (linebuf);
    if ((curlen + len) > 77)
    {
	fputs ("\n          ", mq_mffp);
	curlen = 10;
    }
    else
	if (curlen > 0)
	    putc (' ', mq_mffp);
    fputs (linebuf, mq_mffp);
    return (curlen + len);
}

/* generate a delay channel 'host parameter' */

char    *
mgt_dstgen()
{
	char	*multcat();
	return(multcat(mgt_vchan.mgt_achan == NULL ?
			mgt_dlname : mgt_vchan.mgt_achan->ch_name,
			"&", mgt_vchan.mgt_ahost, (char *)0));
}
