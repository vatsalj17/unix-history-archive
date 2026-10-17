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
/*  LM_WTMAIL:  Submit local mail                                       */
/*                                                                      */
/*  Jun, 80 Dave Crocker    make immediate local delivery conditional   */

extern struct ll_struct *logptr;

/* ************  (mm_)  LOCAL MAIL-WRITING SUB-MODULE  **************** */

mm_winit (vianet, info, retadr)  /* initialize for one message         */
char    *vianet;		  /* what channel coming in from        */
char    *info,			  /* general info                       */
        *retadr;		  /* return address for error msgs      */
{
    short     retval;
    char   infoline[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mm_winit (%s, %s, %s)",
	    vianet ? vianet : "", info, retadr ? retadr : "");
#endif

    if (vianet != (char *)0)       /* we are relay; add name of source   */
	sprintf (infoline, "ti%s*",vianet);
    else
	infoline[0] = '\0';

    strcat (infoline, info);

    retval = mm_wrec (infoline, strlen (infoline));

    /* if -r switch used, retadr == 0 */
    /* if no return address available, retadr[0] == '\0' */

    if (isstr(retadr))
	retval = mm_wrec (retadr, strlen (retadr));
    else if (retadr)	/* Don't send a null if handed "" */
	retval = mm_wrec (" ", 1);

    return (retval);
}
/**/

mm_wadr (host, adr)              /* send one address spec to local     */
char    *host,			  /* "next" location part of address    */
        *adr;			  /* rest of address                    */
{
    short     retval;
    char adrbuf[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mm_wadr()");
#endif

    if (isstr (host)) {
	sprintf (adrbuf, "%s@%s", adr, host);
	adr = adrbuf;
    }

    retval = mm_wrec (adr, strlen (adr));
    return (retval);
}

mm_waend ()                      /* end of address list                */
{
    short     retval;

    if (rp_isbad (retval = mm_wrec ("!", 1)))
	return (retval);
    return (RP_DONE);
}

mm_wtxt (buffer, len)            /* send next part of msg text         */
char    *buffer;		  /* the text                           */
int     len;                      /* length of text                     */
{
    return (mm_wstm (buffer, len));
}

mm_wtend ()                      /* end of message text                */
{
    return (mm_wrec ((char *) 0, 0));     /* flush buffer and indicate end      */
}
