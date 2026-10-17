#include "util.h"
#include "conf.h"
#include "ch.h"
#include "dm.h"
#include "ll_log.h"

extern char *strdup();
extern char *malloc();
extern struct ll_struct *logptr;

char *
ap_dmflip (buf)
char * buf;
{
    char tbuf [LINESIZE];
    char *cp;
    int argc;
    char *argv [DM_NFIELD];
    int  i;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "Flipping domain '%s'", buf);
#endif
    if ((buf[0] == '"') || (buf[0] == '['))
			/*, don't flip if quoted  or dlit */
	return (strdup (buf));

    (void) strcpy (tbuf, buf);
    argc = cstr2arg (tbuf, DM_NFIELD, argv, '.');

    cp = malloc (strlen (buf) + 1);
    if (cp == (char *) 0) {
	ll_log (logptr, LLOGTMP, "dm_flip - malloc failure");
	return ((char *) 0);
    }
    (void) strcpy (cp, argv [argc - 1]);
    for (i = argc - 2; i >= 0; i--) {
	strcat (cp, ".");
	strcat (cp, argv [i]);
    }

#ifdef DEBUG
   ll_log (logptr, LLOGFTR, "Flipped domain is '%s'", cp);
#endif
   return (cp);
}
