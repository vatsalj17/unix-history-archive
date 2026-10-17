/* Server SMTP daemon */

#include "sys/types.h"
#include "signal.h"
#include "netlib.h"
#include "con.h"
#include <stdio.h>

char *Channel;
char *Server;
int  Port;
int  Maxconnections = 4;
char *Network;
int numconnections = 0;

extern int errno;

struct con openparam;

main (argc, argv)
int argc;
char **argv;
{
	register  netfid;
	struct netstate statparam;
	char *us, *them;
	char *getuc ();
	int i;
	int pid;
	int alarmtrap() ;

	if (argc != 6)
	{
		printf ("Usage:  %s server channels port maxconns network\n", argv[0]);
		exit(1);
	}

	Server = argv[1];
	Channel = argv[2];
	Port = atoi(argv[3]);
	Maxconnections = atoi(argv[4]);
	Network = argv[5];

	signal (SIGALRM, alarmtrap);

	dup2 (1, 2);			/* make log file standard error */
	while (1)
	{
		openparam.c_mode = CONTCP;
		openparam.c_lport = Port;
		openparam.c_rbufs = 2;	/* Ask for 2K receive buffer */
		if (isbadhost(openparam.c_lcon = gethost(Network)))
		    {
		    printf ("SMTP daemon cannot find network '%s'", Network);
		    exit (1);
		    }

		while ((netfid = open ("/dev/net/net", &openparam)) < 0)
			{
			printf ("Return of %d from open, errno = %d\n", netfid, errno); 
			fflush (stdout);
			sleep (60);
			}

		ioctl (netfid, NETSETE, 0);

/*		ioctl (netfid, NETGETS, &statparam);

		printf ("Connection from %s\n", hostfmt(openparam.c_fcon,1));
*/
		if(( pid = fork()) < 0 ) {
/*			ll_log( "could not fork (%d)", errno);  */
			(void) close( netfid );
		} else if( pid == 0 ) {
			/*
			 *  Child
			 */

/*			us = getuc (thisname());	/* who are we? */
			us = getuc (Network);  /* daemon knows correct name */
			ioctl (netfid, NETGETS, &statparam);
			them = hostname (statparam.n_fcon); /* who are they? */
			dup2 (netfid, 0);
			dup2 (netfid, 1);
			close (netfid);
			execl (Server, Server, them, us, Channel, (char *)0);
			exit (1);
		}


		/*
		 *  Parent
		 */
		close (netfid);
		numconnections++;

		/*
		 *  This code collects ZOMBIES and implements load
		 *  limiting by staying in the do loop while the
		 *  Maxconnections active.
		 */
		if( numconnections > 1 || Maxconnections == 1 )
			do {
				alarm( 2 );
				if( wait( &i ) > 0 )
					numconnections--;
				alarm( 0 );
			} while( numconnections >= Maxconnections );

	}

}

/*
 * Uppercase a string in place. Return pointer to
 * null at end.
 */
char *
getuc(s)
    char *s;
{
    register char *p,
		  c;
    for (p = s; c = *p; p++)
    {
	if (c <= 'z' && c >= 'a')
	    *p -= ('a' - 'A');
    }
    return(s);
}

alarmtrap ()
{

	signal (SIGALRM, alarmtrap);
}
