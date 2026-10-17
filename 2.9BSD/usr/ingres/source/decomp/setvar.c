# include	"../ingres.h"
# include	"../tree.h"
# include	"../symbol.h"
# include	"decomp.h"

/*
** SETVAR -- Routines to convert a VAR to a constant and back to a VAR
**
**	This file contains the routines for tuple substitution.
**
**	Setvar -- Make a VAR node reference its position in the tuple.
**
**	Clrvar -- Make the VAR node refer back to a VAR
*/

setvar(tree1, var, intid, tuple)
struct querytree 	*tree1;
int			var;
long			*intid;
char			*tuple;

/*
** Setvar -- Var's are changed to reference their values in a tuple.
**	ROOT and AND nodes are changed to update the variable maps.
*/

{
	register struct querytree	*tree;
	register int			mask, nvc;
	struct descriptor		*readopen();

	tree = tree1;
	if (!tree) 
		return;
	switch (tree->sym.type)
	{
	  case VAR:
		if ((tree=ckvar(tree))->varno == var)
		{
#			ifdef xDTR1
			if (tTf(12, 0))
			{
				printf("setvar:%d;tree:", var);
				writenod(tree);
			}
#			endif
			if (tree->attno)
				tree->valptr = tuple +
					readopen(var)->reloff[tree->attno];
			else
				tree->valptr = intid;
		}
		return;

	  case ROOT:
	  case AND:
		mask = 01 << var;
		nvc = tree->tvarc;
		if (tree->lvarm & mask)
		{
			setvar(tree->left, var, intid, tuple);
			tree->lvarm =&  ~mask;
			--tree->lvarc;
			nvc = tree->tvarc - 1;
		}
		if (tree->rvarm & mask)
		{
			setvar(tree->right, var, intid, tuple);
			tree->rvarm =&  ~mask;
			nvc = tree->tvarc - 1;
		}
		tree->tvarc = nvc;
		return;

	  default:
		setvar(tree->left, var, intid, tuple);
		setvar(tree->right, var, intid, tuple);
		return;
	}
}


clearvar(tree1, var1)
struct querytree	*tree1;
int			var1;

/*
**	Clearvar is the opposite of setvar. For
**	each occurence of var1 in the tree, clear
**	the valptr.
*/

{
	register struct querytree	*tree;
	int				var;

	tree = tree1;
	if (!tree)
		return;

	var = var1;
	if (tree->sym.type == VAR)
	{
		if ((tree = ckvar(tree))->varno == var)
			tree->valptr = 0;
		return;
	}
	clearvar(tree->left, var);
	clearvar(tree->right, var);
}
