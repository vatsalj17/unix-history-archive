# include	<sccs.h>

SCCSID(@(#)IIlength.c	8.1	12/31/84)



/*
**	determines the length of a string.
**	if a null byte cannot be found after 255 chars
**	the the length is assumed to be 255.
*/

IIlength(s)
char	*s;
{
	register char	*ss;
	register int	len, cnt;

	ss = s;
	cnt = 255;
	len = 0;

	while (cnt--)
		if (*ss++)
			len++;
		else
			break;
	return (len);
}
