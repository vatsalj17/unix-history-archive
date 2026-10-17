/***************************************************************\
* 							        *
* 	Rcvprint - a program to take a mail message as input    *
* 	and print it on a line printer			        *
* 							        *
* 	A really trivial little program.		        *
* 	Rewritten to clean up				        *
* 		Jpo@cs.nott.ac.uk 		19/2/84	        *
* 							        *
\***************************************************************/

#include "util.h"
#include "mmdf.h"

int childid;
int retval;
char linebuf[LINESIZE];

main ()
{
    int pid;

    while( fgets(linebuf, sizeof(linebuf), stdin) != NULL &&
    			linebuf[0] != '\n')
		/* do nothing */;
    if ((childid = fork ()) == 0)
    {
				/* hunt around for a printer program */
	execlp("lpr", "rcvlpr", (char *)0);
	execlp("opr", "rcvopr", (char *)0);
	perror ("rcvopr exec error");
	exit (-1);
    }
    if( childid == -1)
    	exit(RP_MECH);

    while ((pid = wait(&retval)) != childid)
	if (pid == -1)
	    exit(RP_AGN);
	
    exit ((retval&0377) ? RP_AGN : 0);
				  /* say "delivered" only if opr ends   */
				  /* cleanly                            */
}
