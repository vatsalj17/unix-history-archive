# include	<useful.h>
# include	<sccs.h>

SCCSID(@(#)sequal.c	8.2	2/5/85)

/*
**  SEQUAL -- string equality test
**
**	null-terminated strings `a' and `b' are tested for
**		absolute equality.
**	returns one if equal, zero otherwise.
*/

sequal(a, b)
register char	*a, *b;
{
	while (*a || *b)
		if (*a++ != *b++)
			return(FALSE);
	return(TRUE);
}
