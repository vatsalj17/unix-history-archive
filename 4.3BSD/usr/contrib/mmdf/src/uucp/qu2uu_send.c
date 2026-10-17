#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include <signal.h>
#include "ap.h"
#include "phs.h"

/*
 *                      Q U 2 U U _ S E N D . C
 *
 *                  SEND FROM DELIVER TO UUCP PROGRAMS
 *
 *  The UUCP channel developed for MMDF at the US Army
 *  Ballistics Research Lab by Doug Kingston.    <dpk@brl>
 *
 *                     Original Version 21 Oct 81
 */

/*
 *     MULTI-CHANNEL MEMO DISTRIBUTION FACILITY  (MMDF)
 *
 *
 *     Department of Electrical Engineering
 *     University of Delaware
 *     Newark, Delaware  19711
 *
 *     Phone:  (302) 738-1163
 *
 *
 */

extern	char	*malloc();
extern	char	*rindex();
extern  char    *strdup();
extern  char    *ap_p2s();
extern  char    *multcat();
extern	char	*blt();

extern struct ll_struct   *logptr;
extern char *qu_msgfile;          /* name of file containing msg text   */
extern Chan      *curchan;  /* Who we are */


int pbroke;             /* Set if SIGPIPE occurs (in qu2uu_each) */

LOCVAR struct rp_construct
	rp_aend =
{
    RP_OK, 'u', 'u', 'c', 'p', ' ', 'e', 'n', 'd', ' ', 'o', 'f', ' ', 'a',
    'd', 'd', 'r', ' ', 'l', 'i', 's', 't', '\0'
},
	rp_mok =
{
    RP_MOK, 'm', 'o', 'k', '\0'
},
	rp_noop =
{
    RP_NOOP, 's', 'u', 'b', '-', 'l', 'i', 's', ' ', 'n', 'o', 't', ' ',
    's', 'p', 'e', 'c', 'i', 'a', 'l', '\0'
},
	rp_ns =
{
    RP_NS, 't', 'e', 'm', 'p', ' ', 'n', 'a', 'm', 'e', 's', 'e', 'r', 'v',
    'e', 'r', ' ', 'f', 'a', 'i', 'l', 'u', 'r', 'e', '\0'
},
	rp_err =
{
    RP_NO, 'u', 'n', 'k', 'n', 'o', 'w', 'n', ' ', 'e', 'r', 'r', 'o', 'r', '\0'
},
	rp_pipe =
{
    RP_NO, 'u', 'u', 'x', ' ', 'p', 'i', 'p', 'e', ' ',
    'b', 'r', 'o', 'k', 'e', ' ', '(', 's', 'y', 's', 't', 'e', 'm', ' ',
    'u', 'n', 'k', 'n', 'o', 'w', 'n', '?', ')', '\0'
},
	rp_bhost =
{
    RP_USER, 'b', 'a', 'd', ' ', 'h', 'o', 's', 't', ' ', 'n', 'a',
    'm', 'e', '\0'
};

/**/

qu2uu_send ()                     /* overall mngmt for batch of msgs    */
{
    short   result;
    char    *findfrom();
    char    *realfrom;
    char    info[LINESIZE],
	    sender[ADDRSIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu2uu_send ()");
#endif

    if (rp_isbad (result = qu_pkinit ()))
	return (result);

    /*
     *  AP_SAME == NO header munging.  We need to read the header
     *  ourselves and the munging just gets in the way.
     */
    for(;;){        /* get initial info for new message   */
	result = qu_rinit (info, sender, curchan -> ch_apout);
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
	phs_note (curchan, PHS_WRSTRT);

	printx ("\r\nReformatting message from %s (%s)\r\n", sender, info);
	realfrom = findfrom (sender);
	if(realfrom == (char *)MAYBE){
	    result = RP_NS;
	    break;
	}
	printx ("Realfrom is '%s'\r\n", realfrom);

	if (rp_isbad (result = qu2uu_each (sender, realfrom)))
	    return (result);      /* process each adrlst/text pair     */
	qu_rend();
    }
    qu_rend();

    if (rp_gval (result) == RP_NS)
    {
	ll_log (logptr, LLOGTMP, "Nameserver failure");
	return (RP_NS);           /* catch protocol errors              */
    }
    if (rp_gval (result) != RP_DONE)
    {
	ll_log (logptr, LLOGTMP, "not DONE (%s)", rp_valstr (result));
	return (RP_RPLY);         /* catch protocol errors              */
    }

    qu_pkend ();                  /* done getting messages              */
    phs_note (curchan, PHS_WREND);

    return (result);
}
/**/

LOCFUN
	qu2uu_each (sender, realfrom)   /* generate a UUX for an address */
	char *sender;
	char      *realfrom;
{
    RP_Buf    replyval;
    short     result;
    char      host[ADDRSIZE];
    char      adr[ADDRSIZE];
    extern    brpipe();
    int       (*oldsig)();

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu2uu_each()");
#endif

    FOREVER                       /* iterate thru list                    */
    {                             /* we already have and adr              */
	result = qu_radr (host, adr);
	if (rp_isbad (result))
	    return (result);      /* get address from Deliver           */
	if (rp_gval (result) == RP_HOK)
	{
	    qu_wrply ((RP_Buf *) &rp_noop, sizeof rp_noop);
	    continue;
	}
	else if (rp_gval (result) == RP_DONE)
	{
	    qu_wrply ((RP_Buf *) &rp_aend, sizeof rp_aend);
	    return (RP_OK);       /* end of address list                */
	}

	/*
	 *  Following function takes     (name+host)     and
	 *  looks up a!b!c!host then puts "a" in host and
	 *  "b!c!name" in the adr.
	 */
	pbroke = 0;
	oldsig = signal(SIGPIPE, brpipe);
	replyval.rp_val = uu_wtadr (host, adr, sender, realfrom);
	switch (replyval.rp_val)
	{
	    case RP_AOK:
	    case RP_OK:
#ifdef DEBUG
		ll_log (logptr, LLOGFTR, "Calling txtcpy()\n");
#endif
		replyval.rp_val = uu_txtcpy();
		break;

	    case RP_USER:
		ll_log (logptr, LLOGFAT, "host (%s) not in table", host);
		blt((char *)&rp_bhost, (char *) &replyval, sizeof rp_bhost);
		break;

	    case RP_NS:
		ll_log (logptr, LLOGTMP, "nameserver failure in host lookup");
		blt((char *)&rp_ns, (char *) &replyval, sizeof rp_ns);
		break;

	    default:
		ll_log (logptr,LLOGFAT,"unknown error (0%o)", replyval.rp_val);
		blt((char *)&rp_err, (char *) &replyval, sizeof rp_err);
		replyval.rp_val = RP_NO;
	}
	if (replyval.rp_val != RP_MOK) {
		qu_wrply (&replyval, sizeof(replyval.rp_val)
			  + strlen(replyval.rp_line));
	} else {
	    replyval.rp_val = uu_wttend();
	    switch (replyval.rp_val) {
		    case RP_AOK:
		    case RP_OK:
		    case RP_MOK:
			qu_wrply ((RP_Buf *)&rp_mok, sizeof rp_mok);
			break;

		    case RP_USER:
			ll_log (logptr, LLOGFAT, "host (%s) not in table", host);
			qu_wrply ((RP_Buf *)&rp_bhost, sizeof rp_bhost);
			break;

		    case RP_LIO:
			ll_log (logptr,LLOGTMP,"uux pipe broke (unknown?)");
			qu_wrply ((RP_Buf *)&rp_pipe, sizeof rp_pipe);
			break;

		    default:
			ll_log (logptr,LLOGFAT,"unknown error on close (0%o)", replyval.rp_val);
			qu_wrply ((RP_Buf *)&rp_err, sizeof rp_err);
	    }
	}
	signal(SIGPIPE, oldsig);
    }
}

brpipe()
{
	pbroke = 1;
	signal(SIGPIPE, SIG_IGN);
}

/********/

/*
 *      This function probably should be an address parser
 *      but for now this will have to do.  -DPK-
 *
 *      Simple address parser use inserted by smb.
 */
extern int ap_outtype;
extern AP_ptr ap_normalize ();

char *
findfrom (sender)
char *sender;
{
	int     aptypesav;
	char    *adr;
	register char    *p;
	AP_ptr  ap;
	AP_ptr  local,
		domain,
		route;
	char    *MakeUucpFrom();

/* SEK have axed looking at top of file.  */
/* This may not be wise - but very much neater */
/* Delver has no business being given UUCP style messsages */

	if ((ap = ap_s2tree (sender)) == (AP_ptr) NOTOK)
	{
	    ll_log (logptr, LLOGTMP, "Failure to parse address '%s'", sender);
	    return (strdup (sender));
	}

	ap = ap_normalize (curchan -> ch_lname, curchan -> ch_ldomain,
		ap, curchan);
	if(ap == (AP_ptr)MAYBE)
	    return( (char *)MAYBE);
	ap_t2parts (ap, (AP_ptr *) 0, (AP_ptr *) 0, &local, &domain, &route);
	aptypesav = ap_outtype;
	ap_outtype = AP_733;
	adr = ap_p2s ((AP_ptr) 0, (AP_ptr) 0, local, domain, route);
	if(adr == (char *)MAYBE){
	    ap_outtype = aptypesav;
	    return(adr);
	}
	if ((route == (AP_ptr) 0))
	{
		p = multcat (curchan -> ch_lname, ".",
				curchan -> ch_ldomain, (char *)0);
		if (lexequ (p, domain -> ap_obvalue))
		{
			free (adr);
			adr = strdup (local -> ap_obvalue);
		}
		free (p);
	}
	ll_log (logptr, LLOGFST, "sender = '%s'", adr);
	ap_outtype = aptypesav;

	lowerfy(adr);
	return(MakeUucpFrom(adr));
}

/*
 * This added by pc (UKC) to generate correct 'From' lines with
 * `!' separated routes for uucp sites
 * the rules
 * 	a@b   -> b!a
 *	a%b@c -> c!b!a
 *	etc
 *	a%b%c%d%e@x -> x!e!d!c!b!a
 *	This is done by a call to a recursive routine which I hope is OK
 */
static
char *
MakeUucpFrom(adr)
char *adr;
{	char *new;
	char *localname;
	register char *site;
	
	/*NOSTRICT*/
	if ((new = malloc(strlen(adr)+1)) == (char *)0)
		return ((char *)NOTOK);
	/*
	 * Can we assume that this is a legal 733 address ?
	 * look for the first site
	 */
	site = rindex(adr, '@');
	if (site)
	{	*site++ = '\0';
		/*
		 * some input channels (notably ni_niftp) will add the
		 * name of the local machine into this address
		 * so we look for it and delete it if found
		 */
		localname = multcat(curchan->ch_lname, ".", curchan->ch_ldomain, 0);
		/*
		 * if not the same then put back a % to let ScanUucpFrom work
		 */
		if (!lexequ (localname, site))
			site[-1] = '%';
		free(localname);
	}
	ScanUucpFrom(new, adr);
	free(adr);
#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "sender (From line) = '%s'", new);
#endif
  	return(new);
}

static
ScanUucpFrom(new, adr)
register char *new;
register char *adr;
{	register char *site;
	char	*rindex();

	/*
	 * This presumes that the address we are scanning is somewhat
	 * legal - but the @ has been replaced by a %
	 */
	site = rindex(adr, '%');
	if (site == (char *)0)
	{	(void) strcpy(new, adr);
		return;
	}
	*site++ = '\0';
	(void) strcpy(new, site);
	new += strlen(site);
	*new++ = '!';
	*new = '\0';
	ScanUucpFrom(new, adr);
}
