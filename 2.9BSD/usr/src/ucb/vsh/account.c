#include "hd.h"

/* This records each usage of Vsh in the file LOGFILE */
/* Each entry is in the form "User Time(minutes) Date" */

long time (), ent_time [1];
char *ctime ();

comein () {
time (ent_time);
}

goout () {

/* this proceedure was used upon exit to see who was using
   vsh, for the moment comment out the procedure.
	long lv_time [1];
	char *namep;
	register char *cp;
	FILE *logfile;

	time (lv_time);
	for (namep = cp = envhome; *cp; cp++)
		if (*cp == '/') namep = cp;
	namep++;
	if (compe (namep, "scheibel")) return;
	logfile = fopen (LOGFILE, "a");
	fprintf (logfile, "%8.8s %3.3d %10.10s\n",
		namep, (*lv_time - *ent_time)/60, ctime (ent_time));
	fclose (logfile); */
}

leave () {			/* exit after resetting tty */
	tty_cooked ();
	goout ();
	exit (0);
}
