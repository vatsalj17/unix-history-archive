/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"
#include	"tokens.h"
#include	"types.h"
#include	"xtypes.h"

/*
 *	'decl_item' uses 'dec_gather' and 'decl_body' to parse the part of a
 *	declaration consisting of modifiers on a basic type.  It then sorts out
 *	all the 'pointer to', 'array of' and 'function returning' things and
 *	any declared name.  It returns an expression tree.  The first node holds
 *	the name of any ident found in the declaration in its 'x_value.n'
 *	field.  The type hangs off the 'x_left' field.
 */

token		*name_token;
xnode		no_args;

extern int	comp_constant();

extern tkset	dimension_anchors;
extern tkset	field_anchors;

extern xnode	*decl_body();

/*
 *	Gather an optional argument list.
 */
xnode	*
opt_arg_list()
{
    register xnode	**y;
    register token	*t;
    xnode		*x;

    y = &x;

    while ((t = get_token())->t_what == tk_name)
    {
	*y = new_xnode();
	(*y)->x_what = xt_argname;
	(*y)->x_value.n = t->t_value.n;
	y = &(*y)->x_left;

	free_token(t);

	if ((t = get_token())->t_what != tk_comma)
	{
	    if (t->t_what == tk_name)
		errort(t, "',' missing between formal parameters");

	    unget_token(t);
	}
	else
	    free_token(t);
    }

    unget_token(t);

    *y = NULL;

    return x;
}

/*
 *	Declaration post-amble.  Functions and arrays.
 */
xnode	*
decl_gather(x)
register xnode	*x;
{
    register token	*t;
    register xnode	*y;
    register xnode	*z;

    x = decl_body(x);

    loop
    {
	t = get_token();

	if (t->t_what == tk_onround)
	{
	    free_token(t);
	    y = opt_arg_list();

	    (void)accept(tk_offround, "')' expected at logical end of function arguments");

	    z = new_xnode();
	    z->x_subtype = x;
	    x = z;
	    x->x_value.x = y;
	    x->x_what = xt_ftnreturning;
	}
	else if (t->t_what == tk_onsquare)
	{
	    free_token(t);

	    z = new_xnode();
	    z->x_subtype = x;
	    x = z;
	    x->x_what = xt_arrayof;

	    if ((t = get_token())->t_what != tk_offsquare)
	    {
		unget_token(t);

		x->x_flags = 0;
		(void)comp_constant(dimension_anchors, &x->x_value.i);

		t = get_token();

		if (x->x_value.i < 0)
		{
		    errort(t, "negative array dimension");
		    x->x_value.i = 0;
		}
		else if (x->x_value.i == 0)
		    warnt(t, "zero array dimension");

		if (t->t_what != tk_offsquare)
		{
		    unget_token(t);
		    errort(t, "']' expected after array dimension");
		}
		else
		    free_token(t);
	    }
	    else
	    {
		free_token(t);
		x->x_value.i = -1;
	    }
	}
	else
	    break;
    }

    unget_token(t);

    return x;
}

/*
 *	Declaration body.
 */
xnode	*
decl_body(x)
register xnode	*x;
{
    register token	*t;
    register token	*u;
    register xnode	*z;

    switch ((t = get_token())->t_what)
    {
    case tk_onround:
	if ((u = get_token())->t_what == tk_offround)
	{
	    unget_token(u);
	    unget_token(t);
	    return x;
	}

	unget_token(u);
	free_token(t);

	x = decl_gather(x);
	(void) accept(tk_offround, "')' expected in declarator");
	return x;

    case tk_times:
	free_token(t);
	z = new_xnode();
	z->x_what = xt_ptrto;
	z->x_value.i = 0;
	z->x_subtype = decl_gather(x);
	return z;

    case tk_name:
	z = new_xnode();
	z->x_what = xt_name;
	z->x_value.n = t->t_value.n;
	z->x_subtype = x;

	if (name_token != NULL)
	    free_token(name_token);

	name_token = t;
	return z;

    default:
	unget_token(t);
	return x;
    }
}

/*
 *	Parse a single declared item.  Use decl_gather to pick
 *	up the components and construct the new type using find_type.
 */
xnode	*
decl_item(type)
register xnode	*type;
{
    register xnode	*x;
    register xnode	*y;
    register xnode	*z;
    register xnode	*name;
    register token	*t;
    register xnode	*args;

    name = NULL;
    args = &no_args;

    for (x = decl_gather((xnode *)NULL); x != NULL; x = z)
    {
	z = x->x_subtype;

	if (x->x_what == xt_name)
	    name = x;
	else
	{
	    switch (x->x_what)
	    {
	    case xt_arrayof:
		if (type->x_what == xt_ftnreturning)
		{
		    Ea[0].x = type;
		    error("arrays cannot contain #s");
		}

		break;

	    case xt_ftnreturning:
		switch (type->x_what)
		{
		case xt_arrayof:
		case xt_ftnreturning:
		    Ea[0].x = type;
		    error("functions cannot return #s");
		}

		if (args != NULL && args != &no_args)
		    error("unexpected function argument list");

		args = x->x_value.x;
		x->x_value.x = NULL;
	    }

	    y = x;
	    y->x_subtype = type;
	    type = find_type(y);
	}
    }

    if ((t = get_token())->t_what == tk_colon)
    {
	if (type != type_int && type != type_uint)
	{
	    Ea[0].x = type;
	    errort(t, "bitfields may not be a part of #s");
	    type = (type->x_value.i & UNSIGNED) ? type_uint : type_int;
	}

	free_token(t);

	x = new_xnode();
	x->x_what = xt_bitfield;
	x->x_subtype = type;
	(void)comp_constant(field_anchors, &x->x_value.i);
	type = find_type(x);
    }
    else
	unget_token(t);

    if (name == NULL)
    {
	name = new_xnode();
	name->x_value.n = NULL;
	name->x_what = xt_name;
    }

    name->x_left = type;
    name->x_right = args;

    return name;
}
