#include "hd.h"
#include <signal.h>

static int nointer;

page (stream) FILE *stream; {

int line, linelim;  register ch;
register ttych, ttych1;

int (*oldsig)(); extern catch();

#define	window	23
#define	scroll	10

linelim = window;  ch = 1;
tty_push (COOKEDMODE);
oldsig = signal (SIGINT, catch);  nointer = 1;

bufout ();  erase ();
do {
	for (line = 0; line < linelim; line++) {
		while ((ch = getc (stream)) != EOF && ch != '\n')
			putchar (ch);
		if (ch == EOF) break;
		putch ('\n');  fflush (stdout);
	}
	if (!nointer) break;
	printf ((ch == EOF) ? "Done:  " : "More:  ");
	fflush (stdout);

	ttych = ttych1 = getch ();
	while (ttych != EOF && ttych != '\n') ttych = getch ();
	if (ttych == EOF) {
		linelim = scroll;
		printf ("\r          \r");
		fflush (stdout);
	} else {
		linelim = window;
		erase ();
	}

} while (ch != EOF && ttych1 != 'n' && nointer);
unbufout ();  tty_pop ();  signal (SIGINT, oldsig);
}

catch () {		/* Catch an interrupt */
	nointer = 0;
	signal (SIGINT, catch);
}
