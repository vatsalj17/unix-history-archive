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

/* **************  (ph_)  PHONE MAIL-READING SUB-MODULE  ************** */

LOCVAR short  ph_nadrs;             /* number of addresses this message   */
LOCVAR  long ph_nchrs;            /* number of chars in msg text        */

ph_rinit (info, retadr)          /* get initialization info for msg    */
char   *info,			  /* where to put general init info     */
       *retadr;			  /* where to put return address        */
{
    short     retval;
    int       len;
    char    linebuf[LINESIZE];
    char   *fromptr,
           *toptr;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_rinit");
#endif

    ph_nadrs = 0;
    ph_nchrs = 0L;

    retval = ph_rrec (linebuf, &len);
    if (rp_gval (retval) != RP_OK)
	return (retval);

    for (fromptr = linebuf, toptr = info;
	    *fromptr != '\n' && *fromptr != ';';
	    *toptr++ = *fromptr++)
	switch (*fromptr)	  /* mailing mode                         */
	{			  /* make sure only has ok values         */
	    case ' ': 
	    case 'A':
	    case 'a': 
	    case 'M':
	    case 'm': 
	    case 'S':
	    case 's': 
		break;		  /* ok to copy to info field             */

	    default: 		  /* invalid code                         */
		ll_log (logptr, LLOGFAT,
			"illegal mode value: %c", *fromptr);
		return (RP_NO);
	};
    *toptr = '\0';

    for (toptr = retadr, fromptr++;
	    !isnull (*fromptr) && *fromptr != '\n';
	    *toptr++ = *fromptr++);
    *toptr = '\0';		  /* copy return address                */

    return (RP_OK);
}
/**/

ph_radr (host, adr)              /* get an address spec from remote    */
char   *host,			  /* where to stuff name of next host   */
       *adr;			  /* where to stuff rest of address     */
{
    short     retval;
    int       len;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_radr()");
#endif

    *host = '\0';		  /* no-op this field                   */

    retval = ph_rrec (adr, &len);
    if (rp_gval (retval) != RP_OK)
	return (retval);

    ph_nadrs++;

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "'%s'", adr);
#endif

    return (RP_OK);
}

ph_rtxt (buffer, len)            /* read next part of msg text         */
char   *buffer;			  /* where to stuff copy of text        */
int     *len;                      /* where to indicate text's length    */
{
    short     retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_rtxt()");
#endif

    retval = ph_rstm (buffer, len);
    if (rp_gval (retval) != RP_OK)
	return (retval);

    ph_nchrs += (long) *len;

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "'%s'", buffer);
#endif

    return (RP_OK);
}

ph_rtend ()
{                               /* end of message text */
    return (RP_OK);
}
