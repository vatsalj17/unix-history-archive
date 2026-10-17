#include "hd.h"
#include <sgtty.h>

/* This module controls tty options.

   Tty states are treated as a stack.  Use tty_push and tty_pop.

   Push COOKEDMODE and RAWMODE onto the stack.  RAWMODE is now actually
   CBREAK mode.
*/

struct sgttyb newstty, oldstty;		/* sgtty structures */
int ttysp;				/* Sp for tty stack */
extern short ospeed;			/* Speed for Joy's routines */

#define	TTYLIM	16
char tty_stack [TTYLIM];		/* The tty stack */

/* Initialize tty structures */

tty_init () {

	gtty (infile, &oldstty); gtty (infile, &newstty);
	oldstty.sg_flags &= ~(CBREAK | RAW);
	newstty.sg_flags |= CBREAK;
	ospeed = oldstty.sg_ospeed;

	ttysp = 0;  tty_set (COOKEDMODE);
}

/* Push a new tty mode.  Use RAWMODE and COOKEDMODE as parameters. */

tty_push (pmode) int pmode; {
	ttysp++;
	if (ttysp >= TTYLIM) {
		putmsg ("Tty stack overflow\n");
		leave ();
	}
	tty_set (pmode);
}

tty_pop () {
	ttysp--;
	if (ttysp < 0) {
		putmsg ("Tty stack underflow\n");
		leave ();
	}
	tty_set (tty_stack [ttysp]);
}

/* Tty_set sets the tty mode indicated by its parameter.  Stty is only
   executed if a change will result.
*/

tty_set (pmode) int pmode; {

	static int curmode = -1;	/* Current tty mode */

	tty_stack [ttysp] = pmode;
	if (pmode == curmode) return;
	curmode = pmode;
	if (pmode == RAWMODE) tty_raw ();
	else tty_cooked ();
}

tty_raw () { stty (infile, &newstty); }

tty_cooked () { stty (infile, &oldstty); }
