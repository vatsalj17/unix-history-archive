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

/*  SUBMIT ADDRESS HANDLING                                             */

/*  Apr, 81 Dave Crocker    adr_check able to handle foo<addr> form
 *  Jun, 81 Dave Crocker    adr_check copy test addr, before parsing
 *                          loc_alsrch check for increment to lnk_nadrs
 *                          in case alias entry had no good addresses
 *  Jul 81 Dave Crocker     use separate variable to detect no valid
 *                          alias entries
 *                          fixed alias no-valid-address handling
 *                          lnk_nadrs no longer referenced
 *  May 82 Dave Crocker     adr_ghost use locname, if host field null
 *                          adr_check use chan name as host, if none given
 *                          adr_check added call to loc_gsrch
 *                          loc_g* routines added to use group names as list
 *  Jun 82 Dave Crocker     prevent alias cycling by saving source specs
 *  Aug 82 Dave Crocker     adr_check returns bad user/host distinction
 *  Dec 82 Doug Kingston    fixed error in design of lc_afin(), and lc_gfin().
 *  Feb 83 Doug Kingston    Modified adr_local to recurse through adr_check
 *                          when the local string contains a '%', '.'.
 *  Apr 83 Steve Kille      Add code to transparently deliver between machines
 *                              This uses a table of user to machine matches
 *                              Could use aliases or password instead
 *  May 83 Steve Kille      Change adr_check to handle domain routes
 */

#include <pwd.h>
#include "ch.h"
#include "dm.h"
#include "ap.h"

#define MAXLOOPS 10

extern struct ll_struct *logptr;
extern char *ch_dflnam;
extern char *locname;
extern char *supportaddr;
extern char *sitesignature;
extern char *locmachine;               /* local machine name           */
extern int mgt_inalias;
extern int ap_outtype;
extern short tracing;

char *adr_fulldmn;              /* Name of 'full' domain                */
char *adr_fmc;                  /* name of 'full' machie                */
char *adr_orgspec;              /* original mailbox string              */

extern char *blt();
extern char *index();
extern char *rindex();
extern char mgt_dlname[];	/* name of the delay channel */
extern char *ap_p2s();

/***************  (adr_) PROCESS A SINGLE ADDRESS  *****************  */

LOCVAR char adr_gotone;		/* got at least one valid address     */
LOCVAR char adr_level = 0;	/* Level adr_check/adr_local recursion */

adr_check (local, domain, route) /* check & save an address            */
    AP_ptr  local,             /* beginning of local-part */
	    domain,            /* basic domain reference */
	    route;             /* beginning of 733 forward routing */
{
    extern Domain *dm_v2route ();
    Dmn_route dmrt;
    Dmn_route tmpdmrt;
    AP_ptr hptr;               /* 'next' host */
    Chan   *thechan;
    char    tstline[LINESIZE],
	    official[LINESIZE],
	    tmpstr[LINESIZE];
    int    i;
    int    nextchan;
    AP_ptr ap;
    char   *cp = (char *)0;
    int loopcnt = 0;
    int retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "adr_check (loc='%s', dom='%s', rt='%s')",
	  local -> ap_obvalue,
	  domain != (AP_ptr) 0 ? domain -> ap_obvalue : "[NIL]",
	  route != (AP_ptr) 0 ? route -> ap_obvalue : "[NIL]");
#endif

    /* Limit the number of time we loop looking up routes to domains */
    /* (Sort of a cop-out for foo.bar where foo isn't in the channel table */
    for (thechan = (Chan *) NOTOK; ; loopcnt++)
    {
	if (loopcnt > MAXLOOPS)
		return (RP_BHST);
	/*
	 * This if-then-else clause selects the nxt domain to be
	 * evaluated or calls adr_local if there are none left.
	 */
	if (route != (AP_ptr) 0) {	/* have explicit routing info */
	    hptr = route;		/* a list of fields             */
	    FOREVER
	    {
		switch (route -> ap_ptrtype) {
		    case APP_NIL:
		    case APP_NXT:
			route = (AP_ptr) 0;
			break;  /* no more route */

		    case APP_ETC:
			route = route -> ap_chain;
			if(route == (AP_ptr)0)
				break;
			switch (route -> ap_obtype) {
			    case APV_DLIT:
			    case APV_DOMN:
				break;

			    case APV_CMNT:
				continue;

			    default:
				route = (AP_ptr) 0;
				break;  /* no more route */
			}
		}
		break; /* no route */
	    }
	}
	else                    /* just use the primary reference */
	if (domain != (AP_ptr) 0) {	/* domain ref is a single field */
	    hptr = domain;
	    domain = (AP_ptr) 0;
	}
	else {
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "local address");
#endif
	    if(adr_level++ == 0)
		adr_orgspec = NULL;
	    retval = adr_local (local -> ap_obvalue);
	    adr_level--;
	    return (retval);
	}

/*
 *  a single domain reference is evaluated
 */
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "testing '%s'", hptr -> ap_obvalue);
#endif

	if (thechan != (Chan *) NOTOK) {
	    /* Last time through was channel reference */
	    /* Check the specified table this time */
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "chan specified, checking '%s'",
		thechan -> ch_show);
#endif
	    /* SEK if lookup fails, may still be domain */
	    /* reference, so fall into code down below */
	    retval = tb_k2val(thechan->ch_table,TRUE,hptr->ap_obvalue, tstline);
	    if(retval == MAYBE)
		return(RP_NS);
	    else if(retval == OK)
	    {
		strcpy (tstline, hptr -> ap_obvalue);
		strcpy (official, tstline); /* no domain */
		goto storeit;
	    }
	}

	if (thechan == (Chan *) NOTOK)
	{
	    if (lexequ (hptr -> ap_obvalue, locname) ||
		lexequ (hptr -> ap_obvalue, adr_fulldmn) ||
		lexequ (hptr -> ap_obvalue, locmachine) ||
		lexequ (hptr -> ap_obvalue, adr_fmc)) {
#ifdef DEBUG
		 ll_log (logptr, LLOGFTR, "loc ref '%s' found", hptr ->
			ap_obvalue);
#endif
				/* SEK shortcut normalised address      */
		 continue;
	    }
	}

	switch ((int)dm_v2route (hptr -> ap_obvalue, official, &dmrt)) {
	    case MAYBE:
		return (RP_NS);
				  /* obtain a host reference            */
	    case OK:              /* 'tis us                            */
#ifdef DEBUG
		ll_log (logptr, LLOGFTR, "local domain reference");
#endif
		continue;         /* go to next domain reference        */

	    case NOTOK:		  /* Not a valid hostname */
				  /* SEK: first check for explicit       */
				  /* channel refs (somehow)              */
		if (cp = rindex(hptr -> ap_obvalue, '#')) {
		    *cp = 0;
		    if ((thechan = ch_nm2struct (hptr -> ap_obvalue))
						!= (Chan *) NOTOK) {
#ifdef DEBUG
			ll_log (logptr, LLOGFTR, "explicit chan spec '%s'",
				thechan -> ch_name);
#endif
			cp = 0;
			continue;
		    }
		}
		if ((thechan = ch_nm2struct("badhosts")) != (Chan *)NOTOK) {
		    strcpy (tstline, thechan->ch_host ? thechan->ch_host : "");
		    strcpy (official, hptr -> ap_obvalue);
		    goto storeit;
		}

		/*
		 * Bad name - really not known, and no place to send it.
		 */
		return (RP_BHST);

	    default:
		 if (thechan != (Chan *) NOTOK) {
#ifdef DEBUG
		    ll_log (logptr, LLOGFTR, "chan specified, checking '%s'",
			thechan -> ch_show);
#endif
		    if (dmrt.dm_argc != 1)
			return (RP_BHST);
		    switch(tb_k2val (thechan -> ch_table, TRUE,
				dmrt.dm_argv[0], tstline)) {
		    case MAYBE:
			return (RP_NS);
		    case OK:
			break;
		    default:
			return (RP_BHST);
		    }
		    strcpy (tstline, dmrt.dm_argv[0]);
		    goto storeit;                /* store it */
		}

		for (i = 0; i < (dmrt.dm_argc - 1); i++) {
				/* algorithm - first component in table is */
				/* last component in route, but before     */
				/* route in  address.                      */
				/* Thus take route components from the     */
				/* left, and add to front of explictit     */
				/* route                                   */
		    if (domain == (AP_ptr) 0) {
#ifdef DEBUG
		       ll_log (logptr,LLOGFTR,"Adding local (1) component '%s'",
					dmrt.dm_argv[i]);
#endif
			domain = ap_new (APV_DOMN, dmrt.dm_argv[i]);
		    }
		    else {
#ifdef DEBUG
		       ll_log (logptr, LLOGFTR, "Adding route component 1 '%s'",
					dmrt.dm_argv[i]);
#endif
			ap = ap_new (APV_DOMN, dmrt.dm_argv[i]);
			if (route != (AP_ptr) 0)
			    ap -> ap_ptrtype = APP_ETC;
			ap -> ap_chain = route;
			route = ap;
		    }
		}
#ifdef DEBUG
		ll_log (logptr, LLOGFTR, "Checking '%s' in channel tables",
				dmrt.dm_argv [dmrt.dm_argc - 1]);
#endif
		switch ((int)(thechan = ch_h2chan
			  (dmrt.dm_argv [dmrt.dm_argc -1], 1)))
		{
		    case MAYBE:
				/* NS failure */
				return(RP_NS);
		    case OK:      /* 'tis us                            */
				/* SEK first check for self to avoid    */
				/* loops due to references to           */
				/* non-existent local subdomains        */
			if (dmrt.dm_argc > 1)
			   if (lexequ (dmrt.dm_argv [dmrt.dm_argc - 2],
				official))
			    {
				/* Check first to see if previous entry is */
				/* in channel tables, for handling locmachine */
				switch ((int)(thechan = ch_h2chan
				  (dmrt.dm_argv [dmrt.dm_argc -2], 1)))
				{
				    case MAYBE:
					return(RP_NS);
				    case OK:
				    case NOTOK:
#ifdef DEBUG
					ll_log (logptr, LLOGTMP,
					  "Found unknown subdomain '%s'",
					  hptr -> ap_obvalue);
#endif
					return (RP_BHST);

				   default:
					/* Found host ref               */
					strcpy (tstline, dmrt.dm_argv [dmrt.dm_argc  - 2]);
					strcpy (official, tstline);
					/* Now remove first component of */
					/* route                         */
					if (route == (AP_ptr) 0) {
						ap_free (domain);
						domain = (AP_ptr) 0;
					}
					else {
						ap = route;
						route = ap -> ap_chain;
						ap_free (ap);
					}
					goto storeit;
				}
			    }
#ifdef DEBUG
			ll_log (logptr, LLOGFTR, "local host reference");
#endif
			thechan = (Chan *) NOTOK;               /* DPK */
			continue; /* go to next domain reference        */

		    case NOTOK:   /* hmmm, unknown                      */
			switch((int)dm_v2route (dmrt.dm_argv[dmrt.dm_argc -1],
				tmpstr, &tmpdmrt)) {
			case NOTOK:
			    return (RP_BHST);
			case MAYBE:
			    return (RP_NS);
			}
			thechan = (Chan *) NOTOK;
			ap = ap_new (APV_DOMN, dmrt.dm_argv[dmrt.dm_argc -1]);
			if (route != (AP_ptr) 0)
			    ap -> ap_ptrtype = APP_ETC;
			ap -> ap_chain = route;
			route = ap;
			continue;
		    default:
			strcpy (tstline,
				dmrt.dm_argv [dmrt.dm_argc - 1]);
			if (dmrt.dm_argc > 1)
			    strcpy(official, tstline);
				/* make sure official has full domain   */
			goto storeit;
		}
	}
    	/*NOTREACHED*/
    }

/*
 *  validated non-reflexive address is enqueued
 */
storeit:

     if (domain == (AP_ptr) 0)
     {
#ifdef DEBUG
	 ll_log (logptr, LLOGFTR, "Adding local (2) component '%s'", official);
#endif
	 domain = ap_new (APV_DOMN, official);
     } else {
#ifdef BOTHEND
				/* SEK - at this point we pay for not */
				/* ap_normalize earlier, and must clean up */
				/* set thechan to map any looped local refs */
	if(ap_dmnormalize (domain,  thechan) == MAYBE){
		retval = RP_NS;
		goto bugout;
	}

	for(ap = route ; ap != (AP_ptr)0 ; ap = ap->ap_chain){
		switch(ap->ap_obtype){
		    case APV_DOMN:
			if(ap_dmnormalize (ap, thechan) == MAYBE){
				retval = RP_NS;
				goto bugout;
			}
		    case APV_DLIT:
		    case APV_CMNT:
			continue;
		}
		break; /* no more route */
	}
#endif BOTHEND
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "Adding route component 2 '%s'", official);
#endif
	ap = ap_new (APV_DOMN, official);
	if (route != (AP_ptr) 0)
	    ap -> ap_ptrtype = APP_ETC;
	ap -> ap_chain = route;
	route = ap;
     }
    /* there should be some code in here to delete spurious extra components
     * in the domain and the route, this can do nasty things on some sites
     */


    cp = ap_p2s ((AP_ptr) 0, (AP_ptr) 0, local, domain, route);
    if(cp == (char *)MAYBE){
	retval = RP_NS;
	goto bugout;
    }

    auth_uinit (cp);
    nextchan = 2;
    retval = RP_OK;

    while (mgt_aok (thechan, tstline, cp, "") == 0) {
	if((thechan = ch_h2chan(tstline, nextchan++)) == (Chan *)NOTOK){
		auth_bad();	/* build an error message */
		retval = RP_NAUTH;
#ifdef DEBUG
		ll_log(logptr, LLOGFTR, "No authorized route");
#endif
		break;
	}
	else if(thechan == (Chan *)MAYBE){
		retval = RP_NS;
		break;
	}
    }

    if(rp_gval(retval) == RP_OK) {
	free(cp);

	/* put the address into a form that can be used */
	i = ap_outtype;                 /* save this setting */
	ap_outtype = thechan -> ch_apout;
	cp = ap_p2s ((AP_ptr) 0, (AP_ptr) 0, local, domain, route);
	ap_outtype = i;
	if(cp != (char *)MAYBE){
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "linking for chan '%s'",thechan -> ch_name);
#endif
	    lnk_adinfo (thechan, tstline, cp);
	    adr_gotone = TRUE;
	    retval = RP_AOK;
	}
	else
	    retval = RP_NS;
    }

    auth_uend ();

bugout:
    if (domain != (AP_ptr) 0)
    {
	ap_sqdelete (domain, (AP_ptr) 0);
	ap_free (domain);
    }
    if (route != (AP_ptr) 0)
    {
	ap_sqdelete (route, (AP_ptr) 0);
	ap_free (route);
    }
    if (cp && cp != (char *)MAYBE)
	free (cp);
    return (retval);
}
/**/

adr_dsubmit(addr)
char	*addr;
{
    Chan    *thechan;
    extern  int mgt_nodelay;
    char    *mgt_dstgen();
    char    *xcp;

    if(mgt_nodelay == TRUE) /* can't use the delay channel */
	return(RP_NS);

    if(addr == (char *)MAYBE)	/* is this check needed ?? */
	return(RP_NS);

    if( (thechan = ch_nm2struct(mgt_dlname)) == (Chan *)NOTOK)
	return(RP_NS);      /* delay channel does not exist */

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "linking for chan '%s'",thechan -> ch_name);
#endif

    lnk_adinfo (thechan, (xcp = mgt_dstgen()), addr);
    free(xcp);
    adr_gotone = TRUE;
    return(RP_DOK);
}

adr_local (local)                       /* process host-less reference */
    char *local;
{
    Chan *thechan;
    char    qualstr[LINESIZE],
	    tmpstr[LINESIZE];
    char    old_gotone;           /* stacked value of adr_gotone        */
    char    *cp;
    int     retval, bypass = 0;

    strcpy (tmpstr, local);

    /*
     *  Important Note:
     *      The order of the following conditions determines
     *      precedence for the separators @, ., %, and !.
     *      This assumes standard (from K&R) evaluation.
     *      If your compiler is non-standard, you're screwed.
     */
    if ( (cp = rindex (tmpstr, '@')) != 0
	  || (cp = rindex (tmpstr, '%')) != 0
	  || (cp = index (tmpstr, '!')) != 0
#ifndef JNTMAIL
	  || (cp = rindex (tmpstr, '.')) != 0
#endif
	)
				/* SEK handle quoted @, and quoted or   */
				/* unquoted . or %.  If JNT Mail, do not*/
				/* handle ".".  In this case this is    */
				/* only needed for quoted @ and %       */
    {
	AP_ptr  adrtree;
	AP_ptr  loctree, domtree, routree;
	int     rtnval;

#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "recursing on local part: '%s'", local);
#endif
	/*
	 *  We recurse after changing the % to an `@` after
	 *  running ap_normalize on the resultant string.
	 */
	if (*cp != '!')
	    *cp = '@';
	else
	{
	    char   tbuf [LINESIZE];
	    *cp++ = '\0';
	    sprintf (tbuf, "%s@%s", cp, tmpstr);
	    strcpy (tmpstr, tbuf);
	}
	if ((adrtree = ap_s2tree (tmpstr)) != (AP_ptr) NOTOK) {
	    if(ap_normalize ((char *) 0, (char *) 0, adrtree, (Chan *) 0) ==
						(AP_ptr)MAYBE){
		ap_sqdelete (adrtree, (AP_ptr) 0);
		ap_free (adrtree);   /* delete full tree */
		return(RP_NS);
	    }
	    ap_t2parts (adrtree, (AP_ptr *)0, (AP_ptr *)0,
			&loctree, &domtree, &routree);
	    rtnval = adr_check( loctree, domtree, routree );  /* RECURSE */
	    ap_sqdelete (adrtree, (AP_ptr) 0);
	    ap_free (adrtree);   /* delete full tree */
	    return( rtnval );
	}
    }

    if (ch_dflnam[0] == '\0') /* no local delivery && no hostname   */
	return (RP_BHST);     /*    => address not legal            */

    if ((thechan = ch_nm2struct (ch_dflnam)) == (Chan *) NOTOK)
    {
	ll_log (logptr, LLOGFTR, "default channel '%s' unknown",
		    ch_dflnam);
	return (RP_BHST);
    }
    strcpy (tmpstr, local);
			    /* local will be untouched version    */
    qualstr[0] = '\0';
    adr_gparm (tmpstr, qualstr);

    if (bypass = (local[0] == '~')) {    /* bypass alias search   */
	strcpy (tmpstr, &tmpstr[1]);
	strcpy (local, &local[1]);      /* need both if qualstr     */
    }

    old_gotone = adr_gotone;
    if ((retval = lc_search (tmpstr, qualstr, bypass)) != RP_NO)
    {
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "alias = %s", local);
#endif
	if (adr_gotone)   /* in alias file & has valid addrs    */
	    return (RP_AOK);

	adr_gotone = old_gotone;
	if (retval == RP_NAUTH) {
	    ll_log (logptr, LLOGTMP, "*** No authorized addrs: alias '%s'", tmpstr);
	    return (RP_NAUTH);
	}
	if (retval == RP_NS)
	    return (RP_NS);

	ll_log (logptr, LLOGTMP, "*** No valid addrs: alias '%s'", tmpstr);
	return (RP_USER);
    }
    adr_gotone = old_gotone;

    if (lc_pwsrch (tmpstr))    /* not even a login name.  too bad  */
    {
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "local addr='%s',parm='%s'",
		    tmpstr, qualstr);
#endif
	auth_uinit (tmpstr);
	if (qualstr[0] == '\0' && mgt_aok (thechan, "", tmpstr, ""))
	{                       /* try it without the argument      */
	    lnk_adinfo (thechan, "", tmpstr);
				/* acceptable -> insert into list   */
				/* source string NOT used           */
	    adr_gotone = TRUE;
	    auth_uend ();
	    return (RP_AOK);
	}
	else if (mgt_aok (thechan, "", tmpstr, qualstr))
	{                       /* have to check the parm, too      */
	    lnk_adinfo (thechan, "", local);
				/* acceptable -> insert into list   */
				/* note that SOURCE string is used  */
	    adr_gotone = TRUE;
	    auth_uend ();
	    return (RP_AOK);
	}
	auth_bad ();
	auth_uend ();
	ll_log (logptr, LLOGFST, "Local address '%s' not authorized",
			tmpstr);
	return (RP_NAUTH);
    }

    /*
     *  Last chance:  See if we have a forwarding channel
     */
    if( (thechan = ch_nm2struct("badusers")) != (Chan *)NOTOK) {
	if (!mgt_aok (thechan, "", local, ""))
	    return(RP_NO);

#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "linking for chan '%s'", thechan -> ch_name);
#endif

	lnk_adinfo (thechan,
	    thechan->ch_host ? thechan->ch_host : "", local);
	adr_gotone = TRUE;
	return(RP_OK);
    }

    ll_log (logptr, LLOGFST, "Unknown user '%s'", tmpstr);

    return (RP_USER);         /* bad ref        */
}
/**/

adr_gparm (buf, to)               /* get local-mailbox parameter        */
    char   *buf;                  /* the buffer holding the text        */
    char   *to;                   /* put parameter into here            */
{
    register char  *strptr;

    for (strptr = buf ;; strptr++)
	 switch (*strptr)
	 {
	    case '\0':
		to[0] = '\0';
		return (FALSE);

	    case '/':
	    case '|':
	    case '=':
		strcpy (to, strptr);
		*strptr = '\0'; /* terminate the mailbox portion */
		return (TRUE);
	 }

     /* NOTREACHED */
}

/**************  (lc_)  LOCAL NAME TABLE SEARCHING  **************** */

extern Alias *al_list;            /* list of the aliases tables */

LOCVAR
struct lc_alstruct               /* previous aposinfo's are stored on  */
{                                 /*    lc_alsrch's stack when         */
				  /*    lc_alsrch needs to recurse     */
    char   *abufpos;              /* current position in alias buf      */
    char    aliasbuf[LINESIZE];   /* address-part of alias entry        */
}                  *lc_cralias;

LOCFUN
lc_afin ()                /* alst_proc input routine for file   */
{                         /* parse already-read line            */
    char    c;

    switch (c = *(lc_cralias->abufpos))
    {
	case 0377:
	case 0:
	    return( 0 );

	case '\n':
	    c = ',';
    }
    lc_cralias->abufpos++;
    return ( c );
}
/**/
lc_search (mbox, qualstr, bypass)
char	*mbox;
char	*qualstr;
int	bypass;
{
    register Alias *alp;
    struct lc_alstruct *oldalias, newalias;
    char *oldspec;
    int	oldinalias;
    register int retval;
    int badretval = RP_NO;
    char *badlist = (char *) 0;
    char tmpbuf[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lc_search(%s, %d)", mbox, bypass);
#endif
    if ((qualstr[0] != 0) && (qualstr[0] != '='))
	return(RP_NO);  /* Skip the alias search for user/file, user|prog */

    if (adr_orgspec != NULL)		/* we have been here before ... */
	if(lexequ(mbox, adr_orgspec))	/* this is getting boring ... */
	     return (RP_NO);		/* Pretend we didn't find the alias */

    /*
     *  This could almost use the library routine aliasfetch() except
     *  we need the al_flags information down below (AL_TRUSTED?).
     */
    newalias.abufpos = newalias.aliasbuf;
    for (alp = al_list; alp != (Alias *)0; alp = alp->al_next) {
	if (bypass && !(alp->al_flags & AL_NOBYPASS))
	    continue;
	if ((retval = tb_k2val (alp->al_table, TRUE, mbox,
				newalias.abufpos)) == OK)
	    break;
	if (retval == MAYBE)
	    badretval = RP_NS;	/* NS timeout? */
    }
    if (alp == (Alias *)0)
	return (badretval);

    strcpy (tmpbuf, mbox);
    strcat (tmpbuf, qualstr);

    if (lnk_adinfo ((Chan *) 0, "", tmpbuf) == OK) {
	adr_gotone = TRUE;
	return (RP_AOK);       /* already did this alias              */
    }

#ifdef DEBUG
    ll_log(logptr, LLOGFTR, "lc_search, newalias '%s'", newalias.abufpos);
#endif
    if (tracing)
	printf("alias: %s => %s\n", mbox, newalias.abufpos);
    if (qualstr[0] != '\0')
    {
	char *p;
				/* Found alias for username.  If username   */
				/* was part of "user=foo", check that alias */
				/* was just a simple username (otherwise    */
				/* alias=foo will make no sense).           */
	if ((index (newalias.aliasbuf, '|') != 0) ||
	    (index (newalias.aliasbuf, '/') != 0) ||
	    (index (newalias.aliasbuf, ',') != 0)) {
	    ll_log (logptr, LLOGTMP, "Illegal to use = in alias '%s:%s'",
			mbox,  newalias.aliasbuf);
	    return (RP_NO);
	}
	if ((p = rindex (newalias.aliasbuf, '@')) == 0)
	    strcat (newalias.aliasbuf, qualstr);
	else {
	    *p++ = '\0';
	    sprintf (tmpbuf, "%s%s@%s", newalias.aliasbuf, qualstr, p);
	    strcpy (newalias.aliasbuf, tmpbuf);
	}
    }

    /* so save previous info on stack     */
    oldspec = adr_orgspec;
    oldalias = lc_cralias;
    oldinalias = mgt_inalias;

    adr_orgspec = mbox;
    lc_cralias = &newalias;  /*   to allow processing new list     */
    mgt_inalias = (alp->al_flags & AL_TRUSTED ? TRUE : FALSE);

    adr_gotone = FALSE;
    if (rp_isbad (retval=alst_proc(lc_afin, FALSE, (int (*)()) 0, &badlist))) {
	char        linebuf[LINESIZE];

	ll_log (logptr, LLOGTMP, "Bad address in alias %s", mbox);
	if (tracing)
	    printf ("Bad address in alias %s\n", mbox);
	sprintf (linebuf, "%s %s", locname, sitesignature);
	ml_1adr (NO, NO, linebuf, "Bad address in alias", supportaddr);
	sprintf (linebuf, "Found bad address in alias '%s'.\n", mbox);
	ml_txt (linebuf);
	if (retval == RP_NAUTH)
	    ml_txt ("    (Authorization problem with valid address)\n");
	sprintf (linebuf, "The alias was '%s'.\n\n", newalias.aliasbuf);
	ml_txt (linebuf);
	if (badlist) {
	    sprintf (linebuf, "There were problems with:\n");
	    ml_txt (linebuf);
	    ml_txt (badlist);
	    free (badlist);
	    sprintf (linebuf, 
	"\nThe remaining addresses in the alias were used for submission.\n\n");
	    ml_txt (linebuf);
	}
	if (ml_end(OK) != OK)
	    ll_log (logptr, LLOGFAT, "Can't send to supportaddr");
    }
    adr_orgspec = oldspec;
    lc_cralias = oldalias;   /* pop previous info off stack        */
    mgt_inalias = oldinalias;

    if ((retval == RP_NAUTH) || (retval == RP_NS))
	return (retval);
    return (RP_AOK);
}

lc_pwsrch (name)                 /* search login names (password file) */
char  *name;                      /* search key                         */
{
    extern struct passwd *getpwmid ();
    extern int errno;
    char namebuf[LINESIZE];
    register int ind;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lc_pwsrch(%s)", name);
#endif

    errno = 0;

    if (getpwmid (name) != NULL)   /* case-independent name search       */
	return (TRUE);            /*   found an entry                   */

    if (errno != 0)
	ll_log (logptr, LLOGTMP, "Strange errno after pwsrch failure (%d)",
					errno);

    if ((ind = strindex ("|", name)) >= 0 ||
	(ind = strindex ("/", name)) >= 0   )
    {                             /* piped msg or stored into file      */
	blt (name, namebuf, ind);
	namebuf[ind] = '\0';
#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "lc_pwsrch base part (%s)", namebuf);
#endif
	return ((getpwmid (namebuf) != NULL) ? TRUE : FALSE);
    }

    return (FALSE);               /* return failure                     */
}
