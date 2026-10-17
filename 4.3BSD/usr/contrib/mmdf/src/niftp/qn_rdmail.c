/*                                                                      */
/*                  CH_QNIFTP: NIFTP MAIL RECEIVER CHANNEL              */
/*                                                                      */
/*      Developed originally by Chris Bennett - May 1981                */
/*                                                                      */
/*      Rewritten completely by Steve Kille - August 1982               */

#include "util.h"
#include "mmdf.h"
#include "ap.h"
#include "ch.h"
#include "dm.h"

extern struct ll_struct   *logptr;

extern Chan *curchan;

extern Table *tb_nm2struct();
extern Chan *ch_nm2struct();
extern Domain *dm_v2route();
extern char *ap_dmflip();
extern char *ap_p2s();

extern qn_rdchar ();

LOCVAR  FILE    *qn_fp;         /* File pointer for current file        */
LOCVAR  long    curpos;         /* Current position in file             */
LOCVAR  char    eoheader;       /* Have we reached end of JNT header    */
LOCVAR  char    adr_keyend[] = "\r\n,\377";
				/* Set of characters terminating JNT    */
				/* Mail address                         */
LOCVAR  char    *ack_adr;       /* acknowlege-to address                */

LOCVAR  char    qn_nichanlist [LINESIZE]; /* list of valid channels     */

/*
*/
qn_init (ni_queue)              /* Initialise channel                   */
				/* No-op                                */
char    *ni_queue;              /* NIFTP queue associated with channel  */
{
    Table *tb_niftp;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qn_init (%s)", ni_queue);
#endif

    if ((tb_niftp = tb_nm2struct ("niftp.nets")) == (Table *) NOTOK)
    {
	ll_err (logptr, LLOGTMP, "Failure to locate NIFTP net table");
	return (RP_NO);
    }
				/* Extract name of current channel      */
				/* from list of NIFTP channels          */
    if (tb_k2val (tb_niftp, TRUE, ni_queue, qn_nichanlist)!=OK)
    {
	ll_err (logptr, LLOGFAT, "Unknown NIFTP queue \"%s\"", ni_queue );
	return (RP_PARM);
				/* all NIFTP channels should be in the  */
				/* mapping table - if not then error    */
    }

    return (RP_OK);
}

qn_end (state)                  /* Terminate channel                    */
int state;                      /* No-op                                */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qn_end (%d)", state);
#endif
    return (RP_OK);
}

qn_pkinit ()                    /* Initialise for batch submission      */
				/* Not now, but perhaps later on again  */
				/* when NIFTP is working properly       */
				/* No-op                                */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qn_pkinit ()");
#endif
    return (RP_OK);
}

qn_pkend ()                     /* End batch submission                 */
				/* No-op                                */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qn_pkend ()");
#endif
    return (RP_OK);
}

/*
*/
				/* Initialise for processing message    */
qn_rinit (fname, TSname, info, sender)
char    *fname;                 /* Name of file containing JNT message  */
char    *TSname;                /* TS name of calling host              */
char    *info;                  /* place to stuff info                  */
char    *sender;                /* place to stuff name of message sender*/
{
     char       *reverse;       /* Pointer to reverse ordered domain    */
     char       tmpstr [LINESIZE];
     char       official [LINESIZE];
     Dmn_route  route;
     char       chan_buf [LINESIZE];
     int        argc, n;
     char       *argv[20];      /* lots of channels */

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qn_rinit (%s, %s)", fname, TSname );
#endif

    rtn_init ();
    ack_adr = (char *) 0;

    reverse = ap_dmflip (TSname);
    official [0] = 0;

    strcpy (chan_buf, qn_nichanlist);
    if ((argc = str2arg (chan_buf, 20, argv, (char *)0)) == NOTOK)
    {
	ll_err (logptr, LLOGFAT, "qn_rinit: str2arg failed '%s'", chan_buf);
	return (RP_LIO);
    }
    for ( n = 0 ; n < argc ; n++)
    {
	if ((curchan = ch_nm2struct (argv[n])) == (Chan *) NOTOK)
	{
	    ll_err (logptr, LLOGTMP, "qn_rinit: Bad channel '%s'", argv[n]);
	    continue;
	}

			/* See if we know it, NRS order first    */
	if (tb_k2val (curchan -> ch_table, TRUE, reverse, tmpstr)==OK)
	{
	    /* should check return value ?? */
	    dm_v2route (reverse, official, &route);
	    sprintf (info, "h%s*", official);
	    break;
	}
	else if (tb_k2val (curchan -> ch_table, TRUE, TSname, tmpstr)==OK)
	{
	    /* should check return value ?? */
	    dm_v2route (TSname, official, &route);
	    sprintf (info, "h%s*", official);
	    break;
	}
    }
				/* Strip "," and "*" from host name     */
				/* as it may be TS name with funnies    */
				/* we might as well munge this!!        */
    if (official [0] == '\0')
    {
	ll_err (logptr, LLOGTMP, "qnrinit: Unknown source address '%s'",TSname);
	return (RP_NDEL);
    }

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "We are channel '%s'", curchan -> ch_name);
#endif
    free (reverse);
    ch_llinit(curchan);

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "Info string is \"%s\"", info);
#endif

				/* Now open up the message file         */

    if ((qn_fp = fopen (fname, "r")) == NULL )
    {
	ll_err (logptr, LLOGTMP, "can't open file '%s'",
			    fname);
	return (RP_FIO);
    }

				/* Extract sender name from text        */
				/* This is pretty ugly, but the best    */
				/* That JNT mail can do                 */
				/* If there is no sender field valid    */
				/* dont worry about it                  */
    sender [0] = '\0';
    if (rp_isbad(hdr_sender (sender, &ack_adr, qn_fp, official)))
	return (RP_NDEL);
				/* now return to the start of the file  */

    ll_log (logptr, LLOGFST, "sender: '%s'", sender);

    fseek (qn_fp, 0L, 0);
    if (ferror (qn_fp))
    {
	ll_err (logptr, LLOGTMP, "Fail to seek on file '%s'", fname);
	return (RP_FIO);
    }
    curpos = 0;                 /* mark postition                       */

    eoheader = FALSE;           /* ready to read JNT header             */

    return (RP_OK);
}
/*
*/
extern char *rindex ();

qn_radr (host, adr)             /* extract next address from file       */
				/* might get confused by very badly     */
				/* damaged files                        */
char    *host;
char    *adr;
{
    char        *strptr;
    char        *cp;
    AP_ptr      local,          /* pointers for parse coponents         */
		domain,
		route;
    AP_ptr      ptr;
    AP_ptr      ap;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qn_radr ()");
#endif

    if ((ptr = ap_pinit (qn_rdchar)) == (AP_ptr) NOTOK)
    {
	ll_err (logptr, LLOGTMP, "Problem initialising for parse");
	return (RP_LIO);
    }

#ifdef DEBUG
     ll_log (logptr, LLOGFTR, "Parse corectly initialised");
#endif

    switch ( ap_1adr ())
    {
	case NOTOK:
	    ap_free (ptr);
	    ll_log (logptr, LLOGFAT, "Bad format address in JNT header");
	    return (RP_HUH);
	case DONE:
	    ap_free (ptr);
	    return (RP_DONE);   /* end of address list                  */
    }

				/* Now analyse address                  */

    ap_t2parts (ptr, (AP_ptr *) 0, (AP_ptr *) 0, &local, &domain, &route);
				/* remove any trailing DLIT          */
    if (route == (AP_ptr) 0)
    {
	if (domain ->  ap_obtype == APV_DLIT)
	{
	    ap_free (domain);
	    domain = (AP_ptr) 0;
	}
    }
    else
    {
	if (route -> ap_obtype == APV_DLIT)
	{
	    ap = route;
	    route = ap -> ap_chain;
	    if (route -> ap_obtype != APV_DLIT ||
		route ->  ap_obtype != APV_DOMN)
	    route = (AP_ptr) 0;
	    ap_free (ap);
	}
    }

    strptr = ap_p2s ((AP_ptr) 0, (AP_ptr) 0, local, domain, route);
    if(strptr == (char *)MAYBE)
	return(RP_NS);

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "Full address is '%s'", strptr);
#endif
    strcpy (adr, strptr);
    cp = rindex (adr, '@');
    if (cp == (char *) 0)
	host [0] = '\0';
    else
    {
	*cp  = '\0';
	strcpy (host, ++cp);
    }
    free (strptr);

    ll_log (logptr, LLOGFST, "JNT Address h='%s', a='%s'", host, adr);
    return (RP_AOK);
}


/*
*/
qn_raend ()                      /* Reached end of address list          */
				/* skip ofver any LWSP before 733 header*/
{
    int         c;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qn_raend ()");
#endif

    FOREVER
    {
	if ((c = getc (qn_fp)) == EOF)
	{
	    ll_err (logptr, LLOGTMP, "JNT mail header with no body" );
	    return (RP_NO);
	}
	curpos++;
	if (!isspace (c))       /* reached start of 733 header          */
	{
	    fseek (qn_fp, --curpos, 0);
				/* set back one character               */
	    if (ferror (qn_fp))
		return (RP_FIO);
	    return (RP_OK);
	}
    }
}
/*
*/
qn_wrply (reply, adr, host)     /* Used to mark status of address       */
				/* If reply is bad, passed downwards    */
				/* to build up list of erroneous        */
				/* addresses to be passed back          */
struct rp_bufstruct  *reply;
char    *adr,
	*host;
{
    char   line[LINESIZE];
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qn_wrply ()");
#endif

    if (rp_isgood (reply -> rp_val))
    {
	sprintf (line, "%s@%s", adr, host);
	return (rtn_adr (line, TRUE));
    }
    else
    {
	strcpy (line, reply -> rp_line);
	return (rtn_adr (line, FALSE));
    }

}
/*
*/
qn_rtxt (buffer, len)             /* read buffered block of text        */
char   *buffer;                   /* where to stuff next part of text   */
int    *len;                      /* where to stuff length of part      */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "qn_rtxt ()");
#endif


    switch (*len = gcread (qn_fp, buffer, *len - 1, "\n\000\377"))
    {                             /* raw read of the file               */
	case NOTOK:
	    ll_err (logptr, LLOGFAT, "File read error");
	    return (RP_FIO);

	case OK:                  /* end of message; not treated as EOF */
#ifdef DEBUG
	    ll_log (logptr, LLOGBTR, "qn_rtxt (DONE)");
#endif
	    return (RP_DONE);
    }
    switch (*len)
    {
	case 0:
	    return (RP_DONE);

	case 1:
	    if (isnull (buffer[0]))
		return (RP_DONE);
    }
    buffer[*len] = '\0';          /*   so it can be ll_loged as a string  */
#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "\"%s\"", buffer);
#endif
    return (RP_OK);
}
/**/
extern char *supportaddr;

qn_rtend (sender, fname, alright)
				/* finished reading text - close file      */
				/* send off error message                  */
char    *sender;        /* sender of message                               */
char    *fname;         /* full pathname of file                           */
int     alright;
{
    int retval;
#ifdef debug
    ll_log (logptr, LLOGBTR, "qn_rtend ()");
#endif

    retval = RP_OK;
    if (rp_isgood (alright))
    {
	if (ack_adr  != (char *) 0)
	{
	    rtn_ack (ack_adr, qn_fp);
				/* don't care if this fails             */
	    free (ack_adr);
	}

	if (rtn_errmsg (sender, qn_fp) == NOTOK)
	{
	    ll_log (logptr,  LLOGTMP, "Failed to send error mesage to '%s'",
				sender);
	    if (rtn_errmsg (supportaddr, qn_fp) == NOTOK)
	    {
		ll_log (logptr, LLOGTMP, "Failed to send error message to spport addr");
		retval = RP_NO;
	    }
	}
				/* fire off error message                  */
				/* pass error upwards if it fails          */
    }


    if ((qn_fp != NULL) && (fclose (qn_fp) == NOTOK)) /* close file */
    {
	ll_err (logptr, LLOGFAT, "Failure to close file '%s'", fname);
	return (RP_FIO);
    }

    if (unlink (fname) == NOTOK)
				/* delete file                            */
    {
	ll_err (logptr, LLOGFAT, "Failure to unlink file '%s'", fname);
	return (RP_FOPN);
    }

    return (retval);
}
/**/

LOCFUN
qn_rdchar ()                    /* read chars from jnt header           */
				/* stop when we reach blank line        */
{
    int         c;

#ifdef DEBUG
   ll_log (logptr, LLOGMAX, "qn_rdchar ()");
#endif

    if (eoheader)
	return (EOF);

    if ((c = getc (qn_fp)) == EOF)
    {
	ll_err (logptr, LLOGTMP, "Premature end of JNT Mail Header");
	return (EOF);
    }
    curpos++;

    if (c != '\n')
    {
#ifdef DEBUG
    ll_log (logptr, LLOGMAX, "Returning (%c)", c);
#endif
	return (c);
    }

    while (isspace (c = getc (qn_fp)) && c != '\n')
	curpos++;

    curpos++;
    if (c == '\n')              /* note NIFTP changes <CRLF> sequence */
    {
#ifdef DEBUG
    ll_log (logptr, LLOGMAX, "Returning EOF");
#endif
	eoheader = TRUE;
	return (EOF);
    }
				/* end of JNT header                   */

#ifdef DEBUG
    ll_log (logptr, LLOGMAX, "Returning (%c)", c);
#endif

    return (c);                 /* Just a newline                       */
}
