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
#include	"void.h"

extern xnode	*test_expr();

extern xtset	int_constants;
extern tkdope	tkdopes[];

extern xtset	assops;
extern tkset	expr_ops;
extern tkset	assop_starts;
extern tkset	enum_warn;

/*
 *	Make a cast node with x as its son and t as its type.
 */
xnode	*
cast(x, t)
xnode	*x;
xnode	*t;
{
    register xnode	*y;

    y = new_xnode();
    y->x_what = xt_cast;
    y->x_left = x;
    y->x_right = NULL;
    y->x_type = t;
    y->x_flags = 0;
    return y;
}

/*
 *	Could x be a NULL?
 */
int
null_ptr(x)
register xnode	*x;
{
    if
    (
	(
	    in(int_constants, x->x_what)
	    &&
	    x->x_value.i == 0
	)
	||
	(
	    x->x_what == xt_name
	    &&
	    x->x_type->x_what == xt_enum
	    &&
	    x->x_value.i == 0
	)
    )
    {
	x->x_what = xt_cnull;
	return 1;
    }

    return 0;
}

/*
 *	Type checking for a binary operator, inserting casts if necessary
 *	Return NULL on error after giving diagnostic.
 */
xnode	*
fix_bin_type(x)
register xnode	*x;
{
    register xnode	*l;
    register xnode	*r;
    xnode		*y;
    register int	is_u;
    register int	is_l;
    register int	complex;
    int			enums;

    l = x->x_left->x_type;
    r = x->x_right->x_type;

    /*
     *	Enumerated type conversion and
     *	check for dubious combinations.
     */
    enums = 0;

    if (l->x_what == xt_enum)
	enums++;

    if (r->x_what == xt_enum)
	enums += 2;

    switch (enums)
    {
    case 1:
	l = type_int;
	break;

    case 2:
	r = type_int;
	break;

    case 3:
	if (l != r && in(enum_warn, x->x_what))
	{
	    Ea[0].m = xdope[(int)x->x_what].xd_name;
	    Ea[1].x = x->x_left;
	    Ea[2].x = x->x_right;
	    warn("operands of '%' are $ and $");
	}

	l = type_int;
	r = type_int;
    }

    /*
     *	Simple size conversions.
     */
    if (l == type_float)
	l = type_double;
    else if (l == type_char || l == type_short)
	l = type_int;
    else if (l == type_uchar || l == type_ushort)
	l = type_uint;
    else if (l->x_what == xt_bitfield)
    {
	x->x_left = extract_field(x->x_left);
	l = x->x_left->x_type;
    }

    if (r == type_float)
	r = type_double;
    else if (r == type_char || r == type_short)
	r = type_int;
    else if (r == type_uchar || r == type_ushort)
	r = type_uint;
    else if (r->x_what == xt_bitfield)
    {
	x->x_right = extract_field(x->x_right);
	r = x->x_right->x_type;
    }

    complex = 0;

    /*
     *	Implicit arithmentic conversions.
     */
    if (l == type_double)
    {
	if (r->x_what == xt_basetype)
	    r = type_double;
	else
	    complex++;
    }
    else if (r == type_double)
    {
	if (l->x_what == xt_basetype)
	    l = type_double;
	else
	    complex++;
    }
    else
    {
	/*
	 *	This determines if we
	 *	should consider this
	 *	operation unsigned or long.
	 */
	is_u = 0;
	is_l = 0;

	if (l == type_uint)
	    is_u++;
	else if (l == type_long)
	    is_l++;
	else if (l == type_ulong)
	{
	    is_u++;
	    is_l++;
	}
	else if (l != type_int)
	    complex++;

	if (r == type_uint)
	    is_u++;
	else if (r == type_long)
	    is_l++;
	else if (r == type_ulong)
	{
	    is_u++;
	    is_l++;
	}
	else if (r != type_int)
	    complex++;

	if (!complex)
	{
	    if (is_u)
	    {
		if (is_l)
		    l = type_ulong;
		else
		    l = type_uint;
	    }
	    else
	    {
		if (is_l)
		    l = type_long;
		else
		    l = type_int;
	    }

	    r = l;
	}
    }

    switch (x->x_what)
    {
    case xt_eq:
	if (complex && l != r)
	{
	    /*
	     *	Check for pointer = NULL.
	     *  Void * assigments, too.
	     */
	    if (l->x_what == xt_ptrto && 
		(null_ptr(x->x_right) || isvoidp(l) || isvoidp(r)))
	    {
		x->x_right->x_type = l;
		x->x_type = l;
	    }
	    else if
	    (
		l->x_what == xt_bitfield
		&&
		(
		    r == type_int
		    ||
		    r == type_uint
		)
	    )
	    {
		/*
		 *	Bitfield assignment.
		 */
		x->x_what = xt_insf;
		x->x_type = r;
	    }
	    else
		goto incompat;
	}
	else
	{
	    /*
	     *	Cast the source to the type of the destination.
	     */
	    x->x_type = x->x_left->x_type;

	    if (x->x_right->x_type != x->x_left->x_type)
		x->x_right = cast(x->x_right, x->x_left->x_type);
	}

	return x;

    case xt_ptrplus:
	/*
	 *	Created from subscript expression.
	 *	Left op known to be ptrto.
	 *	Right op known to be integral.
	 */
	if(isvoidp(l))
	    goto incompat;
	r = type_int;
	goto leftprop;

	/*
	 *	Float/int arithemetics.
	 */
    case xt_minuseq:
    case xt_pluseq:
    case xt_slasheq:
    case xt_timeseq:
    case xt_plus:
    case xt_minus:
    case xt_times:
    case xt_slash:
	if (complex)
	{
	    if (x->x_what == xt_plus)
	    {
		/*
		 *	Pointer plus int.
		 */
		if (complex == 2)
			goto incompat;

		if
		(
		    l->x_what == xt_ptrto
		    &&
		    r->x_what == xt_basetype
		    &&
		    (r->x_value.i & INT) != 0
		)
		    r = type_int;
		else if
		(
		    r->x_what == xt_ptrto
		    &&
		    l->x_what == xt_basetype
		    &&
		    (l->x_value.i & INT) != 0
		)
		{
		    y = x->x_left;
		    x->x_left = x->x_right;
		    x->x_right = y;
		    l = r;
		    r = type_int;
		}
		else
			goto incompat;

		x->x_what = xt_ptrplus;
		goto leftprop;
	    }
	    else if (l->x_what == xt_ptrto)
	    {
		switch (x->x_what)
		{
		case xt_minus:
		    if (r->x_what == xt_basetype && (r->x_value.i & INT) != 0)
		    {
			/*
			 *	Pointer minus int.
			 */
			x->x_what = xt_ptrminus;
			r = type_int;
			goto leftprop;
		    }
		    else if (l == r)
		    {
			/*
			 *	Pointer minus pointer.
			 */
			x->x_what = xt_ptrdiff;
			x->x_type = type_int;
			return x;
		    }
		    else
			goto incompat;

		case xt_minuseq:
		case xt_pluseq:
		    /*
		     *	Pointer [+-]= int.
		     */
		    if (r->x_what == xt_basetype && (r->x_value.i & INT) != 0)
		    {
			if (x->x_what == xt_pluseq)
			    x->x_what = xt_ptrpluseq;
			else
			    x->x_what = xt_ptrminuseq;

			r = type_int;
			goto leftprop;
		    }
		}
		goto incompat;
	    }
	    else
		goto incompat;
	}
	else if (in(assops, x->x_what))
	{
	    /*
	     *	Arithmetic assignment operator.
	     *	It's hard if left hand type is not
	     *	equal to right hand type.
	     */
	    x->x_type = x->x_left->x_type;

	    if (x->x_left->x_type != r)
		x->x_flags |= XIS_HARDASSOP;

	    goto rightcast;
	}

	goto leftprop;

	/*
	 *	Bit operations.
	 */

	/*
	 *	Shifts.  Right is int.
	 */
    case xt_lessless:
    case xt_lslseq:
    case xt_grgreq:
    case xt_grtgrt:
	if (complex || (l->x_value.i & INT) == 0 || (r->x_value.i & INT) == 0)
	    goto incompat;
	else
	{
	    r = type_int;
	    goto leftprop;
	}

	/*
	 *	Ands and ors.
	 */
    case xt_and:
    case xt_andeq:
    case xt_bar:
    case xt_bareq:
    case xt_percent:
    case xt_pereq:
    case xt_uparrow:
    case xt_uparroweq:
	if (complex || (l->x_value.i & INT) == 0 || (r->x_value.i & INT) == 0)
	    goto incompat;
	else if (in(assops, x->x_what))
	{
	    /*
	     *	Bit assignment operator.
	     *	It's hard if left hand type is not
	     *	equal to right hand type.
	     */
	    if (x->x_left->x_type != r)
		x->x_flags |= XIS_HARDASSOP;

	    x->x_type = x->x_left->x_type;
	    goto rightcast;
	}
	else
	    goto leftprop;

	/*
	 *	McCarty and and or.
	 */
    case xt_barbar:
    case xt_andand:
	if (l->x_what != xt_basetype && l->x_what != xt_ptrto)
	    goto incompat;

	if (r->x_what != xt_basetype && r->x_what != xt_ptrto)
	    goto incompat;

	x->x_type = type_int;
	goto addcasts;

	/*
	 *	Equality comparisons.
	 */
    case xt_eqeq:
    case xt_noteq:
	switch (complex)
	{
	case 0:
	    x->x_type = type_int;
	    goto addcasts;

	case 1:
	    /*
	     *	Comparison of pointer and NULL
	     */
	    if (r->x_what == xt_ptrto && null_ptr(x->x_left))
		x->x_left->x_type = r;
	    else if (l->x_what == xt_ptrto && null_ptr(x->x_right))
		x->x_right->x_type = l;
	    else
		goto incompat;

	    break;

	case 2:
	    /*
	     *  Only comparison of ptrs of same types or of
	     *  pointer with void* allowed
	     */
	    if (r->x_what == xt_ptrto && (r==l || isvoidp(l)))
		x->x_right->x_type = l;
	    else if (l->x_what == xt_ptrto && isvoidp(r))
		x->x_left->x_type = r;
	    else
		goto incompat;
	}

	x->x_type = type_int;
	return x;

	/*
	 *	Inequality comparisons.
	 */
    case xt_less:
    case xt_grt:
    case xt_lesseq:
    case xt_grteq:
	switch (complex)
	{
	case 0:
	    x->x_type = type_int;
	    goto addcasts;

	case 1:
	    /*
	     *	Comparison of pointer and NULL not allowed.
	     */
	    goto incompat;

	    break;

	case 2:
	    if (l->x_what != xt_ptrto || l != r)
		goto incompat;
	}

	x->x_type = type_int;
	return x;

    case xt_colon:
	/*
	 *	Two halves of a question's colon.
	 *	Must be same type or pointer with NULL.
	 */
	if (l != r)
	{
	    /*
	     *	Check void* and pointer: void * rules.
	     *	Check pointer and NULL: pointer type rules.
	     */
	    if (r->x_what == xt_ptrto && isvoidp(l))
	    {
		x->x_right->x_type = r = l;
	    }
	    else if (l->x_what == xt_ptrto && isvoidp(r))
		x->x_left->x_type = l = r;
	    else if (r->x_what == xt_ptrto && null_ptr(x->x_left))
	    {
		x->x_left->x_type = r;
		l = r;
	    }
	    else if (l->x_what == xt_ptrto && null_ptr(x->x_right))
		x->x_right->x_type = l;
	    else
		goto incompat;
	}

	goto leftprop;

    default:
	Ea[0].i = (int)x->x_what;
	internal("fix_bin_type", "bad x_what");
	/* NOTREACHED */
    }

    /*
     *	Incompatible.
     */
incompat:
    Ea[0].m = xdope[(int)x->x_what].xd_name;
    Ea[1].x = x->x_left;
    Ea[2].x = x->x_right;
    error("operands of '%' are $ and $");
    return NULL;

    /*
     *	Propogate left type.
     */
leftprop:
    x->x_type = l;

    /*
     *	Add casts from implied conversions.
     */
addcasts:
    if (l != x->x_left->x_type)
	x->x_left = cast(x->x_left, l);

    /*
     *	Add right cast from implied conversions.
     */
rightcast:
    if (r != x->x_right->x_type)
	x->x_right = cast(x->x_right, r);

    return x;
}

/*
 *	Binary operator precedence parser.
 *
 *	We maintain a stack of factor, operator and precedence level
 *	in strictly increasing level.  For every new operator we pop
 *	off the items binding tighter than it, constructing a tree,
 *	and push the result.  At the end we pop off all items and
 *	return the resultant tree.  Note that this is non-recursive.
 */
xnode	*
bexpr()
{
    typedef struct
    {
	    xnode	*l_fact;
	    xnodes	l_what;
	    int l_level;
    }
	    explev;

    explev		lvl[10];

    register explev	*l;
    register xnode	*x;
    register xnode	*y;
    register xnodes	w;
    register token	*t;
    register int	lv;

    l = lvl - 1;
    loop
    {
	if ((x = factor()) == NULL)
	    return NULL;

	t = get_token();

	if (in(assop_starts, t->t_what))
	{
	    register token	*t2;

	    /*
	     *	Check for compound assignment operator.
	     */
	    if ((t2 = get_token())->t_what == tk_eq)
	    {
		unget_token(t2);
		lv = 0;
	    }
	    else
	    {
		unget_token(t2);
		lv = tkdopes[(int)t->t_what].prec_lev;
		w = xunpack(tkdopes[(int)t->t_what].equiv_xt);
	    }
	}
	else
	{
	    lv = tkdopes[(int)t->t_what].prec_lev;
	    w = xunpack(tkdopes[(int)t->t_what].equiv_xt);
	}

	while (l >= lvl && lv <= l->l_level)
	{
	    y = new_xnode();
	    y->x_what = l->l_what;
	    y->x_left = l->l_fact;
	    y->x_right = x;
	    y->x_flags = 0;

	    if ((x = fix_bin_type(y)) == NULL)
		return NULL;

	    l--;
	}

	if (lv <= 0)
	    break;

	l++;
	l->l_level = lv;
	l->l_what = w;
	l->l_fact = x;
	free_token(t);
    }
    unget_token(t);
    return x;
}

/*
 *	Parse a ``? :'' conditional expression.
 *
 *	It ends up like this:
 *
 *		?
 *	      /	  \
 *	    e1	    :
 *		  /   \
 *		e2     e3
 *
 *	Since comma and the assignment
 *	operators bind looser than the
 *	question/colon pair they can't
 *	mean anything in here.
 */
static xnode	*
conditional(p)
xnode	*p;
{
    register token	*t;
    register xnode	*e1;
    register xnode	*e2;
    register xnode	*e3;
    register xnode	*x;
    register xnode	*y;

    e1 = test_expr(p);

    if ((e2 = bexpr()) == NULL)
	return NULL;

    loop
    {
	switch ((t = get_token())->t_what)
	{
	case tk_question:
	    free_token(t);

	    if ((e2 = conditional(e2)) == NULL)
		return NULL;

	    continue;

	case tk_colon:
	    break;

	default:
	    if (in(expr_ops, t->t_what))
	    {
		if (in(assop_starts, t->t_what))
		{
		    register token	*t2;

		    if ((t2 = get_token())->t_what != tk_eq)
		    {
			unget_token(t2);
			errort(t, "missing ':'");
		    }
		    else
		    {
			free_token(t2);
			errortf(t, "% precedence confusion", "assignment operator");
		    }
		}
		else
		    errortf(t, "% precedence confusion", t->t_what == tk_comma ? "','" : "'='");
	    }
	    else
		errort(t, "missing ':'");

	    free_token(t);
	    return NULL;
	}

	break;
    }

    if ((e3 = bexpr()) == NULL)
	return NULL;

    /*
     *	Associates to the right.
     */
    while ((t = get_token())->t_what == tk_question)
    {
	free_token(t);

	if ((e3 = conditional(e3)) == NULL)
	    return NULL;
    }

    unget_token(t);

    y = new_xnode();
    y->x_what = xt_colon;
    y->x_left = e2;
    y->x_right = e3;
    y->x_flags = 0;

    x = new_xnode();
    x->x_what = xt_question;
    x->x_left = e1;

    if ((x->x_right = fix_bin_type(y)) == NULL)
	return NULL;

    x->x_type = x->x_right->x_type;
    x->x_flags = 0;
    return x;
}

/*
 *	Expression parser.  Handles assignment operators,
 *	question/colon, and comma, and calls 'bexpr' to handle
 *	other binary operators.
 *
 *	A little known (or perhaps little used) and little supported
 *	fact about C is that the complex assignment operators are
 *	two distinct tokens.  Thus i + = 10 is as good as i += 10.
 *
 *	Right associativity is handled by keeping a linked list up
 *	through the partially constructed tree.	 The 'x_type' field
 *	is used for this link.	Type checking is done as we undo the
 *	thread and climb to the top of the tree.
 *
 *	'thread' points to the first element of this list.
 *	'root' is what is currently considered to be the root of the tree.
 *	'parent' points to a pointer to the current subtree.
 *
 *	The 'comma' flag permits the comma operator (disallowed in lists).
 */
xnode	*
expr(comma)
int	comma;
{
    register token	*t;
    register xnode	*x;
    register xnode	*thread;
    register xnode	**parent;
    xnode		*root;

    thread = NULL;

    if ((root = bexpr()) == NULL)
	return NULL;

    parent = &root;
    t = get_token();

    while (in(expr_ops, t->t_what))
    {
	if (t->t_what == tk_comma)
	{
	    if (!comma)
		break;

	    /*
	     *	Make a 'comma' node with the
	     *	old tree as the left son and the
	     *	next factor (the new current tree)
	     *	as the right.
	     */
	    x = new_xnode();
	    x->x_type = thread;
	    thread = x;
	    x->x_what = xt_comma;
	    x->x_left = root;
	    x->x_flags = 0;

	    if ((x->x_right = bexpr()) == NULL)
		return NULL;

	    root = x;
	    parent = &x->x_right;
	    t = get_token();
	}
	else if (t->t_what == tk_question)
	{
	    free_token(t);

	    if ((*parent = conditional(*parent)) == NULL)
		return NULL;

	    t = get_token();
	}
	else
	{
	    if (in(assop_starts, t->t_what))
	    {
		register token	*t2;

		if ((t2 = get_token())->t_what != tk_eq)
		{
			unget_token(t2);
			break;
		}

		free_token(t2);

		x = new_xnode();

		switch (t->t_what)
		{
		case tk_and:
		    x->x_what = xt_andeq;
		    break;

		case tk_bar:
		    x->x_what = xt_bareq;
		    break;

		case tk_grtgrt:
		    x->x_what = xt_grgreq;
		    break;

		case tk_lessless:
		    x->x_what = xt_lslseq;
		    break;

		case tk_minus:
		    x->x_what = xt_minuseq;
		    break;

		case tk_percent:
		    x->x_what = xt_pereq;
		    break;

		case tk_plus:
		    x->x_what = xt_pluseq;
		    break;

		case tk_slash:
		    x->x_what = xt_slasheq;
		    break;

		case tk_times:
		    x->x_what = xt_timeseq;
		    break;

		case tk_uparrow:
		    x->x_what = xt_uparroweq;
		    break;

		default:
		    internal("expr", "bad switch on assop start");
		}
	    }
	    else
	    {
		x = new_xnode();
		x->x_what = xunpack(tkdopes[(int)t->t_what].equiv_xt);
	    }

	    /*
	     *	Make an appropriate node with the
	     *	old tree as the left son and the
	     *	next factor (the new current tree)
	     *	as the right.
	     */
	    x->x_flags = 0;
	    free_token(t);
	    x->x_left = *parent;

	    if ((x->x_right = bexpr()) == NULL)
		return NULL;

	    x->x_type = thread;
	    thread = x;
	    *parent = x;
	    parent = &x->x_right;
	    t = get_token();
	}
    }

    unget_token(t);

    /*
     *	Follow the thread up the tree, performing type checking
     *	and propogation.  Make it into a real tree.
     */
    while (thread != NULL)
    {
	x = thread;
	thread = x->x_type;

	if (x->x_what == xt_comma)
	    x->x_type = x->x_right->x_type;
	else
	{
	    if ((x->x_left->x_flags & XIS_LVAL) == 0)
	    {
		errorf("left operand of '%' is not a lvalue", xdope[(int)x->x_what].xd_name);
		return NULL;
	    }

	    x->x_left->x_flags |= XWAS_LVAL;

	    /*
	     *	This assumes that fix_bin_type cannot return
	     *	other than its argument for an assop.
	     */
	    if (fix_bin_type(x) == NULL)
		return NULL;
	}
    }

    return root;
}
