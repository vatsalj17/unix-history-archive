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
/*                  MAIL-COMMANDS FOR SMTP MAIL                      */

/*  Oct 82 Dave Crocker   derived from arpanet ftp/ncp channel
 *
 *      -------------------------------------------------
 *
 *  Feb 83 Doug Kingston  major rewrite, some fragments kept
 */

#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include <signal.h>
#include "phs.h"
#include "ap.h"
#include "dm.h"
#include "smtp.h"

extern LLog     *logptr;
extern Chan     *chanptr;
extern char     *blt();
extern char     *strdup();
extern char     *strncpy ();

char    *sm_curname;

struct sm_rstruct sm_rp;            /* save last reply obtained           */
LOCVAR Chan     *sm_chptr;      /* structure for channel that we are  */
FILE    *sm_rfp, *sm_wfp;
LOCVAR char     sm_rnotext[] = "No reply text given";
LOCVAR  char    netobuf[BUFSIZ];
LOCVAR  char    netibuf[BUFSIZ];

/**/

sm_wfrom (sender)
char    *sender;
{
	char    linebuf[LINESIZE];

	sprintf (linebuf, "MAIL FROM:<%s>", sender);
	if (rp_isbad (sm_cmd (linebuf, SM_STIME)))
	    return (RP_DHST);

	switch( sm_rp.sm_rval ) {
	    case 250:
		break;          /* We're off and running! */

	    case 500:
	    case 501:
	    case 552:
		return( sm_rp.sm_rval = RP_PARM );

	    case 421:
	    case 450:
	    case 451:
	    case 452:
		return( sm_rp.sm_rval = RP_AGN);

	    default:
		return( sm_rp.sm_rval = RP_BHST);
	}
	return( RP_OK );
}

sm_wto (host, adr)         /* send one address spec to local     */
char    host[];                   /* "next" location part of address    */
char    adr[];                    /* rest of address                    */
{
    char linebuf[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "sm_wto(%s, %s)", host, adr);
#endif

    sprintf (linebuf, "RCPT TO:<%s>", adr);
    if (rp_isbad (sm_cmd (linebuf, SM_TTIME)))
	return (RP_DHST);

    switch (sm_rp.sm_rval)
    {
	case 250:
	case 251:
	    sm_rp.sm_rval = RP_AOK;
	    break;

	case 421:
	case 450:
	case 451:
	case 452:
	    sm_rp.sm_rval = RP_AGN;
	    break;

	case 550:
	case 551:
	case 552:
	case 553:
	case 554:               /* BOGUS: sendmail is out of spec! */
	    sm_rp.sm_rval = RP_USER;
	    break;

	case 500:
	case 501:
	    sm_rp.sm_rval = RP_PARM;
	    break;

	default:
	    sm_rp.sm_rval = RP_RPLY;
    }
    return (sm_rp.sm_rval);
}

sm_init (curchan)                 /* session initialization             */
Chan *curchan;                    /* name of channel                    */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "sm_init ()");
#endif
    sm_chptr = curchan;
    phs_note (sm_chptr, PHS_CNSTRT);
    return (RP_OK);               /* generally, a no-op                 */
}

/**/

LOCFUN
	sm_irdrply ()             /* get net reply & stuff into sm_rp   */
{
    static char sep[] = "; ";     /* for sticking multi-lines together  */
    short     len,
	    tmpreply,
	    retval;
    char    linebuf[LINESIZE];
    char    tmpmore;
    register char  *linestrt;     /* to bypass bad initial chars in buf */
    register short    i;
    register char   more;         /* are there continuation lines?      */

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "sm_irdrply ()");
#endif

newrply: 
    for (more = FALSE, sm_rp.sm_rgot = FALSE, sm_rp.sm_rlen = 0;
	    rp_isgood (retval = sm_rrec (linebuf, &len));)
    {                             /* 1st col in linebuf gets reply code */
	printx("<-(%s)\r\n", linebuf);
	fflush( stdout );

	for (linestrt = linebuf;  /* skip leading baddies, probably     */
		len > 0 &&        /*  from a lousy Multics              */
		    (!isascii ((char) *linestrt) ||
			!isdigit ((char) *linestrt));
		linestrt++, len--);

	tmpmore = FALSE;          /* start fresh                        */
	tmpreply = atoi (linestrt);
	blt (linestrt, sm_rp.sm_rstr, 3);       /* Grab reply code      */
	if ((len -= 3) > 0)
	{
	    linestrt += 3;
	    if (len > 0 && *linestrt == '-')
	    {
		tmpmore = TRUE;
		linestrt++;
		if (--len > 0)
		    for (; len > 0 && isspace (*linestrt); linestrt++, len--);
	    }
	}

	if (more)                 /* save reply value from 1st line     */
	{                         /* we at end of continued reply?      */
	    if (tmpreply != sm_rp.sm_rval || tmpmore)
		continue;
	    more = FALSE;         /* end of continuation                */
	}
	else                      /* not in continuation state          */
	{
	    sm_rp.sm_rval = tmpreply;
	    more = tmpmore;   /* more lines to follow?              */

	    if (len <= 0)
	    {                     /* fake it, if no text given          */
		blt (sm_rnotext, linestrt = linebuf,
		       (sizeof sm_rnotext) - 1);
		len = (sizeof sm_rnotext) - 1;
	    }
	}

	if ((i = min (len, (LINESIZE - 1) - sm_rp.sm_rlen)) > 0)
	{                         /* if room left, save the human text  */
	    blt (linestrt, &sm_rp.sm_rstr[sm_rp.sm_rlen], i);
	    sm_rp.sm_rlen += i;
	    if (more && sm_rp.sm_rlen < (LINESIZE - 4))
	    {                     /* put a separator between lines      */
		blt (sep, &(sm_rp.sm_rstr[sm_rp.sm_rlen]), (sizeof sep) - 1);
		sm_rp.sm_rlen += (sizeof sep) - 1;
	    }
	}
#ifdef DEBUG
	else
	    ll_log (logptr, LLOGFTR, "skipping");
#endif

	if (!more)
	{
#ifdef DEBUG
	    ll_log (logptr, LLOGBTR, "(%u)%s", sm_rp.sm_rval, sm_rp.sm_rstr);
#endif
	    if (sm_rp.sm_rval < 100)
		goto newrply;     /* skip info messages                 */

	    sm_rp.sm_rgot = TRUE;
	    return (RP_OK);
	}
    }
    return (retval);              /* error return                       */
}

sm_rpcpy (rp, len)           /* return arpanet command reply       */
RP_Buf *rp;      /* where to put it                    */
short    *len;                      /* its length                         */
{
    if( sm_rp.sm_rgot == FALSE )
	return( RP_RPLY );

    rp -> rp_val = sm_rp.sm_rval;
    *len = sm_rp.sm_rlen;
    blt (sm_rp.sm_rstr, rp -> rp_line, sm_rp.sm_rlen + 1);
    sm_rp.sm_rgot = FALSE;        /* flag as empty                      */

    return (RP_OK);
}
/**/

LOCFUN
	sm_rrec (linebuf, len)   /* read a reply record from net       */
char   *linebuf;                  /* where to stuff text                */
short    *len;                      /* where to stuff length              */
{
    extern int errno;
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "sm_rrec ()");
#endif

    *len = 0;                     /* for clean logging if nothing read  */
    linebuf[0] = '\0';

    fgets (linebuf, LINESIZE, sm_rfp);
    *len = strlen (linebuf);

    if (ferror (sm_rfp) || feof (sm_rfp))
    {                             /* error or unexpected eof            */
	printx ("problem reading from net, ");
	fflush (stdout);
	ll_err(logptr, LLOGTMP, "netread:  ret=%d, fd=%d",
		*len, fileno (sm_rfp));
	sm_nclose (NOTOK);         /* since it won't work anymore        */
	return (RP_BHST);
    }
    if (linebuf[*len - 1] != '\n')
    {
	ll_log (logptr, LLOGTMP, "net input overflow");
	while (getc (sm_rfp) != '\n'
		&& !ferror (sm_rfp) && !feof (sm_rfp));
    }
    else
	if (linebuf[*len - 2] == '\r')
	    *len -= 1;            /* get rid of crlf or just lf         */

    linebuf[*len - 1] = '\0';
#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "(%u)'%s'", *len, linebuf);
#endif
    return (RP_OK);
}
/**/

sm_cmd (cmd, time)              /* Send a command */
char    *cmd;
int     time;                   /* Max time for sending and getting reply */
{
    short     retval;

    ll_log (logptr, LLOGPTR, "sm_cmd (%s)", cmd);

    printx("->(%s)\r\n", cmd);
    fflush( stdout );

    if (setjmp(timerest)) {
	ll_log (logptr, LLOGGEN, "sm_cmd(): host died?");
	sm_nclose (NOTOK);
	return (sm_rp.sm_rval = RP_DHST);
    }
    s_alarm( (unsigned) time );
    fwrite (cmd, sizeof (char), strlen(cmd), sm_wfp);
    fputs ("\r\n", sm_wfp);
    fflush (sm_wfp);

    if (ferror (sm_wfp))
    {
	s_alarm ( 0 );
	ll_log (logptr, LLOGGEN, "sm_cmd(): host died?");
	sm_nclose (NOTOK);
	return (sm_rp.sm_rval = RP_DHST);
    }

    if (rp_isbad (retval = sm_irdrply ())) {
	s_alarm( 0 );
	return( sm_rp.sm_rval = retval );
    }
    s_alarm( 0 );
    return (RP_OK);
}
/**/

sm_wstm (buf, len)            /* write some message text out        */
char    *buf;                 /* what to write                      */
register int    len;              /* how long it is                     */
{
    static char lastchar = 0;
    short     retval;
    register char  *bufptr;
    register char   newline;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "sm_wstm () (%u)'%s'", len, buf ? buf : "");
#endif

    if (buf == 0 && len == 0)
    {                             /* end of text                        */
	if (lastchar != '\n')     /* make sure it ends cleanly          */
	    fputs ("\r\n", sm_wfp);
	if (ferror (sm_wfp))
	    return (RP_DHST);
	lastchar = 0;             /* reset for next message             */
	retval = RP_OK;
    }
    else
    {
	newline = (lastchar == '\n') ? TRUE : FALSE;
	for (bufptr = buf; len--; bufptr++)
	{
	    switch (*bufptr)      /* cycle through the buffer           */
	    {
		case '\n':        /* Telnet requires crlf               */
		    newline = TRUE;
		    putc ('\r', sm_wfp);
		    break;

		case '.':         /* Insert extra period at beginning   */
		    if (newline)
			putc ('.', sm_wfp);
				  /* DROP ON THROUGH                    */
		default: 
		    newline = FALSE;
	    }
	    putc ((lastchar = *bufptr), sm_wfp);
	    if (ferror (sm_wfp))
		return (RP_DHST);
				  /* finally send the data character    */
	}
	retval = ferror(sm_wfp) ? RP_DHST : RP_OK;
    }

    return (retval);
}

/**/

union Haddru {
	long hnum;
	char hbyte[4];
};

sm_hostid (hostnam, addr, first)  /* addresses to try if sending to hostnam */
char    *hostnam;                 /* name of host */
union Haddru *addr;
int	first;			  /* first try on this name ?*/
{
    int     argc;
    register long    n;
    char   *argv[20];
    char    numstr[50];
    int     rval;
    int     tlookup=1;		/* used table lookup? */

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "sm_gethostid (%s)", hostnam);
#endif

    for (;;) 
    {
	/* look for [x.x.x.x] format -- don't need table lookup */

	if (hostnam[0] == '[')
	{
	    tlookup = 0;
	    (void) strncpy(numstr,hostnam,sizeof(numstr)-1);
	}
	else if ((rval=tb_k2val(sm_chptr->ch_table,first,hostnam,numstr))!=OK)
	{
	    /* No such host */
	    if (first)
		ll_log (logptr, LLOGTMP, "channel '%s' unknown host '%s'",
			sm_chptr -> ch_name, hostnam);

	    if (rval == MAYBE)
		return(RP_NS);

	    return(RP_BHST);
	}

	ll_log ( logptr, LLOGFTR, "Trying to break down %s", numstr) ;

	/* watch out for quoted and bracketed strings */
	argc = cstr2arg ((numstr[0]=='"' || numstr[0]=='[') ? &numstr[1]:numstr,
		20, argv, '.');

	ll_log ( logptr, LLOGFTR, "%d fields, '%s'", argc, argv[0] );


	switch (argc)               /* what form is hostnum in?             */
	{
	    case 4:                 /* dot-separated                        */
		n = atoi (argv[0]);
		n = (n<<8) | atoi (argv[1]);
		n = (n<<8) | atoi (argv[2]);
		addr->hnum = (n<<8) | atoi (argv[3]);
		return(RP_OK);
		break;

	    default:
		ll_log (logptr, LLOGTMP,
				"channel '%s' host %s' has bad address format",
				sm_chptr -> ch_name, hostnam);
		if (!tlookup)
		{
		    /* bad hostname in brackets */
		    return(RP_NO);
		}
		break;
	}
    } /* end for */
    
    /* NOTREACHED */
}

sm_nopen( hostnam )
char    *hostnam;
{
    Pip     fds;
    short   retval;
    char    linebuf[LINESIZE];
    union   Haddru haddr;
    unsigned atime;
    int     first;
    int	    rval;

    ll_log (logptr, LLOGPTR, "[ %s ]", hostnam);

    printx ("trying...\n");
    fflush (stdout);
    first = 1;
    atime = SM_ATIME;

    /* keep coming here until connected or no more addresses */
retry:

    if ((rval = sm_hostid(hostnam, &haddr, first)) != RP_OK)
    {
	switch (rval)
	{
	    case RP_NS:
		printx("\tno answer from nameserver\n");
		break;

	    case RP_NO:
		printx("\tbad hostname format\n");
		break;

	    default:
		/* some non-fatal error */
		rval = RP_BHST;
		break;
	}
	fflush(stdout);
	return(rval);
    }

    if (first)
	first = 0;

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "trying %u.%u.%u.%u",
		      haddr.hbyte[3]&0xff, haddr.hbyte[2]&0xff,
		      haddr.hbyte[1]&0xff, haddr.hbyte[0]&0xff);
#endif

    /* tell them who we are trying */
    printx("\tconnecting to [%u.%u.%u.%u]...",
			haddr.hbyte[3]&0xff, haddr.hbyte[2]&0xff,
			haddr.hbyte[1]&0xff, haddr.hbyte[0]&0xff);
    fflush(stdout);

    /* SMTP is on socket 25 */
    retval = tc_uicp (haddr.hnum, 25L, SM_OTIME, &fds);

    if (retval != RP_OK)
    {
	/* common event, so LLOGGEN (not TMP) */
        if (retval == RP_TIME) {
            ll_err (logptr, LLOGGEN, "%s (%8lx) open timeout", hostnam, haddr.hnum);
            printx (" timeout...\n");
        } else {
            ll_err (logptr, LLOGGEN, "%s (%8lx) no open", hostnam, haddr.hnum);
            printx (" can't...\n");
        }
	fflush (stdout);
	goto retry;	/* can't reach -- try someone else */
    }
#ifdef DEBUG
    else
	ll_log (logptr, LLOGFTR,
		    "fdr = %d,fdw = %d", fds.pip.prd, fds.pip.pwrt);
#endif

    sm_curname = strdup(hostnam);
    phs_note (sm_chptr, PHS_CNGOT);
    if ((sm_rfp = fdopen (fds.pip.prd, "r")) == NULL ||
	(sm_wfp = fdopen (fds.pip.pwrt, "w")) == NULL) {
	printx (" can't fdopen!\n");
	fflush(stdout);
	return (RP_LIO);	/* new address won't fix this problem */
    }
    printx (" open.\n");
    fflush (stdout);

    setbuf (sm_wfp, netobuf);
    setbuf (sm_rfp, netibuf);

    if (setjmp(timerest)) {
	sm_nclose (NOTOK);
	goto retry;		/* too slow, try someone else */
    }
    s_alarm (atime);

    atime -= SM_ATINC;
    if (atime < SM_ATMIN)
	atime = SM_ATMIN;

    if (rp_isbad (retval = sm_irdrply ())) {
	s_alarm (0);
	sm_nclose (NOTOK);
	goto retry;		/* problem reading -- try someone else */
    }
    s_alarm (0);

    if( sm_rp.sm_rval != 220 )
    {
	sm_nclose (NOTOK);
	goto retry;
    }

    if (sm_chptr -> ch_confstr)
	sprintf (linebuf, "HELO %s", sm_chptr -> ch_confstr);
    else
	sprintf (linebuf, "HELO %s.%s", sm_chptr -> ch_lname,
				    sm_chptr -> ch_ldomain);
    if (rp_isbad (sm_cmd( linebuf, SM_HTIME )) || sm_rp.sm_rval != 250 ) {
	sm_nclose (NOTOK);
	goto retry;		/* try more intelligent host? */
    }
    return (RP_OK);
}
/**/

sm_nclose (type)                /* end current connection             */
short     type;                 /* clean or dirty ending              */
{
	if (type == OK) {
		sm_cmd ("QUIT", SM_QTIME);
	} else {
		printx ("\r\nDropping connection\r\n");
		fflush (stdout);
	}
	if (sm_curname)
		free(sm_curname);
	sm_curname = 0;
	if (setjmp(timerest)) {
		return;
	}
	s_alarm (15);
	if (sm_rfp != NULL)
		fclose (sm_rfp);
	if (sm_wfp != NULL)
		fclose (sm_wfp);
	s_alarm (0);
	sm_rfp = sm_wfp = NULL;
	phs_note (sm_chptr, PHS_CNEND);
}
