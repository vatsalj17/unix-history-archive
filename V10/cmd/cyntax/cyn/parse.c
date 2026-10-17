/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"
#include	"tokens.h"
#include	"cnodes.h"
#include	"types.h"
#include	"macdep.h"

cfrag	*autoi_cfrag;

cnode	*case_tree;
cnode	*break_label;
cnode	*cont_label;
cnode	*def_label;
cnode	*ujmp_thread;

ident	*label_thread;

int	in_switch;
int	levnum;

level	*levelp;

extern tkset	statement_anchors;

auto_initialise(id, x)
ident	*id;
xnode	*x;
{
    register cnode	*c;
    register xnode	*y;
    extern xnode	*fix_bin_type();

    y = new_xnode();

    y->x_right = x;
    y->x_left = new_xnode();
    y->x_left->x_what = xt_name;
    y->x_left->x_type = id->id_type;
    y->x_left->x_value.d = id;
    y->x_left->x_left = NULL;
    y->x_left->x_right = NULL;
    y->x_left->x_flags = XWAS_LVAL;
    y->x_what = xt_eq;
    y->x_flags = 0;

    if ((y = fix_bin_type(y)) == NULL)
	return;

    c = new_cnode();
    c->c_what = ct_xnd;
    c->c_value.x = y;
    c->c_next = NULL;

    if (autoi_cfrag == NULL)
    {
	autoi_cfrag = new_cfrag();
	autoi_cfrag->c_head = c;
    }
    else
	autoi_cfrag->c_tail->c_next = c;

    autoi_cfrag->c_tail = c;
}

xnode	*
block()
{
    register ident	*id;
    register token	*t;
    register cfrag	*c;
    register xnode	*x;
    ident		*mlist;
    static int		level_max;
    static level	*level_vect;

    if (levnum == level_max)
    {
	level_max += 16;
	level_vect = vector(level_vect, level_max, level);
    }

    levelp = &level_vect[levnum];
    levelp->lv_idents = NULL;
    levnum++;

    autoi_cfrag = NULL;
    mlist = declaration((ident *)NULL, in_block);
    c = autoi_cfrag;

    t = get_token();

    while (t->t_what != tk_offcurly && t->t_what != tk_eof)
    {
	register cfrag	*d;

	unget_token(t);

	if ((d = statement()) == NULL)
	{
	    token	*t2;

	    while (t = get_token(), !in(statement_anchors, t->t_what))
		free_token(t);

	    if (t->t_what == tk_oncurly)
		continue;

	    if ((t2 = get_token())->t_what == tk_else)
		free_token(t2);
	    else
		unget_token(t2);

	    if (t->t_what == tk_semicolon)
	    {
		free_token(t);
		t = get_token();
	    }
	}
	else
	{
	    if (c == NULL)
		c = d;
	    else
	    {
		ctack(c, d);
		free_cfrag(d);
	    }

	    t = get_token();
	}
    }

    if (t->t_what == tk_eof)
	errort(t, "insufficient '}'");

    /*
     *	Remove idents that still only have local significance
     *	from the symbol table.  Don't free them as they now hang
     *	off 'mlist' and will be dealt with anon.
     */
    for (id = levelp->lv_idents; id != NULL; id = id->id_levelp)
    {
	if (id->id_next != NULL)
	    id->id_next->id_last = id->id_last;

	*id->id_last = id->id_next;
    }

    levelp--;
    levnum--;

    x = new_xnode();
    x->x_what = xt_block;
    x->x_var = mlist;

    if (c == NULL)
    {
	x->x_value.k = new_cfrag();
	x->x_value.k->c_head = x->x_value.k->c_tail = new_cnode();
	x->x_value.k->c_head->c_what = ct_nop;
	x->x_value.k->c_head->c_next = NULL;
    }
    else
	x->x_value.k = c;

    return x;
}
