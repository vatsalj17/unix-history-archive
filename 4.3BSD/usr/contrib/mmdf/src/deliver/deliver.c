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
 *     This program module was developed as part of the University
 *     of Delaware's Multi-Channel Memo Distribution Facility (MMDF).
 *
 *     This module and its listings may be used freely by United States
 *     federal, state, and local government agencies and by not-for-
 *     profit institutions, after sending written notification to
 *     Professor David J. Farber at the above address.
 *
 *     For-profit institutions may use this module, by arrangement.
 *
 *     Notification must include the name of the acquiring organization,
 *     name and contact information for the person responsible for
 *     maintaining the operating system, name and contact information
 *     for the person responsible for maintaining this program, if other
 *     than the operating system maintainer, and license information if
 *     the program is to be run on a Western Electric Unix(TM) operating
 *     system.
 *     
 *     Documents describing systems using this module must cite its
 *     source.
 *     
 *     Development of this module was supported in part by the
 *     University of Delaware and in part by contracts from the United
 *     States Department of the Army Readiness and Materiel Command and
 *     the General Systems Division of International Business Machines.
 *     
 *     Portions of the MMDF system were built on software originally
 *     developed by The Rand Corporation, under the sponsorship of the
 *     Information Processing Techniques Office of the Defense Advanced
 *     Research Projects Agency.
 *
 *     The above statements must be retained with all copies of this
 *     program and may not be removed without the consent of the
 *     University of Delaware.
 *     
 *
 *     version  -1    David H. Crocker    March  1979
 *     version   0    David H. Crocker    April  1980
 *     version   1    David H. Crocker    May    1981
 *
 */
#include "util.h"
#include "mmdf.h"
#include <signal.h>
#include <sys/stat.h>
#include <pwd.h>

/* ***        RUN SETUID TO ROOT, so it can setuid to MMDF          *** */
/* ***                                                              *** */
/* ***  This is so that BOTH real and effective id's show as MMDF   *** */

/*
 *  OVERVIEW
 *
 *      The program may be called to process the entire mail queue or just
 *  handle some explicitly-named messages.  It will try all available
 *  channels that the mail needs to go out on, or will limit itself to
 *  explicitly, named channels.  It tries to minimize channel invocations,
 *  and process messages in order.
 *
 *      A list of channels to process is made from an argument, or simply
 *  is copied from the list of all active channels.
 *
 *      A list of messages to process is made from the arguments, or is
 *  built from a scan of the entire queue.  This list is in-core, so it is
 *  subject to serious size limits.  If more than 1000 messages are in the
 *  queue, the list is not built and the message names are read directly
 *  from the directory, as the messages are to be processed.
 *
 *      With an incore list, the messages can be sorted, according to
 *  creation date, and therefore processed in order.  In the overflow
 *  condition, this is not possible.  The primary sorting key is channels,
 *  with messages next.  This causes many passes over the messages (i.e.,
 *  access to their address lists) but minimizes channel invocations.  A
 *  final sorting key should be hosts on the channels, to minimize re-
 *  connections, but this currently is not done.
 */
/**/

/*  DELIVER:  Mail transmission manager
 *
 *  May 80 Dave Crocker     fix fd, sig, setuid handling & extral dial
 *  Jun 80 Dave Crocker     mn_mmdf reset userid, when setuid to mmdf
 *                          fix fd bugs in rtn_report
 *                          added ovr_free, fixing abnormal ending
 *  Aug 80 Dave Crocker     moved ovr_free, so free always done
 *                          removed use of ovr_seq
 *                          created msg_note, to control printout
 *  Sep 80 Dave Crocker     added -r switch to rtn-report mail call
 *  Oct 80 Dave Crocker     converted to stdio & V7 ability
 *                          split off rtn_io and ml_io
 *  Nov 80 Dave Crocker     re-organized ovr_ module
 *  Jun 81 Dave Crocker     remove use of ffio package
 *                          change adr_t2ok -> adr_t2fin, to allow
 *                          permanent failure during text phase
 *                          move adr_tok=FALSE from adr_t2fin to adr_done
 *  Jul 81 Dave Crocker     ovr_mfree check for valid ovr_mlist before free
 *         Mike Minnich     add -s, to force no-sort of queue
 *         Dave Crocker     ovr_cstep had bad fread return test
 *                          upped max # messages to 1000
 *                          adr_more=TRUE in adr_done() on temp error
 *                          disable SIGQUIT, for background mode
 *  Aug 81 Dave Crocker     change mn_*init ordering for safe callerid refs
 *                          allow partitioning queue by message size
 *  Sep 81 Dave Crocker     adr_send() added RP_RPLY and RP_PROT to BHST
 *                          case for 'temporary' error
 *  Nov 81 Dave Crocker     ovr_ismsg require message name begin "msg."
 *  Dec 81 Dave Crocker     ovr_curch -> arg rather than global
 *                          ch_init called with chan ptr, not char code
 *  Jan 82 Dave Crocker     add setgid, along with setuid
 *  Mar 83 Doug Kingston    changed to use format independent directory
 *                          access routines a la 4.2BSD  (libndir.a)
 */

/**/

extern LLog msglog;
extern char *logdfldir;
LLog *logptr = &msglog;

extern char *blt ();
extern time_t time ();
extern char *strdup();
extern struct passwd *getpwuid();

time_t   curtime;                 /* Current time secconds              */

int     effecid,                  /* system number of pgm/file's owner  */
	callerid;                 /* who invoked me?                    */
char    *callername = NULL;       /* login name of invoker (for pickup) */
int   named;                      /* number of named files to do        */

char    pgm_bakgrnd;              /* True if running as system daemon   */

extern char *mquedir;            /* directory under quedfldir for texts */
extern char *supportaddr;        /* orphaned mail goes here             */

/*  mn_  */

short   inhome;                   /* need to do full-path chdir?        */

/*  ch_  */

#include "ch.h"

extern Chan **ch_tbsrch,          /* full list of channels              */
	    **ch_exsrch,          /* default order to execute channels  */
				  /*   not same list as ch_tbsrch       */
	    *ch_ptrlist[],        /* where to put list of chans to proc */
	    *ch_curchan;          /* currently invoked channel program  */

extern int  ch_numchans;          /* number of channels which are known */

extern RP_Buf ch_rp;              /* last channel reply text            */

/*  ovr_  */

extern int  maxqueue;             /* when to do linear processng        */
DIR     *ovr_dirp;                /* handle on the queue directory      */

short   ovr_pickup;               /* pickup passive channels            */
char    ovr_dolin;                /* force linear search                */
/*NOSTRICT*/
time_t  ovr_time = 0L;            /* time based selection (0 = off)     */

/**/

/*  msg_  */

#include "msg.h"		/*  msg_cite() is defined in adr_queue.h  */

char    msg_sender[ADDRSIZE + 1]; /* return address of current message  */
/*NOSTRICT*/
time_t  msg_ttl = 0L;           /* Time to live for cache entries (secs) */
				/* This value will override the channel's */
				/* value if it is non-zero */

/*  adr_  */

#include "adr_queue.h"

long    adr_start;              /* Start of sublist being sent */
long    adr_end;                /* Where we left off to go back */
short   adr_skipped,              /* some addresses were skipped        */
	adr_more,                 /* some addreseses not completed      */
	adr_tok;                  /* adr ok; text not yet sent          */


/*****  (mn_)  MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN  ***** */

char    obuf[BUFSIZ];   /* buffering for STDOUT */

extern char *quedfldir,          /* home directory for mmdf            */
	    *aquedir,            /* subordinate address directory      */
	    *squepref;
extern unsigned mailsleep;   /* how long daemon should sleep       */

main (argc, argv)
int       argc;
char   *argv[];
{
    extern char *dupfpath ();

    mmdf_init (argv[0]);
    setbuf( stdout, obuf );
    siginit ();                   /* standard interrupt initialization  */
				  /* distinguish different delivers     */

    getwho (&callerid, &effecid); /* who am I and who is running me?    */

    time (&curtime);              /* Get current time (in hours)        */

    mn_mmdf();                    /* set up id's properly               */
    mn_arginit (argc, argv);      /* listen to the parameters           */

    if (!inhome && chdir (quedfldir) < OK)	/* already in quedfldir */
	    err_abrt (RP_LIO, "can't chdir to %s", quedfldir);

    if (named != 0)
				  /* active send of named messages      */
	ovr_named (argc, argv);
				  /* argv has the list                  */
    else
    {                             /* go through the whole queue         */
	if (pgm_bakgrnd)          /* background daemon                  */
	    FOREVER
	    {
		ovr_cstep (ovr_dolin, 0);
				  /* do the entire mail queue           */
		mn_epilogue (TRUE);
				  /* random special actions             */
		sleep (mailsleep);/* DAEMON PERIODICITY                 */
	    }
	else                      /* one-shot pass over the queue       */
	    ovr_cstep (ovr_dolin, 0);
    }


    /*
     *  Poll only if full-queue daemon and not invoked for pickup only.
     */
    mn_epilogue ((named == 0) && (!ovr_pickup));

#ifdef DEBUG
    printx ("%s:  normal end\n", argv[0]);
#endif
    exit (RP_OK);                 /* "normal" end, even if pgm_bakgrnd      */
}
/**/

LOCFUN
	mn_mmdf ()                /* setuid to mmdf: bypass being root  */
{                                 /* get sys id for mmdf; setuid to it  */
    extern char *pathsubmit;     /* submit command file name           */
    extern char *cmddfldir;      /* directory w/mmdf commands          */
    char    temppath[LINESIZE];
    struct stat    statbuf;

#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "mn_mmdf(); effec==%d, pickup==%d",
		effecid, ovr_pickup);
#endif

/*  the following is a little strange, doing a stat on the object
 *  file, because setuid-on-execute does not work when the caller
 *  is root, as will happen when this is started by the rc file.
 *  hence, the effective id, from a getuid, will show root & not mmdf.
 *
 *  the goal is to have this process be name-independent of the caller,
 *  so that returned mail comes from mmdf and not the invoker.
 *
 *  in pickup mode, the id of the caller has to be retained, since
 *  pobox channels use that to determine access rights to mail.
 *
 *  Also sets gid to mmdf's gid  --  <DPK@BRL>
 */

    if (effecid == 0 && !ovr_pickup)
    {
	getfpath (pathsubmit, cmddfldir, temppath);

	if (stat (temppath, &statbuf) < OK)
	    err_abrt (RP_LIO, "Unable to stat %s", temppath);
				  /* use "submit" to get mmdf id        */

	if (setgid (statbuf.st_gid) == NOTOK)
	    err_abrt (RP_LIO, "Can't setgid to mmdf (%d)", statbuf.st_gid);
	if (setuid (statbuf.st_uid) == NOTOK)
	    err_abrt (RP_LIO, "Can't setuid to mmdf (%d)", statbuf.st_uid);

	effecid = statbuf.st_uid; /* mostly needed for return mail      */
    }
}

LOCFUN
	mn_arginit (argc, argv)   /* basic process initialization       */
int       argc;
char   *argv[];
{
    register int    tmp;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "mn_arginit");
#endif

    for (tmp = 1; tmp < argc; tmp++)
    {                             /* tell us your arguments             */
	if (*argv[tmp] == '-')    /* a real switch                      */
	    prm_set (&argv[tmp][1]);
	else                      /* was a msgname, if not a switch     */
	    named++;              /* count number of named messages     */
    }

    prm_end ();                   /* parm cleanup & value verification  */
}

LOCFUN
	mn_epilogue (maypoll)     /* cleanup after a pass               */
    int maypoll;
{
    register Chan **chanptr;

#ifdef DEBUG
	    ll_log (logptr, LLOGBTR, "mn_epil");
#endif
    ovr_end (OK);                 /* no children kept around            */

#ifndef RUNALON
    if (maypoll)                  /* do any channel-polling             */
	ch_poll (ch_ptrlist);
#endif

    for (chanptr = ch_tbsrch; *chanptr != 0; chanptr++)
	(*chanptr) -> ch_access &= ~DLVRDID;
				/* clear the "did it" bit on full list  */
    ll_close (logptr);         /* in case it's in overflow condition    */
}
/*******************  (prm_)  PARMETER SETTING  ********************* */

prm_set (str)                     /* assign parm vals, as user specs    */
register char  *str;
{
    extern char *prm_cload();

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "prm_set '%s'", str);
#endif
    for (; !isnull (*str); str++)
	switch (*str)
	{
	    case 'L':           /* Specify alternate logfile, SU only */
		if ((callerid == 0 || callerid == effecid) && *(++str)) {
		    ll_log (logptr, LLOGFST, "Logfile is '%s'", str);
		    logptr->ll_file = dupfpath(str, logdfldir);
		}
		return;

	    case 'T':             /* set daemon sleep time       */
		str++;
		if (*str != '\0')
		{
		    mailsleep = atoi (str);
#ifdef DEBUG
		    ll_log (logptr, LLOGFTR, "Mailsleep %dsecs", mailsleep);
#endif
		}
		return;

	    case 'V':
		if ((callerid == 0 || callerid == effecid) && *(++str)) {
		    char *cp = str;
		    int level;

		    ll_log (logptr, LLOGFST, "Loglevel '%s'", str);
		    if ((level = tai_llev (1, &cp)) < 0)
			logptr->ll_level = level;
		}
		return;

	    case 'b':             /* run as background daemon           */
		if (callerid == 0 || callerid == effecid)
		    pgm_bakgrnd = TRUE;
		else
		    err_abrt (RP_PARM,
				"User id (%d) not authorized for background",
				callerid);
		break;

	    case 'c':             /* process specific channels          */
		str = prm_cload (++str);
		if (isnull (*str))
		    return;
		break;

	    case 'd':             /* already in quedfldir, only for SU */
		if (callerid == 0 || callerid == effecid)
		    inhome = TRUE;
		break;

	    case 'l':           /* Specify the time to live in minutes */
		/*NOSTRICT*/
		msg_ttl = ((time_t)atoi(++str)) * 60L;
		return;

	    case 'm':             /* set maxsort threshold		*/
		str++;
		if (*str != '\0')
		{
		    maxqueue = atoi(str);
#ifdef DEBUG
		    ll_log (logptr, LLOGFTR, "Maxsort set to '%d' msgs",
						maxqueue);
#endif
		}
		return;

	    case 'p':             /* pickup passive channel mail        */
		ovr_pickup = TRUE;
		break;

	    case 's':             /* force linear search                  */
		ovr_dolin = TRUE;
		break;

	    case 't':             /* time selection of messages */
		ovr_time = (time_t) atoi(++str) * 3600; /* get value in secs */
		return;

	    case 'w':
		domsg = TRUE;
		break;            /* caller wants to watch                */

	    default:
		err_abrt (RP_PARM, "switch '%c' is not defined", *str);
	}
}
/**/

prm_end ()                        /* cleanup vals after user settings   */
{
    register int ch_cand,         /* candidate for checking             */
		 ch_auth;         /* authorized for checking            */

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "prm_end");
#endif

    (void) signal (SIGINT, SIG_IGN);    /* disable keyboard interrupt   */
    if (pgm_bakgrnd)              /* running as daemon                  */
    {                             /* get rid of unnecessary fd's        */
	(void) signal (SIGQUIT, SIG_IGN);/* disable keyboard quit       */
	sprintf (logptr -> ll_hdr, "BG-%04d", getpid () % 1000);
				  /* distringuish different daemons     */
	freopen ("/dev/null", "r", stdin);
				  /* open stdin to null                 */
	if (!domsg)               /*  stdout null if no domsg           */
	    freopen ("/dev/null", "w", stdout);
	freopen ("/dev/null", "w", stderr);
				  /*  stderr null always                */
    }
    if (!ovr_pickup && !domsg)
	(void) signal (SIGHUP, SIG_IGN); /* Ignore hangups               */

    if (ovr_pickup) {
	register struct passwd *pw;

	if ((pw = getpwuid(callerid)) == (struct passwd *)NULL)
	    err_abrt (RP_LIO, "UID (%d) not in /etc/passwd, pickup denied");
	callername = strdup(pw->pw_name);
#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "pickup caller's name is '%s'", callername);
#endif
	printx ("pickup caller's name is '%s'\n", callername);
    }

    if (ch_ptrlist[0] == (Chan *) 0) /* none requested, make all eligible  */
    {
#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "do all chans");
#endif
	for (ch_cand = 0; ch_cand <= ch_numchans; ch_cand++)
	    ch_ptrlist[ch_cand] = ch_exsrch[ch_cand];
				  /* copy the default execution list    */
    }
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "check channel authorization");
    ll_log (logptr, LLOGBTR, "callerid=%d,effecid=%d,bakgrnd=%d,pickup=%d",
		callerid, effecid, pgm_bakgrnd, ovr_pickup);
#endif
    for (ch_cand = ch_auth = 0; ch_ptrlist[ch_cand] != (Chan *) 0; ch_cand++)
    {                             /* what channels really to be checked */
	/* CHANNEL INVOCATION POLICY */
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "(%d) %s:  passive = %o", ch_cand,
		    ch_ptrlist[ch_cand] -> ch_name,
		    (ch_ptrlist[ch_cand] -> ch_access) & DLVRPSV);
#endif
	if ((!pgm_bakgrnd && (callerid != 0) && (callerid != effecid) &&
	    (ch_ptrlist[ch_cand] -> ch_access & DLVRBAK)) ||
				  /* background only and we are not     */
				  /* root or mmdf                       */
	    (!ovr_pickup && (ch_ptrlist[ch_cand] -> ch_access & DLVRPSV)) ||
				  /* passive only & we aren't           */
	    (ovr_pickup && !(ch_ptrlist[ch_cand] -> ch_access & DLVRPSV)) ||
				  /* active chan & we aren't            */
	    (ovr_pickup && (ch_ptrlist[ch_cand] -> ch_login != 0) &&
	      !lexequ(ch_ptrlist[ch_cand] -> ch_login, callername)))
	{
	    printx ("Channel %s (%s) not authorized\n",
			ch_ptrlist[ch_cand] -> ch_show,
			ch_ptrlist[ch_cand] -> ch_name);
	    ll_log (logptr, LLOGTMP, "Channel %s (%s) not authorized",
			ch_ptrlist[ch_cand] -> ch_show,
			ch_ptrlist[ch_cand] -> ch_name);
	}
	else
	    ch_ptrlist[ch_auth++] = ch_ptrlist[ch_cand];
    }
    ch_ptrlist[ch_auth] = 0;      /* end list of authorized channels    */
}
/**/

LOCFUN  char *                    /* make list of chans to process      */
		prm_cload (strtptr)
    register char *strtptr;
{                                 /* format: "channame1,channame2;"     */
    short namelen;
    char channame[LINESIZE];
    short more;
    register int ch_ind;
    register char *endptr;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "prm_cload (%s)", strtptr);
#endif

    for (more = TRUE, ch_ind = 0, endptr = strtptr; more; strtptr = ++endptr)
    {                             /* list is put into ch_ptrlist        */
	FOREVER
	{
	    switch (*endptr)
	    {
		case '\0':
		case ';':
		    more = FALSE;
		case ',':
		    namelen = endptr - strtptr;
		    blt (strtptr, channame, namelen);
		    channame[namelen] = '\0';
		    break;

		default:
		    endptr++;
		    continue;
	    }
	    break;
	}

#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "channame = '%s'", channame);
#endif
	if ((ch_ptrlist[ch_ind] = ch_nm2struct (channame)) == (Chan *) NOTOK)
	{
	    printx ("channel '%s' is not defined\n", channame);
	    ll_log (logptr, LLOGFST, "*** channel '%s' is not defined", channame);
	    continue;
	}
	ch_ind++;
    }
    ch_ptrlist[ch_ind] = (Chan *) 0; /* end of list of chans to process    */
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "chans to proc = %d", ch_ind);
    ll_log (logptr, LLOGFTR, "chans:");
    for (ch_ind = 0; ch_ptrlist[ch_ind] != (Chan *) 0; ch_ind++)
	ll_log (logptr, LLOGFTR, "(%d) %s", ch_ind,
		     ch_ptrlist[ch_ind] -> ch_name);
#endif
    return (--endptr);              /* indicate where end of parameter is */
}
/************  (ovr_)  OVERALL SEQUENCING MANAGEMENT  *************** */

extern int  warntime,    /* hours to wait before notification  */
	    failtime;    /* hours to wait before returning msg */


LOCVAR Msg *ovr_mlist;              /* list of files in queue             */

ovr_end (status)                    /* cleanup and terminate session      */
register int    status;
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ovr_end (%d)", status);
#endif

    if (status == NOTOK)            /* abnormal termination               */
	mq_rkill (OK);           /* terminate message processing       */

    ch_end (status);                /* force child to die                 */
}


ovr_named (argc, argv)            /* process a set of named messages    */
    int   argc;
    char *argv[];
{

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ovr_named ()");
#endif

    if (ovr_malloc (named) == NOTOK)
	err_abrt (RP_MECH, "unable to ovr_malloc for (%d) messages", named);

    named = ovr_nload (argc, argv); /* load names into ovr_mlist          */
    ovr_cstep (ovr_dolin, named);   /* step through channels for list     */

    ovr_mfree ();                  /* free up ovr_malloc'd space          */
    ovr_end (OK);                 /* signal done with channel           */
    return;
}
/**/

ovr_queue (curch, dolin)         /* Process entire message queue       */
	Chan *curch;
	int dolin;

{
    int ovr_qnum ();
    int maxcount;
    int actualcount;
    char subque[LINESIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR,
		"ovr_queue (%s, %o)", curch -> ch_name, dolin);
#endif

    sprintf (subque, "%s%s", squepref, curch -> ch_queue);
    if ((ovr_dirp = opendir (subque)) == NULL)
	 /*   err_abrt (RP_FOPN, "can't open address queue"); */
			      /* SEK - I don't see any reason to abort */
    {
	ll_log (logptr, LLOGGEN, "Failure to open queue dir '%s'", subque);
	ovr_dirp = (DIR *) EOF;
	return;
    }

    if (dolin)                /* sequential processing demanded     */
    {
	ovr_mstep (curch, TRUE, NOTOK);
	goto endit;
    }

    printx ("sorting the queue.\n");
    maxcount = ovr_qnum ();   /* how many messages in the queue?    */

    if (maxcount == 0)
	goto endit;

/*  The following is a hack to deal with the fact that the queue searching
 *  is linear and therefore VERY slow when it gets large.  This can lead
 *  to deliver's taking too long, before coming up with the first message
 *  to send.  A caller, using a pickup channel, may timeout before getting
 *  the next data packet.
 */
    if (maxcount > maxqueue)    /* XXX was "ovr_pickup && ..." */
    {                             /* sequential, if too large           */
	ovr_mstep (curch, TRUE, NOTOK);
	goto endit;
    }

    if (ovr_malloc (maxcount += 5) == NOTOK)
				  /* allow a few extra arrivals         */
    {                             /* problem with allocation            */
	ovr_mstep (curch, TRUE, maxcount);
	goto endit;               /* cop out with linear processing     */
    }

    actualcount = ovr_qload (curch, maxcount);
    printx ("%d messages, %d to be processed\n", maxcount - 5, actualcount);
    (void) fflush (stdout);
    if (actualcount != 0)
    {                             /* actual number to be processed      */
	ovr_sort (actualcount);      /* get into processing order          */
	ovr_mstep (curch, FALSE, actualcount);
				  /* step through channels              */
    }
    ovr_mfree ();                 /* free up ovr_mlist storage          */

endit:
    closedir (ovr_dirp);
    ovr_dirp = (DIR *) EOF;
}
/**/

LOCFUN
	ovr_cstep (dolin, numproc) /* step by chan, thru list of msgs    */
	    int dolin;
	    int numproc;
{
    register Chan *curch;
    register int ind;
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ovr_cstep (linear=%o, num=%d)", dolin, numproc);
#endif

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "chans:");
    for (ind = 0; ch_ptrlist[ind] != (Chan *) 0; ind++)
	ll_log (logptr, LLOGFTR, "(%d) %s", ind, ch_ptrlist[ind] -> ch_name);
#endif

    for (ind = 0; (curch = ch_ptrlist[ind]) != (Chan *) 0; ind++)
    {                             /* choose channel list & process it   */
#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "checking %s (%s)",
		    curch -> ch_show, curch -> ch_name);
#endif
	printx ("\n[ Checking for %s (%s) mail ]\n",
		    curch -> ch_show, curch -> ch_name);
	(void) fflush (stdout);

/* process message queue, for this channel; abort if problem with channel */
	if (numproc != 0)       /* we have a named list */
	    ovr_mstep (curch, dolin, numproc);
	else                    /* build the list from the queue */
	    ovr_queue (curch, dolin);
    }
}
/**/

LOCFUN
	ovr_mstep (curch, dolin, numproc)
				  /* step through msgs for a channel    */
    Chan *curch;
    int dolin;
    int numproc;
{
    register struct direct *dp;
    register int  ind;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ovr_mstep(chan=%s, lin=%d, %d)",
		curch -> ch_name, dolin, numproc);
#endif

    if (dolin)                /* have to read the queue directory   */
    {                             /* for each message in directory      */
	ll_log (logptr, LLOGGEN, "unsorted queue search");
	printx ("Linear search of unsorted full queue.\n\t");
	if (ovr_malloc (1) == NOTOK)
	    err_abrt (RP_MECH, "unable to ovr_malloc for 1 message");
				  /* create a placeholder               */
	for (rewinddir (ovr_dirp), numproc = 0;
		(dp = readdir (ovr_dirp)) != NULL; )
	{                         /* straight linear processing         */
	    if (ovr_ismsg (dp))
	    {
		numproc++;
		(void) strcpy (ovr_mlist[0].mg_mname, dp->d_name);
		if (msg_proc (curch, &ovr_mlist[0]) == RP_NET)
		{
			printx("killing channel '%s'\n", curch->ch_name);
			(void) fflush( stdout );
			ch_sbend( NOTOK );
		}
	    }
	}
	ovr_mfree ();
    }
    else                          /* we have an in-core list            */
    {
	for (ind = 0; ind < numproc; ind++)
	    switch (msg_proc (curch, &ovr_mlist[ind]))
	    {
		case RP_DONE:
		    break;

	        case RP_NET:        /* problem with channel */
		    printx("killing channel '%s'\n", curch->ch_name);
		    (void) fflush( stdout );
		    ch_sbend (NOTOK);
	    }
    }
    return;
}
/**/

LOCFUN
	ovr_nload (argc, argv)  /* load named message list            */
    register int   argc;
    char *argv[];
{
    register int argind,
		 msgind;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ovr_nload()");
#endif

    for (msgind = 0, argind = 1; argind < argc; argind++)
	if (argv[argind][0] != '-') {
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "name='%s'", argv[argind]);
#endif
	    (void) strcpy (ovr_mlist[msgind++].mg_mname, argv[argind]);
	}

    return (msgind);
}

LOCFUN  int
    ovr_qnum ()  /* how many messages are in the queue */
{
    register struct direct *dp;
    register int num;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ovr_qnum()");
#endif

    for (rewinddir (ovr_dirp), num = 0; (dp = readdir (ovr_dirp)) != 0; )
	if (ovr_ismsg (dp)) {
	    num++;
	    if (num > maxqueue)
		break;
	}


#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "%d msgs", num);
#endif
    return (num);
}
/**/

LOCFUN
	ovr_qload (curch, numproc)  /* get list of messages from queue    */
	    Chan *curch;
	    int numproc;
{
    struct stat statbuf;
    char msg_txname[LINESIZE];
    register struct direct *dp;
    register int num;
    time_t cutoff_time;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ovr_qload(%d)", numproc);
#endif
    rewinddir (ovr_dirp);
    if (ovr_time != 0L) {
	time(&curtime);			/* get current time in seconds */
	cutoff_time = curtime - ovr_time;
    }
    for (num = 0; num < numproc && (dp = readdir (ovr_dirp)) != NULL; )
	if (ovr_ismsg (dp))
	{
	    (void) strcpy (ovr_mlist[num].mg_mname, dp->d_name);
				  /* get queue entry name (msg name)   */
	    if (mq_rinit (curch, &ovr_mlist[num], msg_sender) != OK)
		continue;

	    if (ovr_time != 0L && ovr_mlist[num].mg_time < cutoff_time) {
		mq_rkill (OK);
		printx ("%s: old msg\n", dp->d_name);
		(void) fflush (stdout);
		continue;               /* Ignore old messages */
	    }

#ifdef LARGESIZE
	    sprintf (msg_txname, "%s%s", mquedir, ovr_mlist[num].mg_mname);
	    if (stat (msg_txname, &statbuf) != NOTOK &&
		    st_gsize (&statbuf) > LARGESIZE)
	    {
		ovr_mlist[num].mg_large = TRUE;
#ifdef DEBUG
		ll_log (logptr, LLOGFTR, "large message (%s)",
			    ovr_mlist[num].mg_mname);
#endif
	    }
	    else
		ovr_mlist[num].mg_large = FALSE;
#endif
	    mq_rkill (OK);
	    num++;
	}

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "actual = %d", num);
#endif
    return (num);
}
/**/

LOCFUN
	ovr_qcompare (a, b)       /* compare msg creation times         */
      Msg *a,             /*    msg a                           */
	  *b;                /*    msg b                           */
{                                 /* called by qsort                    */
    register long   a2b_diff;

#ifdef LARGESIZE
    if (b -> mg_large)           /* B > 100K bytes                     */
    {
	if (!a -> mg_large)      /* but A is not                       */
	    return (-1);
    }
    else
	if (a -> mg_large)       /* A isn't but B is                   */
	    return (1);
#endif
    if ((a2b_diff = a -> mg_time - b -> mg_time) == 0)
	return (0);               /* straight time-of-creation compare  */
    return ((a2b_diff  < 0) ? -1 : 1);
}

LOCFUN
	ovr_sort (numproc)        /* Sort the entries by creation date.   */
	   int numproc;
{
#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "ovr_sort (%d)", numproc);
    ovr_log ("before", numproc);
#endif
    qsort ((char *)ovr_mlist, numproc, sizeof (Msg), ovr_qcompare);
#ifdef DEBUG
    ovr_log ("after", numproc);
#endif
}

#ifdef DEBUG
ovr_log (tag, numproc)
    char tag[];
    int numproc;
{
    register int ind;

    ll_log (logptr, LLOGFTR, "que %s:", tag);
    for (ind = 0; ind < numproc; ind++)
	ll_log (logptr, LLOGFTR, "(%d) %s %ld %s", ind,
		    ovr_mlist[ind].mg_mname, ovr_mlist[ind].mg_time,
		    (ovr_mlist[ind].mg_large == TRUE) ? "LARGE" : "");
}
#endif
/**/
LOCFUN
	ovr_malloc (count)          /* allocate array for msg names       */
unsigned  count;                    /* number of entries in queue list    */
{
    extern char *calloc();

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "ovr_malloc (%d)", count);
#endif
    if (ovr_mlist != (Msg *)0) {
	ll_log (logptr, LLOGTMP, "*** ovr_malloc of un-freed ovr_mlist");
	ovr_mfree ();
    }

    /*NOSTRICT*/
    if ((ovr_mlist = (Msg *) calloc (count, sizeof (Msg))) == (Msg *)0) {
	ll_log (logptr, LLOGTMP, "*** ovr_malloc error (%d msgs -> %d bytes)",
		count, (count * sizeof (Msg)));
    	return(NOTOK);
    }
    return (count);
}

LOCFUN
	ovr_ismsg (entry)         /* a processable message?             */
    register struct direct *entry;
{
#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "ovr_ismsg (name='%s')",
			entry -> d_name);
#endif
    return ((entry -> d_namlen <= MSGNSIZE
	     && equal (entry -> d_name, "msg.", 4)) ? TRUE : FALSE);
}

LOCFUN
	ovr_mfree ()              /* free queue list storage            */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "ovr_mlist (%o)", ovr_mlist);
#endif
    if(ovr_mlist != (Msg *)0)
	    free ((char *) ovr_mlist);
    ovr_mlist = (Msg *)0;
}
/****************  (msg_)  HANDLE A SINGLE MESSAGE  ***************** */

extern int    errno;                /* system error number                */

msg_proc (curch, themsg)        /* get, process & release a msg       */
    Chan *curch;
    Msg *themsg;
{
    register char   retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "msg_proc (%s)", themsg -> mg_mname);
#endif
    if (mq_rinit (curch, themsg, msg_sender) == OK)
    {
	msg_note(themsg);     /* inform watchers of intent to send  */
	retval = msg_mng (curch, themsg);
	mq_rkill (OK);
	printx ("End of processing for %s\n", themsg -> mg_mname);
	(void) fflush(stdout);
	return (retval);
    }
    return (RP_DONE);
}

msg_note (themsg)                 /* notify watchers about this msg     */
    Msg *themsg;
{
    int elapsed;                /* Time in queue in hours */

    /*NOSTRICT*/
    elapsed = (int)((curtime - themsg -> mg_time)/3600L);
    printx ("\nMessage '%s', from '%s'\n", themsg -> mg_mname, msg_sender);
    printx ("Queued: %d Days, %d Hours\n", elapsed/24, elapsed%24);
    ll_log (logptr, LLOGPTR, "[ %s, %ld, %s ]",
	    themsg -> mg_mname, themsg -> mg_time, msg_sender);
}
/**/

LOCFUN
	msg_dequeue (theque, themsg) /* remove message from queue          */
	    char *theque;
	    Msg *themsg;
{
    char thename[LINESIZE];
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "msg_deque (%s,%s)", 
		(theque == (char *) 0) ? "(Base entry)" : theque,
		themsg -> mg_mname);
#endif

    printx ("dequeuing %s from %s\n", 
		themsg -> mg_mname, theque != 0 ? theque : "main queue");
#ifdef RUNALON
    return;
#endif

    if (theque == (char *) 0)
	sprintf (thename, "%s%s", aquedir, themsg -> mg_mname);
    else
	sprintf (thename, "%s%s/%s",
			squepref, theque, themsg -> mg_mname);

    if (unlink (thename) < OK) /* this is real queue handle  */
	ll_err (logptr, LLOGTMP, "Problem unlinking '%s' address: %s",
		    themsg -> mg_mname, thename);

    if (theque == (char *) 0)
    {                             /* get rid of ALL the message */
	sprintf (thename, "%s%s", mquedir, themsg -> mg_mname);
	if (unlink (thename) < OK) /* the text is just "baggage"         */
	    ll_err (logptr, LLOGTMP, "Problem unlinking %s text: '%s'",
		themsg -> mg_mname, thename);
    }
}

/**/

LOCFUN
	msg_mng (curch, themsg)    /* overall management of transmission */
		Chan *curch;
		Msg *themsg;
{
    int retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "msg_mng");
#endif

    if ((retval = adr_each (curch, themsg)) == RP_DONE)
    {                             /* do a FOREACH loop thru addr list   */
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "skipped (%o), more(%o)",
			adr_skipped, adr_more);
#endif
	if (adr_skipped && adr_more)
	    return (RP_OK);       /* not really done with message   */

	if (!adr_more)
	{                         /* done with this sub-queue           */
	    msg_dequeue (curch -> ch_queue, themsg); /* ain't no mo' to process         */
	    if (!adr_skipped)     /* done with entire message           */
	    {
		msg_dequeue ((char *) 0, themsg);
		printx ("Message completely processed.\n");
	    }
	    else
		printx ("Message fully processed in sub-queue '%s'\n",
			    curch -> ch_queue);
	    (void) fflush (stdout);
	}
	return (RP_DONE);
    }
    return (retval);
}
/************  (adr_)  INDIVIDUAL ADDR DELIVERY ATTEMPT  ************ */

LOCVAR Chan *adr_lastdone;        /* flag to tell if e-o-list needed    */


adr_each (curch, themsg)          /* do each address                    */
	Chan *curch;              /* Channel of current interest        */
	Msg *themsg;
{
    RP_Buf rply;                  /* success of address                 */
    struct adr_struct theadr;     /* address parts                      */
    char    ch_chost[FILNSIZE];   /* current host being processed       */
    char    first;                /* first address send to a channel?   */
    char    addr_sent;            /* At least one address went to chan  */

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "adr_each");
#endif
/*  Normal termination returns RP_DONE; a problem specific to the channel
 *  returns OK (ok to proceed to next channel).
 *  Problems with the address list are treated as general,
 *  so that the return is a general NOTOK.
 */

    adr_more =
	adr_tok =
	adr_skipped =
	addr_sent = FALSE;

    adr_start = 0L;
    for (first = TRUE, mq_setpos (0L); ; )
    {                             /* go through the list                */
	switch (mq_radr (&theadr))
	{                         /* was theadr setup correctly?        */
	    case DONE:            /* end of list                        */
		if (!first)       /* Something was exchanged with chan  */
			adr_done (themsg);
				  /* mark temps and return              */
		return (RP_DONE);

	    case NOTOK:           /* rest of list must be skipped       */
		return (RP_NO);   /* treat as permanent problem         */
	}

	/* filter what addresses get processed */
	if (theadr.adr_delv == ADR_DONE)
	    continue;             /* this one's already been finished   */

	if (!lexequ (curch -> ch_queue, theadr.adr_que))
	{
	    adr_skipped = TRUE;   /* pending, but...                    */
	    continue;             /* not the chan we are looking for    */
	}

	if (ovr_pickup && (curch -> ch_login == 0)
	  && !lexequ(callername, theadr.adr_host)) {
	    adr_skipped = TRUE;   /* caller can only pickup for his host */
	    continue;
	}

	if (curch != ch_curchan || !strequ (ch_chost, theadr.adr_host))
	{
	    /*
	     *  This stuff gets called the first time through but
	     *  that's OK since the routines all handle the case when
	     *  nothing has actually happened yet.
	     */
	    if (addr_sent)                      /* update any temp-ok's */
		if (adr_hdone (themsg, ch_chost, ch_curchan) != RP_NOOP) {
		    addr_sent = FALSE;
		    adr_start = theadr.adr_pos;	/* start new sublist */
		}
	    (void) strcpy (ch_chost, theadr.adr_host);
	}

	if (ca_find (&curch -> ch_dead, theadr.adr_host) != 0) {
		adr_more = TRUE;
#ifdef DEBUG
		ll_log (logptr, LLOGBTR, "address skipped (dead host)");
#endif
		printx ("%s", theadr.adr_local);
		if (theadr.adr_host[0] != '\0')
			printx (" via %s", theadr.adr_host);
		printx (":  dead host\n");
		(void) fflush (stdout);
		continue;
	}

	/*
	 *  The following condition will also go off if the channel
	 *  bombs out in the middle of adr_hdone().
	 */
	if (curch != ch_curchan) {              /* need a whole new channel */
	    if (rp_isbad (ch_sbend (OK)))
		return (RP_OK);
	    if (rp_isbad (ch_sbinit (curch)))
		return (RP_OK);   /* treat as permanent problem */
	    first = TRUE;
	}

	if (first)                /* msg's 1st addr */
	{                         /* tell chan of msg_sender & name */
#ifdef DEBUG
	    ll_log (logptr, LLOGBTR, "chan new msg");
#endif
	    printx ("Channel: %s\n", curch -> ch_name);
	    (void) fflush (stdout);
	    first = FALSE;        /* treat as if done with channel      */
	    if (rp_isbad (ch_winit (msg_sender, themsg -> mg_mname)))
		return (RP_OK);
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR, "first host '%s'", ch_chost);
#endif
	}

	theadr.adr_tmp = ADR_CLR; /* reset temp bit, if set earlier     */
	rply.rp_line[0] = '\0';

	adr_end = theadr.adr_pos;

	if (rp_isbad (adr_wadr (&theadr)))
	{
	    adr_more = TRUE;
	    rply.rp_val = RP_NO;  /* give to channel & get status       */
	}
	else
	    adr_rrply (&rply, curch, themsg, &theadr);

	addr_sent = TRUE;

	switch (rply.rp_val)  /* oops    */
	{
	    case RP_NDEL:
		printx ("cannot deliver");
		(void) fflush (stdout);
		if (msg_noret (themsg->mg_stat)) {
		    printx (", error returns not wanted");
		    ll_log (logptr, LLOGGEN, "%s err noret", themsg->mg_mname);
		}
		else if (rtn_error (themsg, msg_sender, &theadr, rply.rp_line)
		     == OK) {
		    printx (", returned");
		    ll_log (logptr, LLOGGEN, "%s err ret", themsg -> mg_mname);
		} else {
		    char orphanage[ADDRSIZE];

		    sprintf (orphanage, "Orphanage <%s>", supportaddr);
		    printx (", couldn't return\n");
		    printx ("to '%s', trying orphanage", msg_sender);
		    (void) fflush (stdout);
		    ll_log (logptr, LLOGGEN, "%s orph ret", themsg->mg_mname);
		    if (rtn_error (themsg, orphanage, &theadr, rply.rp_line)
			 == OK) {
			printx (", returned");
		    } else {
			printx (", couldn't return.");
			dead_letter (themsg->mg_mname, rply.rp_line);
		    }
		}
		break;

	    case RP_NO:
		adr_more = TRUE;                /* Paranoid */
		printx("queuing for retry");
		break;

	    case RP_NET:        /* problem with channel, so terminate it */
	    case RP_NIO:
		printx("\n%s channel I/O error\n", curch->ch_name);
		adr_t2fin (curch, themsg, RP_NO);
		return (RP_NET);        /* skip the rest of this message */
	}

	mq_wrply (&rply, themsg, &theadr);
				  /* modify addr's queue entry)         */
	printx ("\n");
	(void) fflush (stdout);
    }
    /* NOTREACHED */
}

adr_log (theval, thechan, themsg, theadr) /* record final status of address */
    char theval[];
    Chan *thechan;
    Msg *themsg;
    struct adr_struct   *theadr;
{
    time_t thetime,
	   elapsed;
    int   msg_elhour,           /* # hours since message queued       */
	    msg_elsec;            /* additional seconds                 */

    time (&thetime);

    elapsed = thetime - themsg -> mg_time;
    msg_elhour = (int) (elapsed / (time_t) 3600);
    msg_elsec = (int) (elapsed % (time_t) 3600);

    if (lexequ (theadr -> adr_que, thechan -> ch_name))
	ll_log (logptr, LLOGFST, "end %s (%s) %s %s %s [%d:%2d:%2d]",
		themsg -> mg_mname, theval, theadr -> adr_que,
		theadr -> adr_host, theadr -> adr_local,
		msg_elhour, msg_elsec / 60, msg_elsec % 60);
    else                        /* chan name is not redundant w/queue */
	ll_log (logptr, LLOGFST, "end %s (%s) %s(%s) %s %s [%d:%2d:%2d]",
		themsg -> mg_mname, theval, theadr -> adr_que,
		thechan -> ch_name, theadr -> adr_host, theadr -> adr_local,
		msg_elhour, msg_elsec / 60, msg_elsec % 60);
}
/**/

LOCFUN
	adr_t2fin (chan, msg, final) /* convert temp to final status       */
    register Chan *chan;             /* channel to convert stats for       */
    register Msg *msg;
    int  final;                  /* final status                       */
{
    RP_Buf rply;
    struct adr_struct theadr;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "adr_t2fin (%s[%s], %s, %s) tok=%d",
		chan -> ch_show, chan -> ch_name,
		msg -> mg_mname, rp_valstr (final), adr_tok);
#endif

    if (!adr_tok)             /* there weren't any temp ok's    */
	return;

    adr_tok = FALSE;          /* reset */

    switch (rp_gbval (final))
    {                     /* if result is permanent             */
	case RP_BOK:      /* good ending                        */
	    final = RP_AOK;
	case RP_BNO:      /* irreversibly bad ending            */
	    rply.rp_val = RP_DONE;
	    rply.rp_line[0] = '\0';
	    mq_setpos (adr_start);
	    /*
	     *  A side effect of the ordering of the condition below
	     *  is to cause us to read to the same point that brought
	     *  us here (the first address not in this sublist.  In this
	     *  we detect it because adr_pos is nolonger <= adr_end.
	     *                                  DPK@BRL
	     */
	    while(mq_radr (&theadr) == OK && theadr.adr_pos <= adr_end)
	    {             /* get address values & mark status    */
		if (lexequ (theadr.adr_que, chan -> ch_queue) &&
		    theadr.adr_tmp == ADR_AOK && theadr.adr_delv != ADR_DONE)
		{         /* same chan && marked with temp_ok   */
		    mq_wrply (&rply, msg, &theadr);
			  /* modify addr's entry as per reply */
		    adr_log (rp_valstr (final), chan, msg, &theadr);

		    /*
		     *  Here we handle the case of a permanent
		     *  error after the successful receipt of
		     *  the addresses.   (From Steve Kille, UCL)
		     */
		    if (rp_gbval (final) == RP_BNO) {
			printx ("  Permanent failure for '%s via %s'",
			    theadr.adr_local, theadr.adr_host);
			(void) fflush (stdout);
			if (msg_noret (msg->mg_stat)) {
			    printx (", NOT returned on request");
			    ll_log (logptr, LLOGGEN, "%s err noret",
				    msg->mg_mname);
			} else if (rtn_error (msg, supportaddr, &theadr,
					ch_rp.rp_line) == OK) {
			    printx (", returned to supportaddr");
			} else {
			    printx (", couldn't return\n");
			    printx ("attempting to append to DeadLetters");
			    dead_letter (msg -> mg_mname, ch_rp.rp_line);
			}
			printx ("\n");
			(void) fflush (stdout);
		    }
		}
	    }
	    printx ((rp_gbval (final) == RP_BNO) ? "flushed\n" : "sent\n");
	    (void) fflush (stdout);
	    if (rp_gbval (final) == RP_BNO)
		goto logit;
	    break;

	default:          /* note that tok's weren't completed  */
	    /*
	     *  See comment above!
	     *  (typically you get in when there is an error sending text)
	     */
	    mq_setpos (adr_start);
	    while(mq_radr (&theadr) == OK && theadr.adr_pos <= adr_end)
		if (lexequ (theadr.adr_que, chan -> ch_queue) &&
		    theadr.adr_tmp == ADR_AOK && theadr.adr_delv != ADR_DONE) {
		    mq_wrply (&rply, msg, &theadr);     /* Clear AOK */
		    deadhost (&chan -> ch_dead, theadr.adr_host,
				RP_BHST, chan -> ch_ttl);
		}
	    printx ("not sent, queued for retry\n");
	    (void) fflush (stdout);
	    adr_more = TRUE;

    logit:
	    ll_log (logptr, LLOGTMP, "end %s (%s) ** %s (%s) error **",
			msg -> mg_mname, rp_valstr (final),
			chan -> ch_show, chan -> ch_name);
    }
}
/**/

LOCFUN
adr_wadr (theadr) /* tell channel of address   */
    struct adr_struct *theadr;
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "adr_wadr (%s, %s, %s)",
		theadr -> adr_que, theadr -> adr_host, theadr -> adr_local);
#endif

    if( ch_curchan != 0 && lexequ(theadr -> adr_que, ch_curchan->ch_queue) )
	adr_lastdone = ch_curchan;
    else
	adr_lastdone = ch_qu2struct (theadr -> adr_que);
				/* standardized reference to the queue */
    printx ("%s", theadr -> adr_local);
    if (theadr -> adr_host[0] != '\0')
	printx (" via %s", theadr -> adr_host);
    printx (":  ");
    (void) fflush (stdout);

    return (ch_wadr (theadr -> adr_host, theadr -> adr_local));
}
/**/


LOCFUN
adr_rrply (therply, chan, themsg, theadr) /* tell channel of address   */
    Chan *chan;
    RP_Buf *therply;
    Msg *themsg;
    struct adr_struct *theadr;
{                                 /* channel must prshortx addr to user   */
    int     rplen;
    register int    retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "adr_rrply ()");
#endif

    if (rp_isbad (retval = ch_rrply (therply, &rplen)))
    {
	adr_more = TRUE;
	therply -> rp_val = retval;
	return;
    }

    retval = therply -> rp_val;
    if (rp_gval (retval) == RP_MOK || rp_gbval (retval) == RP_BNO)
				   /* note final status of address       */
	adr_log (rp_valstr (retval), chan, themsg, theadr);
#ifdef DEBUG
    else                          /* note temp errors except DHST */
	if (rp_gval (retval) != RP_DHST)
	    ll_log (logptr, LLOGBTR, "%s (%s) %s %s %s",
		    themsg -> mg_mname, rp_valstr (retval),
		    theadr -> adr_que, theadr -> adr_host,
		    theadr -> adr_local);
#endif

    switch (rp_gval (retval))
    {                             /* map reply into final meaning       */
	case RP_RPLY:             /* host sent bad reply                */
	case RP_PROT:             /* host send other bad reply          */
	case RP_BHST:             /* Can't handle host behavior         */
	    adr_more = TRUE;
	    printx ("error at destination, ");
	    deadhost (&chan -> ch_dead, theadr -> adr_host, RP_BHST,
			chan -> ch_ttl);
	    retval = RP_NO;       /* sometimes recoverable...           */
	    break;

	case RP_USER:             /* bad address                        */
	    printx ("unknown address, ");
	    retval = RP_NDEL;
	    break;

	case RP_NDEL:             /* Permanent failure                  */
	    retval = RP_NDEL;     /* have to give up on the message     */
	    break;

	case RP_AOK:              /* only address accepted, so far      */
	    adr_tok = TRUE;
	    printx ("address ok");
	    retval = RP_AOK;
	    break;

	case RP_MOK:              /* all of message sent                  */
	    printx ("sent");
	    retval = RP_DONE;
	    break;

	case RP_LOCK:             /* mailbox locked                       */
	    adr_more = TRUE;
	    printx ("mailbox busy, ");
	    retval = RP_NO;
	    break;

	case RP_AGN:              /* Temporary failure                    */
	    adr_more = TRUE;
	    printx ("not deliverable now, ");
	    retval = RP_NO;
	    break;

	case RP_NS:               /* Temporary NS failure                 */
	    adr_more = TRUE;
	    printx ("NameServer timed out, ");
	    deadhost (&chan -> ch_dead, theadr -> adr_host, RP_NS,
			chan -> ch_ttl);
	    retval = RP_NO;
	    break;

	case RP_TIME:             /* Host timed out                       */
	    adr_more = TRUE;
	    printx ("destination took too long, ");
	    deadhost (&chan -> ch_dead, theadr -> adr_host, RP_TIME,
			chan -> ch_ttl);
	    retval = RP_NO;
	    break;

	case RP_NET:              /* Sockets closed or something          */
	    adr_more = TRUE;
	    printx ("transmission error, ");
	    deadhost (&chan -> ch_dead, theadr -> adr_host, RP_NET,
			chan -> ch_ttl);
	    retval = RP_NO;
	    break;

	case RP_DHST:             /* Couldn't open                        */
	    adr_more = TRUE;
	    printx ("destination not available, ");
	    deadhost (&chan -> ch_dead, theadr -> adr_host, RP_DHST,
			chan -> ch_ttl);
	    retval = RP_NO;
	    break;

	case RP_NOOP:             /* Skip over this one                   */
	    adr_more = TRUE;
	    printx ("skipping address, ");
	    retval = RP_NOOP;
	    break;

	default: 
	    printx ("unknown problem, ");
	    if (*rp_valstr (retval) == '*')
	    {
		ll_log (logptr, LLOGTMP, "%s", rp_valstr (retval));
		adr_more = TRUE;
		retval = RP_NO;   /* illegal return value               */
	    }
	    else
	    if (rp_gbval (retval) == RP_BNO)
		retval = RP_NDEL; /* some sort of permanent error       */
	    else
	    {
		deadhost (&chan -> ch_dead, theadr->adr_host, RP_RPLY,
				chan -> ch_ttl);
		adr_more = TRUE;
		retval = RP_NO;
	    }
    }

    therply -> rp_val = retval;
}
/**/

LOCFUN
	adr_done (themsg)   /* end of address list for this chan  */
    Msg *themsg;
{
    int retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "adr_done (%s)", themsg -> mg_mname);
#endif

    if (ch_curchan != 0 && (adr_lastdone == ch_curchan))
    {                             /* if have a chan, say we are done    */
	retval = ch_waend ();
	adr_t2fin (adr_lastdone, themsg, retval);
    }

    adr_lastdone = (Chan *) 0;
}

LOCFUN
	adr_hdone (themsg, oldhost, chan) /* end of address list for host  */
    Msg *themsg;
    char *oldhost;
    Chan *chan;
{
    int retval = RP_OK;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "adr_hdone (%s)", themsg -> mg_mname);
#endif

    if (ch_curchan != 0 && (adr_lastdone == ch_curchan))
    {                             /* if have a chan, say we are done    */
	retval = ch_whend ();
	switch (retval)
	{
	    case RP_NOOP:         /* channel doesn't distinguish        */
		break;

	    case RP_MOK:
		adr_t2fin (adr_lastdone, themsg, retval);
		break;

	    default:               /* believe bad, but not good         */
		if (rp_isgood (retval)) {
		    printx("illegal HEND reply (%s)\n", rp_valstr(retval));
		    retval = RP_NO;     /* change response to temp. error */
		    ll_log (logptr, LLOGTMP, "illegal HEND reply (%s)", rp_valstr (retval));
		}
		deadhost (&chan -> ch_dead, oldhost, RP_BHST, chan -> ch_ttl);
		adr_t2fin (adr_lastdone, themsg, retval);
	}
    }
    return (retval);
}

deadhost (cachep, host, why, ttl)
Cache   **cachep;
char    *host;
int     why;
time_t  ttl;
{
	char    *index();       /* don't cache delay hosts */

	if(index(host, '&') != NULL)
		return;
	if (!isstr(host))
		return;                 /* Don't timeout local host */
	ca_add (cachep, host, why, msg_ttl ? msg_ttl : ttl);
}

/***************  (err_)  PROCESS ERROR CONDITIONS  ***************** */

/* VARARGS2 */
err_abrt (code, fmt, b, c, d)     /* terminate ourself                  */
int       code;
char    *fmt, *b, *c, *d;
{
    char    newfmt[LINESIZE];

    ovr_end (NOTOK);

    if (rp_isbad (code))
    {
/*      printf() is used, rather than printx(), to force
 *      the caller to see the case for the abort.
 */
	printf ("deliver: ");
	printf (fmt, b, c, d);
	printf ("\n");
	(void) fflush (stdout);

	sprintf (newfmt, "%s%s", "err [ ABEND (%s) ] ", fmt);
	ll_err (logptr, LLOGFAT, newfmt, rp_valstr (code), b, c, d);
#ifdef DEBUG
	sigabort (fmt);
#endif
    }
    ll_close (logptr);           /* in case of cycling, close neatly   */
    exit (RP_NO);
}
