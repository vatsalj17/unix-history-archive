#
/***************************************************************************\
* 									    *
*     R C V F I L E . C							    *
* 									    *
*     Written by Dave Crocker.						    *
*     Rewritten by J. Onions, combined the original two programs into	    *
*     one to make documentation easier, not to mention updates etc.	    *
*     most of the original text kept.					    *
*     Reworked again by Doug Kingston to make secure and clean.		    *
* 									    *
*     usage:   rcvfile directory [-l logfile] [-m]			    *
*	"directory" specifies where the files will be created under	    *
*	"-l logfile" specifies the logging file for the program		    *
*       "-m" instructs the program to make directories if necessary	    *
*									    *
*     Message has a Subject: line of the form				    *
*     Subject: rcvfile filename						    *
* 									    *
*     The body of the message will always be filed under the "directory"    *
*     specified (".." is trapped).  This can be made "/" to enable	    *
*     fileing anywhere.							    *
* 									    *
\***************************************************************************/

#include "util.h"
#include "mmdf.h"
#include <pwd.h>
#include <sys/stat.h>

extern int errno;
extern int sentprotect;
extern char *locname;

extern char *compress();
extern char *rindex();

long    filesize;
int	makedirs = 0;
char	*filedir = (char *)0;
char	*logfile = (char *)0;
char	dirname[LINESIZE];
char	filename[LINESIZE];
char	subjbuf[LINESIZE];
char	frombuf[LINESIZE];
char	datebuf[LINESIZE];
char	buffer[BUFSIZ];
char	inbuf[BUFSIZ];

/**/
main (argc, argv)
int	argc;
char	**argv;
{
    register int nread;
    register int fd;
    register int i;
    FILE *output;

    mmdf_init (argv[0]);
    setbuf (stdin, inbuf);

    init_log();

    for (i = 1; i < argc; i++) {
    	if (argv[i][0] != '-') {
	    if (filedir != (char *)0) {
		fputs ("  *** directory already specified\n", stderr);
		finish (RP_PARM);
	    }
	    filedir = argv[i];
	} else {
	    if (strcmp (argv[i], "-l") == 0) {
	    	if (i+1 == argc) {
		    fputs ("  *** missing logfile after -l\n", stderr);
		    finish (RP_PARM);
		}
	    	logfile = argv[++i];
	    } else if (strcmp(argv[i], "-m") == 0) {
		makedirs++;
	    } else {
		fprintf (stderr, "  *** unknown parameter '%s'\n", argv[i]);
		finish (RP_PARM);
	    }
	}
    }

    if (filedir == (char *)0) {
	fputs ("  *** missing filing directory\n", stderr);
	finish (RP_PARM);
    }
    sprintf(filename, "%s/", filedir);

    /*
     *  parse the header, find interesting info and save it.
     */
    while (fgets (buffer, sizeof buffer, stdin) != NULL && buffer[0] != '\n') {
	if (equal ("to", buffer, 2) || equal ("cc", buffer, 2))
	    fputs (buffer, stderr);
				  /* log "to" and "cc" information */
	if (equal ("date", buffer, 4))
	    compress (&buffer[strindex (":", buffer) + 1], datebuf);
				  /* save date    text in buffer        */
	if (equal ("from", buffer, 4))
	    compress (&buffer[strindex (":", buffer) + 1], frombuf);
				  /* save from    text in buffer        */
	if (equal ("subject", buffer, 7))
	    compress (&buffer[strindex (":", buffer) + 1], subjbuf);
				  /* save subject text in buffer        */
    }

    /* We accept "rcvarch" for backwords compat, and its the same size */
    if (!equal ("rcvfile", subjbuf, 7) && !equal ("rcvarch", subjbuf, 7))
    {                             /* doesn't have the command word      */
	fputs ("  *** Subject lacks keyword\n", stderr);
	finish (RP_PARM);
    }

    compress (&subjbuf[7], subjbuf);
    if ((strlen(subjbuf)+strlen(filename)+1) > sizeof filename) {
	fputs ("  *** filename too long\n", stderr);
	finish (RP_PARM);
    }

    /*
     *  Must not contain ".." and must not be null, we also make
     *  some other sanity/security checks.
     */
    if (subjbuf[0] == '\0'
      || strcmp(subjbuf, ".") == 0
      || strcmp(subjbuf, "..") == 0
      || initstr("../", subjbuf, 3)
      || strindex(subjbuf, "/../") != (-1)
      || endstr("/..", subjbuf, 3)) {
	fputs ("  *** Illegal filename\n", stderr);
	finish (RP_PARM);
    }

    strcat (filename, subjbuf);
    dirpart (filename);

    if ((fd = creat (filename, sentprotect)) < OK) {
	if( !makedirs ) {
	    fperror("  *** creat");
	    finish (RP_FCRT);
	} else {
	    if (creatdir (dirname, 0755, 0, 0) < 0) {
		fprintf( stderr, "%s *** unsuccessful mkdir\n",
			 dirname);
		finish (RP_FCRT);
	    }
	}
	if ((fd = creat (filename, sentprotect)) < 0) {
	    fperror("  *** creat after mkdir");
	    finish (RP_FCRT);
	}
    }
    if (chmod (filename, sentprotect) < 0) {
	fperror("  *** chmod");
	finish (RP_FIO);
    }

    if ((output = fdopen (fd, "a")) == NULL) {
	fperror("  *** fdopen");
	finish (RP_FIO);
    }
    filesize = 0;
    while ((nread = fread (buffer, sizeof(char), sizeof buffer, stdin)) != 0) {
	if (fwrite (buffer, sizeof(char), nread, output) != nread) {
	    fperror("  *** File output error");
	    finish (RP_FIO);
	}
    	filesize += nread;
    }
    if (ferror(stdin)) {
	fperror("  *** file input error");
	fclose (output);
	unlink (filename);
	finish (RP_FIO);
    }
    fclose (output);
    fprintf (stderr, "  (%ldc)\n", filesize);
    notify (datebuf, frombuf, filename, filesize);
    finish (RP_MOK);

    /* NOTREACHED */
}
/**/

finish (retval)
    int retval;
{
    fprintf (stderr, "%s\n\n", rp_valstr (retval));
    fflush (stderr);

    exit (retval == RP_MOK ? 0 : RP_MECH);
}

notify (thedate, thefrom, file, size)
char	*thedate, *thefrom, *file;
long	size;
{
    extern struct passwd *getpwuid ();
    struct stat statbuf;
    struct passwd *owner;
    char linebuf[2*LINESIZE];

    if (stat(dirname, &statbuf) < 0 ||
	(owner = getpwuid(statbuf.st_uid)) == NULL) {
	fprintf (stderr, "  *** get owner error (%d)\n", errno);
    }

    sprintf (linebuf, "[%s]%s got %ld characters.\n\nFrom %s, sent %s.\n",
	     locname, file, size, thefrom, thedate);

    if (ml_1adr (NO, NO, "FILE SERVER", (char *) 0, owner -> pw_name) < OK ||
	    ml_txt (linebuf) < OK ||
	    ml_end (OK) < OK )
	fprintf (stderr, "  *** notification error (%d)\n", errno);
}

dirpart (path)
char *path;
{
    register char *ptr;

    strcpy (dirname, path);
    if (ptr = rindex(dirname+1, '/'))
	*ptr = '\0';
    else
	dirname[0] = '\0';
}

init_log()
{
    if( access(logfile, 02) == 0)
    {
	freopen (logfile, "a", stderr);
    }
}

fperror(s)
char *s;
{
	fprintf(stderr, "%s: errno %d\n", s, errno);
}
