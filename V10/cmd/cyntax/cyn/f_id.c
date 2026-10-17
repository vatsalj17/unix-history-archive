/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"
#include	"cnodes.h"
#include	"types.h"
#include	"flow.h"

static ident	**id_vector;
static int	id_limit;

static void
make_new_id(id, ord)
ident	*id;
int	ord;
{
    register ident	*nid;

    nid = talloc(ident);
    *nid = *id;
    nid->id_flags |= IDIS_ALLOC;
    nid->id_next = alloced_ids;
    alloced_ids = nid;
    id_vector[ord] = nid;
}

static void
id_add(c, id, ord, flag)
register cnode    *c;
ident		*id;
register int	ord;
int		flag;
{
    register xnode	*x;

    if (ord < 0 || ord >= id_limit)
	internal("id_add", "ord out of range");

    if (id_vector[ord] == NULL)
	make_new_id(id, ord);

    x = new_xnode();
    x->x_left = c->c_flow;
    x->x_value.d = id_vector[ord];
    x->x_flags = flag;

    c->c_flow = x;
}

extern xtset	constants;
extern xtset	binary_ops;
extern xtset	unary_ops;

static void
id_paint(x, old, new)
register xnode	*x;
ident		*old;
ident		*new;
{
    if (x == NULL)
	return;

    if (in(binary_ops, x->x_what))
    {
	id_paint(x->x_right, old, new);
	id_paint(x->x_left, old, new);
    }
    else if (in(unary_ops, x->x_what))
    {
	if (x->x_what == xt_call && x->x_value.k != NULL)
	{
	    register cnode	*c;

	    for (c = x->x_value.k->c_head; c != NULL; c = c->c_next)
		id_paint(c->c_value.x, old, new);
	}

	id_paint(x->x_left, old, new);
    }
    else if (x->x_what == xt_name && x->x_value.d == old)
	x->x_value.d = new;
}

static int
id_cut(x, old, new_in, new_out)
register xnode	*x;
ident		*old;
ident		*new_in;
ident		*new_out;
{
    register int	ret;

    ret = 0;

    switch (x->x_what)
    {
    case xt_andand:
    case xt_barbar:
	ret = id_cut(x->x_left, old, new_in, new_out);
	id_paint(x->x_right, old, new_out);
	break;

    case xt_comma:
    case xt_question:
	if (id_cut(x->x_left, old, new_in, new_out))
	{
	    ret = 1;
	    id_paint(x->x_right, old, new_out);
	}
	else
	    ret = id_cut(x->x_right, old, new_in, new_out);

	break;

    case xt_eq:
	if (pure_var(x->x_left) && x->x_left->x_left->x_value.d == old)
	{
	    ret = 1;
	    x->x_left->x_left->x_value.d = new_in;
	    id_paint(x->x_right, old, new_out);
	}
	else
	    ret = id_cut(x->x_left, old, new_in, new_out) + id_cut(x->x_right, old, new_in, new_out);

	break;

    case xt_name:
	/*
	 *	This happens when a vret or a switch is
	 *	approached from below (not a flow path).
	 *	There is no cut.
	 */
	if (x->x_value.d == old)
	    x->x_value.d = new_in;

	break;

    case xt_call:
	ret = id_cut(x->x_left, old, new_in, new_out);

	if (x->x_value.k != NULL)
	{
	    register cnode	*c;

	    for (c = x->x_value.k->c_head; c != NULL; c = c->c_next)
		ret += id_cut(c->c_value.x, old, new_in, new_out);
	}

	break;

    case xt_dot:
	if (x->x_left != NULL)
	    ret = id_cut(x->x_left, old, new_in, new_out);

	break;

    default:
	if (in(binary_ops, x->x_what))
	    ret = id_cut(x->x_left, old, new_in, new_out) + id_cut(x->x_right, old, new_in, new_out);
	else if (in(constants, x->x_what))
	    break;
	else if (in(unary_ops, x->x_what))
	    ret = id_cut(x->x_left, old, new_in, new_out);
	else
	    internal("id_cut", "unknown xnode");
    }

    return ret;
}

void
alloc_ids(tail, id, ord)
cnode		*tail;
register ident	*id;
register int	ord;
{
    register cnode	*c;
    register int	i;
    register int	cur;

    if (ord > id_limit)
    {
	id_limit = ord;
	id_vector = vector(id_vector, id_limit, ident *);
    }

    for (i = 0; i < ord; i++)
	id_vector[i] = NULL;

    cur = L_DEAD;

    for (c = tail; c != NULL; c = c->c_last)
    {
	if (c->c_what == ct_sync)
	    csync(c);
	if (c->c_ord != cur)
	{
	    if (cur == L_DEAD)
	    {
		id_add(c, id, c->c_ord, C_ID_END);

		if (in(expressions, c->c_what))
		    id_paint(c->c_value.x, id, id_vector[c->c_ord]);
	    }
	    else if (c->c_ord == L_DEAD)
	    {
		id_add(c, id, cur, C_ID_BEGIN);

		if (in(expressions, c->c_what))
		    id_paint(c->c_value.x, id, id_vector[cur]);
	    }
	    else
	    {
		id_add(c, id, cur, C_ID_BEGIN);
		id_add(c, id, c->c_ord, C_ID_BECOMES);

		if (in(expressions, c->c_what))
		    (void)id_cut(c->c_value.x, id, id_vector[c->c_ord], id_vector[cur]);
	    }

	    cur = c->c_ord;
	}
	else if (cur != L_DEAD && in(expressions, c->c_what))
	    id_paint(c->c_value.x, id, id_vector[cur]);
    }
}
