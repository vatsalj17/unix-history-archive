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
/*
	this is a surrogate err_abrt, which is part of the
	mmdf library and is included in case the user's program
	doesn't specify one.  

	it is called by the lm_ routines, for example.

	the action, here, is to try to  print the error message
	and log the message, and then terminate unceremoniously.

*/

extern struct ll_struct *logptr;

/* VARARGS2 */
err_abrt (code, fmt, b, c, d)     /* terminate ourself                  */
int     code;
char    fmt[],
        b[],
        c[],
        d[];
{
    char    newfmt[LINESIZE];

    if (rp_isbad (code))
    {
	printx ("mmdf: ");
	printx (fmt, b, c, d);
	printx ("\n");
	fflush (stdout);

	sprintf (newfmt, "%s%s\n", "err [ ABEND (%s) ] ", fmt);
	ll_err (logptr, LLOGFAT, newfmt, rp_valstr (code), b, c, d);
	fprintf (stderr, newfmt, rp_valstr (code), b, c, d);
#ifdef DEBUG
	fflush (stdout);
	fflush (stderr);
	abort ();
#endif
    }
    ll_close (logptr);           /* in case of cycling, close neatly   */
    exit (RP_NO);
}
