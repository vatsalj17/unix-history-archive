/* $Header: cleanque.c,v 1.6 85/04/05 13:42:13 long Exp $ */
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
#include "util.h"
#include "mmdf.h"
#include "msg.h"
#include "adr_queue.h"
#include "ch.h"
#include <sys/stat.h>

/*  QUECLEAN:  Clean up the mail queue directories                      */

/*  Jul 80 Dave Crocker     noaddr, correct the 2d multcpy
 *  Aug 80 Dave Crocker     fix orphaned
 *  Aug 80 Dave Crocker     fix name-handling for mclean
 *  Aug 81 Dave Crocker     test for chdir failures
 *  Mar 83 Doug Kingston    modified to use format independent directory
 *			    access routines a la 4.2BSD.  (libndir.a)
 */

/*#define RUNALON */

#define MINAGE  (long) (60 * 60 * 4)
				  /* at least four hours old             */

extern int warntime;    /* hours to wait before notification  */
extern int failtime;    /* hours to wait before returning msg */
extern int errno;
extern LLog msglog;
LLog *logptr = &msglog;

extern char *quedfldir,
	    *aquedir,
	    *squepref,
	    *tquedir,
	    *mquedir,
	    *supportaddr;

DIR *quep;                     /* for reading directory entries      */

struct stat    testnode;

time_t  curtime;              /* Current time in hours              */
int	elaphour;	      /* Message waiting time in hours      */

char    aname[LINESIZE],
	mname[LINESIZE];

int     effecid,                  /* system number of pgm/file's owner  */
	callerid;                 /* who invoked me?                    */

/**/

main (argc, argv)		  /* remove old queue files             */
int       argc;
char   *argv[];
{
    extern char *dupfpath ();
    extern time_t time ();

    mmdf_init (argv[0]);
    nice (-5);                 /* try to run faster, if root           */

    getwho (&callerid, &effecid); /* who am I and who is running me?    */
    mn_mmdf();			/* set up effective and group id's properly */

    if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'w')
	domsg = TRUE;

    time (&curtime);

    if (chdir (quedfldir) < OK || chdir (tquedir) < OK)
    {
	printx ("couldn't chdir to tquedir\n");
	ll_err (logptr, LLOGFAT, "couldn't chdir tquedir");
	exit (-1);
    }

    tclean ();			  /* clean out temporary files          */

    if (chdir (quedfldir) < OK)
    {
	printx ("couldn't chdir to quedfldir\n");
	ll_log (logptr, LLOGFAT, "couldn't chdir to quedfldir");
	exit (-1);
    }

    mclean ();			  /* get rid of orphaned text files     */

    if (chdir (quedfldir) < OK)
    {
	printx ("couldn't chdir to quedfldir\n");
	ll_log (logptr, LLOGFAT, "couldn't chdir to quedfldir");
	exit (-1);
    }

    aclean ();                    /* get rid of old queued messages     */
}
/* ***************  GET RID OF OLD TEMPORARY FILES  ****************  */

tclean ()			  /* clean out temporary files          */
{
    register struct direct *dp;
	
    if ((quep = opendir (".")) == NULL)
    {
	printx ("couldn't open tquedir\n");
	ll_log (logptr, LLOGFAT, "couldn't open tquedir");
	exit (-1);
    }

    while ((dp = readdir (quep)) != NULL)
	if (ismsg (dp) && minage (dp->d_name))
	{
	    printx ("removing temp file: %s\n", dp->d_name);
	    ll_log (logptr, LLOGGEN, "removing temp file: %s", dp->d_name);

#ifndef RUNALON
	    (void)  unlink (dp->d_name);
#endif
	}

    closedir (quep);
}
/* ***************  GET RID OF ORPHANED TEXT FILES  ****************  */

mclean ()			  /* get rid of orphaned text files     */
{
    register struct direct *dp;
	
    if ((quep = opendir (mquedir)) == NULL)
    {
	printx ("couldn't open quedfldir\n");
	ll_log (logptr, LLOGFAT, "couldn't open quedfldir");
	exit (-1);
    }

    while ((dp = readdir (quep)) != NULL)
	if (ismsg (dp))
	    mproc (dp->d_name);

    closedir (quep);
    quep = (DIR *) EOF;
}

mproc (filename)                /* check for & remove orphaned msgs   */
char    filename[];
{
    (void) sprintf (mname, "%s%s", mquedir, filename);
    (void) sprintf (aname, "%s%s", aquedir, filename);

    if (minage (mname) && stat (aname, &testnode) == -1)
    {                             /* old & no aquedir association       */
	printx ("removing old orphaned text file: %s\n", filename);
	ll_log (logptr, LLOGGEN, "removing text file: %s", filename);

#ifndef RUNALON
	(void)  unlink (mname);
#endif
    }
}

/**/

minage (filename)                /* is file more than x hours old?      */
char    filename[];
{
    if (stat (filename, &testnode) == -1)
	return (FALSE);           /* doesn't really exist               */

#ifdef RUNALON
    printx ("O? %s\n", filename);
#endif

    return (((curtime - testnode.st_mtime) > MINAGE) ? TRUE : FALSE);
				  /* two hours since modified?          */
}
/* **************  GET RID OF MESSAGES IN QUEUE TOO LONG **********  */

aclean ()
{
    Msg  themsg;
    char retadr[LINESIZE];
    register struct direct *dp;

    if ((quep = opendir (aquedir)) == NULL)
	    err_abrt (RP_FOPN, "can't open address queue");

    themsg.mg_null = '\0';
    while ((dp = readdir (quep)) != NULL)
	if (ismsg (dp))
	{
				  /* get queue entry name (msg name)   */
	    (void) strcpy (themsg.mg_mname, dp->d_name);
	    if (mq_rinit ((Chan *) 0, &themsg, retadr) != OK)
		continue;
	    elaphour = (int) ((curtime - themsg.mg_time) / (time_t) 3600);
#ifdef DEBUG
	    ll_log (logptr, LLOGFTR,
			    "%s (%d hrs)", themsg.mg_mname, elaphour);
#endif
	    if (elaphour > warntime) /* message is old                   */
	    {
		if (elaphour > failtime)
		{
                    /* old enough to return?            */
		    doreturn (&themsg, retadr);
		    deque (&themsg);
		}
		else if (!msg_warned(themsg.mg_stat))
		    dowarn (&themsg, retadr);
	    }
	    mq_rkill (OK);
	}

    closedir (quep);
    quep = (DIR *) EOF;
}

/**/

LOCFUN
	ismsg (theentry)         /* a processable message?             */
    register struct direct *theentry;
{
#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "ovr_ismsg (name='%s')",
			theentry -> d_name);
#endif

/*  valid message:  entry allocated & name begins with "msg." */

    return ((theentry -> d_namlen < MSGNSIZE
		&& equal (theentry -> d_name, "msg.", 4)) ? TRUE : FALSE);
}

deque (themsg)
    Msg *themsg;
{
    struct adr_struct theadr;
    char curque[ADDRSIZE];

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "deque");
#endif

    curque[0] = '\0';   /* no queue in effect */

    mq_setpos (0L);	/* start at the beginning of the queue */
    while (mq_radr (&theadr) == OK)
    {
#ifdef DEBUG
	ll_log (logptr, LLOGFTR,
		    "(%c)%s:%s", theadr.adr_delv, theadr.adr_que, curque);
#endif
    	/*
    	 *  We have to dequeue from every queue since the return
    	 *  function already has marked all the addresses as "DONE".
    	 *  The msg_dequeue function was modified to ignore ENOENT
    	 *  so if the file is already gone, it is not an error.
    	 */
	if (!lexequ (curque, theadr.adr_que))
	{
	    (void) strcpy (curque, theadr.adr_que);
	    msg_dequeue (theadr.adr_que, themsg);
	}
    }
    msg_dequeue ((char *) 0, themsg);
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
#ifdef RUNALON
    return;
#endif

    if (theque == (char *) 0)
        (void) sprintf (thename, "%s%s", aquedir, themsg -> mg_mname);
    else
	(void) sprintf (thename, "%s%s/%s",
			squepref, theque, themsg -> mg_mname);

    if (unlink (thename) < OK && errno != ENOENT) {
	 /* this is real queue handle  */
	 ll_err (logptr, LLOGTMP, "Problem unlinking '%s' address: %s",
		    themsg -> mg_mname, thename);
    }

    if (theque == (char *) 0)
    {				  /* get rid of ALL the message */
	(void) sprintf (thename, "%s%s", mquedir, themsg -> mg_mname);
	if (unlink (thename) < OK) /* the text is just "baggage"         */
	    ll_err (logptr, LLOGTMP, "Problem unlinking %s text: '%s'",
	        themsg -> mg_mname, thename);
    }
}
/**/

dowarn (themsg, retadr)
	Msg *themsg;
	char retadr[];
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "dowarn (%s, %s)", themsg -> mg_mname, retadr);
#endif

    printx ("%s:  delivery overdue; ", themsg -> mg_mname);
    (void) fflush (stdout);

    if (msg_nowarn (themsg -> mg_stat)) {
	printx ("warning not wanted\n");
    }
    else if (rtn_warn (themsg, retadr) == OK)
    {                     /* flag as already warned               */
	printx ("warning sent\n");
	ll_log (logptr, LLOGGEN, "warn *** Time warning (%s, %s)",
		    themsg -> mg_mname, retadr);
    }
    else
    {
	printx ("couldn't send warning\n");
	ll_err (logptr, LLOGTMP, "warn *** Couldn't time warn (%s, %s)",
		    themsg -> mg_mname, retadr);
    }
    (void) fflush (stdout);

    mq_rwarn ();
}

doreturn (themsg, retadr)
    Msg *themsg;
    char retadr[];
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "doreturn (%s, %s)", themsg -> mg_mname, retadr);
#endif

    printx ("%s:  not delivered in time; waiting %d hrs;", themsg -> mg_mname, elaphour);
    (void) fflush (stdout);
    if (msg_noret (themsg -> mg_stat)) {
	printx (" error return not wanted\n");
    }    
    else if (rtn_time (themsg, retadr) == OK)
    {                         /* dequeue if couldn't notify   */
	printx (" returned\n");
	(void) fflush (stdout);
	ll_log (logptr, LLOGTMP, "ret *** Timeout return (%s, %s)",
		    themsg -> mg_mname, retadr);
    }
    else
    {                         /* dequeue if couldn't notify   */
	char orphanage[ADDRSIZE];

	(void) sprintf (orphanage, "Orphanage <%s>", supportaddr);
	printx (" couldn't return,\ntrying orphanage...");
	(void) fflush (stdout);
	if (rtn_time (themsg, orphanage) == OK)
	{
	    printx (" returned to orphanage.\n");
	    (void) fflush (stdout);
	}
	else
	{
	    ll_err (logptr, LLOGTMP, "ret *** Timeout couldn't return (%s,%s)",
		    themsg -> mg_mname, retadr);
	    dead_letter (themsg->mg_mname, "Timeout on delivery");
	}
    }
}


/**/

LOCFUN
        mn_mmdf ()		  /* setuid to mmdf: bypass being root  */
{				  /* get sys id for mmdf; setuid to it  */
    extern char *pathsubmit;     /* submit command file name           */
    extern char *cmddfldir;      /* directory w/mmdf commands          */
    char    temppath[LINESIZE];
    struct stat    statbuf;

#ifdef DEBUG
    ll_log (logptr, LLOGPTR, "mn_mmdf(); effec==%d",
		effecid );
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
 *  All sets gid to mmdf's gid  --  <DPK@BRL>
 */

    if (effecid == 0)
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
