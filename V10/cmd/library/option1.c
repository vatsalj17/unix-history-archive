/*	library:option1.c	1.15	*/
#include "sccsid.h"
VERSION(@(#)library:option1.c	1.15)

/******************  option1.c  *****************************/

/* This routine accepts requests for documents known by number.
 * This includes:
 *	 TMs, SLs (and variants - GLs, ILs, etc), DLs, DSs,
 *	 technical reports, ctp items.
 *
 * In addition it includes two special classes of items:
 *
 *	Memos kept by date (or switching case numbers). For these this
 *	routine also prompts for the following info:
 *		title
 *		author: - this author is looked up for location of memos
 *
 *	Table of content items known by short number which has the form
 *			<letter><3 digits><optional letter>
 *
 *	For these this routine also prompts for a page number.
 */

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
char *malloc(), *realloc();

option1(logfile, argc, argv)
FILE *logfile;
int argc;
char *argv[];
{  
    int i, numreq=0;
    extern char *Cmd, *whereto, libcntl[], pan[], req_name[];
    long day, resp_size;
    char instr[512], done_flag=0, parm_flag=0, err_flag=0,
	    *dateptr, *ctime(), multi_flag, itemline[1000];
    register char *ptr;
    char  docid[256], *strptr, *tptr, *strchr();
    char *response;	/* this is the buffer where requests go */
    int can_remark;	/* flag to say whether remarks with this request */
    char *endptr, *eptr;
    FILE *fopen(), *helpfile, *popen(), *sendfile;

    /* get the date - note used only in the log */
    time(&day);
    dateptr = ctime(&day);
    /* get initial buffer for requests  */
    resp_size = 5000;
    if ((response = malloc(resp_size)) == NULL) {
	fprintf(stderr, "library: Couldn't get space for request (%d). No requests sent!\n", resp_size);
	exit(1);
	}
    *response = '\0';

    /* special handling in the case of parameters passed */
    if (argc) {
	parm_flag = 1;
	*itemline = '\0';
	for (i=0; i<argc; i++) {
	    if (*itemline) strcat(itemline, " ");
	    strcat(itemline, argv[i]);
	    }
	}

    eptr = response;
    while (!done_flag) {  /* loop until break when the user types . <return>  */
	/* check if getting low on space */
	if ((resp_size - strlen(response)) < 500) {
	    resp_size += 5000;
	    ptr = response;
	    if ((response = realloc(response, resp_size)) == NULL) {
		fprintf(stderr, "library: Couldn't get space for this much request (%d). No requests sent!\n", resp_size);
		exit(1);
		}
	    eptr += (response - ptr);
	    }
	/* now ready to get document id */
	if (err_flag || !parm_flag) {
	    printf("\nEnter item identifier(s) (. to exit):");
	    if (fgets(itemline, 1000, stdin) == NULL) break;
	    if (*itemline == '.') break;
	    if ((tptr = strchr(itemline, '\n')) != NULL) *tptr = '\0';
	    if (!strcmp(itemline, "exit") || !strcmp(itemline, "quit")) break;
	    if (strlen(itemline) == 0) {
		printf("Please enter IDs of items requested. ? for help.\n");
		continue;
		}
	    }
	if (*itemline == '~') {
	    fprintf(stderr, "Tilde escape do not work in option 1! Please reeenter requests\n");
	    continue;
	    }
	uppercase(itemline);
	/* check if help is desired */
	if ((strlen(itemline) == 1) && ((*itemline=='?')||(*itemline=='H')) ) {
	    sprintf(itemline, "%s/known.list", WHERE);
	    if ((helpfile = fopen(itemline, "r")) == NULL) {
		printf("Sorry. Help statement unavailable!\n");
		continue;
		}
	    while (fgets(itemline, BUFSIZ, helpfile) != NULL)
		if (*itemline == '?') printf("%s", itemline+1);
	    fclose(helpfile);
	    continue;
	    }

	/* remove trailing blanks */
	ptr = itemline + strlen(itemline) - 1;
	while ( (*ptr == ' ')  || (*ptr == '\t') )
		    ptr--;
	*++ptr = '\0';

	/* now break apart the input string to get ids */
	strptr = itemline;
	multi_flag = err_flag = 0;
	while ((strptr != NULL) && (strlen(strptr) > 0) ) {
	    /* Find start of the string */
	    ptr = strptr;
	    while (*ptr && (*ptr == ' ' || *ptr == '\t') ) ptr++;
	    if (*ptr == '\0') break;

	    /* find the end of this id - can be tricky */
	    endptr = strchr(ptr, ' ');
	    /* ptr is pointing to the id  */
	    if (endptr != NULL)
		*endptr++ = '\0';
	    strptr = endptr;
	    strcpy(docid, ptr);

	    if (strlen(docid) > 70) {
		printf("Document id %s too long. Reenter\n", docid);
		err_flag = 1;
		continue;
		}

	    /* we need to know if there is more than one i|em on this line */
	    if ( (strptr != NULL) && (*strptr != '\0') ) multi_flag = 1;

	    if ((can_remark = checkdoc(docid)) < 0) {
		/* error in id */
		err_flag = 1;
		printf("Incorrect id %s given\n", docid);
		continue;
		}
	    
	    /* Check in the logfile to see if this has been ordered */
	    if (logfile != NULL) {
		rewind(logfile);
		i = strlen(docid);
		while (fgets(eptr, 100, logfile) != NULL) 
		    if (!strncmp(eptr, docid, i) && (*(eptr+i) == '\n')) {
			/* Make sure pointer at end, for write later */
			fseek(logfile, 0L, 2);
			printf("Document ID %s already exists in your log file.\n", docid);
			printf("Do you wish to request it again (type y if yes): ");
			if ((fgets(eptr, 100, stdin) == NULL) || ((*eptr != 'y') && (*eptr != 'Y')) )
				i = 0;
			break;
			} /* end deling with already ordered */
		if (!i) continue; /* i is flag, only reorder if set to 0 */
		} /* end of check in logfile */

	    sprintf(eptr, "**-**\n%s\n", docid);
	    eptr += strlen(eptr);
	    if (can_remark && !parm_flag && !multi_flag) {
		printf("Enter any additional remarks. End with a blank line.\n*");
		while (strlen(fgets(eptr, 200, stdin)) > 1) {
		    if (*eptr == '.') {
			*eptr = '\0';
			done_flag = 1;
			break;
			}
		    printf("*");
		    eptr += strlen(eptr);
		    }
		} /* end of remark getting */

	    /* if first put on header */
	    numreq++;
	    } /* end of parts of entered string handling loop */
	if (parm_flag && !err_flag) break;
	} /* end of main request getting loop */

    /* output the record */
    signal(SIGINT, SIG_IGN);
    if (numreq == 0) {
	printf("No requests sent!\n");
	return;
	}
#ifdef MSDOS
    if ((sendfile = fopen("\\pipe.tmp", "w")) == NULL) {
#else
    sprintf(instr, "%s %s", Cmd, whereto);
    if ((sendfile = popen(instr, "w")) == NULL) {
#endif
	fprintf(stderr, "Couldn't open mail pipe to send requests! No requests sent\n");
	exit(1);
	}
    fprintf(sendfile, "* *#KNOWN-%s\n%s\n%s\n", libcntl, pan, req_name);
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
	ptr = response;
	while (ptr != NULL) {
	    ptr = strchr(ptr, '\n')+1; /* skip **-** line */
	    if (strlen(ptr) < 5) break;
	    fprintf(logfile, "From %s %sSubject: option 1 request\n",
			req_name, dateptr);
	    tptr = ptr;
	    while ((tptr = strchr(tptr, '\n')) != NULL)
		if (strncmp(++tptr, "**-**", 5) == 0) break;
	    if (tptr != NULL) *tptr++ = '\0';
	    fputs(ptr, logfile);
	    fputc('\n', logfile);
	    ptr = tptr;
	    }
	fclose(logfile);
	}
    if (numreq == 1)
	fprintf(stdout, "Your request has been sent\n");
    else
	fprintf(stdout, "Your %d requests have been sent\n", numreq);
    return;
    }

