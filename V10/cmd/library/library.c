/*	library:library.c	1.33	*/
#include "sccsid.h"
VERSION(@(#)library:library.c	1.33)

/******************  library.c  *****************************/

/* This routine accepts requests for any type of document
 * for which center 7811 and associated networks are responsible.
 *
 * Presently accepts requests for:
 *	document stored at ITDS (e.g. TMs, MFs, SLs)
 *	Bulletin items
 *	books
 *	photo copies of items
 *	(un)subscribe to bulletins
 */

#include "gnamef.h"
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
char *malloc(), *realloc();
#ifdef MSDOS
char *Cmd="sendfile";
#else
char *Cmd="/bin/mail ";
#endif
char *whereto="library!techlib";
int expert=0;
char libcntl[100], pan[15], req_name[80];
char libfile[120];	/* full path and name of .lib file */
char read_cntl[50];	/* User specified controls of reader function */
char *pager=NULL;	/* Pager to use in reading full things in opt7 */
void showavail();

static char *program="@(#) R Waldstein, Dept 78111, general purpose orderer";
static char *version="@(#) library, ordering program, version 3.6++";

#if  ( defined SVR4 ) || ( defined MSDOS )
void ordint(int junk)
#else
ordint()
#endif
 {
    printf("\nInterrupt! No requests sent\n");
    exit(0);
    }

main(argc, argv)
int argc;
char *argv[];
{  
    int len, reqint(), what;
    int got_name=0, got_id=0;
    char instr[250], copy_str[250], *tptr, *chrptr, *homeptr;
    char  *getenv(), logf_name[120], *strchr(), *ptr, *libptr;
    FILE *fopen(), *idfile, *tfile, *logfile, *helpfile;

    (void)signal(SIGINT, ordint);
    if ((argc == 2) && !strcmp(argv[1], "-v")) {
	    printf("%s\n", version);
	    exit(0);
	    }

    /* if they want let them enter info via a an id file */
    *pan = '\0';
    *req_name = '\0';
    *libcntl = '\0';
    *read_cntl = *logf_name = '\0';
    logfile = NULL;
    libptr = getenv("LIBFILE");
    homeptr = getenv("HOME");
#ifdef MSDOS
    if (libptr == NULL)
	strcpy(libfile, "c:/library/library.lib");
    else {
#endif
    if (libptr != NULL) {
	if (*libptr == '/')
	    strcpy(libfile, libptr);
	else {
	    strcpy(libfile, homeptr);
	    strcat(libfile, "/");
	    strcat(libfile, libptr);
	    }
	}
    else if (homeptr != NULL) {
	strcpy(libfile, homeptr);
	strcat(libfile, "/.lib");
	}
    else
	strcpy(libfile, ".lib");
#ifdef MSDOS
  }
#endif

    if ((idfile = fopen(libfile, "r")) == NULL) {
	/* let them use the old .id file */
	if ( homeptr != NULL) {
	    strcpy(instr, homeptr);
	    strcat(instr, "/.id");
	    }
	else
	    strcpy(instr, ".id");
	idfile = fopen(instr, "r");
	if (idfile != NULL) strcpy(libfile, instr);
	}

    if (idfile != NULL) {
	while (fgets(instr, 250, idfile) != NULL) {
	    if ((tptr = strchr(instr, '\n')) != NULL) *tptr = '\0';
	    strcpy(copy_str, instr);
	    uppercase(instr);
	    if  ((strncmp(instr, "NAME", 4) == 0) || (strncmp(instr, "LIBNAME", 7) == 0)) {
		tptr = instr + ((*instr == 'L') ? 7 : 4);
		while (!isalpha(*tptr) ) tptr++;
		if (strlen(tptr) > 79) {
		    fprintf(stderr, "library: Name <%s> from file unacceptably long! Max is 79 characters!\n", tptr);
		    exit(1);
		    }
		strcpy(req_name, tptr);
		}
	    else if ((strncmp(instr, "PAN", 3) == 0) || (strncmp(instr, "ID", 2) == 0)) {
		tptr = instr + ((*instr == 'P') ? 3 : 2);
		while (!isalnum(*tptr)) tptr++;
		if (strlen(tptr) > 14) {
		    fprintf(stderr, "library: ID <%s> from file unacceptably long! Max is 14 characters!\n", tptr);
		    exit(1);
		    }
		strcpy(pan, tptr);
		}
	    /* see if they want any libcntl info on the request */
	    else if ((strncmp(instr, "ITDSCNTL", 8) == 0) ||
		(strncmp(instr, "CNTL", 4) == 0) || (strncmp(instr, "LIBCNTL", 7) == 0) ) {
		    tptr = instr + ((*instr == 'C') ? 4 : 7);
		    if (*instr == 'I') tptr++;
		    while (!isalnum(*tptr) ) tptr++;
		    strcpy(libcntl, tptr);
		    }
	    /* see if they want any special reader control */
	    else if (strncmp(instr, "READER", 6) == 0) {
		    tptr = instr + 6;
		    while (!isalpha(*tptr) ) tptr++;
		    strncpy(read_cntl, tptr, 50);
		    }
	    /* see if they want any local cntl info for the session */
	    else if (strncmp(instr, "LIBLOCAL", 8) == 0) {
		    tptr = instr + 8;
		    while (!isalpha(*tptr) ) tptr++;
		    if ( strchr(tptr, 'F') != NULL) 
			fprintf(stderr, "Sorry - ask for fiche no longer supported!\n");
		    if ( strchr(tptr, 'X') != NULL) expert = 1;
		    }
	    /* see if they want an itds log */
	    else if ((strncmp(instr, "ITDSLOG", 7) == 0) || (strncmp(instr, "LIBLOG", 6) == 0)) {
		tptr = copy_str + ((*instr == 'L') ? 6 : 7);
		while ((*tptr != '/') && (*tptr != '.') && !isalnum(*tptr) )
			tptr++;
		if (*tptr != '/') {
		    strcpy(logf_name, homeptr);
		    strcat(logf_name, "/");
		    }
		strcat(logf_name, tptr);
		}
	    else if (!strncmp(instr, "PAGER", 5)) {
		tptr = copy_str + 5;
		while (*tptr && !ispunct(*tptr)) tptr++;
		if (*tptr) tptr++;
		while (isspace(*tptr)) tptr++;
		if (*tptr) {
		    if (*tptr == '"') {
			tptr++;
			ptr = tptr + strlen(tptr) - 1;
			if (*ptr == '"') *ptr = '\0';
			}
		    pager = malloc(strlen(tptr) + 1);
		    if (pager != NULL) strcpy(pager, tptr);
		    }
		}
	    }
	fclose(idfile);
	}
	    
    tptr = getenv("LIBNAME");
    if (tptr != NULL) strcpy(req_name, tptr);
    tptr = getenv("LIBID");
    if (tptr != NULL) strcpy(pan, tptr);
    tptr = getenv("LIBCNTL");
    if (tptr != NULL) strcpy(libcntl, tptr);
    tptr = getenv("LIBLOCAL");
    if (tptr != NULL) {
	if ( (strchr(tptr, 'f') != NULL) || (strchr(tptr, 'F') != NULL))
	    fprintf(stderr, "Sorry - ask for fiche no longer supported!\n");
	if ( (strchr(tptr, 'x') != NULL) || (strchr(tptr, 'X') != NULL))
			    expert = 1;
	}
    /* see if they want a log kept */
    tptr = getenv("LIBLOG");
    if (tptr != NULL) {
	    *logf_name = '\0';
	    if (*tptr != '/') {
		strcpy(logf_name, homeptr);
		strcat(logf_name, "/");
		}
	    strcat(logf_name, tptr);
	    }
    if (*logf_name != '\0') {
	/* careful about the \n on the end of the log file name */
	tptr = logf_name + strlen(logf_name) - 1;
	if (*tptr == '\n') *tptr = '\0';
	/* open up the log file. Note done here to assure that we can */
	if ( (logfile = fopen(logf_name, "a+")) == NULL) {
	    fprintf(stderr, "library: Problems opening log file %s\n", logf_name);
	    exit(1);
	    }
	}
    /* open a temp file for use for the requests */
    if (*pan == '\0' || *req_name == '\0') {
	printf("Welcome to the library network request service\n");
	printf("Use . <period> to exit, break to quit without sending requests.\n");
	}
    idfile = stdin;
    if (!isatty(fileno(stdin)) &&
	     ((*pan == '\0') || (*req_name == '\0') || (argc < 2))) {
	idfile = fopen("/dev/tty", "r");
	}
    while (*pan == '\0') {
	printf("\nEnter AT&T Identification Number (e.g. SS#): ");
	fflush(stdout);
	if (fgets(instr, 250, idfile) == NULL) {
	    fprintf(stderr, "Input ended?! Bye.\n");
	    exit(1);
	    }
	if ((tptr = strchr(instr, '\n')) != NULL) *tptr = '\0';
	len = strlen(instr);
	if ((*instr == '.') || !len) exit(1);
	if ((len<3) || (len > 14)) {
	    printf("ID must be 3 to 14 characters long. Reenter");
	    continue;
	    }
	strcpy(pan,instr);
	got_id = 1;
	}

    while (*req_name == '\0') {
	printf("Enter your last name (note LAST):");
	fflush(stdout);
	if (fgets(instr, 250, idfile) == NULL) {
	    fprintf(stderr, "Input ended?! Bye.\n");
	    exit(1);
	    }
	if ((tptr = strchr(instr, '\n')) != NULL) *tptr = '\0';
	if (*instr == '.') exit(1);
	tptr = instr;
	while (*tptr && !isalpha(*tptr) ) tptr++;
	if ((strlen(tptr) >= 2) && (strlen(tptr) < 79))
	    strcpy(req_name, tptr);
	else
	    printf("A name is required!. Reenter\n");
	got_name = 1;
	}

    /* Save it for them if new entry */
    while (got_name || got_id) {
	printf("Do you want this personnel data saved (in file $HOME/.lib). Enter y if yes: ");
	fflush(stdout);
	if (fgets(instr, 250, idfile) == NULL) {
	    fprintf(stderr, "Input ended?! Bye.\n");
	    exit(1);
	    }
	if ((*instr == 'y') || (*instr == 'Y')) {
	    if ((tfile = fopen(libfile, "a")) != NULL) {
		if (got_id)
		    fprintf(tfile, "id: %s\n", pan);
		if (got_name)
		    fprintf(tfile, "name: %s\n", req_name);
		fclose(tfile);
		chmod(libfile, 0600);
		}
	    break;
	    }
	else if (*instr == '?') {
	    printf("\n\
This feature allows you to save the personnel information prompted\n\
for above in a file. This will mean that in the future the library\n\
command will not ask again for this information, it will get the\n\
information from the file \"%s\". Do NOT do this if other\n\
people use this login - their orders would be placed in your name.\n\
For further information about this feature, or other questions\n\
about the library command call 908 582-4840. THank you.\n\n", libfile);
	    }
	else
	    break;
	}

    /* first see if they passed a parm saying what they wanted */
    argc--;
    argv++;
    if (argc > 0 && *argv[0] == '-') {
	uppercase(argv[0]);
	chrptr = argv[0] + 1;
	if ((*chrptr == 'C') || (*chrptr == '4'))
		    what = 4;
	else if ((*chrptr == 'K') || (*chrptr == '1'))
		    what = 1;
	else if ((*chrptr == 'P') || (*chrptr == '2'))
		    what = 2;
	else if ((*chrptr == 'S') || (*chrptr == '3'))
		    what = 3;
	else if ((*chrptr == 'R') || (*chrptr == '5'))
		    what = 5;
	else if ((*chrptr == 'I') || (*chrptr == '6'))
		    what = 6;
	else if ((*chrptr == 'B') || (*chrptr == '7'))
		    what = 7;
	else {
	    printf("Illegal parameter passed\n\tk (or 1) = order known item (by number)\n");
	    printf("\tp (or 2) = order unannounced items\n");
	    printf("\ts (or 3) = subscribe (or unsubscribe) to a bulletin\n");
	    printf("\tc (or 4) = AT&T Library Network database search\n");
	    printf("\tr (or 5) = request human assistance\n");
	    printf("\ti (or 6) = AT&T Library Network information/document\n");
	    printf("\tb (or 7) = Read AT&T Library Network Email transmissions\n");
	    exit(1);
	    }
	argc--;
	argv++;
	} /* end of specified type as parameter */
	    
    else { /* let them select what they want */
      showlist:
	printf("\n");
	printf("1)  Order by number - this includes nearly everything announced\n");
	printf("\tby the AT&T Library Network (e.g. TMs, bulletin items) or handled\n");
	printf("\tby the Engineering Information Services (e.g. J docs, CPSs).\n");
	printf("\n");
	printf("2)  Order item not announced by the AT&T Library Network. Available:\n");
	showavail("2.");
	printf("\n");
	printf("3)  Subscribe or unsubscribe to a bulletin (e.g. Mercury, CTP)\n");
	printf("\n");
	printf("4)  Submit a database search. Examples of available databases:\n");
	printf("\tbook catalog, internal documents, AT&T personnel, released papers\n");
	printf("\n");
	printf("5)  Request human assistance / interaction. Type(s) available:\n");
	showavail("5.");
	printf("\n");
	printf("6)  Request AT&T Library Network information/services. Examples:\n");
	printf("\tLINUS info, products/services descriptions, loan/reserve status\n");
	printf("\n");
	printf("7)  Read AT&T Library Network email transmissions\n");
	printf("\n");


	while (1) {
	    printf("Enter number from menu (? for help [? number for specific help]): ");
	    if (fgets(instr, 250, idfile) == NULL) exit(0);
	    if (*instr == 'v') {
		printf("%s\n", version);
		continue;
		}
	    if ((*instr == '.') || (*instr == 'q')) exit(0);
	    ptr = instr;
	    what = atoi(ptr);
	    if (what < 0 || what > 7) {
		fprintf(stderr, "Illegal option %d picked. Must be in range 1-6\n", what);
		continue;
		}
	    if (what) {
		ptr++;
		while (*ptr && (*ptr == ' ')) ptr++;
		}
	    if ( (toupper(*ptr) == 'H') || (*ptr == '?') ) {
		if (!what) {
		    ptr++;
		    while (*ptr && (*ptr == ' ')) ptr++;
		    what = atoi(ptr);
		    }
		if ((what > 0) && (what < 8)) {
		    sprintf(instr, "%s/option%d.help", WHERE, what);
		    if (access(instr, 04) != 0)
			sprintf(instr, "%s/help.option%d", WHERE, what);
		    }
		else
		    sprintf(instr, "%s/library.help", WHERE);
		if ((helpfile = fopen(instr, "r")) == NULL) {
		    printf("Sorry. Help statement unavailable!\n");
		    goto showlist;
		    }
		while (fgets(instr, 250, helpfile) != NULL)
			fputs(instr, stdout);
		fclose(helpfile);
		}
	    else if (!what)
		fprintf(stderr, "Sorry - don't understand your input <%s>\n", instr);
	    else 
		break;
	    }
	}

    switch (what) {
	case 1: option1(logfile, argc, argv);
		break;
	case 2: /* Option 2 is a request for an item */
		option2(logfile, argc, argv, 2);
		break;
	case 3: /* a bulletin subscription */
		option3(logfile, argc, argv);
		break;
	case 4: /* a search of the book catalog */
		option2(logfile, argc, argv, 4);
		break;
	case 5: /* option 5 is request for human assistance */
		option2(logfile, argc, argv, 5);
		break;
	case 6: /* a document request */
		option2(logfile, argc, argv, 6);
		break;
	case 7: /* read some bulletin stuff */
		reader(logfile, argc, argv);
		break;
	default: /* error  */
		fprintf(stderr, "Program error. Please call it in\n");
		exit(1);
	}

    /* Just come back in case we need to clean up anything */
    exit(0);
    }


void showavail(str)
char *str;
 {
    int i, j;
    char filename[60], instr[100], *tptr;
    FILE *tfile;
    DIR *tdir;

    printf("\t ");
    /* use the file names - or W line in the library directory */
    if ((tdir = opendir(WHERE)) == NULL) {
	printf("No %s directory!!\n", WHERE);
	return;
	}
    i = 0;
    j = 9; /* j keeps track of number of characters out */
    while (gnamef(tdir, filename) == TRUE) {
	if (strncmp(filename, str, strlen(str)) == 0) {
	    /* check if they say special words what to call it */
	    sprintf(instr, "%s/%s", WHERE, filename);
	    if ((tfile = fopen(instr, "r")) == NULL) continue;
	    if (i++ != 0) printf("; ");
	    if (fgets(instr, 100, tfile) == NULL) {
		fclose(tfile);
		continue;
		}
	    if (*instr == 'W') {
		tptr = instr+1;
		while ((*tptr == ' ') || (*tptr == '\t')) tptr++;
		strcpy(instr, tptr);
		tptr = instr;
		while (*tptr && (*tptr != '\n')) tptr++;
		if (*tptr) *tptr = '\0';
		if (!strlen(instr)) strcpy(instr, filename+2);
		}
	    else strcpy(instr, filename+2);
	    fclose(tfile);
	    if ((strlen(instr) + j) > 74) {
		printf("\n\t ");
		j = 9;
		}
	    printf("%s", instr);
	    j += strlen(instr) + 2; /* the +2 counts the semicolon put on */
	    }
	}
    if (!i) printf("Nothing");
    printf(".\n");
    closedir(tdir);
    return;
    }
