# include  "util.h"
# include <signal.h>
# include  "d_syscodes.h"
# include  "d_clogcodes.h"
# include  "d_proto.h"
# include  "d_returns.h"
# include  "d_structs.h"
#ifdef SYS5
# include <termio.h>
#else
# include  <sgtty.h>
#endif SYS5
# include  <sys/stat.h>
#ifdef V4_2BSD
# include  <sys/file.h>
#endif V4_2BSD

/*  Jun 81  Dave Crocker    fixed some text, referring to errno
 *  Aug 81  Dave Crocker    added sleep before kill & alarm around
 *                          parent waiting for acu
 *  Sep 81  Dean Krafft     put signal (ALRM,d_catch) before ACUWAIT
 *  Apr 82  Dave Crocker    modify dial timings & lock-file handling
 *  Jun 82  Dave Crocker    do_dial use of kill() moved around
 *                          d_drop() timeouts around modem line fclose()
 *  Jan 84  Dennis Rockwell converted to do sane things with 4.2 select(2)
 *                          by setting non-blocking open option
 *  Feb 84  Dan Long        moved d_cstart to start of phone conversation
 *  Feb 84  Dan Long        added support for line-type selection
 *  Mar 84  Dennis Rockwell converted from select to s_io library
 */

extern int errno;
extern int d_errno;
extern struct directlines  *d_lines;
extern struct directlines  *d_ptline;
extern struct dialports *d_prtpt;
extern struct dialports *d_prts;
extern char d_uname[];
extern int  d_didial;
extern int  d_baudrate;
extern  FILE * d_prtfp;
extern struct speedtab d_spdtab[];
extern char **d_typelist();
extern char *strdup();
extern char *multcat();
extern int  d_lckfd;


/*
 *     D_CONNECT
 *    
 *     this routine is called to make a connection to the remote system.  it
 *     is capable of using direct lines or dial-up connections, as specified
 *     in the connection path table ('numtab').
 *    
 *     numtab -- array of structure whch contain speed indicies and associated
 *               phone numbers or direct connect line names.
 *    
 *     nnumbs -- number of entries in numtab
 *    
 *     ntries -- number of time to retry a connection.
 *    
 *     interval -- number of seconds between connection retries.
 */

d_connect (numtab, nnumb, ntries, interval)
struct telspeed numtab[];
int     nnumb,
        ntries,
        interval;
{
    register int    num,
                    try,
                    result;

#ifdef D_DBGLOG
    d_dbglog ("d_connect", "nnumb %d, ntries %d, interval %d",
	    nnumb, ntries, interval);
#endif

/*  make sure we were given some numbers to dial  */

    if (nnumb <= 0)
    {
#ifdef D_LOG
	d_log ("d_connect", "no connection paths given!");
#endif D_LOG
	d_errno = D_NONUMBS;
	return (D_FATAL);
    }

/*  for each attempt, try each of the paths given  */

    for (try = 1; try <= ntries; try++)
    {
	for (num = 0; num < nnumb; num++)
	{
#ifdef D_DBGLOG
	    d_dbglog ("d_connect", "%d: (speed %d) num '%s'",
			num, numtab[num].t_speed, numtab[num].t_number);
#endif D_DBGLOG
	    if (numtab[num].t_speed == 0)
		result = d_direct (numtab[num].t_number);

	    else
		result = d_dial (numtab[num].t_number, numtab[num].t_speed);

	    if (result != D_NONFATAL)
		return (result);
	}

/*  if we're going to try again, and we're supposed to wait awhile, sleep  */

	if ((try < ntries) && (interval > 0))
	{
#ifdef D_DBGLOG
	    d_dbglog ("d_connect", "sleeping for %d seconds", interval);
#endif D_DBGLOG
	    sleep ((unsigned) interval);
	}
    }

#ifdef D_LOG
    d_log ("d_connect", "couldn't make connection.");
#endif D_LOG
    return (D_FATAL);
}

/*
 *     D_DIRECT
 *
 *     routine called to try to make a direct connection using the line
 *     given as the argument.
 *
 *     linename -- name of direct connect line
 */

d_direct (linename)
register char  *linename;
{
    int     fd;
    register int    result;
#ifdef D_LOG
    register int    i;
#endif D_LOG
#ifdef SYS5
    struct termio hupbuf;
#endif SYS5

/*  see if a direct line with the given name is available  */

    if ((result = d_avline (linename)) < 0)
	return (result);

    d_baudrate = d_ptline -> l_speed;

#ifdef D_LOG
    for (i=0; d_spdtab[i].s_speed != 0; i++) {
        if (d_spdtab[i].s_index == d_baudrate) {
	   d_log ("d_direct", "Trying direct connection on %s at %s.",
		d_ptline -> l_tty, d_spdtab[i].s_speed);
	   break;
	}
    }
#endif D_LOG

/*  found a line.  set an alarm in case the open hangs  */

    if (setjmp (timerest)) {
#ifdef D_LOG
	    d_log ("d_direct", "direct line open timeout for %s",
			d_ptline -> l_tty);
#endif D_LOG
	d_errno = D_PORTOPEN;
	return (D_FATAL);
    }
    s_alarm (CONNWAIT);
#ifndef V4_2BSD
    if ((fd = open (d_ptline -> l_tty, 2)) >= 0) {
#else V4_2BSD
    if ((fd = open (d_ptline -> l_tty, O_RDWR | O_NDELAY)) >= 0) {
#endif V4_2BSD

#ifdef SYS5
	ioctl (fd, TCGETA, &hupbuf);
	hupbuf.c_cflag |= HUPCL;
	if (ioctl (fd, TCSETA, &hupbuf) < OK) {
#else
	if (ioctl (fd, TIOCHPCL, 0) < OK) {
#endif SYS5
#ifdef D_LOG
	    d_log ("d_direct", "problem setting close-on-hangup; errno = %d", errno);
#endif D_LOG
	}
#ifdef	TIOCEXCL
	if (ioctl (fd, TIOCEXCL, 0) < OK) { /* gain exclusive access [mtr] */
#ifdef D_LOG
	    d_log ("d_direct",
			"problem gaining exclusive access; errno = %d", errno);
#endif D_LOG
	}
#endif	TIOCEXCL
    }
    s_alarm (0);

    if (fd < 0)
    {
#ifndef V4_2BSD
	if (errno == EINTR) {
#ifdef D_LOG
	    d_log ("d_direct", "direct line open timeout for %s",
			d_ptline -> l_tty);
#endif D_LOG
	} else
#endif V4_2BSD
#ifdef D_LOG
	    d_log ("d_direct", "direct line open errno %d for %s",
			errno, d_ptline -> l_tty);
#endif D_LOG

	d_errno = D_PORTOPEN;
	return (D_FATAL);
    }

/* stdio for input and straight write() for output */
    d_prtfp = fdopen (fd, "r");
    if (d_ttsave (d_prtfp, d_ptline -> l_tty) < 0 ||
	    d_ttscript (d_baudrate) < 0)
    {
#ifdef D_LOG
	d_log ("d_direct",
		    "can't set direct-line parameters; errno = %d", errno);
#endif D_LOG
	(void) close (fd);
	fd = NOTOK;     /* make sure it hangs up, later       */
	d_errno = D_PORTOPEN;
	return (D_FATAL);
    }

#ifdef D_LOG
    d_log ("d_direct", "Open");
#endif D_LOG
    return (D_OK);
}

/*
 *     D_DIAL
 *
 *     routine called to make dial-up connection to remote system
 *
 *     number -- number to be called
 *
 *     speed -- speed index
 */

d_dial (number, speed)
char   *number;
int     speed;
{
    register int    result;
    char **tlist;

    /* get ordered list of acceptable linetypes for this phone number */
    tlist = d_typelist(&number);


    /* try to find an available dial-out port for the given speed and types */
    result = d_avport (speed, tlist);

    if (result < 0)
	return (result);

    d_baudrate = speed;

#ifdef D_LOG
    d_log ("d_dial", "port %s at %d", d_prtpt -> p_port, d_baudrate);
#endif D_LOG

    /*  do the dialing  */
    if ((result = d_dodial (number)) < 0)
	d_drop ();

    return (result);
}


/*
 *     D_AVPORT
 *
 *     routine which finds an available dial-out port with the given speed.
 *     an ordered list of acceptable types is given--the first type is best.
 *     exclusive open of lock files is employed to check for other
 *     users.  if the open succeeds, a global pointer 'd_prtpt' is set
 *     to the entry in the available port structure.
 *
 *     speed -- the speed index of the port be sought
 *
 *     typelist -- a list of acceptable line types for the port
 */

d_avport (speed, typelist)
int     speed;
char **typelist;
{
    register struct dialports  *dpt;
    register int    result;
#ifdef D_LOG
    register int    i;
#endif D_LOG
    char **tpt;

#ifdef D_DBGLOG
    d_dbglog ("d_avport", "looking for port with speed %d", speed);
#endif D_DBGLOG

/* for each type in the typelist, look at each of the known dial-out    */
/* ports.  if the type and speed match, check the access list (if there */
/* is one).  they try to lock the port.                                 */



    for (tpt = typelist; *tpt; tpt++)
    {
#ifdef D_DBGLOG
	d_dbglog("d_avport", "checking for type: %s", *tpt);
#endif D_DBGLOG

	for (dpt = d_prts; dpt -> p_port; dpt++)
	{
	    if ( ((dpt -> p_speed & (1 << (speed - 1))) == 0) ||
 	         !lexequ(dpt -> p_ltype, *tpt) )
	        continue;

/*  check the access list for this port, if there is one  */

	    if (dpt -> p_access)
	    {
	        if ((result = d_chkaccess (d_uname, dpt -> p_access)) < 0)
		    return (result);

	        if (result == D_NO)
		    continue;
	    }

/*  try to lock the port.  if we can, we're in business.  */

	    if ((result = d_lock (dpt -> p_lock)) < 0)
	        return (result);


	    if (result == D_OK)
	    {
	        d_prtpt = dpt;
	        d_didial = 1;
	        return (D_OK);
	    }
        }

    }


/*  if we get here, then we've looked at all the ports and found none  */
/*  that are available.                                                */

#ifdef D_LOG
    for (i=0; d_spdtab[i].s_speed != 0; i++) {
        if (d_spdtab[i].s_index == speed) {
	   d_log ("d_avport", "No %s baud dial-out ports available",
		  d_spdtab[i].s_speed);
	   break;
	}
    }
#endif D_LOG
    if (typelist[1] != 0) {
#ifdef D_LOG
        d_log ("d_avport", "of type: %s", typelist[0]);
#endif D_LOG
    } else {
	char *cp, *textlist;
	textlist = strdup(typelist[0]);
	for (tpt = &typelist[1]; *tpt != 0; tpt++)
	{
	    cp = multcat(textlist, ",", *tpt, (char *)0);
	    free(textlist);
	    textlist = cp;
	}
#ifdef D_LOG
        d_log("d_avport", "of types: %s", textlist);
#endif D_LOG
    }

    d_errno = D_NOPORT;
    return (D_NONFATAL);
}


/*
 *     D_AVLINE
 *
 *     routine to search for a direct connect line of the given name, and to
 *     secure its use exclusively.
 *
 *     linename -- direct connect line name
 */

d_avline (linename)
register char  *linename;
{
    register struct directlines *lpt;
    register int    result;
#ifdef D_LOG
    int foundone = 0;      /* any lines exist at all? */
#endif D_LOG

/*  cycle through the available direct lines  */

    for (lpt = d_lines; lpt -> l_name != (char *) 0; lpt++)
    {
	if (strcmp (linename, lpt -> l_name) != 0)
	    continue;

#ifdef D_LOG
	foundone = 1;
#endif D_LOG

/*  check the access (if there is one)  */

	if (lpt -> l_access != (char *) 0)
	{
	    if ((result = d_chkaccess (d_uname, lpt -> l_access)) < 0)
		return (result);

	    if (result == D_NO)
		continue;
	}

/*  try to lock it.  */

	if ((result = d_lock (lpt -> l_lock)) < 0)
		return (result);

	if (result == D_OK)
	{
	    d_ptline = lpt;
	    d_didial = 0;
	    return (D_OK);
	}
    }

/*  if we get here, there is no available direct line  */

#ifdef D_LOG
    if (foundone)
	d_log ("d_avline", "No %s direct lines available", linename);
    else
	d_log ("d_avline", "No direct lines named '%s' are known", linename);
#endif D_LOG
    d_errno = D_NOPORT;
    return (D_NONFATAL);
}


/*
 *     D_DODIAL
 *
 *     this routine trys to make the connection to the remote system.
 *     it forks, the parent opens the port and the child drives the acu
 *
 *     number -- pointer to the phone number string to be passed to the
 *               dialer
 */

d_dodial (number)
char   *number;
{
    register int    parentpid,
                    childpid;
    int     fd;
    int     errcode;
#ifdef SYS5
    struct termio hupbuf;
#endif SYS5

#ifdef D_DBGLOG
    d_dbglog ("d_dodial", "about to attempt to call '%s'", number);
#endif D_DBGLOG
    parentpid = getpid ();
    if ((childpid = d_tryfork ()) == -1)
    {
#ifdef D_LOG
	d_log ("d_dodial", "couldn't fork to dial");
#endif D_LOG
	d_errno = D_FORKERR;
	return (D_FATAL);
    }

    /*  the parent tries to open the port  */
    if (childpid)
    {
#ifdef D_DBGLOG
	d_dbglog ("d_dodial", "dialing parent opening '%s'",
		d_prtpt -> p_port);
#endif D_DBGLOG

    	if (setjmp (timerest)) {
	    d_ttrestore ();
	    return (D_NONFATAL);
    	}
	s_alarm (ACUWAIT);
	fd = open (d_prtpt -> p_port, 2);
	s_alarm (0);
	d_errno = errno;      /* save, so kill does not overwrite   */
#ifdef D_DBGLOG
	d_dbglog ("d_dodial", "parent open return (errno=%d)", d_errno);
#endif D_DBGLOG

	d_cstart (number,d_prtpt -> p_ltype); /* carrier: start call timer */

	kill (childpid, SIGKILL);  /* so it doesn't waste it time */
	errcode = pgmwait (childpid);
				  /* gather up the dialing child        */
#ifdef D_DBGLOG
	d_dbglog ("d_dodial", "acu child exit error code %d", errcode);
#endif D_DBGLOG

	if (fd < 0)
	{
	    if (errno != EINTR)   /* not timeout or child kill          */
	    {                     /* so must terminate                  */
#ifdef D_LOG
		d_log ("d_dodial", "can't open port '%s', errno #%d",
			d_prtpt -> p_port, d_errno);
#endif D_LOG
		d_errno = D_PORTOPEN;
		return (D_FATAL);
	    }
	}
	else
	{
#ifdef SYS5
	    ioctl(fd, TCGETA, &hupbuf);
	    hupbuf.c_cflag |= HUPCL;
	    if (ioctl (fd, TCSETA, &hupbuf) < OK)
#else
	    if (ioctl (fd, TIOCHPCL, 0) < OK)
#endif SYS5
	    {
#ifdef D_LOG
		d_log ("d_dodial",
			    "problem setting hangup on close; errno = %d", errno);
#endif D_LOG
		(void) close (fd);
		fd = NOTOK;     /* make sure it hangs up, later       */
		d_errno = D_PORTOPEN;
		return (D_FATAL);
	    }
	    else
	    {
		d_prtfp = fdopen (fd, "r");
		if (d_ttsave (d_prtfp, d_prtpt -> p_port) < 0 ||
			d_ttscript (d_baudrate) < 0)
		{
#ifdef D_LOG
		    d_log ("d_dodial",
				"can't set dial-port parameters; errno = %d", errno);
#endif D_LOG
		    (void) close (fd);
		    fd = NOTOK;     /* make sure it hangs up, later       */
		    d_errno = D_PORTOPEN;
		    d_ttrestore ();
		    return (D_FATAL);
		}
	    }                     /* stdio for input and straight       */
				  /*   write() for output               */
	}

	switch (d_errno = errcode)
	{
	    case 0:
		if (fd < 0)
		    return (D_NONFATAL);
		return (D_OK);

	    case D_BUSY:
	    case D_ABAN:
		d_ttrestore ();
		return (D_NONFATAL);

	    default:
		d_ttrestore ();
		return (D_FATAL);
	}
    }

    /*  the child drives the acu.  if there's an error, then a
     *  signal is sent to the parent to interrupt his open call.
     *  The error code is returned to the parent through his
     *  exit status.
     */
    d_errno = 0;

    sleep ((unsigned) 5);       /* give parent time to get into open()  */
    d_drvacu (number);

    sleep ((unsigned) CONNWAIT); /* wait for open to complete    */
    kill (parentpid, SIGALRM);   /* kick parent out of open call     */

    exit (d_errno);
#ifdef lint
	return D_FATAL;
#endif lint
}

/*
 *     D_DRVACU
 *
 *     this routine opens the appropriate acu and feeds the phone number to
 *     it.  it's possible that it's in use, so we'll wait up to a minute,
 *     trying every couple of seconds.  D_FATAL and D_NONFATAL errors are
 *     possible here.
 *
 *     number -- phone number string to be passed to the dialer
 */

d_drvacu (number)
char   *number;
{
    int len;
    char linebuf[100];
    register int    acufd,
                    warning;

    warning = 0;
    if (d_prtpt -> p_acupref != (char *) 0)
	(void) strcpy (linebuf, d_prtpt -> p_acupref);
    else
	linebuf[0] = '\0';

    strcat (linebuf, number);
    if (d_prtpt -> p_acupost != (char *) 0)
	strcat (linebuf, d_prtpt -> p_acupost);

    len = strlen (linebuf);
#ifdef D_DBGLOG
    d_dbglog ("d_drvacu", "about to write '%s' on acu", linebuf);
#endif D_DBGLOG

    while (1)
    {                             /* make sure open & write don't hang  */
				/* this doesn't really work under 4.2 */
#ifdef D_DBGLOG
	d_dbglog ("d_drvacu", "opening dialer");
#endif D_DBGLOG
    	if (setjmp (timerest)) {
    	    break;
    	}
	s_alarm (DIALWAIT);
	acufd = open (d_prtpt -> p_acu, 2);
#ifdef D_DBGLOG
	d_dbglog ("d_drvacu", "dialer open (errno=%d)", errno);
#endif D_DBGLOG

/*  if we get it, then try to dial the number.  if that goes alright,  */
/*  return                                                             */

	if (acufd >= 0)
	{
	    sleep ((unsigned) 1);  /* let things settle down           */
#ifdef D_LOG
	    d_log ("d_drvacu", "Dialing...");
#endif D_LOG
	    if (write (acufd, linebuf, len) != len)
		break;

	    (void) close (acufd);      /* free the dialer */
#ifdef D_DBGLOG
	    d_dbglog ("d_drvacu", "acu successful write");
#endif D_DBGLOG
	    s_alarm (0);
	    return (D_OK);
	}

/*  we get here from a failure of the acu open.  if it's worse than just  */
/*  being busy, break out to the common error decoder                     */

	s_alarm(0);
#ifdef D_DBGLOG
	d_dbglog ("d_drvacu", "acu open (errno=%d)", errno);
#endif D_DBGLOG

	if (errno != EBUSY)
	    break;

/*  print the warning message once, and then sleep before trying again  */

	if (!warning)
	{
#ifdef D_LOG
	    d_log ("d_drvacu", "waiting for dialer.");
#endif D_LOG
	    warning = 1;
	}
	sleep ((unsigned) 5);
    }

/*  this stuff prints error messages for failures of both the open and  */
/*  write calls                                                         */

    s_alarm (0);
    switch (errno)
    {
	case EBUSY: 

#ifdef D_LOG
	    d_log ("d_drvacu", "That number is busy.");
#endif D_LOG
	    d_errno = D_BUSY;
	    return (D_NONFATAL);

	case EDNPWR: 

#ifdef D_LOG
	    d_log ("d_drvacu", "dialer power off");
#endif D_LOG
	    d_errno = D_NOPWR;
	    return (D_FATAL);

	case EDNABAN: 

#ifdef D_LOG
	    d_log ("d_drvacu", "call abandoned");
#endif D_LOG
	    d_clog (LOG_ABAN);
	    d_errno = D_ABAN;
	    return (D_NONFATAL);

	case EDNDIG: 

#ifdef D_LOG
	    d_log ("d_drvacu", "internal error: bad digit to dialer");
#endif D_LOG
	    d_errno = D_SYSERR;
	    return (D_FATAL);

	default: 

#ifdef D_LOG
	    d_log ("d_drvacu", "system errno #%d", errno);
#endif D_LOG
	    d_errno = D_INTERR;
	    return (D_FATAL);
    }
}


/*
 *     D_DROP
 *
 *     routine which is called to drop the connection to the remote system.
 *     if closes the port and lock file, and logs the call, of this was a
 *     dial-up.
 */

d_drop ()
{
#ifdef SYS5
    struct termio hupbuf;
#endif SYS5

#ifdef D_DBGLOG
    d_dbglog ("d_drop", "dropping connection.");
#endif D_DBGLOG

    /*  restore the line to its old state  */
    d_ttrestore ();

    /*  close port  */
    if (d_prtfp != (FILE *) EOF && d_prtfp != NULL)
    {
    	if (setjmp (timerest)) {
	    return;
    	}
	s_alarm (CONNWAIT);

#ifdef SYS5
	ioctl (fileno(d_prtfp), TCGETA, &hupbuf);
	hupbuf.c_cflag &= ~CBAUD;	/* set 0 baud -- hangup */
	ioctl (fileno(d_prtfp), TCSETA, &hupbuf);
#endif SYS5

	fclose (d_prtfp);
	s_alarm (0);
/* XXX Why commented out? */	/* d_prtfp = (FILE *) EOF; */
 
	if ((FILE *) EOF == d_prtfp)
	{
#ifdef D_LOG
	    d_log ("d_drop", "problem closing connection line.");
#endif D_LOG
	    return;     /* leave the line locked, in case of major problem */
	}

	if (d_didial)
	    d_clog (LOG_COMP);
    }

    /*  release the port to others by closing lock file  */
    d_unlock (d_ptline -> l_lock);
}
