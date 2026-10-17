/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"
#include	"cnodes.h"
#include	"tokens.h"

/*
 *	Machine independant jump and dead-code optimisations.
 */

static int	j_changed;
static cnode	*cur_switch;

extern cnset	executables;

/*
 *	Expand test expressions.
 */
static void
expand_test(k)
cfrag	*k;
{
    register cnode	*c;
    register cnode	*d;
    register xnode	*x;
    extern xtset	base_constants;

    c = k->c_head;

    while (c != NULL)
    {
	if (c->c_what == ct_xndtest)
	{
	    if (c->c_next->c_what != ct_jmpt && c->c_next->c_what != ct_jmpf)
		internal("expand_test", "bad jump");
	    switch (c->c_value.x->x_what)
	    {
	    case xt_not:
		x = c->c_value.x;
		c->c_value.x = x->x_left;
		free_xnode(x);

		if (c->c_next->c_what == ct_jmpt)
		    c->c_next->c_what = ct_jmpf;
		else
		    c->c_next->c_what = ct_jmpt;

		continue;

	    case xt_andand:
		if (c->c_next->c_what == ct_jmpt)
		{
		    /*
		     *	a && b; jmpt L
		     *
		     *	a; jmpf T;
		     *	b; jmpt L;
		     *	T:
		     */
		    d = new_cnode();
		    d->c_what = ct_label;
		    d->c_next = c->c_next->c_next;
		    c->c_next->c_next = d;

		    d = new_cnode();
		    d->c_what = ct_xndtest;
		    d->c_next = c->c_next;
		    c->c_next = d;

		    x = c->c_value.x;
		    c->c_value.x = x->x_left;
		    d->c_value.x = x->x_right;
		    free_xnode(x);

		    d = new_cnode();
		    d->c_what = ct_jmpf;
		    d->c_value.c = c->c_next->c_next->c_next;
		    d->c_next = c->c_next;
		    c->c_next = d;
		}
		else
		{
		    /*
		     *	a && b; jmpf L
		     *
		     *	a; jmpf L;
		     *	b; jmpf L;
		     */
		    d = new_cnode();
		    d->c_what = ct_xndtest;
		    d->c_next = c->c_next;
		    c->c_next = d;

		    x = c->c_value.x;
		    c->c_value.x = x->x_left;
		    d->c_value.x = x->x_right;
		    free_xnode(x);

		    d = new_cnode();
		    d->c_what = ct_jmpf;
		    d->c_value.c = c->c_next->c_next->c_value.c;
		    d->c_next = c->c_next;
		    c->c_next = d;
		}

		continue;

	    case xt_barbar:
		if (c->c_next->c_what == ct_jmpt)
		{
		    /*
		     *	a || b; jmpt L
		     *
		     *	a; jmpt L;
		     *	b; jmpt L;
		     */
		    d = new_cnode();
		    d->c_what = ct_xndtest;
		    d->c_next = c->c_next;
		    c->c_next = d;

		    x = c->c_value.x;
		    c->c_value.x = x->x_left;
		    d->c_value.x = x->x_right;
		    free_xnode(x);

		    d = new_cnode();
		    d->c_what = ct_jmpt;
		    d->c_value.c = c->c_next->c_next->c_value.c;
		    d->c_next = c->c_next;
		    c->c_next = d;
		}
		else
		{
		    /*
		     *	a || b; jmpf L
		     *
		     *	a; jmpt T;
		     *	b; jmpf L;
		     *	T:
		     */
		    d = new_cnode();
		    d->c_what = ct_label;
		    d->c_next = c->c_next->c_next;
		    c->c_next->c_next = d;

		    d = new_cnode();
		    d->c_what = ct_xndtest;
		    d->c_next = c->c_next;
		    c->c_next = d;

		    x = c->c_value.x;
		    c->c_value.x = x->x_left;
		    d->c_value.x = x->x_right;
		    free_xnode(x);

		    d = new_cnode();
		    d->c_what = ct_jmpt;
		    d->c_value.c = c->c_next->c_next->c_next;
		    d->c_next = c->c_next;
		    c->c_next = d;
		}

		continue;

	    default:
		if (in(base_constants, c->c_value.x->x_what))
		{
		    x = c->c_value.x;

		    if
		    (
			x->x_what == xt_cdouble
			?
			(
			    x->x_value.f == 0.0
			    ?
			    c->c_next->c_what == ct_jmpt
			    :
			    c->c_next->c_what == ct_jmpf
			)
			:
			(
			    x->x_value.i == 0
			    ?
			    c->c_next->c_what == ct_jmpt
			    :
			    c->c_next->c_what == ct_jmpf
			)
		    )
			c->c_next->c_what = ct_nop;
		    else
			c->c_next->c_what = ct_jmp;

		    c->c_what = ct_nop;
		    free_xnode(x);
		}
	    }
	}

	c = c->c_next;
    }
}

/*
 *	Change all references of old_label to new_label.
 */
static void
change_dest(old_label, new_label)
cnode		*old_label;
register cnode	*new_label;
{
    register cnode	*c;
    register cnode	*d;

    for (c = old_label->c_value.c; c != NULL; c = d)
    {
	if ((c->c_flags & C_SWITCH) != 0)
	    c->c_label = new_label;
	else
	    c->c_value.c = new_label;

	d = c->c_link;
	c->c_link = new_label->c_value.c;
	new_label->c_value.c = c;
    }
}

/*
 *	Link case labels into label threads.
 */
static void
link_case(c)
register cnode	*c;
{
    while (c != NULL)
    {
	link_case(c->c_left);

	c->c_flags = C_SWITCH;
	c->c_link = c->c_label->c_value.c;
	c->c_label->c_value.c = c;
	c->c_switch = cur_switch;

	c = c->c_right;
    }
}

/*
 *	Label optimisation.  Delete unreferenced labels and merge adjacent
 *	labels.  This is achieved by making a thread through the code starting
 *	at a label and visiting all references to it.  The only complication
 *	is switches where c_label is used rather than c_value.c.
 */
static void
label_opt(k)
cfrag	*k;
{
    register cnode	*c;
    register cnode	**n;

    for (c = k->c_head; c != NULL; c = c->c_next)
    {
        if (c->c_what == ct_label)
	    c->c_value.c = NULL;

	c->c_flags = 0;
    }

    for (c = k->c_head; c != NULL; c = c->c_next)
    {
	switch (c->c_what)
	{
	case ct_jmp:
	case ct_jmpf:
	case ct_jmpt:
	    /*
	     *	Add to labels list.
	     */
	    c->c_link = c->c_value.c->c_value.c;
	    c->c_value.c->c_value.c = c;
	    break;

	case ct_switch:
	    /*
	     *	Traverse tree and link default.
	     */
	    cur_switch = c;
	    link_case(c->c_choice->ch_case);

	    {
		register cnode	*d;

		d = c->c_choice->ch_def;
		d->c_flags = C_SWITCH;
		d->c_link = d->c_label->c_value.c;
		d->c_label->c_value.c = d;
		d->c_switch = c;
	    }
	}
    }

    n = &k->c_head;

    while (*n != NULL)
    {
	if ((*n)->c_what == ct_label)
	{
	    c = *n;

	    loop
	    {
		/*
		 *	Combine adjacent labels.
		 */
		while ((c = c->c_next)->c_what == ct_sync)
		    ;

		if (c->c_what != ct_label)
		    break;

		change_dest(c, *n);
		j_changed++;
		c->c_what = ct_nop;
	    }

	    if ((*n)->c_value.c == NULL)
	    {
		/*
		 *	Remove unreferenced label.
		 */
		c = *n;
		*n = c->c_next;
		free_cnode(c);
		j_changed++;
		continue;
	    }
	}
	else if ((*n)->c_what == ct_nop)
	{
	    /*
	     *	Remove obsolete label.
	     */
	    c = *n;
	    *n = c->c_next;
	    free_cnode(c);
	    j_changed++;
	    continue;
	}

	n = &(*n)->c_next;
    }
}

#define	JMP	0
#define	JMPT	1
#define	JMPF	2

#define	duple(j1, j2)	(3 * j1 + j2)

static void
jump_to_jump(k)
cfrag	*k;
{
    register cnode	*c;
    register cnode	*d;
    register cnode	*e;
    register int	src;
    register int	dst;

    c = k->c_head;

    while (c != NULL)
    {
	switch (c->c_what)
	{
	case ct_jmp:
	    src = JMP;
	    break;

	case ct_jmpt:
	    src = JMPT;
	    break;

	case ct_jmpf:
	    src = JMPF;
	    break;

	default:
	    c = c->c_next;
	    continue;
	}

	for (e = c->c_value.c->c_next; e->c_what == ct_sync; e = e->c_next)
	    ;

	if (e == c)
	{
	    c = c->c_next;
	    continue;
	}

	switch (e->c_what)
	{
	case ct_jmp:
	    dst = JMP;
	    break;

	case ct_jmpt:
	    dst = JMPT;
	    break;

	case ct_jmpf:
	    dst = JMPF;
	    break;

	default:
	    c = c->c_next;
	    continue;
	}

	switch (duple(src, dst))
	{
	case duple(JMP, JMP):
	case duple(JMPT, JMP):
	case duple(JMPT, JMPT):
	case duple(JMPF, JMP):
	case duple(JMPF, JMPF):
	    /*
	     *	Jump to sympathetic jump, replace src destination
	     *	with dest destination.
	     */
	    if (c->c_value.c == e->c_value.c)
	    {
		c = c->c_next;
		continue;
	    }

	    c->c_value.c = e->c_value.c;
	    break;

	case duple(JMP, JMPT):
	case duple(JMP, JMPF):
	    /*
	     *	Jump to jump[tf], replace src node by copy with dest node,
	     *	add jump to label after dest after src.
	     */
	    d = new_cnode();
	    d->c_next = e->c_next;
	    e->c_next = d;
	    d->c_what = ct_label;
	    d = new_cnode();
	    d->c_next = c->c_next;
	    c->c_next = d;
	    d->c_what = ct_jmp;
	    d->c_value.c = e->c_next;
	    c->c_what = e->c_what;
	    c->c_value.c = e->c_value.c;
	    break;

	case duple(JMPT, JMPF):
	case duple(JMPF, JMPT):
	    /*
	     *	Jump to hostile jump, replace src destination
	     *	with label after dest.
	     */
	    d = new_cnode();
	    d->c_next = e->c_next;
	    e->c_next = d;
	    d->c_what = ct_label;
	    c->c_value.c = d;
	    break;

	default:
	    internal("jump_to_jump", "bad duple");
	}

	j_changed++;
    }
}

/*
 *	Eliminate redundant branches.  A set of jumps is passed.
 *	This can also be called by the macdep optimiser after translit.
 */
void
branch_to_dot(k, jset)
cfrag		*k;
register setptr	jset;
{
    register cnode	**n;

    n = &k->c_head;

    while (*n != NULL)
    {
	if (in(jset, (*n)->c_what))
	{
	    register cnode	*c;

	    for (c = (*n)->c_next; c->c_what == ct_sync; c = c->c_next)
		;

	    if (c == (*n)->c_value.c)
	    {
		c = *n;
		*n = c->c_next;
		free_cnode(c);
		j_changed++;
		continue;
	    }
	}

	n = &(*n)->c_next;
    }
}


static void	trace_control_flow();

/*
 *	Trace the case tree.
 */
static void
trace_control_tree(c)
register cnode	*c;
{
    while (c != NULL)
    {
	trace_control_tree(c->c_left);
	trace_control_flow(c->c_label);

	c = c->c_right;
    }
}

/*
 *	Trace the possible flow of control from c.
 */	
static void
trace_control_flow(c)
register cnode	*c;
{
    while (c != NULL && !c->c_known)
    {
	c->c_known = 1;

	switch (c->c_what)
	{
	case ct_switch:
	    trace_control_tree(c->c_choice->ch_case);
	    trace_control_flow(c->c_choice->ch_def->c_label);
	    return;

	case ct_jmp:
	    c = c->c_value.c;
	    continue;

	case ct_jmpf:
	case ct_jmpt:
	    trace_control_flow(c->c_value.c);
	    break;

	case ct_ret:
	case ct_vret:
	    return;
	}

	c = c->c_next;
    }
}

static void
dead_code(k)
cfrag	*k;
{
    register cnode	*c;
    register cnode	**n;

    for (c = k->c_head; c != NULL; c = c->c_next)
	c->c_known = 0;

    trace_control_flow(k->c_head);

    for (n = &k->c_head; *n != NULL; n = &(*n)->c_next)
    {
	if (!(*n)->c_known)
	{
	    register int	have_tok;
	    token		tok;

	    have_tok = 0;

	    while ((c = *n)->c_what != ct_ftnxit && !c->c_known)
	    {
		*n = c->c_next;
		j_changed++;

		switch (have_tok)
		{
		case 0:
		    if (c->c_what == ct_sync)
		    {
			tok.t_filename = c->c_value.no.no_name;
			tok.t_lineno = c->c_value.no.no_offset;
			have_tok++;
		    }

		    break;

		case 1:
		    if (in(executables, c->c_what))
		    {
			warnt(&tok, "unreachable code");
			have_tok++;
		    }
		}

		free_cnode(c);
	    }
	}
    }
}

/*
 *	Machine independant branch cnodes.
 */
extern cnset	mip_branches;

/*
 *	Machine independant dead ends.
 */
extern cnset	mip_dead_ends;

/*
 *	Machine independant jump optimisation driver routine.
 *	The 'function exit reached' result of 'dead_code' is returned.
 */
int
jump_opt(k)
register cfrag	*k;
{
    expand_test(k);

    do
    {
	j_changed = 0;
	label_opt(k);
	jump_to_jump(k);
	branch_to_dot(k, mip_branches);
	dead_code(k);
    }
    while (j_changed);

    if (k->c_tail->c_what != ct_ftnxit)
	internal("jump_opt", "ftnxit eliminated");

    return k->c_tail->c_known;
}
