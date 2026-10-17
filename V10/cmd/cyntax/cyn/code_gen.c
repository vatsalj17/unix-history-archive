/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"
#include	"cnodes.h"

extern xnodes	conv_assop();
extern xtset	assops;
extern xtset	binary_ops;
extern xtset	unary_ops;

extern xtset	yield_truth;


/*
 *	Flatten out the parse tree, but leave expressions as trees.
 */
cnode	**
flatten(src, dst)
register cnode	*src;
register cnode	**dst;
{
    register cnode	*c;

    while (src != NULL)
    {
	switch (src->c_what)
	{
	case ct_nop:
	    src = (c = src)->c_next;
	    free_cnode(c);
	    continue;

	case ct_block:
	    /*
	     *	Recurse on blocks.
	     */
	    dst = flatten(src->c_value.x->x_value.k->c_head, dst);
	    src = (c = src)->c_next;
	    free_cnode(c);
	    continue;

	case ct_ret:
	case ct_switch:
	case ct_xnd:
	case ct_xndtest:
	    src->c_value.x = traverse(src->c_value.x, 1);

	default:
	    /*
	     *	Pass through all other nodes.
	     */
	    *dst = src;
	    dst = &src->c_next;
	}
	src = src->c_next;
    }
    return dst;
}

