#include "util.h"
#include <grp.h>

/* modified getgrnam(), to do lexical (case-independent) comparison */

struct group  *
ggrlnam(name)
char *name;
{
    struct group *getgrent();
    char lownam[30];
    register struct group *g;
    register int ind;

    for (ind = 0; ind < ((sizeof lownam) - 1) && !isnull (name[ind]); ind++)
	lownam[ind] = uptolow (name[ind]);
    lownam[ind] = '\0';

    for (setpwent(); (g = getgrent()) != NULL; )
    {
	for (ind = 0; !isnull (name[ind]); ind++)
	    g -> gr_name[ind] = uptolow (g -> gr_name[ind]);

	if (strcmp (lownam, g->gr_name) == 0)
	    break;
    }
    endpwent();
    return(g);
}
