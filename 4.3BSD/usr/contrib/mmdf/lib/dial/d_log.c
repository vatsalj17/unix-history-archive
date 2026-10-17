# include  "util.h"
# include  "d_returns.h"
# include  <stdio.h>
# include  "d_proto.h"
# include  "d_structs.h"
# include  "ll_log.h"


int  d_debug;           /*  set to non-zero if debug loggin should be done  */

static struct ll_struct *d_logptr;       /*  file pointer to logging file  */


/*
 *     D_OPNLOG
 *
 *     this routine is called to open the log file for appending.  the
 *     date is written as the first entry.
 *
 *     filename -- path name of the logging file
 */

d_opnlog(logptr)
  struct ll_struct * logptr;
    {

    d_logptr =(struct ll_struct *) (logptr);           /* save the pointer */

    if (d_logptr -> ll_level >= LLOGPTR)
	d_debug = 1;
    return(D_OK);
}


/*
 *     D_CLSLOG
 *
 *     this routine is called to close the log file if one was open.
 */

d_clslog()
    {

#ifdef D_DBGLOG
    d_log("d_clslog", "closing log file");
#endif D_DBGLOG

    ll_close(d_logptr);

    return(D_OK);
}

/*
 *     D_LOG
 *
 *     this routine writes entries on the log file.  its use is similar
 *     to printf, and it accepts the same style format strings.
 *
 *     routine -- the name of the subroutine where the call to this function
 *                is made.
 *
 *     format -- printf style format string
 *
 *     a, b, c, d, ... -- variables to be printed
 */

/* VARARGS2 */

d_log(routine, format, a, b, c, d, e, f, g, h)
  char  *routine, *format;
  unsigned  a, b, c, d, e, f, g, h;
    {

#ifdef D_LOG
#ifdef D_DBGLOG
    if (d_debug)        /*  print routine name if debugging  */
	d_dbglog (routine, format, a, b, c, d, e, f, g, h);
    else
#endif D_DBGLOG
	ll_log(d_logptr, LLOGGEN, format, a, b, c, d, e, f, g, h);
#endif D_LOG

    return(D_OK);
}

/*
 *     D_DBGLOG
 *
 *     this routine is called to write debugging logging stuff on the log
 *     file.  the entry is made only if the 'd_debug' variable is non-zero.
 *
 *     routine -- the name of the subroutine where the call to this function
 *                is made.
 *
 *     format -- printf style format string
 *
 *     a, b, c, d, ... -- variables to be printed
 */

/* VARARGS2 */

d_dbglog(routine, format, a, b, c, d, e, f, g, h)
  char  *routine, *format;
  unsigned  a, b, c, d, e, f, g, h;
    {
#ifdef D_DBGLOG
    char fmtbuf[64];

    if (d_debug)
    {
	sprintf (fmtbuf, "%s%s", "%s: ", format);
	ll_log(d_logptr, LLOGBTR, fmtbuf, routine, a, b, c, d, e, f, g, h);
    }
#endif D_DBGLOG
}



/*
/*
 *     D_FILLOG
 *
 *     routine which is used to log messages concerning things found in
 *     files.  the point of this routine is that there are a lot of messages
 *     in this package that concern errors found on lines in files.  i got
 *     tired of putting stuff in the format to 'd_log' calls to write out
 *     the file name and line number, so i put that in this call.  all this
 *     routine does is build a special format string containg fields for the
 *     name and number, and then call 'd_log'.
 *
 *     filename -- name of the file to be put in entry
 *
 *     linenum -- line number in file to be put in entry
 *
 *     routine -- the name of the subroutine where the call to this function
 *                is made.
 *
 *     format -- printf style format string
 *
 *     a, b, c, d, ... -- variables to be printed
 */

/* VARARGS4 */

d_fillog(filename, linenum, routine, format, a, b, c, d, e, f, g, h)
  char  *filename, *routine, *format;
  unsigned  linenum, a, b, c, d, e, f, g, h;
    {
#ifdef D_LOG
    char  tmpfmt[85];

    /*  if a filename is given, make a new format to include
     *  it.  then call the log routine.
     */

    if (filename)
    {
	sprintf (tmpfmt, "%s%s", "%s, line %d: ", format);
	d_log(routine, tmpfmt, filename, linenum, a, b, c, d, e, f, g, h);
    }
    else
	d_log(routine, format, a, b, c, d, e, f, g, h);
#endif D_LOG
}




/*
 *	D_PLOG
 *
 *	Log a message at a given priority.
 *	It is necessary to have this here so as to access the
 *	struct d_logptr.
 *
 *	priority - the priority level to log at.
 *	format, a, b, ... - standard printf stuff
 *
*/

d_plog (priority, format, a, b, c, d, e, f, g, h)
  int priority;
  char *format;
  unsigned a, b, c, d, e, f, g, h;
    {

    ll_log (d_logptr, priority, format, a, b, c, d, e, f, g, h);
}

/*
 *	D_PELOG
 *
 *	Log a message at a given priority and print ERRNO
 *	It is necessary to have this here so as to access the
 *	struct d_logptr.
 *
 *	priority - the priority level to log at.
 *	format, a, b, ... - standard printf stuff
 *
*/

d_pelog (priority, format, a, b, c, d, e, f, g, h)
  int priority;
  char *format;
  unsigned a, b, c, d, e, f, g, h;
    {

    ll_err (d_logptr, priority, format, a, b, c, d, e, f, g, h);
}
