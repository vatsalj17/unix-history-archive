/*
 *                      U U _ W T M A I L . C
 *
 *      Part of the UUCP channel of the MMDF Mail system developed
 *      by Doug Kingston at the US Army, Ballistics Research Laboratory.
 *                              <DPK@BRL>
 *
 *      Low-level IO to handle mail delivery to the UUCP
 *      queuing system.  Calls are made from qu2uu_send.c.
 *
 *                  Original Version November 1981
 */

#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include "ap.h"

extern struct ll_struct *logptr;
extern Chan *curchan;
extern char *Uuxstr;
extern int errno;
extern int pbroke;

extern int ap_outtype;
extern AP_ptr ap_s2tree ();

/*
 *      -----  Variables Local to this Module  -----
 */
static FILE     *uucpf;                   /* used by POPEN(III) */
static char     nextnode[LINESIZE];
static char     who[LINESIZE];

/**/

/*
 *      uu_wtadr() takes the given host and address and generates
 *      a valid uucp style address and verifies that the given host
 *      is in the mapping tables.  If so, it then invokes UUCP with
 *      the appropriate arguments.
 */
uu_wtadr (host, adr, sender, realfrom)
	char    *host, *adr, *sender, *realfrom;
{
    char        *index(), *rindex();
    FILE        *popen();
    char        *bangptr;
    time_t      timenow;
    char        linebuf[LINESIZE];
    char        *atp, *percentp, *lp;
    AP_ptr      ap, local, domain;

    ap_outtype = AP_733;        /* LMCL */
    /*
     *	Added by Peter C temporarily to munge host names with underscores
     *  and remove any suffix.
     *  Nasty kludge
     */
    for (atp = host; atp = index(atp, '_');)
    	*atp = '.';
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "uu_wtadr()");
    ll_log (logptr, LLOGFTR, "host='%s', adr='%s', sender='%s', realfrom='%s'",
		host, adr, sender, realfrom );
    ll_log (logptr, LLOGFTR, "ap_outtype = %o", ap_outtype);
#endif

    strcpy(nextnode,"");
    strcpy(who,"");

    if ((ap = ap_s2tree (adr)) == (AP_ptr) NOTOK)
    {
	ll_log (logptr, LLOGTMP, "Failure to parse address '%s'", adr);
	return (RP_PARM);
    }
    ap_t2parts(ap, (AP_ptr *)0, (AP_ptr *)0, &local, &domain, (AP_ptr *)0);
    lp = ap_p2s( (AP_ptr)0, (AP_ptr)0, local, domain, (AP_ptr)0);

    atp = index (lp, '@');
    if (atp != (char *)0)
	*atp++ = '\0';
    if (lexequ(atp, host))
	atp = (char *)0;	/* don't make path-to-foo!foo.uucp!user */
	
    percentp = rindex (lp, '%');
    if (percentp != (char *) 0) {
	*percentp = '\0';
    	if (atp)
	    sprintf (adr, "%s!%s!%s", atp, ++percentp, lp);
    	else
	    sprintf (adr, "%s!%s", ++percentp, lp);
    } else if (atp) {
	sprintf (adr, "%s!%s", atp, lp);
    } else
	strcpy(adr, lp);
    free (lp);

    ll_log (logptr, LLOGFST, "address = '%s'", adr);

    if (!isstr(host))
	    strcpy(who, adr);
    else {
	    switch(tb_k2val (curchan -> ch_table, TRUE, host, nextnode)) {
	    case NOTOK:
		return (RP_USER);       /* No such host */
	    case MAYBE:
	    	return (RP_NS);
	    }
	    sprintf(who, nextnode, adr);
    }

    /* Extract first host name for destination */
    if ((bangptr=index (who, '!')) != NULL)
    {
	/* There is at least one relay machine */
	*bangptr++ = '\0';
	strcpy (nextnode, who);
	strcpy(who, bangptr);
    }
    else strcpy(nextnode, "");

    sprintf (linebuf, "%s %s!rmail \\(%s%s\\)",
		Uuxstr, nextnode, *who=='~' ? "\\\\" : "", who);

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "calling uux with <%s>", linebuf);
#endif

    printx ("Queuing UUCP mail for %s via %s...\n",
		who, nextnode);

    if ((uucpf = popen (linebuf, "w")) == NULL) {
	ll_log (logptr, LLOGFAT, "can't popen UUX (errno %d)", errno);
	return (RP_AGN);
    }

    time (&timenow);
    fprintf (uucpf, "From %s %.24s remote from %s\n",
		realfrom, ctime(&timenow), curchan->ch_lname);

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "Done uu_wtadr().");
#endif
    return (RP_OK);
}
/**/

/*
 *      UU_TXTCPY()
 *
 *      The special function of this guy is to grap the "From" line
 *      from the message header and move it to the first line and to
 *      put in standard UUCP form.  There are programs that depend
 *      on this line on less sophisticated systems.  I hate to louse
 *      up a perfectly good RFC733 letter but that's life!
 *
 *      Too refresh your memory, when this module is called, the "From"
 *      line has already been written out onto the pipe.  We now want
 *      copy the rest of the header out onto pipe a line at a time
 *      so that we cat remove the original From line.
 *
 *      SEK - have changed this.  Do not mungle now so we
 *      can use the deliver reformatting.
 *      It seems preferable to leave the orginal From: line
 *      Other sendmail and MMDF systems will prefer this
 *      Older systems will have to lump it
 */

uu_txtcpy()
{
    int     nread;
    char    buffer[BUFSIZ];

#ifdef DEBUG
    ll_log (logptr, LLOGFTR," uu_txtcpy()");
#endif

    qu_rtinit (0L);             /* ready to read the text             */

    nread = sizeof(buffer);
    while (!pbroke && (rp_gval (qu_rtxt (buffer, &nread)) == RP_OK))
    {                             /* send the text                      */
	if (fwrite (buffer, sizeof *buffer, nread, uucpf) == 0) {
	    ll_log (logptr, LLOGFAT, "write on pipe error (errno %d)", errno);
	    ll_log (logptr, LLOGFAT, "pclose returned %d", pclose (uucpf));
	    return (RP_LIO);
	}
    	nread = sizeof(buffer);

    }

    fflush(uucpf);              /* see if the pipe broke */
    if (pbroke) {
	ll_log (logptr, LLOGFAT, "pipe broke -- probably bad host");
	pclose(uucpf);
	return (RP_LIO);
    }

    return (RP_MOK);              /* got the text out                   */
}

/*
 *      uu_wttend()  --  Cleans up after the UUCP
 */
uu_wttend()
{
	if (pclose (uucpf) != 0)
		return (RP_LIO);
	return (RP_MOK);
}

/**/

/*
 *      LOWERFY()  -  convert string to lower case
 */
lowerfy (strp)
	char *strp;
{
	while (*strp = uptolow (*strp))
		strp++;
}
