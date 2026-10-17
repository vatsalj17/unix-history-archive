#include "util.h"

/* convert an array of strings to one or more lines of 'arguments'.
 *
 * this is intended to be used along with the str2arg() routine.
 */

/*VARARGS*/
#ifdef NO_VARARGS
arg2lstr (linelen, maxlen, buf, arg1, a,b,c,d,e,f,g,h,i,j,k,l,m)
#else
arg2lstr (linelen, maxlen, buf, arg1) /* convert list to argument array */
#endif NO_VARARGS
    int linelen;        /* when to insert newlines; 0=> don't       */
    int maxlen;
    char *buf;
    char *arg1;		/* merely the first of the list		    */
{
    arg2vstr (linelen, maxlen, buf, &arg1);
}

arg2vstr (linelen, maxlen, buf, argv) /* convert the list to a string            */
    int linelen;
    int maxlen;	/* length of string			*/
    char *buf;	/* where to put the output string	*/
    char **argv;	/* the argument vector			*/
{
    unsigned totlen,    /* total length of current line         */
	     len;       /* length of current argument           */
    unsigned gotdelim;       /* a delimiter char is in arg           */
    unsigned gotpair;        /* key/value pair                       */
    char tmpstr[256];	/* LINESIZE = 256, string under construction  */
    register char *src,
		  *dest;

    for (totlen = gotpair = 0, buf[0] = '\0';
		*argv != (char *) 0; argv++)
    {
	if (gotpair  == 0 && strcmp ("=", *argv) == 0)
	{
	    dest = tmpstr;
	    gotpair = 2;     /* take the next two arguments  */
	    continue;
	}

	for (src = *argv, gotdelim = FALSE; *src != '\0'; src++)
	    switch (*src)
	    {
		case ' ':
		case '\t':
		case '=':
		case ',':
		case ';':
		case ':':
		case '/':
		case '|':
		case '.':
		    gotdelim = TRUE;
		    goto nextone;
	    }

    nextone:
	if (gotpair == 0)
	    dest = tmpstr;
	if (gotdelim)
	    *dest++ = '"';
	for (src = *argv; *src != '\0'; src++)
	{
	    switch (*src)
	    {
		case '\b':
		    *dest++ = '\\';
		    *dest++ = 'b';
		    break;

		case '\t':
		    *dest++ = '\\';
		    *dest++ = 't';
		    break;

		case '\f':
		    *dest++ = '\\';
		    *dest++ = 'f';
		    break;

		case '\r':
		    *dest++ = '\\';
		    *dest++ = 'r';
		    break;

		case '\n':
		    *dest++ = '\\';
		    *dest++ = 'n';
		    break;

		case '\\':		/*  Added by Doug Kingston  */
		case '\"':
		    *dest++ = '\\';
		    *dest++ = *src;
		    break;

		default:
		    if (iscntrl (*src))
		    {
			*dest++ = '\\';
			*dest++ = ((*src >> 6) & 07) + '0';
			*dest++ = ((*src >> 3) & 07) + '0';
			*dest++ =  (*src & 07) + '0';
		    }
		    else
			*dest++ = *src;
	    }
	}

	if (gotdelim)
	    *dest++ = '"';

	if (src == *argv)       /* null-valued argument data            */
	{
	    if (totlen != 0)    /* beyond the beginning, so double it   */
		strcat (buf, ",");
	    else
		strcat (buf, " ");
	    *dest++ = ',';
	}

	switch (gotpair)        /* handle key/value differently         */
	{
	    case 2:
		*dest++ = '=';
		gotpair--;
		continue;

	    case 1:
		gotpair = 0;
	}

	*dest = '\0';
	len = dest - tmpstr;

	if (totlen != 0)
	{
	    if ((linelen > 0) && ((totlen + len) > linelen))
	    {
		strcat (buf, "\n\t");
		totlen = 8;
	    }	
	    else
	    {
		strcat (buf, " ");
		totlen += 1;
	    }
	}
	strcat (buf, tmpstr);
	totlen += len;
    }
}
