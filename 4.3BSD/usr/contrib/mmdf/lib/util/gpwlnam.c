#include "util.h"
#include <pwd.h>

/* modified getpwnam(), to do lexical (case-independent) comparison */

struct passwd *
gpwlnam(name)
char *name;
{
    struct passwd *getpwent();
    char lownam[30];
    register struct passwd *p;
    register int ind;

    for (ind = 0; ind < ((sizeof lownam) - 1) && !isnull (name[ind]); ind++)
	lownam[ind] = uptolow (name[ind]);
    lownam[ind] = '\0';

    for (setpwent(); (p = getpwent()) != NULL; )
    {
	for (ind = 0; !isnull (name[ind]); ind++)
	    p -> pw_name[ind] = uptolow (p -> pw_name[ind]);

	if (strcmp (lownam, p->pw_name) == 0)
	    break;
    }
    endpwent();
    return(p);
}
