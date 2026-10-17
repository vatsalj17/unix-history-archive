static char Sccsid[] "@(#)zeropad	1.1";
/*
	Replace initial blanks with '0's in `str'.
	Return `str'.
*/

zeropad(str)
char *str;
{
	register char *s;

	for (s=str; *s == ' '; s++)
		*s = '0';
	return(str);
}
