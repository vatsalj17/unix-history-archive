#include "hd.h"
#include "strings.h"
#include "command.h"

/* Interface to grep */

#define GBUFLEN 200
#define LIM1	(gbuf + 100)

#define QUOTE	'\''
#define BSLASH	'\\'

#define GSTR1	" -n '"
#define GSTR2	"' /dev/null "

#define CPSET	{cp = gbuf + strlen (gbuf);}

#define	GREPLEAVE	{tty_pop ();  clearmsg (0);  return NOREPLOT;}

/* User is asked to supply patterns and file names.  Grep is then
   run with its output directed to .grepout.  If the user changes his
   mind, and leaves without running grep, NOREPLOT is returned.
   Else REPLOT | CMD_SG is returned.
*/
grep () {
	register char *cp, *clim;  register ch;
	char gbuf [GBUFLEN];
	char inline [STRMAX];  int inlength;  register char *incp;

	FILE *sstream;
	extern FILE *showopen ();

	int saveout;

	extern char wdname [];

	tty_push (COOKEDMODE);
	strcpy (gbuf, GREP);  strcat (gbuf, GSTR1);
	clearmsg (2);  printf ("Grep pattern: ");
	CPSET;  clim = LIM1;

	inlength = getline (inline);  incp = inline;
	if (inlength == 0) GREPLEAVE;

	while ((ch = *incp++) && cp < clim) {
		if (ch == QUOTE) {
			*cp++ = QUOTE;  *cp++ = BSLASH;
			*cp++ = QUOTE;  *cp++ = QUOTE;
		} else *cp++ = ch;
	}
	*cp=0;
	strcat (gbuf, GSTR2);
	at (2317 + inlength);  printf ("--  Grep files: ");

	CPSET;
	inlength = xgetline (stdin, cp, GBUFLEN - strlen (gbuf));
	if (inlength == 0) GREPLEAVE;

	/* Now run the command in gbuf */

	sstream = showopen ("w", GREPMODE);
	if (sstream == NULL) GREPLEAVE;

	printf ("Searching\n");

	saveout = dup (outfile);	/* Set up files */
	close (outfile); dup (fileno (sstream)); fclose (sstream);

	printf ("%s is search directory\n", wdname);
	mysystem (gbuf);
	close (outfile);  dup (saveout);  close (saveout);


	tty_pop ();  return CMD_SG | REPLOT;
}
