#include "util.h"
#include "mmdf.h"
#include "d_returns.h"
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
/*#define RUNALON   */

/*                  Handle telephone-based i/o                          */

extern struct ll_struct   *logptr;
extern int    d_errno;              /* dial package error                 */
extern char *blt ();

/*  NOTE:  load initialization routines from ph_iouser or ph_ioslave    */

/*   ************  (ph_)  PHONE MAIL I/O SUB-MODULE  ****************** */

ph_wrply (valstr, len)           /* pass a reply to local process      */
struct rp_bufstruct *valstr;      /* string describing reply            */
int       len;                      /* length of the string               */
{
    char    rp_string[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_wrply()");
    ll_log (logptr, LLOGFTR, "(%s)'%s'",
	    rp_valstr (valstr -> rp_val), valstr -> rp_line);
#endif
				  /* turn rp code into two hex bytes    */
				  /*   to avoid using high bit on phone */
    rp_string[0] = d_tohex ((valstr -> rp_val >> 4) & 017);
				  /* high four bits                     */
    rp_string[1] = d_tohex (valstr -> rp_val & 017);
				  /* low four bits                      */
    blt (valstr -> rp_line, &rp_string[2], len);
    return (ph_wrec (rp_string, len + 1));
}
/**/

ph_rrply (valstr, len)           /* get a reply from remote process    */
struct rp_bufstruct *valstr;      /* where to stuff copy of reply text  */
int      *len;                      /* where to indicate text's length    */
{
    short     retval;
    char   *rplystr;
    char    rp_string[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_rrply()");
#endif

    retval = ph_rrec (
#ifdef RUNALON
	    (char  *) valstr,
#else
	        rp_string,
#endif
	        len);
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
#else
    valstr -> rp_val = ((d_fromhex (rp_string[0]) & 017) << 4) |
	(d_fromhex (rp_string[1]) & 017);
    *len -= 1;			  /* reply is shorter                   */
    blt (&rp_string[2], valstr -> rp_line, *len);
				  /* copy string + its null ending      */
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
/*                READ FROM REMOTE MAIL PROCESS                    */

ph_rrec (linebuf, len)           /* read one "record"                    */
char   *linebuf;		  /* where to stuff the text              */
int      *len;                      /* where to stuff the length count      */
{
    short     retval;
    int       goteot;               /* completed packet                   */

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_rrec ()");
#endif

#ifdef RUNALON
    *len = read (0, linebuf, LINESIZE - 1);
    *len -= 1;
#else
    if ((*len = d_rcvdata (linebuf, LINESIZE - 1, &goteot)) < 0)
    {
	retval = ph_d2rpval (*len);
	ll_log (logptr, LLOGTMP,
		"ph_rrec:  d_rcvdata err (%s)", rp_valstr (retval));
	return (retval);
    }
#endif

    if (*len == 0)
    {
	ll_log (logptr, LLOGFTR, "zero len read");
	return (RP_DONE);
    }
    linebuf[*len] = '\0';	  /* keep things null-terminated        */

    if (!goteot)		  /* packet is too long                 */
	ll_log (logptr, LLOGFTR, "*** oversized packet");
				  /* let is slip, for now               */

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "(%d)'%s'", *len, linebuf);
#endif
    return (RP_OK);
}
/**/

ph_rstm (buffer, len)            /* read buffered block of text        */
char   *buffer;			  /* where to stuff the text            */
int      *len;                      /* where to stuff count               */
{
    static int    goteos;           /* save end-of-stream across calls    */

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_rstm ()");
#endif

    if (goteos)			  /* was set automatically by dial      */
    {
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "returning DONE");
#endif
	goteos = FALSE;
	*buffer = '\0';
	*len = 0;
	return (RP_DONE);
    }

#ifdef RUNALON
    printx ("dm_rstm: ");

    *len = read (0, buffer, *len - 1);
    if (*len == 0)
	goteos = TRUE;
#else
    *len = d_rcvdata (buffer, *len - 1, &goteos);
#endif

    if (*len < 0)
    {
	ll_log (logptr, LLOGGEN, "rdstm: error rcvdata");
	return (ph_d2rpval (*len));
    }
    buffer[*len] = '\0';

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "(%d)'%s'", *len, buffer);
#endif
    return (RP_OK);
}
/*                WRITE TO REMOTE MAIL PROCESS                        */

ph_wrec (linebuf, len)           /* write a record/packet              */
char	*linebuf;		  /* chars to write                     */
int	len;                      /* number of chars to write           */
{
    short     retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_wrec () (%d)'%s'", len, linebuf ? linebuf : "");
#endif

#ifdef RUNALON
    if (linebuf == 0 && len == 0)
    {
	printf ("p_wrec (eof)\n");
	fflush (stdout);
	return (RP_OK);
    }
    if (write (1, linebuf, len) != len)
	return (RP_DHST);
    write (1, "\n", 1);		  /* make it pretty */
    return (RP_OK);
#endif

    if ((retval = d_snstream (linebuf, len)) < D_OK ||
	    (retval = d_sneot ()) < D_OK)
	return (ph_d2rpval (retval));

    return (RP_OK);
}
/**/

ph_wstm (buffer, len)            /* write next part of char stream     */
char    buffer[];		  /* chars to write                     */
int       len;                      /* number of chars to write           */
{
    short     retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ph_wstm () (%d)'%s'", len, buffer ? buffer : "");
#endif

#ifdef RUNALON
    retval = write (1, buffer, len);
    if (retval == len)
	return (RP_OK);
    else
	return (RP_NO);
#endif

    if (buffer == 0 && len == 0)
	retval = d_sneot ();     /* flush the buffer                   */
    else
	retval = d_snstream (buffer, len);

    if (retval < D_OK)
	return (ph_d2rpval (retval));
    return (RP_OK);
}
/**/

ph_d2rpval (retval)               /* map d_errno into rp.h value        */
int       retval;
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "(%d) => dial err (%d)", retval, d_errno);
#endif

    switch (d_errno)
    {
	case D_LOGERR: 		  /* probably locked                    */
	    ll_err (logptr, LLOGTMP, "phone log");
	    return (RP_AGN);      /* maybe  transient problem           */

	case D_NOPORT:            /* no port or line available          */
	case D_BUSY: 
	case D_ABAN: 
	    return (RP_AGN);      /* highly transient problem           */

	case D_TIMEOUT:           /* alarm during port read/write call  */
	    return (RP_TIME);     /* same type of code                  */

	case D_RCVQUIT:           /* a QUIT packet has been received    */
	    return (RP_DONE);

	case D_PORTEOF:           /* eof on port                        */
	    return (RP_EOF);

	case D_NOPWR:             /* the dialer has no power            */
	case D_PORTOPEN:          /* error trying to open port          */
	    return (RP_NET);

	case D_PORTRD:            /* error on port read                 */
	case D_PORTWRT:           /* error on port write                */
	    return (RP_NIO);

	case D_NORESP:            /* no response to transmitted packet  */
	    return (RP_DHST);     /* assume the host is messed up       */

	case D_PATHERR:           /* error in path packet               */
	case D_PACKERR:           /* error in packet format             */
	    return (RP_BHST);

	case D_TSOPEN:            /* error opening transcript file      */
	case D_LOCKERR:           /* error opening/creating lock file   */
	    return (RP_LOCK);

	case D_CALLLOG:           /* error opening call log file        */
	    return (RP_FOPN);

	case D_TSWRITE:           /* error writing on transcript file   */
	    return (RP_FIO);

	case D_SYSERR:            /* undistinguished system error       */
	case D_FORKERR:           /* couldn't fork after several tries  */
	case D_PRTSGTTY:          /* error doing stty or gtty on port   */
	    return (RP_LIO);

	case D_SCRERR:            /* error in script file               */
	case D_ACCERR:            /* error in access file               */
	case D_NONUMBS:           /* no numbers given to dialer         */
	case D_NOESC:             /* no esacpe character could be found */
	case D_BADSPD:            /* bad speed designation              */
	    return (RP_PARM);

	case D_INTERR:            /* internal package error             */
	case D_INITERR:           /* trouble initializing               */
	case D_BADDIG:            /* bad digit passed to dialer         */
	    return (RP_NO);

	default: 		  /* punt                               */
	    ll_err (logptr, LLOGTMP,
		    "unclassified category (%d) phone error (%d)",
				retval, d_errno);
	    return (RP_NO);
    }
}
