#
#include "hd.h"
/*  Display and cursor control routines */

#define BELL	07

int putch ();
extern char *CM, *CL, *UP, *tgoto ();		/* Joy's cursor stuff */

/*  Position cursor at the given (x, y) location.  Origin is
    lower left hand side.  */

atxy (x, y)  int x, y;  {

tputs (tgoto (CM, x, 23 - y), 0, putch);

}

/*  This subroutine takes one parameter.
    Position cursor at line ab from top at character cd.
    At (101) positions cursor at top left;
    At (2480) positions cursor at lower right.  */

at (abcd)  int abcd;  {

	int ab, cd;
	ab = abcd / 100;  cd = abcd - (ab * 100);
	atxy (cd - 1, 24 - ab);
}

erase () {
	tputs (CL, 0, putch);
	putch (CR);
}

/* erase chars on current line */
clearline () {
bufout ();
printf ("%c%80s%s", CR, "", UP);
unbufout ();
}

/* Position cursor at specified file */
atfile (file, col) int file, col; {

atxy (col, 21 - file);
}

/* buffering subroutines */

char outbuf [BUFSIZ];		/* the buffer */
int bcount = 0;			/* can buffer only once */
				/* additional attempts ignored */

bufout () {

if (bcount++ == 0) setbuf (stdout, outbuf);
}

unbufout () {

fflush (stdout);
if (--bcount == 0) setbuf (stdout, CNULL);
}

/* Clear message lines (23-24) for a pcount line message */
/* Lastcount keeps track of the lines with characters on them */
/* Dispdir calls with parameter -1 to reset lastcount */

clearmsg (pcount) int pcount; {

static int lastcount;

at (2301);
if (pcount == -1) lastcount = 0;
else {
	if (lastcount == 0);
	else if (lastcount == 1) {
		clearline ();
	} else {
		bufout ();
		printf ("%159s", "");  unbufout ();
	}
	lastcount = pcount;
}
at (2301);
}

/* Putmsg counts the number of lines in its parameter,
   calls clearmsg with that count, and then displays the message.  */

putmsg (msg) char * msg; {

	extern char * index ();

	clearmsg (index (msg, LF) == NULL ? 1 : 2);
	printf ("   %s", msg);
}

/* Beep bell on terminal */
beep () {
	putch (BELL);
}

/* Print error message about a file */
myperror (parm) char * parm; {

	extern int errno, sys_nerr;
	extern char *sys_errlist[];
	register char *c;

	c = "Unknown error";
	if(errno < sys_nerr)
		c = sys_errlist[errno];
	clearmsg (1);
	printf ("  %s: %s", parm, c);
}
