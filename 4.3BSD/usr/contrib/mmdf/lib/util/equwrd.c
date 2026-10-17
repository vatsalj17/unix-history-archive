#include "util.h"
/*                                                                      */
/*      Determine whether the two given strings begin with the same     */
/*  alphabetic prefix of the given length.                              */
/*                                                                      */

equwrd (wrd1, wrd2, length)
register char   *wrd1,
		*wrd2;
int              length;
{
    extern char chrcnv[];

    while (chrcnv[*wrd1++] == chrcnv[*wrd2++] && --length > 0);
    return (length == 0 && !isalpha (*wrd1) && !isalpha (*wrd2));
}
