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
extern struct ll_struct   *logptr;

/* *******  (po_)  POBOX (PICKUP) MAIL-WRITING I/O SUB-MODULE  ******** */

/* ARGSUSED */

po_winit (vianet, info, retadr)  /* pass msg initialization info       */
char    vianet[];		  /* what channel coming in from        */
char    info[],			  /* general info                       */
        retadr[];		  /* return address for error msgs      */
{
    short     retval;
    char linebuf[LINESIZE];

/* DBG:  make sure info has right form */

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "po_winit");
#endif

    sprintf (linebuf, "%s;%s", info, retadr);
				  /* slave:  <info> ';'<retadr>    */
    retval = po_wrec (linebuf, strlen (linebuf));

    return (retval);
}
/**/

/*ARGSUSED*/
po_wadr (host, adr)              /* send one address spec to local     */
char    *host,			  /* "next" location part of address    */
        *adr;			  /* rest of address                    */
{
    short     retval;
    char adrbuf[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "po_wadr()");
#endif

    strcpy (adrbuf, adr);
				  /* tell the user, if watching         */
    retval = po_wrec (adrbuf, strlen (adrbuf));

    return (retval);
}

po_waend ()                      /* end of address lsit                */
{
    short     retval;
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "po_waend");
#endif

    if (rp_isbad (retval = (po_wrec ((char *) 0, 0))))
	return (retval);
    return (RP_DONE);
}
/**/

po_wtxt (buffer, len)            /* send next part of msg text         */
char    buffer[];		  /* the text                           */
short     len;                      /* length of text                     */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "po_wtxt()");
#endif

    return (po_wstm (buffer, len));
}

po_wtend ()                      /* end of message text                */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "po_wtend");
#endif

    po_wstm ((char *) 0, 0);      /* flush the output buffer            */
    return (po_wrec ((char *) 0, 0));  /* signal end of stream               */
}
