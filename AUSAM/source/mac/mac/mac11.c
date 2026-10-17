#include	"mac.h"
#include	"mac.x"


/*
 *   Define constant routines.
 */
prdc1()
{
	prdc(0);
	return;
}

prdc2()
{
	prdc(1);
	return;
}

prdc3()
{
	prdc(2);
	return;
}

prdc4()
{
	prdc(3);
	return;
}

prdc(n)
register int n;
{
	p = intercode.i_opr;
	if (!*p)  {
		synerr("expression required");
		return;
		}

	/*
	 *   Bump pc up by the length of the constant.
	 */
	locn[lcntr].l_value =+ head.dctype[n].f_len;

	return;
}



/*
 *   Special dc.
 *	Can define strings or constants.
 */
prdcs()
{
	register struct fd *f;
	register int len;

	/*
	 *   Allow a string, or a single expr.
	 */

	f = &head.dctype[4];
	if (!f->f_len)  {
		/* dc has not been declared in the r-file */
		synerr("dc not allowed");
		return;
		}

	p = intercode.i_opr;

	if (*p == '"')  {
		p++;
		len = 0;
		while (*p++ != '"')
			len++;

		locn[lcntr].l_value =+ (len * f->f_len);
		return;
		}

	/*
	 *   Expr.
	 */

	locn[lcntr].l_value =+ f->f_len;
	return;
}



/*
 *   Null routine - just return.
 */
prnull()
{
	return;
}



/*
 *   Equate a label to a value. (defined expression)
 *   A check has already been made for a re-defined
 *   label tag.
 */
prequ()
{
	register struct	st *q;
	register int l;

	p = intercode.i_opr;		/* operand field */
	l = intercode.i_label;
	if (l == ERR)  {
		synerr("label tag required");
		return;
		}

	q = &symtab[l];
	q->s_value = expr();
	q->s_mode =| reloc;

	return;
}


/*
 *   Turn on global indicator.
 *   Useful only for loader format output.
 */
prglobal()
{
	register struct	st *q;
	register int i;

	p = intercode.i_opr;
	if (*p++ != '$')  {
		synerr("label required");
		return;
		}

	i = argnum();
	q = &symtab[i];
	q->s_mode =| GLOB;

	return;
}


/*
 *   Align pc to an even multiple of expr().
 *   If already at the boundary - no align done.
 */
pralign()
{
	register int v;
	register int l;

	p = intercode.i_opr;
	v = expr();
	if (v < 1)
		return;
	l = locn[lcntr].l_value;
	v = (l + v) % v;
	if (!v)
		/* no align needed */
		return;

	locn[lcntr].l_value =+ v;
	return;
}


/*
 *   Set origin of pc
 */
prorg()
{
	register int v;
	register int l;

	p = intercode.i_opr;
	v = expr();
	if (v < 0)  {
		synerr("negative org");
		return;
		}

	/*
	 *   Check l_limit:
	 *	if backwards org, check that l_limit
	 *	is max(l_limit, l_value).
	 *
	 *	This is so that we know the overall
	 *	size of a code segment for pass 2.
	 *
	 *   Check l_start:
	 *	if no code generated yet - set start addr of
	 *	code to v;
	 *	else ensure l_start = min(l_start, v);
	 *
	 *   This will NOT work if the user orgs, generates
	 *	code, then orgs to zero and doesn't generate
	 *	any code.
	 */
	if (!locn[lcntr].l_value)
		/*
		 *   No code - initialise start addr
		 */
		locn[lcntr].l_start = v;
	else
		if (v < locn[lcntr].l_start)
			/*
			 *   Lower org than we have - remember it.
			 */
			locn[lcntr].l_start = v;

	/*
	 *   Check limit.
	 */
	l = locn[lcntr].l_value;
	if (v < l  &&  l > locn[lcntr].l_limit)
		locn[lcntr].l_limit = l;

	locn[lcntr].l_value = v;		/* org pc */
	return;
}


/*
 *   Set segment indicator
 */
prseg()
{
	register int v;
	register int l;

	p = intercode.i_opr;
	v = expr();
	if (v < 0 || v >= LCOUNT)  {
		synerr("no such location counter");
		return;
		}

	lcntr = v;
	return;
}


/*
 *   Reserve (n) bu's of null storage.
 */
prds()
{
	register int v;
	register int l;

	p = intercode.i_opr;
	v = expr();
	if (v < 0)  {
		synerr("negative ds");
		return;
		}

	locn[lcntr].l_value =+ v;
	return;
}


/*
 *   end:   Set eof flag, and check symbol table.
 *	    (not ref, undefined etc)
 */
prend()
{
	register struct	st *q;
	register int n;

	eof = TRUE;			/* set end-of-source flag */
	q = symtab;			/* start of symbol table */

	n = 0;
	while (q < coreptr)  {
		if (!(q->s_mode & DEFN)  &&  !(q->s_mode & GLOB))  {
			n++;
			printf("label %-8s is undefined\n", q->s_name);
			}

		q++;				/* next symbol */
		}

	if (n)
		synerr("Undefined labels");

	return;
}


/*
 *   Structure offset definition.
 */
prstruc()
{
	register struct st *q;
	register int l;

	p = intercode.i_opr;		/* operand field */
	l = intercode.i_label;

	if (l != ERR)  {
		/* Equate label to struct offset now */
		q = &symtab[l];
		q->s_value = nstruc;
		}

	nstruc =+ expr();

	return;
}



/*
 *   End structure offset counter.
 */
prends()
{
	register struct st *q;
	register int l;

	l = intercode.i_label;
	if (l != ERR)  {
		q = &symtab[l];
		q->s_value = nstruc;
		}

	/* Reset structure counter */
	nstruc = 0;
	return;
}
