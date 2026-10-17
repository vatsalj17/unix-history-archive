#include	<btree.h>
#include	<ingres.h>
#include	<opsys.h>
#include	<sccs.h>

SCCSID(@(#)btree_util.c	8.2	1/31/86)

/*	Trace up to the root of the BTree, incrementing (or decrementing) all
**	key values.
**
**	Parameters :
**		tree - BTree filename (I)
**		block - starting node from which path is traced (I)
**		inc - either +1 or -1 (I)
*/

tracepath(rootpage, block, inc)

long rootpage;
register struct locator *block;
register int inc;
{
	struct locator 	p, child;

	if (block->pageno != rootpage)	/* if at root, no need for adjustment */
	{
		bmove(block, &child, sizeof(struct locator));

		/* trace up to root node */
		do
		{
			p.pageno = child.page.parent;
			parentkey(child.pageno, &p);
			p.page.node.intnode.key[p.offset] += inc;
			write_node(p.pageno, &p.page);
			bmove(&p, &child, sizeof(struct locator));
		} while (p.pageno != rootpage);
	}
}

/*	Determines which key/ptr pair is the parent of the given page
**
**	Parameters :
**		tree - BTree filename (I)
**		block - page number of child (I)
**		prt - information about the parent of the child (I, O)
**
*/

parentkey(block, prt)

long block;
register struct locator *prt;
{
	register int	i;

	get_node(prt->pageno, &prt->page);
	/* find pointer in parent which references desired block */
	for (i = 0; prt->page.node.intnode.ptr[i] != block && i < prt->page.nelmts; ++i)
		continue;

	if (i == prt->page.nelmts)
		syserr("parentkey: child = %ld", block);
	prt->offset = i;
	return(0);
}

/*	Retrieve a node from the BTree
**
**	Parameters :
**		tree - BTree filename (I)
**		pagenum - page number of page to be retrieved (I)
**		buffer - buffer into which page is retrieved (O)
*/

get_node(pagenum, buffer)

long pagenum;
register struct BTreeNode *buffer;
{
	extern int	Btree_fd;

	if ( lseek(Btree_fd, pagenum * PGSIZE, 0) == -1 )
		syserr("GET_NODE: seek to %ld failed",pagenum);
	if (read(Btree_fd, buffer, sizeof *buffer) != sizeof *buffer)
		syserr("GET_NODE: read btree, page %ld", pagenum);
}

/*	Write a node into the BTree file name 
**
**	Parameters :
**		tree - BTree filename (I)
**		pagenum - page number of page to be written (I)
**		buffer - buffer containing page to be written (I)
*/

write_node(pagenum, buffer)

long pagenum;
register struct BTreeNode *buffer;
{
	extern int	Btree_fd;

	lseek(Btree_fd, pagenum * PGSIZE, 0);
	if (write(Btree_fd, buffer, sizeof *buffer) != sizeof *buffer)
		syserr("WRITE_NODE: write btree, page %ld", pagenum);
}

/*	Retrieves a new page from the BTree file.  If there is an empty page
**	in the file, that page is used.  Otherwise, a new page is added to
**	the end of the file.
**
**	Parameters :
**		tree - BTree filename (I)
**
**	Returns :
**		page number of new page
*/

long
new_page(tree)

char *tree;
{
	int 			i;
	long			freepage, newpage;
	struct BTreeNode 	page, root, garbage;
	struct stat		fileinfo;
	extern DESC		Btreesec;

	get_node(RT, &root);
	freepage = root.parent;
	if (freepage == 0)
	/* no free pages in file, add a new page to the end */
	{
		/* determine page number of page at very end of file */
		stat(tree, &fileinfo);
		newpage = fileinfo.st_size / PGSIZE;
		write_node(newpage, &page);
	}
	else
	/* use first free page, adjusting free page chain */
	{
		newpage = freepage;
		get_node(newpage, &garbage);
		root.parent = garbage.parent;
		write_node(RT, &root);
	}
	return(newpage);
}

/*	Returns an empty file to the empty file list.  The head of the list
**	is indicated through the parent of the root and linked together
**	through the parents of the empty pages.
**
**	Parameters :
**		tree - BTree filename (I)
**		pagenum - page number of empty page (I)
*/

return_page(pagenum)

long pagenum;
{
	struct BTreeNode	root, freepage;	

	/* indicate that page is free by inserting its page number at the
	** head of the free page list */
	get_node(RT, &root);
	get_node(pagenum, &freepage);
	freepage.parent = root.parent;
	freepage.depth = 0;
	write_node(pagenum, &freepage);
	root.parent = pagenum;
	write_node(RT, &root);
}

/*
**	Determine the lid that corresponds to a given position in the B-Tree.
**
**	Parameters :
**		tree - BTree name (I)
**		tidloc - location in BTree (I)
**
**	Returns :
**		lid value
*/
get_lid(tidloc, lid)

TID *tidloc;
long lid[];
{
	static struct locator	tid;
	register int		i, j;
	long			l, child;

	do
	{
		/* determine where in BTree to search */
		pluck_page(tidloc, &tid.pageno);
	
		get_node(tid.pageno, (char *) &tid.page);
		tid.offset = tid.page.node.leafnode.back_ptr[tidloc->line_id & I1MASK];

		if (tid.offset > tid.page.nelmts - 1)
			syserr("get_lid: bad tid location %ld, tid.offset=%d, tid.page.nelmts=%d", *tidloc, tid.offset,tid.page.nelmts);
	
		/* scan through leaf */
		for (l = 1, i = tid.offset; i > 0; ++l, --i)
			continue;
		/* trace up to root, adding keys */
		while (!tid.page.depth)
		{
			child = tid.pageno;
			tid.pageno = tid.page.parent;
			parentkey(child, &tid);
			for (i = tid.offset - 1; i >= 0; l += tid.page.node.intnode.key[i--])
				continue;
		}
		lid[tid.page.depth - 1] = l;
#		ifdef xATR1
			if (tTf(24,0))
				printf("lid=%ld\n", lid[tid.page.depth - 1]);
#		endif
		bmove(&tid.page.prttree, tidloc, LIDSIZE);
	} while (tid.page.depth > 1);
}

/*
**	Uses the secondary btree relation to find the btree tid corresponding 
**	to a given main relation tid
*/

search_btree(mtid, tidloc)

TID mtid; 
register TID *tidloc;
{
	char		key[2 * LIDSIZE], tup[2 * LIDSIZE];
	TID		tid;
	extern DESC	Btreesec;

#	ifdef xATR1
		if (tTf(24,0))
		{
			printf("In Search_btree: searching for tid %ld\n", mtid);
			printdesc(&Btreesec);
		}
#	endif
	clearkeys(&Btreesec);
	setkey(&Btreesec, key, &mtid, 1);
	if (!getequal(&Btreesec, key, tup, &tid))
		bmove(tup + LIDSIZE, tidloc, LIDSIZE);
	else
		syserr("search_btree:can't find mtid %ld in btree", mtid);
#	ifdef xATR1
		if (tTf(24,0))
			printup(&Btreesec, tup);
#	endif
}

/*
**	Linearly searches the leaves of the BTree for a desired tid
**
**	Parameters :
**		tree - BTree file name (I)
**		tid - desired tid value (I)
**		tidloc - location of the desired tid (O)
*/

lin_search(level, tid, tidloc, lid, tupcnt)

int level;
long tid;
TID *tidloc;
long lid[];
long tupcnt;
{
	struct locator	block;
	register int	i;
	long		page, t, next;
	int		found;
	long		nextpage, count;
	int		forward, first;

	page = RT;
	for (i = 0; i < level - 1; ++i)
	{
		t = get_tid(page, lid[i], &block);
		page = t;
	}

	found = 0;
	forward  = 0;
	first = 1;
	do
	{
		if (!forward)
		{
			get_node(page, &block.page);
			next = block.page.nexttree;
		}
		count = 0;

		/* start at leftmost leaf */
		do
		{
			get_node(page, &block.page);
			if (forward)
				nextpage = block.page.nexttree;
			else
				nextpage = block.page.prevtree;
			get_tid(page, 1, &block);
			for ( ; ; )
			{
				/* search through leaf */
				for (i = 0; i < block.page.nelmts && block.page.node.leafnode.tid_pos[block.page.node.leafnode.tid_loc[i]] != tid; ++i)
				{
					if (!first)
						++count;
				}
				if (i < block.page.nelmts && block.page.node.leafnode.tid_pos[block.page.node.leafnode.tid_loc[i]] == tid)
				{
					found = 1;
					break;	/* tid found */
				}
				first = 0;
				if (!(block.pageno = block.page.node.leafnode.nextleaf) || count >= tupcnt)
					break;	/* all leaves exhausted */
				else
					/* try leaf to the right */
					get_node(block.pageno, &block.page);
			}
			if (found || count >= tupcnt)
				break;
		} while (page = nextpage);
		nextpage = next;
		forward = !forward;
	} while (forward != 0 && !found);
	if (!found)
		syserr("search_btree: tid not found %ld", tid);
	else
	{
		stuff_page(tidloc, &block.pageno);
		tidloc->line_id = block.page.node.leafnode.tid_loc[i];
	}
}

/*
**	Determines the value corresponding to the very last lid in the
**	BTree.
**
**	Parameters :
**		tree - BTree file name (I)
**
**	Returns :
**		value of last lid
*/
long
last_lid(rootpage)

long rootpage;

{
	register int 		i;
	long			lid;
	struct BTreeNode	root;

	lid = 0;
	get_node(rootpage, &root);
	if (root.nodetype == 'L')
		lid = root.nelmts;
	else
		for (i = 0; i < root.nelmts; ++i)
			lid += root.node.intnode.key[i];
	++lid;
	return(lid);
}

/*
**	Changes the tid value stored in the leaf of a BTree node
**
**	Parameters :
**		tree - BTree file name (I)
**		tid - new tid value (I)
**		tidloc - location of tid to be replaced (I)
*/

replace_btree(tid, tidloc)

long tid;
register TID *tidloc;

{
	long			pageno;
	struct BTreeNode	p;

	pluck_page(tidloc, &pageno);
	get_node(pageno, &p);
	p.node.leafnode.tid_pos[tidloc->line_id] = tid;
	write_node(pageno, &p);
}
