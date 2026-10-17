/*	library:option2.c	1.30	*/
#include "sccsid.h"
VERSION(@(#)library:option2.c	1.30)

/*****************   option2.c   *************/

/* this package handles option 2 of library. Basically this
 * option consists of requests that can be formatted as forms
 * to be filled in.  The forms all have the name 2.*.
 * Every form must have a dashed line in it (a line starting with at least 10
 * dashes) which separates control info from the actual form part.
 *  Some conventions:
 *     -  lines starting with a # are comments - above and below the dash line.
 *     -  If one of the first two lines starts with a P (capital p) this is
 *        what the user is shown to select from. Otherwise the file name.
 *     - lines starting with a ? above the dash line are general comments.
 *     - lines with a ? below the dash line are help for the previous query.
 *
 *  Each line ends with either
 *	*number  saying a response is optional. max number lines is number.
 *	+number  saying a response is manditory. max number lines is number.
 */
#include "gnamef.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
char *malloc(), *realloc();
char *ctime(), *getenv();
void put_respond(), put_lib_file();

option2(logfile, argc, argv, optnum)
FILE *logfile;
int argc;
char *argv[];
int optnum;
 {
    char *response, filename[80], hold_where[100], what_avail[50],
	     format[150], tempname[60], instr[150], templine[200], query[80];
    char *ptr, *eptr, *tptr, tchar, tchar2, *endptr, this_char, *dateptr,
	 *clptr, *beginptr, *editor, *tptr2;
    char *libptr, *get_lib_file(), *find_respond();
    extern char *Cmd, *whereto, libcntl[], pan[], req_name[], libfile[];
    int i, j, k, ans, avail, num, numreq, resp_size, first, max, nolog,
	multi_req, nodata, level, said_blank,
	save_respond, respond_flag, use_respond;
    int used_arg;	/* flag that arguments used in selection */
#ifdef SVR4
    time_t today;
#else
    long today;
#endif
    FILE *tfile, *popen(), *efile, *sendfile;
    DIR *tdir;

    time(&today);
    used_arg = respond_flag = level = 0;
    strcpy(format, "Your %d requests have been sent\n");
    dateptr = ctime(&today);
    /* get initial space for responses */
    nolog = said_blank = first = 0;
    resp_size = 5000;
    if ((response = malloc(resp_size)) == NULL) {
	fprintf(stderr, "library: Couldn't get space for request (%d). No requests sent!\n", resp_size);
	exit(1);
	}
    /* first count what is available - If only one say nothing */
    tdir = opendir(WHERE);
    avail = 0;
    ptr = response;
    while (gnamef(tdir, ptr) == TRUE) 
	if ((atoi(ptr) == optnum) && (*(ptr+1) == '.')) {
	    avail++;
	    ptr += strlen(ptr) + 1;
	    }
    closedir(tdir);
    if (avail == 0) {
	fprintf(stderr, "Nothing available via this option! No requests sent\n");
	exit(1);
	}
    /* see if they already said what they want */
    if (argc) {
	tptr = argv[0];
	if ((*tptr != '-') || ! isalnum(*(tptr+1)))  {
	    fprintf(stderr, "Confused arguments for option %d (%s)\?\? No requests sent\n", optnum, tptr);
	    exit(1);
	    }
	ans = *(tptr+1);
	argc--;
	argv++;
	}
    else ans = 0;
    eptr = ptr;
    if (avail == 1) ans = -1;
    else {
	if (ans == 0) {
	    printf("The following types of material are available.\n");
	    printf("For more details on a given type do  <option number> ?.  E.g. p?\n\n");
	    }
	/* use the file names - or D line in the library directory */
	ptr = response;
	for (this_char='1', j=1; j <= avail; j++, ptr += strlen(ptr) + 1) {
	    /* check if they say special words what to call it */
	    sprintf(eptr, "%s/%s", WHERE, ptr);
	    if ((tfile = fopen(eptr, "r")) == NULL) {
		fprintf(stderr, "Can't read option file for %s\n", ptr+2);
		continue;
		}
	    /* scan to dash line for D line */
	    while (fgets(eptr, 150, tfile) != NULL) {
		if (strncmp(eptr, "-----", 5) == 0) break;
		if (*eptr == 'D')  break;
		}
	    tptr = strchr(eptr, '\n');
	    fclose(tfile);
	    if (tptr != NULL) *tptr = '\0';
	    tptr = eptr + 1;
	    while ((*tptr == ' ') || (*tptr == '\t')) tptr++;
	    if ((*eptr != 'D') || ! *tptr) {
		printf("  %c) %s\n\n", this_char, ptr+2);
		if (ans == 0) what_avail[j-1] = this_char++;
		continue;
		}
	    if ((*(tptr+1) == '\0') || (*(tptr+1) == ' ') || (*(tptr+1) == '\t')) {
		what_avail[j-1] = *tptr;
		tptr++;
		while ((*tptr == ' ') || (*tptr == '\t')) tptr++;
		}
	    else {
		what_avail[j-1] = this_char++;
		}
	    if (ans != 0) continue;
	    if (!strlen(tptr)) 
		printf("  %c) %s\n\n", what_avail[j-1], ptr+2);
	    else
		printf("  %c) %s\n\n", what_avail[j-1], tptr);
	    }
	} /* display of available options */
    what_avail[j-1] = 0;
    if (ans == -1) ans = 1;
    else if (ans > 0) {
	for (i=0; i < avail; i++)
	    if (ans == what_avail[i]) break;
	i++;
	if ((i < 1) || (i > avail)) {
	    fprintf(stderr, "Illegal option selected (%c). available are [%s]\n",
			ans, what_avail);
	    exit(1);
	    }
	ans = i;
	}
    while (ans == 0) {
	printf("Type desired: ");
	if ((fgets(eptr, 80, stdin) == NULL) || (*eptr == '.')) {
	    fprintf(stderr, "No requests sent!!\n");
	    exit(1);
	    }
	if ((tptr = strchr(eptr, '\n')) != NULL) *tptr = '\0';
	if (!strlen(eptr)) {
	    printf("Please select from available options, . to exit, ? for help\n");
	    continue;
	    }
	ptr = eptr + 1;
	while ((*ptr == ' ') || (*ptr == '\t')) ptr++;
	if ((*eptr == '?') && !isalnum(*ptr)) {
	    sprintf(eptr, "%s/option%d.help", WHERE, optnum);
	    if ((tfile = fopen(eptr, "r")) == NULL) {
		printf("Sorry. General help on option %d not on this machine!\n",
			    optnum);
		continue;
		}
	    while (fgets(instr, 150, tfile) != NULL)
		    fputs(instr, stdout);
	    fclose(tfile);
	    continue;
	    }
	if (*eptr == '?') {
	    tptr = ptr;
	    ptr = eptr;
	    eptr = tptr;
	    }
	for (i=0; i < avail; i++)
	    if (*eptr == what_avail[i]) break;
	i++;
	if ((i < 1) || (i > avail)) {
	    fprintf(stderr, "Illegal option. Must be from list above (%s)\n", what_avail);
	    continue;
	    }
	/* see if they want help */
	if (*ptr == '?') {
	    ptr = response;
	    for (j = 1; j < i; j++) ptr += strlen(ptr) + 1;
	    sprintf(eptr, "%s/%s", WHERE, ptr);
	    if ((tfile = fopen(eptr, "r")) == NULL) {
		fprintf(stderr, "Can't read option file for %s\n", ptr+2);
		continue;
		}
	    j = 0;
	    while (fgets(eptr, 150, tfile) != NULL) {
		if (strncmp(eptr, "-----", 5) == 0) break;
		if (*eptr == '?') {
		    fputs(eptr+1, stdout);
		    j++;
		    }
		}
	    fclose(tfile);
	    if (!j) printf("Sorry - no help available about %s\n", ptr+2);
	    continue;
	    } /* end giving of help */
	ans = i;
	} /* end more than one thype thing to choose from */
    ptr = eptr = response;
    numreq = 0;
    for (j = 1; j < ans; j++) ptr += strlen(ptr) + 1;
    sprintf(filename, "%s/%s", WHERE, ptr);
    if ((tfile = fopen(filename, "r")) == NULL) {
	fprintf(stderr, "library: Can't read selected option file for %s\n",
				ptr+2);
	exit(1);
	}
    /* we may need the base of filename for saved responses below */
    strcpy(filename, ptr);

    nodata = 0;
    multi_req = 1;
    while (1) {
	endptr = eptr;
	rewind(tfile);
	while (fgets(eptr, 150, tfile) != NULL) {
	    if (strncmp(eptr, "-----", 5) == 0) break;
	    /* all set up already */
	    if (numreq) continue;
	    if (*eptr == 'S') {
		eptr++;
		while (*eptr == ' ' || (*eptr == '\t')) eptr++;
		strcpy(response, eptr);
		eptr = response + strlen(response); 
		}
	    else if (strncmp(eptr, "NODATA", 6) == 0) {
		nodata = 1;
		}
	    else if (strncmp(eptr, "FORMAT", 6) == 0) {
		tptr = eptr + 6;
		while (*tptr && isspace(*tptr)) tptr++;
		if (*tptr) strcpy(format, tptr);
		}
	    else if (strncmp(eptr, "NOBLANKMSG", 10) == 0) {
		said_blank = -1;
		}
	    else if (strncmp(eptr, "NOLOG", 5) == 0) {
		nolog = 1;
		}
	    else if (strncmp(eptr, "LEVEL", 5) == 0) {
		level = 1;
		}
	    else if (strncmp(eptr, "ONE", 3) == 0) {
		multi_req = 0;
		}
	    else if (*eptr == 'M') {
		tptr = eptr+1;
		while (*tptr == ' ' || (*tptr == '\t')) tptr++;
		if (strlen(tptr) > sizeof(hold_where)) continue;
		strcpy(hold_where, tptr);
		ptr = hold_where + strlen(hold_where);
		while (!isalnum(*--ptr));
		*++ptr = '\0';
		if (strlen(hold_where)) whereto = hold_where;
		}
	    else if (!first && (*eptr == 'H') && !argc) {
		fputs(eptr+1, stdout);
		}
	    }
	if (strncmp(eptr, "-----", 5) != 0) {
	    fprintf(stderr, "library: No dash list for %s option form! No requests sent\n", ptr + 2);
	    exit(1);
	    }
	if ((eptr == endptr) && !numreq) {
	    fprintf(stderr, "library: No Send line for this option! No requests sent\n");
	    exit(1);
	    }
	/* don't just duplicate this if user wiped out last being entered */
	if ((eptr < (response + 10)) || strncmp(eptr-6, "**-**", 5)) {
		    strcpy(eptr, "**-**\n");
		    eptr += strlen(eptr);
		    }
	endptr = eptr;
	if ((said_blank == 0) && !argc) {
	    fprintf(stdout, "Use blank lines to end multi-line inputs\n");
	    said_blank++;
	    }
	first++;
	/* now read in an utilize the form */
	while (fgets(eptr, 150, tfile) != NULL) {
	    /* Level handling */
	    if (level && (*eptr == '@')) {
		if ((*(eptr+1)-'0') != level) continue;
		strcpy(eptr, eptr+2);
		}
	    /* skip comment and help lines */
	    if ((*eptr == '#') || (*eptr == '?')) continue;
	    /* check if getting low on space */
	    if ((resp_size - strlen(response)) < 500) {
		resp_size += 5000;
		ptr = response;
		if ((response = realloc(response, resp_size)) == NULL) {
		    fprintf(stderr, "library: Couldn't get space for this much request (%d). No requests sent!\n", resp_size);
		    exit(1);
		    }
		eptr += (response - ptr);
		endptr += (response - ptr);
		}
	    /* find how much the user can respond  */
	    ptr = eptr + strlen(eptr) - 1;
	    if (*ptr == '\n') *ptr-- = '\0';
	    while (isdigit(*ptr) && (ptr > eptr)) ptr--;
	    num = atoi(ptr+1);
	    save_respond = 0;
	    if (*ptr == 'S') {
		if (respond_flag == 0) {
		    libptr = get_lib_file(libfile);
		    if (libptr == NULL)
			respond_flag = -1;
		    else
			respond_flag = 1;
		    }
		if ((respond_flag > 0) && (num == 1)) save_respond = 1;
		ptr--;
		}
	    if ((*ptr != '+') && (*ptr != '*')) {
		if (!argc) printf("%s\n", eptr);
		continue;
		}
	    tchar = *ptr--;
	    while ((*ptr == ' ') || (*ptr == '\t')) ptr--;
	    *++ptr = '\0';
	    if (num == 0) num = 1;
	    /* save the query in case we need it later */
	    strcpy(query, eptr);
	    /* now get in a maximum of num lines */
	    strcat(eptr, ": ");
	    if (strlen(eptr) > 30) strcat(eptr, "\n\t");
	    beginptr = ptr = eptr + strlen(eptr);
	    j = 0;  /* use j to count the lines */
	    max = num;
	    use_respond = 0;
	    while (num-- > 0) {
		/* check if saved response to this */
		tptr = NULL;
		if (!j && !argc && save_respond) {
		    /* remove () expressions off query */
		    tptr = query + strlen(query) - 1;
		    if (*tptr == ')') {
			tptr = strrchr(query, '(');
			if (tptr != NULL) {
			    while ((*tptr == ' ') || (*tptr == '\t') || (*tptr == '(')) tptr--;
			    *++tptr = '\0';
			    }
			}
		    tptr = find_respond(libptr, filename, query);
		    if (tptr != NULL) {
			printf("\nUse response \"%s\" for \"%s\"?\n     Enter y (or return) if okay, n to give new response: ",
					tptr, query);
			if (fgets(ptr, 80, stdin) == NULL) {
			    fprintf(stderr, "Input ended in middle? No requests sent\n");
			    exit(1);
			    }
			if ((clptr = strchr(ptr, '\n')) != NULL) *clptr = '\0';
			if ((*ptr == 'y') || (*ptr == 'Y') || !strlen(ptr)) {
			    use_respond = 1;
			    strcpy(ptr, tptr);
			    num = 0;
			    }
			else if (*ptr == '.') {
			    use_respond = 1;
			    strcpy(ptr, ".");
			    }
			else
			    *ptr = '\0';
			} /* end of response found in file */
		    } /* deal with saved response */
		if (!use_respond && !j && !argc) {
		    printf("Enter %s", eptr);
		    if (max >1)
			printf("(Allowed a maximum %d lines)\n==> ", max);
		    else if (strlen(eptr) > 30)
			printf("\n==> ");
		    }
		else if (!argc && !use_respond) printf("==> ");
		if (argc) {
		    tptr = argv[0];
		    if (*tptr == '-') tptr++;
		    strcpy(ptr, tptr);
		    argc--;
		    argv++;
		    used_arg = 1;
		    }
		else if (!use_respond && (fgets(ptr,500, stdin) == NULL)) {
		    fprintf(stderr, "Input ended in middle? No requests sent\n");
		    exit(1);
		    }
		if ((clptr = strchr(ptr, '\n')) != NULL) *clptr = '\0';
		if (*ptr == '?') {
		    /* give any help lines - make it so we can get back */
		    i = ftell(tfile);
		    k = 0;
		    while (fgets(templine, 200, tfile) != NULL) {
			tptr = templine;
			if (*tptr == '#') continue;
			/* Level handling */
			if (level && (*tptr == '@')) {
			    if ((*(tptr+1)-'0') != level) continue;
			    tptr += 2;
			    }
			if (*tptr == '#') continue;
			if (*tptr == '?') {
			    if ((*(tptr+1) == '!') || (*(tptr+1) == '@'))
				    fputs(tptr+2, stdout);
			    else
				fputs(tptr+1, stdout);
			    k++;
			    }
			else break;
			}
		    if (!k) printf("Sorry - no help for this prompt!\n");
		    fseek(tfile, (long)i, 0);
		    num++;
		    *ptr = '\0';
		    continue;
		    }
		else if (*ptr == '.') {
		    printf("A period wipes out a request. Use return (a blank line) to end input.\n");
		    printf("  Wipe out request? (n): ");
		    if (fgets(ptr, 80, stdin) == NULL) {
			fprintf(stderr, "Input ended in middle? No requests sent\n");
			exit(1);
			}
		    if ((*ptr == 'y') || (*ptr == 'Y')) {
			j = -1;
			*eptr = '\0';
			break;
			}
		    *ptr = '\0';
		    num++;
		    continue;
		    }
		else if (strncmp(ptr, "~e", 2) == 0) {
		    if (max < 20) {
			fprintf(stderr, "Sorry - library() doesn't allow editer use on responses of %d lines\n", max);
			*ptr = '\0';
			num++;
			continue;
			}
		    editor = getenv("EDITOR");
		    if (editor == NULL) editor = "ed";
		    *ptr = '\0';
		    sprintf(tempname, "/tmp/Lo25.%d", getpid());
		    if ((efile = fopen(tempname, "w")) == NULL) {
			fprintf(stderr, "Couldn't open tmp file %s for edit\n", tempname);
			*ptr = '\0';
			num++;
			continue;
			}
		    if (strlen(beginptr)) {
			/* write out what is there - clean up start of lines */
			tptr = beginptr;
			while ((tptr != NULL) && *tptr) {
			    while (*tptr && (*tptr == ' ') || (*tptr == '\t')) tptr++;
			    tptr2 = strchr(tptr, '\n');
			    if (tptr2 != NULL) *tptr2++ = '\0';
			    fputs(tptr, efile);
			    fputc('\n', efile);
			    tptr = tptr2;
			    }
			}
		    fclose(efile);
		    sprintf(ptr+1, "%s %s", editor, tempname);
		    system(ptr+1);
		    if ((efile = fopen(tempname, "r")) == NULL) {
			fprintf(stderr, "Couldn't open tmp file %s after edit\n", tempname);
			num++;
			*ptr = '\0';
			continue;
			}
		    ptr = beginptr;
		    while (fgets(ptr, 1000, efile) != NULL) {
				ptr += strlen(ptr);
				*ptr++ = '\t';
				j++;
				}
		    *ptr = '\0';
		    fclose(efile);
		    unlink(tempname);
		    printf("(Continue entry)\n");
		    continue;
		    }
		else if (strncmp(ptr, "~r", 2) == 0) {
		    if (max < 20) {
			fprintf(stderr, "Sorry - library() doesn't allow file reading on responses of %d lines\n", max);
			*ptr = '\0';
			num++;
			continue;
			}
		    tptr = ptr;
		    ptr += 2;
		    while ((*ptr == ' ') || (*ptr == '\t')) ptr++;
		    if ((efile = fopen(ptr, "r")) == NULL) {
			fprintf(stderr, "Couldn't open file %s for reading\n", ptr);
			num++;
			ptr = tptr;
			*ptr = '\0';
			continue;
			}
		    ptr = tptr;
		    while (fgets(ptr, 1000, efile) != NULL) {
			ptr += strlen(ptr);
			/* check if getting low on space */
			if ((resp_size - strlen(response)) < 500) {
			    resp_size += 5000;
			    tptr = response;
			    if ((response = realloc(response, resp_size)) == NULL) {
				fprintf(stderr, "library: Couldn't get space for this much request (%d). No requests sent!\n", resp_size);
				exit(1);
				}
			    ptr += (response - tptr);
			    eptr += (response - tptr);
			    endptr += (response - tptr);
			    }
			*ptr++ = '\t';
			j++;
			}
		    *ptr = '\0';
		    fclose(efile);
		    printf("(Continue entry)\n");
		    continue;
		    }
		if (!strlen(ptr)) {
		    if (!j && tchar == '+') {
			if (argc) {
			    printf("Attempted to skip required input. Parameter handling stopped!\n");
			    argc = 0;
			    }
			printf("This input is required. Please enter a question mark (?) for help\n");
			num++;
			continue;
			}
		    break;
		    }
#ifndef MSDOS
		/* User did some input here - check if allowed is restricted */
		i = ftell(tfile);
		k = 0;
		while (fgets(templine, 200, tfile) != NULL) {
		    tptr = templine;
		    if (*(tptr) == '#') continue;
		    /* Level handling */
		    if (level && (*tptr == '@')) {
			if ((*(tptr+1)-'0') != level) continue;
			tptr += 2;
			}
		    if (*(tptr) != '?') break;
		    tchar2 = *(tptr+1);
		    if ((tchar2 != '!') && (tchar2 != '@')) continue;
		    k = -1;  /* flag that have option list */
		    tptr += 2;
		    while (*tptr == ' ') tptr++;
		    tptr2 = strchr(tptr, '\n');
		    if (tptr2 != NULL) *tptr2 = '\0';
		    tptr2 = strchr(tptr, '\t');
		    if (tptr2 != NULL) *tptr2++ = '\0';
		    if (strcmp(ptr, tptr) == 0) {
			k = 1;
			if (tchar2 == '@') k++;
			if (tptr2 != NULL) {
			    strcpy(ptr, tptr2);
			    if (!argc) printf("Requested: %s\n", ptr);
			    }
			break;
			}
		    }
		
		/* see if just jumping a level */
		if (k == 2) {
		    level++;
		    *ptr = '\0';
		    continue;
		    }
		fseek(tfile, (long)i, 0);
		if (k < 0) {
		    printf("Request must be from those listed. Do a question mark (?) for list\n");
		    num++;
		    *ptr = '\0';
		    continue;
		    }
#endif  /* Got rid of all this cause DOS rouble */
		/* have real result at ptr */
		ptr += strlen(ptr);
		strcpy(ptr, "\n\t");
		ptr += 2;
		j++;
		if (level) level = 1;
		}
	    if (j > 0) {
		if (save_respond && !use_respond) {
		    tptr = strchr(eptr, ':');
		    if (tptr++ != NULL) {
			while ((*tptr == ' ') || (*tptr == '\t') || (*tptr == '\n')) tptr++;
			put_respond(libptr, filename, query, tptr);
			respond_flag++;
			}
		    }
		*--ptr = '\0';
		eptr += strlen(eptr);
		}
	    else if (j < 0) {
		if (eptr != endptr) {
		    fprintf(stderr, "Request being entered is removed!\n");
		    eptr = endptr;
		    j = -5;
		    }
		break;
		}
	    } /* end of dealing with this form */
	/* See if we need to loop */
	if (j >= 0) {
	    if ((eptr == endptr) && !nodata)
		fprintf(stderr, "Request without data skipped!\n");
	    else
		numreq++;
	    if (!multi_req) break;
	    }
	/* If they invoked with arguments, no pause here */
	if (used_arg) break;
	printf("\nHave entered %d request%s. Enter another (n/y): ",
			numreq, ((numreq != 1) ? "s" : ""));
	if (fgets(instr, 80, stdin) == NULL) break;
	if ((clptr = strchr(instr, '\n')) != NULL) *clptr = '\0';
	if ((*instr != 'y') && (*instr != 'Y')) break;
	} /* loop for multiple requests */
    *eptr = '\0';
    fclose(tfile);
    /* output the record */
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
    fprintf(sendfile, "* *#OPTION2/5-%s\n%s\n%s\n", libcntl, pan, req_name);
    fputs(response, sendfile);
#ifdef MSDOS
    fclose(sendfile);
    sprintf(instr, "%s -f \\pipe.tmp -slibRequest %s", Cmd, whereto);
    system(instr);
    unlink("\\pipe.tmp");
#else
    pclose(sendfile);
#endif
    if (logfile != NULL && !nolog) {
	ptr = strchr(response, '\n')+1; /* skip send info */
	while (ptr != NULL) {
	    ptr = strchr(ptr, '\n')+1; /* skip **-** line */
	    if (strlen(ptr) < 5) break;
	    fprintf(logfile, "From %s %sSubject: option %d request\n",
			req_name, dateptr, optnum);
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
    fprintf(stdout, format, numreq);
    /* save new responses if any */
    if (respond_flag > 1) put_lib_file(libfile, libptr);
    return;
    }

char *get_lib_file(filename)
char *filename;
 {
    char *ptr, *ptr2;
    FILE *file;
    struct stat stbuf;
    int st_rtn;

    /* get the displays if there are any */
    st_rtn = stat(filename, &stbuf); 
    if (st_rtn != 0) stbuf.st_size = 10;
    ptr = malloc(stbuf.st_size + 2000);
    if (ptr == NULL) {
	fprintf(stderr, "library: Couldn't get system space for buffer!\n");
	exit(1);
	}
    /* leave emptr char before just to simplify stuff later */
    *ptr++ = '\0';
    *ptr = '\0';
    if (st_rtn) return(ptr);
    if ((file = fopen(filename, "r")) == NULL) return(ptr);
    ptr2 = ptr;
    while (fgets(ptr2, 1000, file) != NULL) 
	ptr2 += strlen(ptr2);
    *ptr2++ = '\0';
    *ptr2 = '\0';
    fclose(file);
    return(ptr);
    }

void put_lib_file(filename, ptr)
char *filename;	/* the name of the lib prompt file */
char *ptr;
 {
    FILE *file;

    if ((file = fopen(filename, "w")) != NULL) {
	fputs(ptr, file);
	fclose(file);
	}
    return;
    }

char *find_respond(ptr, prefix, lookfor)
register char *ptr;
char *prefix;
char *lookfor;
 {
    static char buf[200];
    int len;
    char *ptr2;

    /* don't even attempt on long strings */
    if ((strlen(prefix)+strlen(lookfor)) >= 200) return(NULL);
    sprintf(buf, "%s/%s", prefix, lookfor);
    ptr2 = buf + strlen(buf) - 1;
    while ((*ptr2 == '\t') || (*ptr2 == '\n') || (*ptr2 == ' ')) ptr2--;
    if (*ptr2 != ':') *++ptr2 = ':';
    *++ptr2 = '\0';
    len = strlen(buf);

    while (*ptr) {
	if (!strncmp(ptr, buf, len)) {
	    ptr += len;  /* point it after the : */
	    while ((*ptr == ' ') || (*ptr == '\t')) ptr++;
	    ptr2 = strchr(ptr, '\n');
	    if (ptr2 != NULL) *ptr2 = '\0';
	    if (strlen(ptr) > 200) {
		if (ptr2 != NULL) *ptr2 = '\n';
		return(NULL);
		}
	    strcpy(buf, ptr);
	    if (ptr2 != NULL) *ptr2 = '\n';
	    ptr2 = strchr(buf, '\n');
	    if (ptr2 != NULL) *ptr2 = '\0';
	    return(buf);
	    }
	ptr = strchr(ptr, '\n');
	if (ptr++ == NULL) return(NULL);
	}
    return(NULL);
    }

void put_respond(fullptr, prefix, newstuff, response)
register char *fullptr;
char *prefix;
char *newstuff;
char *response;
 {
    char buf[200];
    int len;
    register char *ptr2;

    /* don't even attempt on long strings */
    if ((strlen(prefix)+strlen(newstuff)) >= 200) return;

    /* remove any existing response */
    sprintf(buf, "%s/%s", prefix, newstuff);
    ptr2 = buf + strlen(buf) - 1;
    while ((*ptr2 == '\t') || (*ptr2 == '\n') || (*ptr2 == ' ')) ptr2--;
    if (*ptr2 != ':') *++ptr2 = ':';
    *++ptr2 = '\0';
    len = strlen(buf);

    while (*fullptr) {
	ptr2 = strchr(fullptr, '\n');
	if (ptr2 == NULL)
		ptr2 = fullptr + strlen(fullptr);
	else
		ptr2++;
	if (!strncmp(fullptr, buf, len)) {
	    strcpy(fullptr, ptr2);
	    break;
	    }
	fullptr = ptr2;
	}
    
    fullptr += strlen(fullptr);
    if (*(fullptr-1) != '\n') {
	*fullptr++ = '\n';
	*fullptr = '\0';
	}
    sprintf(fullptr, "%s %s\n", buf, response);
    fullptr = strchr(fullptr, '\n');
    if (fullptr != NULL) 
	*++fullptr = '\0';
    return;
    }
