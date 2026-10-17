#include "hd.h"
#include "command.h"

/* Interface to make.  Fmake forks make off and returns.  Wmake waits
   for completion.  Both cause output to be saved in the file .makerror.
*/
fmake () {

	int p;			/*	Process number */
	int mfile;		/*	File number of .makerror */
	FILE *mstream;		/*	Stream of .makerror */
	FILE *showopen ();	/*	Opener of mstream */

	mstream = showopen ("w", MAKEMODE);
	if (mstream == NULL) return NOREPLOT;
	mfile = fileno (mstream);

	if (myfork () == 0) {
		if ((p = myfork ()) == 0) {
			close (outfile);  close (errorfile);
			dup (mfile);  dup (mfile);
			close (infile);  open ("/dev/null", 0);

			myexecl (MAKE, MAKE, 0);
		} else {
			join (p);
			beep (); sleep (2); beep ();
			exit (0);
		}
	} else {
		fclose (mstream);
		putmsg (MAKE);
	}
	return NOREPLOT;
} 

wmake () {

	int p;			/*	Process number */
	FILE *mstream;		/*	Stream of .makerror */
	FILE *showopen ();	/*	Opener of mstream */
	register ch;		/*	Work character */

	int pipefile [2];	/*	Pipe file numbers */
#	define pipein	pipefile [0]
#	define pipeout	pipefile [1]

	FILE *pipestrm;		/*	Stream of pipein */

	mstream = showopen ("w", MAKEMODE);
	if (mstream == NULL) return NOREPLOT;

	tty_push (COOKEDMODE);
	pipe (pipefile);
	if ((p = myfork ()) == 0) {
		close (outfile);  close (errorfile);
		dup (pipeout);  dup (pipeout);
		myexecl (MAKE, MAKE, 0);
	} else {
		close (pipeout);
		pipestrm = fdopen (pipein, "r");
		while ((ch = getc (pipestrm)) != EOF) {
			putchar (ch);  putc (ch, mstream);
		}
		fclose (pipestrm);  fclose (mstream);
		join (p);
	}
	tty_pop ();
	return CMD_SE | REPLOT;
}
