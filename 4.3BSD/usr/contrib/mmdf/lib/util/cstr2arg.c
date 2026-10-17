
#include "util.h"

/*  convert string into argument list
 *
 *  stash a pointer to each field into the passed array.
 *  any common seperators split the words.  extra white-space
 *  between fields is ignored.
 *
 *  if the separator is '=', then the current argument position in
 *  the array points to "=", the next the one is the key and the
 *  value follows it.  This permits detecting variable assignment,
 *  in addition to positional arguments.
 *      i.e.,  key=value ->  = key value
 *
 *  specially-interpreted characters:
 *      space, tab, double-quote, backslash, comma, equal, slash, period,
 *      semi-colon, colon, carriage return, and line-feed (newline).
 *      preceding a special char with a backslash removes its
 *      interpretation.  a backslash not followed by a special is used
 *      to preface an octal specification for one character
 *
 *      a string begun with double-quote has only double-quote and
 *      backslash as special characters.
 *
 *  a field which begins with semi-colon is interpreted as marking the
 *  rest of the line as a comment and it is skipped, as are blank lines
 */


/* Steve Kille
 * Modified version, which splits a string given a specific
 * separator
 */

extern int errno;

cstr2arg (srcptr, max, argv, dlmchar)/* convert srcptr to argument list */
	register char *srcptr;  /* source data */
	int max;                /* maximum number of permitted fields */
	char *argv[];           /* where to put the pointers */
	char dlmchar;           /* Delimiting character */
{
    char gotquote;      /* currently parsing quoted string */
    register int ind;
    register char *destptr;

    if (srcptr == 0)
    {
	errno = EINVAL;     /* emulate system-call failure */
	return (NOTOK);
    }

    for (ind = 0, max -= 2;; ind++)
    {
	if (ind >= max)
	{
	    errno = E2BIG;      /* emulate system-call failure */
	    return (NOTOK);
	}

	argv [ind] = srcptr;
	destptr = srcptr;

/**/

	for (gotquote = FALSE; ; )
	{
	    if (*srcptr == dlmchar)
	    {
		if (gotquote)
		{           /* don't interpret the char */
		    *destptr++ = *srcptr++;
		    continue;
		}

		srcptr++;
		*destptr = '\0';
		goto nextarg;
	    }
	    else
	    {
		switch (*srcptr)
		{
		    default:        /* just copy it                     */
			*destptr++ = *srcptr++;
			break;

		    case '\"':      /* beginning or end of string       */
			gotquote = (gotquote) ? FALSE : TRUE;
			srcptr++;   /* just toggle */
			break;

		    case '\\':      /* quote next character             */
			srcptr++;   /* just skip the back-slash         */
			if (*srcptr >= '0' && *srcptr <= '7')
			{
				*destptr = '\0';
				do
				    *destptr = (*destptr << 3) | (*srcptr++ - '0');
				while (*srcptr >= '0' && *srcptr <= '7');
				destptr++;
				break;
			}    /* otherwise DROP ON THROUGH */
			else
			     *destptr++ = *srcptr++;
			break;


		    case '\0':
			*destptr = '\0';
			ind++;
			argv[ind] = (char *) 0;
			return (ind);
		}
	    }
	}
    nextarg:
	continue;
    }
}

