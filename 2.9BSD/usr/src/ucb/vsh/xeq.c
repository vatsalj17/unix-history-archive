#include "hd.h"
#include <signal.h>

/*  F_exec forks and executes a process.  It then waits for the
    forked process to finish.
    Use f_exec like execl, except f_exec waits for termination of the
    called program and then falls through.
*/

extern leave ();

/*VARARGS1*/
f_exec (a, b) char *a, *b;  {

int retval;
int p;

tty_push (COOKEDMODE);
if ((p = myfork ()) == 0) myexecv (a, &b);
else retval = join (p);
tty_pop ();
return retval;
}

/* Exec takes parameters like a command and interfaces properly
   to the command processor in command.c. */

exec (argv) char **argv; {

	register p;

	if (*argv == CNULL) {
		putmsg ("Execute command must have parameter");
		return NOREPLOT;
	}
	tty_push (COOKEDMODE);

	if ((p = myfork ()) == 0) myexecv (*argv, argv);
	else join (p);

	getrtn ();
	tty_pop ();
	return REPLOT;
}
/* Mysystem is similar to system, except the bugs are fixed.  In
   addition, the tty is set to cooked mode and the command is printed.
*/
mysystem (name)  char *name; {

	int pipefile[2];
#	define pipein	pipefile [0]
#	define pipeout	pipefile [1]

	int p; FILE *stream;

	tty_push (COOKEDMODE);  pipe (pipefile);

	if ((p = myfork()) == 0) {
		close (infile);  dup (pipein);
		myexecl (envshell, "+", 0);
	}
	else {
		stream = fdopen (pipeout, "w");
		close (pipein);
		printf ("%s\n", name);
		fprintf (stream, "%s\n", name);
		fclose (stream);
		join (p);
	}
	tty_pop ();
}

/* p_exec is just like f_exec except output is paged */

/*VARARGS1*/
p_exec (a, b) char *a, *b;  {

int pipefile [2];

int p;
FILE *stream;

pipe (pipefile);
if ((p = myfork ()) == 0) {
	close (outfile);  dup (pipeout);
	close (pipein);   close (pipeout);
	myexecv (a, &b);
}
else {
	stream = fdopen (pipein, "r");
	close (pipeout);
	page (stream);
	fclose (stream);
	join (p);
}
}

/* Special interfaces to exec */
/* Myexecl and myexecv close files numbered > 3 and
   print their arguments. */

/*VARARGS1*/
myexecl (a, b) char *a, *b; {
	myexecv (a, &b);
}

myexecv (a, b) char *a, **b; {

register char **sp;  register i;

if (**b != '+') {
	for (sp = b; *sp; sp++) printf ("%s ", *sp);
	printf ("\r\n");
}
for (i = 3; i <= _NFILE; i++) close (i);
execv (a, b);
myperror (a); getrtn (); exit (1);
}

/* Myfork acts like fork but never fails */
#define	MAXTRIES	10	/* Max tries of a fork */
myfork () {
	int p;			/* process number */
	int tries;		/* number of tries */

	for (tries = 1; tries <= MAXTRIES; tries++) {
		p = fork ();
		if (p > 0)  signal (SIGINT, SIG_IGN);
		if (p == 0) signal (SIGINT, SIG_DFL);
		if (p != -1) return p;
		myperror ("Cannot fork");
		sleep (tries);
		clearmsg (0);
	}
	putmsg ("Fatal error -- cannot fork\n");
	leave ();
return -1;
}

/* Join is the compliment of fork */

join (p) int p; {

	int status [2];  int w;

	do {
		w = wait (status);
	} while (p != -1 && w != p);

	signal (SIGINT, leave);

	return (status [0]);
}
