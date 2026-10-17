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
#include	"func.h"
#include	"parse.h"
#include	"options.h"

func_data	func;

/*
 *	'x' points to a block.   Gather it's local variables and return
 *	a pointer to the end of the list.
 */
static ident **
get_locals(x, dst)
register xnode	*x;
register ident	**dst;
{
    register cnode	*c;

    *dst = x->x_var;

    while (*dst != NULL)
	dst = &(*dst)->id_memblist;

    for (c = x->x_value.k->c_head; c != NULL; c = c->c_next)
    {
	if (c->c_what == ct_block)
	    dst = get_locals(c->c_value.x, dst);
    }

    return dst;
}

remove_ids(id)
register ident	*id;
{
    while (id != NULL)
    {
	if (id->id_next != NULL)
	    id->id_next->id_last = id->id_last;

	*id->id_last = id->id_next;

	id = id->id_memblist;
    }
}

function(id, t)
register ident	*id;
register token	*t;
{
    register xnode	*x;
    register cnode	*c;
    cfrag		cfrg;
    int			implicit_return;
    int			previous_errors;
    extern void		release_memory();

    c = new_cnode();

    c->c_what = ct_sync;
    c->c_value.no.no_name = t->t_filename;
    c->c_value.no.no_offset = t->t_lineno;

    if (func.fn_token != NULL)
	free_token(func.fn_token);

    func.fn_token = t;
    func.fn_name = id->id_name;
    func.fn_type = id->id_type->x_subtype;

    if (func.fn_ret_token != NULL)
    {
	free_token(func.fn_ret_token);
	func.fn_ret_token = NULL;
    }

    if (func.fn_vret_token != NULL)
    {
	free_token(func.fn_vret_token);
	func.fn_vret_token = NULL;
    }

    break_label = NULL;
    case_tree = NULL;
    cont_label = NULL;
    label_thread = NULL;
    ujmp_thread = NULL;

    previous_errors = errored;

    if
    (
        (x = block()) == NULL
	||
	satisfy_gotos(errored == previous_errors) != 0
	||
	errored
    )
	return;

    *get_locals(x, &func.fn_locals) = NULL;

    cfrg.c_tail = new_cnode();
    cfrg.c_tail->c_next = NULL;
    cfrg.c_tail->c_what = ct_ftnxit;

    c->c_next = x->x_value.k->c_head;
    x->x_value.k->c_head = c;

    *flatten(x->x_value.k->c_head, &cfrg.c_head) = cfrg.c_tail;
    implicit_return = jump_opt(&cfrg);

    if (func.fn_ret_token == NULL)
    {
	if (func.fn_type->x_what != xt_basetype)
	{
	    Ea[0].m = func.fn_name;
	    Ea[1].x = id->id_type;
	    Ea[2].t = func.fn_vret_token;
	    warnt(func.fn_token, "'%' declared as # but returns no value@");
	}
    }
    else if (func.fn_vret_token != NULL || implicit_return)
    {
	Ea[0].m = func.fn_name;

	if (func.fn_vret_token != NULL)
	{
	    Ea[1].t = func.fn_vret_token;
	    Ea[2].t = func.fn_ret_token;
	    warnt(func.fn_token, "function '%' has 'return;'@ and 'return e;'@");
	}
	else
	{
	    Ea[1].t = func.fn_ret_token;
	    warnt(func.fn_token, "function '%' has implicit return and 'return e;'@");
	}
    }

    analyse_flow(&cfrg);

    if (errored)
	return;

    if (option(o_optimise))
	alloc_locals(&cfrg);

    free_flow(&cfrg);


    if (errored)
	return;

    release_memory();
}
