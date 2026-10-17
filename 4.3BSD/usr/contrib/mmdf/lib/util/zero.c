/*                                                                      */
/*      Zero out the given string of the given length.                  */
/*                                                                      */

zero (str, length)
register char   *str;
register short     length;
{
    while (length--)
	 *str++ = '\0';
}
