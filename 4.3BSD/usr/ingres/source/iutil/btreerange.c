# include	<ingres.h>
# include	<btree.h>
# include	<sccs.h>

SCCSID(@(#)btreerange.c	8.1	12/31/84)

/*
**	Btreerange finds the smallest and largest tids corresponding to
**	the lids found between two given lids.
*/

btreerange(d, low_lid, high_lid, lotid, hitid)

DESC *d;
long low_lid[], high_lid[];
register TID *lotid, *hitid;
{
	register int 	i;
	long		l, h, tid, temp;
	struct locator  block;
	long		page, t, last, hbtid, last_lid();
	int		done, first;
	long		start, next;

	/* find tid corresponding to high lid */
	page = RT;
	for (i = 0; i < d->reldum.reldim; ++i)
	{
		if ((t = get_tid(page, high_lid[i], &block)) < 0)
			syserr("get_tid error in btreerange, lid = %ld\n", high_lid[i]);
		page = t;
	}
	hbtid = page;
	/* find starting point of scan */
	page = RT;
	for (i = 0; i < d->reldum.reldim - 1; ++i)
	{
		last = last_lid(page) - 1;
		if (low_lid[i] > last)
			low_lid[i] = last;
		if ((t = get_tid(page, low_lid[i], &block)) < 0)
			syserr("get_tid error in btreerange, lid = %ld\n", low_lid[i]);
		page = t;
	}
	first = 1;
	last = last_lid(page) - 1;
	if (low_lid[d->reldum.reldim - 1] > last)
		low_lid[d->reldum.reldim - 1] = last;
	start = low_lid[d->reldum.reldim - 1];
	do
	{
		get_node(page, &block.page);
		next = block.page.nexttree;
		if ((tid = get_tid(page, start, &block)) < 0)
			syserr("get_tid error in btreerange, lid = %ld\n", low_lid[d->reldum.reldim - 1]);
		/* set high and low to intial value */
		if (first)
		{
			first = 0;
			pluck_page(&tid, &l);
			pluck_page(&tid, &h);
		}
		page = block.pageno;
		done = 0;
		while (done == 0)
		{
			for (i = 0; i < block.page.nelmts && done == 0; ++i)
			{
				tid = block.page.node.leafnode.tid_pos[block.page.node.leafnode.tid_loc[i]];
				pluck_page(&tid, &temp);
				if (temp > h)
					h = temp;
				else if (temp < l)
					l = temp;
				if (tid == hbtid)
					done = 1;
			}
			page = block.page.node.leafnode.nextleaf;
			if (page == NULL)
				done = 1;
			else
				get_node(page, &block.page);
		}
		start = 1;
	} while (page = next && !done);
	stuff_page(lotid, &l);
	stuff_page(hitid, &h);
	lotid->line_id = hitid->line_id = -1;
}
