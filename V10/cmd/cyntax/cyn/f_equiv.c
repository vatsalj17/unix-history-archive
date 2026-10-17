/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"
#include	"cnodes.h"
#include	"flow.h"

/*
 *	Lifetime determination.
 *	
 *	Method (per pure ident):
 *	
 *	Initially the ords are set to L_NONE and the flags contain
 *	possibly the five flow set flags.  The outer loop attempts to
 *	fill in the graph's ords.  Each fill is cut by C_CUT_ID nodes
 *	and dead ends.  A fill is further processed twice.  The fill's
 *	ords are set to L_ACTIVE in the outer loop.  The first scan
 *	breaks the fill up into id equivalent classes and detects which
 *	of these classes are active.  Equivalence classes start at
 *	L_EQUIV and go up.  The equivalence class values are stored in a
 *	vector.  The second pass marks the nodes in inactive classes as
 *	dead and assigns a new ordinal, unique to this fill, to the
 *	active node.  These start at L_HEAD, which signifies that this
 *	fill started at the function entry point, and go down.
 *	
 *	Four types of diagnostics can be gained from this analysis.  The
 *	error [variable used where it cannot have a meaningful value],
 *	the weaker warning [variable may be used before set], and the
 *	warnings [value assigned to variable not used] and [initial
 *	value of argument not used].
 *
 *	Suspensions.
 *	------------
 *	An equiv class that is under consideration goes into E_SUSP
 *	mode.  His knowledge starts off suspended.  E_SUSP is the
 *	identity element under (|), equiv combination.  Since we have no
 *	unreachable code each fill must be resolved by a write or the
 *	top of the code.  Thus we can change all current suspensions to
 *	the return value at the base level.
 */

/*
 *	Initialise equivalence data.
 */
void
equiv_init()
{
    register int	i;

    eord = L_EQUIV;

    for (i = 0; i < equiv_max; i++)
	equiv_value[i] = 0;
}

/*
 *	Store equivalence value.
 */
void
equiv_store(i, v)
register int	i;
int		v;
{
    if (i >= equiv_max)
    {
	equiv_max = E_ROUND(i);
	equiv_value = vector(equiv_value, equiv_max, char);
    }

    equiv_value[i] = v;
}

/*
 *	Trace an equivalence class of an instance of an identifier.
 */
int
trace_equiv(start)
cnode	*start;
{
    typedef enum
    {
	eq_combine,
	eq_return,
	eq_scan,
    }
	eq_states;

    register cnode	*c;
    register int	ret;
    register int	ord;
    register int	combine;
    register eq_states	state;
    register int	cmod;

    static int	equiv_combine[4][4]	=
    {
    /*  E_NONE,	E_SOME,	E_ALL,	E_SUSP	*/
	E_NONE,	E_SOME,	E_SOME,	E_NONE,	/* E_NONE */
	E_SOME,	E_SOME,	E_SOME,	E_SOME,	/* E_SOME */
	E_SOME,	E_SOME,	E_ALL,	E_ALL,	/* E_ALL */
	E_NONE,	E_SOME,	E_ALL,	E_SUSP,	/* E_SUSP */
    };

    combine = E_NONE;
    c = start;
    ord = eord++;
    equiv_store(ord, E_SUSP);
    ret = E_SUSP;
    cmod = 0;
    state = eq_scan;

    loop
    {
	switch (state)
	{
	case eq_combine:
	    ret = equiv_combine[ret][combine];
	    state = eq_scan;
	    break;

	case eq_return:
	    if (ret == E_NONE && cmod)
		ret = E_SOME;

	    return equiv_value[ord] = equiv_combine[ret][combine];

	case eq_scan:
	    if (c == NULL)
	    {
		if (trace_argument)
		    combine = E_ALL;
		else
		    combine = E_NONE;

		state = eq_return;
		continue;
	    }


	    if (in(mip_dead_ends, c->c_what) && c != start)
	    {
		combine = E_SUSP;
		state = eq_return;
		continue;
	    }

	    if (c->c_ord == ord)
		internal("trace_equiv", "circular last trail");

	    if ((c->c_flags & C_X_MOD) != 0 && c != start)
	    {
		combine = E_ALL;
		state = eq_return;
		continue;
	    }

	    if ((c->c_flags & C_X_CMOD) != 0 && c != start)
	    	cmod = 1;

	    if (c->c_ord != L_ACTIVE)
	    {
		if (c->c_ord < L_NONE)
		    internal("trace_equiv", "ran into trouble");
		else if (c->c_ord == L_NONE)
		    internal("trace_equiv", "ran off fill");

		combine = equiv_value[c->c_ord];
		state = eq_return;
		continue;
	    }

	    if (c->c_what == ct_label)
	    {
		register cnode	*d;

		for (d = c->c_value.c; d != NULL; d = d->c_link)
		{
		    if ((d->c_flags & C_SWITCH) != 0)
		    {
			register cnode	*s;

			s = d->c_switch;

			if (s->c_ord != ord && s->c_ord != L_NONE)
			{
			    if ((s->c_flags & C_X_CUT) != 0)
				combine = E_ALL;
			    else if (s->c_ord == L_ACTIVE)
			    {
				s->c_ord = ord;
				combine = trace_equiv(s->c_last);
			    }
			    else
			    {
				if (s->c_ord < L_NONE)
				    internal("trace_equiv", "ran into switch trouble");
				combine = equiv_value[s->c_ord];
			    }
			}
		    }
		    else
			combine = trace_equiv(d);

		    ret = equiv_combine[ret][combine];
		}
	    }

	    break;

	default:
	    internal("trace_equiv", "bad state");
	}


	c->c_ord = ord;
	c = c->c_last;
    }
}
