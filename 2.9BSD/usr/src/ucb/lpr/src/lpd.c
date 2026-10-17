/*	lpd.c	4.9	81/09/04	*/

/*
 * lpd -- line-printer daemon
 *
 *      Recoded into c from assembly. Part of the code is stolen
 *      from the data-phone daemon.
 */

#include <signal.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <whoami.h>
#include "lp.local.h"

#define	SLEEPER

#define DORETURN	0	/* absorb fork error */
#define DOABORT		1	/* abort lpd if dofork fails */
#define DOUNLOCK	2	/* remove lock file before aborting */

char    line[132];		/* line from daemon file */
char	title[80];		/* ``pr'' title */
FILE	*dfd;			/* daemon file */
int     fo;			/* output file */
int	lp;			/* line printer file descriptor */
int     df;			/* lpd directory */
int	lfd;			/* lock file */
int     pid;                    /* id of process */
int	child;			/* id of any children */
static  int filter = 0;		/* id of output filter, if any */
int	tof = TOF;		/* flag - at top of form */

char	*LP;			/* line printer name */
char	*LO;			/* lock file name */
char	*SD;			/* spooling directory */
char	*AF;			/* accounting file */
char	*LF;			/* log file for error messages */
char	*OF;			/* name of ouput filter */
char	*FF;			/* form feed string */
int	PL;			/* page length */
short	SF;			/* suppress FF on each print job */
short	SH;			/* suppress header page */
int	DU;			/* daemon uid */
#ifdef	SLEEPER
static  int haveslept = 0;	/* sleep just in case... */
#endif	SLEEPER

extern	banner();		/* big character printer */
char    *logname;               /* points to user login name */
char    class[20];              /* classification field */
char    jobname[20];            /* job or file name */

char	*name;			/* name of program */
char	*printer;		/* name of printer */

char	*rindex();
char	*pgetstr();

/*ARGSUSED*/
main(argc, argv)
char *argv[];
{
	struct stat dfb;
	register struct direct *pdir;
	register int nitems;
	struct direct *p;
	int i;
	int dcomp();
	int dqcleanup();
	extern char *malloc(), *realloc();

	/*
	 * set-up controlled environment; trap bad signals
	 */
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, dqcleanup);	/* for use with lprm */

	name = argv[0];
	if(argc>1)
		printer = argv[1];
	else
		printer = "lp";
	init();				/* set up capabilities */
	for (i = 0; i < NOFILE; i++)
		close(i);
	open("/dev/null", 0);		/* standard input */
	open(LF, 1);			/* standard output */
	lseek(1, 0L, 2);		/* append if into a file */
	dup2(1, 2);			/* standard error */

	/*
	 * opr uses short form file names
	 */
	if(chdir(SD) < 0) {
		log("can't change directory");
		exit(1);
	}
	if(stat(LO, &dfb) >= 0 || (lfd=creat(LO, 0444)) < 0)
		exit(0);
	/*
	 * kill the parent so the user's shell can function
	 */
	if (dofork(DOUNLOCK))
		exit(0);
	/*
	 * write process id for others to know
	 */
	pid = getpid();
	if (write(lfd, (char *)&pid, sizeof(pid)) != sizeof(pid))
		log("can't write daemon pid");
	/*
	 * acquire lineprinter
 	 */
	for (i = 0; (lp = open(LP, 1)) < 0; i++) {
		/* this means we try for 10 minutes */
		if (i > 20) {
			log("%s open failure", LP);
			unlink(LO);
			exit(1);
		}
		sleep(30);
	}

	/*
	 * search the directory for work (file with name df which is
	 * short for daemon file)
	 */
again:
	if ((df = open(".", 0)) < 0) {
		extern int errno;

		log("can't open \".\" (%d)", errno);
		unlink(LO);
		exit(2);
	}

	/*
	 * Find all the spool files in the spooling directory
	 */
	lseek(df, (long)(2*sizeof(struct direct)), 0); /* skip . & .. */
	pdir = (struct direct *)malloc(sizeof(struct direct));
	nitems = 0;
	while (1) {
		register struct direct *proto;

		proto = &pdir[nitems];
		if (read(df, (char *)proto, sizeof(*proto)) != sizeof(*proto))
			break;
		if (proto->d_ino == 0 || proto->d_name[0] != 'd' ||
		    proto->d_name[1] != 'f')
			continue;	/* just daemon files */
		nitems++;
		proto = (struct direct *)realloc((char *)pdir,
				(unsigned)(nitems+1)*sizeof(struct direct));
		if (proto == NULL)
			break;
		pdir = proto;
	}
	if (nitems == 0) {		/* EOF => no work to do */
#ifndef	SLEEPER
		unlink(LO);
		exit(0);
	}
#else	SLEEPER
		if ( haveslept ) {
		    unlink(LO);
		    close(fo);
		    while (wait(0) > 0)
			;
		    exit(0);
		} else {
		    sleep(NAPTIME);
		    haveslept = 1;
		}
	} else
	    haveslept = 0;
#endif	SLEEPER
	close(df);

	/*
	 * Start up an output filter, if needed.
	 */
	qsort(pdir, nitems, sizeof(struct direct), dcomp);
	/*
	 * we found something to do now do it --
	 *    write the pid of the current daemon file into the lock file
	 *    so the spool queue program can tell what we're working on
	 */
	for (p = pdir; nitems > 0; p++, nitems--) {
		extern int errno;

		if (stat(p->d_name, &dfb) == -1)
			continue;
		lseek(lfd, (long)sizeof(int), 0);
		pid = atoi(p->d_name+3);	/* pid of current daemon file */
		if (write(lfd, (char *)&pid, sizeof(pid)) != sizeof(pid))
			log("can't write (%d) daemon file pid", errno);
		doit(p->d_name);
	}
	free((char *)pdir);
	close(fo);
	if (filter)
		while ((pid = wait(0)) > 0 && pid != filter)
			;
	goto again;
}

/*
 * Compare routine for qsort'n the directory
 */
dcomp(d1, d2)
register struct direct *d1, *d2;
{
	return(strncmp(d1->d_name, d2->d_name, DIRSIZ));
}

/*
 * The remaining part is the reading of the daemon control file (df)
 */
doit(file)
char *file;
{
	time_t tvec;
	extern char *ctime();

	/*
	 * open daemon file
	 */
	if ((dfd = fopen(file, "r")) == NULL) {
		extern int errno;

		log("daemon file (%s) open failure <errno = %d>", file, errno);
		return;
	}

	/*
	 *      read the daemon file for work to do
	 *
	 *      file format -- first character in the line is a command
	 *      rest of the line is the argument.
	 *      valid commands are:
	 *
	 *              L -- "literal" contains identification info from
	 *                    password file.
	 *              I -- "indent"changes default indents driver
	 *                   must have stty/gtty avaialble
	 *              F -- "formatted file" name of file to print
	 *              U -- "unlink" name of file to remove (after
	 *                    we print it. (Pass 2 only).
	 *		R -- "pr'ed file" print file with pr
	 *		H -- "header(title)" for pr
	 *		M -- "mail" to user when done printing
	 *
	 *      getline read line and expands tabs to blanks
	 */

	/* pass 1 */
	while (getline()) switch (line[0]) {

	case 'J':
		if(line[1] != '\0' )
			strcpy(jobname, line+1);
		else
			strcpy(jobname, "              ");
		continue;
	case 'C':
		if(line[1] != '\0' )
			strcpy(class, line+1);
		else
			gethostname(class, sizeof (class));
		continue;

	case 'H':	/* header title for pr */
		strcpy(title, line+1);
		continue;

	case 'L':	/* identification line */
		logname = line+1;
		if (OF) {
		    int p[2], i;
		    char *cp;

		    if (filter) {	/* wait for last one to complete */
			close(fo);
			while (wait(0) > 0)
			    ;
		    }

		    pipe(p);
		    if ((filter = dofork()) == 0) {	/* child */
			dup2(p[0], 0);		/* pipe is std in */
			dup2(lp, 1);		/* printer is std out */
			for (i = 3; i < NOFILE; i++)
				close(i);
			if ((cp = rindex(OF, '/')) == NULL)
				cp = OF;
			else
				cp++;
			execl(OF, cp, logname, 0);
			log("can't execl output filter %s", OF);
			exit(1);
		    }
		    fo = p[1];			/* use pipe for output */
		    close(p[0]);		/* close input side */
		} else {
		    fo = dup(lp);	/* use printer for output */
		    filter = 0;
		}
		if (SH)
			continue;
		time(&tvec);
		if (!tof)
			write(fo, FF, strlen(FF));
		write(fo, "\n\n\n", 3);
		banner(logname, jobname);
		if (strlen(class) > 0) {
			write(fo,"\n\n\n",3);
			scan_out(fo, class, '\0');
		}
		write(fo, "\n\n\n\n\t\t\t\t\t     Job:  ", 20);
		write(fo, jobname, strlen(jobname));
		write(fo, "\n\t\t\t\t\t     Date: ", 17);
		write(fo, ctime(&tvec), 24);
		write(fo, "\n", 1);
		write(fo, FF, strlen(FF));
		tof = 1;
		continue;

	case 'F':	 /* print formatted file */
		dump(0);
		title[0] = '\0';
		continue;

	case 'R':	 /* print file using 'pr' */
		dump(1);
		title[0] = '\0';	/* get rid of title */
		continue;

	case 'N':	/* file name for lpq */
	case 'U':	/* unlink deferred to pass2 */
		continue;

	}
/*
 * Second pass.
 * Unlink files
 */
	fseek(dfd, 0L, 0);
	while (getline()) switch (line[0]) {

	default:
		continue;
	
	case 'M':
		sendmail();
		continue;

	case 'U':
		unlink(&line[1]);
		continue;

	}
	/*
	 * clean-up incase another daemon file exists
	 */
	fclose(dfd);
	unlink(file);
}

/*
 * print a file.
 *  name of file is in line starting in col 2
 */
dump(prflag)
{
	register n, f;
	char buf[BUFSIZ];

	f = open(&line[1], 0);
	if (!SF && !tof)
		write(fo, FF, strlen(FF));       /* start on a fresh page */
	if (prflag && pr(f, fo) >= 0)
		tof = 1;
	else {
		while ((n=read(f, buf, BUFSIZ))>0)
			write(fo, buf, n);
		tof = 0;
	}
	close(f);
}

/*
 * pr - print a file using 'pr'
 */
pr(fi, fo)
{
	int pid, stat;
	char tmp[20];

	if ((child = dofork(DORETURN)) == 0) {	/* child - pr */
		dup2(fi, 0);
		dup2(fo, 1);
		for (fo = 3; fo < NOFILE; fo++)
			close(fo);
		sprintf(tmp, "-l%d", PL);
		execl(PRLOC, "pr", tmp, "-h", *title ? title : " ", 0);
		log("can't execl %s", PRLOC);
		exit(1);
	} else if (child < 0)			/* forget about pr'ing */
		return(-1);
	/* parent, wait */
	while ((pid = wait(&stat)) > 0 && pid != child)
		;
	child = 0;
	return(0);
}

dqcleanup()
{
	signal(SIGTERM, SIG_IGN);
	if (child > 0)
		kill(child, SIGKILL);	/* get rid of pr's */
	if (filter > 0)
		kill(filter, SIGTERM);	/* get rid of output filter */
	while (wait(0) > 0)
		;
	exit(0);			/* lprm removes the lock file */
}

getline()
{
	register int linel = 0;
	register char *lp = line;
	register c;

	/*
	 * reads a line from the daemon file, removes tabs, converts
	 * new-line to null and leaves it in line. returns 0 at EOF
	 */
	while ((c = getc(dfd)) != '\n') {
		if (c == EOF)
			return(0);
		if (c=='\t') {
			do {
				*lp++ = ' ';
				linel++;
			} while ((linel & 07) != 0);
			continue;
		}
		*lp++ = c;
		linel++;
	}
	*lp++ = 0;
	return(1);
}

/*
 * dofork - fork with retries on failure
 */
dofork(action)
{
	register int i, pid;

	for (i = 0; i < 20; i++) {
		if ((pid = fork()) < 0)
			sleep((unsigned)(i*i));
		else
			return(pid);
	}
	log("can't fork");

	switch(action) {
	case DORETURN:
		return(-1);
	default:
		log("bad action (%d) to dofork", action);
		/*FALL THRU*/
	case DOUNLOCK:
		unlink(LO);
		/*FALL THRU*/
	case DOABORT:
		exit(1);
	}
	/*NOTREACHED*/
}

/*
 * Banner printing stuff
 */

banner (name1, name2)
char *name1, *name2;
{
	scan_out(fo, name1, '\0');
	write(fo, "\n\n", 2);
	scan_out(fo, name2, '\0');
}

char *
scnline(key, p, c)
register char key, *p;
char c;
{
	register scnwidth;

	for(scnwidth = WIDTH; --scnwidth;) {
		key <<= 1;
		*p++ = key & 0200 ? c : BACKGND;
	}
	return(p);
}

#define TR(q)	(((q)-' ')&0177)

scan_out(scfd, scsp, dlm)
char *scsp, dlm;
int scfd;
{
	register char *strp;
	register nchrs, j;
	char outbuf[LINELEN+1], *sp, c, cc;
	int d, scnhgt;
	extern char scnkey[][HEIGHT];	/* in lpdchar.c */

	for(scnhgt = 0; scnhgt++ < HEIGHT+DROP;) {
		strp = &outbuf[0];
		sp = scsp;
		for(nchrs = 0;;) {
			d = dropit(c = TR(cc = *sp++));
			if ((!d && scnhgt>HEIGHT ) || (scnhgt<=DROP && d))
				for(j=WIDTH; --j;)
					*strp++ = BACKGND;
			else
				strp = scnline(scnkey[c][scnhgt-1-d], strp, cc);
			if(*sp==dlm || *sp=='\0' || nchrs++>=LINELEN/(WIDTH+1)-1)
				break;
			*strp++ = BACKGND;
			*strp++ = BACKGND;
		}
		while(*--strp== BACKGND && strp >= outbuf)
			;
		strp++;
		*strp++ = '\n';	
		write(scfd, outbuf, strp-outbuf);
	}
}

dropit(c)
char c;
{
	switch(c) {

	case TR('_'):
	case TR(';'):
	case TR(','):
	case TR('g'):
	case TR('j'):
	case TR('p'):
	case TR('q'):
	case TR('y'):
		return(DROP);

	default:
		return(0);
	}
}

/*
 * sendmail ---
 *   tell people about job completion
 */
sendmail()
{
	static int p[2];
	register int i;
	int stat;

	pipe(p);
	if ((stat = dofork(DORETURN)) == 0) {
		close(0);
		dup(p[0]);
		for (i=3; i <= NOFILE; i++)
			close(i);
		execl(MAIL, "mail", &line[1], 0);
		exit(0);
	} else if (stat > 0) {
		close(1);
		dup(p[1]);
		printf("To: %s\n", &line[1]);
		printf("Subject: printer job\n\n");
		if (*jobname)
			printf("Your printer job (%s) is done\n", jobname);
		else
			printf("Your printer job is done\n");
		fflush(stdout);
		close(1);
	}
	close(p[0]);
	close(p[1]);
	open(LF, 1);
	wait(&stat);
}

/*VARARGS1*/
log(message, a1, a2, a3)
char *message;
{
	short console = isatty(fileno(stderr));

	fprintf(stderr, console ? "\r\n%s: " : "%s: ", name);
	fprintf(stderr, message, a1, a2, a3);
	if (console)
		putc('\r', stderr);
	putc('\n', stderr);
	fflush(stderr);
}

init()
{
	char b[BUFSIZ];
	static char buf[BUFSIZ/2];
	static char *bp = buf;
	int status;

	if ((status = pgetent(b, printer)) < 0) {
		printf("%s: can't open printer description file\n", name);
		exit(3);
	} else if (status == 0) {
		printf("%s: unknown printer\n", printer);
		exit(4);
	}
	if ((LP = pgetstr("lp", &bp)) == NULL)
		LP = DEFDEVLP;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	if ((LF = pgetstr("lf", &bp)) == NULL)
		LF = DEFLOGF;
	AF = pgetstr("af", &bp);
	if ((PL = pgetnum("pl", &bp)) == NULL)
		PL = DEFPAGESIZE;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((FF = pgetstr("ff", &bp)) == NULL)
		FF = DEFFF;
	OF = pgetstr("of", &bp);
	SF = pgetflag("sf");
	SH = pgetflag("sh");
	if ((DU = pgetnum("du")) < 0)
		DU = DEFUID;
	setuid(DU);
}
