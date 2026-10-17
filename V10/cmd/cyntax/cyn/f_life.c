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
#include	"func.h"
#include	"options.h"
#include	"tokens.h"

/*
 *	Trace an instance.
 */
void
trace_instance(id, head, tail, ord, usage)
register ident	*id;
register cnode	*head;
register cnode	*tail;
register int	ord;
int		usage;
{
    register cnode	*c;

    trace_life(head);

    equiv_init();

    for (c = tail; c != NULL; c = c->c_last)
    {
	if (c->c_ord == L_ACTIVE && (c->c_flags & C_X_REF) != 0)
	{
	    register int	i;
	    int			ret;
	    int			save;

	    save = eord;

	    switch (ret = trace_equiv(c))
	    {
	    case E_ALL:
		break;

	    case E_SOME:
		if (option(o_warnings))
		{
		    sync_back(c);
		    warnf("'%' read where it may not have a meaningful value", id->id_name);
		}

		break;

	    case E_SUSP:
		internal("trace instance", "suspended node");

	    case E_NONE:
		sync_back(c);
		errorf("'%' read where it cannot have a meaningful value", id->id_name);

		break;

	    default:
		internal("bad switch", "trace_instance");
	    }

	    for (i = save; i < eord; i++)
	    {
		if (equiv_value[i] == E_SUSP)
		    equiv_value[i] = ret;
	    }
	}
    }

    for (c = head; c != NULL; c = c->c_next)
    {
	if (c->c_ord > L_NONE)
	{
	    if (c->c_ord != L_ACTIVE && equiv_value[c->c_ord] != E_NONE)
		c->c_ord = ord;
	    else
	    {
		c->c_ord = L_DEAD;

		if
		(
		    c->c_last != NULL
		    &&
		    (c->c_last->c_flags & (C_X_MOD | C_X_CMOD)) != 0
		    &&
		    c->c_last->c_what != ct_switch
		    &&
		    (c->c_last->c_flags & C_X_HIDE) == 0
		    &&
		    usage != 0
		)
		{
		    sync_back(c->c_last);

		    Ea[0].m = id->id_name;
		    Ea[1].m = (c->c_last->c_flags & C_X_MOD) == 0 ? "may not be" : "is not";

		    warn("value assigned to '%' % used");
		}
	    }
	}
    }
}

/*
 *	Determine the instance ordinals for an identifier.
 */
void
life_determine(id, head)
register ident	*id;
register cnode	*head;
{
    register cnode	*c;
    register cnode	*tail;
    register int	ord;
    register int	f;

    f = 0;

    for (c = head; c != NULL; c = c->c_next)
    {
	c->c_ord = L_NONE;
	c->c_flags = 0;

	if (in(expressions, c->c_what))
	{
	    c->c_flags |= flow_flags(c->c_flow, id);
	    f |= c->c_flags;
	}

	tail = c;
    }

    if (f == 0)
    {
	Ea[0].m = trace_argument ? "argument" : "variable";
	Ea[1].m = id->id_name;
	Ea[2].m = func.fn_name;

	warnt(id->id_token, "% '%' is not used in function '%'");
    }
    else if ((f & C_X_CUT) == 0 && !trace_argument)
    {
	if (is_member(id, eq_set))
	{
	    if (option(o_warnings))
	    {
		Ea[0].m = id->id_name;
		Ea[1].m = func.fn_name;

		warnt(id->id_token, "variable '%' may not be assigned a meaningful value in function '%'");
	    }
	}
	else
	{
	    Ea[0].m = (f & (C_X_MOD | C_X_CMOD)) == 0 ? "" : " pure";
	    Ea[1].m = id->id_name;
	    Ea[2].m = func.fn_name;

	    errort(id->id_token, "no% assignment to '%' in function '%'");
	}
    }
    else if ((f & (C_X_REF | C_X_HIDE)) == 0)
    {
	Ea[0].m = id->id_name;
	Ea[1].m = func.fn_name;

	warnt(id->id_token, "value of variable '%' is not used in function '%'");
    }
    else
    {
	trace_instance(id, head, tail, L_HEAD, f);

	ord = L_HEAD - 1;

	for (c = head; c != NULL; c = c->c_next)
	{
	    if (c->c_ord == L_NONE)
	    {
		trace_instance(id, c, tail, ord, f);
		ord--;
	    }
	}

	if (trace_argument && head->c_ord == L_DEAD)
	{
	    Ea[0].m = id->id_name;
	    Ea[1].m = func.fn_name;

	    warnt(id->id_token, "initial value of argument '%' is not used in function '%'");
	}

	for (c = head; c != NULL; c = c->c_next)
	{
	    if (c->c_ord >= L_NONE)
		internal("life determine", "missed a c->c_ord");
	    if (c->c_ord <= L_HEAD)
		c->c_ord = L_ORD(c->c_ord);
	}


	if (errored)
	    return;

	alloc_ids(tail, id, L_ORD(ord));
    }
}

/*
 *	Life time determination for an ident list.
 */
void
life_list(id, c)
register ident	*id;
register cnode	*c;
{
    while (id != NULL)
    {
	if (is_pure(id))
	    life_determine(id, c);

	id = id->id_memblist;
    }
}
