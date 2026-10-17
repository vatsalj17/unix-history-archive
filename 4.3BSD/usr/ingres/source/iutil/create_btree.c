#include	<btree.h>
#include	<ingres.h>
#include	<aux.h>
#include 	<sccs.h>

SCCSID(@(#)create_btree.c	8.2	1/18/85)

/*	CREATE_BTREE -- creates a new B-Tree
**
**	Creates an empty B-Tree whose root is an empty leaf.  The B-Tree
** 	filename is the relation name with "TREE" concatenated to the end
**	of the name.
**
**	Parameters:
**		relname - relation name (I)
*/

create_btree(relname)
char *relname;

{
	struct BTreeNode	root;
	register int		i;
	extern int		Btree_fd;

	root.depth = 1;
	root.prevtree = root.nexttree = 0l;
	bmove(&root.prevtree, &root.prttree, LIDSIZE);
	/* the root is initially an empty leaf node */
	root.nodetype = 'L';
	root.nelmts = 0;
	root.parent = 0;	/* '0' indicates no other empty pages in file */

	root.node.leafnode.prevleaf = 0;
	root.node.leafnode.nextleaf = 0;

	for (i = 0; i< MAXLEAVES; ++i)
		root.node.leafnode.tid_loc[i] = root.node.leafnode.back_ptr[i] = i;

	close(creat(relname, FILEMODE));
	if ((Btree_fd = open(relname, O_RDWR)) < 0)
		syserr("create_btree: can't open %s", relname);
	write_node(RT, &root);

#	ifdef xATR1
	if (tTf(24, 0))
		printf("creating btree %s", relname);
#	endif

}
