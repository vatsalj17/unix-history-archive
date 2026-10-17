#include "util.h"

/* Modified gcread to use stdio. Jim Lieb SRI International Aug 1980 */
/*   returns -1 for error, 0 for eof, positive with count for success */
/*   Successive calls will return error or eof after first encounter  */
/*   Neither eof nor error will be indicated until buffer is emptied  */
/* Modified by Phil Cockcroft UCL Mar 85 to handle quoted strings properly */

gcread (fp, obuf, len, endchrs)
FILE  *fp;
int       len;                  /* maximum allowed to xfer              */
char   *obuf,                   /* destination buffer                   */
       *endchrs;                /* list of chars to terminate on        */
                                /*   "abcdef\377"                       */
{
	return (qread (fp, obuf, len, endchrs, 0));
}

qread (fp, obuf, len, endchrs, quotechar)
FILE  *fp;
int       len;                  /* maximum allowed to xfer              */
char   *obuf,                   /* destination buffer                   */
       *endchrs;                /* list of chars to terminate on        */
                                /*   "abcdef\377"                       */
char    quotechar;              /* a single character quote             */
{
    char *bptr;
    register int c;
    register short cnt;
    register char *end;

    if (feof (fp))
      return (OK);

    if (ferror (fp))
      return (NOTOK);

    for (bptr = obuf, cnt = 0; cnt < len; cnt++)
    {
        if ((c = getc(fp)) == EOF)      /* maybe end-of-file            */
        {
            if (ferror (fp))
                return ((cnt == 0) ? NOTOK : cnt);
            if (feof (fp))
                return (cnt);
        }
        if(quotechar != '\0' && c == quotechar)
        {
            if ((c = getc(fp)) == EOF)      /* maybe end-of-file        */
            {
                if (bptr != 0)              /* what do we do here ????  */
                    *bptr = quotechar;
                if (ferror (fp))
                    return ((cnt == 0) ? NOTOK : cnt);
                if (feof (fp))
                    return (cnt);
                /* EH ? I hope we will never get here */
            }
            if (bptr != 0)              /*  0 => just skip the text     */
                *bptr++ = c;            /*  xfer one char               */
            continue;
        }
        if (bptr != 0)                  /*  0 => just skip the text     */
            *bptr++ = c;                /*  xfer one char               */

        if (end = endchrs)              /*  check if it's in breakset   */
            while ((int)*end != (-1))	/*  Grrrrrr.... */
                if (c == *end++)
                    return ++cnt;
    }
    return (cnt);                        /* return num xferred           */
}
