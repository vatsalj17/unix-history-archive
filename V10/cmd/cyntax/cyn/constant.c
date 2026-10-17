/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */
#include	"defs.h"
#include	"cnodes.h"
#include	"tokens.h"
#include	"types.h"
#include	"xtypes.h"
#include	"void.h"

/*
 *	Constant expression routines.
 */

extern xtset	constants;
extern xtset	int_constants;
extern token	*name_token;

/*
 *	Initialiser expression.
 */
xnode	*
init_expr(want_type, anchors)
xnodes	want_type;
setptr	anchors;
{
    register xnode	*x;
    register token	*t;

    if ((x = expr(0)) == NULL)
    {
	t = get_token();
	goto skip_to_anchor;
    }

    if (want_type == xt_clong || want_type == xt_cdouble)
    {
	if
	(
	    x->x_type->x_what == xt_enum
	    ||
	    (x->x_type->x_what == xt_basetype && !isvoid(x->x_type))
	)
	    x = cast(x, want_type == xt_clong ? type_long : type_double);
	else
	{
	    error("constant scalar expression expected");
	    return NULL;
	}
    }

    x = traverse(x, 0);

    if (want_type == xt_caddr && null_ptr(x))
	return x;

    if (x->x_what == xt_caddr)
    {
	if (x->x_left != NULL && (x->x_left->x_value.d->id_value.i & (ARGUMENT | AUTO)) != 0)
	{
	    register ident	*id;

	    id = x->x_left->x_value.d;
	    Ea[0].m = (id->id_value.i & AUTO) != 0 ? "argument" : "auto";
	    Ea[1].m = id->id_name;
	    t = get_token();
	    errort(t, "a reference to an % (%) is not a constant pointer");
	    goto skip_to_anchor;
	}

	return x;
    }

    if (in(constants, x->x_what))
	return x;

    t = get_token();
    errort(t, "initialiser expression expected");

skip_to_anchor:
    while (!in(anchors, t->t_what))
	t = get_token();

    unget_token(t);
    return NULL;
}

/*
 *	Compiler constant expression (case, array bound, etc.)
 */
int
comp_constant(anchors, p)
setptr	anchors;
long	*p;
{
    register xnode	*x;
    register token	*t;
    register int	gobble;

    t = name_token;
    x = expr(0);
    name_token = t;
    if (x == NULL)
	    goto chomp;

    x = traverse(x, 0);

    if (in(int_constants, x->x_what))
    {
	*p = x->x_value.i;
	free_xnode(x);
	return 0;
    }
    error("constant integer expression expected");
chomp:

    gobble = 0;
    while (t = get_token(), !in(anchors, t->t_what))
        gobble++;

    unget_token(t);

    /*
    if (gobble>0)
	error("constant integer expression botch");
    else
	error("constant integer expression expected");
    */
    return 1;
}
