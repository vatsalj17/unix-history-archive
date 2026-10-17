/*	library:reader.c	1.35	*/
#include "sccsid.h"
VERSION(@(#)library:reader.c	1.35)

/******************   reader.c     ***********************/

/* This is crude general purpose reader of bulletins that
 * have been sent to users at a remote.
 */
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#define MAXITEM 100
char *getenv(), *malloc(), *strchr(), *strrchr(), *ctime();
char *next_item(), *show_line(), *full_display();
static int lines, cols;
static int have_seen[MAXITEM];	/* Flags whether read this announcement item */
static int item_items[MAXITEM];	/* COunt of items in each announcement */
static int total_ann=0;		/* Total number of announcements available */
static int next_read=0;		/* Next announcement being read */
static char ann_desc[60];	/* Description of thing being announced */
static char *rest_full=NULL;	/* ptr to rest of partially displayed record */
int int_hit = 0;	/* Interrupt hit flag */
char req_str[5000], *req_ptr=req_str;
int numreq = 0;	/* Number of requests entered */
extern char read_cntl[]; /* User specified controls of reader function */
extern char *pager;	/* Pager to use in reading full things in opt7 */
#if (defined SVR4) || (defined MSDOS)
FILE *popen (const char *command, const char *type);

void sig_catch(int junk)
#else

sig_catch()
#endif
 { 
	signal(SIGINT, sig_catch);
	if (int_hit) exit(1);
	int_hit = 1;
	return;
	}

void reader(logfile, argc, argv)
FILE *logfile;
int argc;
char *argv[];
 {
    extern char *Cmd, *whereto, libcntl[], pan[], req_name[];
    int see_head, i;
    int tmp_used = 0;		/* Flag that temp file in use as mailbox */
    int skip_top;		/* Whether the user wants to skip top menu */
    long item_ptr[MAXITEM], end_ptr[MAXITEM];
    long today;
    char *ptr, *tptr, line[120], mailbox[150];
    FILE *infile, *outfile, *helpfile;

    time(&today);
    if ((ptr = getenv("LINES")) == NULL) 
	lines = 24 - 1;
    else {
	lines = atoi(ptr) - 1;
	if (lines < 5) {
	    fprintf(stderr, "library: Can't read bulletins in a window with this few lines (%d)\n", lines);
	    exit(1);
	    }
	}
    if ((ptr = getenv("COLUMNS")) == NULL) 
	cols = 80;
    else {
	cols = atoi(ptr);
	if (cols < 30) {
	    fprintf(stderr, "library: Can't read bulletins in a window with this few columns (%d)\n", cols);
	    exit(1);
	    }
	}
    item_ptr[0] = -1;
    *mailbox = '\0';
    if (isatty(fileno(stdin))) {
	/* Get the data file name - can be a parameter */
	if (argc) strcpy(mailbox, argv[0]);
	while (! *mailbox) {
	    printf("Enter file/mailbox with Library Network transmission: ");
	    if (fgets(mailbox, 150, stdin) == NULL) exit(1);
	    if ((*mailbox == 'q') || (*mailbox == '.')) return;
	    ptr = strchr(mailbox, '\n');
	    if (ptr != NULL) *ptr = '\0';
	    /* check for help request */
	    if ((*mailbox == '?') || !strlen(mailbox)) {
		sprintf(line, "%s/help.mbox7", WHERE);
		if ((helpfile = fopen(line, "r")) == NULL) {
		    printf("Sorry. Help statement unavailable!\n");
		    continue;
		    }
		while (fgets(line, 120, helpfile) != NULL)
		    fputs(line, stdout);
		fclose(helpfile);
		*mailbox = '\0';
		}
	    }
	}
    else {
	sprintf(mailbox, "/tmp/Lo25.%d", getpid());
	tmp_used = 1;
	if ((outfile = fopen(mailbox, "w")) == NULL) {
	    fprintf(stderr, "library: Couldn't open tmp file %s\n", mailbox);
	    exit(2);
	    }
	while (fgets(line, 120, stdin) != NULL) 
	    fputs(line, outfile);
	fclose(outfile);
	freopen("/dev/tty", "r", stdin);
	}
    total_ann = scan_for_item(mailbox, item_ptr, end_ptr);
    if (total_ann <= 0) {
	fprintf(stderr, "No library announcements available for reading!\n");
	if (tmp_used) unlink(mailbox);
	exit(0);
	}
    next_read = -1;
    see_head = 1;
    skip_top = ((strchr(read_cntl, 'N') == NULL) ? 0 : 1);
    while (1) {
	if (skip_top && (see_head != 2)) {
	    if (next_read == (total_ann -1)) {
		break;
		}
	    infile = fopen(mailbox, "r");
	    printf("\n\n");
	    show_Ihead(infile, item_ptr[next_read+1], next_read+2, -2);
	    fclose(infile);
	    *line = '\0';
	    }
	else {
	    if (see_head) {
		show_head(mailbox, item_ptr, next_read+1);
		see_head = 0;
		}
	    printf("Enter number of desired announcement to read (q to quit): ");
	    if (fgets(line, 120, stdin) == NULL) break;
	    /* get rid of white space at start */
	    ptr = line;
	    while (isspace(*ptr)) ptr++;
	    if (ptr > line) strcpy(line, ptr);
	    if ((*line == '.') || (*line == 'q') || (*line == 'x')) break;
	    if ((*line == 'h') || (*line == '=')) {
		see_head = 1;
		continue;
		}
	    }
	ptr = strchr(line, '\n');
	if (ptr != NULL) *ptr = '\0';
	if (!strlen(line)) {
	    if (next_read == (total_ann -1)) {
		printf("No more announcements to read!\n");
		skip_top = 0;
		continue;
		}
	    next_read++;
	    }
	else {
	    i = atoi(line);
	    if ((i < 1) || (i > total_ann)) {
		printf("Illegal announcement selection <%s>\n", line);
		continue;
		}
	    next_read = i - 1;
	    }
	have_seen[next_read] = 1;
	i = dsp_announce(mailbox, item_ptr[next_read], end_ptr[next_read]-item_ptr[next_read], item_items[next_read], logfile);
	if (i == 1) break; /* They typed q in the lower level */
	see_head = 1;
	if (i == 2) see_head = 2;
	} /* end main loop - for reading whats there */
    if (tmp_used) unlink(mailbox);

    /* output the requests to handler */
    signal(SIGINT, SIG_IGN);
    /* No requests entered noted here */
    if (numreq == 0) return;
    /* Tell them what we are about to send orders for */
    if (strchr(read_cntl, 'C') != NULL) {
	printf("About to transmit requests for:\n");
	ptr = req_str;
	while (ptr != NULL) {
	    ptr = strchr(ptr, '\n')+1; /* skip **-** line */
	    if (strlen(ptr) < 5) break;
	    tptr = ptr;
	    while ((tptr = strchr(tptr, '\n')) != NULL)
		if (strncmp(++tptr, "**-**", 5) == 0) break;
	    if (tptr != NULL) *--tptr = '\0';
	    printf("\t%s\n", ptr);
	    if (tptr != NULL) *tptr++ = '\n';
	    ptr = tptr;
	    }
	/* Let them decide whther to kill requests */
	printf("\nSend the requests? Enter y if yes: ");
	fgets(line, 50, stdin);
	if ((*line != 'y') && (*line != 'Y')) return;
	}
#ifdef MSDOS
    if ((outfile = fopen("\\pipe.tmp", "w")) == NULL) {
#else
    sprintf(line, "%s %s", Cmd, whereto);
    if ((outfile = popen(line, "w")) == NULL) {
#endif
	fprintf(stderr, "Couldn't open mail pipe to send requests! No requests sent\n");
	exit(1);
	}
    fprintf(outfile, "* *#KNOWN-%s\n%s\n%s\n", libcntl, pan, req_name);
    fputs(req_str, outfile);
#ifdef MSDOS
    fclose(outfile);
    sprintf(line, "%s -f \\pipe.tmp -slibRequest %s", Cmd, whereto);
    system(line);
    unlink("\\pipe.tmp");
#else
    pclose(outfile);
#endif
    if (logfile != NULL) {
	ptr = req_str;
	while (ptr != NULL) {
	    ptr = strchr(ptr, '\n')+1; /* skip **-** line */
	    if (strlen(ptr) < 5) break;
	    fprintf(logfile, "From %s %sSubject: option 7 request\n",
			req_name, ctime(&today));
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
    } /* end of reader routine */

scan_for_item(filename, item_ptr, end_ptr)
char *filename;	/* the file being checked out for library announcements */
register long *item_ptr;
register long *end_ptr;
 {
    FILE *file;
    int num, need_end, saw_dashes, items;
    char line[120];

    while (*item_ptr >= 0) item_ptr++;
    saw_dashes = need_end = num = items = 0;
    if ((file = fopen(filename, "r")) == NULL)
	return(-1);
    while (fgets(line, 120, file) != NULL) {
	if (need_end && (!strncmp(line, "From ", 5) || !strncmp(line, "Search done", 11) || !strncmp(line, "Lib_Announce: ", 14))) {
		    *end_ptr++ = ftell(file) - strlen(line);
		    need_end = 0;
		    item_items[num-1] = items;
		    items = 0;
		    }
	if (!strncmp(line, "Lib_Announce: ", 14) || !strncmp(line, "Search done", 11) ) {
		    *item_ptr++ = ftell(file) - strlen(line);
		    need_end = 1;
		    num++;
		    }
	if (!strncmp(line, "---------------- ", 17)) {
		    saw_dashes = 1;
		    items++;
		    }
	}
    /* See if no trigger off hit - assume they know what doing */
    if ((num == 0) && saw_dashes) {
	*item_ptr++ = 0;
	need_end = 1;
	num++;
	}
    if (need_end) {
	fseek(file, 0, 2);
	*end_ptr++ = ftell(file);
	item_items[num-1] = items;
	}
    *item_ptr = -1;
    *end_ptr = 0;
    fclose(file);
    return(num);
    }

dsp_announce(filename, start, size, max_items, logfile)
char *filename;	/* the file being checked out for library announcements */
long start;
long size;
int max_items;	/* The number items in the set, 1 based count */
FILE *logfile;	/* file keeping log of entered requests */
 {
    FILE *file;
    char *Buf, line[120], parms[20], tline[80];
    register char *ptr;
    int brief_size, i, type_ann;

    if ((file = fopen(filename, "r")) == NULL)
	return(-1);
    fseek(file, start, 0);

    /* get the paramters if any */
    type_ann = 0;
    if (fgets(line, 120, file) == NULL) {
	printf("No content found!!!  Aborted read\n");
	return(-2);
	}
    size -= strlen(line);
    /* Capture the description part from this line  */
    ptr = strchr(line, ':');
    if (ptr == NULL) ptr = line;
    if (!isdigit(*(ptr-1)) ||!isdigit(*(ptr+1)))
	ptr++;
    else  {
	ptr += 11;
	/* If time zone now at year */
	if (!strncmp(ptr, "199", 3)) ptr += 4;
	while (isspace(*ptr)) ptr++;
	if (!strncmp(ptr, "on ", 3)) ptr += 3;
	}
    while (isspace(*ptr)) ptr++;
    strncpy(ann_desc, ptr, 50);
    ann_desc[50] = '\0';
    ptr = ann_desc + strlen(ann_desc);
    while (isspace(*--ptr));
    *++ptr = '\0';

    if (*line == 'L')
	type_ann = 1;
    ptr = strchr(line, '{');
    if (ptr != NULL) {
	strncpy(parms, ptr+1, 19);
	parms[19] = '\0';
	ptr = strchr(line, '}');
	if (ptr != NULL) *ptr = '\0';
	}
    else
	strcpy(parms, "2");

    /* Skip header lines */
    while (fgets(line, 120, file) != NULL) {
	size -= strlen(line);
	ptr = strchr(line, ' ');
	if ((ptr == NULL) || (*(ptr-1) != ':')) break;
	}
    
    /* show stuff before parseable part */
    if (strncmp(line, "---------------- ", 17)) {
	printf("\n");
	if (type_ann == 0) printf("The following preceded the actual items being read:\n");
	printf("==============================================\n");
	while (strncmp(line, "---------------- ", 17) && strncmp(line, "From ", 5) && strncmp(line, "Search done", 11) && strncmp(line, "Lib_Announce: ", 14)) {
	    fputs(line, stdout);
	    /* Let the error below tell them this end happened */
	    if (fgets(line, 120, file) == NULL) break;
	    size -= strlen(line);
	    }
	printf("==============================================\n");
	if (strncmp(line, "---------------- ", 17)) {
	    if (type_ann != 1) printf(" No readable items this message!!\n");
	    if (strchr(read_cntl, 'N') != NULL) {
		printf("\nHit return to continue: ");
		if (fgets(tline, 80, stdin) == NULL) exit(1);
		}
	    if (type_ann == 1) return(0);
	    return(-3);
	    }
	printf("  Hit return to see one screen of items: ");
	if (fgets(tline, 80, stdin) == NULL) exit(1);
	ptr = strchr(tline, '\n');
	if (ptr != NULL) *ptr = '\0';
	}
    /* now we are at items being read */
    if (*parms == 'F')
	brief_size = 300;
    else
	brief_size = atoi(parms);
    if (! brief_size) brief_size = 2;
    /* get this message remainder in core for user perusal */
    if ((Buf = malloc(size+strlen(line)+100)) == NULL) {
	fprintf(stderr, "Couldn't get %d space for item! Reading aborted.\n", size);
	exit(1);
	}
    strcpy(Buf, line);
    ptr = Buf + strlen(Buf);
    i = fread(ptr, 1, size, file);
    ptr += size;
    *ptr++ = '\0';
    *ptr = '\0';
    fclose(file);
    /* now start actual reading of items - decide how many at a time */
    i = dsp_stuff(Buf, brief_size, max_items, logfile);
    free(Buf);
    return(i);
    }

dsp_stuff(Item, brief_size, max_items, logfile)
char *Item;
int brief_size;
int max_items;	/* The number items in the set, 1 based count */
FILE *logfile;	/* file keeping log of entered requests */
 {
    int at_end, cnt, i, full_rec, low_val;
    int screen_size;	/* Size of screen in brief record displays */
    char *ptr, *ptr2,
	 *docid, /* Document number during request function */
	 *order_ptr, line[250], *screen_ptr;
    FILE *tfile;

    screen_size = cnt = at_end = 0;
    screen_ptr = Item;
    /* Set so takes to breaks to break */
    signal(SIGINT, sig_catch);
    while (1) {
	ptr = next_item(screen_ptr, 0);
	low_val = cnt + 1;
	if (brf_display(ptr, brief_size, &cnt, stdout, max_items) == 0) break;
	if (screen_size == 0) screen_size = cnt;
	order_ptr = NULL;
	full_rec = 0;
	while (1) {
	    printf("\nEnter (return/number/q/?)  ==> ");
	    if (fgets(line, 250, stdin) == NULL) {
		if (int_hit) {
		    printf("(Interrupt -- one more to kill reader)\n");
		    continue;
		    }
		exit(1); /* This means the input was from a pipe and ended */
		}
	    /* get rid of white space at start */
	    ptr2 = line;
	    while (isspace(*ptr2)) ptr2++;
	    if (ptr2 > line) strcpy(line, ptr2);
	    if ((*line == 'q') || (*line == 'x')) return(1);
	    if ((*line == '.') || (*line == '^')) return(2);
	    ptr2 = strchr(line, '\n');
	    if (ptr2 != NULL) *ptr2 = '\0';
	    /* turn catcher back on in case was off */
	    signal(SIGINT, sig_catch);
	    int_hit = 0;
	    if (*line == '?') {
		printf("\
The following are acceptable commands:\n\
   return     displays next item or set of items;\n\
   number     for more detailed display of a given item;\n\
   r          request the present displayed item;\n\
   r number   request given item from present display;\n\
   | cmd      Pipe present screen to a Unix system command;\n\
   s file     Save present screen to the file specified;\n\
   h or =     show brief descriptions of present items;\n\
   . or ^     return to list of available announcements for reading;\n\
   z          displays next screen of brief displays;\n\
   q (or x)   quit this reading session totally;\n\
   ?          this help display\n");
		continue;
		}
	    /* Note n is like, pulling up brief displays (=), then hitting a return */
	    if ((*line == 'h') || (*line == '=') || (*line == 'z')) {
		rest_full = NULL;	/* Make no partial read record being tracked */
		if (full_rec) {
		    cnt = (full_rec / screen_size) * screen_size;
		    screen_ptr = next_item(Item, cnt);
		    full_rec = 0;
		    }
		else
		    cnt = low_val - 1;
		if (*line != 'z') break;
		*line = '\0'; /* Fall thru as though hit return */
		cnt += screen_size;
		}
	    if (*line == 'r') {
		if (strlen(line) > 1) {
		    ptr2 = line + 1;
		    while (*ptr2 && !isdigit(*ptr2)) ptr2++;
		    i = atoi(ptr2);
		    if ( (i < 1) || (i > max_items)) {
			    printf("Illegal item specified <%s>\n", line+1);
			    continue;
			    }
		    ptr2 = next_item(Item, i-1);
		    order_ptr = strchr(ptr2, '(');
		    ptr2 = strchr(ptr2, '\n');
		    if ((order_ptr == NULL) || (order_ptr >= ptr2)) {
			printf("Sorry, this is not a requestable item!\n");
			continue;
			}
		    } /* end order with item specified */
		else if (order_ptr == NULL) {
		    if (full_rec)
			printf("Sorry, this is not a requestable item!\n");
		    else
			printf("Specify which item is desired (i.e. r <number>)\n");
		    continue;
		    }
		numreq++;
		ptr2 = req_ptr; /* Save start pointer, in case canceled below */
		strcpy(req_ptr, "**-**\n");
		req_ptr += strlen(req_ptr);
		docid = req_ptr;
		printf("You have requested: ");
		order_ptr++;
		while (*order_ptr && (*order_ptr != '\n') && (*order_ptr != ')')) {
		    *req_ptr++ = *order_ptr;
		    fputc(*order_ptr++, stdout);
		    }
		*req_ptr = '\0';
		/* Check in the logfile to see if this has been ordered */
		if (logfile != NULL) {
		    rewind(logfile);
		    i = strlen(docid);
		    while (fgets(line, 250, logfile) != NULL) 
			if (!strncmp(line, docid, i) && (*(line+i) == '\n')) {
			    /* Make sure pointer at end, for write later */
			    fseek(logfile, 0L, 2);
			    printf("\nDocument ID %s already exists in your log file.\n", docid);
			    printf("Do you wish to request it again (type y if yes): ");
			    if ((fgets(line, 100, stdin) == NULL) || ((*line != 'y') && (*line != 'Y')) )
				    i = 0;
			    break;
			    } /* end deling with already ordered */
		    if (!i) { /* i is flag, only reorder if set to 0 */
			numreq--;
			req_ptr = ptr2;
			*req_ptr = '\0';
			continue;
			}
		    } /* end of check in logfile */
		*req_ptr++ = '\n';
		*req_ptr = '\0';
		/* Note a new line put at start of Enter prompt above */
		continue;
		}
	    if ((*line == '|') || (*line == 's') || ((*line == 'p') && (line[1] == 'i'))) {
		ptr2 = line;
		if (*ptr2 == '|')
		    ptr2++;
		else
		    while (*ptr2 && !isspace(*ptr2)) ptr2++;
		while (isspace(*ptr2)) ptr2++;
		if (! *ptr2) {
		    if (*line == 's')
			fprintf(stderr, "A file must be specified on a save command\n");
		    else
			fprintf(stderr, "A command must be given for a pipe request!!\n");
		    continue;
		    }
		if (*line == 's')
		    tfile = fopen(ptr2, "a");
		else
		    tfile = popen(ptr2, "w");
		if (tfile == NULL) {
		    fprintf(stderr, "Couldn't open a pipe to <%s>\n", ptr2);
		    continue;
		    }
		if (!full_rec) {
		    cnt = low_val - 1;
		    brf_display(screen_ptr, brief_size, &cnt, tfile, max_items);
		    }
		else {
		    ptr2 = next_item(Item, full_rec-1);
		    order_ptr = full_display(ptr2, full_rec, tfile, max_items);
		    }
		if (*line == 's')
		    fclose(tfile);
		else
		    pclose(tfile);
		continue;
		}
	    if (! *line && (rest_full != NULL)) {
		full_display(NULL, 0, stdout, max_items);
		continue;
		}
	    if (!strlen(line)) {
		if (full_rec && (full_rec < cnt)) {
		    i = full_rec + 1;
		    }
		else {
		    screen_ptr = next_item(Item, cnt);
		    break;
		    }
		}
	    else {
		i = atoi(line);
		}
	    if ( (i < 1) || (i > max_items)) {
		    printf("Illegal response <%s>\n", line);
		    continue;
		    }
	    full_rec = i;
	    ptr2 = next_item(Item, i-1);
	    if (pager != NULL) 
		tfile = popen(pager, "w");
	    else
		tfile = stdout;
	    order_ptr = full_display(ptr2, full_rec, tfile, max_items);
	    if (tfile != stdout) pclose(tfile);
	    int_hit = 0;
	    } /* Loop on specific item request */
	} /* loop to see all items */
    return(0);
    } /* end dsp_stuff */

/* this routine finds the size of the next item */
find_size(ptr)
register char *ptr;
 {
    register int size;
    if (ptr == NULL) return(-1);
    if ( strncmp(ptr, "---------------- ", 17)) return(-1);
    size = -1;
    do {
	ptr = strchr(ptr, '\n');
	if (ptr++ == NULL) break;
	size++;
	} while (strncmp(ptr, "---------------- ", 17));
    return(size);
    } /* end of find_size */

/* Jumps forward to next item */
char *next_item(ptr, skip)
register char *ptr;
int skip;
 {

    while (ptr != NULL) {
	while (strncmp(ptr, "---------------- ", 17)) {
	    ptr = strchr(ptr, '\n');
	    if (ptr++ == NULL) return(NULL);
	    }
	if (!skip--) return(ptr);
	ptr = strchr(ptr, '\n');
	if (ptr++ == NULL) return(NULL);
	} 
    return(NULL);
    } /* end of next_item getter */

char *full_display(ptr, which, file, total)
char *ptr;
int which;
FILE *file;
int total;	/* Total number records in display */
 {
    register char *lptr, *optr;
    int line_cnt;

    if (ptr == NULL)
	lptr = rest_full;
    else {
	lptr = strchr(ptr, '\n') + 1;
	optr = strchr(ptr, '(');
	if ((optr != NULL) && (optr >= lptr)) optr = NULL;
	/* Note the %28s makes a 79 char line */
	fprintf(file, "------ item %d (of %d) in %.28s ---", which, total, ann_desc);
	if (optr != NULL) fprintf(file, " [type \"r\" to request]");
	fprintf(file, "\n");
	}
    /* Count the lines - and leave 3 more */
    line_cnt = 4;
    rest_full = NULL;
    while ((lptr != NULL) && strncmp(lptr, "---------------- ", 17)) {
	if (int_hit) break;
	if (line_cnt++ > lines) {
	    if (file == stdout) {
		rest_full = lptr;
		printf("    ---  Hit return for more of record  ---\n");
		break;
		}
	    }
	lptr = show_line(lptr, NULL, file);
	}
    return(optr);
    }

/* This shows the show display of each item on present screen */
brf_display(ptr, brief_size, cnt_val, file, total)
char *ptr;
int brief_size;
int *cnt_val;
FILE *file;
int total;	/* Total number records in display */
  {
    int linecnt, i;
    char sbuf[200], *ptr2;
    int cnt;
    int next_size;

	linecnt = 0;
	cnt = *cnt_val;
	next_size = find_size(ptr);
	if (next_size > brief_size) next_size = brief_size;
	while (((linecnt+next_size+1) < lines) && (ptr != NULL)) {
	    if (int_hit) break;
	    /* check for control info on this record */
	    ptr = strchr(ptr, '\n');
	    if (ptr == NULL) break;
	    sprintf(sbuf, "%3d/%d) ", ++cnt, total);
	    ptr = show_line(++ptr, sbuf, file);
	    linecnt++;
	    for (i=1; (i < brief_size); i++) {
		if ((ptr == NULL) || !strncmp(ptr, "---------------- ", 17))
		    printf("\n");
		else if (*(ptr-1) != '\n') {
		    ptr2 = strchr(ptr, '\n');
		    if ((ptr2 == NULL) || !strncmp(ptr2+1, "---------------- ", 17))
			ptr = show_line(ptr, "     ", file);
		    else {
			*ptr2 = '\0';
			sprintf(sbuf, "     %.150s ", ptr);
			*ptr2 = '\n';
			ptr = ptr2 + 1;
			/* In wrape case remove spaces */
			while (*ptr == ' ') ptr++;
			ptr = show_line(ptr, sbuf, file);
			}
		    }
		else
		    ptr = show_line(ptr, "     ", file);
		linecnt++;
		}
	    /* If they want, give a blank line between records */
	    if (strchr(read_cntl, 'B') != NULL) {
		fprintf(file, "\n");
		linecnt++;
		}
	    /* skip to next thing to display */
	    ptr = next_item(ptr, 0);
	    next_size = find_size(ptr);
	    if (next_size > brief_size) next_size = brief_size;
	    }
	*cnt_val = cnt;
	if ((ptr == NULL) && !linecnt) return(0);
	return(1);
	}

char *show_line(ptr, start, file)
register char *ptr;
char *start;
FILE *file;
 {
    register int i;
    char *ptr2, tchar;

    if (ptr == NULL) return(ptr);
    i = cols-1;
    if ((start != NULL) && strlen(start)) {
	fputs(start, file);
	i -= strlen(start);
	}
    /* check if line is too big to fit */
    ptr2 = strchr(ptr, '\n');
    if (ptr2 == NULL) ptr2 = ptr + strlen(ptr);
    if ((ptr2 - ptr) > i) {
	ptr2 = ptr + i;
	while ((ptr2 > ptr) && !isspace(*ptr2)) ptr2--;
	if (ptr2 > ptr) {
	    if (ispunct(*ptr2)) tchar = *++ptr2;
			   else tchar = *ptr2;
	    *ptr2 = '\0';
	    }
	else
	    ptr2 = NULL;
	}
    else
	ptr2 = NULL;
    while (i-- && *ptr) {
	if (*ptr == '\n') break;
	fputc(*ptr++, file);
	}
    fputc('\n', file);
    if (*ptr == '\n') ptr++;
    if (ptr2 != NULL) {
	*ptr2 = tchar;
	ptr = ptr2;
	}
    if (! *ptr) ptr = NULL;
    return(ptr);
    }

show_head(filename, item_ptr, next)
char *filename;	/* the file being checked out for library announcements */
long item_ptr[];
int next;	/* THe list they will get if they hit return */
 {
    FILE *file;
    int num;
    long pos;

    if ((file = fopen(filename, "r")) == NULL)
	return(-1);
    num = 0;
    printf("\n  Available announcement(s) for reading:\n");
    while (item_ptr[num] >= 0) {
	pos = item_ptr[num++];
	show_Ihead(file, pos, num, next);
	}
    fclose(file);
    return(1);
    }

show_Ihead(file, pos, num, next)
FILE *file;	/* File containing the announcements */
long pos;	/* Position in the file where this item record is */
int num;	/* number of header being shown */
int next;	/* THe list they will get if they hit return */
 {
    char line[120], *ptr, *ptr2;

	fseek(file, pos, 0);
	if (fgets(line, 120, file) == NULL) 
	    return(-1);
	/* Put on lead tag */
	if (next == -2)
	    printf(" Announcement %d of %d ", num, total_ann);
	else {
	    printf("%c", (((num-1) == next) ? '>' : ' '));
	    printf("%c", ((have_seen[num-1] == 1) ? '*' : 'U'));
	    printf(" %3d\t", num);
	    }
	if (*line == 'L')
	    printf("[Message]    ");
	else
	    printf("[%2d items]  ", item_items[num-1]);
	ptr = strchr(line, ':');
	if (ptr == NULL) {
	   printf("Unlabeled search set\n");
	   return(1);
	   }
	/* Throw out end of line, or everything after { start */
	ptr2 = strchr(ptr, '{');
	if (ptr2 == NULL) ptr2 = strchr(ptr, '\n');
	if (ptr2 != NULL) *ptr2 = '\0';
	/* Ptr should be pointing at first colon of   MMM DD TT:TT:TT XXXX CCYY */
	/* If not in this form, then just show after the : */
	if (!isdigit(*(ptr-1)) ||!isdigit(*(ptr+1))) {
	    ptr++;
	    }
	else {
	    printf("%6.6s, ", ptr-9);
	    ptr += 11;
	    }
	if (next == -2)
	    printf("%-.40s\n", ptr);
	else
	    printf("%s\n", ptr);
	return(1);
	}
