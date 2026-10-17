#include "util.h"

/*                                                                      */
/*      Determine if the two given strings are equivalent.              */
/*                                                                      */

strequ (str1, str2)
register char   *str1,
	*str2;
{
    while (*str1 == *str2++)
    {
	if (*str1++ == '\0')
	    return (TRUE);
    }
    return (FALSE);
}
