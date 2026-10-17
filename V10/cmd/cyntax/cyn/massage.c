/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */
#include	"defs.h"
#include	"cnodes.h"
#include	"macdep.h"
#include	"types.h"
#include	"xtypes.h"

extern xtset	yield_truth;
extern xtset	binary_ops;
extern xtset	unary_ops;
extern xtset	constants;
extern xtset	int_constants;
extern xtset	base_constants;
extern xtset	commutatives;

extern xnodes	cvt_xnodes();
extern xnode	*su_opt();
extern xnode	*zero();

typedef enum
{
	yt_equality,
	yt_inequality,
	yt_less_than,
	yt_less_equal,
	yt_greater_than,
	yt_greater_equal,
}
	truths;

pack_t	rev_op[]	=
{
	pack(xt_eqeq),
	pack(xt_noteq),
	pack(xt_grt),
	pack(xt_grteq),
	pack(xt_less),
	pack(xt_lesseq),
};

pack_t	not_op[]	=
{
	pack(xt_noteq),
	pack(xt_eqeq),
	pack(xt_grteq),
	pack(xt_grt),
	pack(xt_lesseq),
	pack(xt_less),
};


#define CINT	0
#define CUINT	1
#define CDOUB	2

#define	Xv	(x->x_value)
#define	Lv	(x->x_left->x_value)
#define	Rv	(x->x_right->x_value)

/*
 *	Constant folding.  Algorithm is pointer reversal.
 */
xnode	*
traverse(x, do_su)
register xnode	*x;
int		do_su;
{
    register xnode	*dad;
    register xnode	*y;
    register int	t;

    dad = NULL;

    loop
    {
	if (in(binary_ops, x->x_what))
	{
	    if ((x->x_flags & XIS_LTRAV) != 0)
	    {
		/*
		 *	Left traversed.
		 */
		if ((x->x_flags & XIS_RTRAV) != 0)
		{
		    /*
		     *	Left and right traversed.
		     */
		    if
		    (
			in(base_constants, x->x_left->x_what)
			&&
			in(base_constants, x->x_right->x_what)
			&&
			x->x_what != xt_colon
		    )
		    {
			y = x->x_left->x_type;

		    bin_const:
			/*
			 *	Simple binary with basetype constant operands.
			 */
			if (y->x_what != xt_basetype)
			    internal("traverse", "bad binop type");

			switch ((int)y->x_value.i & (int)(FLOAT | INT | UNSIGNED))
			{
			case INT:
			    t = CINT;
			    break;

			case FLOAT:
			    t = CDOUB;
			    break;

			case UNSIGNED | INT:
			    t = CUINT;
			    break;

			default:
			    internal("traverse", "bad binop type");
			}

			switch(x->x_what)
			{
			case xt_and:
			    Xv.i = Lv.i & Rv.i;
			    break;

			case xt_andand:
			    Xv.i = Lv.i && Rv.i;
			    break;

			case xt_bar:
			    Xv.i = Lv.i | Rv.i;
			    break;

			case xt_barbar:
			    Xv.i = Lv.i || Rv.i;
			    break;

			case xt_comma:
			    switch(t)
			    {
			    case CDOUB:
				Xv.f = Rv.f;
				break;

			    case CUINT:
			    case CINT:
				Xv.i = Rv.i;
				break;

			    default:
				goto bad_type;
			    }

			    break;

			case xt_eqeq:
			    Xv.i = Lv.i == Rv.i;
			    break;

			case xt_grt:
			    switch (t)
			    {
			    case CDOUB:
				Xv.i = Lv.f > Rv.f;
				break;

			    case CINT:
				Xv.i = Lv.i > Rv.i;
				break;

			    case CUINT:
				Xv.i = (unsigned)Lv.i > (unsigned)Rv.i;
				break;

			    default:
				goto bad_type;
			    }

			    break;

			case xt_grteq:
			    switch (t)
			    {
			    case CDOUB:
				Xv.i = Lv.f >= Rv.f;
				break;

			    case CINT:
				Xv.i = Lv.i >= Rv.i;
				break;

			    case CUINT:
				Xv.i = (unsigned)Lv.i >= (unsigned)Rv.i;
				break;

			    default:
				goto bad_type;
			    }

			    break;

			case xt_grtgrt:
			    if (t == CINT)
				    Xv.i = Lv.i >> Rv.i;
			    else
				    Xv.i = (unsigned long)Lv.i >> Rv.i;

			    break;

			case xt_less:
			    switch(t)
			    {
			    case CDOUB:
				Xv.i = Lv.f < Rv.f;
				break;

			    case CINT:
				Xv.i = Lv.i < Rv.i;
				break;

			    case CUINT:
				Xv.i = (unsigned)Lv.i < (unsigned)Rv.i;
				break;

			    default:
				goto bad_type;
			    }

			    break;

			case xt_lesseq:
			    switch(t)
			    {
			    case CDOUB:
				Xv.i = Lv.f <= Rv.f;
				break;

			    case CINT:
				Xv.i = Lv.i <= Rv.i;
				break;

			    case CUINT:
				Xv.i = (unsigned)Lv.i <= (unsigned)Rv.i;
				break;

			    default:
				goto bad_type;
			    }

			    break;

			case xt_lessless:
			    if (t == CINT)
				    Xv.i = Lv.i << Rv.i;
			    else
				    Xv.i = (unsigned long)Lv.i << Rv.i;

			    break;

			case xt_minus:
			    switch(t)
			    {
			    case CDOUB:
				Xv.f = Lv.f - Rv.f;
				break;

			    case CINT:
				Xv.i = Lv.i - Rv.i;
				break;

			    case CUINT:
				Xv.i = (unsigned)Lv.i - (unsigned)Rv.i;
				break;

			    default:
				goto bad_type;
			    }

			    break;

			case xt_noteq:
			    Xv.i = Lv.i != Rv.i;
			    break;

			case xt_percent:
			    switch(t)
			    {
			    case CINT:
				if (Rv.i == 0)
				    error("integer modulus zero");
				else
				    Xv.i = Lv.i % Rv.i;

				break;

			    case CUINT:
				if (Rv.i == 0)
				    error("unsigned integer modulus zero");
				else
				    Xv.i = (unsigned)Lv.i % (unsigned)Rv.i;

				break;

			    default:
				goto bad_type;
			    }

			    break;

			case xt_plus:
			    switch(t)
			    {
			    case CDOUB:
				Xv.f = Lv.f + Rv.f;
				break;

			    case CINT:
				Xv.i = Lv.i + Rv.i;
				break;

			    case CUINT:
				Xv.i = (unsigned)Lv.i + (unsigned)Rv.i;
				break;

			    default:
				goto bad_type;
			    }

			    break;

			case xt_slash:
			    switch(t)
			    {
			    case CDOUB:
				if (Rv.f == 0.0)
				    error("floating point division by zero");
				else
				    Xv.f = Lv.f / Rv.f;

				break;

			    case CINT:
				if (Rv.i == 0)
				    error("integer division by zero");
				else
				    Xv.i = Lv.i / Rv.i;

				break;

			    case CUINT:
				if (Rv.i == 0)
				    error("unsigned integer division by zero");
				else
				    Xv.i = (unsigned)Lv.i / (unsigned)Rv.i;

				break;

			    default:
				goto bad_type;
			    }

			    break;

			case xt_times:
			    switch(t)
			    {
			    case CDOUB:
				Xv.f = Lv.f * Rv.f;
				break;

			    case CINT:
				Xv.i = Lv.i * Rv.i;
				break;

			    case CUINT:
				Xv.i = (unsigned)Lv.i * (unsigned)Rv.i;
				break;

			    default:
				goto bad_type;
			    }

			    break;

			case xt_uparrow:
			    Xv.i = Lv.i ^ Rv.i;
			    break;

			bad_type:
			    internal("traverse", "bad opt type");

			default:
			    internal("traverse", "bad binop");
			}

			free_xnode(x->x_right);
			free_xnode(x->x_left);

			if (x->x_type->x_what != xt_basetype)
			    internal("traverse", "could not type binop");

			switch ((int)x->x_type->x_value.i & (int)(FLOAT | INT | UNSIGNED | LONG))
			{
			case INT:
			    x->x_what = xt_cint;
			    break;

			case LONG | INT:
			    x->x_what = xt_clong;
			    break;

			case LONG | FLOAT:
			    x->x_what = xt_cdouble;
			    break;

			case UNSIGNED | INT:
			    x->x_what = xt_cuint;
			    break;

			case UNSIGNED | LONG | INT:
			    x->x_what = xt_culong;
			    break;

			default:
			    internal("traverse", "could not type binop");
			}

			goto const_su;
		    }
		    else
		    {
			/*
			 *	Some special case analysis.
			 */
		        switch (x->x_what)
			{
			case xt_question:
			    /*
			     *	Conditional with constant left side.
			     */
			    if (in(base_constants, x->x_left->x_what))
			    {
				y = x;

				if (x->x_left->x_what == xt_cdouble)
				{
				    if (Lv.f != 0.0)
					x = x->x_right->x_left;
				    else
					x = x->x_right->x_right;
				}
				else
				{
				    if (Lv.i != 0)
					x = x->x_right->x_left;
				    else
					x = x->x_right->x_right;
				}

				free_xnode(y->x_right);
				free_xnode(y);
			    }

			    goto done_bin;

			case xt_ptrdiff:
			    /*
			     *	Pointer difference.  Check for
			     *	&a.b - &a.c type optimisation.
			     */
			    if
			    (
				x->x_left->x_what == xt_caddr
				&&
				x->x_right->x_what == xt_caddr
				&&
				x->x_left->x_left == x->x_right->x_left
			    )
			    {
				if (x->x_left->x_left != NULL)
				{
				    free_xnode(x->x_left->x_left);
				    free_xnode(x->x_right->x_left);
				}

				/*
				 *	Change it to minus and proceed
				 *	as for minus.
				 */
				x->x_what = xt_minus;
				y = x->x_type;
				goto bin_const;
			    }

			    goto no_opt;

			case xt_ptrminus:
			case xt_ptrplus:
			    /*
			     *	Pointer plus/minus with right operand
			     *	constant.  Can be optimised if left is
			     *	constant address or string.
			     */
/* w/o code generation alignments and sizes bogus */
#define size_of_in_bits(type)	BITS_PER_INT
			    if (in(base_constants, x->x_right->x_what))
			    {
				/*
				 *	The left son is of type ptrto ...
				 *	so left base_constants have become
				 *	caddrs with NULL left sons.
				 */
				switch (x->x_left->x_what)
				{
				case xt_caddr:
				    if (x->x_what == xt_ptrminus)
					Xv.i = Lv.i - Rv.i * size_of_in_bits(x->x_type->x_subtype) / BITS_PER_BYTE;
				    else
					Xv.i = Lv.i + Rv.i * size_of_in_bits(x->x_type->x_subtype) / BITS_PER_BYTE;

				    x->x_what = xt_caddr;
				    x->x_left = (y = x->x_left)->x_left;
				    free_xnode(y);
				    goto const_su;

				case xt_string:
				    if (x->x_what == xt_ptrminus)
					x->x_left->x_stroff -= Rv.i * size_of_in_bits(x->x_type->x_subtype) / BITS_PER_BYTE;
				    else
					x->x_left->x_stroff += Rv.i * size_of_in_bits(x->x_type->x_subtype) / BITS_PER_BYTE;

				    y = x;
				    x = x->x_left;
				    free_xnode(y);
				    goto const_su;
				}
			    }

			    goto no_opt;
			}

			if (in(yield_truth, x->x_what))
			{
			    if (in(constants, x->x_right->x_what))
			    {
				/*
				 *	Rearrange e relop C so that
				 *	the constant is the left son.
				 */
				y = x->x_left;
				x->x_left = x->x_right;
				x->x_right = y;
				x->x_what = cvt_xnodes(x->x_what, rev_op);
			    }
			}
			else
			{
			    /*
			     *	Rearrange a commuative operator's sons
			     *	so that the cheapest is on the left.
			     */
			    if
			    (
				in(commutatives, x->x_what)
				&&
				x->x_left->x_su < x->x_right->x_su
			    )
			    {
				y = x->x_left;
				x->x_left = x->x_right;
				x->x_right = y;
			    }

			    if (in(base_constants, x->x_right->x_what))
			    {
				/*
				 *	Some optimisations for single
				 *	base constant operand.
				 *
				 *	The constant is assumed to be on
				 *	the right because of the su code above.
				 */
				y = x->x_right;

				switch (x->x_what)
				{
				case xt_and:
				    /*
				     *	e & 0 -> e , 0
				     */
				    if (y->x_value.i == 0)
					x->x_what = xt_comma;

				    break;

				case xt_bar:
				case xt_plus:
				case xt_uparrow:
				    /*
				     *	e [|+^] 0 -> e
				     */
				    if
				    (
					(
					    y->x_what == xt_cdouble
					    &&
					    y->x_value.f == 0.0
					)
					||
					(
					    y->x_what != xt_cdouble
					    &&
					    y->x_value.i == 0
					)
				    )
				    {
					free_xnode(y);
					y = x;
					x = x->x_left;
					free_xnode(y);
					goto done_bin;
				    }

				    break;

				case xt_times:
				    /*
				     *	e * 0 -> e , 0
				     *	e * 1 -> e
				     */
				    if (y->x_what == xt_cdouble)
				    {
					if (y->x_value.f == 0.0)
					    x->x_what = xt_comma;
					else if (y->x_value.f == 1.0)
					{
					    free_xnode(y);
					    y = x;
					    x = x->x_left;
					    free_xnode(y);
					    goto done_bin;
					}
				    }
				    else
				    {
					if (y->x_value.i == 0)
					    x->x_what = xt_comma;
					else if (y->x_value.i == 1)
					{
					    free_xnode(y);
					    y = x;
					    x = x->x_left;
					    free_xnode(y);
					    goto done_bin;
					}
				    }
				}
			    }
			}

		    no_opt:
			/*
			 *	Sethi-uhlman calculation.
			 */
			t = x->x_left->x_su;

			if (x->x_right->x_su > t)
			    t = x->x_right->x_su;
			else if (x->x_right->x_su == t)
			    t += SAMESU;

			t += sutab[(int)x->x_what];

			if (t > MAXSU)
			    t = MAXSU;

			x->x_su = t;
		    }

		done_bin:
		    x->x_flags &= ~(XIS_LTRAV | XIS_RTRAV);
		}
		else
		{
		    /*
		     *	Go right.
		     */
		    x->x_flags |= XIS_RTRAV;
		    y = x->x_right;
		    x->x_right = dad;
		    dad = x;
		    x = y;
		    continue;
		}
	    }
	    else
	    {
		/*
		 *	Go left.
		 */
		x->x_flags |= XIS_LTRAV;
		y = x->x_left;
		x->x_left = dad;
		dad = x;
		x = y;
		continue;
	    }
	}
	else if (in(unary_ops, x->x_what))
	{
	    if ((x->x_flags & XIS_LTRAV) != 0)
	    {
		/*
		 *	Left traversed.
		 */
		if (in(base_constants, x->x_left->x_what))
		{
		    y = x->x_left->x_type;

		    if (y->x_what != xt_basetype)
			internal("traverse", "bad unop type");

		    /*
		     *	The char can arise here because cast nodes hold on
		     *	tight to their types.
		     */
		    switch ((int)y->x_value.i & (int)(FLOAT | CHAR | INT | UNSIGNED))
		    {
		    case CHAR:
		    case INT:
			t = CINT;
			break;

		    case FLOAT:
			t = CDOUB;
			break;

		    case UNSIGNED | CHAR:
		    case UNSIGNED | INT:
			t = CUINT;
			break;

		    default:
			internal("traverse", "bad unop type");
		    }

		    switch(x->x_what)
		    {
		    case xt_cast:
			switch (x->x_type->x_what)
			{
			case xt_enum:
			    x->x_type = type_int;

			case xt_basetype:
			    switch ((int)x->x_type->x_value.i & (int)(TYPE_BASE | TYPE_ADJECTIVE))
			    {
			    case VOID:
				/*
				 *	No-one will use the value.
				 */
				break;

			    case INT:
				if (t == CDOUB)
				    Xv.i = (int)Lv.f;
				else
				    Xv.i = (int)Lv.i;

				break;

			    case LONG | INT:
				if (t == CDOUB)
				    Xv.i = Lv.f;
				else
				    Xv.i = Lv.i;

				break;

			    case SHORT | INT:
				if (t == CDOUB)
				    Xv.i = (short)Lv.f;
				else
				    Xv.i = (short)Lv.i;

				break;

			    case FLOAT:
			    case LONG | FLOAT:
				if (t == CDOUB)
				    Xv.f = Lv.f;
				else if (t == CINT)
				    Xv.f = Lv.i;
				else if (t == CUINT)
				    Xv.f = (unsigned long)Lv.i;

				break;

			    case UNSIGNED | INT:
				if (t == CDOUB)
				    Xv.i = (unsigned int)Lv.f;
				else
				    Xv.i = (unsigned int)Lv.i;

				break;

			    case UNSIGNED | LONG | INT:
				if (t == CDOUB)
				    Xv.i = (unsigned long)Lv.f;
				else
				    Xv.i = (unsigned long)Lv.i;

				break;

			    case UNSIGNED | SHORT | INT:
				if (t == CDOUB)
				    Xv.i = (unsigned short)Lv.f;
				else
				    Xv.i = (unsigned short)Lv.i;

				break;

			    case CHAR:
				if (t == CDOUB)
				    Xv.i = (char)Lv.f;
				else
				    Xv.i = (char)Lv.i;

				break;

			    case UNSIGNED | CHAR:
				if (t == CDOUB)
				    Xv.i = (uchar)Lv.f;
				else
				    Xv.i = (uchar)Lv.i;

				break;

			    default:
				internal("traverse", "bad cast basetype");
			    }

			    break;

			case xt_ptrto:
			    x->x_what = xt_caddr;

			    if (t == CDOUB)
				Xv.i = (unsigned long)Lv.f;
			    else

				Xv.i = (unsigned long)Lv.i;

			    free_xnode(x->x_left);
			    x->x_left = NULL;

			    goto const_su;

			default:
			    goto done_un;
			}

			break;

		    case xt_not:
			switch(t)
			{
			case CUINT:
			case CINT:
			    Xv.i = !Lv.i;
			    break;

			case CDOUB:
			    Xv.i = !Lv.f;
			    break;

			default:
			    goto bad_type;
			}

			break;

		    case xt_tilda:
			Xv.i = ~Lv.i;
			break;

		    case xt_uminus:
			switch(t)
			{
			case CUINT:
			case CINT:
			    Xv.i = -Lv.i;
			    break;

			case CDOUB:
			    Xv.f = -Lv.f;
			    break;

			default:
			    goto bad_type;
			}

			break;

		    default:
			internal("traverse", "bad switch on unop");
		    }

		    free_xnode(x->x_left);

		    if (x->x_type->x_what == xt_basetype)
		    {
			switch ((int)x->x_type->x_value.i & (int)(FLOAT | CHAR | INT | UNSIGNED | LONG))
			{
			case CHAR:
			case INT:
			    x->x_what = xt_cint;
			    break;

			case LONG | INT:
			    x->x_what = xt_clong;
			    break;

			case FLOAT:
			case LONG | FLOAT:
			    x->x_what = xt_cdouble;
			    break;

			case UNSIGNED | CHAR:
			case UNSIGNED | INT:
			    x->x_what = xt_cuint;
			    break;

			case UNSIGNED | LONG | INT:
			    x->x_what = xt_culong;
			    break;

			default:
			    internal("traverse", "could not type unop");
			}
		    }
		    else if (x->x_type->x_what == xt_ptrto)
			x->x_what == xt_clong;
		    else {
			internal("traverse", "couldn't type unop");
			}

		    goto const_su;
		}
		else
		{
		    /*
		     *	Some specific unary optimisations and transformations.
		     */
		    switch (x->x_what)
		    {
		    case xt_cast:
			switch (x->x_left->x_what)
			{
			case xt_caddr:
			case xt_string:
			    /*
			     *	Cast of string or constant address, change type.
			     */
			    y = x->x_left;
			    y->x_type = x->x_type;
			    free_xnode(x);
			    x = y;
			    goto done_un;
			}

			break;

		    case xt_dot:
			if (x->x_left->x_what == xt_dot)
			{
			    /*
			     *	Cmbine multiple dots.
			     */
			    y = x->x_left;
			    y->x_value.i += Xv.i;
			    y->x_type = x->x_type;
			    y->x_flags = x->x_flags;
			    free_xnode(x);
			    x = y;
			}

			break;

		    case xt_not:
			if (in(yield_truth, x->x_left->x_what))
			{
			    /*
			     *	not of relop
			     */
			    y = x->x_left;
			    y->x_what = cvt_xnodes(y->x_what, not_op);
			    free_xnode(x);
			    x = y;
			    goto done_un;
			}

			switch (x->x_left->x_what)
			{
			case xt_andand:
			case xt_barbar:
			    /*
			     *	not of && or ||, de morgans law
			     */
			    y = x->x_left;
			    x->x_flags = 0;
			    x->x_left = y->x_left;
			    y->x_left = x;
			    x = new_xnode();
			    x->x_what = xt_not;
			    x->x_type = type_int;
			    x->x_flags = 0;
			    x->x_left = y->x_right;
			    y->x_right = x;

			    /*
			     *	Revisit.
			     */
			    y->x_flags |= XIS_LTRAV;
			    x = y->x_left;
			    y->x_left = dad;
			    dad = y;
			    continue;
			}

			break;

		    case xt_uand:
			switch (x->x_left->x_what)
			{
			case xt_dot:
			    /*
			     *	& a.b -> constant address.
			     */
			    if (x->x_left->x_left->x_what == xt_name)
			    {
				y = x->x_left;
				y->x_what = xt_caddr;
				y->x_type = x->x_type;
				free_xnode(x);
				x = y;
				goto const_su;
			    }

			    break;

			case xt_name:
			    /*
			     *	& a -> constant address.
			     */
			    x->x_what = xt_caddr;
			    Xv.i = 0;
			    x->x_su = sutab[(int)xt_caddr];
			    goto done_un;

			case xt_utimes:
			    /*
			     *	& * cancel
			     */
			    goto cancel;
			}

			break;

		    case xt_uminus:
			/*
			 *	- - cancel
			 */
			if (x->x_left->x_what == xt_minus)
			    goto cancel;

			break;

		    case xt_utimes:
			switch (x->x_left->x_what)
			{
			case xt_uand:
			    /*
			     *	* & cancel.
			     */
		    cancel:
			    y = x->x_left->x_left;
			    y->x_flags = x->x_flags;
			    free_xnode(x->x_left);
			    free_xnode(x);
			    x = y;
			    goto done_un;

			case xt_caddr:
			    /*
			     *	* constant addr -> dot
			     */
			    y = x->x_left;
			    y->x_what = xt_dot;
			    y->x_flags = x->x_flags;
			    y->x_type = x->x_type;
			    free_xnode(x);
			    x = y;
			}
		    }

		    /*
		     *	Sethi-uhlman calculation.
		     */
		    t = x->x_left->x_su + sutab[(int)x->x_what];

		    if (t > MAXSU)
			t = MAXSU;

		    x->x_su = t;
		}

	    done_un:
		x->x_flags &= ~XIS_LTRAV;
	    }
	    else
	    {
		if (x->x_what == xt_call && x->x_value.k != NULL)
		{
		    register cnode	*c;

		    for (c = x->x_value.k->c_head; c != NULL; c = c->c_next)
			c->c_value.x = traverse(c->c_value.x, 1);
		}

		if (x->x_what != xt_dot || x->x_left != NULL)
		{
		    x->x_flags |= XIS_LTRAV;
		    y = x->x_left;
		    x->x_left = dad;
		    dad = x;
		    x = y;
		    continue;
		}
	    }
	}
	else
	{
	    /*
	     *	Leaf.
	     */

	    /*
	     *	enum -> int
	     */
	    if (x->x_type->x_what == xt_enum && x->x_what == xt_name && (x->x_value.d->id_flags & IDIS_MOENUM) != 0)
	    {
		x->x_what = xt_cint;
		Xv.i = Xv.d->id_value.i;
	    }

	    /*
	     *	name -> name . 0
	     */
	    if (x->x_what == xt_name)
	    {
		y = new_xnode();
		*y = *x;
		x->x_what = xt_dot;
		Xv.i = 0;
		x->x_left = y;
	    }

	const_su:
	    x->x_su = sutab[(int)x->x_what];
	}

	if (x->x_type->x_what == xt_enum)
	    x->x_type = type_int;

	/*
	 *	Sethi-Uhlman equiv class optimisation.
	 */
	{
	    pack_t	seq;

	    if
	    (
		do_su
		&&
		(int)sunpack(seq = xdope[(int)x->x_what].xd_suequiv) > (int)su_null
		&&
		(
		    dad == NULL
		    ||
		    xdope[(int)dad->x_what].xd_suequiv != seq
		)
		&&
		(
		    xdope[(int)x->x_left->x_what].xd_suequiv == seq
		    ||
		    x->x_what == xt_uminus
		    ||
		    xdope[(int)x->x_right->x_what].xd_suequiv == seq
		)
	    )
		x = su_opt(x, seq);
	}

	if (dad == NULL)
	    return x;

	if (dad->x_flags & XIS_RTRAV)
	{
	    y = dad->x_right;
	    dad->x_right = x;
	}
	else
	{
	    y = dad->x_left;
	    dad->x_left = x;
	}

	x = dad;
	dad = y;
    }
}

/*
 *	Convert a relational operator.
 */
xnodes
cvt_xnodes(w, v)
xnodes	w;
pack_t	*v;
{
    truths	t;

    switch (w)
    {
    case xt_eqeq:
	t = yt_equality;
	break;

    case xt_grt:
	t = yt_greater_than;
	break;

    case xt_grteq:
	t = yt_greater_equal;
	break;

    case xt_less:
	t = yt_less_than;
	break;

    case xt_lesseq:
	t = yt_less_equal;
	break;

    case xt_noteq:
	t = yt_inequality;
	break;

    default:
	internal("cvt_xnodes", "bad what");
    }

    return xunpack(v[(int)t]);
}
