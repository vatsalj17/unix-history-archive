# include	<ingres.h>
# include	<access.h>
# include	<catalog.h>
# include	<batch.h>
# include	<btree.h>
# include	<sccs.h>

SCCSID(@(#)delete.c	8.1	12/31/84)

/*
**	Delete - delete the tuple specified by tid
**
**	Delete removes the tuple specified by tid
**	and reclaims the tuple space.
**
**	returns:
**		<0  fatal error
**		0   success
**		2   tuple specified by tid aleady deleted
*/

delete(dx, tidx)
DESC	*dx;
TID	*tidx;
{
	register DESC	*d;
	register TID	*tid;
	register int	i;
	char		btree[MAXNAME + 4];
	TID		tidloc;
	long		lid[MAXLID];

	d = dx;
	tid = tidx;

#	ifdef xATR1
	if (tTf(24, 8))
	{
		printf("delete: %.14s,", d->reldum.relid);
		dumptid(tid);
	}
#	endif

	if (i = get_page(d, tid))
		return (i);

	if (i = invalid(tid))
		return (i);

	i = tup_len(tid);

	del_tuple(tid, i);

	if (d->reldum.reldim > 0)
	/* remove corresponding lid from B-Tree */
	{
		btreename(d->reldum.relid, btree);
		search_btree(*tid, &tidloc);
		get_lid(&tidloc, lid);
		if (fwrite(lid, 1, LIDSIZE * d->reldum.reldim, Del_infp) != LIDSIZE * d->reldum.reldim)
			syserr("write error");
		++Del_cnt;
	}

	d->reladds--;

	return (0);
}
