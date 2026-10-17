/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"
#include	"sort.h"

#define	se_x	so_u.so_x
#define	se_neg	so_value

extern xtset	constants;

/*
 *	Sethi Uhlman tree balancing and optimisation.
 */
pack_t	x_from_su[]	=
{
    pack(xt_null),
    pack(xt_and),
    pack(xt_bar),
    pack(xt_plus),
    pack(xt_times),
    pack(xt_uparrow),
};

/*
 *	String out subtrees starting from `x' in the equivalence
 *	class `seq'.  `neg' is used for su_plus class.  It indicates
 *	that this subtree is negated.
 */
string_out(x, seq, neg)
register xnode	*x;
pack_t		seq;
int		neg;
{
    register xnode	*y;

    while (xdope[(int)x->x_what].xd_suequiv == seq)
    {
	if (x->x_what == xt_uminus)
	{
	    string_out(x->x_left, seq, 1 - neg);
	    return;
	}
	string_out(x->x_left, seq, neg);
	if (x->x_what == xt_minus)
	    neg = 1 - neg;
	y = x;
	x = x->x_right;
	free_xnode(y);
    }

    if (sort_index == sort_size)
    	sort_extend();

    sort_vect[sort_index].se_x = x;
    sort_vect[sort_index].se_neg = neg;

    sort_index++;
}

/*
 *	Build up the new tree.
 */
xnode	*
su_build(type, seq, const)
register xnode	*type;
pack_t		seq;
int		const;
{
    register xnode	*x;
    register xnode	*y;
    register int	neg;
    register int	i;
    register xnodes	xt;
    int			const_base;

    if (const)
    {
	i = 0;

	while (!in(constants, sort_vect[i].se_x->x_what))
	{
	    if (++i == sort_index)
		return NULL;
	}
	const_base = i;
    }
    else
	i = 0;

    xt = xunpack(x_from_su[(int)seq - (int)su_null]);

    if (xt == xt_plus)
	neg = sort_vect[i].se_neg;
    else
	neg = 0;

    y = sort_vect[i].se_x;

    for (i++; i < sort_index; i++)
    {
	x = new_xnode();
	x->x_type = type;
	x->x_flags = 0;

	if (xt == xt_plus)
	{
	    if (neg)
	    {
		if (sort_vect[i].se_neg)
		{
		    x->x_what = xt_plus;
		    x->x_left = y;
		    x->x_right = sort_vect[i].se_x;
		}
		else
		{
		    neg = 0;
		    x->x_what = xt_minus;
		    x->x_left = sort_vect[i].se_x;
		    x->x_right = y;
		}
	    }
	    else
	    {
		if (sort_vect[i].se_neg)
		    x->x_what = xt_minus;
		else
		    x->x_what = xt_plus;
		x->x_left = y;
		x->x_right = sort_vect[i].se_x;
	    }
	}
	else
	{
	    x->x_what = xt;
	    x->x_left = y;
	    x->x_right = sort_vect[i].se_x;
	}
	y = x;
    }

    if (const)
    {
	sort_vect[const_base].se_x = y;
	sort_vect[const_base].se_neg = neg;
	sort_index = const_base + 1;
    }
    else if (neg)
    {
	x = new_xnode();
	x->x_type = type;
	x->x_flags = 0;
	x->x_what = xt_uminus;
	x->x_left = y;
	x->x_right = NULL;
	return x;
    }

    return y;
}

/*
 *	Su comparison.
 */
su_cmp(p, q)
register sorts	*p;
register sorts	*q;
{
    return q->se_x->x_su - p->se_x->x_su;
}

/*
 *	String out `x'.  Sort the subtrees and build a new tree.
 */
xnode	*
su_opt(x, seq)
xnode	*x;
pack_t	seq;
{
    xnode	*type;

    type = x->x_type;
    sort_index = 0;
    string_out(x, seq, 0);
    quicksort(sort_vect, sort_index, su_cmp);
    (void) su_build(type, seq, 1);
    return traverse(su_build(type, seq, 0), 0);
}
