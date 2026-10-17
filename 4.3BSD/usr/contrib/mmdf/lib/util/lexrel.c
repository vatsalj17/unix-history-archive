
lexrel (str1, str2)
register char   *str1,
		*str2;
{
    extern char chrcnv[];

    while (chrcnv[*str1] == chrcnv[*str2++])
	if (*str1++ == '\0')
	    return (0);           /* exactly equal                      */
    if (chrcnv[*str1] < chrcnv[*--str2])
	return (-1);              /* str1 earlier in lexicon            */
    return (1);                   /* str2 is earlier                    */
}
