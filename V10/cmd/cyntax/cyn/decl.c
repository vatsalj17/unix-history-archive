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
#include	"func.h"
#include	"xtypes.h"
#include	"parse.h"
#include	"stdobj.h"
#include	"io.h"
#include	"options.h"

extern long	var_index;

extern token	*name_token;
extern xnode	no_args;

/*
 *	Head tokens for a local declaration.  Also used as a
 *	restricted head token set for error recovery.  Doesn't take
 *	into account degenerate declarations as they have no
 *	meaning within a block and don't help much in error
 *	recovery.
 */
extern tkset	local_decl_head_tokens;
extern tkset	decl_head_tokens;
extern xtset	agg_types;

/*
 *	Modify an arguments type to conform to calling mechanism.
 */
static void
fix_arg_type(id)
register ident	*id;
{
    register xnode	*newtype;

    switch (id->id_type->x_what)
    {
    case xt_arrayof:
	newtype = new_xnode();
	newtype->x_what = xt_ptrto;
	newtype->x_value.i = 0;
	newtype->x_subtype = id->id_type->x_subtype;
	id->id_type = find_type(newtype);
	break;

    case xt_basetype:
	if ((id->id_type->x_value.i & (CHAR | SHORT)) != 0)
	{
	    if ((id->id_type->x_value.i & UNSIGNED) != 0)
		newtype = type_uint;
	    else
		newtype = type_int;

	    if (option(o_warnings))
	    {
		Ea[0].m = id->id_name;
		Ea[1].x = id->id_type;
		Ea[2].x = newtype;

		warn("argument '%' (#) modified to #");
	    }

	    id->id_type = newtype;
	}
	else if ((id->id_type->x_value.i & (LONG | FLOAT)) == FLOAT)
	{
	    if (option(o_warnings))
	    {
		Ea[0].m = id->id_name;
		Ea[1].x = id->id_type;
		Ea[2].x = type_double;

		warn("argument '%' (#) modified to #");
	    }

	    id->id_type = type_double;
	}

	break;

    case xt_ftnreturning:
	Ea[0].m = id->id_name;
	Ea[1].x = id->id_type;
	error("argument '%' is #");
    }
}

/*
 *	If an identifier has just been declared within a block
 *	remove it from the block's list of identifiers.
 */
void
outer_scope(id)
register ident	*id;
{
    if (levelp->lv_idents == id)
	levelp->lv_idents = id->id_levelp;

    id->id_levnum = 0;
}

/*
 *	Skip to a matching off curly.
 */
static token	*
skip_to_off_curly(oc)
token	*oc;
{
    register token	*t;

    loop
    {
	t = get_token();

	switch (t->t_what)
	{
	case tk_eof:
	    errort(oc, "unmatched '{'");
	    return t;

	case tk_offcurly:
	    free_token(oc);
	    return t;

	case tk_oncurly:
	    if ((t = skip_to_off_curly(t))->t_what == tk_eof)
		return t;
	}

	free_token(t);
    }
}

/*
 *	Declaration parser.
 *
 *	Anywhere but 'at_outer' this will return a member list of
 *	the things declared.  When 'where' is 'at_outer' this will
 *	loop gathering declarations and return NULL.
 */
ident	*
declaration(parent, where)
ident		*parent;
register places where;
{
    register token	*t;
    register long	class;
    register ident	*id;
    register ident	**m;
    ident		*memblist;
    ident		**next_arg;
    int			error_token;
    int			id_error;
    xnode		*shared_type;
    xnode		*formal_args;
    extern 		initialise();

    memblist = NULL;
    m = &memblist;
    error_token = 0;

    loop
    {
	/*
	 *	error_token -> still recovering from syntax error
	 */

	t = get_token();

	switch (t->t_what)
	{
	case tk_semicolon:
	    error_token = 0;

	    /*
	     *	Empty declarations.
	     */
	    do
	    {
		free_token(t);
		t = get_token();
	    }
	    while (t->t_what == tk_semicolon);

	    break;

	case tk_eof:
	    return memblist;
	}

	sync(t);

	if (where == at_outer)
	{
	    /*
	     *	External definition.
	     */
	    if (!in(decl_head_tokens, t->t_what))
	    {
		switch (t->t_what)
		{
		case tk_offcurly:
		    if (!error_token) /* {... */
			error("unexpected '}'");

		    break;

		case tk_eof:
		case tk_oncurly:
		    /*
		     *	Handled below.
		     */
		    break;

		default:
		    if (!error_token)
			error("external definition expected");
		}

		/*
		 *	Error recovery for external definition.
		 */
		do
		{
		    switch (t->t_what)
		    {
		    case tk_eof:
			return NULL;

		    case tk_oncurly:
			if ((t = skip_to_off_curly(t))->t_what == tk_eof)
			    return NULL;

			error("unexpected '{', skipped to matching '}'");
		    }

		    free_token(t);
		    t = get_token();
		}
		while (!in(local_decl_head_tokens, t->t_what));
	    }
	}
	else if
	(
	    !
	    (
		in(local_decl_head_tokens, t->t_what)
		||
		(t->t_what == tk_name && is_typedefed(t))
	    )
	)
	{
	    switch (where)
	    {
	    case in_struct:
	    case in_union:
		if (t->t_what == tk_offcurly)
		{
		    unget_token(t);
		    return memblist;
		}
		else if (!error_token)
		{
		    if (t->t_what == tk_name)
			errorf("unexpected identifier '%'", t->t_value.n->st_name);
		    else
		    {
			Ea[0].i = (int)where;
			error("syntax error in / declaration");
		    }
		}

		/*
		 *	Seek a semicolon (continue parsing elaboration),
		 *	an offcurly (unget and return), an oncurly
		 *	(skip) or eof.
		 */
		loop
		{
		    free_token(t);
		    t = get_token();

		    switch (t->t_what)
		    {
		    case tk_eof:
			return memblist;

		    case tk_semicolon:
			free_token(t);
			break;

		    case tk_oncurly:
			if ((t = skip_to_off_curly(t))->t_what == tk_eof)
			    return memblist;

			continue;

		    case tk_offcurly:
			unget_token(t);
			return memblist;

		    default:
			continue;
		    }

		    break;
		}

		error_token = 0;
		continue;

	    case in_arguments:
		if (t->t_what == tk_oncurly)
		{
		    unget_token(t);
		    return memblist;
		}
		else if (!error_token)
		{
		    if (t->t_what == tk_name)
			errorf("unexpected identifier '%'", t->t_value.n->st_name);
		    else
		    {
			Ea[0].i = (int)where;
			error("syntax error in / declaration");
		    }
		}

		/*
		 *	Seek a semicolon (continue parsing arguments),
		 *	an oncurly (unget and return) or eof.
		 */
		loop
		{
		    switch (t->t_what)
		    {
		    case tk_eof:
			return memblist;

		    case tk_semicolon:
			free_token(t);
			break;

		    case tk_oncurly:
			unget_token(t);
			return memblist;

		    default:
			free_token(t);
			t = get_token();
			continue;
		    }

		    break;
		}

		error_token = 0;
		continue;

	    default:
		/*
		 *	We are in_block or in_cast.
		 */
		unget_token(t);
		return memblist;
	    }
	}
	else if (where == in_block && t->t_what == tk_name)
	{
	    register token	*u;

	    /*
	     *	Catch "label:" at top of block where label is also typedefed.
	     */
	    if ((u = get_token())->t_what == tk_colon)
	    {
		unget_token(u);
		unget_token(t);
		return memblist;
	    }

	    unget_token(u);
	}

	unget_token(t);
	error_token = 0;

	/*
	 *	Gather type and storage class.
	 */
	{
	    register xnode	*x;

	    x = get_shared_type(where);
	    shared_type = x->x_subtype;
	    class = x->x_value.i;
	    free_xnode(x);
	}

	/*
	 *	Class vs context checks.
	 */
	switch ((int)class & (int)(TYPEDEF | AUTO | EXTERN | STATIC))
	{
	case TYPEDEF:
	    switch (where)
	    {
	    case in_arguments:
	    case in_struct:
	    case in_union:
		Ea[0].i = (int)where;
		Ea[1].m = where == in_arguments ? " declaration" : "";
		error("typedef in /%");
		class &= ~TYPEDEF;
	    }

	    break;

	case AUTO:
	    if (where != in_block)
	    {
		class &= ~AUTO;
		error("auto not in block");
	    }

	    break;

	case EXTERN:
	    if (where != at_outer && where != in_block)
	    {
		class &= ~EXTERN;
		errorf("%s may only be declared globally or locally", "extern");
	    }

	    break;

	case STATIC:
	    if (where == in_block)
	    {
		class &= ~STATIC;
		class |= BLOCKSTATIC;
	    }
	    else if (where != at_outer)
	    {
		class &= ~STATIC;
		errorf("%s may only be declared globally or locally", "static");
	    }

	    break;

	case 0:
	    break;

	default:
	    internal("declaration", "bad storage class");
	}

	if ((class & REGISTER) != 0 && where != in_block && where != in_arguments)
	{
	    error("register can only be applied to arguments or autos");
	    class &= ~REGISTER;
	}

	if (where == in_arguments)
	    class |= ARGUMENT;

	/*
	 *	class contains:
	 *
	 *	TYPEDEF, EXTERN, STATIC, BLOCKSTATIC,
	 *	ARGUMENT ( incl. REGISTER), AUTO ( incl.  REGISTER),
	 */

	/*
	 *	id_error -> current id is in error, don't add to list etc.
	 */
	id_error = 0;

	/*
	 *	Gather items and take action.
	 */
	loop
	{
	    register xnode	*item;
	    register int	iclass;

	    id = NULL;
	    formal_args = &no_args;
	    item = decl_item(shared_type);

	    /*
	     *	Default storage classes.
	     */
	    switch (where)
	    {
	    case in_block:
		if (class == TYPEDEF)
		    iclass = class;
		else if (item->x_left->x_what == xt_ftnreturning)
		{
		    if (class != EXTERN && class != 0)
			error("functions declared within blocks must be extern");

		    iclass = EXTERN;
		}
		else if ((class & (BLOCKSTATIC | EXTERN)) == 0)
		    iclass = AUTO;
		else
		    iclass = class;

		break;

	    case at_outer:
		if ((class & (EXTERN | TYPEDEF | STATIC)) == 0)
		{
		    if (!option(o_restricted) || item->x_left->x_what == xt_ftnreturning)
			iclass = class | EXTERN;
		    else
			iclass = class | GLOBAL;
		}
		else
		    iclass = class;

		break;

	    case in_struct:
	    case in_union:
		if (item->x_left->x_what == xt_ftnreturning)
		{
		    Ea[0].m = in_struct ? "struct" : "union";
		    Ea[1].x = item->x_left;

		    error("%s cannot contain #s");
		}

		iclass = class;
		break;

	    default:
		iclass = class;
	    }

	    /*
	     *	Dimensionless arrays can only be at outer or extern.
	     *	Be careful of BLOCKSTATIC with implicit dimension.
	     */
	    if
	    (
		item->x_left->x_what == xt_arrayof
		&&
		item->x_left->x_value.i < 0
		&&
		(iclass & EXTERN) == 0
		&&
		where != at_outer
		&&
		where != in_arguments
	    )
	    {
		t = get_token();

		if ((iclass & BLOCKSTATIC) == 0 || t->t_what != tk_eq)
			errort(t, "dimensionless arrays must be extern or file static");

		unget_token(t);
	    }

	    /*
	     *	Bitfields only allowed in structs.
	     */
	    if (where != in_struct && item->x_left->x_what == xt_bitfield)
	    {
		error("bit field not in struct");
		item->x_subtype = type_int;
	    }

	    /*
	     *	We only really have a declaration if we have a name.
	     */
	    if (item->x_value.n != NULL)
	    {
		register long	id_flags;

		switch (where)
		{
		case at_outer:
		case in_block:
		    if (iclass == TYPEDEF)
			id_flags = IDIS_TYPENAME;
		    else
			id_flags = IDIS_NAME;

		    break;

		case in_arguments:
		    id_flags = IDIS_NAME | IDIS_ARGUMENT;
		    break;

		case in_struct:
		case in_union:
		    id_flags = IDIS_MOAGG;
		    break;

		default:
		    internal("declaration", "unknown where");
		}

		/*
		 *	Enter identifier in symbol table.  Returns NULL on
		 *	inconsistent declaration after giving diagnostic.
		 */
		if ((id = find_ident(item->x_value.n, id_flags, parent)) == NULL)
		    goto end_decl_item;

		/*
		 *	No void declarations.
		 */
		if (item->x_subtype == type_void && iclass != TYPEDEF)
		{
		    errorf("declaration of void (%)", item->x_value.n->st_name);
		    id_error = 1;
		    goto end_decl_item;
		}

		/*
		 *	Tests for redeclaration.
		 */

		if ((where == in_arguments || where == in_struct || where == in_union) && id->id_type != NULL)
		{
		    /*
		     *	No dups allowed in args or aggs.
		     */
		    Ea[0].m = id->id_name;
		    Ea[1].i = (int)where;
		    Ea[2].m = where == in_arguments ? " declaration" : "";
		    error("'%' already appears in this /%");
		    id_error = 1;
		    goto end_decl_item;
		}

		/*
		 *	Pick up formal arguments.
		 */
		if (id->id_type != NULL && id->id_type->x_what == xt_ftnreturning)
			formal_args = item->x_right;

		/*
		 *	Only one storage class allowed.
		 */
		if
		(
		    ((id->id_value.i & EXTERN) != 0 && (iclass & (BLOCKSTATIC | STATIC)) != 0)
		    ||
		    ((iclass & (BLOCKSTATIC | EXTERN)) != 0 && (id->id_value.i & (AUTO | BLOCKSTATIC | STATIC)) != 0)
		)
		{
		    Ea[0].m = id->id_name;
		    Ea[2].t = id->id_token;

		    switch ((int)id->id_value.i & (int)(AUTO | BLOCKSTATIC | EXTERN | STATIC))
		    {
		    case AUTO:
			Ea[1].m = "auto";
			break;

		    case EXTERN:
			Ea[1].m = "extern";
			break;

		    case BLOCKSTATIC:
		    case STATIC:
			Ea[1].m = "static";
			break;

		    default:
			internal("declaration", "bad storage class");
		    }

		    error("'%' already declared with storage class %@");
		    id_error = 1;
		    goto end_decl_item;
		}

		/*
		 *	This differs from the previous diagnostic only
		 *	in the amount of information we can give away.
		 */
		if
		(
		    (id->id_value.i & ID_SCOPES) != 0
		    &&
		    (iclass & ID_SCOPES) != 0
		    &&
		    (id->id_value.i & iclass & STATIC) == 0
		)
		{
		    Ea[0].m = id->id_name;
		    Ea[1].t = id->id_token;
		    error("'%' already declared@");
		    id_error = 1;
		    goto end_decl_item;
		}

		/*
		 *	Move extern to outer scope.
		 */
		if (where == in_block && (iclass & EXTERN) != 0)
		    outer_scope(id);

		/*
		 *	Remember token.
		 */
		if (id->id_token == NULL)
		{
		    id->id_token = name_token;
		    name_token = NULL;
		}

		if (id->id_type == NULL)
		{
		    /*
		     *	New definition.
		     */
		    id->id_type = item->x_left;

		    if (where == in_struct || where == in_union)
			id->id_index = item->x_value.n->st_index;

		    if (!option(o_restricted) || (id->id_value.i & GLOBAL) == 0)
			id->id_value.i = iclass;

		    if (id->id_type->x_what == xt_ftnreturning)
			formal_args = item->x_right;

		    if (where == in_arguments)
			fix_arg_type(id);

		    /*
		     *	Remember the type name given to an otherwise
		     *	unnamed aggregate for diagnostics.
		     */
		    if
		    (
			id->id_value.i == TYPEDEF
			&&
			in(agg_types, id->id_type->x_what)
			&&
			(id->id_type->x_value.d->id_flags & IDIS_FAKE) != 0
			&&
			id->id_type->x_value.d->id_name[0] == '\0'
		    )
			id->id_type->x_value.d->id_name = id->id_name;
		}
		else if (id->id_type == item->x_left)
		{
		    /*
		     *	Redeclaration as the same.
		     */
		    id->id_type = item->x_left;

		    if (!option(o_restricted) || (id->id_value.i & GLOBAL) == 0)
			id->id_value.i = iclass;

		    if (id->id_type->x_what == xt_ftnreturning)
			formal_args = item->x_right;
		}
		else if
		(
		    id->id_type->x_what == xt_arrayof
		    &&
		    item->x_left->x_what == xt_arrayof
		    &&
		    id->id_type->x_subtype == item->x_left->x_subtype
		)
		{
		    /*
		     *	Arrays of the same gender but differing 'type'.
		     */
		    if (id->id_type->x_value.i < 0)
		    {
			/*
			 *	Definition of previously unknown array dimension.
			 */
			put(obj_code(i_var, v_array_size));
			putnum( id->id_index );
			putnum( item->x_left->x_index );
			id->id_type = item->x_left;

			if (name_token != NULL)
			{
			    id->id_token = name_token;
			    name_token = NULL;
			}
		    }
		    else if (item->x_left->x_value.i > 0)
		    {
			/*
			 *	Conflict of sizes.
			 */
			Ea[0].x = id->id_type;
			Ea[1].m = id->id_name;
			Ea[2].t = id->id_token;
			Ea[3].x = item->x_left;
			error("! '%'@ redefined as !");
			id->id_type = item->x_left;
			id_error = 1;
		    }
		}
		else
		{
		    /*
		     *	Type clash on redeclaration.
		     */
		    Ea[0].x = id->id_type;
		    Ea[1].m = id->id_name;
		    Ea[2].t = id->id_token;
		    Ea[3].x = item->x_left;
		    error("! '%'@ redefined as !");
		    id->id_type = item->x_left;
		    id_error = 1;
		}
	    }
	    else if (where == in_struct && item->x_left->x_what == xt_bitfield)
	    {
		/*
		 *	Unnamed bitfield.
		 */
		id = talloc(ident);
		id->id_type = item->x_left;
		id->id_flags = IDIS_FAKE;
		id->id_memblist = NULL;
		t = get_token();
		id->id_token = new_token();
		*id->id_token = *t;
		unget_token(t);
		*m = id;
		m = &id->id_memblist;
		id = NULL;
	    }

	end_decl_item:

	    free_xnode(item);

	    if (id != NULL)
	    {
		/*
		 *	Perhaps tack this ident onto our list.
		 */
		if
		(
		    where != at_outer
		    &&
		    (id->id_value.i & EXTERN) == 0
		    &&
		    (id->id_flags & IDIS_TYPENAME) == 0
		    &&
		    !id_error
		)
		{
		    *m = id;
		    m = &id->id_memblist;
		}

		/*
		 *	Construct symbol table entry for new declaration.
		 */
		if (id->id_index == 0 && !id_error)
		{
		    if (where == at_outer)
		    {
			switch ((int)id->id_value.i & (int)(EXTERN | GLOBAL | STATIC | TYPEDEF))
			{
			case EXTERN:
			    if (option(o_restricted))
				break;

			case GLOBAL:
			    put(obj_code(i_var, v_global));
			    goto put_var;

			case STATIC:
			    put(obj_code(i_var, v_static));
			    goto put_var;

			case TYPEDEF:
			    break;

			default:
			    internal("declaration", "bad outer storage");
			}
		    }
		    else if (where == in_block)
		    {
			switch ((int)id->id_value.i & (int)(AUTO | BLOCKSTATIC | EXTERN | GLOBAL | TYPEDEF))
			{
			case AUTO:
			    put(obj_code(i_var, v_auto));
			    goto put_var;

			case BLOCKSTATIC:
			    put(obj_code(i_var, v_block_static));
			    goto put_var;

			case EXTERN:
			    if (option(o_restricted))
				break;

			case GLOBAL:
			    put(obj_code(i_var, v_global));
			    goto put_var;

			case TYPEDEF:
			    break;

			default:
			    internal("declaration", "bad block storage");
			}
		    }

		    goto skip_put;

		put_var:

		    putnum( id->id_token->t_value.n->st_index );
		    putnum( id->id_type->x_index );
		    putnum( id->id_token->t_filename->st_index );
		    putnum( id->id_token->t_lineno );
		    id->id_index = var_index++;

		skip_put:
		    ;
		}

		/*
		 *	Continue parsing list.
		 */
		switch ((t = get_token())->t_what)
		{
		case tk_comma:
		case tk_semicolon:
		    if (id->id_type->x_what == xt_ftnreturning)
		    {
			/*
			 *	Functions without bodies are forward
			 *	declarations and don't have argument lists.
			 */
			if (formal_args != NULL && formal_args != &no_args)
			    errort(t, "unexpected function argument list");
		    }
		    else if ((id->id_value.i & EXTERN) != 0)
		    {
			if
			(
			    (
				id->id_type->x_what == xt_arrayof
				&&
				id->id_type->x_value.i < 0
			    )
			    ||
			    option(o_restricted)
			)
			    /* extern_id(id) */;
			else
			{
			    /* label_id(id);
			    reserve_space(size_of(id->id_type)); */
			}
		    }
		    else if ((id->id_value.i & (BLOCKSTATIC | GLOBAL | STATIC)) != 0)
		    {
			/* label_id(id);
			reserve_space(size_of(id->id_type)); */
		    }

		    unget_token(t);
		    break;

		case tk_eq:
		    if (option(o_restricted) && (id->id_value.i & EXTERN) != 0)
			errorf("cannot initialise extern '%'", id->id_name);
		    else if ((id->id_flags & IDIS_INIT) != 0)
			errorf("'%' has already been initialised", id->id_name);
		    else if (id->id_value.i == TYPEDEF)
			errorf("typename '%' cannot be initialised", id->id_name);

		    id->id_flags |= IDIS_INIT;

		    switch (where)
		    {
		    case in_block:
			if ((id->id_value.i & BLOCKSTATIC) != 0)
			{
				/*
				 *	label_block_static(id)
				 */
				goto do_initialise;
			}
			else
			{
			    xnode	*x;

			    switch (id->id_type->x_what)
			    {
			    case xt_ptrto:
			    case xt_basetype:
			    case xt_enum:
				break;

			    default:
				Ea[0].x = id->id_type;
				Ea[1].m = id->id_name;
				errort(t, "cannot initialise auto # '%'");
			    }

			    if ((x = expr(0)) != NULL)
				auto_initialise(id, x);
			}

			break;

		    case at_outer:
			/*
			 *	label_global(id)
			 */
		    do_initialise:
			{
			    put(obj_code(i_data, 0));
			    putnum( id->id_index );
			    putnum( t->t_filename->st_index );
			    putnum( t->t_lineno );
			    put(obj_code(d_end, 0));
			}
			(void)initialise(id, id->id_type, t); 
			free_token(t);
			break;

		    default:
			Ea[0].m = id->id_name;
			Ea[1].i = (int)where;
			Ea[2].m = where == in_arguments ? " declaration" : "";
			errort(t, "cannot initialise '%' in /%");
			free_token(t);

			if ((t = get_token())->t_what == tk_oncurly)
			{
			    if ((t = skip_to_off_curly(t))->t_what == tk_eof)
				return memblist;

			    free_token(t);
			}
			else
			{
			    unget_token(t);
			    (void)expr(0);
			}
		    }

		    break;

		default:
		    unget_token(t);
		}
	    }

	    /*
	     *	End of declarator and perhaps initialisation.
	     */
	    t = get_token();

	    if (t->t_what == tk_comma)
	    {
		free_token(t);
		continue;
	    }

	    if (t->t_what == tk_semicolon)
	    {
		free_token(t);
		break;
	    }

	    if (id != NULL && id->id_type->x_what == xt_ftnreturning)
	    {
		register ident	*ids;
		register xnode	*y;

		if (formal_args == &no_args)
		{
			formal_args = NULL;

			errorf("function '%' has no argument list", id->id_name);
		}

		/*
		 *	Function body.
		 */
		unget_token(t);

		if (where != at_outer)
		{
		    Ea[0].i = (int)where;
		    Ea[1].m = where == in_arguments ? " declaration" : "";
		    error("function body in /%");
		}
		else if ((id->id_flags & IDIS_INIT) != 0)
		    errorf("function body for '%' has already been defined", id->id_name);
		else if ((id->id_flags & IDIS_INTRIN) != 0)
		    warnf("intrinsic version of function '%' has already been used", id->id_name);

		id->id_flags |= IDIS_INIT;

		if (name_token != NULL)
		{
		    free_token(id->id_token);
		    id->id_token = name_token;
		}

		if (id->id_value.i == TYPEDEF)
		    errorf("function body for typedef '%'", id->id_name);

		if (option(o_restricted))
		{
		    id->id_value.i |= GLOBAL;
		    id->id_value.i &= ~EXTERN;
		}

		/*
		 *	Arguments.
		 */
		for (ids = declaration(id, in_arguments); ids != NULL; ids = ids->id_memblist)
		{
		    for
		    (
			y = formal_args;
			y != NULL && y->x_value.n->st_name != ids->id_name;
			y = y->x_left
		    )
			;

		    if (y == NULL)
			errorf("'%' is declared as a parameter but is not in formal parameter list", ids->id_name);
		    else
		    {
			y->x_what = xt_argident;
			y->x_value.d = ids;
		    }
		}

		next_arg = &func.fn_args;

		/*
		 *	Start the local variable list with the arguments,
		 *	making any undeclared ones int as we go.
		 */
		{
		    register xnode	*x;
		    put(obj_code(i_var, v_arglist));
		    putnum( id->id_index );
		    putnum( id->id_token->t_filename->st_index );
		    putnum( id->id_token->t_lineno );
		    t = get_token();

		    for (y = formal_args; y != NULL; y = x)
		    {
			if (y->x_what == xt_argname)
			{
			    putnum( y->x_value.n->st_index );
			    y->x_value.d = find_ident(y->x_value.n, IDIS_NAME | IDIS_ARGUMENT, id);
			    y->x_value.d->id_value.i = ARGUMENT;

			    if (y->x_value.d->id_type != NULL)
			    {
				Ea[0].m = y->x_value.d->id_name;
				error("formal parameter '%' multiply declared");
			    }
			    else
			    {
				y->x_value.d->id_type = type_int;
				y->x_value.d->id_token = new_token();
				*y->x_value.d->id_token = *t;
			    }
			}
			else
			    putnum( y->x_value.d->id_token->t_value.n->st_index );

			putnum( y->x_value.d->id_type->x_index );
			putnum( y->x_value.d->id_token->t_filename->st_index );
			putnum( y->x_value.d->id_token->t_lineno );
			y->x_value.d->id_index = var_index++;
			*next_arg = y->x_value.d;
			x = y->x_left;
			next_arg = &y->x_value.d->id_memblist;
			free_xnode(y);
		    }
		}

		putnum(0);

		*next_arg = NULL;

		if (t->t_what != tk_oncurly)
		{
		    errort(t, "'{' expected to start function body");
		    unget_token(t);
		    /* recovery } */
		}
		else
		    function(id, t);

		remove_ids(func.fn_args);
	    }
	    else
	    {
		/*
		 *	Error in declaration.  Try for missing semicolon.
		 */
		if (in(local_decl_head_tokens, t->t_what) || (t->t_what == tk_name && is_typedefed(t)))
		    errort(t, "missing ';' in declaration");
		else
		    errort(t, "',' or ';' expected at logical end of declaration");

		error_token = 1;
		unget_token(t);
	    }

	    break;
	}
    }
}

/*
 *	A declaration for a cast or typedef.  No name or storage class.
 */
xnode	*
decl_cast(purpose)
char	*purpose;
{
    register xnode	*x;
    register xnode	*y;

    x = get_shared_type(in_cast);

    if (x->x_value.i != 0)
    {
	if (x->x_value.i == TYPEDEF)
	    Ea[0].m = "typedef";
	else
	    Ea[0].m = "storage class";

	Ea[1].m = purpose;

	error("% in %");
    }

    if ((y = decl_item(x->x_left))->x_value.n != NULL)
    {
	Ea[0].m = y->x_value.n->st_name;
	Ea[1].m = purpose;

	error("identifier '%' in %");
    }

    free_xnode(x);
    x = y->x_left;
    free_xnode(y);

    return x;
}
