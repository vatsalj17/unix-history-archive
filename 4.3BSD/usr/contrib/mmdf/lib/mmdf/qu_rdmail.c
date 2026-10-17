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
/*  DM_IO:  Channel interaction with Deliver                            */
/*                                                                      */
/*  Nov, 80 Dave Crocker    convert to V7                               */

#include "adr_queue.h"

extern char *strdup();
extern char *strncpy();

extern LLog   *logptr;
LOCVAR char *qu_host,             /* to store 1st adr of msg            */
	   *qu_adr,
	    qu_info[2];

LOCVAR struct rp_construct
	    rp_dmiook =
{
    RP_OK, '\0'
};
/* *************  (qu_)  DELIVER MAIL-READING SUB-MODULE  ************* */


qu_rinit (info, retadr, dohdr)   /* ready to get a message             */
char   *info;			  /* where to stuff init ifno           */
char   *retadr;			  /* where to stuff return address      */
int     dohdr;                    /* massage address headers            */
{
    int     len;
    int     retval;
    char    *arglist[20];
    char    host[LINESIZE],
	    adr[LINESIZE],
	    linebuf[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu_rinit (dohdr=%d)", dohdr);
#endif

    if (rp_isbad (retval = qu_rrec (linebuf, &len)))
	return (retval);        /* get message initialization info      */

    str2arg (linebuf, 20, arglist, (char *) 0);
    if (lexequ ("end", arglist[0]))
	return (RP_DONE);
    if (!lexequ ("msg", arglist[0]))
    {
	ll_log (logptr, LLOGFAT,
		    "qu_rinit err reading new msg:  '%s'", arglist[0]);
	return (RP_PARM);
    }
    (void) strncpy (retadr, arglist[2], ADDRSIZE-1);
    if (rp_isbad (retval = qu_minit (arglist[1], dohdr)))
	return (retval);          /* open the message text              */

    qu_wrply ((RP_Buf *) &rp_dmiook, rp_conlen (rp_dmiook));

    if (rp_isgood (retval = qu_radr (host, adr)))
    {
	if (!isnull (host[0]))
	    qu_host = strdup (host);
	qu_adr = strdup (adr);
	(void) strcpy (info, qu_info);
    }
    return (retval);
}

qu_radr (host, adr)              /* read next address from Deliver     */
char   *host,			  /* where to put name of next host     */
       *adr;			  /* where to put rest of address       */
{
    int       len;
    short     retval;
    char      linebuf[LINESIZE];
    char      *arglist[20];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu_radr");
#endif

    if (qu_adr != 0)              /* fake read of 1st address           */
    {
	(void) strncpy (adr, qu_adr, ADDRSIZE-1);
	free (qu_adr);
	qu_adr = 0;
	if (qu_host != 0)
	{
	    (void) strncpy (host, qu_host, ADDRSIZE-1);
	    free (qu_host);
	    qu_host = 0;
	}
	else
	    host[0] = '\0';
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "1st msg/call: '%s'@'%s'", adr, host);
#endif
	return (RP_OK);
    }

    retval = qu_rrec (linebuf, &len);
    if (rp_gval (retval) != RP_OK)
	return (retval);

    str2arg (linebuf, 20, arglist, (char *) 0);
    if (lexequ ("aend", arglist[0]))
	return (RP_DONE);
    if (lexequ ("hend", arglist[0]))
	return (RP_HOK);
    if (!lexequ ("addr", arglist[0]))
    {
	ll_log (logptr, LLOGFAT,
		    "qu_radr err reading addr:  '%s'", arglist[0]);
	return (RP_PARM);
    }
    (void) strncpy (host, arglist[1], ADDRSIZE-1);
    (void) strncpy (adr, arglist[2], ADDRSIZE-1);
				  /* copy host & address parts          */
    qu_info[0] = 'm';

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "('%s'@'%s')", adr, host);
#endif

    return (RP_OK);
}
/**/

qu_rtinit (theseek)              /* initialize for reading text        */
    long theseek;                /* position in message to start at    */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu_rtinit (%ld)", theseek);
#endif

    return (qu_rsinit (theseek));   /* => stream init                     */
}

qu_rtxt (buffer, len)             /* read next portion of msg text      */
char   *buffer;                   /* where to stuff buffer (not line)   */
int    *len;                     /* where to stuff len (BUF not LINE)  */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qu_rtxt");
#endif

    return (qu_rstm (buffer, len));
}

qu_rend ()
{
	return( qu_mend());
}
