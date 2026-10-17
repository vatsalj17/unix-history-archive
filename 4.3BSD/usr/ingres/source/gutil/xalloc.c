# include	<useful.h>
# include	<sccs.h>

SCCSID(@(#)xalloc.c	8.1	12/31/84)

/*
**  XALLOC -- allocate block of memory.
**
**	This is just like malloc, except that it is guaranteed
**	to succeed.  It will syserr if it fails.
**
**	Parameters:
**		sz -- size in bytes of memory area to allocate.
**
**	Returns:
**		pointer to area allocated.
**
**	Side Effects:
**		none.
**
**	Trace Flags:
**		none.
*/

char *
xalloc(sz)
int	sz;
{
	register char	*p;
	extern char	*malloc();

	p = malloc(sz);
	if (p == NULL)
		syserr("Out of memory");
	return (p);
}
