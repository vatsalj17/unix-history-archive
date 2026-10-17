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
/*  CHECKQUE:  Peruse status of mail queue
 *
 *  Oct 81  D. Crocker          Initial coding
 *  Nov 81  D. Crocker          message name must begin "msg."
 *                              show hours since last contact
 *                              show size of aquedir directory
 *  Apr 82  D. Crocker          show last pickup in elapsed hours, also
 *  Jun 82  S. Manion           added flags s,p,z,f, and t.
 *                              rearanged output format.
 *  Jul 82  D. Crocker          convert to mq_ package
 *  Mar 83  Doug Kingston       changed to use format independent directory
 *                              access routines a la 4.2BSD.  (libndir.a)
 *  Sep 84  D. Rockwell		-c only looks at relevant queues
 */

#include "util.h"
#include "mmdf.h"
#include <sys/stat.h>
#include <utmp.h>
#include "ch.h"
#include "msg.h"
#include "adr_queue.h"
#include "phs.h"

#define WARNTIME    (60L * 60L * 24) /* 1 day */

extern time_t time();

time_t  curtime;              /* Current time secconds              */
time_t	toolate;              /* Time to wait before panicing       */

extern char *quedfldir,            /* home directory for mmdf            */
	    *mquedir,            /* subordinate message directory       */
	    *squepref,		 /* subordinate queue prefix */
	    *aquedir;            /* subordinate address directory      */

extern Chan **ch_tbsrch;          /* full list of channels              */

extern int ch_numchans;           /* number of channels which are known */

struct ch_chstat
{
    int ch_nummsgs;
    long ch_bytes;
    time_t  dstrt,
	    dmsg,
	    ddone,
	    pstrt,
	    pmsg,
	    pdone,
	    oldest,          /* oldest message in queue */
	    llog;
    char oldmsg[15];         /* name of oldest msg */
}       ch_stat[NUMCHANS];
long msglen,                 /*  length of the current message  */
     totblks;                /*  number of blocks (1K) of msg data   */

DIR *ovr_dirp;                  /* handle on the queue directory      */

int msg_total;                  /* total in the queue                   */
long msg_dirsiz;                /* total spaces in aquedir              */

char    msg_sender[ADDRSIZE + 1]; /* return address of current message  */

int ssflag,                       /* print site summary lines only      */
    nzflag,                       /* print only non-zero sites          */
    pcflag,                       /* print only problem channels        */
    fflag,                        /* print name of first queued msg     */
    chflag;                       /* print only specified channels      */

int nchan;
Chan *chcodes[NUMCHANS];

char    bufout[BUFSIZ];

/*****  (mn_)  MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN  ***** */

main (argc, argv)
int       argc;
char   *argv[];
{
    extern char *dupfpath ();

    mmdf_init (argv[0]);
    setbuf (stdout, bufout);
    if (ch_numchans > NUMCHANS)
    {
	printf("%s: Struct ch_stat[NUMCHANS] not large enough - recompile\n",
		    argv[0]);
	exit(-1);
    }

    siginit ();                   /* standard interrupt initialization  */
				  /* distinguish different delivers     */
    flaginit(argc, argv);         /*  initialize the flags  */

    mn_dirinit ();                /* get right working directory        */

    ovr_queue ();                 /* do the entire mail queue           */

    ch_callstat ();
    ch_report ();

    exit (RP_OK);
}

flaginit(argc, argv)      /*  initialize flags  */
int argc;
char **argv;
{
    register int argind, charind;

    chflag = fflag = ssflag = pcflag = nzflag = 0;
    nchan = 0;
    /*NOSTRICT*/
    toolate = WARNTIME;
    for (argind = 1;  argind < argc;  argind++)
    {
	if (argv[argind][0] != '-')
	{
	    /*  if the chflag is not set, then this arg is meaningless  */
	    if (chflag == 0)
		continue;

	    /*  Is there room for the char?  */
	    if(nchan == NUMCHANS)
	    {
		printf("Can only specify %d channels.\n", NUMCHANS);
		continue;
	    }

	    /*  Convert from the ch_spec form to structure pointer */

	    if ((chcodes[nchan++] = ch_nm2struct (argv[argind])) ==
			(Chan *) NOTOK)
	    {
		printf("Unknown channel: %s\n", argv[argind]);
		(void) fflush (stdout);
	    }
	    continue;

	}

	for(charind = 1;  argv[argind][charind] != '\0';  charind++)  {
	    switch(argv[argind][charind])  {
		case 'c':       /*  channel flag  */
		    chflag++;
		    break;

		case 'p':       /*  problem channel flag  */
		    pcflag++;
		    break;

		case 's':       /*  site summary flag  */
		    ssflag++;
		    break;

		case 'z':       /*  non-zero flag  */
		    nzflag++;
		    break;

		case 'f':       /*  print name of first queued msg  */
		    fflag++;
		    break;

		case 't':       /*  set time on problem channels  */
		    /*  toolate is seconds to start with  */
		    toolate = atoi(&argv[argind][++charind]);
		    if(toolate == 0L)  {
			printf("Need a number after the 't'.\n");
			exit(-1);
		    }
		    /*  check for a trailing M to indicate minutes  */
		    while( (argv[argind][charind] >= '0') &&
			   (argv[argind][charind] <= '9')    )
			charind++;
		    if( (argv[argind][charind] == 'm') ||
			(argv[argind][charind] == 'M')   )
			/*NOSTRICT*/
			toolate *= 60L;
		    else  {
			/*  this char is not part of this flag  */
			charind--;
			/*NOSTRICT*/
			toolate *=  60L * 60L;
		    }
		    /*  At this point charind must index the last position
		     *  actually used by this flag;  it does.
		     */
		    break;

		default:        /*  abzorb unrecognized flags  */
		    printf(
"Usage: %s [-z] [-s] [-p] [-f] [-t<age>[m]] [-c <channel name(s)>]\n",
						argv[0]);
		    printf("\t-c <channel names>\n");
		    printf("\t\tspecifies the channel names to look at\n");
		    printf("\t-f  ->  print name of oldest queued msg\n");
		    printf("\t-p  ->  print only the problem channels\n");
		    printf("\t-s  ->  summary lines only\n");
		    printf("\t-t<age>[m]\n");
		    printf(
"\t\tspecifies the age for problem channels in hours or minutes\n");
		    printf("\t-z  ->  non zero channels only\n");
		    exit(-1);
	    }
	}
    }
    (void) fflush(stdout);
    return(0);
}


LOCFUN
	mn_dirinit ()             /* get to the working directory       */
{
    if (chdir (quedfldir) < OK)
    {
	printf ("can't chdir to %s\n", quedfldir);
	exit (-1);
    }
}

/************  (ovr_)  OVERALL SEQUENCING MANAGEMENT  *************** */

LOCFUN
	ovr_ismsg (entry)         /* a processable message?             */
    register struct direct *entry;
{
    return ((entry -> d_namlen < MSGNSIZE
	  && equal (entry -> d_name, "msg.", 4)) ? TRUE : FALSE );
}

ovr_queue ()                      /* Process entire message queue       */
{
    char qname[128];
    register int index;

    time (&curtime);              /* Get current time (in hours)        */

    if (chflag != 0)
    {
	for (index = 0; index < nchan; index++)
	{
	    (void) strcpy(qname, squepref);
	    strcat(qname, chcodes[index]->ch_queue);
	    ovr_aqueue(qname);
	}
    } else
	ovr_aqueue(aquedir);
}

ovr_aqueue (ch)                      /* Process a single message queue   */
register char *ch;                   /* queue directory name             */
{
    struct stat statbuf;
    register struct direct *dp;

    stat (ch, &statbuf);
    msg_dirsiz += st_gsize(&statbuf);

    if ((ovr_dirp = opendir (ch)) == NULL)
    {
	printf ("can't open queue %s\n", ch);
	return;
    }

    while ((dp = readdir (ovr_dirp)) != NULL)
    {                         /* straight linear processing         */
	if (ovr_ismsg (dp))
	    msg_proc (dp -> d_name);
    }
    closedir(ovr_dirp);
}
/****************  (msg_)  HANDLE A SINGLE MESSAGE  ***************** */

extern int    errno;                /* system error number                */

/*  msg_cite() is defined in adr_queue.h                                */

msg_proc (thename)                /* get, process & release a msg       */
    char thename[];
{
    Msg curmsg;
    struct stat statbuf;
    char msgpath[28];

    (void) sprintf (msgpath, "%s%s", mquedir, thename);
    if (stat (msgpath, &statbuf) < 0) {
	printf ("\n*** %s: Couldn't stat;  error %d.\n", thename, errno);
	(void) fflush (stdout);
	msglen = 0;
    } else {
	msglen = st_gsize (&statbuf);
	if (msglen == 0)
	{
	    printf ("\n*** %s: msg length of zero.\n", thename);
	    (void) fflush (stdout);
	} else {
	    totblks += ((msglen+1023L)/1024L);
	    msg_total++;
	}
    }

    /*
     *  Check for real anomalies!
     */
    (void) strcpy (curmsg.mg_mname, thename);

    if (mq_rinit ((Chan *) 0, &curmsg, msg_sender) == OK)
    {
	if (adr_each (&curmsg) == 0)
	{
	    printf ("\n*** %s: msg exists (%d chars), but goes nowhere.\n",
			thename, msglen);
	    (void) fflush (stdout);
	}
	mq_rkill (OK);
    }
}
/************  (adr_)  INDIVIDUAL ADDR DELIVERY ATTEMPT  ************ */

adr_each (themsg)                 /* do each address                    */
Msg *themsg;
{
    struct adr_struct newadr;
    Chan *curch,
	 *newch;
    register int naddrs;


    for (naddrs = 0, curch = (Chan *) 0, mq_setpos (0L);;)
	switch (mq_radr (&newadr))
	{
	    default:              /* should not get this                */
		printf ("\n*** %s: bad return from mq_radr\n", themsg -> mg_mname);
		exit (NOTOK);

	    case NOTOK:           /* rest of list must be skipped       */
	    case DONE:            /* end of list                        */
		return (naddrs);

	    case OK:              /* normal addr acquisition            */
		naddrs++;
		if ((newch = ch_qu2struct (newadr.adr_que)) == (Chan *) NOTOK)
		{
		    printf ("\n*** Unknown channel queue ('%s') in message '%s'\n",
				newadr.adr_que, themsg -> mg_mname);
		    (void) fflush (stdout);
		}
		else
		{
		    if ((newadr.adr_delv != ADR_DONE) && (curch != newch))
		    {
			curch = newch;
			ch_newmsg (curch, themsg);
		    }
		}
	}
}
/**/

ch_newmsg (thech, themsg)
    Chan *thech;
    Msg  *themsg;
{
    register int chind;

    for (chind = 0; chind < ch_numchans; chind++)
	if (ch_tbsrch[chind] == thech)
	{
	    ch_stat[chind].ch_nummsgs++;
	    ch_stat[chind].ch_bytes += msglen;
	    if (themsg -> mg_time < ch_stat[chind].oldest ||
		    ch_stat[chind].oldest == 0)
	    {
		ch_stat[chind].oldest = themsg -> mg_time;
		(void) strcpy (ch_stat[chind].oldmsg, themsg -> mg_mname);
	    }
	    return;
	}

    printf ("\n*** Could not locate channel ('%s') in table\n",
		thech -> ch_name);
    (void) fflush (stdout);
}
/**/

ch_callstat ()
{
    extern struct utmp *getllnam ();
    struct utmp *llogptr;
    register int chind;

    for (chind = 0; chind < ch_numchans; chind++)
    {
	if (ch_tbsrch[chind] -> ch_login != NOLOGIN)
	{
	    setllog ();         /* setup for checking last logins */
	    if ((llogptr = getllnam (ch_tbsrch[chind] -> ch_login)) != NULL)
		/*NOSTRICT*/
		ch_stat[chind].llog = llogptr -> ut_time;
	}

	ch_stat[chind].dstrt = phs_get (ch_tbsrch[chind], PHS_WRSTRT);
	ch_stat[chind].dmsg  = phs_get (ch_tbsrch[chind], PHS_WRMSG);
	ch_stat[chind].ddone = phs_get (ch_tbsrch[chind], PHS_WREND);
	ch_stat[chind].pstrt = phs_get (ch_tbsrch[chind], PHS_RESTRT);
	ch_stat[chind].pmsg  = phs_get (ch_tbsrch[chind], PHS_REMSG);
	ch_stat[chind].pdone = phs_get (ch_tbsrch[chind], PHS_REEND);
    }

    endllog ();
}
/**/

ch_report ()
    {
    register int chind;
    register int index;
    register Chan *chptr;
    char *tptr;
    char *ctime();
    time_t lasttime, logtime, delstart, delmsg, deldone, 
	   pickstrt, pickmsg, pickdone, age;
    int prtpos, prtflag;

    tptr = ctime (&curtime);
    tptr[16] = '\0';
    printf ("\n%s:  %d queued msgs / %ld byte queue directory\n",
		tptr, msg_total, msg_dirsiz);
    printf("\t\t   %ld Kbytes in msg dir\n\n", totblks);

    prtflag = 0;
    for (chind = 0; chind < ch_numchans; chind++)
    {
	/*  If the non-zero flag is set, then make sure the number
	 *  of messages on this channel is non-zero.
	 */
	if (nzflag != 0)
	    if (ch_stat[chind].ch_nummsgs == 0)
		continue;

	/*  An often used variable  */
	chptr = ch_tbsrch[chind];

	/*  If specific channels were requested, make sure this is
	 *  one of them.
	 */
	if (chflag != 0)
	{
	    for (index = 0;  index < nchan;  index++)
		if (chptr == chcodes[index])
		    break;
	    if (index >= nchan)
		continue;
	}

	/*  find time information  */
	lasttime = (time_t) 0;
	if (ch_tbsrch[chind] -> ch_login != NOLOGIN)
	{
	    logtime = ch_stat[chind].llog;
	    if( (lasttime == (time_t) 0) ||
		(logtime > lasttime    )   )
	    {
		lasttime = logtime;
		prtpos = 0;
	    }
	}

	delstart = ch_stat[chind].dstrt;
	if ( (lasttime == (time_t) 0) ||
	     (delstart > lasttime   )   )
	{
	    lasttime = delstart;
	    prtpos = 1;
	}

	delmsg = ch_stat[chind].dmsg;
	if (delmsg > lasttime)
	{
	    lasttime = delmsg;
	    prtpos = 2;
	}

	deldone = ch_stat[chind].ddone;
	if (deldone > lasttime)
	{
	    lasttime = deldone;
	    prtpos = 3;
	}

	pickstrt = ch_stat[chind].pstrt;
	if (pickstrt > lasttime)
	{
	    lasttime = pickstrt;
	    prtpos = 4;
	}

	pickmsg = ch_stat[chind].pmsg;
	if (pickmsg > lasttime)
	{
	    lasttime = pickmsg;
	    prtpos = 5;
	}

	pickdone = ch_stat[chind].pdone;
	if (pickdone > lasttime)
	{
	    lasttime = pickdone;
	    prtpos = 6;
	}

	/*  how long ago was the contact?  */
	if(lasttime != (time_t) 0)
	    age = curtime - lasttime;
	else
	    age = 0;

	/*  if problem channel flag is set, check age  */
	if (pcflag != 0)
	{
	    if (age < toolate)
		continue;
	    else
		prtflag++;
	}

	prtinfo(chind, logtime, delstart, delmsg, deldone, pickstrt, 
                                pickmsg, pickdone, age, prtpos);
    }
    if ((pcflag != 0 ) &&
	(prtflag == 0)   )
	printf("No problems with any channels.\n");
}


prtinfo(chind, logtime, delstart, delmsg, deldone, 
	pickstrt, pickmsg, pickdone, age, prtpos)
  int chind, prtpos;
  time_t logtime, delstart, delmsg, deldone, pickstrt, pickmsg, pickdone, age;
{
    register Chan *chptr;
    char *tptr, *ctime();
    time_t ageh;
    int temp;

    chptr = ch_tbsrch[chind];


    /*  print the site summary line  */
    if(ssflag == 0)
	putchar('\n');
    else
    {
	/*  print a star to indicate overdue channels  */
	if ( (age != (time_t) 0) &&
	     (age > toolate    )   )
	    printf("* ");
	else
	    printf("  ");
    }
    /*NOSTRICT*/
    temp = (ch_stat[chind].ch_bytes + 500) / 1000;
    if( (ch_stat[chind].ch_bytes != 0) &&
	(temp == 0                   )   )
	temp++;
    /*NOSTRICT*/
    ageh = age / (60L * 60L);
    printf ("%3d msg%s %4d Kb ",
		    ch_stat[chind].ch_nummsgs,
		    ((ch_stat[chind].ch_nummsgs == 1) ? " " : "s"),
		    temp);
    if (ssflag != 0)
    {
	if ((age != (time_t) 0))
	    printf ("%5ld hour%s ", ageh, ((ageh == 1)  ?  " " : "s"));
	else
	if (chptr -> ch_login == NOLOGIN)
	    printf ("   inactive ");
	else
	    printf ("   no login ");
    }

	printf ("(%-8s) %-11s :  %s\n",
		    chptr -> ch_queue, chptr -> ch_name, chptr -> ch_show);

    /*  If only a summary was requested, then go on  */
    if (ssflag != 0)
	{
	/*  print oldest file if specifically requested  */
	if(fflag != 0)
	    printf("    oldest msg:  %s\n", ch_stat[chind].oldmsg);
	return;
    }


    if (chptr -> ch_login != NOLOGIN)
    {
	printf ("\t\t  %-8s login        :  ", chptr -> ch_login);

	if (logtime == (time_t) 0)
	    printf ("no login date\n");
	else
	{
	    tptr = ctime (&logtime);
	    tptr[16] = '\0';
	    printf ("%s", tptr);
	    if(prtpos == 0)
	    {
		printf(" / %ld hour%s",
			ageh,
			(ageh == 1)  ?  "" : "s");
		overdue(age);
	    }
	    putchar('\n');
	}
    }

    if (delstart == (time_t) 0)
        printf ("\t\t  No deliver start\n");
    else
    {
        tptr = ctime (&delstart);
        tptr[16] = '\0';
        printf ("\t\t  deliver start         :  %s", tptr);
        if(prtpos == 1)
        {
            printf(" / %ld hour%s",
    	        ageh,
    	        (ageh == 1)  ?  "" : "s");
            if (ch_stat[chind].ch_nummsgs > 0)
    	    overdue(age);
        }
        putchar('\n');
    }
    
    if (delmsg == (time_t) 0)
        printf ("\t\t  No message delivered\n");
    else
    {
        tptr = ctime (&delmsg);
        tptr[16] = '\0';
        printf ("\t\t  deliver message       :  %s", tptr);
        if(prtpos == 2)
        {
            printf(" / %ld hour%s",
    	        ageh,
    	        (ageh == 1)  ?  "" : "s");
            if (ch_stat[chind].ch_nummsgs > 0)
    	    overdue(age);
        }
        putchar('\n');
    }
    
    if (deldone == (time_t) 0)
        printf ("\t\t  No deliver end\n");
    else
    {
        tptr = ctime (&deldone);
        tptr[16] = '\0';
        printf ("\t\t  deliver end           :  %s", tptr);
        if(prtpos == 3)
	    {
            printf(" / %ld hour%s",
    	        ageh,
    	        (ageh == 1)  ?  "" : "s");
    	if (ch_stat[chind].ch_nummsgs > 0)
		    overdue(age);
	    }
        incomplete(delstart, deldone);
        putchar('\n');
            
    }

    if (pickstrt == (time_t) 0)
        printf ("\t\t  No pickup start\n");
    else
    {
        tptr = ctime (&pickstrt);
        tptr[16] = '\0';
        printf ("\t\t  pickup start          :  %s", tptr);
        if(prtpos == 4)
        {
            printf(" / %ld hour%s",
    	            ageh,
    	            (ageh == 1)  ?  "" : "s");
            overdue(age);
        }
        putchar('\n');
    }

    if (pickstrt == (time_t) 0)
        printf ("\t\t  No message pickup\n");
    else
    {
        tptr = ctime (&pickstrt);
        tptr[16] = '\0';
        printf ("\t\t  pickup message        :  %s", tptr);
        if(prtpos == 5)
        {
            printf(" / %ld hour%s",
    	            ageh,
    	            (ageh == 1)  ?  "" : "s");
            overdue(age);
        }
        putchar('\n');
    }
    
    if (pickdone == (time_t) 0)
        printf ("\t\t  No pickup end\n");
    else
    {
        tptr = ctime (&pickdone);
        tptr[16] = '\0';
        printf ("\t\t  pickup done           :  %s", tptr);
        if(prtpos == 6)
            printf (" / %ld hour%s", (curtime - pickdone) / 3600L,
    	            (((curtime - pickdone) / 3600L) == 1) ? "" : "s");
        incomplete(pickstrt, pickdone);
        if(overdue(age) == 0)
            /*  Failure to complete a pickup within twice the
             *  normal 'problem time' is a special problem;
             *  use the normal compare routine, but halve the
             *  'age'.
             */
            overdue((curtime - pickdone)/2);
    putchar('\n');
    }        
    /*  print oldest file in queue if requested  */
    if ( (fflag != 0) && (ch_stat[chind].oldest != (time_t) 0) )
	printf ("\t\t  oldest msg            :  %s\n", ch_stat[chind].oldmsg);


    if (ch_stat[chind].ch_nummsgs != 0 &&
	    ch_stat[chind].oldest < (curtime - toolate))
    {
	    tptr = ctime (&(ch_stat[chind].oldest));
	    tptr[16] = '\0';

	    printf ("  *** WAITING **  First message         :  %s\n", tptr);
    }
}



overdue(age)
  time_t age;
    {

    if (age == (time_t) 0 || age <= toolate)
	return(0);

    printf ("\n  *** OVERDUE **");
    return(1);
}

incomplete(strt, done)
  time_t strt, done;
    {

    if ( strt != (time_t) 0 && strt > done)
	printf ("\n  *** INCOMPLETE **");

    return;
}
