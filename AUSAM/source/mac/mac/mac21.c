#include	"mac.h"
#include	"mac.x"

gocode(op)
register int op;
{
	register struct fd *fmt;
	register int nargs;
	register int i;
	register char *r;


	nargs = 0;
	p = intercode.i_opr;

	/*
	 *   Decode each argument expression and
	 *   evaluate it. Stack it's value for the
	 *   instruction formatter. Check the 
	 *   number of args is correct for the selected
	 *   format descriptor.
	 */
	for (;;)  {
		while (*p == ',')
			p++;
		if (!*p)
			break;
		oprstac[++nargs] = expr();
		relstac[  nargs] = reloc;
		}


	r = &opcode[0] + (op * head.h_o_len);
	if (intercode.i_selc[0] & SELOPC)
		/* select opcode column */
		op = intercode.i_selc[2];

	else
		/* select default */
		op = 0;

	if (intercode.i_selc[0] & SELFMT)
		/* select new format */
		fmt = &memory[intercode.i_selc[3]];
	else
		/* select default format */
		fmt = &memory[r->o_code[op].o_format];

	if (nargs != fmt->f_class)  {
		synerr("wrong # of args");
		return;
		}


	/*
	 *   Set instruction length into 'length'
	 *   so that listing rtns can do their job.
	 */

	length = fmt->f_len;

						/* pre incr pc */
	if (!head.h_pc_post)
		locn[lcntr].l_value =+ length;

	format(fmt, r->o_code[op].o_value);	/* format code into core */

	if (head.h_pc_post)			/* post incr pc */
		locn[lcntr].l_value =+ length;

	return;
}

gpcode(op)
register int op;
{
	/*
	 *   Look up pseudo routine addr in the address vector
	 *   and call it to actually process the pseudo.
	 *
	 *   Pseudo routines will fudge the location
	 *   counters if they wish.
	 */
	fn = peadr[op];
	(*fn)();

	return;
}
