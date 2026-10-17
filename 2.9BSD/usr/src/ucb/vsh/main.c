#include "hd.h"
#include "mydir.h"
#include <signal.h>

/* Extract data about environment */
#define ENV_COUNT	3

char *env_defalt [ENV_COUNT] =			/* Defaults */
	{"/bin/csh", "/", "unknown"};
char *env_name   [ENV_COUNT] =			/* Names */
	{"SHELL", "HOME", "TERM"};

char *envshell, *envhome, *envterm;

char **env_dest [ENV_COUNT] =			/* Place stored */
	{&envshell, &envhome, &envterm};

main () {

/*  Initialize everything, then run.  */

int i;
extern char * mygetenv ();

extern leave ();

comein ();

for (i = 0; i < ENV_COUNT; i++)
	*env_dest [i] = mygetenv (env_name [i], env_default [i]);

signal (SIGINT, leave);
tty_init (); curs_init ();
erase (); printf ("Vsh %0.1f\n", VERSION);
cmdldrc ();
wdfile = -1;
if (curdir () || enterdir (DOT) == FAILURE) leave ();
tty_push (RAWMODE);

process ();
}

/* Return environment value of pstr.  If none found, or the string
   length is too long, the default returned */

char *mygetenv
(pstr, def)
char *pstr, *def; {

char *rstr;	/* return value */
char *getenv ();

rstr = getenv (pstr);
if ((rstr == NULL) || (strlen (rstr) >= STRMAX)) return def;
else return rstr;
}
