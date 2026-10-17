/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"
#include	"sort.h"

/*
 *	Quicksort.
 */
quicksort(base, n, qcmp)
register sorts	*base;
register int	n;
int		(*qcmp)();
{
    while (n > 1)
    {
	if (n < 5)
	{
	    register int	i;
	    register int	j;
	    sorts	temp;

	    /*
	     *	Bubblesort quicker on < 5 elements.
	     *	At most 6 comparisons.
	     */
	    i = n;
	    while (--i > 0)
	    {
		j = i;
		while (--j >= 0)
		{
		    if ((*qcmp)(&base[i], &base[j]) < 0)
		    {
			temp = base[j];
			base[j] = base[i];
			base[i] = temp;
		    }
		}
	    }
	    return;
	}
	else
	{
	    register sorts	*q;
	    register sorts	*b;
	    register sorts	*t;
	    register int	m;
	    sorts		temp;

	    /*
	     *	Make the pivot.
	     */
	    q = base + n / 2;
	    temp = *q;
	    b = base;
	    t = base + n - 1;
	    *q = *t;

	    /*
	     *	Segment the entries about the pivot.
	     */
	    while (b != t)
	    {
		while ((*qcmp)(b, &temp) < 0)
		{
		    if (++b == t)
			goto finish;
		}
		*t = *b;

		do
		{
		    if (--t == b)
			goto finish;
		}
		while ((*qcmp)(t, &temp) > 0);

		*b++ = *t;
	    }
	finish:
	    *b = temp;
	    m = n;
	    n = b - base;
	    m -= n + 1;

	    /*
	     *	Recurse on smaller side.
	     */
	    if (n > m)
	    {
		if (m > 1)
		    quicksort(b + 1, m, qcmp);
	    }
	    else
	    {
		if (n > 1)
		    quicksort(base, n, qcmp);
		base = b + 1;
		n = m;
	    }
	}
    }
}

sorts	*sort_vect;
int	sort_size;
int	sort_index;

/*
 *	Extend the sort vector.
 */
sort_extend()
{
    sort_size += SBUFFZ;
    sort_vect = vector(sort_vect, sort_size, sorts);
}
