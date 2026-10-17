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

/* **************  (ph_)  PHONE MAIL-WRITING SUB-MODULE  ************** */

LOCVAR short  ph_nadrs;             /* number of addresses this message   */
LOCVAR  long ph_nchrs;            /* number of chars in msg text        */

/* ARGSUSED */

ph_winit (vianet, info, retadr)  /* pass msg initialization info       */
char    vianet[];		  /* what channel coming in from        */
char    info[],			  /* general info                       */
        retadr[];		  /* return address for error msgs      */
{
    short     retval;
    char  initbuf[LINESIZE];

/* DBG:  make sure info has right form */

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_winit");
#endif

    ph_nadrs = 0;
    ph_nchrs = 0L;

    sprintf (initbuf, "%s;%s", info, retadr);
				  /* slave:  <info> \n <retadr>    */
    retval = ph_wrec (initbuf, strlen (initbuf));

    return (retval);
}
/**/
/*ARGSUSED*/
ph_wadr (host, adr)               /* send one address spec to local     */
char	*host;			  /* not used on phone channel */
char	*adr;			  /* "next" location part of address    */
{
    extern char *strdup ();
    short     retval;
    register char  *adrstr;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_wadr()");
#endif

    ph_nadrs++;

    adrstr = strdup (adr);
				  /* tell the user, if watching         */
    retval = ph_wrec (adrstr, strlen (adrstr));

    free (adrstr);
    return (retval);
}

ph_waend ()                      /* end of address lsit                */
{
    short     retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_waend");
#endif

    if (rp_isbad (retval = ph_wrec ((char *) 0, 0)))
	return (retval);
    return (RP_DONE);
}
/**/

ph_wtxt (buffer, len)            /* send next part of msg text         */
char    buffer[];		  /* the text                           */
short     len;                      /* length of text                     */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_wtxt()");
#endif

    ph_nchrs += (long) len;

    return (ph_wstm (buffer, len));
}

ph_wtend ()                      /* end of message text                */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_wtend");
#endif

    return (ph_wstm ((char *) 0, 0));
				/* flush text buffer / send eos       */
}
