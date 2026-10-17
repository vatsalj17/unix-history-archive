#include	"mac.h"
#include	"mac.x"


/*
 *   Define constant handlers.
 */
pedc1()
{
	pedc(0);
	return;
}

pedc2()
{
	pedc(1);
	return;
}

pedc3()
{
	pedc(2);
	return;
}

pedc4()
{
	pedc(3);
	return;
}

pedc(n)
register int n;
{
	/*
	 *   Evaluate the expression (argument)
	 *   and stack it as arg 'a' for the
	 *   code formatter format().
	 */
	p = intercode.i_opr;
	oprstac[1] = expr();
	relstac[1] = reloc;

	format(&head.dctype[n], ERR);		/* format value in core */

						/* incr pc */
	locn[lcntr].l_value =+ head.dctype[n].f_len;
	return;
}




/*
 *   Special dc.
 */
pedcs()
{
	register char *form;
	register char *f;
	register int len;

	/*
	 *   Special dc format descr.
	 */
	form = &head.dctype[4];
	len  = head.dctype[4].f_len;

	f = intercode.i_opr;
	if (*f == '"')  {
		f++;
		length = 0;
		while (*f != '"')  {
			oprstac[1] = *f++;
			relstac[1] = ABS;
			format(form, ERR);
			length =+ len;
			}

		locn[lcntr].l_value =+ length;
		return;
		}

	p = intercode.i_opr;
	oprstac[1] = expr();
	relstac[1] = reloc;
	format(form, ERR);
	locn[lcntr].l_value =+ len;
	length = len;
	return;
}


/*
 *   Null routine - just return.
 */
penull()
{
	return;
}


/*
 *   Align location counter to an even multiple
 *   of the argument expression.
 */
pealign()
{
	register int v;
	register int l;

	p = intercode.i_opr;
	v = expr();
	if (v < 1)
		return;

	l = locn[lcntr].l_value;
	v = (l + v) % v;
	if (!v)				/* no align needed */
		return;

	/*
	 *   Increment pc and code assembly pointer
	 */
	locn[lcntr].l_value =+ v;
	locn[lcntr].l_next  =+ v;

	locn[lcntr].l_rel_n =+ v;

	return;
}


/*
 *   Set location counter origin.
 */
peorg()
{
	register int offset;
	register int v;

	p = intercode.i_opr;
	v = expr();

	/*
	 *   Restore curr. value and
	 *   code assembly pointer.
	 */
	offset = v - locn[lcntr].l_start;
	locn[lcntr].l_next  = locn[lcntr].l_rel_f + offset;
	locn[lcntr].l_value = v;

	locn[lcntr].l_rel_n = locn[lcntr].l_reloc + offset;

	return;
}


/*
 *   Set segment indicator.
 *   (already checked for validity)
 */
peseg()
{
	p = intercode.i_opr;
	lcntr = expr();

	return;
}


/*
 *   Define (n) basic addr. units of storage.
 */
peds()
{
	register int v;

	p = intercode.i_opr;
	v = expr();

	locn[lcntr].l_value =+ v;
	locn[lcntr].l_next  =+ v;

	*locn[lcntr].l_rel_n = RSTOR | (v & 0xff);
	locn[lcntr].l_rel_n =+ v;

	return;
}


/*
 *   end:  Set eof flag
 */
peend()
{
	eof = TRUE;
	return;
}


/*
 *   Set title info.
 */
petitle()
{
	register char *r;
	register char *s;
	register int i;

	r = intercode.i_opr;
	if (*r != '"')  {
		synerr("title not a string");
		return;
		}

	/*
	 *   Copy string into title buffer
	 */
	r++;
	s = ctitle;
	while (*r != '"')
		*s++ = *r++;
	*s = '\0';

	return;
}
