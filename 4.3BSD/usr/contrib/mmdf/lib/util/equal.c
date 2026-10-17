/*                                                                      */
/*      Determine whether the two given strings have an identical       */
/*  prefix of the given length.                                         */
/*                                                                      */

equal (str1, str2, length)
register char   *str1,
		*str2;
short              length;
{
    extern char chrcnv[];

    while (chrcnv[*str1++] == chrcnv[*str2++] && --length > 0);
    return (length == 0);
}
