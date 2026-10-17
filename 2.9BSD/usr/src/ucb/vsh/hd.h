/* Header file for all procedures in Vsh */
#include "stdio.h"

#define VERSION	2.5

#define max(arg1,arg2)	((arg1 > arg2) ? arg1 : arg2)
#define min(arg1,arg2)	((arg1 < arg2) ? arg1 : arg2)
#define compe(arg1,arg2)  (strcmp (arg1, arg2) == 0)

/* Standard file numbers */
#define infile		0
#define outfile		1
#define errorfile	2

/* The values of special keys */
#define EOT		4
#define	RUBOUT		0177
#define	CR		015
#define	LF		012

/* Standard file names */
#define LOGFILE		"/usr/grad/scheibel/vsh/log"
#define DEBUGGER	"/bin/adb"

/* Other parameters */
#define	STRMAX	120	/* Length of string buffers */
#define CNULL	((char *) 0)	/* Null char pointer */
#define ARGVMAX		20	/* Size of a readarg argv */

/* Parms loadable through .vshrc are accessed through command.h.
   Alter their default values in cmdini.c	*/

/* Tty_set parameters */
#define RAWMODE		0
#define	COOKEDMODE	1

/* Pointers to data about the environment */
extern char *envshell, *envhome, *envedit, *envterm;

/* Functions called by command return the next command to execute.  */
/* In addition, the following bits are returned.  */

#define CMDMASK		0x00ff	/* Bits of next command */
#define	REPLOT		0x0100	/* Must replot directory */
#define NOOP		0x0200	/* Return for command not found */
#define ENTERDIR	0x0400	/* New directory entered */

/* If no special return is necessary, use return REPLOT or NOREPLOT. */
#define NOREPLOT	0x0000

/* Failure and success may be a double meaning for REPLOT/NOREPLOT.
   Check individule commands.  */

#define SUCCESS		REPLOT
#define	FAILURE		NOREPLOT

/* When calling command, indicate type of command:  */
#define DIRCMD	1
#define SHOWCMD	2

/* Show operates in two modes */
#define GREPMODE	0
#define MAKEMODE	1

/* Parameters from .vshrc file. P_name is the parameter name, p_val is
   the parameter's value.
*/

struct parmstruct {
	char *p_name,
	     *p_val;
};

extern struct parmstruct  parmtab[];

/* References to the various parameters */

#define	EDITOR		parmtab[0].p_val
#define	MAKE		parmtab[1].p_val
#define	GREP		parmtab[2].p_val
#define	RMHELP		parmtab[3].p_val
#define	SHOWHELP	parmtab[4].p_val
#define	MAKERROR	parmtab[5].p_val
#define	GREPOUT		parmtab[6].p_val
