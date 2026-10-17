static	char	sccsid[] = "@(#)pwtable.c	2.6";	/*	SCCS id keyword	*/
#include <stdio.h>
#include <signal.h>

/*
 * create password tables
 */

main ()  {
	signal (SIGHUP,SIG_IGN);
	signal (SIGINT,SIG_IGN);
	signal (SIGQUIT,SIG_IGN);
	signal (SIGTERM,SIG_IGN);
	if (pwlock("w")) {
		printf("Temp file busy\n");
		exit(-1);
	}
	pwunlock();	
}
