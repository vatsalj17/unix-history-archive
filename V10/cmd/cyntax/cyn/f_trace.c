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
 *	Trace the case tree.
 */
void
trace_tree(c)
register cnode	*c;
{
    while (c != NULL)
    {
	trace_tree(c->c_left);
	trace_life(c->c_label);

	c = c->c_right;
    }
}

/*
 *	Trace case labels.
 */
void
trace_choice(ch)
register choice	*ch;
{
    trace_tree(ch->ch_case);
    trace_life(ch->ch_def->c_label);
}

/*
 *	Trace down a list of label refs.
 */
void
trace_link(l)
register cnode	*l;
{
    while (l != NULL)
    {
	/*
	 *	If C_SWITCH it is a fake node.  Follow
	 *	the link to the ct_switch node.
	 */
	if ((l->c_flags & C_SWITCH) != 0)
	{
	    register cnode	*s;

	    s = l->c_switch;

	    if ((s->c_flags & C_X_CUT) == 0 && s->c_ord == L_NONE)
	    {
		s->c_ord = L_ACTIVE;
		trace_back(s->c_last);
	    }

	    trace_choice(s->c_choice);
	}
	else
	    trace_life(l);

	l = l->c_link;
    }
}

/*
 *	Trace back from c.
 */
void
trace_back(c)
register cnode	*c;
{
    while (c != NULL && c->c_ord == L_NONE && !in(mip_dead_ends, c->c_what))
    {
	switch (c->c_what)
	{
	case ct_label:
	    /*
	     *	Mark the label and branch.
	     *	(When you're going up the labels branch.)
	     */
	    c->c_ord = L_ACTIVE;
	    trace_link(c->c_value.c);
	    break;

	case ct_jmpf:
	case ct_jmpt:
	    /*
	     *	Mark conditional jumps and trace their destinations.
	     */
	    c->c_ord = L_ACTIVE;
	    trace_life(c->c_value.c);
	    break;

	default:
	    if ((c->c_flags & C_X_CUT) != 0)
		return;

	    c->c_ord = L_ACTIVE;
	}

	c = c->c_last;
    }
}

/*
 *	Trace the life from c.  Trace the successors and trace back.
 */	
void
trace_life(c)
register cnode	*c;
{
    trace_back(c->c_last);

    while (c != NULL && c->c_ord == L_NONE)
    {
	c->c_ord = L_ACTIVE;

	switch (c->c_what)
	{
	case ct_label:
	    trace_link(c->c_value.c);
	    break;

	case ct_switch:
	    if ((c->c_flags & C_X_CUT) == 0)
		trace_choice(c->c_choice);

	    return;

	case ct_jmp:
	    c = c->c_value.c;
	    trace_back(c->c_last);
	    continue;

	case ct_jmpf:
	case ct_jmpt:
	    trace_life(c->c_value.c);
	    break;

	default:
	    if
	    (
		in(mip_dead_ends, c->c_what)
		||
		(c->c_flags & C_X_CUT) != 0
	    )
		return;
	}

	c = c->c_next;
    }
}
