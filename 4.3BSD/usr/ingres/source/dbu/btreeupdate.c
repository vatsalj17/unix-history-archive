# include	<ingres.h>
# include	<aux.h>
# include	<catalog.h>
# include	<symbol.h>
# include	<access.h>
# include	<batch.h>
# include	<btree.h>
# include	<sccs.h>

SCCSID(@(#)btreeupdate.c	8.3	1/18/85)

btreeupdate(r)
register DESC	*r;
{
	register char	*p;
	register int	i;
	int		j, domcnt, mode, dom;
	long		oldtid, newtid;
	long		tupcnt;
	long		uptid;
	char		oldtup[MAXTUP], newtup[MAXTUP];
	char		oldkey[2 * LIDSIZE], newkey[2 * LIDSIZE];
	char		dumtup[2 * LIDSIZE];
	struct relation	rkey, rtup;
	DESC		b_desc;
	extern DESC	Reldes;
	TID		tid, btid;
	char 		file[MAXNAME + 4], btree[MAXNAME + 4];
	long		oldlid[MAXLID], newlid[MAXLID];
	struct locator	tidpos;
	long		l;
	extern int	Btree_fd;
	char		*tp;
	long		page, t;
	int		lidwid;
	int		dellevel;
	int		compare;

#	ifdef xZTR1
	if (tTf(47, 0))
		printf("BTREEUPDATE\n");
#	endif

	mode = Batchhd.mode_up;
	Batch_dirty = FALSE;
	opencatalog("relation", OR_READ);
	capital(trim_relname(r->reldum.relid), file);
	setkey(&Reldes, &rkey, file, RELID);
	setkey(&Reldes, &rkey, r->reldum.relowner, RELOWNER);

	if (!getequal(&Reldes, &rkey, &rtup, &tid))
	{
		if (i = openr(&b_desc, OR_WRITE, file))
			syserr("btreeupdate:can't openr %.12s %d", file, i);
		/* reposition batch file to the beginning. */
		if ((i = lseek(Batch_fp, 0L, 0)) < 0)
			syserr("secupdate:seek %d %d", i, Batch_fp);
		Batch_cnt = BATCHSIZE;
		getbatch(&Batchhd, sizeof Batchhd);	/* reread header */

		ingresname(r->reldum.relid, r->reldum.relowner, file);
		btreename(file, btree);
		if ((Btree_fd = open(btree, O_RDWR)) < 0)
			syserr("btreeupdate: can't open %s", btree);

		/*
		** Start reading the batch file and updating
		** the secondary indexes.
		*/
		l = r->reladds;
		tupcnt = Batchhd.num_updts;
		lidwid = LIDSIZE * r->reldum.reldim;
		dellevel = r->reldum.reldim - 1;
		for (j = 0; j < r->reldum.reldim; ++j)
		{
			if (Repl_cnt[j] > 0)
			{
				dellevel = j;
				break;
			}
		}
		while (tupcnt--)
		{
			getbatch(&oldtid, Batchhd.tido_size);
			getbatch(oldtup, Batchhd.tupo_size);
			getbatch(newtup, Batchhd.tupn_size);
			getbatch(&newtid, Batchhd.tidn_size);

			clearkeys(&b_desc);
			/* if this is a replace or append form the new key */
			if (mode != mdDEL)
			{
				if (newtid < 0)
					continue;
				tp = newtup + Batchhd.tupn_size - lidwid;
				bmove(tp, newlid, lidwid);
				if (mode == mdREPL)
				{
					if (newlid[r->reldum.reldim - 1] < 0)
						continue;
					tp = oldtup + Batchhd.tupo_size - lidwid;
					bmove(tp, oldlid, lidwid);
					compare = 0;
					for (j = 0; j < r->reldum.reldim; ++j)
					{
						if (newlid[j] > oldlid[j])
						{
							compare = 1;
						}
						if (newlid[j] != oldlid[j])
							break;
					}
					if (compare == 1)
					{
						for (j = dellevel - 1; j >= 0; --j)
						{
							if (newlid[j] != oldlid[j])
							{
								compare = 0;
								break;
							}
						}
					}
					if (compare == 1)
						/* adjust due to deleted lids */
						newlid[dellevel] -= Repl_cnt[dellevel];
				}
				page  =  RT;
				for (j = 0; j < r->reldum.reldim; ++j)
				{
					if (!newlid[j])
						newlid[j] = 1;
					t = get_tid(page, newlid[j], &tidpos);
					page = t;
				}
				if (page != newtid)
				{
					/* try linear search of btree */
					lin_search(r->reldum.reldim, newtid, &btid, newlid, Batchhd.num_updts);
					setkey(&b_desc, newkey, &newtid, 1);
				}
				else
				{
					setkey(&b_desc, newkey, &newtid, 1);
					stuff_page(&btid, &tidpos.pageno);
					btid.line_id = tidpos.page.node.leafnode.tid_loc[tidpos.offset];
				}
				setkey(&b_desc, newkey, &btid, 2);
#				ifdef xZTR1
				if(tTf(47,0))
				{
					printf("new key\n");
					printup(&b_desc, newkey);
				}
#				endif
			}

			/* if this is delete or replace form the old key */
			if (mode != mdAPP)
			{
				setkey(&b_desc, oldkey, &oldtid, 1);
#				ifdef xZTR1
				if(tTf(47,0))
				{
					printf("old key\n");
					printup(&b_desc, oldkey);
				}
#				endif
			}

			switch (mode)
			{

			  case mdDEL:
				if (i = getequal(&b_desc, oldkey, dumtup, &uptid))
				{
					if (i > 0)
						break;
					syserr("btreeupdate:getequal %d", i);
				}
				if ((i = delete(&b_desc, &uptid)) < 0)
					syserr("btreeupdate:delete %d", i);
				break;

			  case mdREPL:
				/* btree tid not provided */
				b_desc.relgiven[2] = 0;
				if (i = getequal(&b_desc, oldkey, dumtup, &uptid))
				{
					if (Batch_recovery && i > 0)
						goto btreeinsert;
					printup(&b_desc, oldkey);
					syserr("btreeupdate:getequal-repl %d", i);
				}
				/* btree tid provided */
				b_desc.relgiven[2] = 1;
				if (i = replace(&b_desc, &uptid, newkey, TRUE))
				{
					/* if newtuple is dup of old, ok */
					if (i == 1)
						break;
					/* if this is recovery and old tid not there, try an insert */
					if (Batch_recovery && i == 2)
						goto btreeinsert;
					syserr("secupdate:replace %d", i);
				}
				break;

			  case mdAPP:
			  btreeinsert:
				if ((i = insert(&b_desc, &uptid, newkey, TRUE)) < 0)
					syserr("secupdate:insert %d", i);
			}
		}
		if (i = closer(&b_desc))
			syserr("btreeupdate:closer %.12s %d", file, i);
		close(Btree_fd);
	}
	if (i < 0)
		syserr("btreeupdate:bad get from indexes %d", i);
}
