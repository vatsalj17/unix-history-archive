# include	<ingres.h>
# include	<access.h>
# include	<catalog.h>
# include	<btree.h>
# include	<sccs.h>

SCCSID(@(#)insert_mbtree.c	8.1	12/31/84)

insert_mbtree(d, btree, lid, tid, tidpos)

register DESC *d;
char *btree;
long lid[];
long *tid;
TID *tidpos;
{
	register int 	i, j;
	long		page;
	long		l;
	long		last_lid();
	struct locator	tidloc;
	long		ntid;
	long		t;
	struct BTreeNode	junk;

	page = RT;
	for (i = 0; i < d->reldum.reldim - 1; ++i)
	{
		if (lid[i] && lid[i+1])
		{
			if ((t = get_tid(page, lid[i], &tidloc)) < -1)
				return(-1);
			if (t != -1)
				page = t;
		}
		if (!lid[i] || t == -1 || !lid[i+1])
		{
			for (j = i; j < d->reldum.reldim - 1; ++j)
			{
				if (j != i)
					lid[j] = 1;
				else if (t == -1 || !lid[i])
					lid[j] = last_lid(page);
				insert_btree(btree, page, lid[j], &ntid, tidpos, j + 2);
				page = ntid;
			}
			lid[d->reldum.reldim - 1] = 1;
			break;
		}
	}
	l = last_lid(page);
	if (lid[d->reldum.reldim - 1] < 0 || lid[d->reldum.reldim - 1] >  l)
		return(-1);
	if (!lid[d->reldum.reldim - 1])
		lid[d->reldum.reldim - 1] = l;
	insert_btree(btree, page, lid[d->reldum.reldim - 1], tid, tidpos, 0);

	return (0);
}
