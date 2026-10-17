#include "util.h"
#include "mmdf.h"
/*                                                                      */
/*                  CH_NIFTP: NIFTP MAIL DONOR CHANNEL                  */
/*                                                                      */
/*         This module does all the hard work for the donor channel     */
/*              Chris Bennett University College London 24/2/81         */
/*                                                                      */
/*      Rewritten to incorporate new release MMDF stuff and             */
/*      to generally streamline the code                                */
/*              Steve Kille             July 1982                       */
/*      SEK  Feb 84     Rewrite to reformat sender                      */
/*
/*                  CH_NIFTP: NIFTP MAIL DONOR CHANNEL                  */

#include <signal.h>
#include "ch.h"
#include "ap.h"
#include "jnt.h"                /* JNT Mail return value definition     */

extern struct ll_struct   *logptr;

extern char     *pn_quedir;
				/* Location of JNTmail queues for NIFTP */

extern int      ap_outtype;     /* Default output format                */

extern Chan  *chanptr;          /* Current channel                      */


char    *pn_chname;             /* Name of channel we are called as     */
char    *pn_sender;             /* Sender of the message being processed*/
char    *pn_msgfile;            /* name of file with message in         */
char    pn_fname[ LINESIZE ];   /* Name of file for NIFTP output        */
int     pn_fd;                  /* fd of NIFTP ouput file               */
char    pn_curhost[ LINESIZE ]; /* Host being processed                 */
short   firstadr;               /* Boolean - first address for host     */

char    jnt_hedsep[] = "\n\n";
				/* Separatpr between header and text    */
				/* for JNT mail                         */
char    jnt_adrsep[] = ",\n"; /* Separator between addresses          */
				/* for JNT mail                         */

LOCVAR  int inheader;           /* flags for text copy                  */
LOCVAR  int donesender;

				/* This lot need to be glbal to allow   */
				/* the structure to be initialised      */
LOCVAR  char TSname [LINESIZE];
				/* The TS address to be handed down     */
LOCVAR char    senderbuf [LINESIZE];
LOCVAR char    *vararray[] = {
	"file", pn_fname,
	"host", pn_curhost,
	"adr",  TSname,
	"sender", senderbuf,
	0, 0
};


extern char *index ();
extern char *blt ();
extern char *ap_p2s();

/*
*/


pn_init ()                        /* session initialization             */
{
#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "pn_init ()");
#endif
	return (RP_OK);
}

pn_end (state)                   /* end of session                     */
int     state;
{
#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "pn_end (%d)", state);
#endif
	return (RP_OK);               /* no-op                          */
}

pn_sbinit ()                      /* ready for submission               */
{
#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "pn_sbinit()");
#endif
	return (RP_OK);
}

pn_sbend ()                       /* end of submission                  */
{
#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "pn_sbend ()");
#endif

	return (RP_OK);               /* no-op                          */
}

/*            DELIVER MESSAGE TO NIFTP DONOR                          */




pn_winit (chname, sender, msgfile)/* initialize for one message         */
char    chname[],                 /* what channel coming in from        */
	sender[],                 /* return address for error msgs      */
	msgfile[];                /* name of file with msg text         */
{
    extern char *strdup ();
    AP_ptr ap;
    AP_ptr local,
	domain,
	route;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "pn_winit");
#endif

    if (pn_chname != (char *) 0)
	free (pn_chname);
    pn_chname = strdup (chname);
    if (pn_sender != (char *) 0)
	free (pn_sender);
    ap = ap_s2tree (sender);
    if (ap != (AP_ptr) NOTOK)
    {
	ap_t2parts (ap, (AP_ptr *) 0, (AP_ptr *) 0, &local, &domain, &route);
	pn_sender = ap_p2s ((AP_ptr) 0, (AP_ptr) 0, local, domain, route);
    }
			/* ap_outtype will have corect value*/

    else
	ll_log (logptr, LLOGTMP,  "Illegal sender '%s'", sender);

    if (pn_msgfile != (char *) 0)
	free (pn_msgfile);
    pn_msgfile = strdup (msgfile);

    ll_log (logptr, LLOGFTR, "c='%s', s='%s', f='%s'",
	    pn_chname, (pn_sender==(char *)MAYBE)?"":pn_sender, pn_msgfile);

    if(pn_sender == (char *)MAYBE)
	return (RP_NS);
    return (RP_OK);
}


/*
*/

pn_wainit ()                           /* generate and open file        */
				       /* needed befor we can start     */
				       /* and get ready for next set of */
				       /* addresses                     */
{
    extern char *mktemp ();     /* Stuff for filename genration         */
    static char template[] = "ni.aXXXXXX";
    short tries;
    extern char *multcpy ();

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "pn_wainit ()" );
#endif

    firstadr = TRUE;            /* Ready for first host         */


/* File name generation extracted from submit.
 * make the unique part of the queue entry file name.  if mktemp
 * can't do it, goose the value of the first char in the filename
 * part.  mktemp uses qf_mfname, rather than qf_filnam, so that its
 * stat() can accurately tell if the name is unique.  the goosing
 * strategy permits up to 500 messages under the same process id.
 * mmdf will probably fall apart before that limit is hit.
 */
    for (tries = 25; tries-- > 0; template[strlen (pn_quedir) + 4]++)
    {                             /* template[4] limited to lower case  */
	multcpy (pn_fname, pn_quedir, "/", template, (char *)0);
				  /* add standard part of unique name   */
	if (strcmp (mktemp(pn_fname), "/") != 0)
	    break;                /* add unique part                    */
    }
#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "JNT mail file is = \"%s\"", pn_fname);
#endif


    if (rp_isbad (pn_fd = creat (pn_fname, 0444)))
    {
	ll_err (logptr, LLOGBTR, "Failure to open file" );
	return (RP_FCRT);
    }

    return (RP_OK);
}
/*
/*      Process individual address                                      */
/*      Produce JNT style address from host and address and place       */
/*      into file.  "," separator if not first one                      */

extern      AP_ptr ap_s2tree ();

pn_wadr (host, adr)
char    *host;          /* host official name                           */
char    *adr;           /* rest of address / route                      */
{
    char        *jntadr;	/* 733 /JNT format address              */
    int         len;

#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "pn_wadr (h= '%s' a= '%s')", host, adr);
#endif

    jntadr = adr;
    ll_log (logptr, LLOGFST, "JNT address = '%s'", jntadr);

    if (firstadr)
    {
	firstadr = FALSE;
	strcpy (pn_curhost, host);
    }
    else
    {
	len = strlen (jnt_adrsep);
	if (write (pn_fd, jnt_adrsep, len) != len)
	{
	    ll_err (logptr, LLOGBTR, "File write error (separator)" );
	    return (RP_FIO);
	}
    }
    len = strlen(jntadr);
    if (write (pn_fd, jntadr, len) != len)
    {
	ll_err (logptr, LLOGBTR, "File write error" );
	return (RP_FIO);
    }

    return (RP_AOK );
}

/*
*/
LOCVAR  char partialbuf [LINESIZE];

pn_wtinit()
				/* End of addres list for this host     */
				/* Write separtator between address     */
				/* and start of body                    */
{
	int     len;
#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "pn_wtinit ()");
#endif

	inheader  = TRUE;
	donesender = FALSE;
	partialbuf [0] = '\0';
	ap_outtype = chanptr -> ch_apout;
	len = strlen (jnt_hedsep);
	if (write (pn_fd, jnt_hedsep, len) != len)
	{
	    ll_err (logptr, LLOGBTR, "Failure writing separator" );
	    return (RP_FIO);
	}

	return(RP_OK);
}
/*
*/

#ifndef VIATRACE

LOCVAR char *txt_sender = "Sender: ";
LOCVAR char *txt_original = "Original-";
				/* Write block of text to file          */
pn_wtxt (buffer, len)
char *buffer;
int len;
{
    int retval;
    char  *front;
    char  *back;
    char  csav;
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "pn_wttxt()");
#endif
				/* whilst in header split into lines */
				/*for reformatting                   */

    if (inheader)
    {
	back = index (buffer, '\n');
	if (back == (char *) 0)
	{
	    strcat (partialbuf, buffer);
	    return (RP_OK);
	}
	back++;
	csav = *back;
	*back = '\0';
	strcat (partialbuf, buffer);
	if (rp_isbad (retval = pn_whdr (partialbuf, strlen (partialbuf))))
	   return (retval);
	*back = csav;


	while ((front = index (back, '\n')) != (char *) 0)
	{
	    front++;
	    csav = *front;
	    *front = '\0';
	    if (rp_isbad (retval = pn_whdr (back, strlen (back))))
		return (retval);
	    *front = csav;
	    back = front;
	}
	if (!inheader)  /* reached end of header                */
	   return (pn_wtxt (back, strlen (back)));
	strcpy (partialbuf, back);
	return (RP_OK);
    }
    else
    {
	if (write (pn_fd, buffer, len) != len)
	{
		ll_err (logptr, LLOGBTR, "Failure writing text to file" );
		return (RP_FIO);
	}
    }
    return(RP_OK);
}


/* */

			/* take  single line and mungle it      */

LOCFUN pn_whdr (buffer, len)
char *buffer;
int  len;
{
	int namlen;
	char *cp;
	char name [LINESIZE];
	AP_ptr  ap,
		local,
		domain,
		route;

#ifdef DEBUG
	ll_log (logptr , LLOGFTR, "pn_whdr ('%s',%d)", buffer,  len);
#endif

    if (inheader)
    {
	if (buffer [0] == '\n')
	{
	     inheader = FALSE;
	     if (!donesender)
	     {
#ifdef DEBUG
		ll_log (logptr, LLOGBTR, "Outputting new sender '%s'",
				pn_sender);
#endif
		namlen = strlen (txt_sender);
		if (write (pn_fd, txt_sender, namlen) != namlen)
		{
		    ll_err (logptr, LLOGBTR, "Failure writing text to file" );
		    return (RP_FIO);
		}
		namlen = strlen (pn_sender);
		if (write (pn_fd, pn_sender, namlen) != namlen)
		{
		    ll_err (logptr, LLOGBTR, "Failure writing text to file" );
		    return (RP_FIO);
		}
		if (write (pn_fd, "\n\n", 2) != 2)
		{
		    ll_err (logptr, LLOGBTR, "Failure writing text to file" );
		    return (RP_FIO);
		}
	    }
	    else
		if (write (pn_fd, "\n", 1) != 1)
		{
		    ll_err (logptr, LLOGBTR, "Failure writing text to file" );
		    return (RP_FIO);
		}
	    return (RP_OK);
	}

				/* Now get name of line                 */
				/* just output continuations at once    */
	if (!isspace (buffer [0]) &&
		(cp = index (buffer, ':')) != (char *) 0)
	{
	    namlen = cp - buffer;
	    blt (buffer, name, namlen);
	    name [namlen]= '\0';
	    if (lexequ (name, "via"))
	    {
		namlen = strlen (txt_original);
		if (write (pn_fd, txt_original, namlen) != namlen)
		{
		    ll_err (logptr, LLOGBTR, "Failure writing text to file" );
		    return (RP_FIO);
		}
	    }

	    if (lexequ (name, "from"))
	    {
		ap = ap_s2tree (cp + 1);
		if (ap  != (AP_ptr) NOTOK)
		{
		    ap_t2parts (ap, (AP_ptr *) 0, (AP_ptr *) 0,
				&local, &domain, &route);
		    cp = ap_p2s ((AP_ptr) 0, (AP_ptr) 0,
				local, domain, route);
		    if(cp == (char *)MAYBE)
			return (RP_NS);
		    if (lexequ (pn_sender, cp))
		    {
			donesender = TRUE;
#ifdef DEBUG
			ll_log (logptr, LLOGFTR, "Found matching From '%s'",
				cp);
#endif
		    }
		    free (cp);
		}
	    }

	    if (lexequ (name, "sender"))
	    {
		ap = ap_s2tree (cp + 1);
		if (ap  != (AP_ptr) NOTOK)
		{
		    ap_t2parts (ap, (AP_ptr *) 0, (AP_ptr *) 0,
				&local, &domain, &route);
		    cp = ap_p2s ((AP_ptr) 0, (AP_ptr) 0,
				local, domain, route);
		    if(cp == (char *)MAYBE)
			return (RP_NS);
		    if (lexequ (pn_sender, cp))
		    {
			donesender = TRUE;

#ifdef DEBUG
		       ll_log (logptr, LLOGFTR, "Found matching Sender '%s'",
				cp);
#endif
		    }
		    else
		    {
			donesender = FALSE;
					/* in case a matchin From: field */
			namlen = strlen (txt_original);
			if (write (pn_fd, txt_original, namlen) != namlen)
			{
			    ll_err (logptr, LLOGBTR,
				"Failure writing text to file" );
			    return (RP_FIO);
			}
		    }
		    free (cp);
		}
	    }
	}
    }
    if (write (pn_fd, buffer, len) != len)
    {
	ll_err (logptr, LLOGBTR, "Failure writing text to file" );
	return (RP_FIO);
    }
    return(RP_OK);
}

#else VIATRACE

/*
*/
				/* Write block of text to file          */
				/* simple non-munging version           */
pn_wtxt (buffer, len)
char *buffer;
int len;
{
#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "pn_wttxt()");
#endif

	if (write (pn_fd, buffer, len) != len)
	{
	    ll_err (logptr, LLOGBTR, "Failure writing text to file" );
	    return (RP_FIO);
	}

	return(RP_OK);
}
#endif VIATRACE
/*
/*      End of text                                                     */
/*      Close file and pass to NIFTP                                    */
/*      The host name is translated into the standard UCL TS address    */
/*      of the form TSservice/hostaddres/service (e.g. tcp/isid/mail    */


pn_wtend (tmp_result)
int     tmp_result;
{
int     i;
char    *p,*q;
int     pid;                    /* Process id for fork                  */
int     status;                 /* Status for wait                      */
char    buf [LINESIZE];
int     argc;
char    *argv[50];
extern  char *expand();

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "pn_wtend ()");
#endif

    if (close(pn_fd) == NOTOK)         /* close file                   */
    {
	ll_err (logptr, LLOGBTR, "Failure to close file" );
	return (RP_LIO);
    }

				/* if status so far is bad unlink file  */
				/* and pass on                          */
    if (rp_isbad (tmp_result))
    {
	if (unlink (pn_fname) == NOTOK) /* delete file */
	{
	    ll_err (logptr, LLOGFAT, "Failure to unlink file '%s'", pn_fname);
	    return (RP_FOPN);
	}
	ll_log (logptr, LLOGBTR, "Failure (%s) for file '%s'",
		rp_valstr (tmp_result), pn_fname);
	return (tmp_result);
    }

				/* Now extract the TS address           */
    if (tb_k2val (chanptr->ch_table, TRUE,
	(chanptr->ch_host == NORELAY ? pn_curhost :  chanptr->ch_host),
	TSname)!=OK)
    {
	ll_err (logptr,  LLOGBTR, "TS address not found for '%s'",
							     pn_curhost );
	return (RP_PARM);
    }

				/* 'quote' chars in sender              */
    for (p=pn_sender, q=senderbuf; *p != '\0';)
    {
	 switch (*p)
	 {
	    case '\"':
	    case '\\':
		*q++ = '\\';
		*q++ = *p++;
		break;
	    default:
		*q++ = *p++;
		break;
	}
    }
    *q = '\0';
				/* Now build execl string using         */
				/* ch_ipcsstring as format              */
    expand  (buf, chanptr -> ch_confstr, vararray);
    ll_log (logptr, LLOGFST, "execl (%s)", buf);

    argc = cstr2arg (buf, 50, argv, ' ');

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "n=(%d) 0='%s', 1='%s', 2='%s', 3='%s', 4='%s'",
		argc, argv[0], argv[1], argv[2], argv[3], argv[4]);
#endif


    if  ((pid = fork()) == NOTOK)
    {
	ll_err (logptr, LLOGTMP, "Unable to fork for %s", argv[0]);
	if (unlink (pn_fname) == NOTOK) /* delete file */
	{
	    ll_err (logptr, LLOGFAT, "Failure to unlink file '%s'", pn_fname);
	    return (RP_FOPN);
	}
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "File: %s deleted", pn_fname);
#endif
	return (RP_NET);
    }
    if (pid == 0)               /* In child so do execl                 */
    {

ll_log (logptr, LLOGFTR, "In the child");
				/* Close down all of the pipes to child  */

	signal (SIGHUP, SIG_IGN);
	signal (SIGINT, SIG_IGN);
	signal (SIGQUIT, SIG_IGN);

	for (i = _NFILE - 1; i > 0; i--)
		close (i);
	open("/dev/null", 2);
	open("/dev/null", 1);

	execv (argv[0], &argv[1]);

	ll_log (logptr, LLOGTMP, "Unable to execl %s", argv[0]);
	exit (-1);
    }

    alarm (300);                 /* The NIFTP is not file size          */
				 /* dependent so 5 mins is ample        */
    status = pgmwait (pid);      /* Parent waits for child              */
    alarm (0);

    if (status != JNT_OK)
    {                           /* Killed by alarm or general failure   */
	if (unlink (pn_fname) == NOTOK) /* delete file */
	{
	    ll_err (logptr, LLOGFAT, "Failure to unlink file '%s'", pn_fname);
	    return (RP_FOPN);
	}
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "File: %s deleted", pn_fname);
#endif
	switch (status)
	{
	    case JNT_TEMP:
	    default:            /* temporary error                      */
		ll_log (logptr, LLOGTMP,
		"Temporary failure (%d) for file: '%s' - requeued, h='%s'",
			status, pn_fname, pn_curhost);
		return (RP_DHST);


	    case JNT_PERM:      /* permanent error                     */
		ll_log (logptr, LLOGTMP,
		    "Permanent USP failure (%d) - rejected", status);
		return (RP_NDEL);
	}
    }


    ll_log (logptr, LLOGFST, "Result is GOOD (%d)", status);
				 /* For NIFTP evidence                  */

    return(RP_MOK);
}
