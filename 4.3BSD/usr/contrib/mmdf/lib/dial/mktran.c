#
/*	Program to help make a transcript file  */

#include	<stdio.h>
#include	"d_returns.h"
#include	"ll_log.h"
#include	"d_proto.h"
#include	"d_script.h"

#define		SCRIPT		"Script"
#define		TRANFILE	"Tranfile"
#define		LOGFILE		"Log"

struct ll_struct log = {
    "Log",
    "mktran log",
    '\0177',
    100,
    0,
    0,
    0,
};

FILE *input, *output;
char *sfile;

main (argc, argv)
  int argc;
  char **argv;
    {
    register int command;
    char linebuf[MAXSCRLINE +2], *fields[MAXFIELDS];
    register int result;
    int nfields;

    
    /*  init various things:  logging, files, etc.  */
    init (argc, argv);

    /*  run through that part of the script which already exists  */
    printf("Starting to process existing script.\n");
    if ((result = doscript ()) < 0)
    {
	printf("Problem encountered in processing existing script.\n");
	printf("Return value of %d\n", result);
	printf("Aborting.\n");
	exit(-1);
    }
    printf("Existing script processed successfully\n");

    /*  Now add more to it  */
    for (;;)
    {
	/*  Read a new command line from the terminal  */
	printf ("Enter command: ");
	fflush (stdout);

	command = d_cmdget (linebuf, &nfields, fields, stdin);
	if (command < 0)
	{
	    /*  may be an error;  may also be the command to turn
	     *  the line around.  Check it.
	     */
	    if (turnar (linebuf))
		doread();
	    else
	    {
		printf ("Unrecognized command.  getcmd returns %d\n", command);
		printf ("Try again\n");
	    }
	    continue;
	}

	/*  write the new line to the script  */
	wrtscr ();


	/*  process the new line  */
	result = d_cmdproc (command, nfields, fields);

	/*  check on the results of the processing  */
	switch (result)
	{
	    case D_CONTIN:
		continue;

	    case D_OK:
		quit ();

	    case D_FATAL:
		printf ("Fatal error on command line.  Abort\n");
		quit ();
	}
    }

}





init (argc, argv)
  int argc;
  char **argv;
    {
    extern int errno;
    extern FILE *d_scfp;
    extern int d_master, d_snseq, d_rcvseq;
    extern int d_debug;
    register int result;
    register int word;

    printf("Usage: %s [<script file> [<debug value> [<transcript file>]]]\n",
			argv[0]);
    /*  First, determine the name of the script file  */
    if (argc > 1)
	sfile = argv[1];
    else
	sfile = SCRIPT;

    /*  open it for appending  */
    output = fopen (sfile, "a");
    if (output == NULL)
    {
	printf ("Can't open '%s': errno %d - ", sfile, errno);
	fflush (stdout);
	perror ("");
	exit (-1);
    }
    fseek (output, 0L, 2);

    /*  check for debugging  */
    if (argc > 2)
	d_debug = atoi (argv[2]);
    else
	d_debug = 0;

    /*  open the log and transcript files  */
    if ((result = d_opnlog (&log)) < 0)
    {
	printf("Couldn't open log file;  d_opnlog returns %d\n", result);
	exit(-1);
    }
    d_init ();


    /*  check to see if a transcript is wanted  */
    if (argc > 3)
	if (atoi (argv[3]) != 0)
	    if ((result = d_tsopen (TRANFILE)) < 0)
		return (result);

    d_master = 1;
    d_snseq = 3;
    d_rcvseq = 3;

    /*  at last, more or less done  */
    return (0);
}






wrtscr ()
    {
    extern char d_origlin[];
    register int length, nwrote;

    length = strlen (d_origlin);

    nwrote = fwrite (d_origlin, sizeof (char), length, output);
    if (nwrote != length)
    {
	printf ("fwrite tried %d, but returned %d\n", length, nwrote);
	printf ("Couldn't write to script file: ");
	fflush (stdout);
	perror ("");
	fflush (stderr);
	exit (-1);
    }

    return;
}





doscript ()
    {
    extern FILE *d_scfp;
    extern int errno;
    int nselect, result;
    char linebuf[MAXSCRLINE + 2],
	 *fields[MAXFIELDS];


    /*  open the script file  */
    if ((result = d_scopen (sfile, 0, (char **) 0)) < 0)
    {
	printf ("Couldn't open '%s': errno %d - ", sfile, errno);
	fflush (stdout);
	perror ("");
	fflush (stderr);
	exit (-1);
    }

    /*  now process it as a normal script with the exception that
     *  an EOF means read from the standard input instead of stop.
     */
    for (;;)
    {
	result = d_scrblk ();
	switch (result)
	{
	    case D_OK:
		return (D_OK);

	    case D_EOF:
	    case D_NFIELDS:
	    case D_QUOTE:
	    case D_UNKNOWN:
	    case D_FATAL:
		/*  a fatal error  */
		return (D_FATAL);

	    case D_EOF:
		/*  a normal termination since we expect the user to
		 *  add more immediately.
		 */
		return (0);

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
	}

    }
}




quit ()
    {

    printf ("Quitting\n");
    exit (0);
}

