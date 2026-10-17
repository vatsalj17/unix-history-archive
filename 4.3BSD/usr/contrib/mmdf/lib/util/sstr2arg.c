
#include "util.h"

/*  convert string into argument list
 *
 *	This version takes an array of delimiters.	<DPK@BRL>
 *
 *  stash a pointer to each field into the passed array.
 *  any common seperators split the words.  extra white-space
 *  between fields is ignored.
 *
 *  specially-interpreted characters:
 *      double-quote, backslash (preceding a special char with a
 *	backslash removes its interpretation.  A backslash not
 *	followed by a special is used to preface an octal specification
 *	for one character a string begun with double-quote has only
 *      double-quote and backslash as special characters.
 */

extern int errno;

/* convert srcptr to argument list */
sstr2arg (srcptr, max, argv, dlmstr)
register char *srcptr;	/* source data */
int max;		/* maximum number of permitted fields */
char *argv[];		/* where to put the pointers */
char *dlmstr;		/* Delimiting character */
{
    char gotquote;      /* currently parsing quoted string */
    register int ind;
    register char *destptr;

    if (srcptr == 0) {
	errno = EINVAL;     /* emulate system-call failure */
	return (NOTOK);
    }

    for (ind = 0, max -= 2;; ind++) {
	if (ind >= max) {
	    errno = E2BIG;      /* emulate system-call failure */
	    return (NOTOK);
	}

    	/* Skip leading white space */
    	for (; *srcptr == '\t' || *srcptr == ' '; srcptr++);

	argv [ind] = srcptr;
	destptr = srcptr;

	for (gotquote = FALSE; ; ) {
	    register char *cp;

	    for (cp = dlmstr; (*cp != '\0') && (*cp != *srcptr); cp++);

	    if (*cp != '\0')
	    {
		if (gotquote) {           /* don't interpret the char */
		    *destptr++ = *srcptr++;
		    continue;
		}

		srcptr++;
		*destptr = '\0';
		goto nextarg;
	    } else {
		switch (*srcptr) {
		default:        /* just copy it                     */
			*destptr++ = *srcptr++;
			break;

		case '\"':      /* beginning or end of string       */
			gotquote = (gotquote) ? FALSE : TRUE;
			srcptr++;   /* just toggle */
			break;

		case '\\':      /* quote next character             */
			srcptr++;   /* just skip the back-slash         */
			if (*srcptr >= '0' && *srcptr <= '7') {
				*destptr = '\0';
				do
				    *destptr = (*destptr << 3) | (*srcptr++ - '0');
				while (*srcptr >= '0' && *srcptr <= '7');
				destptr++;
				break;
			}
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
