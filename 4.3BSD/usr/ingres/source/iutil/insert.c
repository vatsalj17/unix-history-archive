# include	<ingres.h>
# include	<access.h>
# include	<catalog.h>
# include	<btree.h>
# include	<sccs.h>

SCCSID(@(#)insert.c	8.1	12/31/84)

/*
**	INSERT - add a new tuple to a relation
**
**	Insert puts a given tuple into a relation in
**	the "correct" position.
**
**	If insert is called with checkdups == TRUE then
**	the tuple will not be inserted if it is a duplicate
**	of some already existing tuple. If the relation is a
**	heap then checkdups is made false.
**
**	Tid will be set to the tuple id where the
**	tuple is placed.
**
**	returns:
**		<0  fatal error
**		0   success
**		1   tuple was a duplicate
**		2   bad lid
*/

insert(d, tid, tuple, checkdups)
register DESC	*d;
register TID	*tid;
char		*tuple;
bool		checkdups;
{
	register int	i, j;
	int		need;
	char		*tp;
	long		lid[MAXLID], l;
	char		btree[MAXNAME + 4];
	TID		tidpos;
	struct locator	tidloc;
	short		nolid;

#	ifdef xATR1
	if (tTf(24, 0))
	{
		printf("insert:%.14s,", d->reldum.relid);
		dumptid(tid);
		printup(d, tuple);
	}
#	endif

	if (d->reldum.reldim != 0)
		checkdups = FALSE;

	/* determine how much space is needed for tuple */
	need = canonical(d, tuple);

	/* find the "best" page to place tuple */
	if (i = findbest(d, tid, tuple, need, checkdups))
		return (i);

	if (d->reldum.reldim > 0)
	/* get lids and check for errors */
	{
		btreename(d->reldum.relid, btree);
		tp = tuple + d->reldum.relwid - LIDSIZE * d->reldum.reldim;
		bmove(tp, lid, LIDSIZE * d->reldum.reldim);
		nolid = 0;
		for (i = 0; i < d->reldum.reldim; ++i)
		{
			if (lid[i] < 0 || (lid[i] > 0 && nolid))
				return(2);
			nolid = !(lid[i]);
		}
	}

	if (d->reldum.reldim > 0)
	{
		if (insert_mbtree(d, btree, lid, tid, &tidpos) < 0)
			return(2);
		tp = tuple + d->reldum.relwid - LIDSIZE * d->reldum.reldim;
		bmove(lid, tp, d->reldum.reldim * LIDSIZE);
	}

	/* put tuple in position "tid" */
	put_tuple(tid, Acctuple, need);

	d->reladds++;

	return (0);
}
