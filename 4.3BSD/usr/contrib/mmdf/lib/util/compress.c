#include "util.h"
/* compress-out redundant linear white space & strip trailing lwsp */

char *
	compress (fromptr, toptr)
register char *fromptr,
	      *toptr;
{
    register char   chr;

    chr = ' ';                    /* init to skip leading spaces  */
    while ((*toptr = *fromptr++) != '\0')
    {                             /* convert newlines and tabs to */
	if (isspace (*toptr))
	{                         /* only save first space      */
	    if (chr != ' ')
		*toptr++ = chr  = ' ';
	}
	else
	    chr = *toptr++;
    }

    if (chr == ' ')               /* remove trailing space if any */
	*--toptr = '\0';
    return (toptr);
}
