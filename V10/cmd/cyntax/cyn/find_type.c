/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"
#include	"stdobj.h"
#include	"io.h"

/*
 *	Map a type to a unique (private) xnode.  The type we are given has
 *	an interned subtype (if applicable) and freeable top node.  The
 *	type table is indexed (effectively) by xt_what (range reduced by
 *	fake su entries).  An interned type can be found by following
 *	the tree from the type table base entry.
 */
xnode	*
find_type(x)
register xnode	*x;
{
	typedef struct ttnode	ttnode;

	struct ttnode
	{
		ttnode	*tt_left;
		ttnode	*tt_right;
		xnode	*tt_type;
	};

	register xnode	*p;
	register xnode	*q;
	register long	i;
	register ttnode	**n;
	register ttnode	*t;

	static ttnode	*type_table[(int)su_types];

	extern long	type_index;

	n = &type_table[(int)sunpack(xdope[(int)x->x_what].xd_suequiv)];

	while (*n != NULL)
	{
		p = x;
		q = (*n)->tt_type;

		if ((i = ((int)p->x_what) - ((int)q->x_what)) == 0)
		{
			switch (p->x_what)
			{
			case xt_basetype:
				i = p->x_value.i - q->x_value.i;
				break;

			case xt_arrayof:
			case xt_bitfield:
				if ((i = p->x_value.i - q->x_value.i) == 0)
					i = (char *)p->x_subtype - (char *)q->x_subtype;

				break;

			case xt_enum:
			case xt_structof:
			case xt_unionof:
				i = (char *)p->x_value.d - (char *)q->x_value.d;
				break;

			case xt_ftnreturning:
			case xt_ptrto:
				i = (char *)p->x_subtype - (char *)q->x_subtype;
				break;

			default:
				internal("find_type", "bad type");
			}

			if (i == 0)
			{
				free_xnode(x);
				return (*n)->tt_type;
			}
		}

		n = i < 0 ? &((*n)->tt_left) : &((*n)->tt_right);
	}

	switch (x->x_what)
	{
	case xt_arrayof:
		if ((i = x->x_value.i) < 0) {
			put(obj_code(i_type, t_dimless));
		}
		else
		{
			put(obj_code(i_type, t_arrayof));
			putnum(i);
		}

		putnum( x->x_subtype->x_index );
		x->x_index = type_index++;
		break;
		
	case xt_basetype:
		put(obj_code(i_type, t_basetype));
		put(x->x_value.i);
		x->x_index = type_index++;
		break;

	case xt_bitfield:
		put(obj_code(i_type, t_bitfield));
		putnum( x->x_value.i );
		putnum( x->x_subtype->x_index );
		x->x_index = type_index++;
		break;

	case xt_enum:
	case xt_structof:
	case xt_unionof:
		x->x_index = 0;
		break;

	case xt_ftnreturning:
		put(obj_code(i_type, t_ftnreturning));
		putnum( x->x_subtype->x_index );
		x->x_index = type_index++;
		break;

	case xt_ptrto:
		put(obj_code(i_type, t_ptrto));
		putnum( x->x_subtype->x_index );
		x->x_index = type_index++;
		break;

	default:
		internal("find_type", "bad type");
	}

	t = talloc(ttnode);
	*n = t;
	t->tt_type = talloc(xnode);
	*t->tt_type = *x;
	t->tt_left = NULL;
	t->tt_right = NULL;
	free_xnode(x);
	return t->tt_type;
}
