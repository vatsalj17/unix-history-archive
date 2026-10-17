# include	<useful.h>
# include	<sccs.h>

SCCSID(@(#)bequal.c	8.2	2/5/85)

/*
**  BLOCK EQUALITY TEST
**
**	blocks `a' and `b', both of length `l', are tested
**		for absolute equality.
**	Returns one for equal, zero otherwise.
*/

bequal(a, b, l)
register char	*a, *b;
register int	l;
{
	if ( a == (char *) 0 || b == (char *) 0 )
		return ( FALSE );

	while (l-- > 0)
		if (*a++ != *b++)
			return(FALSE);
	return(TRUE);
}
