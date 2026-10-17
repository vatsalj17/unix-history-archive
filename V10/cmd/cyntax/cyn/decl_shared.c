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
#include	"macdep.h"
#include	"io.h"
#include	"stdobj.h"

extern char	*where_name[];
extern int	comp_constant();
extern long	roundup();
struct
{
    long	if_got_this;
    long	dont_have_these;
    char	*my_name_is;
}
    static incompatibles[] =
{
    {
	AUTO,
	EXTERN | REGISTER | STATIC | TYPEDEF,
	"auto"
    },
    {
	EXTERN,
	AUTO | REGISTER | STATIC | TYPEDEF,
	"extern"
    },
    {
	REGISTER,
	AUTO | EXTERN | STATIC | TYPEDEF,
	"register"
    },
    {
	STATIC,
	AUTO | EXTERN | REGISTER | TYPEDEF,
	"static"
    },
    {
	TYPEDEF,
	AUTO | EXTERN | REGISTER | STATIC,
	"typedef"
    },
    {
	LONG,
	SHORT | CHAR | ENUM | STRUCT | UNION | VOID,
	"long"
    },
    {
	SHORT,
	LONG | CHAR | ENUM | FLOAT | STRUCT | UNION | VOID,
	"short"
    },
    {
	UNSIGNED,
	ENUM | FLOAT | STRUCT | UNION | VOID,
	"unsigned"
    },
    {
	CHAR,
	LONG | SHORT | ENUM | FLOAT | INT | STRUCT | UNION | VOID,
	"char"
    },
    {
	ENUM,
	LONG | SHORT | UNSIGNED | CHAR | FLOAT | INT | STRUCT | UNION | VOID,
	"enum"
    },
    {
	FLOAT,
	SHORT | UNSIGNED | CHAR | ENUM | INT | STRUCT | UNION | VOID,
	"float"
    },
    {
	INT,
	CHAR | ENUM | FLOAT | STRUCT | UNION | VOID,
	"int"
    },
    {
	STRUCT,
	LONG | SHORT | UNSIGNED | CHAR | ENUM | FLOAT | INT | UNION | VOID,
	"struct"
    },
    {
	UNION,
	LONG | SHORT | UNSIGNED | CHAR | ENUM | FLOAT | INT | STRUCT | VOID,
	"union"
    },
    {
	VOID,
	LONG | SHORT | UNSIGNED | CHAR | ENUM | FLOAT | INT | STRUCT | UNION,
	"void"
    },
};

ident	*
is_typedefed(t)
token	*t;
{
    register ident	*id;

    for
    (
	id = t->t_value.n->st_idlist;
	id != NULL && (id->id_flags & IDIS_TYPENAME) == 0;
	id = id->id_next
    )
    {
	if ((id->id_flags & ID_NAMES) != 0)
	    return NULL;
    }

    return id;
}

/*
 *	get_shared_type has just seen 'enum' and perhaps even
 *	the '{' of an enum definition.
 }
 */
def_enum(my_type, enum_token, index, defn)
register xnode	*my_type;
token		*enum_token;
long		index;
int		defn;
{
    register token	*t;
    register ident	*id;
    register long	from;
    register long	to;
    int			first;
    long		value;

    extern long		type_index;

    extern tkset	enum_anchors;

    id = my_type->x_value.d;

    from = MAXOF_INT;
    to = MINOF_INT;
    value = -1;
    first = 1;

    if (my_type->x_index != 0)
    {
	if (!defn)
	    return;

	put(obj_code(i_type, t_elaboration));
	putnum( my_type->x_index );
	putnum( enum_token->t_filename->st_index );
	putnum( enum_token->t_lineno );
	put(obj_code(i_type, t_enum));
    }
    else
    {
	my_type->x_index = type_index++;
	put(obj_code(i_type, t_enum));
	putnum( index );
	putnum( enum_token->t_filename->st_index );
	putnum( enum_token->t_lineno );
    }

    if (!defn)
    {
	putnum(0);
	return;
    }

    if (id != NULL)
    {
	if ((id->id_flags & IDIS_INIT) != 0)
	    errorf("enum '%' has already been defined", id->id_name);
	else
	    id->id_flags |= IDIS_INIT;
    }

    loop
    {
	switch ((t = get_token())->t_what)
	{
	case tk_name:
	    {
		register ident	*d;

		putnum( t->t_value.n->st_index );

		d = find_ident(t->t_value.n, IDIS_MOENUM, (ident *)NULL);

		free_token(t);

		if ((t = get_token())->t_what == tk_eq)
		{
		    free_token(t);
		    (void)comp_constant(enum_anchors, &value);
		}
		else
		{
		    unget_token(t);
		    value++;
		}

		if (d != NULL)
		{
		    d->id_type = my_type;
		    d->id_value.i = value;
		}
	    }

	    putnum(value);

	    if (value < from)
		from = value;

	    if (value > to)
		to = value;

	    t = get_token();
	    first = 0;
	    break;

	case tk_offcurly:
	    if (!first)
		break;

	default:
	    first = 0;
	    errort(t, "identifier expected in enum declaration");

	    while (!in(enum_anchors, t->t_what))
	    {
		free_token(t);
		t = get_token();
	    }
	}

	switch (t->t_what)
	{
	case tk_offcurly:
	    break;

	default:						/* { */
	    errort(t, "',' or '}' expected in enum declaration");

	    while (!in(enum_anchors, t->t_what))
	    {
		free_token(t);
		t = get_token();
	    }

	    if (t->t_what != tk_comma)
		break;

	case tk_comma:
	    free_token(t);

	    if ((t = get_token())->t_what == tk_offcurly)
		break;
	    else
		unget_token(t);

	    continue;
	}

	break;
    }

    if (t->t_what == tk_offcurly)
	free_token(t);
    else
	unget_token(t);

    if (id != NULL)
    {
	id->id_value.r.from = from;
	id->id_value.r.to = to;
    }

    putnum(0);
    putnum(from);
    putnum(to);
}

/* w/o code generation alignments bogus */
#define alignment_in_bits(type)		BITS_PER_BYTE

/*
 *	get_shared_type has just seen 'struct' or 'union' and
 *	perhaps the '{' of a struct or union definition.
 */
def_struct_union(my_type, agg_token, index, defn)
register xnode	*my_type;
token		*agg_token;
long		index;
int		defn;
{
    register ident	*id;
    register ident	*ids;
    register long	offset;
    register long	size;
    register int	max_alignment;
    register places	where;

    extern long		type_index;

    if (my_type->x_what == xt_structof)
	where = in_struct;
    else
	where = in_union;

    if (my_type->x_index == 0)
    {
	my_type->x_index = type_index++;

	if (where == in_struct) {
	    put(obj_code(i_type, t_structof));
	}else {
	    put(obj_code(i_type, t_unionof));
	}

	putnum( index );
	putnum( agg_token->t_filename->st_index );
	putnum( agg_token->t_lineno );
    }

    if (!defn)
 	return;

    id = my_type->x_value.d;

    if (id == NULL) {
	id = talloc(ident);
    }
    else if (id->id_memblist != NULL)
    {
	Ea[0].i = (int)where;
	Ea[1].m = id->id_name;
	error("/ '%' has already been defined");
    }

    id->id_memblist = declaration(id, where);
							/*{*/
    (void)accept(tk_offcurly, "'}' expected");

    put(obj_code(i_type, t_elaboration));
    putnum( my_type->x_index );
    putnum( agg_token->t_filename->st_index );
    putnum( agg_token->t_lineno );

    if (where == in_struct) {
	put(obj_code(i_type, t_structof));
    } else {
	put(obj_code(i_type, t_unionof));
    }

    offset = 0;
    size = 0;
    max_alignment = 0;

    if (id->id_memblist == NULL)
    {
	Ea[0].i = (int)where;
	error("null / declaration");
	return;
    }

    for (ids = id->id_memblist; ids != NULL; ids = ids->id_memblist)
    {
	register int	a;
	
	if (where == in_union)
	    offset = 0;

	if ((ids->id_flags & IDIS_FAKE) == 0)
	{
	    putnum( ids->id_index );
	    putnum( ids->id_type->x_index );

	    if (where == in_struct)
		putnum( offset );
	}

	if (ids->id_type->x_what == xt_bitfield)
	{
	    if (ids->id_type->x_value.i == 0)
	    {
		if ((ids->id_flags & IDIS_FAKE) == 0)
		    errortf(ids->id_token, "zero width bitfield tag '%'", ids->id_name);

		offset = roundup(offset, BITS_PER_INT);
	    }
	    else if (ids->id_type->x_value.i < 0)
	    {
		if ((ids->id_flags & IDIS_FAKE) == 0)
		    errortf(ids->id_token, "bitfield '%' has negative width", ids->id_name);
		else
		    errortf(ids->id_token, "tagless bitfield has negative width", ids->id_name);
	    }
	    else if (ids->id_type->x_value.i > BITS_PER_INT)
	    {
		if ((ids->id_flags & IDIS_FAKE) == 0)
		    errortf(ids->id_token, "bitfield '%' wider than an int", ids->id_name);
		else
		    errortf(ids->id_token, "tagless bitfield wider than an int", ids->id_name);
	    }
	    else
	    {
		a = offset % BITS_PER_INT;

		if (a + ids->id_type->x_value.i > BITS_PER_INT)
		    offset = roundup(offset, BITS_PER_INT);

	    	ids->id_value.i = offset;
		offset += ids->id_type->x_value.i;
	    }

	    a = alignment_in_bits(type_int);
	}
	else
	{
	    a = alignment_in_bits(ids->id_type);
	    offset = roundup(offset, a);
	    ids->id_value.i = offset;
	    offset += size_of_in_bits(ids->id_type);
	}

	if (a > max_alignment)
	    max_alignment = a;

	if (offset > size)
	    size = offset;
    }

    if (max_alignment == 0)
	internal("def_struct_union", "zero alignment");

    size = roundup(size, max_alignment);

    id->id_value.i = size;

    putnum(0);
    putnum( size );
}

/*	
 *	Gather up the list of type and storage class information which prefixes
 *	the comma seperated list of things being declared.  These are of course
 *	order independant and are just gathered into two masks until they run
 *	out.  Now and again something like 'def_enum' is called to pick up those
 *	optional complications like structure definitions that can follow some
 *	keywords.  We return an 'x_node' of which only two fields are
 *	significant, 'x_subtype' is the shared type and 'x_value.i' is a mask
 *	of the class information.  See 'types.h' for the values of this mask.
 */	
xnode	*
get_shared_type(where)
places	where;
{
    register token	*t;
    register long	type;
    register long	class;
    register xnode	*x;
    register xnode	*y;
    register int	i;
    register ident	*type_id;
    xnodes		what_type;
    long		id_flags;
    long		index;
    int			(*defining_function)();
    int			do_unget;
    token		*agg_token;

    type = 0;
    class = 0;
    x = NULL;

    loop
    {
	do_unget = 0;

	switch ((t = get_token())->t_what)
	{
	case tk_auto:
	    class |= AUTO;
	    break;

	case tk_char:
	    type |= CHAR;
	    break;

	case tk_double:
	    type |= LONG | FLOAT;
	    break;

	case tk_extern:
	    class |= EXTERN;
	    break;

	case tk_float:
	    type |= FLOAT;
	    break;

	case tk_int:
	    type |= INT;
	    break;

	case tk_long:
	    type |= LONG;
	    break;

	case tk_register:
	    class |= REGISTER;
	    break;

	case tk_short:
	    type |= SHORT;
	    break;

	case tk_static:
	    class |= STATIC;
	    break;

	case tk_typedef:
	    class |= TYPEDEF;
	    break;

	case tk_unsigned:
	    type |= UNSIGNED;
	    break;

	case tk_void:
	    type |= VOID;
	    break;

	case tk_union:
	    type |= UNION;
	    what_type = xt_unionof;
	    id_flags = IDIS_UNDEF | IDIS_UNIONNAME;
	    defining_function = def_struct_union;
	    goto complex_definition;

	case tk_struct:
	    type |= STRUCT;
	    what_type = xt_structof;
	    id_flags = IDIS_UNDEF | IDIS_STRUCTNAME;
	    defining_function = def_struct_union;
	    goto complex_definition;

	case tk_enum:
	    type |= ENUM;
	    what_type = xt_enum;
	    id_flags = IDIS_UNDEF | IDIS_ENUMNAME;
	    defining_function = def_enum;

	complex_definition:
	    {
		register ident	*new_id;

		agg_token = t;

		if ((t = get_token())->t_what == tk_name)
		{
		    register st_node	*n;

		    n = t->t_value.n;
		    free_token(t);

		    if ((t = get_token())->t_what == tk_oncurly)
			new_id = find_ident(n, id_flags, (ident *)NULL);
		    else
			new_id = refind_ident(n, id_flags, (ident *)NULL);

		    unget_token(t);
		    index = n->st_index;
		}
		else
		{
		    unget_token(t);

		    new_id = talloc(ident);
		    new_id->id_name = "";
		    new_id->id_next = NULL;
		    new_id->id_last = NULL;
		    new_id->id_parent = NULL;
		    new_id->id_memblist = NULL;
		    new_id->id_type = NULL;
		    new_id->id_object = NULL;
		    new_id->id_token = NULL;
		    new_id->id_flags = IDIS_FAKE | id_flags;
		    index = 0;
		}

		x = new_xnode();

		x->x_what = what_type;
		x->x_flags = 0;
		x->x_left = NULL;
		x->x_right = NULL;
		x->x_value.d = new_id;

		if (new_id != NULL)
		    x = find_type(x);
	    }

	    if ((t = get_token())->t_what == tk_oncurly)
		(*defining_function)(x, agg_token, index, 1);
	    else
	    {
		do_unget = 1;
		(*defining_function)(x, agg_token, index, 0);
	    }

	    free_token(agg_token);
	    break;

	case tk_name:
	    if (type != 0)
		do_unget = 1;
	    else
	    {
		if ((type_id = is_typedefed(t)) != NULL)
		{
		    x = type_id->id_type;

		    type |= TYPEDEF_USE;
		}
		else
		    do_unget = 1;
	    }

	    break;

	default:
	    do_unget = 1;
	}

	if (do_unget)
	{
	    unget_token(t);
	    break;
	}
	else
	{
	    sync(t);
	    free_token(t);
	    continue;
	}
    }

    /*
     *	If it hasn't got some fundamental type yet then it defaults to int.
     */
    if ((type & (CHAR | ENUM | FLOAT | INT | STRUCT | UNION | TYPEDEF_USE | VOID)) == 0)
	type |= INT;

    if ((where == in_struct || where == in_union) && (class & TYPE_SCOPE) != 0)
    {
	Ea[0].m = where_name[(int)where];
	error("% members may not have storage classes");

	class = 0;
    }

    if ((type & TYPEDEF_USE) != 0 && type != TYPEDEF_USE)
    {
	errorf("type '%' may not be modified", type_id->id_name);
	type = TYPEDEF_USE;
    }

    for (i = 0; i < nels(incompatibles); i++)
    {
	register long	got;
	
	got = type | class;

	if ((got & incompatibles[i].if_got_this) != 0 && (got & incompatibles[i].dont_have_these) != 0)
	{
	    register int	j;
	    register int	common;

	    Ea[0].m = incompatibles[i].my_name_is;
	    Ea[1].m = "unknown";

	    common = got & incompatibles[i].dont_have_these;

	    for (j = 0; j < nels(incompatibles); j++)
	    {
		if (common & incompatibles[j].if_got_this)
		{
		    Ea[1].m = incompatibles[j].my_name_is;
		    break;
		}
	    }

	    error("% is incompatible with %");
	    type &= ~incompatibles[i].if_got_this;
	    class &= ~incompatibles[i].if_got_this;
	}
    }

    y = new_xnode();

    if (type == TYPEDEF_USE && x->x_what == xt_basetype)
	type = x->x_value.i;

    if (x == NULL)
    {
	/*
	 *	This was some simple combination of non complicated
	 *	types and classes.
	 */
	x = new_xnode();
	x->x_flags = 0;
	x->x_right = x->x_left = NULL;
	x->x_what = xt_basetype;
	x->x_value.i = type;
	y->x_subtype = find_type(x);
    }
    else
    {
	/*
	 *	This was some user defined structured type.
	 */
	y->x_subtype = x;
    }

    y->x_value.i = class;
    y->x_what = xt_class;

    return y;
}
