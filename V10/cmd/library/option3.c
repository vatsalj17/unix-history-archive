/*	library:option3.c	1.10	*/
#include "sccsid.h"
VERSION(@(#)library:option3.c	1.10)

/******************  option3.c  *****************************/

/* This routine accepts bulletin subscriptions.
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <signal.h>

option3(logfile, argc, argv)
FILE *logfile;
int argc;
char *argv[];
{  
    int firsttime = 1;  /* initialize to indicate first time through  */
    int i, numreq=0;
    long day;
    char *dateptr, *ctime(), *ptr;
    char  instr[512], response[5000], *strchr();
    FILE *helpfile, *sendfile, *popen();
    extern char *Cmd, *whereto, libcntl[], pan[], req_name[];

    /* get the date - note used only in the log */
    time(&day);
    dateptr = ctime(&day);

    while (1) {  /* continue until user is done */
	if (!argc) {
	    if (firsttime) {
		printf("Enter your subscriptions in the form:\n (un)subscribe to <bulletin name>\n");
		firsttime = 0;
		}
	    /* get the subscription */
	    printf("Enter subscription");
	    printf(" (exit with a period):");
	    if (fgets(instr, 512, stdin) == NULL) break;
	    if (*instr == '.') break;
	    if ((ptr = strchr(instr, '\n')) != NULL) *ptr = '\0';
	    }
	/* special handling in the case of parameters passed */
	else {
	    *instr = '\0';
	    for (i=0; i<argc; i++) {
		if (*instr) strcat(instr, " ");
		strcat(instr, argv[i]);
		}
	    }
	uppercase(instr);
	/* check for help request */
	if ((*instr == '?') || (*instr == 'H')) {
	    sprintf(instr, "%s/option3.help", WHERE);
	    if ((helpfile = fopen(instr, "r")) == NULL) {
		printf("Sorry. Help statement unavailable!\n");
		continue;
		}
	    while (fgets(instr, 250, helpfile) != NULL)
		    fputs(instr, stdout);
	    fclose(helpfile);
	    if (argc) break;
	    continue;
	    }
	if (strlen(instr) > 0) {
	    if ((strncmp(instr, "UNSUB", 5) != 0) && (strncmp(instr, "SUB", 3) != 0) ) {
		    printf("Illegal entry - a subscription or unsubscription is required.\n");
		    firsttime=1;
		    if (argc) break;
		    continue;
		    }
	    strcat(response, instr);
	    strcat(response, "\n");
	    numreq++;
	    }

	if (argc) break;
	} /* end of subscription getting loop */
	
    if (!numreq) {
	    printf("No subscription entered!\n");
	    return;
	    }
    /* output the record */
    signal(SIGINT, SIG_IGN);
#ifdef MSDOS
    if ((sendfile = fopen("\\pipe.tmp", "w")) == NULL) {
#else
    sprintf(instr, "%s %s", Cmd, whereto);
    if ((sendfile = popen(instr, "w")) == NULL) {
#endif
	fprintf(stderr, "Couldn't open mail pipe to send requests! No requests sent\n");
	exit(1);
	}
    fprintf(sendfile, "* *#SUB-%s\n%s\n%s\n**-**\n", libcntl, pan, req_name);
    fputs(response, sendfile);
#ifdef MSDOS
    fclose(sendfile);
    sprintf(instr, "%s -f \\pipe.tmp -slibRequest %s", Cmd, whereto);
    system(instr);
    unlink("\\pipe.tmp");
#else
    pclose(sendfile);
#endif
    if (logfile != NULL) {
	fprintf(logfile, "From %s %sSubject: Subscription request\n",
		    req_name, dateptr);
	fputs(response, logfile);
	fputc('\n', logfile);
	fclose(logfile);
	}
    fprintf(stdout, "Your %d subscription requests have been sent\n", numreq);
    return;
    }

