#include "util.h"
#include "mmdf.h"
#include "ch.h"
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

extern Table **tb_list;
#ifdef DEBUG
extern LLog *logptr;
#endif

Table  *tb_nm2struct (name)       /* give pointer to structure for chan  */
register char  *name;		  /* string name of chan                */
{
    register Table **tableptr;

#ifdef DEBUG
    ll_log(logptr, LLOGBTR, "tb_nm2struct(%s)",name);
#endif

    for (tableptr = tb_list; *tableptr != (Table *) 0; tableptr++)
	if (lexequ (name, (*tableptr) -> tb_name))
	    return (*tableptr);    /* return addr of chan struct entry   */

    return ((Table *) NOTOK);
}
