
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
 *  a field which begins with semi-colon or a hash (#) is interpreted
 *  as marking the rest of the line as a comment and it is skipped, as
 *  are blank lines
 */

extern int errno;

str2arg (srcptr, max, argv, dlm)/* convert srcptr to argument list */
	register char *srcptr;  /* source data */
	int max;                /* maximum number of permitted fields */
	char *argv[];           /* where to put the pointers */
	char dlm[];             /* list of delimiting characters */
{
    char gotquote;      /* currently parsing quoted string */
    char lastdlm;       /* last delimeter character     */
    register int ind;
    register char *destptr;

    if (srcptr == 0)
    {
	errno = EINVAL;     /* emulate system-call failure */
	return (NOTOK);
    }

    for (lastdlm = (char) NOTOK, ind = 0, max -= 2; *srcptr != '\0'; ind++)
    {
	if (ind >= max)
	{
	    errno = E2BIG;      /* emulate system-call failure */
	    return (NOTOK);
	}
	for (argv[ind] = destptr = srcptr; isspace (*srcptr); srcptr++)
		;               /* skip leading white space         */
/**/

	for (gotquote = FALSE; ; )
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
		    switch (*srcptr)
		    {           /* convert octal values             */
			case 'r':
			    *destptr++ = '\r';
			    srcptr++;
			    break;

			case 'n':
			    *destptr++ = '\n';
			    srcptr++;
			    break;

			case 'b':
			    *destptr++ = '\b';
			    srcptr++;
			    break;

			case 'f':
			    *destptr++ = '\f';
			    srcptr++;
			    break;

			default:
			    if (*srcptr >= '0' && *srcptr <= '7')
			    {
				*destptr = '\0';
				do
				    *destptr = (*destptr << 3) | (*srcptr++ - '0');
				while (*srcptr >= '0' && *srcptr <= '7');
				destptr++;
				break;
			    }    /* otherwise DROP ON THROUGH */
			case '\\':
			case '\"':
			case ' ':
			case ',':
			case '\t':
			case ';':
			case '#':
			case ':':
			case '=':
			case '/':
			case '|':
			    *destptr++ = *srcptr++;
			    break; /* just copy it */
		    }       /* DROP ON THROUGH */
		    break;

		case '=':   /* make '=' prefix to pair  */
		    if (gotquote)
		    {
			*destptr++ = *srcptr++;
			break;
		    }

		    ind++;      /* put value ahead of '=' */

		    if ( dlm != (char *) 0)
		    {
			dlm[ind - 1] =
			    dlm[ind] = '=';
		    }
		    argv[ind] = argv[ind - 1];
		    lastdlm = '=';
		    argv[ind - 1] = "=";
		    *destptr = '\0';
		    srcptr++;
		    goto nextarg;

		case ' ':
		case '\t':
		case '\n':
		case '\r':
		case ',':
		case ';':
		case '#':
		case ':':
		case '/':
		case '|':
		    if (gotquote)
		    {           /* don't interpret the char */
			*destptr++ = *srcptr++;
			break;
		    }

		    if (isspace (lastdlm))
		    {
			if (isspace (*srcptr))
			{           /* shouldn't be possible */
			    errno = EINVAL;
			    return (NOTOK);
			}
			else
			    if (*srcptr != ';' && *srcptr != '#')
				ind--;  /* "xxx , yyy" is only 2 fields */
		    }
		    lastdlm = *srcptr;
		    if ( dlm != (char *) 0)
			dlm[ind] = *srcptr;
		    srcptr++;
		case '\0':
		    *destptr = '\0';
		    goto nextarg;
	    }

	    lastdlm = (char) NOTOK;     /* disable when field started */
	}

    nextarg:
	if (argv[ind][0] == '\0' && (lastdlm == ';' || lastdlm == '#'))
	    break;      /* rest of line is comment                  */
    }

    argv[ind] = (char *) 0;
    return (ind);
}
