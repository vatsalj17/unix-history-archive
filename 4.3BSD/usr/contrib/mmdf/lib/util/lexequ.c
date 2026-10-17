#include "util.h"

/*                                                                      */
/*      Determine if the two given strings are equivalent.              */
/*                                                                      */

lexequ (str1, str2)
register char   *str1,
		*str2;
{
    extern char chrcnv[];

    if(str1 == 0 || str2 == 0)
	return (str1 == str2);

    while (chrcnv[*str1] == chrcnv[*str2++])
	if (*str1++ == 0)
	    return (TRUE);

    return (FALSE);
}
