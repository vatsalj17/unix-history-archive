#include "util.h"
#include "mmdf.h"
#include "hdr.h"
#include "ch.h"
#include "ap.h"
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
/* This module has bits extracted from submit modules                   */
/* It is used by the Q NIFTP channel to extract a sender address        */
/* From the text                                                        */
/* This is really a pretty ugly way to do things - but I didnt write    */
/* the protocol                                                         */
/*                                                                      */
/*      Steve Kille             August 1982                             */
/*      Steve Kille             Feb 83          extensive rehack        */


extern struct ll_struct *logptr;

extern Chan *curchan;

extern char *index ();
extern char *compress ();
extern char *ap_p2s();
extern AP_ptr ap_new();

extern char *locmachine;
extern char *locname;
extern char *locdomain;
/*
*/
				/* This is used to get the sender name  */
				/* Passes over JNT header and then      */
				/* examines the 822 header fields       */
				/* Takes Sender field if it exists      */
				/* If not takes first From field        */
hdr_sender (sender, ack, hdr_fp, viahost)
char    *sender;                /* Where to stuff name of sender        */
char    **ack;                  /* ack address - note double indirection*/
FILE    *hdr_fp;                /* File being examined                  */
char    *viahost;               /* Host message receoved from           */
{
    char        line[LINESIZE];         /* temp buffer                  */
    char        name[LINESIZE];         /* Name of header field         */
    char        contents[LINESIZE];     /* Contents of header field     */
    int         gotfrom;                /* Got address from From        */
    int         len;
    int         donevia;
    char        *p;
    AP_ptr      ap_ack,
		ap_sender,
		jntroute,
		ap,
		domain,
		route,
		mbox;

#ifdef DEBUG
   ll_log (logptr, LLOGBTR, "hdr_sender (%s)", viahost);
#endif

    gotfrom = FALSE;
    ap_sender = (AP_ptr) 0;
    sender [0] = '\0';
    jntroute = (AP_ptr) 0;

				/* We are at start of file              */
				/* Therefore skip JNT mail header       */
    while ((len = gcread (hdr_fp, line, LINESIZE, "\n\377")) > 1)
    {
	line [len - 1] = '\0';
	for (p = line ; isspace(*p) ;p++);
	if (*p == '\0')
	    break;
    }
    if (len == NOTOK)
    {
	ll_err (logptr, LLOGFAT, "Can't find 733 header");
	return (RP_NO);
    }



				/* Now process header lines             */
   FOREVER
   {
				/* Get a line                           */
	if ((len = gcread (hdr_fp, line, LINESIZE, "\n\377")) < 1)
	{
	    ll_log (logptr, LLOGTMP, "Message has no body" );
				/* someone MIGHT want to do this so     */
				/* just log and pass on through         */
	    if (gotfrom)        /* drop down and return adr             */
		break;
	    return (RP_NO);
	}
	line [len] = '\0';
	ll_log (logptr, LLOGFTR, "gcread (in header) = '%s' (%d)", line, len);

	switch (hdr_parse (line, name, contents))
	{
	    case        HDR_NAM:
				/* No name so contine through header    */
		continue;

	    case        HDR_EOH:
				/* End of header                        */
		if (gotfrom)    /* if we have From address pass it back */
		       break;
		return (RP_NO);

	    case        HDR_NEW:
	    case        HDR_MOR:
				/* Real field so check it               */
		if (lexequ (name, "Via"))
		{
		    if ((p = index (contents, ';')) == 0)
		    {
			ll_log (logptr, LLOGTMP, "Illegal via: field '%s'",
				contents);
			continue;
		    }
		    *p = '\0';
		    compress (contents, contents);
		    /*
		     * must take comments out of via field, ( to get info
		     * correct )
		     * ( delete RFC822 comments from via field )
		     */
		    while ((p = index (contents, '(')) != NULL)
		    {
			char    *q;
			if ((q = index(p, ')')) == NULL)
			{
			     ll_log(logptr, LLOGTMP,
				"Illegal comment in via: field '%s'", contents);
			     break;
			}
			strcpy(p, q+1);         /* zap () comment */
		    }
		    if(p != NULL)               /* only non null if no ')' */
			continue;
		    compress (contents, contents);
#ifdef DEBUG
		    ll_log (logptr, LLOGFTR, "%s: %s", name, contents);
#endif
		    ap = ap_new (APV_DOMN, contents);
		    ap -> ap_chain = jntroute;
		    jntroute = ap;
		}

		if (lexequ (name, "acknowledge-to"))
		    if (*ack == (char *) 0)
		    {
			ap_ack = ap_s2tree (contents);
			if (ap_ack != (AP_ptr) NOTOK)
			{
			    if(ap_t2s (ap_ack, ack) == (AP_ptr)MAYBE)
				return(RP_NS);
#ifdef DEBUG
			    ll_log (logptr, LLOGFTR, "Ack field '%s", &ack);
#endif
			    ap_sqdelete (ap_ack, (AP_ptr) 0);
			}
		    }

		if (lexequ (name, "Sender"))
		{
		    ap_sender = ap_s2tree (contents);
		    if (ap_sender == (AP_ptr) NOTOK)
		    {
			ll_log (logptr, LLOGFTR, "Illegal sender field: %s",
					contents);
			return (RP_NO);
		    }
		    break;      /* now build route */
		}

		if (lexequ (name, "From") && !gotfrom)
		{
		    if ((ap_sender = ap_s2tree (contents)) !=
					(AP_ptr) NOTOK)
			gotfrom = TRUE;
		}
		continue;
	}
	break;          /* always fall out of loop */
    }
				/* build on the route from the trace */
    ap_t2parts (ap_sender, (AP_ptr *) 0, (AP_ptr *) 0,
		&mbox, &domain, &route);

    donevia = FALSE;
    FOREVER
    {
	if (jntroute != (AP_ptr) 0)
	{
	    ap = jntroute;
	    jntroute = jntroute -> ap_chain;
	}
	else if (donevia)
	    break;
	else
	{
	    donevia = TRUE;
	    if (viahost [0] == '\0')
	    {	char *vh;
	    	vh = isnull(*locmachine) ? locname : locmachine;
		if (curchan->ch_lname) vh = curchan->ch_lname;
		ap = ap_new (APV_DOMN, vh);
	    }
	    else
		ap = ap_new (APV_DOMN, viahost);
	}

	if (domain == (AP_ptr) 0)
	    domain = ap;
	else
	{
	    ap -> ap_chain = route;
	    route = ap;
	}
    }

				/* now normalise the domains        */
    if(ap_dmnormalize (domain, (Chan *) NULL) == MAYBE)
	return(RP_NS);
    for (ap = route; ap != (AP_ptr) 0; ap = ap -> ap_chain)
	if(ap_dmnormalize (ap, (Chan *) NULL) == MAYBE){
		ap_free (route);	/* I suspect this is not needed */
		return(RP_NS);
	}

				/* eliminating redundant entries     */
    if (route != (AP_ptr) 0)
    {
	for (ap = route; ap -> ap_chain != (AP_ptr) 0; )
		ap = ap -> ap_chain;

	if (lexequ (domain -> ap_obvalue, ap -> ap_obvalue))
	{
		ap -> ap_obtype = APV_NIL;
		free (ap -> ap_obvalue);
		ap -> ap_obvalue = (char *) 0;
		if (ap == route)
		{
			ap_free (route);
			route = (AP_ptr) 0;
		}
	}
    }
    p = ap_p2s ((AP_ptr) 0, (AP_ptr) 0, mbox, domain, route);
    if(p == (char *)MAYBE)
	return(RP_NS);
    strcpy (sender, p);
    free (p);
    ll_log (logptr, LLOGFTR, "Sender = '%s'", sender);
    return (RP_OK);
}
/*
*/
/* basic processing of incoming header lines */
LOCFUN
hdr_parse (src, name, contents)   /* parse one header line              */
    register char *src;           /* a line of header text              */
    char *name,                   /* where to put field's name          */
	 *contents;               /* where to put field's contents      */
{
    extern char *compress ();
    char linetype;
    register char *dest;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "hdr_parse(%s)", src);
#endif

    if (isspace (*src))
    {                             /* continuation text                  */
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "cmpnt more");
#endif
	if (*src == '\n')
	    return (HDR_EOH);
	linetype = HDR_MOR;
    }
    else
    {                             /* copy the name                      */
	linetype = HDR_NEW;
	for (dest = name; *dest = *src++; dest++)
	{
	    if (*dest == ':')
		break;            /* end of the name                    */
	    if (*dest == '\n')
	    {                     /* oops, end of the line              */
		*dest = '\0';
		return (HDR_NAM);
	    }
	}
	*dest = '\0';
	compress (name, name);    /* strip extra & trailing spaces      */
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "cmpnt name '%s'", name);
#endif
    }

    for (dest = contents; isspace (*src); )
	if (*src++ == '\n')       /* skip leading white space           */
	{                         /* unfulfilled promise, no contents   */
	    *dest = '\0';
	    return ((linetype == HDR_MOR) ? HDR_EOH : linetype);
	}                          /* hack to fix up illegal spaces      */

    while ((*dest = *src) != '\n' && *src != 0)
	     src++, dest++;       /* copy contents and then, backup     */
    while (isspace (*--dest));    /*   to eliminate trailing spaces     */
    *++dest = '\0';

    return (linetype);
}
