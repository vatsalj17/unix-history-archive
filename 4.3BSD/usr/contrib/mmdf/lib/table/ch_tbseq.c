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

/*  Sep 81  Dave Crocker      added ch_tclose & ch_tfree, to allow
 *                            handling file descriptor overflow
 *  May 82  Dave Crocker      add ch_tread, to focus input
 *  Jun 82  Dave Crocker      split ch_tio off from ch_table
 *                            if local name, use lname, for proper case
 *  May 84  G. B. Reilly      added in domain support
 */

#include "util.h"
#include "mmdf.h"
#include "ch.h"			  /* has table state strcture def       */
#include "dm.h"			  /* has domain info */

extern Chan **ch_tbsrch;
extern LLog *logptr;
extern char *locname;
extern char *ch_dflnam;

extern Domain **dm_list;


/*******************  FIND HOST NAME IN ANY TABLE  ****************** */

Chan   *
	ch_h2chan (hostr, pos) /* which chan name table is host in?  */
register char  *hostr;		  /* name of host                       */
int	pos;			  /* position */
{
    register Chan  **chanptr;
    char adrstr[LINESIZE];

/*  the list of channel name tables is first searched.  If a hit is found
 *  in the table for the local channel, OK is returned to indicate a local
 *  reference.
 */

    for (chanptr = ch_tbsrch; *chanptr != 0; chanptr++)
    {
	if (tb_k2val ((*chanptr) -> ch_table, TRUE, hostr, adrstr) == OK)
	{			  /* the hostname IS in this table      */
	    if (lexequ (ch_dflnam, (*chanptr) -> ch_name))
	    {
		return ((Chan *) OK); /* local reference                */
	    }
	    return (*chanptr);    /* let caller know which chan         */
	}
    }

    return ((Chan *) NOTOK);      /* host not listed anywhere           */
}
/* ***********  GIVEN Subdomain, FIND Domain  ***************** */



Domain *
	dm_s2dom (subdomain, official, dmbuf)
char	*subdomain;		/* name of subdomain to look up */
char	*official;		/* where to stuff official name */
char	*dmbuf;			/* Domain route buffer */
{
    char *argv[DM_NFIELD];
    char val[LINESIZE];
    register Domain **dmnptr;

    for (dmnptr = dm_list; *dmnptr != 0; dmnptr++)
    {
	if (tb_k2val ((*dmnptr) -> dm_table, TRUE, subdomain, val) == OK) {
	    (void) strcpy(dmbuf, val);
	    str2arg(val, DM_NFIELD, argv, 0);
	    (void) strcpy(official, argv[0]);
    	    return (*dmnptr);
	}
    }

    (void) strcpy (official, subdomain); /* to have something in the buffer */
    (void) strcpy (dmbuf, subdomain);    /* to have something in the buffer */
    return ((Domain *) NOTOK);
}

/* *******  FIND VALUE (address), GIVEN ITS KEY (hostname)  ********* */

tb_k2val (table, first, name, buf) /* use key and return value */
register Table  *table;
int     first;                    /* start at beginning of list?        */
char   *buf;			  /* put value int this buffer          */
register char  *name;		  /* name of ch "member" / key          */
{
    char    host[LINESIZE];

    if (!tb_open (table, first))
	return (NOTOK);		  /* not opened                         */

    while (tb_read (table, host, buf))
    {				  /* cycle through keys                 */
	if (lexequ (name, host))  /* does key match search name?        */
	{
	    compress (buf, buf);  /* get rid of extra white space       */
	    return (OK);
	}
    }

    (void) strcpy (buf, "(ERROR)");
    return (NOTOK);
}
