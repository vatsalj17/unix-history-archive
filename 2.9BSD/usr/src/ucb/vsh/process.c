#
/*  This is the main loop of vsh.  Each interation processes a command.
    Commands are one character long.  They are acquired in raw mode
    and processed without the need to press return.  The goal of vsh
    is to minimize keypresses as much as possible.
*/

#include "hd.h"

process () {

	register cmd, next;		/* single character command */

	next = REPLOT;
	for (;;) {			/* loop forever */
		if (next & REPLOT) dispdir (1);
		at (2301);
		printf (" \b");
		cmd = getch ();
		putch ('\b');

		next = command (cmd, DIRCMD);
	}
}
