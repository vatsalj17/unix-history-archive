#include "util.h"
#include "mmdf.h"

/*  Main routine called by Q NIFTP process to invoke            */
/*  Q NIFTP channel of MMDF                                     */
/*                                                              */
/*  Note that the files are prefixed nn_                        */
/*  wheras the routines are prefixed ni_                        */
/*  This is to distinguish between linked and unlinked files    */
/*                                                              */
/*      Steve Kille     August 1982                             */

#define RETRY_TIME      5       /* Delay between retries in secs*/


extern char *cmddfldir;
				/* Default directory for NIFTP  */
				/* to MMDF routine              */

char nichan[] = "ni_niftp";


extern int   ipcs_print;

extern char *dupfpath ();


LOCVAR char tmpstring[LINESIZE];
				/* static buffer for return message */

/**/

ni_mail (fname, queue, TSname, max_tries, rpstring)
char  fname[];                  /* File to be transferred       */
char  queue[];                  /* name of NIFTP queue          */
char  TSname[];                 /* TS name of calling host      */
int   max_tries;                /* Max number of retries        */
				/* If 0 mailer will detach and  */
				/* Not return                   */
char  **rpstring;               /* Where to stuff result string */
				/* This might be passed to the  */
				/* remote NIFTP                 */
{
    int   pid;                  /* for fork                     */
    int   status;
    char  chpath[LINESIZE];     /* Full path of NIFTP channel   */
    int   no_tries;             /* Number of tries to deliver   */
    int   fd;                   /* temp file pointer            */



    if (ipcs_print > 0)
	printf ("ni_mail (f='%s', q='%s', TS='%s', n='%d')\n", fname,
		queue, TSname, max_tries);


    *rpstring = tmpstring;
		/* point to line buffer                         */


    getfpath (nichan, cmddfldir, chpath);
					/* get path of channel process  */


				/* Now try to submit the mail to the chan */
				/* In the case of partial failure repeat  */
				/* as specified by max_tries              */
    if (ipcs_print > 0)
	printf ("execl (%s, %s, %s, %s, %s)", chpath, nichan, queue,
		fname, TSname);


    no_tries = 0;               /* First attempt                          */
    FOREVER                     /* no test here as we must enter once     */
    {
	if ((pid = fork ()) == NOTOK)
	{
	    status = RP_AGN;    /* Should be temporary                    */
	    continue;
	}

	switch (pid)
	{
	    case 0:            /* In the child                            */
				/* close off any open files  NIFTP leaves   */
				/* lying around                             */
		for (fd = 0; fd < _NFILE; fd++)
			close (fd);
				/* open standard output                    */
		open ("/dev/null", 1);

		execl (chpath, nichan, queue, fname, TSname, (char *)0);
				/* If we get here something hs screwed      */
		exit (RP_MECH);
		break;

	   default:
		if (max_tries < 1)      /* don't wait around              */
		{
		    sprintf (tmpstring,
				"Not waiting for MMDF to transfer mail");
		    return (OK);
		}

		status = pgmwait (pid); /* hang around for MMDF            */
					/* but not any of its children     */
					/* should protect this with alarms */
					/* But UCL being the place it is...*/

		if (status == 0)
					/* Fatal crash                  */
		{
		    sprintf (tmpstring, "MMDF crashed - aarrg");
		    return (NOTOK);
		}

		if (rp_isgood (status))
					/* Success                         */
		{
		    sprintf (tmpstring, "Mail transferred correctly (%s)",
						rp_valstr (status));
		    return (OK);
		}

		if ((rp_gbval (status)) ==  RP_BNO)
					/* Complete disaster so go home     */
		{
		    switch (status)
		    {
			case  RP_NDEL:
				sprintf (tmpstring, "No valid recipients" );
				return (OK);
				break;
			default:
				sprintf (tmpstring,
					"Fatal error on mail transfer (%s)",
					rp_valstr (status));
				return (NOTOK);
		     }
		}
	}
					/* Do we need to make any more tries*/
	if (no_tries++ >= max_tries)
	    break;
					/* Otherwise sleep a while and then */
					/* Try again                        */
	sleep (RETRY_TIME);
   }

					/*  We have tried as many times as  */
					/*  allowed                         */

    sprintf (tmpstring, "Repeated temporary MMDF failure (%s) after %d attempts",
			      rp_valstr (status), no_tries);
    return (NOTOK);
}


