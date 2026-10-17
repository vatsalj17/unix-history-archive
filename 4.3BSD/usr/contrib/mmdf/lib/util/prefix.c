#include "util.h"

/* lexical check if prefstr is at start of target */

prefix (prefstr, target)
register char  *target,
               *prefstr;
{
    for (; *prefstr; target++, prefstr++)
	if (uptolow (*target) != uptolow (*prefstr))
	    return (FALSE);

    return (TRUE);
}
