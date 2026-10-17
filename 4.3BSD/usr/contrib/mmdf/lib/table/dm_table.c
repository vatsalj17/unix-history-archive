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
#include "ch.h"                   /* has table state strcture def       */
#include "dm.h"
#include "chdbm.h"

extern Domain **dm_list;
extern Domain *dm_s2dom();
extern struct ll_struct   *logptr;
extern char *strcpy ();
extern char *index();
extern char *rindex();
extern char *locdomain;
extern char *locname;

/*
 * when mangling addresses, should we try the reverse before or after
 * the RFC822 direction
 * is set by submit otherwise is as below.
 */

typedef struct {char *dptr; int dsize;} datum;


/*
 * Steve Kille   jan 84   Redo this module
 *
 * G. Brendan Reilly May 84     take out dependencies on DBM package
 *
 *
 */


/* *******  FIND VALUE (address), GIVEN ITS KEY (hostname)  ********* */


LOCFUN
dm_k2val (dmntbl, subdmn, domain, dmnroute)
				   /* sub-domain spec -> routing host   */
register Domain  *dmntbl;         /* domain table to look in            */
register char  *subdmn;          /* name of sub-domain to look for     */
char *domain;                   /* where to stuff full domain name      */
Dmn_route *dmnroute;              /* where to put routing information   */
{
    char *p;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "dm_k2val (%s, %s)", dmntbl -> dm_name, subdmn);
#endif

    switch(tb_k2val (dmntbl -> dm_table, TRUE, subdmn, dmnroute -> dm_buf)){
    case MAYBE:
	return(MAYBE);
    case OK:
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "hit");
#endif
	dmnroute -> dm_argc = str2arg (dmnroute -> dm_buf,
			DM_NFIELD, dmnroute -> dm_argv, (char *)0);
	(void) strcpy(domain, dmnroute -> dm_argv[0]);
	return (OK);
    }

    p = subdmn;
    while ((p = index (p, '.')) != (char *)0)
    {
	char tdmnbuf[LINESIZE];

	p++;
	switch(tb_k2val (dmntbl -> dm_table, TRUE, p, tdmnbuf)) {
	case MAYBE:
	    return(MAYBE);
	default:
	    continue;
	case OK:
	    break;
	}
					/* get the entry                    */
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "hit");
#endif
	*(p - 1) = '\0';            /* build the domain             */
	if (dmntbl -> dm_domain[0] == '\0') {
	    if (*p == '\0')
		(void) strcpy (domain, subdmn);
	    else
		sprintf (domain, "%s.%s", subdmn, p);
	}
	else
	    sprintf (domain, "%s.%s.%s", subdmn, p, dmntbl -> dm_domain);

	sprintf (dmnroute -> dm_buf, "%s %s", domain, tdmnbuf);
	dmnroute -> dm_argc = str2arg (dmnroute -> dm_buf,
			DM_NFIELD, dmnroute -> dm_argv, (char *)0);
	return (OK);
    }

    return (NOTOK);
}

/* ***********  GIVEN Subdomain, FIND HOST ROUTE  ************* */
/*
 * If BOTHEND is defined then we are almost certainly running in JNT
 * land and so we should try the reversed address before the RFC822
 * direction, should save some cpu cycles
 */

LOCFUN
Domain *
#ifndef BOTHEND
	dm_sd2route (value, domain, dmnroute)
    char *value;                /* what the use provides                */
    char *domain;               /* full domain value                    */
    Dmn_route *dmnroute;        /* source route                         */
#else
	dm_sd2route (value, reverse, domain, dmnroute)
    char *value;                /* what the use provides                */
    char *reverse;              /* the same bacwards                    */
    char *domain;               /* full domain value                    */
    Dmn_route *dmnroute;        /* source route                         */
#endif
{
    char *sdptr;
    Domain *dmnptr;
    char official[LINESIZE];
#ifdef BOTHEND
    char *revsdptr = reverse;
#endif
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "dm_sd2route (%s)", value);
#endif
					/* first try the JNT way round if */
					/* got BOTHEND defined */
#ifdef	BOTHEND
    if ((dmnptr = dm_s2dom (reverse, official, dmnroute -> dm_buf)) !=
(Domain *) NOTOK || (dmnptr = dm_s2dom (value, official, dmnroute -> dm_buf))
	!= (Domain *) NOTOK)
#else
    if ((dmnptr = dm_s2dom (value, official, dmnroute -> dm_buf))
	!= (Domain *) NOTOK)
#endif
    {
	if(dmnptr == (Domain *)MAYBE)
	    return(dmnptr);
	(void) strcpy(domain, official);
	dmnroute -> dm_argc = str2arg (dmnroute -> dm_buf, DM_NFIELD,
		dmnroute -> dm_argv, (char *)0);
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "Domain value '%s' from '%s'",
		domain, dmnptr -> dm_show);
#endif
	return (dmnptr);
    }

    /*
     *  If not, move in from left.  Progressing on both forwards
     *  and backwards options.
     */
    sdptr = value;
    while ((sdptr = index (sdptr, '.')) != (char *) 0)
    {
	char tbuf[LINESIZE];

#ifdef  BOTHEND
	revsdptr = index (revsdptr, '.') + 1;
	if ((dmnptr = dm_s2dom (revsdptr, official, tbuf)) != (Domain *) NOTOK)
	{
	    if(dmnptr == (Domain *)MAYBE)
		return(dmnptr);
	    *(revsdptr - 1) = '\0';
	    sprintf (domain, "%s.%s", reverse, official);
	    sprintf (dmnroute -> dm_buf, "%s %s", domain, tbuf);
	    dmnroute -> dm_argc = str2arg (dmnroute -> dm_buf, DM_NFIELD,
		dmnroute -> dm_argv, (char *)0);
#ifdef DEBUG
	    ll_log(logptr,LLOGFTR, "Domain value (rev) '%s' from '%s',o = '%s'",
		domain, dmnptr -> dm_show, official);
#endif
	    return (dmnptr);
	}
#endif BOTHEND

	if ((dmnptr = dm_s2dom (++sdptr, official, tbuf)) != (Domain *) NOTOK)
	{
	    if(dmnptr == (Domain *)MAYBE)
		return(dmnptr);
	    *(sdptr - 1) = '\0';
	    sprintf (domain, "%s.%s",  value, official);
	    sprintf (dmnroute -> dm_buf, "%s %s", domain, tbuf);
	    dmnroute -> dm_argc = str2arg (dmnroute -> dm_buf, DM_NFIELD,
		dmnroute -> dm_argv, (char *)0);
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "Domain value '%s' from '%s', o = '%s'",
		domain, dmnptr -> dm_show, official);
#endif
	    return (dmnptr);
	}
    }

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "'%s' not found", value);
#endif
    return ((Domain *) NOTOK);
}

/* ***********  GIVEN VALUE, FIND HOST ROUTE  ******************** */

/* This will take any value, and map it into a domain route.            */
/* It is optimised for val being a fully qualified domain               */

#ifndef BOTHEND

Domain *
	dm_v2route (value, domain, dmnroute)
    char *value;                /* what the use provides                */
    char *domain;               /* full domain value                    */
    register Dmn_route *dmnroute;        /* source route                */
{
    Dmn_route tmpdmn;           /* hold parsed domain string            */
    register Domain *dmnptr;
    int sublen;                 /* length of right-hand to look up      */
    int ind;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "dm_v2route (%s)", value);
#endif
    (void) strcpy (tmpdmn.dm_buf, value);
    tmpdmn.dm_argc = cstr2arg (tmpdmn.dm_buf, DM_NFIELD, tmpdmn.dm_argv, '.');
    if (tmpdmn.dm_argc == NOTOK) {
#ifdef  DEBUG
	ll_log (logptr, LLOGTMP, "Cstr2arg failed (%s)", tmpdmn.dm_buf);
#endif
	return ( (Domain *)NOTOK);
    }

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "%d fields", tmpdmn.dm_argc);
#endif

    dmnroute -> dm_argc = 1;    /* initialize with something safe */
    (void) strcpy (dmnroute -> dm_buf, value);
    dmnroute -> dm_argv[0] = dmnroute -> dm_buf;

    tmpdmn.dm_argc--;

    for (ind = sublen = 0; ind <= tmpdmn.dm_argc; ind++)
    {
	if (ind != 0)
		tmpdmn.dm_buf[sublen - 1] = '.';
				/* undo cstr2arg!                       */
	sublen += strlen (tmpdmn.dm_argv[ind]);
				/* how much to peel off, from right     */
				/* the +1 is for the delimiter          */
	if (ind != tmpdmn.dm_argc)
		sublen++;       /* point to next if not last            */
	if ((dmnptr = dm_nm2struct (&value[sublen])) == (Domain *) NOTOK)
		continue;	/* failed				*/
				/* this is the value			*/
	switch(dm_k2val (dmnptr, tmpdmn.dm_argv[0], domain, dmnroute)){
	case MAYBE:
	    return( (Domain *)MAYBE);
	case OK:
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR,
		"%s' hit in domain table '%s' with '%s'",
		value, dmnptr -> dm_show, dmnroute -> dm_buf);
#endif
	   return (dmnptr);
	}
    }
    return (dm_sd2route (value, domain, dmnroute));
}

#else   BOTHEND

LOCFUN Domain *
dm_rv2route (value, domain, dmnroute)
    char *value;                /* what the use provides                */
    char *domain;               /* full domain value                    */
    register Dmn_route *dmnroute;        /* source route                */
{
    Dmn_route tmpdmn;           /* hold parsed domain string            */
    register Domain *dmnptr;
    int sublen;                 /* length of right-hand to look up      */
    int ind;
    char buf[LINESIZE];

    (void) strcpy (tmpdmn.dm_buf, value);
    tmpdmn.dm_argc = cstr2arg (tmpdmn.dm_buf, DM_NFIELD, tmpdmn.dm_argv, '.');
    if (tmpdmn.dm_argc == NOTOK) {
#ifdef  DEBUG
	ll_log (logptr, LLOGTMP, "Cstr2arg failed (%s)", tmpdmn.dm_buf);
#endif
	return ( (Domain *)NOTOK);
    }

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "%d fields", tmpdmn.dm_argc);
#endif

    tmpdmn.dm_argc--;
    for (ind = sublen = 0; ind <= tmpdmn.dm_argc; ind++)
    {
	if (ind != 0)
		tmpdmn.dm_buf[sublen - 1] = '.';
				/* undo cstr2arg!                       */
	sublen += strlen (tmpdmn.dm_argv[ind]);
				/* how much to peel off, from right     */
				/* the +1 is for the delimiter          */
	if (ind != tmpdmn.dm_argc)
		sublen++;       /* point to next if not last            */
	if ((dmnptr = dm_nm2struct (&value[sublen])) == (Domain *) NOTOK)
		continue;	/* failed				*/
				/* this is the value			*/
	switch(dm_k2val (dmnptr, tmpdmn.dm_argv[0], domain, dmnroute)){
	case MAYBE:
	    return( (Domain *)MAYBE);
	case OK:
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR,
		"%s' hit in domain table '%s' with '%s'",
		value, dmnptr -> dm_show, dmnroute -> dm_buf);
#endif
	    return (dmnptr);
	}
    }
    return ( (Domain *) 0);     /* not found */
}

Domain *
	dm_v2route (value, domain, dmnroute)
    char *value;                /* what the use provides                */
    char *domain;               /* full domain value                    */
    Dmn_route *dmnroute;        /* source route                         */
{
    Domain *dmnptr;
    char *reverse;              /* to handle bigend mess                */
    extern char *ap_dmflip();

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "dm_v2route (%s)", value);
#endif
    dmnroute -> dm_argc = 1;    /* initialize with something safe */
    (void) strcpy (dmnroute -> dm_buf, value);
    dmnroute -> dm_argv[0] = dmnroute -> dm_buf;

    if ((dmnptr = dm_rv2route (value, domain, dmnroute)) != (Domain *)0)
	return (dmnptr);
    reverse = ap_dmflip (value);
    if( (dmnptr = dm_rv2route (reverse, domain, dmnroute)) != (Domain *)0){
	free (reverse);
	return (dmnptr);
    }
    dmnptr = dm_sd2route (value, reverse, domain, dmnroute);
    free (reverse);
    return (dmnptr);
}
#endif  BOTHEND
