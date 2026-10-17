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

extern xtset	assops;
extern xtset	constants;
extern xtset	binary_ops;
extern xtset	unary_ops;

#define	CUT_LEFT	0x0001
#define	CUT_UNION	0x0002
#define	CUT_INTER	0x0004
#define	MOD_LEFT	0x0008
#define	MOD_UNION	0x0010
#define	MOD_INTER	0x0020
#define	REF_LR		0x0040
#define	REF_UNION	0x0080
#define	HIDE_UNION	0x0100
#define	HIDE_CRIGHT	0x0200
#define	HIDE_SEQ	0x0400
#define	CMOD_UNION	0x0800
#define	CMOD_CRIGHT	0x1000
#define	CMOD_MOD	0x2000
#define	EVAL_ORDER	0x4000
#define	MOD_ORDER	0x8000
#define	BIN_OP		(CUT_UNION | MOD_UNION | REF_UNION | HIDE_UNION | CMOD_UNION | CMOD_MOD)

/*
 *	Flow set equations.
 */
static void
flow_calc(instr, s0, s1, d)
register int		instr;
register flow_set	*s0;
register flow_set	*s1;
register flow_set	*d;
{
    /*
     *	To test for dependencies on order of evaluation
     *	check ref vs mod and cmod.
     */
    if ((instr & EVAL_ORDER) != 0)
    {
	eval_check(s0->f_ref, s1->f_mod);
	eval_check(s0->f_mod, s1->f_ref);
	eval_check(s0->f_ref, s1->f_cmod);
	eval_check(s0->f_cmod, s1->f_ref);
    }

    /*
     *	To test for dependencies on order of evaluation
     *	check ref vs mod and cmod.
     */
    if ((instr & MOD_ORDER) != 0)
	mod_check(s0, s1);

    /*
     *	Hide sets.  Fortunately the messy cases are rare.
     */
    if ((instr & HIDE_UNION) != 0)
    {
	d->f_hide = flow_union(s0->f_hide, s1->f_hide);

	if ((instr & HIDE_CRIGHT) != 0)
	{
	    d->f_hide
	    =
	    flow_union
	    (
		d->f_hide,
		flow_diff
		(
		    flow_inter
		    (
			flow_union(copy_xlist(s0->f_mod), copy_xlist(s0->f_cmod)),
			copy_xlist(s1->f_ref)
		    ),
		    s1->f_mod
		)
	    );
	}
    }
    else if ((instr & HIDE_SEQ) != 0)
    {
	d->f_hide
	=
	flow_union
	(
	    s1->f_hide,
	    flow_diff
	    (
		flow_union
		(
		    s0->f_hide,
		    flow_inter
		    (
			flow_union(copy_xlist(s0->f_mod), copy_xlist(s0->f_cmod)),
			copy_xlist(s1->f_ref)
		    )
		),
		s1->f_mod
	    )
	);
    }

    /*
     *	ref set, either union or left to ride.
     */
    if ((instr & REF_UNION) != 0)
	d->f_ref = flow_union(s0->f_ref, s1->f_ref);
    else if ((instr & REF_LR) != 0)
	d->f_ref = flow_union(s0->f_ref, flow_diff(s1->f_ref, s0->f_mod));

    /*
     *	Always CMOD_UNION, and then perhaps mod sets.
     */
    d->f_cmod = flow_union(s0->f_cmod, s1->f_cmod);

    if ((instr & CMOD_CRIGHT) != 0)
	d->f_cmod = flow_union(d->f_cmod, copy_xlist(s1->f_mod));
    else if ((instr & CMOD_MOD) != 0)
    {
	d->f_cmod = flow_union(d->f_cmod, copy_xlist(s0->f_mod));
	d->f_cmod = flow_union(d->f_cmod, copy_xlist(s1->f_mod));
    }

    /*
     *	cut and mod sets are much neater.  They
     *	are left to last becuase the values of the
     *	operands are used for other calculations.
     */
    if ((instr & CUT_LEFT) != 0)
    {
	free_xlist(s1->f_cut);
	d->f_cut = s0->f_cut;
    }
    else if ((instr & CUT_UNION) != 0)
	d->f_cut = flow_union(s0->f_cut, s1->f_cut);
    else if ((instr & CUT_INTER) != 0)
	d->f_cut = flow_inter(s0->f_cut, s1->f_cut);

    if ((instr & MOD_LEFT) != 0)
    {
	free_xlist(s1->f_mod);
	d->f_mod = s0->f_mod;
    }
    else if ((instr & MOD_UNION) != 0)
	d->f_mod = flow_union(s0->f_mod, s1->f_mod);
    else if ((instr & MOD_INTER) != 0)
	d->f_mod = flow_inter(s0->f_mod, s1->f_mod);
}

/*
 *	Calculate a flow set for an expression.
 */
void
expr_flow(x, res)
register xnode		*x;
register flow_set	*res;
{
    register int	instr;
    flow_set		left_op;
    flow_set		right_op;

    res->f_cut = NULL;
    res->f_mod = NULL;
    res->f_ref = NULL;
    res->f_hide = NULL;
    res->f_cmod = NULL;

    switch (x->x_what)
    {
    case xt_andand:
    case xt_barbar:
	instr = CUT_LEFT | MOD_LEFT | REF_LR | HIDE_UNION | HIDE_CRIGHT | CMOD_UNION | CMOD_CRIGHT;

	expr_flow(x->x_right, &right_op);
	expr_flow(x->x_left, &left_op);
	break;

    case xt_comma:
    case xt_question:
	instr = CUT_UNION | MOD_UNION | REF_LR | HIDE_SEQ | CMOD_UNION;

	expr_flow(x->x_right, &right_op);
	expr_flow(x->x_left, &left_op);
	break;

    case xt_colon:
	instr = CUT_INTER | MOD_INTER | REF_UNION | HIDE_UNION | CMOD_UNION | CMOD_MOD;

	expr_flow(x->x_right, &right_op);
	expr_flow(x->x_left, &left_op);
	break;

    case xt_eq:
	expr_flow(x->x_right, &right_op);

	if (pure_var(x->x_left))
	{
	    register ident	*d;

	    instr = BIN_OP | MOD_ORDER;

	    d = x->x_left->x_left->x_value.d;

	    add_element(d, &eq_set);

	    make_set(&left_op.f_cut, d);
	    make_set(&left_op.f_mod, d);

	    left_op.f_ref = NULL;
	    left_op.f_hide = NULL;
	    left_op.f_cmod = NULL;
	}
	else
	{
	    instr = BIN_OP | EVAL_ORDER;

	    expr_flow(x->x_left, &left_op);
	}

	break;

    case xt_name:
	if (is_pure(x->x_value.d))
	    make_set(&res->f_ref, x->x_value.d);

	return;

    case xt_ptrpredec:
    case xt_ptrpreinc:
    case xt_predec:
    case xt_preinc:
	if (pure_var(x->x_left))
	{
	    register ident	*d;

	    d = x->x_left->x_left->x_value.d;

	    res->f_cut = NULL;

	    make_set(&res->f_mod, d);
	    make_set(&res->f_ref, d);
	    make_set(&res->f_hide, d);

	    res->f_cmod = NULL;
	}
	else
	    expr_flow(x->x_left, res);

	return;

    case xt_ptrpostdec:
    case xt_ptrpostinc:
    case xt_postdec:
    case xt_postinc:
	if (pure_var(x->x_left))
	{
	    register ident	*d;

	    d = x->x_left->x_left->x_value.d;

	    res->f_cut = NULL;

	    make_set(&res->f_mod, d);
	    make_set(&res->f_ref, d);

	    res->f_hide = NULL;
	    res->f_cmod = NULL;
	}
	else
	    expr_flow(x->x_left, res);

	return;

    case xt_call:
	expr_flow(x->x_left, res);

	if (x->x_value.k != NULL)
	{
	    register cnode	*c;

	    for (c = x->x_value.k->c_head; c != NULL; c = c->c_next)
	    {
		expr_flow(c->c_value.x, &left_op);
		flow_calc(BIN_OP | EVAL_ORDER, res, &left_op, &right_op);
		*res = right_op;
	    }
	}

	return;

    case xt_dot:
	if (x->x_left != NULL)
	    expr_flow(x->x_left, res);

	return;

    default:
	if (in(assops, x->x_what))
	{
	    expr_flow(x->x_right, &right_op);

	    if (pure_var(x->x_left))
	    {
		register ident	*d;

		instr = BIN_OP | MOD_ORDER;
		d = x->x_left->x_left->x_value.d;

		left_op.f_cut = NULL;

		make_set(&left_op.f_mod, d);
		make_set(&left_op.f_ref, d);

		left_op.f_hide = NULL;
		left_op.f_cmod = NULL;
	    }
	    else
	    {
		instr = BIN_OP;
		expr_flow(x->x_left, &left_op);
	    }
	}
	else if (in(binary_ops, x->x_what))
	{
	    instr = BIN_OP | EVAL_ORDER;

	    expr_flow(x->x_right, &right_op);
	    expr_flow(x->x_left, &left_op);
	}
	else if (in(constants, x->x_what))
	    return;
	else if (in(unary_ops, x->x_what))
	{
	    expr_flow(x->x_left, res);
	    return;
	}
	else
	    internal("expr_flow", "unkown xnode");
    }


    flow_calc(instr, &left_op, &right_op, res);
}
