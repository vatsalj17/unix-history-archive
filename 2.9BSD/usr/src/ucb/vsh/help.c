#include "hd.h"

/* Help displays a file in paged mode.  The parm is the file name. */

help (parm) char *parm; {

FILE *helpfile;

helpfile = fopen (parm, "r");
if (helpfile == NULL) {
	myperror (parm);
	return FAILURE;
}
else page (helpfile);
fclose (helpfile);
return SUCCESS;
}

/* Display is the command processor's version of help */
display (argv) char **argv; {
	char name [STRMAX];

	if (*argv == CNULL) putmsg ("Display:  ");

	if (getfname (*argv, name) == FAILURE) return FAILURE;
	else return help (name);
}
