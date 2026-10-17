/*  Translate off-net address */

/*  Apr 81  K. Harrenstein  Initially written for SRI
 *  Apr 81  J. Pickens      Defensive initialization, blank line after header
 *  May 81  A. Knutsen      May have done more SRI Hacking
 *  Jun 81  D. Crocker      Changed indenting of data to preserve same
 *                          offset (after the colon) as was in input line
 *                          minor additional cleanup
 *  Sep 81  A. Knutsen      +itmlen >= MAXLEN, rather than >
 *  Apr 82  D. Crocker      reset 'fail' at each call to amp_hdr
 *  Jan 83  D. Kingston     fixed handling of io with ap_1adr().
 *  Jun 83  D. Kingston     Changed handling of bad addresses.
 */


#include "util.h"
#include "mmdf.h"
#include "ap.h"
#include "ap_lex.h"
#include "ch.h"

extern LLog     *logptr;
extern char     *locname;
extern char     *locdomain;
extern char     *locmachine;
extern int      ap_outtype;        /* 733 or 822 */
extern int      ap_perlev;
extern int      getach ();
extern AP_ptr   findap ();

#if DEBUG > 1
extern char *namtab[], *typtab[];	/* In ap_1adr.c */
extern char debug;			/* In ap_1adr.c */

char tdebug;				/* Top-level debug flag */
#endif

#define MAXCOL 78                 /* # of col positions to use */
#define MAXLEN 20                 /* Max length of field name */

LOCVAR int     pcol;              /* Printing col position */
LOCVAR int     nadrs;             /* # of addresses hacked */
LOCVAR int     nonempty;          /* true if we have printed something on
				   * on this line besides the itemname and
				   * and indent whitespace
				   */
LOCVAR int     savech;            /* Char to re-read, else -1 */
LOCVAR char    aread;             /* Set when reading address line in header */
LOCVAR int     lastch;            /* Last char read                     */
LOCVAR int     gotfrom;           /* have we got a source yet?          */

LOCVAR short   itmind;            /* indent for continuation lines      */
LOCVAR short   itmlen;
LOCVAR char    itmbuf[MAXLEN];
LOCVAR char    *itmptr;           /* normally == itmbuf except for Resent */

LOCVAR char   *afldtab[] =
{                                 /* Fields which contain addresses to
				     translate */
    "To",
    "CC",
    "bcc",
    "From",
    "Sender",
    "Reply-to",
    "Resent-From",
    "Resent-Sender",
    "Resent-To",
    "Resent-Cc",
    "Resent-Bcc",
/* Archaic */
    "Resent-By",
    "Remailed-From",
    "Remailed-To",
    "Remailed-By",
    "Redistributed-From",
    "Redistributed-To",
    "Redistributed-By",
    0,
};

LOCVAR FILE * xin, *out;
LOCVAR long curpos;               /* current char position in file      */
				  /* needed because ftell not in v6     */
long
	amp_hdr (chanptr, ain, aout)
Chan *chanptr;                    /* channel that does not need mapping */
FILE *ain, *aout;
{
    char fromsite[FILNSIZE];
    char *ourdomain;
    register AP_ptr aptmp;
    int     amp_fail;
    int     res;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "amp_hdr ()");
#endif

    xin = ain;
    out = aout;                   /* Set global streams */
    curpos = 0L;                  /* reset the file position hack */
    gotfrom = FALSE;

    if (chanptr == (Chan *) 0) {
	(void) strcpy (fromsite, locname);
	ourdomain = locdomain;
    } else {
	(void) strcpy (fromsite, chanptr -> ch_lname);
	ourdomain = chanptr -> ch_ldomain;
    }

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "otyp=%d, from='%s.%s'",
				ap_outtype, fromsite, ourdomain);
#endif
    savech = lastch = -1;         /*  defensive initialization */
    aread = FALSE;

    while (getaitm ())            /* Scan header lines */
    {
#if DEBUG > 1
	if (tdebug)
	    fprintf (stderr, "Parse: ");
#endif
	ap_clear();

	amp_fail = FALSE;
	for (aread = TRUE, out_init (itmlen); ; )
	{
	    if (ap_pinit (getach) == (AP_ptr) NOTOK)
	    {                     /* problem during parse               */
		ll_log (logptr, LLOGTMP, "problem parsing message");
		fprintf (out, "PROBLEM PARSING MESSAGE (%s): ;\n", fromsite);
		fflush (out);
		return (NOTOK);
	    }

	    res = ap_1adr ();     /* Parse one addr */
	    if (res == DONE)      /* DONE */
	    {
#if DEBUG > 1
		if (tdebug)
		    fprintf (stderr, ", DONE.");
		ll_log (logptr, LLOGBTR, "amp 1adr done");
#endif
		break;
	    }
	    else if (res == NOTOK)
	    {
		amp_fail = TRUE;      /* pass the garbage and generate warning */
#if DEBUG > 1
		ll_log (logptr, LLOGBTR, "amp reject '%s'", namtab[ap_llex]);
		if (tdebug)
		    fprintf (stderr, "\nReject: %s\n", namtab[ap_llex]);
#endif
	    }
	    else                  /* process the good part */
	    {
#if DEBUG > 1
		if (tdebug) {
		    fprintf (stderr, "\n\nAccept\n");
		    pretty (ap_pstrt);
		    fprintf (stderr, "\n\n");
		}
#endif

/*  Here do the check to find sending site, for use with any address that
 *  lacks a host spec.  This won't work for multi-level addressing (should
 *  copy ALL site specs seen) but we can worry about that later.  The
 *  current algorithm (JRP) is to pick up the first valid hostname field
 *  from either from:  or sender:, whichever comes first.  The algorithm
 *  really needs two passes, which it doesn't currently have..  If, for
 *  example, a to: field comes before the from: field, then patching will
 *  not occur.  Another problem happens when from: comes after sender:, and
 *  from: has no host.  In this case from: gets the host of the sender.
 *  Despite these deficiencies, the mapping should work most of the time.
 *
 *  DPK@BRL, 25 Oct 84
 *  We now properly handle the use of resent lines.  When you get a resent
 *  line, it will be necessary stop using old ourdomain, instead we need to
 *  pickup the sending send from the resent-from line and use that site
 *  to fix resent-{to,cc} lines that lack domains.
 */
		if (prefix("resent-", itmbuf)) {
			itmptr += 7; gotfrom = FALSE;
		} else if (prefix("remailed-", itmbuf)) {
			itmptr += 9; gotfrom = FALSE;
		} else if (prefix("redistributed-", itmbuf)) {
			itmptr += 14; gotfrom = FALSE;
		}  /* Fall into ... */

		if ((!gotfrom) &&
			(lexequ (itmptr, "from") || lexequ (itmptr, "sender")))
		{
#if DEBUG > 1
		    if (tdebug)
			fprintf (stderr, "Checking fromsite: \"");
#endif
		    if (aptmp = findap (ap_pstrt, APV_DOMN))
		    {
			gotfrom = TRUE;
			(void) strcpy (fromsite, aptmp -> ap_obvalue);
			ourdomain = (char *) 0;
				/* SEK ap_normalize will add a suitable */
				/* domain if not already there          */
		    }

#if DEBUG > 1
		    if (tdebug)
			fprintf (stderr, "%s\"\n", fromsite);
#endif
		}
	    }

	    res = out_adr (chanptr, fromsite, ourdomain, ap_pstrt);

	    ap_sqdelete (ap_pstrt, (AP_ptr) 0);
	    ap_free (ap_pstrt);   /* delete full string                 */
	    ap_pstrt = (AP_ptr) 0;

	    if(res == MAYBE)
		return( (long)res);
	}

	(void) putc ('\n', out);
	if (ap_perlev) {        /* If still nested */
#if DEBUG > 1
	    if (tdebug)
		fprintf (stderr, "Still nested at level %d!\n", ap_perlev);
#endif
	    ll_log (logptr, LLOGTMP, "amp_hdr nested, level %d", ap_perlev);
	    amp_fail++;
	}

	if (amp_fail)
#ifndef UCL
	    fprintf (out, "MMDF-Warning:  Parse error in original version of preceding line at %s.%s\n", locname, locdomain);
#else
	    fprintf (out, "MMDF-Warning:  Parse error in original version of preceding line at %s.%s.%s\n", locmachine, locname, locdomain);
#endif
	aread = FALSE;
    }

    (void) putc ('\n', out);             /*  Put a blank line after header  */
    fflush (out);
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "amp parse done");
#endif
    switch ((int)ap_pstrt)
    {
	case OK:
	case NOTOK:
	    break;

	default:
	    ap_sqdelete (ap_pstrt, (AP_ptr) 0);
	    ap_free (ap_pstrt);      /* delete full string         */
	    ap_pstrt = (AP_ptr) 0;
    }

    return ((ferror(out) || ferror(xin)) ? NOTOK : curpos);
}

int
	getach ()
{
    register int    c;

    if (savech >= 0)
    {
	if(aread && lastch == '\n')
	    return (lastch = 0);        /* Double end of file for addrlist */
	c = savech;
	savech = -1;
	return (lastch = c);
    }
    if (feof (xin))     /* in case we are called after getting eof */
	return (0);
    c = getc (xin);
    if (feof (xin))     /* ended now */
	return (0);

    curpos++;
    if (aread)
	if (lastch == '\n' && !(c == ' ' || c == '\t'))
	{
#if DEBUG > 1
	    ll_log (logptr, LLOGFTR, "getach end of field");
#endif
	    savech = c;           /* Save for copying */
	    return (0);
	}
    return (lastch = c);          /*  xfer one char               */
}

/* Return true when starting to read from an address field.
 *      itmbuf and itmlen will be set.
 * Return false when ready to copy message text.
 */
getaitm ()
{
    register int    c;
    register char  *cp,
		  **tabv;

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "getaitm ()");
#endif

again: 
    itmptr = itmbuf;            /* RESET possible fiddling above */
    for (itmlen = 0, cp = itmbuf, itmind = 0; ;)
    {                             /* Collect field identifier */
	if ((c = getach ()) == (char ) -1)
	    goto err;

	switch (c)
	{
	     case '\n':
		if (itmlen != 0)
		    break;
	     case '\0':
		return (FALSE);
	}

	(void) putc (c, out);            /* Continuous "echo" */

	switch (c)
	{
	    case ':':             /* Field name collected */
		if (itmlen == 0)
		    goto err;
		*cp = 0;
#ifdef DEBUG
		ll_log (logptr, LLOGFTR, "hdr: '%s'", itmbuf);
#endif
		for (itmind = itmlen + 1;
			isspace (c = getach ()) && c != '\n'; itmind++)
		    (void) putc (c, out);    /* echo the initial white space */

		savech = c;       /* re-read the non-space */
		lastch = 0;

		/*  See if addr-type  */
		for (tabv = afldtab; *tabv; tabv++)
		    if (lexequ (*tabv, itmbuf)) {
			if (savech == '\n') {
			    (void) putc (' ', out);
			    itmind++;
			}
			return (TRUE);
		    }

		goto flsh;

	    case ' ':
		if (itmlen != 0)
		    continue;
		goto err;         /* this should never happen */

	    default:
		if (iscntrl (c))  /* not sure why, but... (dhc) */
		    goto err;

		*cp++ = c;

		if (++itmlen >= MAXLEN)   /* Fieldname too big, so just */
		    goto flsh;            /* flush and restart */
	}
    }

flsh:
    aread = TRUE;
    while ((c = getach ()) != 0)
	(void) putc (c, out);
    aread = FALSE;

    goto again;

err:
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "amp getfield fail on char '%c'", c);
#endif
#if DEBUG > 1
    if (tdebug)
	fprintf (stderr, "Getfield failure, char '%c'\n", c);
#endif
    return (0);
}
/*
 * Search parse list for specific object type, returns ptr thereto.
 * Works if given zero pointer, so findap(ap->ap_chain, FOO) (ie to
 * find next occurrence of FOO) won't blow up even if
 * already at end of list.
 */

AP_ptr
	findap (aap, nval)
    AP_ptr aap;
{
    register AP_ptr ap;

    for (ap = aap; ap && (ap -> ap_obtype != APV_NIL); ap = ap -> ap_chain)
	if (ap -> ap_obtype == nval)
	    return (ap);
	else
	    if (ap -> ap_ptrtype == APP_NIL)
		break;
    return ((AP_ptr) 0);
}

#if DEBUG > 1
pretty (ap)
register AP_ptr ap;
{
    register int    depth;

    ll_log (logptr, LLOGFTR, "pretty ()");

    for (depth = 1;; ap = ap -> ap_chain)
    {
	switch (ap -> ap_obtype)
	{
	    case APV_NIL: 
		ll_log (logptr, LLOGBTR, "end on null ap_obvalue");
		fprintf (stderr, "End on null ap_obvalue\n");
		return;
	    case APV_EGRP: 
	    case APV_EPER: 
		depth -= 2;
		break;
	}

	ll_log (logptr, LLOGFTR, "%.*s%-9s%s%s",
		depth, "          ", typtab[ap -> ap_obtype],
		ap -> ap_obvalue,
		(ap -> ap_ptrtype == APP_NXT) ? "\t(NXT)" : "");
	fprintf (stderr, "%.*s%-9s%s%s\n",
		depth, "          ", typtab[ap -> ap_obtype],
		ap -> ap_obvalue,
		(ap -> ap_ptrtype == APP_NXT) ? "\t(NXT)" : "");

	switch (ap -> ap_obtype)
	{
	    case APV_NGRP: 
	    case APV_NPER: 
		depth += 2;
	}
	if (ap -> ap_ptrtype == APP_NIL)
	{
	    ll_log (logptr, LLOGBTR, "end on null pointer");
	    fprintf (stderr, "End on null pointer\n");
	    break;
	}
    }
}
#endif



out_init (len)
short len;
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "out_init ()");
#endif

    nadrs = 0;
    nonempty = FALSE;
    pcol = len + 1;
}

out_adr (chanptr, dflsite, dfldomain, ap)
Chan *chanptr;
char *dflsite, *dfldomain;
register AP_ptr ap;
{
    int len;
    char *addrp;
    AP_ptr lrval;

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "out_adr ()");
#endif

    if (ap -> ap_obtype == APV_NIL)
	return(OK);               /* Ignore null stuff */

    if (nadrs != 0)               /* not the first address on the line  */
    {
	fputs (", ", out);
	pcol += 2;
    }

    ap = ap_normalize (dflsite, dfldomain, ap, chanptr);
    if(ap == (AP_ptr)MAYBE)
	return(MAYBE);
#if DEBUG > 1
    if (tdebug)
	pretty (ap);
#endif

    lrval = ap_t2s (ap, &addrp);       /* format it */
    if(lrval == (AP_ptr)MAYBE)
	return(MAYBE);
    else if(lrval == (AP_ptr)NOTOK)
	return(NOTOK);
#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "out: '%s'", addrp);
#endif

    if ((len = strlen (addrp)) > 0)
    {                               /* print it */
	if ((pcol += len) >= MAXCOL && nonempty)
	{
	    pcol = itmind + len;
	    fprintf (out, "\n%.*s", itmind, "                    ");
	}

	fputs (addrp, out);
	nadrs++;
	nonempty = TRUE;
    }
    free (addrp);
    return(OK);
}
