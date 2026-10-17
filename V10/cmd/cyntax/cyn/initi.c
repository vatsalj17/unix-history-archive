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
#include	"cnodes.h"
#include	"macdep.h"
#include	"objects.h"
#include	"xtypes.h"
#include	"io.h"
#include	"stdobj.h"
#include	"options.h"

/*
 *	Initialisation routines.
 */

extern xnode	*init_expr();

extern tkset	base_anchors;

extern tkset	init_anchors;



/*
 *	An array which was previously declared dimensionless
 *	has got itself a size.
 */
static void
set_array_size(id, size, t)
ident	*id;
long	size;
token	*t;
{
    register xnode	*x;

    x = new_xnode();
    x->x_what = xt_arrayof;
    x->x_subtype = id->id_type->x_subtype;
    x->x_value.i = size;
    id->id_type = find_type(x);
    *id->id_token = *t;

    put(obj_code(i_var, v_array_size));
    putnum( id->id_index );
    putnum( id->id_type->x_index );
}

/*
 *	Initialise a variable (id, type).  Current token is passed.
 */
initialise(id, type, it)
register ident	*id;
register xnode	*type;
token		*it;
{
    register long	index;
    register xnode	*x;
    register token	*t;
    register ident	*ids;
    xnodes		want_type;
    int			got_curly;
    extern void		cstr_release();


    switch (type->x_what)
    {
    case xt_basetype:
	if ((type->x_value.i & (CHAR | INT)) != 0)
	    want_type = xt_clong;
	else
	    want_type = xt_cdouble;

	goto parse_init_expr;

    case xt_enum:
    case xt_bitfield:
	want_type = xt_clong;
	goto parse_init_expr;

    case xt_ptrto:
	want_type = xt_caddr;
	goto parse_init_expr;


    parse_init_expr:
	/*
	 *	We want "e" or "{ e }" or "{ e, }".
	 */
	if ((t = get_token())->t_what == tk_oncurly)
	{
	    free_token(t);
	    got_curly = 1;
	}
	else
	{
	    unget_token(t);
	    got_curly = 0;
	}

	if ((x = init_expr(want_type, base_anchors)) != NULL)
	{
	    register int	mismatch;

	    mismatch = 0;

	    if (want_type == xt_caddr)
	    {
		if (x->x_what == xt_caddr || x->x_what == xt_string)
		{
		    if (type != x->x_type)
			mismatch++;
		}
		else if (x->x_what == xt_cnull)
		    x->x_type = type;
		else
		    mismatch++;
	    }
	    else if (x->x_what == xt_caddr || x->x_what == xt_string)
	    {
		Ea[0].x = type;
		Ea[1].m = x->x_what == xt_caddr ? "constant address" : "string";
		error("attempt to initialise # with a %");
		goto skip_to_anchor;
	    }
	    else if (want_type != x->x_what)
		mismatch++;

	    if (mismatch)
	    {
		Ea[0].x = type;
		Ea[1].x = x;
		error("attempt to initialise # with $");
		goto skip_to_anchor;
	    }
	    else
	    {
		/*
		 *	Output data definition.
		 */
		switch (type->x_what)
		{
		case xt_enum:
		case xt_bitfield:
		    break;	
		case xt_basetype:
		    switch ((int)type->x_value.i & (int)(INT | LONG | SHORT | FLOAT | CHAR))
		    {
		    case CHAR:
		    case SHORT | INT:
		    case INT:
		    case INT | LONG:
		    case FLOAT:
		    case LONG | FLOAT:
			break;

		    default:
			internal("initialise", "bad basetype");
		    }

		    break;

		case xt_ptrto:

		    switch (x->x_what)
		    {
		    case xt_caddr:
		    case xt_cnull:
			break;

		    case xt_string:

			if (cstr_hiwater)
			    cstr_release();

			break;

		    default:
			internal("initialise", "bad ptr");
		    }

		    break;

		default:
		    internal("initialise", "bad type");
		}
	    }
	} 
	else
	    goto skip_to_anchor;

	if (got_curly)
	{
	    if ((t = get_token())->t_what == tk_comma)
		free_token(t);
	    else
		unget_token(t);
	    if ((t = get_token())->t_what != tk_offcurly)
	    {
		Ea[0].x = type; /* { */
		errort(t, "'}' expected after # initialisation");
		unget_token(t);
		goto skip_to_anchor;
	    }
	    else
		free_token(t);
	}

	return 1;

    case xt_structof:
	if ((ids = type->x_value.d->id_memblist) == NULL)
	{
	    Ea[0].x = type;
	    error("# has not been defined and so cannot be initialised");
	    goto skip_to_anchor;
	}

	if ((t = get_token())->t_what == tk_oncurly)
	{
	    free_token(t);
	    got_curly = 1;
	}
	else
	{
	    unget_token(t);
	    got_curly = 0;
	}

	loop
	{
	    if(ids->id_type->x_what == xt_bitfield && ids->id_type->x_value.i == 0) {
	        ids = ids->id_memblist;
	    }

	    if(!initialise(id, ids->id_type, it))
		goto skip_to_anchor;
	    ids = ids->id_memblist;

	    if ((t = get_token())->t_what == tk_comma)
	    {
		/*
		 * { {
		 * We look ahead and re-write ",}" as "}".
		 */
		register token	*u;

		if ((u = get_token())->t_what == tk_offcurly)
		{
		    free_token(t);
		    unget_token(u);
		}
		else
		{
		    unget_token(u);
		    unget_token(t);
		}
	    }
	    else
		unget_token(t);

	    if (ids == NULL)
	    {
		if (got_curly)
		{
		    if ((t = get_token())->t_what == tk_offcurly)
			free_token(t);
		    else
		    {
			Ea[0].x = type; /* { */
			errort(t, "matching '}' expected at logical end of initialisation of #");
			unget_token(t);
			goto skip_to_anchor;
		    }
		}

		break;
	    }

	    if ((t = get_token())->t_what == tk_offcurly)
	    {
		if (got_curly)
		    free_token(t);
		else
		    unget_token(t);

		break;
	    }

	    if (t->t_what == tk_comma)
		free_token(t);
	    else
	    {
		Ea[0].x = type;
		errort(t, "syntax error in initialisation of #");
		unget_token(t);
		goto skip_to_anchor;
	    }
	}

	return 1;

    case xt_arrayof:
	if (
		(type->x_subtype == type_char)
		|| (type->x_subtype == type_uchar)
	)
	{
	    if ((t = get_token())->t_what == tk_string)
	    {
		if (type->x_value.i < 0)
		    set_array_size(id, (long)t->t_value.s.str_size, it);
		else
		{
		    if (type->x_value.i < t->t_value.s.str_size)
		    {
			t->t_value.s.str_size--;

			if (type->x_value.i < t->t_value.s.str_size)
			    errort(t, "string is larger than character array it is initialising");
			else if (option(o_warnings))
			    warnt(t, "null truncated from string");
		    }
		}


		if (cstr_hiwater)
		    cstr_release();

		free_token(t);
		return 1;
	    }

	    unget_token(t);
	}

	if ((t = get_token())->t_what == tk_oncurly)
	{
	    free_token(t);
	    got_curly = 1;
	}
	else
	{
	    unget_token(t);
	    got_curly = 0;
	}

	index = 0;

	loop
	{
	    if(!initialise(id, type->x_subtype, it))
		goto skip_to_anchor;

	    index++;

	    if ((t = get_token())->t_what == tk_comma)
	    {
		register token	*u;

		/*
		 *	{ { We look ahead and re-write ",}" as "}".
		 */
		if ((u = get_token())->t_what == tk_offcurly)
		{
		    free_token(t);
		    unget_token(u);
		}
		else
		{
		    unget_token(u);
		    unget_token(t);
		}
	    }
	    else
		unget_token(t);

	    if (type->x_value.i >= 0 && index >= type->x_value.i)
	    {
		if (got_curly)
		{
		    if ((t = get_token())->t_what == tk_offcurly)
			free_token(t);
		    else
		    {
			Ea[0].i = type->x_value.i;
			Ea[1].x = type->x_subtype;

			/* {{ */
			if (type->x_value.i == 1)
			    errort(t, "matching '}' expected after initialisation of = #");
			else
			    errort(t, "matching '}' expected after initialisation of = #s");

			unget_token(t);
			goto skip_to_anchor;
		    }
		}

		break;
	    }

	    if ((t = get_token())->t_what == tk_offcurly)
	    {
		if (got_curly)
		    free_token(t);
		else
		    unget_token(t);


		break;
	    }

	    if (t->t_what == tk_comma)
		free_token(t);
	    else
	    {
		Ea[0].x = type;
		errort(t, "syntax error in initialisation of #");
		unget_token(t);
		goto skip_to_anchor;
	    }
	}

	if (type->x_value.i < 0)
	    set_array_size(id, index, it);

	return 1;

    default:
	Ea[0].x = type;
	error("#s cannot be initialised");
	return 0;
    }

skip_to_anchor:

    do
	t = get_token();
    while (!in(init_anchors, t->t_what));

    unget_token(t);
    return 0;

}

