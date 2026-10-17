# include	<sccs.h>

SCCSID(@(#)length.c	8.1	12/31/84)

/*
**  FIND STRING LENGTH
**
**	The length of string `s' (excluding the null byte which
**		terminates the string) is returned.
*/

length(s)
char	*s;
{
	register int	l;
	register char	*p;

	l = 0;
	p = s;
	while (*p++)
		l++;
	return(l);
}
