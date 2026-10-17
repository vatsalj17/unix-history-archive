/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"
#include	"tokens.h"
#include	"cnodes.h"
#include	"types.h"
#include	"flow.h"
#include	"func.h"

extern cnset	expressions;

int
flow_flags(x, id)
register xnode	*x;
register ident	*id;
{
    while (x != NULL)
    {
	if ((x->x_flags & (C_ID_BEGIN | C_ID_END | C_ID_BECOMES)) == 0)
	{
	    if (x->x_value.d == id)
		return x->x_flags;

	    if (x->x_value.d > id)
		return 0;
	}

	x = x->x_left;
    }

    return 0;
}

static void
fp_add(x, n, f)
register xnode	*x;
register xnode	**n;
register int	f;
{
    register xnode	*y;

    while (x != NULL)
    {
	if (*n == NULL)
	{
	    *n = x;

	    while (x != NULL)
	    {
		x->x_flags = f;
		x = x->x_left;
	    }

	    return;
	}

	if (x->x_value.d > (*n)->x_value.d)
	    n = &(*n)->x_left;
	else if (x->x_value.d == (*n)->x_value.d)
	{
	    (*n)->x_flags |= f;
	    y = x->x_left;
	    free_xnode(x);
	    x = y;
	}
	else
	{
	    x->x_flags = f;
	    y = x->x_left;
	    x->x_left = *n;
	    *n = x;
	    x = y;
	}
    }
}

/*
 *	Convert a flow set from time efficient
 *	to space efficient representation.
 */
xnode	*
flow_pack(p)
register flow_set	*p;
{
    xnode	*x;

    x = NULL;

    fp_add(p->f_cut, &x, C_X_CUT);
    fp_add(p->f_mod, &x, C_X_MOD);
    fp_add(p->f_ref, &x, C_X_REF);
    fp_add(p->f_hide, &x, C_X_HIDE);
    fp_add(p->f_cmod, &x, C_X_CMOD);

    return x;
}

/*
 *	Free a id set.
 */
void
free_xlist(x)
register xnode	*x;
{
    register xnode	*t;

    while (x != NULL)
    {
	t = x->x_left;
	free_xnode(x);
	x = t;
    }
}

/*
 *	Add an element to a set.
 */
void
add_element(id, n)
register ident	*id;
register xnode	**n;
{
    register xnode	*x;

    while (*n != NULL)
    {
	if ((*n)->x_value.d == id)
	    return;

	if ((*n)->x_value.d > id)
	    break;

	n = &(*n)->x_left;
    }

    x = new_xnode();
    x->x_left = *n;
    x->x_value.d = id;
    *n = x;
}

/*
 *	Check for id being in set x.
 */
int
is_member(id, x)
register ident	*id;
register xnode	*x;
{
    while (x != NULL)
    {
	if (x->x_value.d == id)
	    return 1;

	if (x->x_value.d > id)
	    return 0;

	x = x->x_left;
    }

    return 0;
}

/*
 *	Calculate union of two flow sets.
 */
xnode *
flow_union(p, q)
register xnode	*p;
register xnode	*q;
{
    register xnode	**n;
    register xnode	*t;
    xnode		*result;

    n = &result;

    while (p != NULL && q != NULL)
    {
	if (p->x_value.d < q->x_value.d)
	{
	    *n = p;
	    n = &p->x_left;
	    p = p->x_left;
	}
	else
	{
	    if (p->x_value.d == q->x_value.d)
	    {
		t = p->x_left;
		free_xnode(p);
		p = t;
	    }
	    *n = q;
	    n = &q->x_left;
	    q = q->x_left;
	}
    }

    while (p != NULL)
    {
	*n = p;
	n = &p->x_left;
	p = p->x_left;
    }

    *n = q;

    return result;
}

/*
 *	Calculate intersection of two flow sets.
 */
xnode *
flow_inter(p, q)
register xnode	*p;
register xnode	*q;
{
    register xnode	**n;
    register xnode	*t;
    xnode		*result;

    n = &result;

    while (p != NULL && q != NULL)
    {
	if (p->x_value.d < q->x_value.d)
	{
	    t = p->x_left;
	    free_xnode(p);
	    p = t;
	}
	else
	{
	    if (p->x_value.d == q->x_value.d)
	    {
		*n = p;
		n = &p->x_left;
		p = p->x_left;
	    }
	    t = q->x_left;
	    free_xnode(q);
	    q = t;
	}
    }

    while (p != NULL)
    {
	t = p->x_left;
	free_xnode(p);
	p = t;
    }

    while (q != NULL)
    {
	t = q->x_left;
	free_xnode(q);
	q = t;
    }

    *n = NULL;
    return result;
}

/*
 *	Find the elements in p that aren't in q.
 */
xnode	*
flow_diff(p, q)
register xnode	*p;
register xnode	*q;
{
    register xnode	**n;
    register xnode	*t;
    xnode		*result;

    n = &result;

    while (p != NULL && q != NULL)
    {
	if (p->x_value.d < q->x_value.d)
	{
	    *n = p;
	    n = &p->x_left;
	    p = p->x_left;
	}
	else
	{
	    if (p->x_value.d == q->x_value.d)
	    {
		t = p->x_left;
		free_xnode(p);
		p = t;
	    }

	    q = q->x_left;
	}
    }

    *n = p;
    return result;
}

/*
 *	Copy a flow lsit.
 */
xnode	*
copy_xlist(x)
register xnode	*x;
{
    register xnode	**n;
    xnode		*result;

    n = &result;

    while (x != NULL)
    {
	*n = new_xnode();
	(*n)->x_value.d = x->x_value.d;
	n = &(*n)->x_left;
	x = x->x_left;
    }

    *n = NULL;

    return result;
}

/*
 *	Check for nil intersection of two flow sets.
 *	Intersection means undefined order of evaluation.
 */
void
eval_check(p, q)
register xnode	*p;
register xnode	*q;
{
    while (p != NULL && q != NULL)
    {
	if (p->x_value.d < q->x_value.d)
	    p = p->x_left;
	else if (p->x_value.d > q->x_value.d)
	    q = q->x_left;
	else
	{
	    add_element(p->x_value.d, &err_set);
	    p = p->x_left;
	    q = q->x_left;
	}
    }
}


/*
 *	Check for nil intersection of two flow sets.
 *	The left operand is known to contain only one element.
 *	Intersection means undefined order of modification.
 */
void
mod_check(s0, s1)
flow_set	*s0;
flow_set	*s1;
{
    register ident	*id;
    register xnode	*p;

    id = s0->f_mod->x_value.d;

    for (p = s1->f_mod; p != NULL; p = p->x_left)
    {
	if (p->x_value.d == id)
	    goto mod_error;

	if (p->x_value.d > id)
	    break;
    }

    for (p = s1->f_cmod; p != NULL; p = p->x_left)
    {
	if (p->x_value.d == id)
	    goto mod_error;

	if (p->x_value.d > id)
	    break;
    }

    return;

mod_error:
    add_element(id, &merr_set);
}

/*
 *	Find and tag pure identifiers.
 */
void
make_pure(id)
register ident	*id;
{
    while (id != NULL)
    {
	if
	(
	    (id->id_value.i & (ARGUMENT | AUTO)) != 0
	    &&
	    (
		id->id_type->x_what == xt_basetype
		||
		id->id_type->x_what == xt_ptrto
	    )
	    &&
	    (id->id_flags & IDIS_UANDED) == 0
	)
	    id->id_value.i |= PURE;

	id = id->id_memblist;
    }
}

/*
 *	Print flow errors.
 */
void
print_errs()
{
    register xnode	*x;
    register xnode	*y;

    if (err_set != NULL)
    {
	x = err_set;

	do
	{
	    y = x->x_left;
	    errorf("undefined order of evaluation for variable '%'", x->x_value.d->id_name);
	    free_xnode(x);
	    x = y;
	}
	while (x != NULL);

	err_set = NULL;
    }

    if (merr_set != NULL)
    {
	x = merr_set;

	do
	{
	    y = x->x_left;
	    errorf("undefined order of modification for variable '%'", x->x_value.d->id_name);
	    free_xnode(x);
	    x = y;
	}
	while (x != NULL);

	merr_set = NULL;
    }
}


/*
 *	Seek back and sync.
 */
void
sync_back(c)
register cnode	*c;
{
    while (c != NULL)
    {
	if (c->c_what == ct_sync)
	{
	    csync(c);
	    return;
	}

	c = c->c_last;
    }

    sync(func.fn_token);
}

