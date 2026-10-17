#ifndef lint
static char sccsid[] = "@(#)autores.c	1.6 (UKC) 14/2/86";
#endif  lint
/***

* program name:
	autores.c
* function:
	Send an automatic reply to mail depending on the subject field
	The subject field is used to select a file in 
	/usr/lib/mmdf/auto which is interpreted as a mail item to send out
	Subject lines starting with 'Re:' are ignored
	If the first line of the file starts with '~', it is taken
	to be the Subject line for the outgoing mail.
	A line should be installed in the mmdf aliases file of the form
		information:mmdf|/usr/lib/mmdf/autores "$(sender)"
	If there are two parameters, the subject line is ignored and the
	second parameter is used to select a file
* switches:
	Called with the return address
	and optionally a named file
* libraries used:
	standard
* compile time parameters:
	cc -o autores autores.c
* history:
	Written Feb 1986
	Peter Collinson UKC

***/
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/wait.h>

/*
 *	Source directory
 */
char *srcdir = "/usr/lib/mmdf/auto";

/*
 *	who to reply to
 */
char *sender; 

/*
 *	system log file
 */
char *logfile = "/usr/adm/log/information";
int	logfd = -1;
int	logpid;

/*
 *	file to look for
 */
char *command;
int	fromarg;
char	submit[]	= "/usr/lib/mmdf/submit";
char	subargs[]	= "-vmltnqxto,*";
char	signature[]	= "information@Ukc";

int	pipein[2];	/* pipes for communication */
int	pipeout[2];

FILE	*pout;		/* re-opened FILE pointer for pipeout */
FILE	*pin;		/* re-opened FILE pointer for pipein */

/*
 *	error response line
 */
char	errline[BUFSIZ];
int	senderror;

char	stdinfo[] =	/* Put out in each message */
"\n\
An index of the available information can be obtained by mailing to\n\
information@ukc.ac.uk with a Subject: line containing the word index\n\
i.e.\n\
	Subject: index\n\
\n";

/*
 *	Data structure used to scan and recognise an inbound mail message
 *	All this is overkill but I had the code hanging around and I am
 *	never one to re-write stuff which works
 */
typedef struct
{	char	*header;
	char	*dataline;
	int	hadthis;
} Header_line;

Header_line header_line[] =
{	{ "subject", },
	{ 0, }
};
#define HSUBJECT	0


main(argc, argv)
char **argv;
{	openlog();	
	if (argc == 3)
	{	command = argv[2];
		fromarg++;
	}
	else
	if (argc != 2)
		fatal("Usage: autores sender\n");
	sender = argv[1];
	if (chdir(srcdir) < 0)
		fatal("No source directory\n");	
	/*
	 *	Mail header decoding
	 *	all we want is the subject line
	 */
	if (fromarg == 0)
	{	if (readheader(stdin)) /* if not OK */
			command = NULL;
	}
	clear_input(stdin);
	if (command)
		send_info(command);
	if (senderror)
		send_error();
	closelog();
	exit(0);
}

/*
 *	Read an incoming mail item looking for the subject line
 *	This all borrowed from another program
 */
readheader(fin)
FILE *fin;
{	register char *cp;
	char *index();

	if (get_header_lines(fin))
		return(1);
	if (header_line[HSUBJECT].hadthis == 0)
	{	usererror("No Subject: line found\n");
		return(1);
	}
	command = header_line[HSUBJECT].dataline;		
	/*
	 * look for Re: bits in lines
	 */
	while ((*command == 'R' || *command == 'r') &&
	       (command[1] == 'e' || command[1] == 'E' ) &&
	       (command[2] == ':'))
	{	log("Re: found request from %s ignored\n", sender);
		return(1);
	}
	return(0);
}

/*
 *	Scan the inbound mail header
 */
get_header_lines(fin)
FILE *fin;
{	register char *p;
	register char *startfield;
	register Header_line *hp;
	char	line[BUFSIZ];
	int	peekc;
	Header_line *look_header();
	char	*storestr();
	char	*storepair();
	char 	*index();
	
	for (;;)
	{	if (fgets(line, sizeof line - 1, fin) == NULL)
		{	usererror("Unexpected end of mail input file\n");
			return(1);
		}
		peekc = getc(fin);
		(void) ungetc(peekc, fin);
		if (line[0] == '\n')
			break;
		hp = (Header_line *)0;
		if (p = index(line, ':'))
		{	*p++ = '\0';
			if (hp = look_header(line))
			{	/* we want to know about this line */
				if (hp->hadthis++)
				{	/* we have had this line before */
					usererror("More than one %s header line found\n", hp->header);
					return(1);
				}
				/* clean up this line */
				if (*p == ' ') p++;
				startfield = p;
				p = index(startfield, '\n');
				if (p == NULL)
					line[BUFSIZ-1] = '\0';
				else
					*p = '\0';
				hp->dataline = storestr(startfield);
			}
		}
		/*
		 *	cope with continuation lines
		 */
		while (peekc == ' ' || peekc == '\t')
		{	if (fgets(line, sizeof line -1, fin) == NULL)
			{	usererror("Unexpected end of mail input file\n");
				return(1);
			}
			peekc = getc(fin);
			(void) ungetc(peekc, fin);
			/*
			 * if hp is set then we need to concatenate the
			 * new bit onto the old stored piece
			 */
			if (hp)
			{	if (p = index(line, '\n'))
					*p = '\0';
				else	line[BUFSIZ-1] = '\0';
				hp->dataline = storepair(hp->dataline, line, 1);
			}
		}
	}
	return(0);
}

/*
 *	scan the Header_line structure looking for a matching
 *	string which is a header - use equstr to match
 */
Header_line *
look_header(str)
char *str;
{	register Header_line *hp;
	for (hp = header_line; hp->header; hp++)
		if (equstr(str, hp->header) == 0)
			return(hp);
	return((Header_line *)0);
}

/*
 *	read very hard on the file descriptor throwing the data away
 */
clear_input(fin)
FILE *fin;
{	register c;
	while ((c = getc(fin)) != EOF);
}

/*
 *	Send information
 */
send_info(cmd)
register char *cmd;
{	register char *p;
	register bytes;
	char	line[BUFSIZ];
	FILE *fin;
	
	/*
	 *	first clean up the command
	 *	All must start with an alpha character
	 */
	while (!isalpha(*cmd))
		cmd++;
	for (p = cmd; *p; p++)
	{	if (isupper(*p))
			*p = tolower(*p);
		if (isspace(*p) || ! isprint(*p))
		{	*p = '\0';
			break;
		}
	}
	if ((fin = fopen(cmd, "r")) == NULL)
	{	usererror("No information on: %s\n", cmd);
		return;
	}
	log("%s requests %s\n", sender, cmd);
	/* Subject is the 1st line of source - assuming is starts with '~' */
	fgets(line, sizeof line, fin);
	init_mail();
	mail_header(cmd, line[0] == '~' ? &line[1] : NULL);
	fprintf(pout, "Thank you for your information request\n");
	fprintf(pout, stdinfo);
	fprintf(pout, "-----%s-----\n", cmd);
	/*
	 * replace initial line it it is not a comment
	 */
	if (line[0] != '~')
		fputs(line, pout);
	/*
	 *	now read the remainder and send to pipe
	 */
	while (bytes = fread(line, 1, sizeof line, fin))
		fwrite(line, 1, bytes, pout);
	(void) fclose(pout);
	mail_termination();
}

/*
 *	Send an error reply
 */
send_error()
{	init_mail();
	mail_header("Error response", NULL);
	if (command)
		fprintf(pout, "Your request for information with a subject of `%s'\n", command);
	else
		fprintf(pout, "Your request for information\n");
	fprintf(pout, "has failed. The reason was:\n\n%s\n", errline);
	fprintf(pout, stdinfo);
	(void) fclose(pout);
	mail_termination();
}

/*
 *	Initialise a pipe to submit
 */
init_mail()
{	register pid;
	register i;
	char	*subcmd;
	char	*rindex();
	char	*maildate();
		
	subcmd = rindex(submit, '/');
	if (subcmd)
		subcmd++;
	else
		subcmd = submit;
		
	(void) pipe(pipein);	/* parent will read from pipein[0] */
	(void) pipe(pipeout);	/* parent will write to pipeout[1] */
	
	if ((pid = fork()) < 0)
	{	fatal("Cannot create a process for mail submission\n");
		return;
	}
	if (pid == 0)
	{	/* child */
		/* stdin is pipeout[0] */
		/* stdout/stderr is pipein[1] */
		(void) close(pipeout[1]);
		(void) close(pipein[0]);
		(void) close(0);
		(void) dup(pipeout[0]);
		(void) close(pipeout[0]);
		(void) close(1);
		(void) dup(pipein[1]);
		(void) close(pipein[1]);
		(void) close(2);
		(void) dup(1);
		for (i = 3; i < 20; i++)
			(void) close(i);
		execl(submit, subcmd, subargs, 0);
		(void) close(0);
		(void) close(1);
		(void) close(2);
		exit(1);
	}
	/*
	 *	Parent
	 */
	(void) close(pipein[1]);
	(void) close(pipeout[0]);
	pout = fdopen(pipeout[1], "w");
	pin = fdopen(pipein[0], "r");
}

/*
 *	Deal with the termination of the mail process
 */
mail_termination()
{	char	logbuf[BUFSIZ];
	union	wait retstat;
	
	while(fgets(logbuf, sizeof logbuf, pin))
		log("%s", logbuf);
	(void) fclose(pin);
	
	if (wait(&retstat) > 0)
	{	if (retstat.w_retcode == 9 &&
			retstat.w_termsig == 0)
		{	log("successful submit\n");
			return;
		}
		log("Submit failed, returning %x %d\n", retstat, retstat);
	}
	log("Submit failed\n");
}

/*
 *	Do a mail header
 */
mail_header(cmd, subject)
char *cmd;
char *subject;
{
	fprintf(pout, "%s\n", signature);	/* special validation argument */
	fprintf(pout, "To: %s\n", sender);
 	fprintf(pout, "From: Information service <%s>\n", signature);
	if (subject)
		fprintf(pout, "Subject: Re: %s - %s", cmd, subject);
	else	fprintf(pout, "Subject: Re: %s\n", cmd);
	fprintf(pout, "Date: %s\n", maildate());
	fprintf(pout, "\n");
}

/*
 *	return a static string with the data in RFC822 format
 */
char *
maildate()
{
	static	char datbuf[64];
	static char *day[] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};
	static char *month[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	struct timeb    timeb;
	extern char *timezone();
	extern struct tm  *localtime ();
 	register struct tm  *i;
		
	ftime(&timeb);
	i = localtime ((time_t *) & timeb.time);
	(void) sprintf(datbuf, "%s, %d %s %d %d:%02d:%02d %s",
		day[i -> tm_wday], i -> tm_mday, month[i -> tm_mon],
		i -> tm_year, i -> tm_hour, i -> tm_min, i -> tm_sec,
			timezone (timeb.timezone, i -> tm_isdst));
	return(datbuf);
}
	 
/*
 *	See if two strings are the same irrespective of case
 *	return 0 if equal, 1 otherwise
 */
equstr(a, b)
register char *a, *b;
{	register ca, cb;
	while (ca = *a)
	{	cb = *b;
		if (cb == 0)
			return(1);
		if (ca != cb)
		{	if (isupper(ca))
				ca = tolower(ca);
			if (isupper(cb))
				cb = tolower(cb);
			if (ca != cb)
				return(1);
		}
		a++;
		b++;
	}
	return(*b == '\0' ? 0 : 1);
}

/*
 *	string management routines
 */
char *
storestr(str)
char *str;
{	register nchs;
	char *area;
	char *malloc();
	
	nchs = strlen(str) + 1;
	if ((area = malloc((unsigned) nchs)) == NULL)
		fatal("No memory for string storage\n");
	bcopy(str, area, nchs);
	return (area);
}

/*
 *	Concatenate and store a pair of strings
 *	the last parameter is used as a mask to indicate that one or other
 *	of the paramaters can be free()'d
 */
char *
storepair(a, b, msk)
char *a;
char *b;
{	register nchs;
	register alen;
	register blen;
	char *area;

	alen = strlen(a);
	blen = strlen(b);
	nchs = alen + blen + 1;
	if ((area = malloc((unsigned) nchs)) == NULL)
		fatal("No memory for string storage\n");
	bcopy(a, area, alen);
	bcopy(b, &area[alen], blen+1);
	if (msk & 01)
		free(a);
	if (msk & 02)
		free(b);
	return(area);
}

/*
 *	set up an error reply line
 */
/*VARARGS1*/
usererror(fmt, a, b, c, d)
char *fmt;
{	(void) sprintf(errline, fmt, a, b, c, d);
	senderror++;
	log(fmt, a, b, c, d);

}

/*
 *	open the log file
 */
openlog()
{
	logpid = getpid();
	
	logfd = open(logfile, O_WRONLY|O_APPEND);

}

/*VARARGS1*/
log(fmt, a, b, c, d)
char *fmt;
{	char line[BUFSIZ];
	time_t ti;
	char *ctime();
	register hdrlen;
	
	if (logfd < 0)
		return;
	
	(void) time(&ti);
	(void) sprintf(line, "%6d %15.15s: ", logpid, ctime(&ti)+4);
	hdrlen = strlen(line);
	(void) sprintf(&line[hdrlen], fmt, a, b, c, d);
	write(logfd, line, strlen(line));
}

closelog()
{	if (logfd >= 0)
		close(logfd);
}

/*VARARGS1*/
fatal(fmt, a, b, c, d)
char *fmt;
{	log(fmt, a, b, c, d);
	closelog();
	exit(0);
}
