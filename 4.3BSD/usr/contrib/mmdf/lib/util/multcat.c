#include "util.h"
#include <varargs.h>

/*                                                                      */
/*              Concatenate a series of arrays                          */

/*VARARGS0*/
char *multcat (va_alist)
va_dcl
{
	register va_list ap;
	register char  *oldstr, *ptr;
	char    *newstr;
	extern char *malloc();
	unsigned  newlen;

	va_start(ap);
	for (newlen = 1; oldstr = va_arg(ap, char *);)
		newlen += strlen (oldstr);
	va_end(ap);

	if ((ptr = newstr = malloc (newlen+1)) == 0)
	    return((char *)0);

	va_start(ap);
	for (; oldstr = va_arg(ap, char *); ptr--)
		while(*ptr++ = *oldstr++);
	va_end(ap);

	return (newstr);
}
