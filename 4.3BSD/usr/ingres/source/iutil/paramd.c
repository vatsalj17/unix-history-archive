# include	<ingres.h>
# include	<aux.h>
# include	<catalog.h>
# include	<access.h>
# include	<sccs.h>

SCCSID(@(#)paramd.c	8.2	2/8/85)



/*
**	PARAMD - get access parameters of a relation from its descriptor	
**		and return them in struct pointed to by "ap".
**
**	Parameters:
**		d - descriptor
**		ap - struct which holds the parameters
**
**	Return Codes:
**			0
**
*/

paramd(d, ap)
register DESC			*d;
register struct accessparam	*ap;
{
	register int	i;
	int		p;


	ap->mode = getmode(d->reldum.relspec);
	ap->sec_index = FALSE;	/* indicate that this isn't the index-rel */

	for (i = 0; i < MAXDOM+1; i++)
		ap->keydno[i] = 0;

	for (p = 1; p <= d->reldum.relatts; p++)
		if (i = d->relxtra[p])
			ap->keydno[i-1] = p;
	return (0);
}


/*
**	PARAMI -get indexes of a relation from  the index struct
**		and place them in the structure pointed to by param
**
**	Parameters:
**		ip - index struct
**		param - holds info about indexes
**
**	Return Codes:
**		0 -- successful
**
*/
parami(ip, param)
struct index		*ip;
struct accessparam	*param;
{
	register struct accessparam	*ap;

	ap  = param;
	ap->mode = getmode(ip->irelspeci);
	ap->sec_index = TRUE;	/* this is an index */

	bmove(ip->idom, ap->keydno, MAXKEYS);
	ap->keydno[MAXKEYS] = 0;
	return(0);
}

/*
** 	GETMODE - get the mode of the storage type	
**
**	Parameters:
**		spec - storage type
**
*/

getmode(spec)
int	spec;
{
	switch (abs(spec))
	{
	  case M_HEAP:
		return(NOKEY);

	  case M_ISAM:
		return(LRANGEKEY);

	  case M_HASH:
		return(EXACTKEY);

	  default: 
		syserr("getmode:bad relspec %d", spec);
	}
}
