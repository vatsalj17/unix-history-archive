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
/* send a piece of mail, using Unix mail command        */

/*  Basic sequence is:
 *
 *          ml_init (YES, NO, "My Name", "The Subject);
 *          ml_adr ("destination address 1");
 *          ml_adr ("destination address 2");
 *          ...
 *          ml_aend ();
 *          ml_tinit ();
 *          ml_txt ("Some opening text");
 *          ml_txt ("maybe some more text");
 *          ml_file (file-stream-descriptor-of-file-to-include);
 *          if (ml_end (OK)) != OK)
 *          {   error-handling code }
 *
 *  Arguments that are to be defaulted should be zero.
 *
 *  ml_init's arguments specify a) whether return-mail (to the sender
 *  should be allowed, b) whether a Sender field should be used to
 *  specify the correct sender (contingent on next argument), c) text
 *  for the From field, and d) text for the Subject field.  If (b) is
 *  NO, then (c)'s text will be followed by the correct sender
 *  information.
 *
 *  ml_to and ml_cc are used to switch between To and CC addresses.
 *  Normally, only To addresses are used and, for this, no ml_to call is
 *  needed.
 *
 *  An "address" is whatever is valid for your system, as if you were
 *  typing it to the mail command.
 *
 *  You may freely mix ml_txt and ml_file calls.  They just append text
 *  to the message.  The text must contain its own newlines.
 *
 *  Note that a special version of the mail command is used, to handle all
 *  the extra arguments.  If its sources weren't included with the
 *  distribution of this file, you probably have a problem.
 */

#include <signal.h>

extern struct ll_struct   *logptr;
extern char *strdup ();
extern char *pathmail,           /* location of mail command           */
	    *nammail;
extern char *cmddfldir;
extern int numfds;

LOCVAR  FILE * ml_fp;             /* handle on output to mail command   */

LOCVAR int   ml_childid;           /* process id of mail child           */
LOCVAR short ml_curarg;            /* index of next argument             */

LOCVAR char *ml_argv[100];        /* arguments to pass to execv         */
/**/

ml_init (ret, sndr, from, sub)    /* set-up for using mail command      */
int     ret,                      /* allow return mail to sender?       */
	sndr;                     /* include Sender field?              */
char    *sub,                     /* subject line                       */
        *from;			  /* from field                         */
{
    ml_argv[0] = strdup ("mail");
    if (ret)                      /* allow return to sender             */
	ml_curarg = 1;
    else
    {                             /* disable return to sender           */
	ml_argv[1] = strdup ("-r");
	ml_curarg = 2;
    }

    if (from != 0)
    {                             /* user-specified From field          */
	ml_argv[ml_curarg++] = strdup ((sndr) ? "-f" : "-g");
				  /* f => Sender field needed           */
	ml_argv[ml_curarg++] = strdup (from);
    }

    if (sub != 0)
    {                             /* user-specified Subject field       */
	ml_argv[ml_curarg++] = strdup ("-s");
	ml_argv[ml_curarg++] = strdup (sub);
    }

    return (ml_to ());		  /* set-up for To: addresses           */
}
/**/


ml_to ()			  /* ready to specify To: address       */
{
    ml_argv[ml_curarg++] = strdup ("-t");
    return (OK);
}

ml_cc ()			  /* ready to specify CC: address       */
{
    ml_argv[ml_curarg++] = strdup ("-c");
    return (OK);
}

ml_adr (address)		  /* a destination for the mail         */
char    address[];
{
    ml_argv[ml_curarg++] = strdup (address);
    return (OK);
}

ml_aend ()			  /* end of addrs                       */
{
    ml_argv[ml_curarg] = 0;
    return (OK);
}
/**/

ml_tinit ()                     /* ready to send text                 */
{
    Pip    pipdes;              /* output pipe                        */
    char temppath[128];
    register short    c;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ml_send arguments:");
    for (c = 0; c < ml_curarg; c++)
	ll_log (logptr, LLOGBTR, "arg(%d) = '%s'", c, ml_argv[c]);
#endif

    if (pipe (pipdes.pipcall))  /* for output to mail                 */
	return (NOTOK);

    ml_childid = fork ();
    switch (ml_childid)
    {
	case NOTOK:               /* bad day all around                 */
	    (void) close (pipdes.pip.prd);
	    (void) close (pipdes.pip.pwrt);
	    return (NOTOK);

	case 0:                   /* this is the child                  */
	    (void) close (0);
	    dup (pipdes.pip.prd);
	    for (c = numfds-1; c > 0; c--)
		(void) close (c);
	    open ("/dev/null", 1);
				  /* give Submit a place to send msgs   */

	    getfpath (pathmail, cmddfldir, temppath);
	    execv (temppath, ml_argv);
	    exit (NOTOK);
    }				  /* BELOW HERE is the parent           */

    (void) close (pipdes.pip.prd);

    ml_fp = fdopen (pipdes.pip.pwrt, "w");
				  /* initialize the stdio for output    */
    while (--ml_curarg >= 0)      /* give the argument list back        */
	free (ml_argv[ml_curarg]);

    return (OK);
}
/**/

ml_file (infp)                    /* send a file to the message         */
register FILE  *infp;             /* input stdio file stream pointer    */
{
    register short len;
    char    buffer[BUFSIZ];

    if ((int) ml_fp == EOF || (int) ml_fp == NULL)
	return (OK);

    while ((len = fread (buffer, sizeof (char), sizeof(buffer), infp )) > 0)
	if (fwrite (buffer, sizeof (char), len, ml_fp) != len)
	{                         /* do raw i/o                         */
	    ml_end (NOTOK);
	    return (NOTOK);
	}

    if (len < OK)
    {
	ml_end (NOTOK);
	return (NOTOK);
    }
    return (OK);
}

ml_txt (text)                     /* some text for the body             */
char text[];                      /* the text                           */
{
    if (ml_fp == (FILE *) EOF || ml_fp == (FILE *) NULL)
	return (OK);

    fputs (text, ml_fp);

    if (ferror (ml_fp))
    {
	ml_end (NOTOK);
	return (NOTOK);
    }
    return (OK);
}
/**/


ml_end (type)			  /* message is finished                */
int     type;                     /* normal ending or not               */
{
    short     retval;               /* wait return value                  */

    switch (ml_childid)
    {
	case OK:
	case NOTOK:
	    return (OK);

	default:
	    switch ((int)ml_fp)
	    {
		case OK:
		case NOTOK:
		    break;

		default:
		    if (ferror (ml_fp) || type == NOTOK)
			kill (ml_childid, SIGKILL);
		    fclose (ml_fp);
		    ml_fp = OK;
	    }

	    retval = pgmwait (ml_childid);
	    ml_childid = OK;
	    return ((retval != 0) ? NOTOK : OK);
    }
}
/**/

ml_1adr (ret, sndr, from, sub, adr)
				  /* all set-up overhead in 1 proc      */
int     ret,                      /* allow return mail to sender?       */
	sndr;                     /* include Sender field?              */
char    *sub,                     /* subject line                       */
	*from,                    /* from field                         */
	*adr;                     /* the one address to receive msg     */
{

    if (ml_init (ret, sndr, from, sub) != OK ||
	    ml_adr (adr) != OK ||
	    ml_aend () != OK ||
	    ml_tinit () != OK)
	return (NOTOK);

    return (OK);
}
