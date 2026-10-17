#include "util.h"
#include "nexec.h"
#include <signal.h>

/*  generic fork/exec procedure, passing re-positioned fd's     */

/*  Jun 81  D. Crocker    make regfdary loop tmp<=HIGHFD
    May 82  D. Crocker    make close of fd tmp<=HIGHFD
    Jul 82  D. Crocker    make vax version, to use vfork
    Mar 84  D. Kingston	  Changed to use _NFILE from stdio.h instead
			  of HIGHFD.  Steve Kille: you don't have
			  to shorten the array if you have less fds.
 */

extern	int	numfds;
static	char	*zap_env[1] = { (char *)0 };

/* VARARGS4 */
#ifdef NO_VARARGS
nexecl (proctyp, pgmflags, fdarray, pgm, pgmparm, a,b,c,d,e,f,g,h,i,j,k,l,m)
#else
nexecl (proctyp, pgmflags, fdarray, pgm, pgmparm)
#endif NO_VARARGS
int   proctyp,                  /* exec / fork / fork-exec            */
	pgmflags,                 /* parent wait? disable interrupts?   */
	*fdarray;                /* what current fd's go where?        */
char	*pgm,                      /* what program to exec?              */
	*pgmparm;                  /* its parameters vector              */
{
    return (nexecv (proctyp, pgmflags, fdarray, pgm, &pgmparm));
}

nexecv (proctyp, pgmflags, fdarray, pgm, pgmparm)
int     proctyp,
	pgmflags,
	*fdarray;
char	*pgm,
	*pgmparm[];
{
    register int fd;
    int     status;
    int     childid;
    int     (*(osig1))();
    int     (*(osig2))();
    int     (*(osig3))();

    if (proctyp != PUREXEC)
    {
				  /* printf ("This is a forking call.\n"); */
	childid = tryfork ();
	if (childid == -1)
	    return (NOTOK);
				  /* printf ("Successful fork\n"); */
	if (childid != 0)
	{			  /* parent process               */
	    if (pgmflags & FRKPIGO)
	    {                     /* disable parent signals       */
				  /* printf ("Parent to be
				     non-interruptible\n"); */
		osig1 = signal (SIGHUP, SIG_IGN);
		osig2 = signal (SIGINT, SIG_IGN);
		osig3 = signal (SIGQUIT, SIG_IGN);
	    }
	    if (((proctyp == FRKEXEC) && (pgmflags & FRKWAIT)) ||
		  proctyp == SPNEXEC)
	    {
				  /* printf ("Parent is to wait\n");      */
		status = pgmwait (childid);
		if (pgmflags & FRKPIGO)
		{
		    (void)signal (SIGHUP, osig1);
		    (void)signal (SIGINT, osig2);
		    (void)signal (SIGQUIT, osig3);
		}
		return (status);
	    }
	    return (childid);
	}
	if (proctyp == SPNEXEC)
	{			  /* want it to be a spawn        */
				  /* printf ("This is a spawn\n");    */
	    switch (childid = tryfork ())
	    {
		case 0:
		    break;        /* gets to do its thing               */

		case NOTOK:       /* oops                               */
		    exit (NOTOK);

		default:          /* grandchild: kill middle proc     */
		    exit (0);
	    }
	}
    }
    if (fdarray)
    {				  /* re-align fd array list           */
	for (fd = 0; fd < numfds; fd++)
	{			  /* first do the re-positions        */
	    if (fdarray[fd] != CLOSEFD && fdarray[fd] != fd)
	    {
#ifndef NODUP2
		(void)dup2( fdarray[fd], fd );
#else NODUP2
		(void)close (fd);
#ifndef NOFCNTL
#include <fcntl.h>	    	
		fcntl (fdarray[fd], F_DUPFD, fd);
#else NOFCNTL
	    	{
		    int dupfd;

		    while ((dupfd = dup (fdarray[fd])) < fd && dupfd != -1);
				  /* did we get right fd?     */
		}
#endif NOFCNTL
#endif NODUP2
	    }
	}
	for (fd = 0; fd < numfds; fd++)
	    if (fdarray[fd] == CLOSEFD)
	    {			  /* printf ("Closing %2d\n", fd);     */
		(void)close (fd);      /* get rid of unwanted ones */
	    }
    }

    if (pgmflags & FRKCIGO)
    {
				  /* printf ("Child's interrupts to be
				     disabled\n");      */
	(void)signal (1, SIG_IGN);
	(void)signal (2, SIG_IGN);
	(void)signal (3, SIG_IGN);
    }

 /* printf ("Execing %s\n", pgm);    */
    execve (pgm, pgmparm, zap_env);
				  /* printf ("Exec not successful\n"); */
    switch (proctyp)
    {
	case PUREXEC:
	    return (NOTOK);

	default:                  /* caller can deal with it            */
	    if (pgmflags & FRKERRR)
		return (NOTOK);
    }
    exit (NOTOK);
    /* NOTREACHED */
}

pgmwait (childid)
    int childid;                  /* process id of child to collect     */
{
    int status;
    register int retval;

    while ((retval = wait (&status)) != childid)
	if (retval == NOTOK)
	    return (NOTOK);

    return (gwaitval (status));
}


LOCFUN
tryfork ()
{
    register int childid;
    register int tried;

    tried = NUMTRY;
    do {
	if(( childid = fork ()) < 0 )
		sleep( 1 );
	else
		return( childid );
    } while( tried-- );
    return( childid );
}
