# include  "util.h"
# include  <signal.h>
# include  "d_syscodes.h"
# include  "d_proto.h"
# include  "d_returns.h"
# include  <stdio.h>
# include  <strings.h>
# include  "d_structs.h"
# include  "d_script.h"

/*  Jun 81  D. Crocker    rgetc() tossed out almost all the control
 *                        characters.  changed to toss only null & DEL
 *  Sep 83  G.B. Reilly   added support for script command for prompt
 *          M. Laubach    recognition.
 */

extern char *dupfpath();
extern char *tbldfldir;
extern int errno;
extern FILE *d_scfp;
extern  unsigned short d_lxill[],
		       d_lrill[];
extern int d_baudrate;
extern int d_errno;
extern int d_lxmax;
extern int d_lrmax;
extern int d_debug;
extern int d_xretry;
extern int d_toack;
extern int d_todata;
extern int d_wpack;
#ifdef SYS5
extern unsigned short d_prbitc, d_prbiti, d_prbito, d_prbitl;
extern unsigned short d_scbitc, d_scbiti, d_scbito, d_scbitl;
#else
extern int d_pron, d_proff, d_scon, d_scoff;
#endif SYS5
extern int d_nbuff;
extern int d_didial;
extern  FILE * d_prtfp;
/*** prompt varibable ******************** reilly@udel-relay     ***/
int d_prompt = 0;         /* global prompt var.  =0 means disabled */
                          /* <>0 is ascii value of prompt character*/

#define MAXNUMS 5
#define NOPARSE -1
#define IGNORE -2
#define ANYARGS -3

/*  structure defining the legal script file commands, their type,
 *  and legal number of fields.
*/
struct scrcmds
{
    char   *s_cmdname;		  /*  command string  */
    int     s_cmdtype;		  /*  command type number  */
    int     s_cmdminfields;	  /*  min number of fields for command  */
    int     s_cmdmaxfields;       /*  max number of fields allowed */
}               d_sccmds[] =
{
                    "dial", S_DIAL, 2, 2,
                    "conn", S_DIAL, 2, 2,
                    "phone", S_DIAL, 2, 2,
                    "xillegal", S_XILL, 2, 2,
                    "rillegal", S_RILL, 2, 2,
                    "xmitill", S_XILL, 2, 2,
                    "recvill", S_RILL, 2, 2,
                    "xmitpack", S_XPCK, 2, 2,
                    "recvpack", S_RPCK, 2, 2,
                    "xmitsize", S_XPCK, 2, 2,
                    "recvsize", S_RPCK, 2, 2,
                    "transmit", S_XMIT, 2, 2,
                    "send", S_XMIT, 2, 2,
                    "xmit", S_XMIT, 2, 2,
                    "receive", S_RECV, 3, 3,
                    "recv", S_RECV, 3, 3,
                    "start", S_GO, 1, 1,
                    "go", S_GO, 1, 1,
                    "end", S_END, 1, 1,
                    "stop", S_END, 1, 1,
                    "bye", S_END, 1, 1,
                    "nrexmit", S_RETR, 2, 2,
                    "waitack", S_TOAK, 2, 2,
                    "waitdata", S_TODA, 2, 2,
		    "{", S_SELST, 1, 1,
		    "}", S_SELEND, 1, 1,
		    "alternate", S_ALT, 1, 1,
		    "case", S_ALT, 1, 1,
		    "cmnt", S_REM, IGNORE, IGNORE,
		    "rem", S_REM, IGNORE, IGNORE, 
		    "rem:", S_REM, IGNORE, IGNORE, 
		    "com", S_REM, IGNORE, IGNORE, 
		    "com:", S_REM, IGNORE, IGNORE,
		    "#", S_REM, IGNORE, IGNORE,
		    "log", S_LOG, 2, 2,
		    "abort", S_ABORT, 1, 1,
		    "bill", S_BILL, 3, 3,
		    "replay", S_REPLAY, 1, 1,
		    "mark", S_MARK, 1, 1,
#ifdef SYS5
		    "stty-pr", S_PRTTY, 3, 5,
		    "stty-sc", S_SCTTY, 3, 5,
#else
		    "stty-pr", S_PRTTY, 3, 3,
		    "stty-sc", S_SCTTY, 3, 3,
#endif SYS5
		    "window", S_DBLBUF, 3, 3,
		    "use", S_USEFILE, 2, ANYARGS,
/*** prompt command has one format  "prompt <val>"      reilly@udel-relay ***/
		    "prompt", S_PROMPT, 2, 2,
                    0, 0, 0, 0,
};

char    d_scfile[82],            /*  script file path name  */
        d_rawq[MATCHLEN],	  /*  pushback queue for string matcher  */
       *d_prawq;                  /*  pointer to next available character */
				  /*  in 'd_rawq'.  */

unsigned d_scline;                /*  current line number in script file  */
int     d_nfields = 0;		  /*  current number of "use" fields */
char    *d_fields[MAXFIELDS];     /*  current "use" fields */

int     d_ntries = 2,             /*  number of times to try to dial numbers
				  */
	d_wait = 15,              /*  number of seconds to wait between dial
				  */
				  /*  attempts.                               
				  */
        d_nrawq;		  /*  number of available characters in
				     'd_rawq'  */
int	d_dodrop;		  /*  non zero if a dial has been made  */

LOCVAR int d_gotxill = 0;	  /* non zero if xill command received */
LOCVAR int d_gotrill = 0;	  /* non zero if rill command received */

/*
 *     D_SCRIPT
 *
 *     this routine reads lines from the script file, parses them up, and
 *     then initiates the requested actions.
 */

d_script ()
    {
    register int nselect, result;
    char linebuf[MAXSCRLINE + 2],
	 *fields[MAXFIELDS];
    int nfields;

    nselect = 0;
    d_dodrop = 0;

    for (;;)
    {
	result = d_scrblk ();

	switch (result)
	{
	    case D_OK:
		return(D_OK);

	    case D_FATAL:
	    case D_EOF:
	    case D_NFIELDS:
	    case D_QUOTE:
	    case D_UNKNOWN:
		return (D_FATAL);

	    case D_NONFATAL:
		/*  An error that may be recoverable.  If there is
		 *  an alternate, use it.
		 */
		if (nselect <= 0)
		    /*  Not within a select block; therefore, not alt  */
		    return (D_FATAL);

		/*  We are within a select block;  go to the alternate  */
		if ((result = d_nxtalt ()) < 0)
		    return (result);
		switch (result)
		{
		    case S_ALT:
			continue;

		    case S_SELEND:
			/*  no more alternates;  return error  */
			return (D_FATAL);

		    default:
			d_scerr ("Bad syntax scriptfile");
			return (D_FATAL);
		}


	    case S_SELST:
		/*  The next line had better be an alternate  */
		if (d_cmdget (linebuf, &nfields, fields, d_scfp) != S_ALT)
		{
		    d_scerr ("Missing 'alternate' after 'begin'");
		    return (D_FATAL);
		}
		nselect++;
		continue;

	    case S_SELEND:
		/*  verify that there was a select block being
		 *  looked at.  If so, then the last alternate
		 *  was the successful one.  Continue normally.
		 */
		if (nselect-- <= 0)
		{
		    d_scerr ("Inappropriate select end");
		    return (D_FATAL);
		}
		continue;

	    case S_ALT:
		/*  First, make sure the context was correct  */
		if (nselect <= 0)
		{
		    d_scerr ("Inappropriate alt\n");
		    return (D_FATAL);
		}
		/*  If we get here, then an alternate within a select
		 *  was completed successfully.  Move on.
		 */
		if ((result = d_selend ()) < 0)
		    return (result);
		nselect--;
		switch (result)
		{
		    case S_SELEND:
			continue;

		    default:
			d_scerr ("error in format of script file");
			return (D_FATAL);
		}

	    default:
		if (result < 0)
		    return (result);
		d_scerr ("d_script", "Internal error: result = %d", result);
		return (D_FATAL);
	}

    }
}

d_scrblk ()
    {
    char linebuf[MAXSCRLINE + 2],
         *fields[MAXFIELDS];
    int command, result, nfields;

    for (;;)
    {
	/*  read the command  */
	command = d_cmdget (linebuf, &nfields, fields, d_scfp);
	if (command < 0)
	    return (command);


	result = d_cmdproc(command, nfields, fields);
	if (result == D_CONTIN)
	    continue;
	return (result);
    }
}

d_cmdproc (command, nfields, fields)
  int command, nfields;
  char *fields[];
    {
    register int result, word, i;
    register char *ptr;
 
    /*  switch out on the command type.  for most of the commands,
     *  there is a routine to handle them.
     */
    switch (command)
    {
	case S_EOF:
	    /*  Not really a script command.  This is returned to
	     *  indicate an unexpected EOF on the script input file.
	     *  Try to revert to a previous script file.  If there is
	     *  none, then this really is an unexpected EOF.  Return
	     *  an error to indicate so.
	     */
	    if (d_scclose() > 0)
		return (D_CONTIN);
	    return (D_EOF);

	case S_DBLBUF:
	    /*  Can use 'd_nbuff' as a counter of the number of
	     *  outstanding packets to allow if we later mod the
	     *  code to allow more than double buffering.
	     *  See the file 'd_packet.c'
	     */
	    result = atoi (fields[1]);
#ifdef D_LOG
	    if ((result != 1) &&
		(result != 2)   )
		d_log("d_cmdproc", "Illegal buffer number, %d", result);
	    else
#endif D_LOG
		d_nbuff = result;

	    /*  This variable will be non-zero if the master startup
	     *  routine should transmit a packet telling the other
	     *  side about the buffering window.  This packet can not
	     *  be transmitted to sites running the old code.
	     */
	    d_wpack = atoi (fields[2]);
	    return (D_CONTIN);

	case S_USEFILE:
	    /*  takes additional input from another file until
	     *  that file is exhausted.
	     */

	    fields[1] = dupfpath(fields[1], tbldfldir);    

	    for (i = 2; i <= nfields; i ++)
	    {
		if (*fields[i] == '"') fields[i]++;
		if (ptr = rindex(fields[i], '"')) *ptr = '\0';
	    }

	    if (d_scopen (fields[1], nfields, &fields[0]) < 0)
		return (D_FATAL);
	    return (D_CONTIN);

	case S_PRTTY:
#ifdef SYS5
	    sscanf(fields[1], "%o", &d_prbitc);
	    sscanf(fields[2], "%o", &d_prbiti);
	    sscanf(fields[3], "%o", &d_prbito);
	    sscanf(fields[4], "%o", &d_prbitl);

#ifdef D_LOG
	    d_log("d_cmdproc", "%s %o %o %o %o", fields[0],
		d_prbitc, d_prbiti, d_prbito, d_prbitl);
#endif D_LOG

#else /* SYS5 */
	    /*  Could use fields[1&2], but let's check consistancy  */
	    sscanf(fields[1], "%o", &d_pron);
	    sscanf(fields[2], "%o", &d_proff);
#ifdef D_LOG
	    d_log("d_cmdproc", "%s %o %o", fields[0], d_pron, d_proff);
#endif D_LOG
#endif SYS5
	    return (D_CONTIN);

	case S_SCTTY:
#ifdef SYS5
	    sscanf(fields[1], "%o", &d_scbitc);
	    sscanf(fields[2], "%o", &d_scbiti);
	    sscanf(fields[3], "%o", &d_scbito);
	    sscanf(fields[4], "%o", &d_scbitl);

#ifdef D_LOG
	    d_log("d_cmdproc", "%s %o %o %o %o", fields[0],
		d_scbitc, d_scbiti, d_scbito, d_scbitl);
#endif D_LOG

#else

	    sscanf(fields[1], "%o", &d_scon);
	    sscanf(fields[2], "%o", &d_scoff);
#ifdef D_LOG
	    d_log("d_cmdproc", "%s %o %o", fields[0], d_scon, d_scoff);
#endif D_LOG
#endif SYS5
	    return (D_CONTIN);

	case S_MARK:
	    d_mark ();
	    return (D_CONTIN);

	case S_REPLAY:
	    d_replay ();
	    return (D_CONTIN);

	case S_LOG:
	    /*  Force this to always log  */
	    d_plog (-1, "%s", fields[1]);
	    return (D_CONTIN);

	case S_BILL:
	    d_didial = 1;  /* pretend we're dialport for dial_log purposes */
	    d_cstart(fields[1], fields[2]);
	    return (D_CONTIN);

	case S_REM:
	    return (D_CONTIN);

	case S_ABORT:
	    return (D_FATAL);

	case S_ALT:
	case S_SELST:
	case S_SELEND:
	    return (command);

	case S_DIAL:        /* get line & make raw mode for matching */
	    /*  Make sure any previous connections are cleaned up  */
	    if (d_dodrop != 0)
	    {
		d_dodrop = 0;
		d_drop ();
	    }

	    /*  Don't let input from last dial get in this one's queue  */
	    d_mark ();

	    if (d_scdial (fields[1]) < 0)
		return (D_NONFATAL);
	    d_dodrop = 1;
	    return (D_CONTIN);

	case S_XMIT: 
	    if ((result = d_scxmit (fields[1])) < 0)
		return (result);
	    return (D_CONTIN);

	case S_RECV: 
	    if ((result = d_screcv (fields[1], fields[2])) < 0)
	        return (result);
	    return (D_CONTIN);

	case S_GO:          /* regular tty mode during session */
/*** expanded to call and log ttscript when prompt active ***/
/*** necessary to get raw mode for control chars          ***/
/*** 9/27/83   laubach@udel-relay                         ***/
	    if (d_prompt == 0) {
                if (d_ttproto (d_baudrate) < 0)
		return (D_NONFATAL);
            } 
            else {
                if (d_ttscript ( d_baudrate) < 0)
                return (D_NONFATAL);
#ifdef D_DBGLOG
                d_dbglog("d_cmdproc","ttscript selected for raw prompt mode");
#endif D_DBGLOG
            }
	    return (D_OK);

	case S_END: 
	    return (D_OK);

	case S_XPCK: 
	    result = atoi (fields[1]);

	    if ((result < MINPATHSIZ) || (result > MAXPACKET))
	    {
		d_scerr ("illegal max transmit packet length: %d", result);
		d_errno = D_SCRERR;
		return (D_NONFATAL);
	    }

#ifdef D_DBGLOG
	    d_dbglog ("d_script", "setting 'd_lxmax' to %d", result);
#endif D_DBGLOG
	    d_lxmax = result;
	    return (D_CONTIN);

	case S_RPCK: 
	    result = atoi (fields[1]);

            if ((result < MINPATHSIZ) || (result > MAXPACKET))
	    {
		d_scerr ("illegal max receive packet length");
		d_errno = D_SCRERR;
		return (D_NONFATAL);
	    }

#ifdef D_DBGLOG
	    d_dbglog ("d_script", "setting 'd_lrmax' to %d", result);
#endif D_DBGLOG
	    d_lrmax = result;
	    return (D_CONTIN);

	case S_XILL: 
	    if (!d_gotxill)  { /* zero the vector on first S_XILL command */
		d_gotxill = 1;
#ifdef D_DBGLOG
		d_dbglog("d_script", "zeroing xill vector");
#endif D_DBGLOG
		for (word = 0; word < 8; word++)
		    d_lxill[word] = 0;
	    }

	    if ((result = d_scill (fields[1], d_lxill)) < 0)
		return (result);

#ifdef D_DBGLOG
	    d_dbglog ("d_script", "d_lxill = %o %o %o %o %o %o %o %o",
			d_lxill[0], d_lxill[1], d_lxill[2], d_lxill[3],
			d_lxill[4], d_lxill[5], d_lxill[6], d_lxill[7]);
#endif D_DBGLOG
	    return (D_CONTIN);

	case S_RILL: 
	    if (!d_gotrill)  { /* zero the vector on first S_RILL command */
		d_gotrill = 1;
#ifdef D_DBGLOG
		d_dbglog("d_script", "zeroing rill vector");
#endif D_DBGLOG
		for (word = 0; word < 8; word++)
		    d_lrill[word] = 0;
	    }

	    if ((result = d_scill (fields[1], d_lrill)) < 0)
		return (result);

#ifdef D_DBGLOG
	    d_dbglog ("d_script", "d_lrill = %o %o %o %o %o %o %o %o",
			d_lrill[0], d_lrill[1], d_lrill[2], d_lrill[3],
			d_lrill[4], d_lrill[5], d_lrill[6], d_lrill[7]);
#endif D_DBGLOG
	    return (D_CONTIN);

	case S_RETR:
	    result = atoi (fields[1]);
	    if (result <= 0)
		d_scerr("d_script", "Bad retry value: %d", result);
	    else
		d_xretry = result;
	    return (D_CONTIN);

	case S_TOAK:
	    result = atoi (fields[1]);
	    if (result <= 0)
		d_scerr ("d_script", "Bad ack time out value: %d", result);
	    else
		d_toack = result;
	    return (D_CONTIN);

	case S_TODA:
	    result = atoi (fields[1]);
	    if (result <= 0)
		d_scerr ("d_script", "Bad data time out value: %d", result);
	    else
		d_todata = result;
	    return (D_CONTIN);

/*** prompt has one field which is the decimal ascii value of the prompt ***/
/*** character                                        reilly@udel-relay  ***/
	case S_PROMPT:
	    d_prompt = atoi (fields[1]);
#ifdef D_DBGLOG
	    d_dbglog ("d_script", "Prompt ASCII %d (decimal)", d_prompt);
#endif D_DBGLOG
	    return (D_CONTIN);

	default: 
	    d_scerr ("internal error -- unknown command %d", command);
	    return (D_FATAL);
    }
}

d_cmdget (linebuf, nfields, fields, channel)
  int *nfields;
  char *linebuf, *fields[];
  FILE *channel;
    {
    register int result;
    char *nstart;
    int command, nwantmin, nwantmax;
    register struct scrcmds *cmdpt;
   
    /*  read a line from the script file  */
again:
    if ((result = d_scgetline (linebuf, channel)) < 0)
	return (result);

    if (result == 0)
	return (S_EOF);

#ifdef D_DBGLOG
    d_dbglog ("d_scrproc", "line %d - '%s'", d_scline, linebuf);
#endif D_DBGLOG

    /*  First, seperate off the first field of the input line  */
    switch (d_getword(linebuf, &fields[0], &nstart))
    {
	case -1:
	    /*  end of line.  I.e, no line.  Try again  */
	    goto again;

	case -2:
	    /*  unterminated quote.  */
	    d_scerr ("quoted string missing end quote");
	    return (D_QUOTE);

	case 0:
	    /*  Found a word OK  */
	    break;

	default:
	    d_scerr ("unknown return from d_getword");
	    return (D_FATAL);
    }

    /*  search the command list for the string in the first field
     *  of the line.
     */
    cmdpt = d_sccmds;
    for(;;)
    {
	if (cmdpt -> s_cmdname == 0)
	{
	    d_scerr ("command not known");
	    return (D_UNKNOWN);
	}

	if (strcmp (fields[0], cmdpt -> s_cmdname) != 0)
	{
	    cmdpt++;
	    continue;
	}

	command = cmdpt -> s_cmdtype;
	nwantmin = cmdpt -> s_cmdminfields;
	nwantmax = cmdpt -> s_cmdmaxfields;
	break;
    }

    /*  if this is meant to be tossed, go get another line */
    if (nwantmin == IGNORE)
	goto again;  

    /*  if this is not meant to be parsed further, don't  */
    if (nwantmin == NOPARSE)
    {
	fields[1] = nstart;
	return (command);
    }

    /*  parse the rest of the line and make sure the right number
     *  of fields are present.
     */
    *nfields = d_linparse (&fields[1], nstart, MAXFIELDS-1);
    if (*nfields < 0)
        switch (*nfields)
	{
	    case -1: 
		d_scerr ("too many fields");
		return (D_NFIELDS);

	    case -2: 
		d_scerr ("quoted string missing end quote");
		return (D_QUOTE);

	    case -3: 
		d_scerr ("backquote appeared at end of line");
		return (D_QUOTE);

	    case -4:
		d_scerr ("invalid $ variable used");
		return (D_FATAL);

	    case -5:
		d_scerr ("line too long");
		return (D_FATAL);
	
	    default: 
		d_scerr ("unknown error from line parser");
	    	return (D_FATAL);
	}

    if ( (nwantmin == ANYARGS || *nfields >= (nwantmin - 1)) &&
         (nwantmax == ANYARGS || *nfields <= (nwantmax - 1)) )
	return (command);

    /*  If it gets here, then this command had wrong # of fields  */
    d_scerr ("command has %d fields", (*nfields+1));
    return (D_NFIELDS);
}




d_selend ()
    {
    register int command;
    char linebuf[MAXSCRLINE + 2],
	 *fields[MAXFIELDS];
    int nfields;

    for (;;)
    {
	command = d_cmdget (linebuf, &nfields, fields, d_scfp);
	if (command < 0)
	    return (command);
	switch (command)
	{
	    default:
		continue;

	    case S_SELEND:
		return (S_SELEND);

	    case S_SELST:
		d_selend ();
		continue;

	}
    }
}

d_nxtalt ()
    {
    register int command;
    char linebuf[MAXSCRLINE + 2],
	 *fields[MAXFIELDS];
    int nfields;

    for (;;)
    {
	command = d_cmdget (linebuf, &nfields, fields, d_scfp);
	if (command < 0)
	    return (command);
	switch (command)
	{
	    default:
		continue;

	    case S_SELEND:
	    case S_ALT:
		return (command);

	    case S_SELST:
		d_selend ();
		continue;
	}
    }
}

/*
 *     D_SCRDIAL
 *
 *     this routine is called to do the dial command in the script file.
 *     it just calls lower level routines to parse the number string and
 *     do the actual dialing.
 *
 *     number -- number specification string from the script file
 */

d_scdial (number)
register char  *number;
{
    register int    nnumbs,
                    result;
    struct telspeed telnumbs[MAXNUMS];

#ifdef D_DBGLOG
    d_dbglog ("d_scdial", "attempting to call '%s'", number);
#endif D_DBGLOG

/*  parse the number spec, do the dialing, and set the port to raw mode  */

    if ((nnumbs = d_numparse (number, telnumbs, MAXNUMS, d_scfile, d_scline)) < 0)
	return (nnumbs);

    if ((result = d_connect (telnumbs, nnumbs, d_ntries, d_wait)) < 0)
	return (result);

    return (D_OK);
}

/*
 *     D_SCRXMIT
 *
 *     this routine is called to transmit a string given in the script file.
 *     the string is converted to standard internal format and then passed
 *     to a lower transmission routine.
 *
 *     string -- string to be transmitted in 'C' style
 */

d_scxmit (string)
register char  *string;
{
    register int    length;
    char    canonstr[MAXSCRLINE];

    if (*string == '"')
	if ((length = d_canon (string, canonstr)) < 0)
	{
	    d_scerr ("transmit string format error");
	    return (D_FATAL);
	}

    if (d_xstring (canonstr, length) < 0)
	return (D_FATAL);

    return (D_OK);
}

/*
 *     D_SCRRECV
 *
 *     this routine is called to wait until a given string is received on
 *     the port.  the routine will return when the sting has been identified.
 *     there is also a timer that is set to cause the routine to return with
 *     an error if the string is not received within the specified interval.
 *
 *     string -- the string to be watched for
 *
 *     timestr -- a timeout value, in seconds, given as an ascii string
 */

d_screcv (string, timestr)
char   *string,
       *timestr;
{
    register unsigned int   timeout;
    register int    length,
                    result;
    char    canonstr[MAXSCRLINE];

#ifdef D_DBGLOG
    d_dbglog ("d_screcv", "looking for '%s' in '%s' seconds", string, timestr);
#endif D_DBGLOG

/*  convert the transmit string, check its length, and convert the timeout  */
/*  value.                                                                  */

    if ((length = d_canon (string, canonstr)) < 0)
    {
	d_scerr ("receive string format error");
	return (D_FATAL);
    }

    if (length > MATCHLEN)
    {
	d_scerr ("receive string too long");
	return (D_FATAL);
    }

    if ((result = atoi (timestr)) < 1)
    {
	d_scerr ("illegal timeout value");
	return (D_FATAL);
    }
    timeout = result;            /* convert to unsigned */

/*  set up the timer  */

    if (setjmp (timerest)) {
	d_scerr ("no match for '%s' after %d seconds", string, timeout);
	d_errno = D_TIMEOUT;
	return (D_NONFATAL);
    }
    s_alarm ((unsigned) timeout);

/*  do the matching  */

    while ((result = d_scmatch (canonstr)) == D_NO)
	d_rgetc ();

    s_alarm (0);

    switch (result)
    {
	case D_OK:
	    return (D_OK);

	case D_NONFATAL:
	    d_scerr ("eof on port while trying match");
	    d_errno = D_PORTEOF;
	    return (D_NONFATAL);

	case D_FATAL:
	    d_scerr ("read error on port while trying match");
	    d_errno = D_PORTRD;
	    return (D_FATAL);

	case D_INTRPT:
	    d_scerr ("no match for '%s' after %d seconds", string, timeout);
	    d_errno = D_TIMEOUT;
	    return (D_NONFATAL);
    }

    d_errno = D_SYSERR;
    return (D_FATAL);
}


/*
 *     D_SCILL
 *
 *     this routine is called to translate a string specifying illegal
 *     characters for either transmission or recption, and set the bits
 *     corresponding to those characters in the given bit vector.
 *
 *     string -- pointer to character specification string in canonical
 *               format
 *
 *     bitvector -- pointer to bit vector
 */

d_scill (string, bitvector)
char   *string;
unsigned short bitvector[];
{
    register int    bit,
                    length;
    register char  *cp;
    char    canonstr[MAXSCRLINE];

/*  translate the character string  */

    if ((length = d_canon (string, canonstr)) < 0)
    {
	d_scerr ("error in illegal character specification");
	return (D_FATAL);
    }

/*  run through the bytes in the converted string and set the bit  */
/*  corresponding to each one                                      */

    cp = canonstr;

    for (bit = 0; bit < length; bit++)
	d_setbit (*cp++, bitvector);

    return (D_OK);
}


/*
 *     D_SCMATCH
 *
 *     this routine checks for a match between the string and the input
 *     stream.
 *
 *     string -- string to be matched in the input stream
 *
 *
 *     return values:
 *
 *          D_OK -- the string has been found
 *
 *          D_NO -- the string cannot be matched with the current starting
 *                character.  delete the first character and try again.
 *
 *          D_NONFATAL -- eof on port while reading characters
 *
 *          D_FATAL -- read error on port
 *
 *          D_INTRPT -- no match after the specified time
 */

d_scmatch (string)
char   *string;
{
    register int    c,
                    result;

    if (*string == '\0')
	return (D_OK);

    if ((c = d_rgetc ()) < D_OK)
	return (c);

    if (c == *string)
    {
	result = d_scmatch (++string);

	if (result < D_OK)
	    d_unrgetc (c);

	return (result);
    }
    else
    {
	d_unrgetc (c);
	return (D_NO);
    }
}

/*
 *     D_RGETC
 *
 *     this routine is used to read characters from the port in raw mode.
 *     to allow matching of strings in the input stream, this function
 *     implements a look behind facility.  the port is read only if the
 *     push back list is empty.
 */

d_rgetc ()
{
    register int    result;
    int     theval;
    char c;

/*  if the queue isn't empty, use what it has  */

    if (d_nrawq)
    {
	d_nrawq--;
	return (*d_prawq--);
    }

/*  have to read the port.  watch for timeout interrupts  */

    while ((theval = d_getc (d_prtfp)) != EOF)
    {
	c = toascii (theval);    /*  filter out some of the junk characters */
	if ((result = d_tscribe ((char *) &c, 1)) < 0)
	    return (result);

	switch (c)
	{                         /* let controls go by, except         */
	    case '\000':          /* skip null & DEL, because they are  */
	    case '\177':          /* almost always just noise           */
		continue;
	}

	return (c);
    }

    if (feof (d_prtfp))
	return (D_NONFATAL);

    if (errno == EINTR)
	return (D_INTRPT);

    return (D_FATAL);
}


/*
 *     D_UNRGETC
 *
 *     this routine pushes back a character onto the look behind queue
 *     used by the 'd_rgetc' routine
 *
 *     c -- the character to be pushed back
 */

d_unrgetc (c)
char    c;
{

/*  set up the queue if it isn't ready  */

    if (d_prawq <= 0)
    {
	d_prawq = d_rawq;
	*d_prawq = c;
	d_nrawq = 1;
	return;
    }

/*  otherwise, stick it on the end  */

    *++d_prawq = c;
    d_nrawq++;
}

/*
 *     D_XSTRING
 *
 *     this routine is called to transmit strings on the port.  If the
 *     string contains octal 377, then the function pauses for 1 second
 *     before continuing.
 *
 *     string -- pointer to string
 *
 *     length -- number of characters in string to be sent.
 */

d_xstring (string, length)
register char   *string;
int     length;
{
    register int    result;

#ifdef D_DBGLOG
    d_dbglog ("d_xstring", "sending '%s', length %d", string, length);
#endif D_DBGLOG

    sleep ((unsigned) 2);       /* permit any needed line-settling      */
    for ( ; length--; string++)
    {
	/* NOSTRICT */
	if ((int)*string == -1)
	    sleep ((unsigned) 1);
	else if ((int)*string == -2)
	    d_brkport();
	else
	    if ((result = d_wrtport (string, 1)) < 0)
		break;
        if (*string == '\r')
	    sleep ((unsigned) 1); /* may nned line-settling   */
    }
    sleep ((unsigned) 2);       /* permit any needed line-settling      */

    return (result);
}

/*
 *     D_SCRERR
 *
 *     routine which is called to log errors in the script file.  the
 *     purpose of this is so the name of the file and the line number
 *     can be tacked on in one place.
 *
 *     format -- 'd_log' type format string
 *
 *     a, b, c, d, ... -- variables to be logged
 */

/* VARARGS1 */
d_scerr (format, a, b, c, d, e, f, g, h)
char   *format;
unsigned a, b, c, d, e, f, g, h;
{
#ifdef D_LOG
    char    tmpfmt[256];

    sprintf (tmpfmt, "%s%s", "Error in %s, line %d: ", format);

    d_log ("d_scerr", tmpfmt, d_scfile, d_scline, a, b, c, d, e, f, g, h);
#endif D_LOG
    d_errno = D_SCRERR;
}
