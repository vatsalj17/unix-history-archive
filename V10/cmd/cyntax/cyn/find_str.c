/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"

#define	STABZ	251

st_node		*stab[STABZ];

/*
 *	Map the string 's' to its string table entry, creating a new one
 *	if required.
 */
st_node	*
find_str(s)
register char	*s;
{
	register int		i;
	register st_node	**n;
	register char		*p;
	register int		length;

	for (i = 0, length = 1, p = s; *p != '\0'; i += i ^ *p++)
		length++;

	if (i < 0)
		i = -i;

	n = &stab[i % STABZ];

	while (*n != NULL)
	{
		register char	*q;

		p = s;
		q = (*n)->st_name;

		while ((i = *p ^ *q) == 0 && *p++ != '\0' && *q++ != '\0')
			;

		if (i == 0)
			return *n;

		n = i & 1 ? &((*n)->st_left) : &((*n)->st_right);
	}

	*n = talloc(st_node);
	(*n)->st_name = str_alloc(s, length, &((*n)->st_index));
	(*n)->st_left = NULL;
	(*n)->st_right = NULL;
	(*n)->st_idlist = NULL;

	return *n;
}
