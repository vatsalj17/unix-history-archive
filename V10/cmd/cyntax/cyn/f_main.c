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
#include	"func.h"

/*
 *	alloced_ids	- newly allocated identifiers
 *	eord		- equivalence class ordinal
 *	equiv_value	- equivalence classes values
 *	equiv_max	- size of the equiv_value vector
 *	trace_argument	- currently tracing an argument
 *	eq_set		- set of identifiers assigned to in
 *	err_set		- set of undefined order of evaluation identifiers
 *	merr_set	- set of undefined order of modification identifiers
 */
ident	*alloced_ids;
int	eord;
char	*equiv_value;
int	equiv_max;
int	trace_argument;
xnode	*eq_set;
xnode	*err_set;
xnode	*merr_set;

analyse_flow(k)
cfrag	*k;
{
    register cnode	*c;
    register cnode	*last;
    flow_set		fs;

    make_pure(func.fn_args);
    make_pure(func.fn_locals);

    eq_set = NULL;
    last = NULL;

    for (c = k->c_head; c != NULL; c = c->c_next)
    {
	c->c_last = last;

	if (in(expressions, c->c_what))
	{
	    expr_flow(c->c_value.x, &fs);
	    fs.f_cmod = flow_diff(fs.f_cmod, fs.f_mod);


	    print_errs();

	    c->c_flow = flow_pack(&fs);
	}
	else
	{
	    if (c->c_what == ct_sync)
		csync(c);

	    c->c_flow = NULL;
	}

	last = c;
    }
}

void
free_fs(c, f)
register cnode	*c;
register int	f;
{
    while (c != NULL)
    {
	register xnode	**n;

	n = &c->c_flow;

	while (*n != NULL)
	{
	    if (((*n)->x_flags & f) != 0)
	    {
		register xnode	*x;

		x = *n;
		*n = x->x_left;
		free_xnode(x);
	    }
	    else
		n = &(*n)->x_left;
	}

	c = c->c_next;
    }
}

/*
 *	Allocate locals storage.
 */
alloc_locals(k)
register cfrag	*k;
{
    trace_argument = 1;
    life_list(func.fn_args, k->c_head);

    trace_argument = 0;
    life_list(func.fn_locals, k->c_head);

    free_xlist(eq_set);

    free_fs(k->c_head, C_X_CUT | C_X_MOD | C_X_REF | C_X_HIDE | C_X_CMOD);
}

free_flow(k)
cfrag	*k;
{
    free_fs(k->c_head, C_X_CUT | C_X_MOD | C_X_REF | C_X_HIDE | C_X_CMOD | C_ID_BEGIN | C_ID_END | C_ID_BECOMES);
}
