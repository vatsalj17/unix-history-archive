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
#include	"macdep.h"
#include	"xtypes.h"
#include	"parse.h"
#include	"stdobj.h"
#include	"io.h"
#include	"void.h"

extern xnode	*fix_bin_type();

extern tkset	local_decl_head_tokens;
extern tkdope	tkdopes[];

/*
 *	Promotion
 *	Check the type of x and insert an implicit type conversion
 *	if necessary.
 */
xnode	*
fix_basetype(x)
xnode	*x;
{
    register xnode	*y;

    y = x->x_type;
    if (y == type_float)
	y = type_double;
    else if (y == type_char || y == type_short)
	y = type_int;
    else if (y == type_uchar || y == type_ushort)
	y = type_uint;
    else if (y->x_what == xt_enum)
	y = type_int;
    else if (y->x_what == xt_bitfield)
	return extract_field(x);
    else
	return x;

    return cast(x, y);
}

/*
 *	x is of type xt_bitfield so add an extf node to extract
 *	the value.
 */
xnode	*
extract_field(x)
register xnode	*x;
{
    register xnode	*y;

    y = new_xnode();
    y->x_left = x;
    y->x_right = NULL;
    y->x_what = xt_extf;
    y->x_type = x->x_type->x_subtype;
    y->x_flags = 0;
    return y;
}

/*
 *	Check the compatibility of the unary operator at x and its
 *	operand, inserting implicit conversions if necessary.
 */
xnode	*
fix_un_type(x)
register xnode	*x;
{
    register xnode	*y;

    switch (x->x_what)
    {
    case xt_uand:
	/*
	 *	Address operator.  Check that the operand is
	 *	a l-value.  If it is a name check that is was
	 *	not declared register.  Finally the operand
	 *	cannot be of type bitfield.
	 */
	if (x->x_left->x_flags & XIS_LVAL)
	{
	    x->x_left->x_flags |= XWAS_LVAL;

	    if (x->x_left->x_what == xt_name)
	    {
		if (x->x_left->x_value.d->id_value.i & REGISTER)
		{
		    warnf("unary '&' of %, ignoring its class of register", x->x_left->x_value.d->id_name);
		    x->x_left->x_value.d->id_value.i &= ~REGISTER;
		}

		x->x_left->x_value.d->id_flags |= IDIS_UANDED;
	    }

	    if (x->x_left->x_type->x_what == xt_bitfield)
	    {
		Ea[0].x = x->x_left->x_type;
		error("operand of unary '&' is #");
		return NULL;
	    }

	    y = new_xnode();
	    y->x_what = xt_ptrto;
	    y->x_subtype = x->x_left->x_type;
	    x->x_type = find_type(y);
	    return x;
	}

	error("operand of unary '&' must be an object");
	return NULL;

    case xt_utimes:
	/*
	 *	Indirection.  Check that the operand is
	 *	of type pointer to ...
	 */
	if (x->x_left->x_type->x_what == xt_ptrto)
	{
	    x->x_type = x->x_left->x_type->x_subtype;
	    return x;
	}
	Ea[0].x = x->x_left;
	error("operand of unary '*' is $");
	return NULL;

    case xt_uminus:
	/*
	 *	Unary minus.  Must be basetype or bitfield.
	 *	((coming soon: Unary plus like this, allows ptr types too.))
	 */
	if
	(
	    (y = x->x_left->x_type)->x_what != xt_basetype
	    &&
	    y->x_what != xt_bitfield
	    &&
	    y->x_what != xt_enum
	)
	{
	    Ea[0].x = x->x_left;
	    error("operand of unary '-' is $");
	    return NULL;
	}
	x->x_left = fix_basetype(x->x_left);
	x->x_type = x->x_left->x_type;		/* reeds, was ... = y; */
	return x;

    case xt_not:
	/*
	 *	Unary negation.  Must be basetype, bitfield
	 *	or pointer (which means != NULL).
	 */
	if
	(
	    (y = x->x_left->x_type)->x_what != xt_basetype
	    &&
	    y->x_what != xt_bitfield
	    &&
	    y->x_what != xt_enum
	)
	{
	    if (y->x_what != xt_ptrto)
	    {
		Ea[0].x = x->x_left;
		error("operand of '?!' is $");
		return NULL;
	    }
	}
	else
	    x->x_left = fix_basetype(x->x_left);
	x->x_type = type_int;
	return x;

    case xt_tilda:
        /*
	 *	Complement.  Must be integral basetype or bitfield.
	 */
	if
	(
	    (
		(y = x->x_left->x_type)->x_what != xt_basetype
		||
		(y->x_value.i & (CHAR | INT)) == 0
	    )
	    &&
	    y->x_what != xt_bitfield
	    &&
	    y->x_what != xt_enum
	)
	{
	    Ea[0].x = x->x_left;
	    error("operand of '~' is $");
	    return NULL;
	}
	x->x_left = fix_basetype(x->x_left);
	x->x_type = x->x_left->x_type;		/* reeds, was ... = y; */
	return x;

    case xt_postinc:
    case xt_postdec:
    case xt_preinc:
    case xt_predec:
	/*
	 *	Increment/decrement.  First check for
	 *	pointer (in which case change the operator
	 *	to the equivalent pointer case), else it must
	 *	be an arithmetic type.  Then check for l-value.
	 */
	if ((y = x->x_left->x_type)->x_what == xt_ptrto)
	{
	    if(isvoidp(y)) {
		Ea[0].m = xdope[(int)x->x_what].xd_name;
		Ea[1].x = x->x_left;
		error("operand of '%' is $");
		return NULL;
	    }
	    
	    switch (x->x_what)
	    {
	    case xt_postinc:
		x->x_what = xt_ptrpostinc;
		break;

	    case xt_postdec:
		x->x_what = xt_ptrpostdec;
		break;

	    case xt_preinc:
		x->x_what = xt_ptrpreinc;
		break;

	    case xt_predec:
		x->x_what = xt_ptrpredec;
	    }
	    x->x_type = y;
	}
	else if (y->x_what == xt_enum)
	    x->x_type = type_int;
	else if (y->x_what != xt_basetype || (y->x_value.i & (CHAR | INT | FLOAT)) == 0)
	{
	    if (y->x_what == xt_bitfield)
		x->x_type = y->x_subtype;
	    else
	    {
		Ea[0].m = xdope[(int)x->x_what].xd_name;
		Ea[1].x = x->x_left;
		error("operand of '%' is $");
		return NULL;
	    }
	}
	else
	    x->x_type = y;

	if ((x->x_left->x_flags & XIS_LVAL) == 0)
	{
	    errorf("operand of '%' is not an object", xdope[(int)x->x_what].xd_name);
	    return NULL;
	}

	x->x_left->x_flags |= XWAS_LVAL;
	return x;

    default:
	Ea[0].i = (int)x->x_what;
	internal("fix_un_type", "bad switch in fix_un_type");
    }
    return 0;
    /* NOTREACHED */
}

/*
 *	Is a cast from type x to type y valid?
 *
 *	NO_PTR_CONV means that pointers and ints can't
 *	intersect (absurdly typed machine?).
 */
valid_cast(y, x)	/* reeds flip */
register xnode	*x;
register xnode	*y;
{
    if(isvoid(y))
	return 1;
    else if(isvoid(x))
	return 0;
    return
    (
	(
	    x->x_what == xt_basetype
	    ||
	    x->x_what == xt_bitfield
	    ||
	    x->x_what == xt_ptrto
	    ||
	    x->x_what == xt_enum
	)
	&&
	(
	    y->x_what == xt_basetype
	    ||
	    y->x_what == xt_ptrto
	    ||
	    y->x_what == xt_enum
	)
    );
}

/*
 *	Factor returns a tree in which all nodes obey the following rules:
 *	x_what		has some value
 *	x_type		has some value
 *	x_left		is NULL if unused
 *	x_right		is NULL if unused
 *	x_value.?	is optional
 *	x_flags		0, or X[IS|WAS]_LVAL or something given to us by expr
 *
 *	XIS_IMPLICIT is added to an implicit xt_uand added when
 *	a array or function name is seen naked.  This is so that
 *	sizeof can strip it away.
 *
 *	Factor will return NULL if an error occured.  In this case
 *	a message will have been generated.
 */
xnode	*
factor()
{
    register token	*t;
    register xnode	*x;
    register xnode	*y;
    register xnode	*z;
    extern void		outer_scope();

    switch ((t = get_token())->t_what)
    {
    case tk_cint:
	/*
	 *	Constant int.
	 */
	(x = new_xnode())->x_what = xt_cint;
	x->x_type = type_int;
	goto cint_factor;

    case tk_clong:
	/*
	 *	Constant long int.
	 */
	(x = new_xnode())->x_what = xt_clong;
	x->x_type = type_long;
    cint_factor:
	x->x_value.i = t->t_value.i;
	goto cnum_factor;

    case tk_string:
	/*
	 *	Literal character string.
	 */
	(x = new_xnode())->x_what = xt_string;
	x->x_type = type_chstr;
	x->x_value.s = t->t_value.s;
	free_token(t);
	x->x_stroff = 0;
	x->x_flags = 0;
	break;

    case tk_cdouble:
	/*
	 *	Constant floating point number.
	 */
	(x = new_xnode())->x_what = xt_cdouble;
	x->x_type = type_double;
	x->x_value.f = t->t_value.f;
    cnum_factor:
	free_token(t);
	x->x_flags = 0;
	break;

    case tk_onround:
	free_token(t);
	t = get_token();
	/*
	 *	Check for cast.
	 */
	if
	(
	    in(local_decl_head_tokens, t->t_what)
	    ||
	    (t->t_what == tk_name && is_typedefed(t))
	)
	{
	    /*
	     *	cast factor
	     */
	    unget_token(t);
	    (x = new_xnode())->x_what = xt_cast;
	    x->x_type = decl_cast("cast");

	    if ((t = get_token())->t_what != tk_offround)
	    {
		errort(t, "')' expected at logical end of cast");
		unget_token(t);
		return NULL;
	    }

	    if ((x->x_left = factor()) == NULL)
		return NULL;

	    if (!valid_cast(x->x_type, x->x_left->x_type))
	    {
		Ea[0].x = x->x_left;
		Ea[1].x = x->x_type;
		errort(t, "cast from $ to #");
	    }

	    free_token(t);
	    x->x_right = NULL;
	    x->x_flags = 0;
	    return x;
	}

	/*
	 *	( expr )
	 */
	unget_token(t);

	if ((x = expr(1)) == NULL)
	    return NULL;

	if ((t = get_token())->t_what != tk_offround)
	{
	    errort(t, "')' expected at logical end of expression");
	    unget_token(t);
	    return NULL;
	}
	else
	    free_token(t);
	break;

    case tk_sizeof:
	free_token(t);

	if ((t = get_token())->t_what == tk_onround)
	{
	    register token	*t2;

	    t2 = get_token();

	    if
	    (
		in(local_decl_head_tokens, t2->t_what)
		||
		(t2->t_what == tk_name && is_typedefed(t2))
	    )
	    {
		/*
		 *	sizeof (type)
		 */
		free_token(t);
		unget_token(t2);
		y = decl_cast("sizeof");

		if ((t = get_token())->t_what != tk_offround)
		{
		    errort(t, "')' expected at logical end of sizeof");
		    unget_token(t);
		    return NULL;
		}

		free_token(t);

		(x = new_xnode())->x_what = xt_cuint;
		x->x_value.i = size_of_in_bits(y) / BITS_PER_BYTE;
		x->x_type = type_uint;
		x->x_left = x->x_right = NULL;
		x->x_flags = 0;
		break;
	    }

	    unget_token(t2);
	}

	/*
	 *	sizeof expr
	 */
	unget_token(t);

	if ((y = factor()) == NULL)
	    return NULL;

	/*
	 *	Implicit conversions disappear.
	 */
	if ((y->x_flags & XIS_IMPLICIT) != 0)
	{
	    x = y->x_left;
	    free_xnode(y);
	    y = x;
	}

	(x = new_xnode())->x_what = xt_cuint;
	x->x_type = type_uint;
	x->x_value.i = size_of_in_bits(y->x_type) / BITS_PER_BYTE;
	x->x_left = x->x_right = NULL;
	x->x_flags = 0;
	return x;

    case tk_name:
    {
	register ident	*id;
	register token	*u;

	/*
	 *	name
	 */
	x = new_xnode();
	x->x_what = xt_name;

	if ((id = refind_ident(t->t_value.n, ID_FACTOR, (ident *)NULL)) == NULL)
	    return NULL;

	x->x_left = NULL;
	x->x_right = NULL;
	x->x_flags = XIS_LVAL;
	x->x_type = id->id_type;
	x->x_value.d = id;

	u = t;

	if ( (id->id_flags & IDIS_UNDEF) && levnum == 0)
	{
	    errorf("'%' undefined in expression", id->id_name);
	    free_token(u);
	    return NULL;
	}
	else if (x->x_type == NULL)
	{
	    if ((t = get_token())->t_what != tk_onround || levnum == 0)
	    {
		if ((id->id_flags & IDIS_UNDEF) == 0)
		{
		    id->id_flags |= IDIS_UNDEF;
		    errortf(t, "'%' undefined in expression", id->id_name);
		}

		free_token(u);
		unget_token(t);
		return NULL;
	    }
	    else
	    {
		/*
		 *	Call of undefined function.
		 *	Implicit declaration:
		 */
		x->x_type = type_ftnretint;
		id->id_type = type_ftnretint;
		id->id_value.i = EXTERN;
		outer_scope(id);

		{
		    extern long		var_index;

		    put(obj_code(i_var, v_implicit_function));
		    putnum( u->t_value.n->st_index );
		    putnum( type_ftnretint->x_index );
		    putnum( u->t_filename->st_index );
		    putnum( u->t_lineno );
		    id->id_index = var_index++;
		}

		free_token(u);

		if (id->id_token != NULL)
		    free_token(id->id_token);

		id->id_token = new_token();
		*id->id_token = *t;
		unget_token(t);
	    }
	}
	else
	{
	    free_token(u);

	    if ((t = get_token())->t_what != tk_onround && x->x_type->x_what == xt_ftnreturning)
	    {
		/*
		 *	Implicit xt_uand on function name.
		 */
		y = new_xnode();
		y->x_what = xt_uand;
		z = new_xnode();
		z->x_what = xt_ptrto;
		z->x_subtype = x->x_type;
		y->x_type = find_type(z);
		y->x_left = x;
		y->x_right = NULL;
		y->x_flags = XIS_IMPLICIT;
		x = y;
	    }

	    unget_token(t);
	}

	break;
    }

    case tk_minusminus:
    case tk_plusplus:
    case tk_and:
    case tk_minus:
    case tk_not:
    case tk_tilda:
    case tk_times:
	/*
	 *	Unary operators.
	 */
	(y = new_xnode())->x_what = xunpack(tkdopes[(int)t->t_what].equiv_xt);
	free_token(t);
	x = y;
	x->x_right = NULL;
	x->x_flags = 0;

	switch (x->x_what)
	{
	case xt_and:
	    x->x_what = xt_uand;
	    break;

	case xt_times:
	    x->x_what = xt_utimes;
	    x->x_flags = XIS_LVAL;
	    break;

	case xt_minus:
	    x->x_what = xt_uminus;
	}

	if ((x->x_left = factor()) == NULL)
	    return NULL;

	return fix_un_type(x);

    default:
	errort(t, "expression syntax error");
	unget_token(t);
	return NULL;
    }

    /*
     *	We now have prefix operators and the factor body.
     *	Now we loop looking for post ops, primary operators,
     *	indexing and function calls.
     */
    loop
    {
	if (x->x_type->x_what == xt_arrayof)
	{
	    /*
	     *	Implicit xt_uand of array references.
	     */
	    y = new_xnode();
	    y->x_what = xt_uand;
	    z = new_xnode();
	    z->x_what = xt_ptrto;
	    z->x_subtype = x->x_type->x_subtype;
	    y->x_type = find_type(z);
	    y->x_left = x;
	    y->x_right = NULL;
	    y->x_flags = XIS_IMPLICIT;
	    x = y;
	}

	switch((t = get_token())->t_what)
	{
	case tk_onsquare:
	    /*
	     *	expr [ expr ]
	     *  one of the expressions must be integer, other pointer.
	     *  permute them to make the first the pointer
	     */

	    free_token(t);
	    (y = new_xnode())->x_what = xt_ptrplus;
	    y->x_left = x;
	    y->x_flags = 0;

	    if ((y->x_right = expr(1)) == NULL)
		return NULL;
	    
	    if (
		!(
		    (x->x_type->x_what == xt_ptrto) ^
	            (y->x_right->x_type->x_what == xt_ptrto)
		)
	    )
	    {
		Ea[0].x = x;
		Ea[1].x = y->x_right;
		errort(t, "attempt to subscript $ with $");
		return NULL;
	    }

	    if (x->x_type->x_what != xt_ptrto) {
		y->x_left = y->x_right;
		y->x_right = x;
	    }

	    if (y->x_right->x_type->x_what == xt_bitfield)
		y->x_right = extract_field(y->x_right);
	    else if (y->x_right->x_type->x_what == xt_enum)
		y->x_right = cast(y->x_right, type_int);
	    else if
	    (
		y->x_right->x_type->x_what != xt_basetype
		||
		(y->x_right->x_type->x_value.i & (CHAR | INT)) == 0
	    )
	    {
		Ea[0].x = y->x_right;
		error("subscript expression is of type $");
		y->x_right->x_type = type_int;
	    }

	    x = fix_bin_type(y);
	    if (x == NULL)
		return NULL;

	    if ((t = get_token())->t_what != tk_offsquare)
	    {
		errort(t, "']' expected after subscript expression");
		return NULL;
	    }

	    free_token(t);
	    (y = new_xnode())->x_what = xt_utimes;
	    y->x_type = x->x_type->x_subtype;
	    y->x_left = x;
	    y->x_right = NULL;
	    y->x_flags = XIS_LVAL;

	    x = y;
	    break;

	case tk_onround:
	{
	    int		named_call;
	    long	line_save;
	    long	file_save;
	    /*
	     *	expr ( opt-par-list )
	     *
	     *	Function invokation.
	     */
	    if (x->x_type->x_what != xt_ftnreturning)
	    {
		Ea[0].x = x;
		errort(t, "attempt to invoke $ as a function");
		return NULL;
	    }

	    (y = new_xnode())->x_what = xt_call;
	    y->x_type = x->x_type->x_subtype;
	    y->x_what = xt_call;
	    y->x_flags = 0;
	    y->x_left = x;
	    y->x_right = NULL;

	    t = get_token();

	    if (x->x_what == xt_name)
	    {
		file_save = t->t_filename->st_index;
		line_save = t->t_lineno;
		named_call = 1;
	    }
	    else
		named_call = 0;

	    x = y;

	    if (t->t_what != tk_offround)
	    {
		register cfrag	*k;
		register cnode	*c;

		/*
		 *	Gather up the parameters.  They live on a cfrag
		 *	hanging off the x->x_value.k.
		 */
		unget_token(t);
		k = new_cfrag();
		c = new_cnode();
		k->c_head = c;
		c->c_what = ct_xnd;

		loop
		{
		    if ((y = expr(0)) == NULL)
			return NULL;

		    y = c->c_value.x = fix_basetype(y);
		    if(isvoid(y->x_type)) {
			error( "void function arg");
			return NULL;
		    }

		    if ((t = get_token())->t_what == tk_offround)
			break;

		    if (t->t_what != tk_comma)
		    {
			errort(t, "syntax error in parameter list of function call");
			return NULL;
		    }

		    free_token(t);
		    cadd(c, ct_xnd);
		}

		c->c_next = NULL;
		k->c_tail = c;
		x->x_value.k = k;

		free_token(t);
	    }
	    else
		x->x_value.k = NULL;

	    if (named_call)
	    {
		register cnode	*c;

		put(obj_code(i_var, v_call));
		putnum( x->x_left->x_value.d->id_index );
		putnum( file_save );
		putnum( line_save );

		if (x->x_value.k != NULL)
		{
		    for (c = x->x_value.k->c_head; c != NULL; c = c->c_next)
		    {
			if (c->c_value.x->x_what == xt_cast && c->c_value.x->x_left->x_type->x_what == xt_enum) {
			    putnum( c->c_value.x->x_left->x_type->x_index );
			} else {
			    putnum( c->c_value.x->x_type->x_index );
			}
		    }
		}

		putnum(0);
	    }

	    break;
	}

	case tk_plusplus:
	case tk_minusminus:
	    /*
	     *	expr ++ and expr --
	     */
	    (y = new_xnode())->x_what = t->t_what == tk_plusplus ? xt_postinc : xt_postdec;
	    y->x_left = x;
	    y->x_right = NULL;
	    y->x_flags = 0;
	    if ((x = fix_un_type(y)) == NULL)
		return NULL;
	    break;

	case tk_minusgrt:
	    /*
	     *	expr -> member
	     */
	    if
	    (
		x->x_type->x_what != xt_ptrto
		||
		(
		    (z = x->x_type->x_subtype)->x_what != xt_structof
		    &&
		    z->x_what != xt_unionof
		)
	    )
	    {
		Ea[0].x = x;
		errort(t, "left operand of '->' is $ (should be pointer to struct or union)");
		return NULL;
	    }

	    if (z->x_value.d->id_memblist == NULL)
	    {
		Ea[0].x = z;
		errort(t, "# not defined");
		return NULL;
	    }

	    free_token(t);

	    (y = new_xnode())->x_what = xt_utimes;
	    y->x_left = x;
	    y->x_right = NULL;
	    y->x_flags = 0;
	    y->x_type = x->x_type;

	    x = y;

	    (y = new_xnode())->x_what = xt_dot;
	    y->x_left = x;
	    y->x_right = NULL;
	    y->x_flags = XIS_LVAL;

	    x = y;

	    if ((t = get_token())->t_what != tk_name)
	    {
		Ea[0].x = z;
		errort(t, "member of # expected after '->'");
		unget_token(t);
		return NULL;
	    }
	    else
	    {
		register ident	*d;

		if ((d = refind_ident(t->t_value.n, IDIS_MOAGG, z->x_value.d)) == NULL)
		{
		    Ea[0].m = t->t_value.n->st_name;
		    Ea[1].x = z;
		    errort(t, "'%' is not a member of #");
		    return NULL;
		}

		x->x_type = d->id_type;
		x->x_value.i = d->id_value.i;

		free_token(t);
	    }
	    break;

	case tk_dot:
	    /*
	     *	expr . member
	     */
	    (y = new_xnode())->x_what = xt_dot;
	    y->x_left = x;
	    y->x_right = NULL;
	    y->x_flags = XIS_LVAL;

	    z = x->x_type;
	    x = y;

	    if
	    (
		z->x_what != xt_structof
		&&
		z->x_what != xt_unionof
	    )
	    {
		Ea[0].x = x->x_left;
		errort(t, "left operand of '.' is $ (should be struct or union)");
		return NULL;
	    }

	    if (z->x_value.d->id_memblist == NULL)
	    {
		Ea[0].x = z;
		errort(t, "# not defined");
		return NULL;
	    }

	    free_token(t);

	    if ((t = get_token())->t_what != tk_name)
	    {
		Ea[0].x = z;
		errort(t, "member of # expected after '.'");
		unget_token(t);
		return NULL;
	    }
	    else
	    {
		register ident	*d;

		if ((d = refind_ident(t->t_value.n, IDIS_MOAGG, z->x_value.d)) == NULL)
		{
		    Ea[0].m = t->t_value.n->st_name;
		    Ea[1].x = z;
		    errort(t, "'%' is not a member of #");
		    return NULL;
		}

		x->x_type = d->id_type;
		x->x_value.i = d->id_value.i;

		free_token(t);
	    }
	    break;

	default:
	    /*
	     *	expr
	     */
	    unget_token(t);
	    return x;
	}
    }
}
