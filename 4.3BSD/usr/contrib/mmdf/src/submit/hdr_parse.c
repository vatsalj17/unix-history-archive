#include "util.h"
#include "mmdf.h"
#include "hdr.h"
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

extern struct ll_struct *logptr;

/* basic processing of incoming header lines */

hdr_parse (src, name, contents)   /* parse one header line              */
    register char *src;           /* a line of header text              */
    char *name,                   /* where to put field's name          */
	 *contents;               /* where to put field's contents      */
{
    extern char *compress ();
    char linetype;
    register char *dest;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "hdr_parse()");
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
