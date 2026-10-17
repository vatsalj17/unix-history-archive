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
#include	"func.h"
#include	"xtypes.h"
#include	"parse.h"

extern int	comp_constant();
extern xnode	*extract_field();
extern xnode	*test_expr();

extern tkset	statement_anchors;

extern tkset	case_anchors;
extern tkset	compound_anchors;
extern tkset	factor_heads;

/*
 *	Parse and generate code for a statement. Loop around picking up
 *	as many labels or cases as you can find. Return a code frag when
 *	you have a complete function. Return NULL on error, but only
 *	after identifying it.
 */
cfrag	*
statement()
{
    register token	*t;
    register cfrag	*k;
    register cnode	*c;
    int			discarded;

    /*
     *	'k' points to the code_frag which we will eventually return.
     *	'c' points to the cnode that we are currently working on.
     */
    k = new_cfrag();
    k->c_head = c = new_cnode();

    discarded = 0;

    loop
    {
	t = get_token();
	sync(t);

	c->c_what = ct_sync;
	c->c_value.no.no_name = t->t_filename;
	c->c_value.no.no_offset = t->t_lineno;

	c->c_next = new_cnode();
	c = c->c_next;

	switch (t->t_what)
	{
	case tk_oncurly:
	    /*
	     *	Block:
	     *
	     *	xnode 'xt_block' with code fragment
	     *	in x_value.k
	     */
	    free_token(t);

	    if ((c->c_value.x = block()) == NULL)
		return NULL;

	    c->c_what = ct_block;
	    break;

	case tk_switch:
	    {
		register cfrag	*j;
		register cnode	*L;
		register xnode	*x;
		register cnode	*break_save;
		register cnode	*case_save;
		register cnode	*def_save;
		register int	in_sw_sv;

		/*
		 *	Switch:
		 *
		 *	xnode with expression in x_left and
		 *	case_tree in x_value.c
		 */
		free_token(t);

		(void)accept(tk_onround, "'(' expected after 'switch'");

		c->c_what = ct_switch;
		c->c_value.x = new_xnode();

		/*
		 *	On detecting an error we
		 *	skip up to ';' or '{'.
		 */
		if ((c->c_value.x = x = expr(1)) == NULL)
		{
		    while (t = get_token(), !in(compound_anchors, t->t_what))
			free_token(t);

		    unget_token(t);
		}
		else
		{
		    /*
		     *	Type		Action
		     *	====		======
		     *	enum		cast to int
		     *	aggreg		illegal
		     *	float		illegal
		     *	integral	cast to int (warn if long)
		     *	bitfield	extract
		     */
		    if (x->x_type->x_what == xt_enum)
			c->c_value.x = cast(x, type_int);
		    else
		    {
			if (x->x_type->x_what == xt_bitfield)
			    x = extract_field(x);
			else if (x->x_type->x_what != xt_basetype || (x->x_type->x_value.i & FLOAT) != 0)
			{
			    Ea[0].x = x;
			    error("switch on type $");
			}
			else
			{
			    if ((x->x_type->x_value.i & LONG) != 0)
			    {
				Ea[0].x = x;
				error("switch on type $");
			    }

			    if (x->x_type != type_int)
				c->c_value.x = cast(x, type_int);
			}
		    }

		    if (accept(tk_offround, "')' expected at logical end of 'switch' expression"))
		    {
			while (t = get_token(), !(in(compound_anchors, t->t_what)))
			    free_token(t);

			unget_token(t);
		    }
		}

		L = new_cnode();
		L->c_what = ct_label;
		break_save = break_label;
		break_label = L;
		case_save = case_tree;
		in_sw_sv = in_switch;
		def_save = def_label;
		case_tree = NULL;
		in_switch = 1;
		def_label = NULL;

		if ((j = statement()) != NULL)
		{
		    cnode	*def;

		    c->c_choice = talloc(choice);
		    c->c_choice->ch_case = case_tree;
		    def = new_cnode();
		    c->c_choice->ch_def = def;
		    def->c_label = def_label != NULL ? def_label : L;
		    c->c_next = j->c_head;
		    c = j->c_tail;
		}

		c->c_next = L;
		c = L;

		break_label = break_save;
		case_tree = case_save;
		in_switch = in_sw_sv;
		def_label = def_save;

		if (j == NULL)
		    return NULL;

		free_cfrag(j);

		if (x == NULL)
		    return NULL;

		break;
	    }

	case tk_case:
	    {
		long		value;

		/*
		 *	Case:
		 *
		 *	a label, and a member of case_tree
		 */
		if (!in_switch)
		    errort(t, "case not in switch");
		else if (comp_constant(case_anchors, &value) == 0)
		{
		    register cnode	**n;
		    register cnode	*L;
		    register long	i;

		    n = &case_tree;

		    while (*n != NULL)
		    {
			if ((i = (*n)->c_value.i - value) == 0)
			{
			    error("duplicate case in switch");
			    goto had_err;
			}
		    /* !!! left lean */
			n = i > 0 ? &(*n)->c_left : &(*n)->c_right;
		    }

		    *n = L = new_cnode();
		    L->c_label = c;
		    L->c_left = NULL;
		    L->c_right = NULL;
		    L->c_value.i = value;
		}

	    had_err:

		free_token(t);

		(void)accept(tk_colon, "':' expected after case expression");

		if (!in_switch)
		    return NULL;

		c->c_what = ct_label;
		c->c_next = new_cnode();
		c = c->c_next;
		discarded = 0;
		continue;
	    }

	case tk_default:
	    /*
	     *	Default:
	     *
	     *	a label (def_label)
	     */
	    if (!in_switch)
		errort(t, "default not in switch");
	    else if (def_label != NULL)
		errort(t, "this switch already has a 'default'");
	    else
		def_label = c;

	    free_token(t);

	    (void)accept(tk_colon, "':' expected after default");

	    if (!in_switch)
		return NULL;

	    c->c_what = ct_label;
	    c->c_what = ct_label;
	    c->c_next = new_cnode();
	    c = c->c_next;
	    discarded = 0;
	    continue;

	case tk_return:
	{
	    register token	*u;

	    /*
	     *	Return:
	     *
	     *	void:		ct_vret
	     *	with value:	ct_ret with expression in
	     *			c_value.x
	     */
	    if ((u = get_token())->t_what == tk_semicolon)
	    {
		if (func.fn_vret_token == NULL)
		    func.fn_vret_token = t;
		else
		    free_token(t);

		free_token(u);
		c->c_what = ct_vret;
	    }
	    else
	    {
		if (func.fn_ret_token == NULL)
		    func.fn_ret_token = t;
		else
		    free_token(t);

		unget_token(u);
		c->c_what = ct_ret;

		if ((c->c_value.x = expr(1)) == NULL)
		    return NULL;

		if
		(
		    func.fn_type->x_what == xt_basetype
		    &&
		    (func.fn_type->x_value.i & VOID) != 0
		)
		{
		    error("void functions cannot return a value");
		    return NULL;
		}

		/*
		 *	If the type of the return value
		 *	is not the same as the type of
		 *	the function then try for:
		 *
		 *	Type		Action
		 *	====		======
		 *	enum		cast
		 *	basetype	cast
		 *	bitfield	extract
		 *	ptr		if constant zero => null
		 *
		 *	else illegal type combination.
		 */
		if (c->c_value.x->x_type != func.fn_type)
		{
		    register xnode	*wanted;

		    if (func.fn_type->x_what == xt_enum)
			wanted = type_int;
		    else if (func.fn_type->x_what == xt_basetype)
		    {
			if (func.fn_type->x_value.i & FLOAT)
			    wanted = type_double;
			else if (func.fn_type->x_value.i & LONG)
			    wanted = func.fn_type;
			else if (func.fn_type->x_value.i & UNSIGNED)
			    wanted = type_uint;
			else
			    wanted = type_int;

			if (c->c_value.x->x_type->x_what == xt_bitfield)
			    c->c_value.x = extract_field(c->c_value.x);
		    }
		    else
			wanted = NULL;
		    if
		    (
			wanted != NULL
			&&
			(
			    c->c_value.x->x_type->x_what == xt_enum
			    ||
			    c->c_value.x->x_type->x_what == xt_basetype
			)
		    )
		    {
			if (c->c_value.x->x_type != wanted)
			    c->c_value.x = cast(c->c_value.x, wanted);
		    }
		    else if
		    (
			func.fn_type->x_what == xt_ptrto
			&&
			null_ptr(c->c_value.x)
		    )
			c->c_value.x->x_type = func.fn_type;
		    else if (func.fn_type != c->c_value.x->x_type)
		    {
			Ea[0].m = func.fn_name;
			Ea[1].x = c->c_value.x;
			Ea[2].x = func.fn_type;

			error("function % returns $; should be #");
			return NULL;
		    }
		}

		(void)accept(tk_semicolon, "';' expected at logical end of 'return' statement");

	    }

	    break;
	}

	case tk_if:
	    {
		register cfrag	*j;
		register cnode	*L1;
		register cnode	*L2;

		/*
		 *	If:
		 *
		 *	. => x	(expression)
		 *	. => jmpf   L1
		 *	. => statement head
		 *	.    ...
		 *	. => statement tail
		 *
		 *   (else)
		 *   /	. => jmp    L2
		 *   |	. => label  L1
		 *   |	. => statement head
		 *   |	.    ...
		 *   |	. => statement tail
		 *   \	. => label  L2
		 *
		 *   (noelse)
		 *	. => label  L1
		 */
		free_token(t);

		(void)accept(tk_onround, "'(' expected after 'if'");

		if ((c->c_value.x = expr(1)) == NULL)
		    return NULL;
		else
		    (void)accept(tk_offround, "')' expected at logical end of 'if' expression");

		c->c_value.x = test_expr(c->c_value.x);

		L1 = new_cnode();
		L1->c_what = ct_label;
		c->c_what = ct_xndtest;
		cadd(c, ct_jmpf);
		c->c_value.c = L1;

		if ((j = statement()) == NULL)
		    return NULL;

		c->c_next = j->c_head;
		c = j->c_tail;
		free_cfrag(j);

		if ((t = get_token())->t_what == tk_else)
		{
		    free_token(t);
		    L2 = new_cnode();
		    L2->c_what = ct_label;
		    cadd(c, ct_jmp);
		    c->c_value.c = L2;
		    c->c_next = L1;
		    c = c->c_next;

		    if ((j = statement()) == NULL)
			return NULL;

		    c->c_next = j->c_head;
		    c = j->c_tail;
		    free_cfrag(j);
		    c->c_next = L2;
		    c = c->c_next;
		}
		else
		{
		    unget_token(t);
		    c->c_next = L1;
		    c = c->c_next;
		}
		break;
	    }

	case tk_while:
	    {
		register cnode	*L1;
		register cnode	*L2;
		register cnode	*break_save;
		register cnode	*cont_save;
		register cfrag	*j;

		/*
		 *	While:
		 *
		 *	. => label  L1		(continue label)
		 *	. => x	(expression)
		 *	. => jmpf   L2
		 *	. => statement head
		 *	.    ...
		 *	. => statement tail
		 *	. => jmp    L1
		 *	. => label  L2		(break label)
		 */
		free_token(t);
		L1 = c;
		c->c_what = ct_label;

		(void)accept(tk_onround, "'(' expected after 'while'");

		cadd(c, ct_xndtest);

		if ((c->c_value.x = expr(1)) == NULL)
		    return NULL;
		else if (accept(tk_offround, "')' expected at logical end of 'while' expression"))
			return NULL;

		c->c_value.x = test_expr(c->c_value.x);

		L2 = new_cnode();
		cadd(c, ct_jmpf);
		c->c_value.c = L2;
		break_save = break_label;
		cont_save = cont_label;
		break_label = L2;
		cont_label = L1;

		if ((j = statement()) == NULL)
		    return NULL;

		break_label = break_save;
		cont_label = cont_save;
		c->c_next = j->c_head;
		c = j->c_tail;
		free_cfrag(j);
		cadd(c, ct_jmp);
		c->c_value.c = L1;
		c->c_next = L2;
		c = L2;
		c->c_what = ct_label;
		break;
	    }

	case tk_do:
	    {
		register cnode	*L1;
		register cnode	*L2;
		register cfrag	*j;
		register cnode	*break_save;
		register cnode	*cont_save;

		/*
		 *	Do:
		 *
		 *	. => label  L1
		 *	. => statement head
		 *	.    ...
		 *	. => statement tail
		 *	. => label		(continue label)
		 *	. => x	(expression)
		 *	. => jmpt   L1
		 *	. => label  L2		(break label)
		 */
		free_token(t);
		c->c_what = ct_label;
		L1 = c;

		break_save = break_label;
		cont_save = cont_label;
		break_label = L2 = new_cnode();
		cont_label = new_cnode();

		if ((j = statement()) == NULL)
		{
		    break_label = break_save;
		    cont_label = cont_save;
		    return NULL;
		}

		c->c_next = j->c_head;
		c = j->c_tail;
		free_cfrag(j);
		c->c_next = cont_label;
		c = c->c_next;
		c->c_what = ct_label;

		break_label = break_save;
		cont_label = cont_save;

		if (accept(tk_while, "'while' expected in 'do' statement"))
		{
		    t = get_token();
		    if (t->t_what == tk_onround)
			free_token(t);
		    else
			unget_token(t);
		}
		else
		    (void)accept(tk_onround, "'(' expected after 'while'");

		cadd(c, ct_xndtest);

		if ((c->c_value.x = expr(1)) == NULL)
		    return NULL;

		c->c_value.x = test_expr(c->c_value.x);

		if (accept(tk_offround, "')' expected at logical end of 'while' expression"))
		{
		    t = get_token();

		    if (t->t_what == tk_semicolon)
			free_token(t);
		    else
			unget_token(t);

		    return NULL;
		}
		else
		    (void)accept(tk_semicolon, "';' expected at logical end of 'do' statement");

		cadd(c, ct_jmpt);
		c->c_value.c = L1;
		c->c_next = L2;
		c = L2;
		c->c_what = ct_label;
		break;
	    }

	case tk_for:
	    {
		register cnode	*L1;
		register cnode	*L2;
		register cnode	*L3;
		register xnode	*x;
		register cfrag	*j;
		register cnode	*break_save;
		register cnode	*cont_save;

		/*
		 *	For:
		 *
		 *	. => x	(init expr)  |	optional
		 *	. => label  L1
		 *	. => x	(test expr)  |	optional
		 *	. => jmpf   L2	     |
		 *	. => statement head
		 *	.    ...
		 *	. => statement tail
		 *	. => label  L3		(continue label)
		 *	. => x	(incr expr)  |	optional
		 *	. => jmp    L1
		 *	. => label  L2		(break label)
		 */
		free_token(t);

		(void)accept(tk_onround, "'(' expected after 'for'");

		if ((t = get_token())->t_what == tk_semicolon)
		{
		    c->c_what = ct_nop;
		    free_token(t);
		}
		else
		{
		    unget_token(t);
		    c->c_what = ct_xnd;

		    if ((c->c_value.x = expr(1)) == NULL)
			return NULL;

		    (void)accept(tk_semicolon, "';' expected in 'for'");
		}

		L1 = new_cnode();
		L2 = new_cnode();
		L3 = new_cnode();
		c->c_next = L1;
		c = L1;
		c->c_what = ct_label;

		if ((t = get_token())->t_what != tk_semicolon)
		{
		    unget_token(t);
		    cadd(c, ct_xndtest);

		    if ((c->c_value.x = expr(1)) == NULL)
			return NULL;

		    c->c_value.x = test_expr(c->c_value.x);

		    cadd(c, ct_jmpf);
		    c->c_value.c = L2;

		    (void)accept(tk_semicolon, "';' expected in 'for'");
		}
		else
		    free_token(t);

		if ((t = get_token())->t_what == tk_offround)
		{
		    x = NULL;
		    free_token(t);
		}
		else
		{
		    unget_token(t);

		    if ((x = expr(1)) == NULL)
			return NULL;

		    (void)accept(tk_offround, "')' expected in 'for'");
		}

		break_save = break_label;
		cont_save = cont_label;
		cont_label = L3;
		break_label = L2;

		if ((j = statement()) == NULL)
		    return NULL;

		c->c_next = j->c_head;
		c = j->c_tail;
		free_cfrag(j);
		c->c_next = L3;
		c = L3;
		c->c_what = ct_label;

		if (x != NULL)
		{
		    cadd(c, ct_xnd);
		    c->c_value.x = x;
		}

		cadd(c, ct_jmp);
		c->c_value.c = L1;
		c->c_next = L2;
		c = L2;
		c->c_what = ct_label;

		break_label = break_save;
		cont_label = cont_save;
		break;
	    }

	case tk_semicolon:
	    /*
	     *	Null statement: ct_nop
	     */
	    free_token(t);
	    c->c_what = ct_nop;
	    break;

	case tk_offcurly:
	    /*
	     *	We only get here when labels/cases
	     *	are hard up against a close curly.
	     */
	    errort(t, "statement expected before '}'");
	    unget_token(t);
	    return NULL;

	case tk_break:
	    /*
	     *	jmp	break_label
	     */
	    free_token(t);

	    if (break_label == NULL)
		errort(t, "illegal break");

	    if (accept(tk_semicolon, "semicolon expected after 'break'"))
		c->c_what = ct_nop;
	    else
	    {
		c->c_value.c = break_label;
		c->c_what = ct_jmp;
	    }
	    break;

	case tk_continue:
	    /*
	     *	jmp	contin_label
	     */
	    free_token(t);

	    if (cont_label == NULL)
		errort(t, "illegal continue");

	    if (accept(tk_semicolon, "semicolon expected after 'continue'"))
		c->c_what = ct_nop;
	    else
	    {
		c->c_value.c = cont_label;
		c->c_what = ct_jmp;
	    }

	    break;

	case tk_goto:
	    /*
	     *	Goto:	(puke)
	     *
	     *	jmp	Label
	     */
	    free_token(t);

	    if ((t = get_token())->t_what == tk_name)
	    {
		register ident	*id;
		register cnode	*L;

		if ((id = refind_ident(t->t_value.n, IDIS_UNDEF | IDIS_LABEL, (ident *)NULL)) != NULL)
		{
		    if (id->id_value.c == NULL)
		    {
			L = new_cnode();
			L->c_next = ujmp_thread;
			L->c_value.c = c;
			ujmp_thread = L;
			c->c_value.d = id;
			c->c_what = ct_ujmp;
		    }
		    else
		    {
			c->c_value.c = id->id_value.c;
			c->c_what = ct_jmp;
		    }
		}

		free_token(t);

		(void)accept(tk_semicolon, "';' expected at logical end of goto");
	    }
	    else
	    {
		errort(t, "label expected after 'goto'");
		unget_token(t);
	    }

	    break;

	case tk_eof:
	    errort(t, "premature end of file");
	    return NULL;

	case tk_name:
	    {
		register token	*t2;

		if ((t2 = get_token())->t_what == tk_colon)
		{
		    register ident	*id;

		    /*
		     *	Label:
		     *
		     *	ct_label linked via label thread
		     */
		    if ((id = refind_ident(t->t_value.n, IDIS_LABEL, (ident *)NULL)) != NULL)
		    {
			if (id->id_value.c != NULL)
			    errortf(t, "redefinition of label '%'", t->t_value.n->st_name);

			id->id_value.c = c;
			c->c_what = ct_label;
			c->c_next = new_cnode();
			c = c->c_next;
			free_token(t);
			free_token(t2);
		    }

		    discarded = 0;
		    continue;
		}

		unget_token(t2);
	    }

	    goto have_name;

	default:
	    /*
	     *	expression
	     */
	    if (!in(factor_heads, t->t_what))
	    {
		if (!discarded)
		{
		    errort(t, "statement syntax error");
		    discarded = 1;
		}

		continue;
	    }

	have_name:

	    unget_token(t);

	    if ((c->c_value.x = expr(1)) == NULL)
		return NULL;

	    c->c_what = ct_xnd;

	    (void)accept(tk_semicolon, "';' expected at logical end of expression");
	}

	c->c_next = NULL;
	k->c_tail = c;
	return k;
    }
}

/*
 *	Satisfy gotos:
 *
 *	Runs along the 'undefined jump' thread and satisfies the gotos
 *	to later declared labels.  Then remove the label thread.
 *
 *	Complain about undefined labels if 'moan' is true.
 */
int
satisfy_gotos(moan)
int	moan;
{
    register cnode	*c;
    register cnode	*d;
    register ident	*id;
    int		erred		= 0;

    for (c = ujmp_thread; c != NULL; c = d)
    {
	if (c->c_value.c->c_value.d->id_value.c == NULL)
	{
	    c->c_value.c->c_value.d->id_value.c = c;

	    if (moan)
	    {
		Ea[0].m = c->c_value.c->c_value.d->id_name;
		Ea[1].m = func.fn_name;
		error("label % undefined in function %");
	    }
	}
	else
	{
	    c->c_value.c->c_value.c = c->c_value.c->c_value.d->id_value.c;
	    c->c_value.c->c_what = ct_jmp;
	}

	d = c->c_next;
	free_cnode(c);
    }

    for (id = label_thread; id != NULL; id = id->id_levelp)
    {
	if (id->id_next != NULL)
	    id->id_next->id_last = id->id_last;

	*id->id_last = id->id_next;
	/*
	free((char *)id);
	*/
    }

    return erred;
}

/*
 *	Test that the test expression is basetype or pointer.
 */
xnode	*
test_expr(x)
xnode	*x;
{
    switch (x->x_type->x_what)
    {
    case xt_bitfield:
	return extract_field(x);

    case xt_basetype:
	if ((x->x_type->x_value.i & VOID) != 0)
	{
	    Ea[0].x = x;
	    error("test expression is $");
	}

    case xt_ptrto:
    case xt_enum:
	break;

    default:
	Ea[0].x = x;
	error("test expression is $");
    }

    return x;
}

int
accept(w, s)
tokens	w;
char	*s;
{
    register token	*t;

    if ((t = get_token())->t_what == w)
    {
	free_token(t);
	return 0;
    }
    else
    {
	errort(t, s);
	unget_token(t);
	return 1;
    }
}
