#include "hd.h"

date () {		/* display current date */

	long datetime [1];

	time (datetime); clearmsg (1);
	printf ("  %.24s", ctime (datetime));
	return NOREPLOT;
}

callshell (argv) char **argv; {

	if (argv[0])  {
		mysystem (argv[0]);
		getrtn ();
	}
	else f_exec (envshell, envshell, 0);
	return REPLOT;
}
