# include	<btree.h>
# include	<ingres.h>
# include	<batch.h>
# include	<symbol.h>
# include	<sccs.h>

SCCSID(@(#)insert_btree.c	8.1	12/31/84)

/*	INSERT_BTREE -- BTree insertion routine
**
**	Insert a tid into the BTree at the position corresponding to its lid.
**	Split the leaf if necessary and adjust all values up the tree.
**
**	Parameters :
**		tree - BTree filename (I)
**		lid - given lid (I)
**		tid - tid to be inserted into BTree leaf (I)
**		tidpos - location where tid was inserted (O)
**
**	Returns :
**		-1  	lid position does not exist
**		0	successful insertion
*/

insert_btree(tree, rootpage, lid, tid, tidpos, create)

char *tree;
long rootpage;
long lid;
long *tid;
register TID *tidpos;
char create;
{
	register int		i, j, start;
	struct locator		block, p;
	struct BTreeNode	newblock, temp, newroot;
	short			rblock, tline;
	long			newpage, tpage;
	long			get_tid(), new_page();
	int			save;
	char			replbatch[MAXNAME + 4];
	FILE 			*fp;
	TID			oldtid, newtid;
	long			count, page, childpage;
	extern char		*Fileset;
	extern DESC		Btreesec;

#	ifdef xATR1
	if (tTf(24,0))
		printf("inserting lid %ld into btree at rootpage %d", lid, rootpage);
#	endif

	/* find location of desired lid in B-Tree */
	if (get_tid(rootpage, lid, &block) < -1)
		return(-1);

	if (newroot.depth = create)
	{
		if (save = block.offset)
			newroot.prevtree = block.page.node.leafnode.tid_pos[block.page.node.leafnode.tid_loc[save -1]];
		else
		{
			if (!block.page.prevtree)
				newroot.prevtree = 0;
			else
			{
				get_node(block.page.prevtree, &temp);
				newroot.prevtree = temp.node.leafnode.tid_pos[temp.node.leafnode.tid_loc[temp.nelmts - 1]];
			}
		}
		if  (save <= block.page.nelmts - 1 && block.page.nelmts)
			newroot.nexttree = block.page.node.leafnode.tid_pos[block.page.node.leafnode.tid_loc[save]];
		else
		{
			if (!block.page.nexttree)
				newroot.nexttree = 0;
			else
			{
				get_node(block.page.nexttree, &temp);
				newroot.nexttree = temp.node.leafnode.tid_pos[temp.node.leafnode.tid_loc[0]];
			}
		}
		*tid = new_page(tree);
		if (block.pageno == RT)
			get_node(RT, &block.page);
		if (newroot.prevtree)
		{
			get_node(newroot.prevtree, &temp);
			temp.nexttree = *tid;
			write_node(newroot.prevtree, &temp);
		}
		if (newroot.nexttree)
		{
			get_node(newroot.nexttree, &temp);
			temp.prevtree = *tid;
			write_node(newroot.nexttree, &temp);
		}
		stuff_page(&newroot.prttree, &block.pageno);
		newroot.nodetype = 'L';
		newroot.nelmts = 0;
		newroot.parent = 0;
		newroot.node.leafnode.prevleaf = 0;
		newroot.node.leafnode.nextleaf = 0;
		for (i = 0; i < MAXLEAVES; ++i)
			newroot.node.leafnode.tid_loc[i] = newroot.node.leafnode.back_ptr[i] = i;
	}

	if (block.page.nelmts != MAXLEAVES)
	/* insert tid into its proper position in leaf */
	{
		save = block.page.node.leafnode.tid_loc[block.page.nelmts];
		/* move other tids down to make room for new tid */
		for (i = block.page.nelmts - 1; i >= block.offset; --i)
		{
			block.page.node.leafnode.tid_loc[i + 1] =
				block.page.node.leafnode.tid_loc[i];
			block.page.node.leafnode.back_ptr[block.page.node.leafnode.tid_loc[i]] = i + 1;
		}
		block.page.node.leafnode.tid_loc[block.offset] = save;
		block.page.node.leafnode.tid_pos[save] = *tid;
		block.page.node.leafnode.back_ptr[save] = block.offset;
		++block.page.nelmts;
		write_node(block.pageno, &block.page);
		if (create)
			newroot.prttree.line_id = save;

		/* trace up B-Tree, incrementing key values */
		tracepath(rootpage, &block, 1);

		tpage = block.pageno;
		tline = save;
	}

	else
	/* leaf is full, must be split */
	{
		/* determine where to split leaf */
		start = MAXLEAVES / 2;
		rblock = 0;

		if (block.offset > start)
		/* new tid will be inserted in the new leaf */
		{		
			++start;
			rblock = 1;
		}

		/* readjust all values in the old leaf and assign them for
		** the new leaf */

		block.page.nelmts = start;	/* assume new tid will be
						** inserted into new leaf */
		newpage = new_page(tree);
		newblock.nodetype = 'L';
		for (i = 0; i < MAXLEAVES; ++i)
			newblock.node.leafnode.tid_loc[i] = newblock.node.leafnode.back_ptr[i] = i;
		newblock.nelmts = MAXLEAVES + 1 - start;
		newblock.parent = block.page.parent;
		newblock.depth = 0;

		if (newblock.node.leafnode.nextleaf = block.page.node.leafnode.nextleaf)
		{
			get_node(newblock.node.leafnode.nextleaf, &temp);
			temp.node.leafnode.prevleaf = newpage;
			write_node(newblock.node.leafnode.nextleaf, &temp);
		}
		block.page.node.leafnode.nextleaf = newpage;
		newblock.node.leafnode.prevleaf = block.pageno;

		/* create file for storing tids that must be replaced in btree
		** secondary relation
		*/
		if (!bequal("_SYS", tree, 4) && !create)
		{
			concat(REPL_IN, Fileset, replbatch);
			if ((fp = fopen(replbatch, "w")) == NULL)
				syserr("can't open batch file in insert_btree");
			count = 0;
			stuff_page(&oldtid, &block.pageno);
			stuff_page(&newtid, &newpage);
		}

		/* copy the right half of the old leaf onto the new leaf */
		for (i = 0, j = start; j < MAXLEAVES || rblock == 1; ++i)
		{
			if (j == block.offset && rblock == 1)
			/* current position in new leaf corresponds to position
			** of new tid */
			{
				newblock.node.leafnode.tid_pos[newblock.node.leafnode.tid_loc[i]] = *tid;
				tline = newblock.node.leafnode.tid_loc[i];
				/* indicate that tid has been inserted */
				rblock = -1;
			}
			else
			{
				childpage = newblock.node.leafnode.tid_pos[newblock.node.leafnode.tid_loc[i]] =
					block.page.node.leafnode.tid_pos[block.page.node.leafnode.tid_loc[j]];
				if (create)
				{
					get_node(childpage, &temp);
					stuff_page(&temp.prttree, &newpage);
					temp.prttree.line_id = newblock.node.leafnode.tid_loc[i];
					write_node(childpage, &temp);
				}
				if (!bequal("_SYS", tree, 4) && !create)
				{
					oldtid.line_id = block.page.node.leafnode.tid_loc[j];
					newtid.line_id = newblock.node.leafnode.tid_loc[i];
					if (fwrite(&oldtid, 1, LIDSIZE, fp) != LIDSIZE)
						syserr("write error in insert_btree");
					if (fwrite(&newtid, 1, LIDSIZE, fp) != LIDSIZE)
						syserr("write error in insert_btree");
					++count;
				}
				++j;
			}
		}
		if (!bequal("_SYS", tree, 4) && !create)
		{
			fclose(fp);
			repl_btree(replbatch, count);
		}

		if (!rblock)
		/* new tid belongs in old leaf, insert it into its proper 
		** position */
		{
			save = block.page.node.leafnode.tid_loc[block.page.nelmts];
			for (i = block.page.nelmts - 1; i >= block.offset; --i)
			{
				block.page.node.leafnode.tid_loc[i + 1] =
					block.page.node.leafnode.tid_loc[i];
				block.page.node.leafnode.back_ptr[block.page.node.leafnode.tid_loc[i]] = i + 1;
			}
			block.page.node.leafnode.tid_loc[block.offset] = save;
			block.page.node.leafnode.tid_pos[save] = *tid;
			block.page.node.leafnode.back_ptr[save] = block.offset;
			tline = save;
			/* readjust element counts to reflect that tid has been
			** placed in the old leaf */
			++block.page.nelmts;
			--newblock.nelmts;
		}

		if (block.pageno == rootpage)
		{
			/* split leaf was the root, a new level to the B-Tree
			** must be added */
			rootsplit(tree, rootpage, &block, newpage, block.page.nelmts, newblock.nelmts);
			newblock.parent = rootpage;
			write_node(block.pageno, &block.page);
			newblock.node.leafnode.prevleaf = block.pageno;
			write_node(newpage, &newblock);

			if (create)
			{
				for (i = 0; i < block.page.nelmts; ++ i)
				{
					get_node(block.page.node.leafnode.tid_pos[block.page.node.leafnode.tid_loc[i]], &temp);
					stuff_page(&temp.prttree, &block.pageno);
					write_node(block.page.node.leafnode.tid_pos[block.page.node.leafnode.tid_loc[i]], &temp);
				}
			}
			/* btree page has changed */
			if (!bequal("_SYS", tree, 4) && !create)
			{
				concat(REPL_IN, Fileset, replbatch);
				if ((fp = fopen(replbatch, "w")) == NULL)
					syserr("can't open batch file in insert_btree");
				count = 0;
				page = 0l;
				stuff_page(&oldtid, &page);
				stuff_page(&newtid, &block.pageno);
				for (i = 0; i < block.page.nelmts; ++i)
				{
					if (rblock || block.page.node.leafnode.tid_loc[i] != tline)
					{
						oldtid.line_id = newtid.line_id = block.page.node.leafnode.tid_loc[i];
						if (fwrite(&oldtid, 1, LIDSIZE, fp) != LIDSIZE)
							syserr("write error in insert_btree");
						if (fwrite(&newtid, 1, LIDSIZE, fp) != LIDSIZE)
							syserr("write error in insert_btree");
						++count;
					}
				}
				fclose(fp);
				repl_btree(replbatch, count);
			}
		}
		else
		/* insert the pointer and key associated with new leaf into the
		** parent of the old leaf */
		{
			write_node(block.pageno, &block.page);
			write_node(newpage, &newblock);
			p.pageno = block.page.parent;
			parentkey(block.pageno, &p);
			p.page.node.intnode.key[p.offset] = block.page.nelmts;
			insert_key(tree, rootpage, newpage, newblock.nelmts, &p);
		}
		tpage = (rblock) ? newpage : block.pageno;
	}
	stuff_page(tidpos, &tpage);
	tidpos->line_id = tline;

	if (create)
		write_node(*tid, &newroot);

}

/*
**	Takes a pair of tids from a file and sequentially replaces the
**	old tid with the new tid in the secondary btree relation
*/

repl_btree(replbatch, count)
register char *replbatch;
long count;
{
	register int	i, j;
	TID		uptid;
	DESC		d;
	char		tids[2 * LIDSIZE], key[2 * LIDSIZE], newkey[2 * LIDSIZE];
	extern DESC	Btreesec;

	if (count > 0)
	{
		/* set up descriptor for sort */
		d.reloff[1] = 0;
		d.reloff[2] = LIDSIZE;
		d.relfrmt[1] = d.relfrmt[2] = INT;
		d.relfrml[1] = d.relfrml[2] = LIDSIZE;
		d.relgiven[1] = 1;
		d.relgiven[2] = 2;
		d.reldum.relspec = M_ORDER;
		d.reldum.relatts = 2;
		d.reldum.relwid = 2 * LIDSIZE;
		sortfile(replbatch, &d, FALSE);
		if ((Repl_outfp = fopen(ztack(REPL_OUT, Fileset), "r")) == NULL)
			syserr("can't open replace file after sort in insertbtreee\n");
		clearkeys(&Btreesec);
		for (i = 0; i < count; ++i)
		{
			if (fread(tids, 1, 2 * LIDSIZE, Repl_outfp) != 2 * LIDSIZE)
				syserr("read error in insert_btree");
			setkey(&Btreesec, key, tids, 2);
			if (getequal(&Btreesec, key, newkey, &uptid))
			{
				printup(&Btreesec, key);
				syserr("getequal error in insert_btree");
			}
			/* place new tid in place */
			setkey(&Btreesec, newkey, tids + LIDSIZE, 2);
			if (j = replace(&Btreesec, &uptid, newkey, TRUE))
				if (j == 1)
					continue;
				else
					syserr("bad replace in insert_btree");
		}
		fclose(Repl_outfp);
		unlink(replbatch);
		unlink(ztack(REPL_OUT, Fileset));
	}
}

/*	Insert a key/ptr pair into a node, splitting nodes if necessary and
**	adjusting values up the tree.
**
**	Parameters :
**		tree - BTree filename (I)
**		p - page number of newly formed node (I)
**		k - key value of newly formed node (I)
**		pkey - information about the node whose ptr/key pair is to
**		       be inserted (I, O)
*/

insert_key(tree, rootpage, p, k, pkey)

char *tree;
long rootpage;
long p, k;
register struct locator *pkey;
{
	register int		i, j, start;
	struct BTreeNode	newblock, temp;
	long			newpage, oldkey, newkey;
	struct locator		prt;
	short			rblock;
	long 			new_page();

	if (pkey->page.nelmts != MAXPTRS)
	/* insert pointer/key pair into proper position in node by moving
	** other pairs down node */
	{
		for (i = pkey->page.nelmts - 1; i >= pkey->offset + 1; --i)
		{
			pkey->page.node.intnode.ptr[i + 1] =
				pkey->page.node.intnode.ptr[i];
			pkey->page.node.intnode.key[i + 1] =
				pkey->page.node.intnode.key[i];
		}
		++pkey->page.nelmts;
		pkey->page.node.intnode.ptr[pkey->offset + 1] = p;
		pkey->page.node.intnode.key[pkey->offset + 1] = k;

		write_node(pkey->pageno, &pkey->page);

		/* trace up B-Tree incrementing keys */ 
		tracepath(rootpage, pkey, 1);
	}

	else
	/* node is full, must be split */
	{
		/* determine where split will be made */
		start = MAXPTRS / 2;
		rblock = 0;

		if (pkey->offset + 1 > start)
		/* ptr/key pair will be inserted in new node */
		{
			++start;
			rblock = 1;
		}

		/* readjust old node values and create new node values */
		pkey->page.nelmts = start;
		newpage = new_page(tree);
		newblock.nodetype = 'I';
		newblock.nelmts = MAXPTRS + 1 - start;
		newblock.parent = pkey->page.parent;
		newblock.depth = 0;

		/* copy right side of old node into new node */

		for (i = 0, j = start; j < MAXPTRS || rblock == 1; ++i)
		{
			if (j == pkey->offset + 1 && rblock == 1)
			/* node position corresponds to that of new ptr/key pair */
			{
				newblock.node.intnode.ptr[i] = p;
				newblock.node.intnode.key[i] = k;
				/* make sure children of newblock have proper
				** parent */
				get_node(p, &temp);
				temp.parent = newpage;
				write_node(p, &temp);

				rblock = -1;
			}
			else
			{
				newblock.node.intnode.ptr[i] =
					pkey->page.node.intnode.ptr[j];
				newblock.node.intnode.key[i] =
					pkey->page.node.intnode.key[j];
				/* make sure children of newblock have proper
				** parent */
				get_node(newblock.node.intnode.ptr[i], &temp);
				temp.parent = newpage;
				write_node(newblock.node.intnode.ptr[i], &temp);
				++j;
			}
		}

		if (!rblock)
		/* insert new ptr/key pair into proper position in old node */
		{
			for (i = pkey->page.nelmts - 1; i >= pkey->offset + 1; --i)
			{
				pkey->page.node.intnode.ptr[i + 1] =
					pkey->page.node.intnode.ptr[i];
				pkey->page.node.intnode.key[i + 1] =
					pkey->page.node.intnode.key[i];
			}
			pkey->page.node.intnode.ptr[pkey->offset + 1] = p;
			pkey->page.node.intnode.key[pkey->offset + 1] = k;
			++pkey->page.nelmts;
			--newblock.nelmts;
		}

		/* calculate the key values of the old and new nodes */
		oldkey = 0;
		for (i = 0; i < pkey->page.nelmts; ++i)
			oldkey += pkey->page.node.intnode.key[i];
		newkey = 0;
		for (i = 0; i < newblock.nelmts; ++i)
			newkey += newblock.node.intnode.key[i];

		if (pkey->pageno == rootpage)
		{
			/* split node was root, add a new level to B-Tree */
			rootsplit(tree, rootpage, pkey, newpage, oldkey, newkey);
			newblock.parent = rootpage;
			write_node(pkey->pageno, &pkey->page);
			write_node(newpage, &newblock);
		}

		else
		/* recursively add ptr/key pair corresponding to new node into
		** the parent of the old node */
		{
			write_node(pkey->pageno, &pkey->page);
			write_node(newpage, &newblock);
			prt.pageno = pkey->page.parent;
			parentkey(pkey->pageno, &prt);
			prt.page.node.intnode.key[prt.offset] = oldkey;
			insert_key(tree, rootpage, newpage, newkey, &prt);
		}
	}
}

/*	Form a new root with two children since the old root was split.
**
**	Parameters :
**		tree - BTree filename (I)
**		oldroot - split root (I, O)
**		rpageno - page number of new root's right child (I)
**		rkey - key of new root's right child (I)
*/

rootsplit(tree, rootpage, oldroot, rpageno, lkey, rkey)

char *tree;
long rootpage;
register struct locator *oldroot;
long rpageno, lkey, rkey;
{
	struct BTreeNode	newroot, temp;
	long			i;

	/* get a new page for the former root */
	oldroot->pageno = new_page(tree);

	newroot.depth = oldroot->page.depth;
	newroot.prevtree = oldroot->page.prevtree;
	newroot.nexttree = oldroot->page.nexttree;
	bmove(&oldroot->page.prttree, &newroot.prttree, LIDSIZE);
	newroot.nodetype = 'I';
	newroot.nelmts = 2;
	newroot.parent = oldroot->page.parent;
	oldroot->page.parent = rootpage;
	oldroot->page.depth = 0;
	newroot.node.intnode.key[0] = lkey;
	newroot.node.intnode.ptr[0] = oldroot->pageno;
	newroot.node.intnode.key[1] = rkey;
	newroot.node.intnode.ptr[1] = rpageno;

	write_node(rootpage, &newroot);

	if (oldroot->page.nodetype == 'I')
		/* make sure the children of the oldroot have the correct parent */
		for (i = 0; i < oldroot->page.nelmts; ++i)
		{
			get_node(oldroot->page.node.intnode.ptr[i], &temp);
			temp.parent = oldroot->pageno;
			write_node(oldroot->page.node.intnode.ptr[i], &temp);
		}
}
