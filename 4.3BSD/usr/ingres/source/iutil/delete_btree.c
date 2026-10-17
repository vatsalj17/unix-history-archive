# include	<stdio.h>
# include	<btree.h>
# include	<ingres.h>
# include	<batch.h>
# include	<sccs.h>

SCCSID(@(#)delete_btree.c	8.1	12/31/84)

/*	DELETE_BTREE -- BTree deletion routine
**
**	Deletes a tid from a leaf node and adjusts all values up the tree
**
**	Parameters :
**		tree - BTree filename (I)
**		lid - lid to be deleted (I)
**
**	Returns :
**		> 0 	success
**		< 0 	bad lid
*/

delete_btree(lid, level)
long lid[];
register int level;

{
	register int		i, j;
	struct locator		tid[MAXLID];
	register int		save;
	int			num[MAXLID];
	int			del[MAXLID];
	long			page[MAXLID + 1], t;
	struct BTreeNode	temp;

#	ifdef xATR1
	if(tTf(24,0))
		printf("deleting lid %ld from tree ", lid);
#	endif

	page[0] = RT;
	for (i = 0; i < level; ++i)
	{
		if ((t = get_tid(page[i], lid[i], &tid[i])) < 0)
			return(-1);
		del[i] = 0;
		num[i] = tid[i].page.nelmts; 
		page[i+1] = t;
	}

	del[level-1] = 1;
	for (i = level - 2; i >= 0; --i)
	{
		if (num[i + 1] == 1 && del[i + 1] == 1)
			del[i] = 1;
	}

	for (i = 0; i < level; ++i)
	{
		if (del[i])
		{
			++Repl_cnt[i];
			for (j = i - 1; j >= 0; --j)
				Prev_lid[j] = lid[j];
			/* remove tid from leaf */
			if (tid[i].offset != tid[i].page.nelmts - 1)
			{
				save = tid[i].page.node.leafnode.tid_loc[tid[i].offset];
				for (j = tid[i].offset; j < tid[i].page.nelmts; ++j)
				{
					tid[i].page.node.leafnode.tid_loc[j] =
						tid[i].page.node.leafnode.tid_loc[j + 1];
					tid[i].page.node.leafnode.back_ptr[tid[i].page.node.leafnode.tid_loc[j]] = j;
				}
				tid[i].page.node.leafnode.tid_loc[tid[i].page.nelmts - 1] = save;
				tid[i].page.node.leafnode.back_ptr[save] = tid[i].page.nelmts - 1;
			}
			--tid[i].page.nelmts;

			if (tid[i].page.nelmts != 0)
			{
				write_node(tid[i].pageno, &tid[i].page);
				/* leaf is now empty as a result of deletion, decrement keys
				** up tree */
				tracepath(page[i], &tid[i], -1);
			}

			else if (tid[i].pageno != page[i])
			{
				/* key/ptr pair corresponding to empty leaf must be removed */
				delete_key(page[i], &tid[i]);
			}
			else if (lid[i] == 1 && page[i] != RT)
			{
				if (tid[i].page.prevtree)
				{
					get_node(tid[i].page.prevtree, &temp);
					temp.nexttree = tid[i].page.nexttree;
					write_node(tid[i].page.prevtree, &temp);
				}
				if (tid[i].page.nexttree)
				{
					get_node(tid[i].page.nexttree, &temp);
					temp.prevtree = tid[i].page.prevtree;
					write_node(tid[i].page.nexttree, &temp);
				}
				return_page(page[i]);
			}
			else
				write_node(page[i], &tid[i].page);
		}
	}
	return(0);
}

/*	Returns an empty page to the empty file space list.  Removes key/ptr
** 	pair corresponding to empty node from tree.  Combines and distributes
**	pairs if a node has less than the minimum number of values.  Adjusts
**	all values up the tree.
**
**	Parameters :
**		tree - BTree filename (I)
**		empty - empty node (I)
*/

delete_key(rootpage, empty)

long rootpage;
register struct locator *empty;
{
	struct locator		prt, gprt, sibling;
	register int		i;
	struct BTreeNode	new, temp;
	long			pkey, skey;
	extern char		*Fileset;
	char			replbatch[MAXNAME + 4];
	FILE			*fp;
	long			count, page;
	TID			oldtid, newtid;

	/* find parent corresponding to empty node, and remove ptr/key pair
	** from parent */
	prt.pageno = empty->page.parent;
	parentkey(empty->pageno, &prt);
	if (prt.offset != prt.page.nelmts - 1)
	{
		for (i = prt.offset; i < prt.page.nelmts; ++i)
		{
			prt.page.node.intnode.ptr[i] = 
				prt.page.node.intnode.ptr[i + 1];
			prt.page.node.intnode.key[i] =
				prt.page.node.intnode.key[i + 1];
		}
	}
	--prt.page.nelmts;

	if (empty->page.nodetype == 'L')
	/* adjust previous and next fields of leaves */
	{
		if (empty->page.node.leafnode.nextleaf != 0)
		{
			get_node(empty->page.node.leafnode.nextleaf, &temp);
			temp.node.leafnode.prevleaf = empty->page.node.leafnode.prevleaf;
			write_node(empty->page.node.leafnode.nextleaf, &temp);
		}
		if (empty->page.node.leafnode.prevleaf != 0)
		{
			get_node(empty->page.node.leafnode.prevleaf, &temp);
			temp.node.leafnode.nextleaf = empty->page.node.leafnode.nextleaf;
			write_node(empty->page.node.leafnode.prevleaf, &temp);
		}
	}

	/* return empty node to empty file space */
	return_page(empty->pageno);

	if (prt.page.nelmts >= (int) ((float) MAXPTRS / 2 + 0.5))
	{
		write_node(prt.pageno, &prt.page);
		/* node still has proper number of elements, decrement key  
		** values up the tree */
		tracepath(rootpage, &prt, -1);
	}

	else if (prt.pageno == rootpage && prt.page.nelmts < 2)
	/* root has only one child, a leaf; root becomes leaf node */
	{
		/* move leaf values into root; root's position always remains
		** the same */
		get_node(prt.page.node.intnode.ptr[0], &new);
		new.parent = prt.page.parent;
		write_node(rootpage, &new);
		return_page(prt.page.node.intnode.ptr[0]);
		if (new.nodetype  == 'I')
		{
			for (i = 0; i < new.nelmts; ++i)
			{
				get_node(new.node.intnode.ptr[i], &temp);
				temp.parent = rootpage;
				write_node(new.node.intnode.ptr[i], &temp);
			}
		}
		else
		{
			/* btree tid is being changed, must be reflected in
			** secondary btree relation
			*/
			concat(REPL_IN, Fileset, replbatch);
			if ((fp = fopen(replbatch, "w")) == NULL)
				syserr("can't open replbatch in delete_btree");
			count = 0;
			page = 0l;
			stuff_page(&newtid, &page);
			stuff_page(&oldtid, &prt.page.node.intnode.ptr[0]);
			for (i = 0; i < new.nelmts; ++i)
			{
				oldtid.line_id = newtid.line_id = new.node.leafnode.tid_loc[i];
				if (fwrite(&oldtid, 1, LIDSIZE, fp) != LIDSIZE)
					syserr("write error in delete_btree");
				if (fwrite(&newtid, 1, LIDSIZE, fp) != LIDSIZE)
					syserr("write error in delete_btree");
				++count;
			}
			fclose(fp);
			repl_btree(replbatch, count);
			unlink(replbatch);
			unlink(ztack(REPL_OUT, Fileset));
		}
	}

	else if (prt.pageno != rootpage)
	{
		/* find the grandparent of the empty node */
		gprt.pageno = prt.page.parent;
		parentkey(prt.pageno, &gprt);
		if (gprt.page.nelmts - 1 != gprt.offset)
		{
			/* determine the right sibling of the parent node */
			sibling.pageno = gprt.page.node.intnode.ptr[gprt.offset + 1];
			get_node(sibling.pageno, &sibling.page);

			if (sibling.page.nelmts > MAXPTRS / 2 + 1)
			/* distribute key/ptr pairs of parent and right sibling
			** between the two nodes (since there are sufficient
			** pairs to guarantee that both will have at the least
			** the minimum number of required children) */
			{
				distribute(&prt, &sibling, &pkey, &skey);
				/* adjust key values in grandparent */
				gprt.page.node.intnode.key[gprt.offset] = pkey;
				gprt.page.node.intnode.key[gprt.offset + 1] = skey;
				write_node(gprt.pageno, &gprt.page);
				tracepath(rootpage, &gprt, -1);
				return;
			}
		}
		if (gprt.offset != 0)
		{
			/* determine the left sibling of the parent node */
			sibling.pageno = gprt.page.node.intnode.ptr[gprt.offset - 1];
			get_node(sibling.pageno, &sibling.page);

			if (sibling.page.nelmts > MAXPTRS / 2 + 1)
			/* distribute key/ptr pairs of parent and left sibling
			** between the two nodes */
			{
				distribute(&sibling, &prt, &skey, &pkey);
				gprt.page.node.intnode.key[gprt.offset - 1] = skey;
				gprt.page.node.intnode.key[gprt.offset] = pkey;
				write_node(gprt.pageno, &gprt.page);
				tracepath(rootpage, &gprt, -1);
				return;
			}
		}

		if (gprt.page.nelmts - 1 != gprt.offset)
		/* combine parent node with its right sibling */
		{
			sibling.pageno = gprt.page.node.intnode.ptr[gprt.offset + 1];
			get_node(sibling.pageno, &sibling.page);
			combine(&prt, &sibling);
		}
		else
		/* combine parent node with its left sibling */
		{
			sibling.pageno = gprt.page.node.intnode.ptr[gprt.offset - 1];
			get_node(sibling.pageno, &sibling.page);
			combine(&sibling, &prt);
			/* grandparent should point to newly combined block,
			** the left sibling */
			--gprt.offset;
		}

		get_node(gprt.page.node.intnode.ptr[gprt.offset], &new);
		if (gprt.pageno == rootpage && gprt.page.nelmts == 2)
		/* root has only one child, reduce B-Tree level */
		{
			/* only child becomes root */
			new.parent = gprt.page.parent;
			write_node(rootpage, &new);

			/* make sure "new's" children's parent is the root */
			for (i = 0; i < new.nelmts; ++i)
			{
				get_node(new.node.intnode.ptr[i], &temp);
				temp.parent = rootpage;
				write_node(new.node.intnode.ptr[i], &temp);
			}
			/* both of root's children empty */
			return_page(gprt.page.node.intnode.ptr[gprt.offset + 1]);
			return_page(gprt.page.node.intnode.ptr[gprt.offset]);
		}

		else
		{
			/* adjust key value in grandparent node */
			pkey = 0;
			for (i = 0; i < new.nelmts; ++i)
				pkey += new.node.intnode.key[i];
			gprt.page.node.intnode.key[gprt.offset] = pkey;
			write_node(gprt.pageno, &gprt.page);

			/* remove ptr/key pair corresponding to empty node */
			gprt.pageno = gprt.page.node.intnode.ptr[gprt.offset + 1];
			get_node(gprt.pageno, &gprt.page);
			delete_key(rootpage, &gprt);
		}
	
	}
}

/*	Divides key/ptr pairs between the left and right nodes, so both will
**	have the proper number of elements.
**
**	Parameters :
**		tree - BTree filename (I)
**		left - left node (I, O)
**		right - "left's" right sibling (I, O)
**		lkey - key value of the parent of left node (O)
**		rkey - key value of the parent of right node (O)
*/

distribute(left, right, lkey, rkey)

register struct locator *left, *right;
int *lkey, *rkey;
{
	register int		i, move;
	struct BTreeNode	temp;

	if (right->page.nelmts > left->page.nelmts)
	/* move elements from the right node to the left node */
	{
		move = MAXPTRS / 2 - left->page.nelmts;

		for (i = 1; i <= move; ++i)
		/* move first part of right node into the end of the left node */
		{
			left->page.node.intnode.ptr[i + left->page.nelmts] =
				right->page.node.intnode.ptr[i - 1];
			left->page.node.intnode.key[i + left->page.nelmts] =
				right->page.node.intnode.key[i - 1];
			/* adjust parents of children whose parents have been
			** moved */
			get_node(left->page.node.intnode.ptr[i + left->page.nelmts], &temp);
			temp.parent = left->pageno;
			write_node(left->page.node.intnode.ptr[i + left->page.nelmts], &temp);
		}
		left->page.nelmts += move;

		for (i = move; i < right->page.nelmts; ++i)
		/* flush right node values to the left */
		{
			right->page.node.intnode.ptr[i - move] =
				right->page.node.intnode.ptr[i];
			right->page.node.intnode.key[i - move] =
				right->page.node.intnode.key[i];
		}
		right->page.nelmts -= move;
	}

	else
	/*  move left node values to the right node */
	{
		move = MAXPTRS / 2 - right->page.nelmts + 1;

		/* move right node values to the right to make room for left
		** node values */
		for (i = right->page.nelmts - 1; i >= 0; --i)
		{
			right->page.node.intnode.ptr[i + move] =
				right->page.node.intnode.ptr[i];
			right->page.node.intnode.key[i + move] =
				right->page.node.intnode.key[i];
		}

		/* move latter part of left node into the now free space at the
		** beginning of the right node */
		for (i = 0; i < move; ++i)
		{
			right->page.node.intnode.ptr[i] = 
				left->page.node.intnode.ptr[left->page.nelmts - move + i];
			right->page.node.intnode.key[i] =
				left->page.node.intnode.key[left->page.nelmts - move + i];
			/* adjust parents of children whose parents have been
			** moved */
			get_node(right->page.node.intnode.ptr[i], &temp);
			temp.parent = right->pageno;
			write_node(right->page.node.intnode.ptr[i], &temp);
		}
		left->page.nelmts -= move;
		right->page.nelmts += move;
	}

	/* calculate key values for parents of the left and right nodes */
	*lkey = 0;
	for (i = 0; i < left->page.nelmts; ++i)
		*lkey += left->page.node.intnode.key[i];
	*rkey = 0;
	for (i = 0; i < right->page.nelmts; ++i)
		*rkey += right->page.node.intnode.key[i];
	write_node(left->pageno, &left->page);
	write_node(right->pageno, &right->page);
}

/*	Combines left and right nodes together to form one node.
**
**	Parameters :
**		tree - BTree filename (I)
**		left - left node (I, O)
**		right - "left's" right sibling (I)
*/

combine(left, right)

register struct locator *left, *right;
{
	register int		i;
	struct BTreeNode	temp;

	/* move all ptr/key values in the right node to the end of the left node */
	for (i = 0; i < right->page.nelmts; ++i)
	{
		left->page.node.intnode.ptr[left->page.nelmts + i] =
			right->page.node.intnode.ptr[i];
		left->page.node.intnode.key[left->page.nelmts + i] =
			right->page.node.intnode.key[i];
		/* adjust parents of children whose parents have been moved */
		get_node(left->page.node.intnode.ptr[left->page.nelmts + i], &temp);
		temp.parent = left->pageno;
		write_node(left->page.node.intnode.ptr[left->page.nelmts + i], &temp);
	}
	left->page.nelmts += right->page.nelmts;
	write_node(left->pageno, &left->page);
}
