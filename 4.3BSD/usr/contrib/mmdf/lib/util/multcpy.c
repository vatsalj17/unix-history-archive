#include <varargs.h>

/*                                                                      */
/*      Copy the list of strings to the given location.                 */
/*                                                                      */

char *
multcpy (to, va_alist)
register char   *to;
va_dcl
{
	register va_list ap;
	register char   *from;

	va_start(ap);

	while(from = va_arg(ap, char *)){
		while(*to++ = *from++);
		to--;
	}
	va_end(ap);
	return (to);            /* return pointer to end of str         */
}

