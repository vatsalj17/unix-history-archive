# include	<sccs.h>

SCCSID(@(#)min.c	8.1	12/31/84)

/*
**  MIN -- return the minimum of two integers.
**
**	Why should I even have to write this????
*/

min(a, b)
int	a;
int	b;
{
	return ((a < b) ? a : b);
}
