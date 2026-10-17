# include	"../unix.h"

/*
**  MALLOC -- allocate memory
**
**	Allocates memory from the free memory pool.  This routine
**	actually calls "alloc" to do the dirty work (and does not
**	exist at all if on version 7, since the system library
**	supplies this).
**
**	Parameters:
**		sz -- size of space to allocate.
**
**	Returns:
**		pointer to new space.
**		zero if no space available.
**
**	Side Effects:
**		as with 'alloc'.
**
**	History:
**		8/15/79 (eric) (6.2/7) -- added for v7 compatibility.
*/

# ifndef xV7_UNIX

char *
malloc(sz)
int	sz;
{
	register char	*p;

	p = alloc(sz);
	if (p == -1)
		p = 0;
	return (p);
}
# endif
