#
/*      Copy the given string of the given length to the given          */
/*  destination string.                                                 */
/*                                                                      */

char *
	blt (from, to, length)
register char   *from,
		*to;
register int       length;
{
    if(length != 0)
	do {
	    *to++ = *from++;
	} while(--length);
    return (to);
}
