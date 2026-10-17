# include	"../ingres.h"
# include	"../tree.h"
# include	"../symbol.h"

/* DECOMP3 -- This file contains routines associated with redefining
**	attribute numbers. This is needed when one variable sub queries
**	or reduction change the positions of attributes in a relation.
**	This file includes:
**
**	Tempvar -- Change the attribute numbers to new ones.
**
**	Origvar -- Restore attribute numbers back to their previous values.
**
**	Ckvar   -- Return the currently active VAR node
*/



tempvar(node, sqlist, buf)
struct querytree	*node;
struct querytree	*sqlist[];
char			*buf;

/*
** Tempvar -- Replace a VAR attribute number with its new number.
**
**	Tempvar is given a list of subqueries which will potentially
**	alter the attribute numbers of VARs they reference. An attno
**	is changed by making the current VAR point to a new VAR node
**	which has the updated attno.
**
**	The new attno is determined from the target list of the subquery
**	for that VAR. The RESDOM number is the new attno and the VAR it
**	points to is the old attno. For example:
**		RESDOM/2 -> right = VAR 1/3
**	The right subtree of result domain 2 is domain 3 of variable 1.
**	Thus domain 3 should be renumbered to be domain 2.
*/

{
	register struct querytree	*v, *sq, *nod;
	int				attno;
	struct querytree		*ckvar();

	if ((nod = node) == NULL)
		return;

	if (nod->sym.type == VAR )
	{
		nod = ckvar(nod);
		if (sq = sqlist[nod->varno])
		{
			/* This var has a subquery on it */

			/* allocate a new VAR node */
			if (buf)
			{
				v = nod->valptr = need(buf, 12);
				v->left = v->right = v->valptr = 0;
				bmove(&nod->sym, &v->sym, 6);
				nod->varno = -1;
			}
			else
				v = nod;

			/* search for the new attno */
			for (sq = sq->left; sq->sym.type != TREE; sq = sq->left)
			{
				if (ckvar(sq->right)->attno == nod->attno) 
				{
	
					v->attno = sq->resno;
#					ifdef xDTR1
					if (tTf(12, 3))
					{
						printf("Tempvar:");
						writenod(nod);
					}
#					endif

					return;
				}
			}
			syserr("tempvar:dom %d of %s missing", nod->attno, rangename(nod->varno));
		}
		return;
	}

	tempvar(nod->left, sqlist, buf);
	tempvar(nod->right, sqlist, buf);
}




origvar(node, sqlist)
struct querytree	*node;
struct querytree	*sqlist[];

/*
** Origvar -- Restore VAR node to previous state.
**
**	Origvar undoes the effect of tempvar. All vars listed
**	in the sqlist will have their most recent tempvar removed.
*/

{
	register struct querytree	*t;
	register int			v;

	t = node;
	if (!t)
		return;
	if (t->sym.type == VAR && t->varno < 0)
	{
		for (; t->valptr->varno<0; t=t->valptr);
		if (sqlist[v=t->valptr->varno])
		{
			t->varno = v;
			t->valptr = 0;
		}
		return;
	}
	origvar(t->left, sqlist);
	origvar(t->right, sqlist);
}



struct querytree *ckvar(node)
struct querytree *node;

/*
** Ckvar -- Return pointer to currently "active" VAR.
**
**	This routine guarantees that "t" will point to
**	the most current definition of the VAR.
*/

{
	register struct querytree	*t;

	t = node;
	if (t->sym.type != VAR)
	{
		syserr("ckvar: not a VAR %d", t->sym.type);
	}
	while (t->varno < 0)
		t = t->valptr;
	return(t);
}
