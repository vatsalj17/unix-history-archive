# include	"../ingres.h"
# include	"../tree.h"
# include	"../symbol.h"

/*
**	MAPVAR -- construct varable maps for ROOT, AND, and AGHEAD nodes.
**	tl is a flag  which indicates if the target list should
**	be included in the mapping.  If tl = 0, it should; else it should not.
*/

mapvar(tree, tl)
struct querytree	*tree;
int			 tl;
{
	register struct querytree	*t;
	register int			rmap, lmap;

	t = tree;
	if (t == 0)
		return (0);

	switch (t->sym.type)
	{

	  case ROOT:
	  case AND:
	  case AGHEAD:
		/* map the right side */
		t->rvarm = rmap = mapvar(t->right, tl);

		/* map the left side or else use existing values */
		if (tl == 0)
		{
			t->lvarm = lmap = mapvar(t->left, tl);
			t->lvarc = bitcnt(lmap);
		}
		else
			lmap = t->lvarm;

		/* form map of both sides */
		rmap =| lmap;

		/* compute total var count */
		t->tvarc = bitcnt(rmap);

		return (rmap);

	  case VAR:
		if ((t = ckvar(t))->valptr)
			return (0);	/* var is a constant */
		return (01 << t->varno);
	}

	/* node is not a VAR, AND, ROOT, or AGHEAD */
	return (mapvar(t->left, tl) | mapvar(t->right, tl));
}
