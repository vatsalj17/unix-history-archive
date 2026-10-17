#include	"cem.h"

/*
 *	Allocate some memory.
 */
char	*
salloc(n)
long	n;
{
	register char	*p;
	extern char	*malloc();

	if ((p = malloc((unsigned int)n)) == NULL)
	{
		fprint(2, "%s: ran out of memory\n", my_name);
		exit(1);
	}

	return p;
}
/*
 *	Reallocate some memory.
 */

char	*
srealloc(p, n)
register char	*p;
long		n;
{
	extern char	*realloc();

	if (p == NULL)
		return salloc(n);
	else if ((p = realloc(p, (unsigned int)n)) == NULL)
	{
		fprint(2, "%s: ran out of memory\n", my_name);
		exit(1);
	}

	return p;
}

/*
 *	Refill the alloc buffer.
 */
char	*
alloc_fill(want)
long	want;
{
	register long	sz;

	if (want < ALLOC_SIZE)
		sz = ALLOC_SIZE;
	else
		sz = want;

	alloc_ptr = salloc(sz);
	alloc_end = alloc_ptr + sz;
	alloc_ptr += want;
	return alloc_ptr - want;
}

/*
 *	Fetch 4 byte number, native byte ordering.
 *	(so .O must be made & read on same type of machine)
 *
 *	Previous integer compression packing scheme bogus,
 *	takes over roles of getu() and getv()
 */
long
getnum()
{
	register long	j;
	j = *((long*)data_ptr);
	skip4();
	return j;
}

