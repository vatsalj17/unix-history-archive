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

#include "ch.h"

extern LLog   *logptr;

/* ***********  (po_)  POBOX PICKUP-MAIL I/O SUB-MODULE  ************** */

po_init ()                /* get ready for mail pickup          */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "po_init ()");
#endif
    return (RP_OK);
}

/* ARGSUSED */

po_end (type)                     /* done with mail process             */
short     type;
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "po_end ()");
#endif
#ifdef RUNALON
    printx ("po_end, ");
#endif
    return (RP_OK);
}

po_sbinit ()                      /* initialize local submission        */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "po_sbinit ()");
#endif
    return (RP_OK);
}

po_sbend ()                       /* done with submission               */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "po_sbend ()");
#endif
#ifdef RUNALON
    printx ("po_sbend, ");
#endif
    return (RP_DONE);
}
/*                    PROCESS REPLIES                                 */

po_rrply (valstr, len)           /* get a reply from remote process    */
struct rp_bufstruct *valstr;      /* where to stuff copy of reply text  */
int    *len;                      /* where to indicate text's length    */
{
    short     retval;
    char   *rplystr;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "po_rrply()");
#endif

    retval = po_rrec ((char *) valstr, len);
    if (rp_gval (retval) != RP_OK)
	return (retval);

#ifdef RUNALON
    switch (rp_gval (valstr -> rp_val))
    {
	case 'd': 
	    valstr -> rp_val = RP_DONE;
	    break;
	case 'y': 
	    valstr -> rp_val = RP_OK;
	    break;
	case 'm': 
	    valstr -> rp_val = RP_MOK;
	    break;
	case 'a': 
	    valstr -> rp_val = RP_AOK;
	    break;
	case 'n': 
	    valstr -> rp_val = RP_NO;
	    break;
    }
#endif

    rplystr = rp_valstr (valstr -> rp_val);
    if (*rplystr == '*')
    {				  /* replyer did a no-no                */
	ll_log (logptr, LLOGTMP, "ILLEGAL REPLY: (%s)", rplystr);
	valstr -> rp_val = RP_RPLY;
    }
#ifdef DEBUG
    else
	ll_log (logptr, LLOGFTR, "(%s)'%s'", rplystr, valstr -> rp_line);
#endif

    return (RP_OK);
}
/*         READ DATA FROM LOCAL MAIL (SUB)PROCESS                     */

po_rrec (linebuf, len)           /* read one "record"                    */
char   *linebuf;		  /* where to stuff the text              */
int      *len;                    /* where to stuff the length count      */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "po_rrec ()");
#endif

    switch (*len = gcread (stdin, linebuf, LINESIZE - 1, "\000\n\377"))
    {				  /* a record == one line                 */
	case NOTOK: 
	    ll_log (logptr, LLOGFAT, "read error");
	    return (RP_LIO);

	case OK: 
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "eof");
#endif
	    return (RP_EOF);

	case 1: 
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "DONE");
#endif
	    return (RP_DONE);     /* the only valid one-char record     */
    }
    linebuf[*len] = '\0';	  /* keep things null-terminated        */
    *len -= 1;
#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "(%d)'%s'", *len, linebuf);
#endif
    return (RP_OK);
}
/*            WRITE DATA TO LOCAL MAIL (SUB)PROCESS                   */

po_wrec (buf, len)           /* write a record/packet              */
char    *buf;			  /* chars to write                     */
int	len;                      /* number of chars to write           */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "po_wrec () (%d)'%s'", len, buf ? buf : "");
#endif

    if (!isstr(buf) && len == 0)
    {				  /* send an end-of-stream              */
#ifdef RUNALON
	printx ("po_wrec (eof), ");
#endif
	putchar ('\0');
    }
    else
    {
	fwrite (buf, 1, len, stdout);
	putchar ('\n');
    }
    fflush (stdout);
    if (ferror (stdout))
    {
	ll_log (logptr, LLOGFTR, "write error");
	return (RP_LIO);
    }
    return (RP_OK);
}
/**/

po_wstm (buffer, len)            /* write next part of char stream     */
char    buffer[];		  /* chars to write                     */
int     len;                      /* number of chars to write           */
{
    register char   doflush;      /* flush all the text out?            */

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "po_wstm ()");
#endif

    doflush = (buffer == 0 && len == 0);

    if (!doflush)
	buffer[len] = '\0';
#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "po_wstm (): (%d)\"%s\"",
	    len, (doflush) ? "[EOF]" : buffer);
#endif
    if (doflush)
	fflush (stdout);
    else
	fwrite (buffer, 1, len, stdout);

    if (ferror (stdout))
    {
	ll_log (logptr, LLOGFAT, "write error");
	return (RP_LIO);
    }
    return (RP_OK);
}
