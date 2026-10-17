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
/*      send return mail to author                              */

/*  Jun 81  D. Crocker    rtn_list had non-stdio code testing read-return
 *  Dec 81  D. Crocker    rtn_intro did not calculate time-in-queue right
 *  Aug 82  D. Crocker    have signature include hostname in name part
 */
#include "msg.h"
#include "adr_queue.h"

#define DOWARN  1                 /* indicate delay                     */
#define DOFAIL  0                 /* indicate final timeout failure     */

extern FILE    *mq_rfp;
#ifdef line
time_t   curtime;		/* keep lint happy - defined in other module */
#else
extern time_t   curtime;
#endif
extern struct ll_struct *logptr;
extern long     mq_lststrt;
extern int      warntime;        /* initial failure time */
extern int      failtime;        /* final failure time */
extern char     msg_sender[];
extern char     *mmdflogin;
extern char     *mquedir;
extern char     *aquedir;
extern char     *locname;               /* name of host */
extern char     *deadletter;            /* Where dead letters go */
extern char     *delim1;
extern char     *delim2;

LOCVAR char rtn_nosnd[] = "Failed mail",
	    rtn_yetsnd[] = "Waiting mail";

/**/


rtn_error (themsg, retadr, adrptr, reason) /* problem with an address   */
Msg *themsg;
char retadr[];                    /* return address                     */
struct adr_struct  *adrptr;       /* the address                        */
char    reason[];                 /* the problem                        */
{
    char    linebuf[2*LINESIZE],  /* one for each line printed          */
	    theaddr[LINESIZE];    /* canonical address                  */
    static char noreason[] = "(Reason not known)";

#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "rtn_error (%o, '%s@%s', '%s')",
		msg_cite (themsg -> mg_stat), adrptr->adr_local,
		  adrptr->adr_host ? adrptr->adr_host : "(null)", reason);
#endif

    ll_close (logptr);

    sprintf (linebuf, "%s  (%s)", rtn_nosnd, themsg -> mg_mname);
    if (rtn_mlinit (linebuf, retadr) != OK)
	return (NOTOK);           /* set up for returning               */

    rtn_pnam (adrptr, theaddr);


    sprintf (linebuf, "%s\n'%s' %s:  '%s'\n\n",
		"    Your message could not be delivered to",
		theaddr, "for the following\nreason",
		(isstr (reason) ? reason : noreason));

    ml_txt (linebuf);

    if (msg_cite (themsg -> mg_stat))
	rtn_cite (themsg -> mg_mname);
    else
	rtn_all (themsg -> mg_mname);

    return (ml_end (OK));
}
/**/

rtn_warn (themsg, retadr)         /* notify of delay in delivery        */
    Msg *themsg;
    char *retadr;
{
    char   subject[32];
#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "rtn_warn ()");
#endif

    sprintf (subject, "%s  (%s)", rtn_yetsnd, themsg -> mg_mname);
    if (rtn_mlinit (subject, retadr) != OK)
	return (NOTOK);           /* set up for returning               */

    rtn_intro (DOWARN, themsg);   /* do a warning introduction          */

    rtn_cite (themsg -> mg_mname); /* include message citation           */

    return (ml_end (OK));
}
/**/

rtn_time (themsg, retadr)         /* notify of delay in delivery        */
Msg *themsg;                      /* just cite the text                 */
char *retadr;
{
    char   subject[32];
#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "rtn_time (%s)", themsg -> mg_mname);
#endif

    sprintf (subject, "%s  (%s)", rtn_nosnd, themsg -> mg_mname);
    if (rtn_mlinit (subject, retadr) != OK)
	return (NOTOK);           /* set up for returning               */

    rtn_intro (DOFAIL, themsg);   /* do a failure introduction          */

    if (msg_cite (themsg -> mg_stat)) /* return some part of the message    */
	rtn_cite (themsg -> mg_mname);
    else
	rtn_all (themsg -> mg_mname);

    return (ml_end (OK));
}
/**/

LOCFUN
    rtn_intro (dowarn, themsg)    /* print failure intro                */
    int dowarn;                   /* warning or full failure            */
    Msg *themsg;
{
    int days;
    int msghour;
    char    linebuf[LINESIZE];

    msghour = (int) ((curtime - themsg -> mg_time) / 3600);
				  /* number of hours already in queue   */
    days = (msghour + 23) / 24;
				  /* round up to nearest whole day      */
    sprintf (linebuf, "    After %d %s (%d hours), your message %s\nfully delivered.",
	    days, ((days == 1) ? "day" : "days"), msghour,
	    (dowarn ? "has not yet been" : "could not be"));
    ml_txt (linebuf);             /* introductory line                  */

    if (dowarn)
    {
	ml_txt ("  Attempts to deliver the message will continue\n");
    	days = ((failtime - msghour) + 23) / 24;
	sprintf (linebuf, "for %d more days.", days < 0 ? 0 : days);
	ml_txt (linebuf);             /* send apologetic nonsense  */
	ml_txt ("  No further action is required by you.\n\n");
	ml_txt ("    Delivery attempts are still pending for the following address(es):\n\n");
    }
    else
	ml_txt ("\n\n    It failed to be received by the following address(es):\n\n");

    rtn_list (FALSE);      /* list who hasn't got it yet         */

#ifdef NVRCOMPIL
	Due to popular outrage, I am removing the second list, which
	shows successful transmissions, after having shown unsuccessful
	(or not-yet-successfull) ones.  In the case of large address
	lists, this can be obnoxiously long.  The text is being retained
	for reference.  (dhc)

    ml_txt ("\n    A copy HAS been sent to the following addressee(s):\n\n");

    rtn_list (TRUE);      /* list who DID get it                */
#endif

    ml_txt ("\n    Problems usually are due to service interruptions at the receiving\n");
    ml_txt ("machine.  Less often, they are caused by the communication system.\n");
}
/**/

LOCFUN
	rtn_mlinit (subject, retadr)
char    *subject;
char    *retadr;
{
    extern char *sitesignature;
    char fromline[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "rtn_mlinit (%s)", subject);
#endif

    if (!isstr (retadr))     /* no return address                  */
    {
	printx ("no return address, ");
	ll_log (logptr, LLOGTMP, "no return address");
	return (NOTOK);
    }
    sprintf (fromline, "%s %s <%s@%s>",
		locname, sitesignature, mmdflogin, locname);
				  /* From field ignores uid             */
    if (ml_1adr (NO, YES, fromline, subject, retadr) != OK)
    {                             /* no return, make Sender field       */
	ll_err (logptr, LLOGTMP, "ml_1adr");
	return (NOTOK);
    }
    return (OK);
}
/**/

LOCFUN
	rtn_list (dodone)         /* list addresses not yet delivered   */
    short dodone;                 /* list the ones delivered            */
{
    struct adr_struct   adrbuf;
    int    numlist;               /* number of addresses listed         */
    char    linebuf[LINESIZE],
	    theaddr[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "rtn_list (%d)", dodone);
#endif

    for (numlist = 0, mq_setpos (0L);  mq_radr (&adrbuf) == OK; )
    {                             /* list all failed recipients         */
	rtn_pnam (&adrbuf, theaddr);

#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "(%c) '%s'", adrbuf.adr_delv, theaddr);
#endif

	if ((dodone && adrbuf.adr_delv == ADR_DONE) ||
	    (!dodone && adrbuf.adr_delv != ADR_DONE)  )
	{                         /* format address and print it        */
	    sprintf (linebuf, "\t%s\n", theaddr);
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "\tlisted");
#endif
	    ml_txt (linebuf);
	    numlist++;
	}
    }
    if (numlist == 0)
	ml_txt ("\t(none)\n");    /* empty list                         */

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "end of return list");
#endif
}
/**/

rtn_pnam (adrinfo, buffer)        /* print the name of an addressee     */
register struct adr_struct *adrinfo;
register char *buffer;
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "rtn_pnam");
#endif

    if (adrinfo -> adr_host[0] == '\0')
	(void) strcpy (buffer, adrinfo -> adr_local);
    else                          /* have a host part                   */
	sprintf (buffer, "%s (host: %s) (queue: %s)", adrinfo -> adr_local,
		    adrinfo -> adr_host, adrinfo -> adr_que);
}


LOCFUN
	rtn_all (mname)
    char mname[];
{
    FILE *msg_tfp;
    char file[FILNSIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "rtn_all ()");
#endif


    ml_txt ("\n    Your message follows:\n\n");

    sprintf (file, "%s%s", mquedir, mname);
    msg_tfp = fopen (file, "r");

    ml_file (msg_tfp);

    fclose (msg_tfp);
}
/**/

rtn_cite (mname)
    char mname[];
{
    short     lines;
    char    linebuf[LINESIZE];
    FILE *msg_tfp;
    char file[FILNSIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "rtn_cite (%s)", mname);
#endif

    ml_txt ("\n    Your message begins as follows:\n\n");

    sprintf (file, "%s%s", mquedir, mname);
    if ((msg_tfp = fopen (file, "r")) == NULL)
    {
	ll_err (logptr, LLOGTMP, "can't open '%s' msg file '%s'",
		    mname, file);
	ml_txt ("       (Could not open message file.)\n");
	return;
    }

    while (fgets (linebuf, sizeof linebuf, msg_tfp) != NULL)
    {
	ml_txt (linebuf);         /* do headers                           */
	if (linebuf[0] == '\n')
	    break;
    }

    if (ferror (msg_tfp))
    {
	fclose (msg_tfp);
	ll_err (logptr, LLOGTMP, "Error reading text for return-to-msg_sender.");
	return;
    }

    if (!feof (msg_tfp))
    {
	for (lines = 3; --lines > 0 &&
		fgets (linebuf, sizeof linebuf, msg_tfp) != NULL;)
	{
	    if (linebuf[0] == '\n')
		lines++;          /* truly blank lines don't count      */
	    ml_txt (linebuf);
	}
	if (!feof (msg_tfp))       /* if more, give an elipses           */
	    ml_txt ("...\n");
    }
    fclose (msg_tfp);
}

/**/

dead_letter (msgname, reason)
char    *msgname;
char    *reason;
{
	FILE    *dfp;                   /* The deadletter file pointer */
	FILE    *mfp;                   /* Message related file pointers */
	char    file[FILNSIZE];
	char    buf[BUFSIZ];
	int     nread;
#ifdef DEBUG
	ll_log (logptr, LLOGFAT, "dead_letter (%s, %s)", msgname, reason);
#endif
	if ((dfp = fopen(deadletter, "a")) == NULL) {
		printx("can't open '%', mail lost.\n", deadletter);
#ifdef DEBUG
		ll_log (logptr, LLOGFAT, "can't open %s", deadletter);
#endif
		return;
	}

	fputs (delim1, dfp);          /* Message separator */
	fprintf (dfp, "Reason:   %s\n", reason);
	sprintf (file, "%s%s", mquedir, msgname);
	if ((mfp = fopen(file, "r")) == NULL) {
		fprintf (dfp, "Can't open message file %s\n", file);
	} else {
		while ((nread = fread(buf, 1, BUFSIZ, mfp)) > 0)
			fwrite(buf, 1, nread, dfp);
	}
	fputs ("-------End of deadletter, start address list-------\n", dfp);
	sprintf (file, "%s%s", aquedir, msgname);
	if ((mfp = fopen(file, "r")) == NULL) {
		fprintf (dfp, "Can't open address list file %s\n", file);
	} else {
		while ((nread = fread(buf, 1, BUFSIZ, mfp)) > 0)
			fwrite(buf, 1, nread, dfp);
	}
	fputs ("-------End of address list-------\n", dfp);
	fputs (delim2, dfp);          /* Message separator */
	fclose (dfp);
}
