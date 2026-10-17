#include "util.h"

/*                                                                      */
/*      Create a duplicate copy of the given string.                    */
/*      Modified for V7 Unix by Jim lieb SRI International Aug 80       */

char *
	strdup (str)
register char   *str;
{
    extern char *malloc ();
    register char  *newptr,
		   *newstr;

    if ((newstr = malloc ((unsigned) (strlen (str) + 1))) == 0)
	return ((char *) 0);

    for (newptr = newstr; *newptr++ = *str++; );

    return (newstr);
}
