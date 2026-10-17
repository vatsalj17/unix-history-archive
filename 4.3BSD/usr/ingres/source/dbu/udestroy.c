# include	<ingres.h>
# include	<aux.h>
# include	<catalog.h>
# include	<btree.h>
# include	<pv.h>
# include	<sccs.h>

SCCSID(@(#)udestroy.c	8.2	1/15/85)


/*
**  USERDESTROY -- auxiliary cleanup for destroy of a user relation
**
**	userdestroy is called during the destroy of a non system
**	relation. If the relation is indexed or is itself an index
**	then the appropriate action is taken. If it is indexed,
**	then all secondary indices on the relation are also destroyed.
**	If it is a secondary index, then the entry in the indexes relation
**	is removed and the "relindxd" bit on the primary relation is
**	cleared if this was the last index on the relation.
**
**	If the relation was a view or had integrity constraints or
**	protection constraints on it, then those definitions are
**	removed from the appropriate system catalogues.
**
**	Parameters:
**		reltup -- the relation relation tuple.
**
**	Returns:
**		none
**
**	Side Effects:
**		zero or more system catalogues will be updated.
**
**	Called By:
**		destroy
*/

userdestroy(reltup)
struct relation	*reltup;
{
	register int			i;
	register struct relation	*rel;
	struct tup_id			tid, limtid;
	char				newrelname[MAXNAME + 3];
	extern DESC			Reldes, Attdes, Inddes;
	extern DESC			Treedes, Intdes, Prodes;
	struct relation			relt, relk;
	struct index			indk, indt;
	char				btree[MAXNAME + 4];
	char				*trim_relname();
	PARM				pv[2];

	rel = reltup;

	/* handle special case of destroying a secondary index */
	if (rel->relindxd == SECINDEX)
	{
		opencatalog("indexes", OR_WRITE);
		setkey(&Inddes, &indk, rel->relid, IRELIDI);
		setkey(&Inddes, &indk, rel->relowner, IOWNERP);
		if ((i = getequal(&Inddes, &indk, &indt, &tid)) != 0)
			syserr("destroy: geteq(ind,%.12s) %d", rel->relid, i);

		/* remove entry in INDEX catalog */
		bmove(indt.irelidp, newrelname, MAXNAME);
		bmove(indt.iownerp, &newrelname[MAXNAME], 2);
		if ((i = delete(&Inddes, &tid)) != 0)
			syserr("DESTROY: delete(ind/%.12s) %d", rel->relid, i);
		clearkeys(&Inddes);
		setkey(&Inddes, &indk, newrelname, IRELIDP);
		setkey(&Inddes, &indk, &newrelname[MAXNAME], IOWNERP);

		/* reset relindxd field in relation catalog if no other indexes exist on this primary */
		if (getequal(&Inddes, &indk, &indt, &tid) != 0)
		{
			clearkeys(&Reldes);
			setkey(&Reldes, &relk, newrelname, RELID);
			setkey(&Reldes, &relk, &newrelname[MAXNAME], RELOWNER);
			if (i = getequal(&Reldes, &relk, &relt, &tid))
				syserr("destroy: getequal(rel, %s) %d", newrelname, i);
			relt.relindxd = 0;
			if ((i = replace(&Reldes, &tid, &relt, 0)) != 0)
				syserr("destroy: replace(rel) %d", i);
		}
	}

	if (rel->reldim > 0)
	{
		/* remove old B-Tree file */
		btreename(rel->relid, btree);
		if (unlink(btree) < 0)
			syserr("userdestroy: unlink %s", btree);
		/* remove btreesec file */
		capital(trim_relname(rel->relid), btree);
		pv[0].pv_val.pv_str = btree;
		pv[1].pv_type = PV_EOF;
		if (destroy(1, pv))
			syserr("can't destroy btreesec");
	}

	/* check special case of destroying primary relation */
	if (rel->relindxd > 0)
	{
		opencatalog("indexes", OR_WRITE);
		setkey(&Inddes, &indk, rel->relid, IRELIDP);
		setkey(&Inddes, &indk, rel->relowner, IOWNERP);
		if (i = find(&Inddes, EXACTKEY, &tid, &limtid, &indk))
			syserr("destroy: find(ind,%.12s) %d", rel->relid, i);
		while ((i = get(&Inddes, &tid, &limtid, &indt, TRUE)) == 0)
		{
			if (kcompare(&Inddes, &indk, &indt) != 0)
				continue;
			if ((i = delete(&Inddes, &tid)) != 0)
				syserr("DESTROY: delete(ind/%.12s) %d", rel->relid, i);
			clearkeys(&Reldes);
			purgetup(&Reldes, indt.irelidi, RELID, indt.iownerp, RELOWNER, 0);
			if (i = flush_rel(&Reldes, FALSE))	/* flush for recovery & concurrency reasons */
				syserr("destroy:flush irel %d", i);
			purgetup(&Attdes, indt.irelidi, ATTRELID, indt.iownerp, ATTOWNER, 0);
			ingresname(indt.irelidi, indt.iownerp, newrelname);
			if (unlink(newrelname))
				syserr("destroy: unlink(%s)", newrelname);
		}
		if (i < 0)
		{
			syserr("destroy: get(ind) %d", i);
		}
	}

	/* if any integrity constraints exist, remove them */
	if (rel->relstat & S_INTEG)
	{
		opencatalog("integrities", OR_WRITE);
		purgetup(&Intdes, rel->relid, INTRELID, rel->relowner, INTRELOWNER);
	}

	/* if any protection clauses exist, remove them */
	if (rel->relstat & S_PROTUPS)
	{
		opencatalog("protect", OR_WRITE);
		purgetup(&Prodes, rel->relid, PRORELID, rel->relowner, PRORELOWN);
	}

	/* remove any trees associated with the relation */
	if (rel->relstat & (S_PROTUPS | S_VIEW | S_INTEG))
	{
		opencatalog("tree", OR_WRITE);
		purgetup(&Treedes, rel->relid, TREERELID, rel->relowner, TREEOWNER);
	}
}
