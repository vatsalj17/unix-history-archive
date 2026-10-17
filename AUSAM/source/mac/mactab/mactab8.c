#include	"../mac/mac.h"
#include	"mactab.h"
#include	"mactab.x"



/*
 *   Argument of instruction decoder.
 *   This generates the parser for MAC.
 */
parg()
{
	register int ns2;
	register int i;

	treeinit();			/* set up dummy node (root) */
	while (getlin())  {
		p = buf;
		gentree();
		}

	if (!tree[0].n_alt)  {
		/* null root */
		error("no parser table generated !", 0);
		return;
		}


					/* cvt tree to linear array */
	ptable(tree[0].n_alt);
	ns2 = tp;			/* size of seg. 2 */


	/*
	 *   Apply various patches to the generated
	 *   table to make it really useable.
	 *
	 *   Symbol type MCH implies always match
	 *   on this parser symbol. It is so MAC
	 *   can do special actions at these times.
	 */

	for (i=0; i<ns2; i++)  switch (s2[i].tb_sym)  {

		case EOL:
			s2[i].tb_sym = MCH;
			s2[i].tb_act = SELC;		/* select options */
			s2[i].tb_mem = NUL;
			s2[i].tb_next = ns2;
			break;

		case ERR:
			s2[i].tb_sym = MCH;
			s2[i].tb_act = OERR;		/* parse error */
			break;

		case LIT:
			s2[i].tb_sym = -s2[i].tb_sym;
			s2[i].tb_act =  NOOP;
			break;

		case DEL:
		case CHR:
		case OPR:
			s2[i].tb_sym = -s2[i].tb_sym;
			s2[i].tb_act = ODEL;
			break;

		case STR:
			s2[i].tb_act = OSTR;
			break;

		case EXP:
			/*
			 *   Call expr parser. An error is
			 *   detectable on 1st symbol. If so -
			 *   the expr parser returns to next
			 *   state. If a good match - it returns
			 *   to the next tree state.
			 */
			s2[i].tb_sym = MCH;
			s2[i].tb_act = EXPR;
			break;


		default:
			/* don't do anything */
			s2[i].tb_act = NOOP;
			break;

		}

	/*
	 *   Relocate all four sections and combine into one.
	 */

	tp = 0;
	relocate(&e1[0], NE1);
	relocate(&s1[0], NS1);
	relocate(&s2[0], ns2);
	relocate(&s3[0], NS3);


	head.h_p_len = tp;
	head.h_p_start = NE1;
	return;
}

/*
 *   This routine was designed with the aid of the
 *   project supervisor - Mr. Richard Miller.
 *
 *   The author of MACTAB duly thanks him.
 *
 */
gentree()
{
	register struct node *q;
	register cc;

	q = &tree[0];

	do	{

		getsym();
		while (sym != q->n_sym || mem != q->n_mem)

			if (q->n_alt)
				q = q->n_alt;

			else  {
				q->n_alt = dslot();
				q = q->n_alt;
				while (sym != EOL)  {
					getsym();
					q->n_next = dslot();
					q = q->n_next;
					}

				}

			/*  */

		q = q->n_next;
		}	while (sym != EOL);

	/*  end of sectional build  */

	return;
}

/*
 *   Initialise root node for gentree();
 */
treeinit()
{
	sym = ERR;
	dslot();
	return;
}

/*
 *   Define a node in the tree.
 */
dslot()
{
	register int i;

	if (nslot+1 > NTREE - 1)  {
		error("parse table too large", 0);
		exit(1);
		}

	tree[nslot].n_sym = sym;
	tree[nslot].n_mem = mem;
	for (i=0; i<4; i++)
		tree[nslot].n_mem4[i] = mem4[i];
	tree[nslot].n_alt = NUL;
	tree[nslot].n_next = NUL;

	return(&tree[nslot++]);
}

/*
 *   Recursive descent routine to build a state table
 *   from the previously generated n-ary tree.
 */
ptable(q)
register struct node *q;
{
	register struct node *r;
	register int rtp;
	register int rltp;
	register int i;
	register struct node *m;
	register struct node *d;
	register int j;

	r = q;
	rtp = tp;

	/*
	 *   Scan list of 'alts' for an EOL.
	 *   For very subtle reasons - this MUST be
	 *   the last alt in the alt list.
	 */
	m = NUL;
	do	{
		if (q->n_sym == EOL)
			m = q;			/* remember where EOL is */

		d = q;				/* update end-of-alt-list ptr */
		q = q->n_alt;			/* move along alt list */

		}	while (q);

	if (m != NUL)  {
		/*
		 *   EOL found - swap entries *m and *d.
		 *   this will put EOL as last on list.
		 *   note - don't swap alt pointers !!!
		 */
		i = d->n_sym;
		d->n_sym = m->n_sym;
		m->n_sym = i;

		i = d->n_mem;
		d->n_mem = m->n_mem;
		m->n_mem = i;

		for (j=0; j<4; j++)  {
			i = d->n_mem4[j];
			d->n_mem4[j] = m->n_mem4[j];
			m->n_mem4[j] = i;
			}

		i = d->n_next;
		d->n_next = m->n_next;
		m->n_next = i;
		}


	q = r;			/* restore q to descend the list again */
	do	{
		s2[tp].tb_sym = q->n_sym;
		s2[tp].tb_mem = q->n_mem;
		for (j=0; j<4; j++)
			s2[tp].tb_arg[j] = q->n_mem4[j];

		tp++;
		q = q->n_alt;
		}	while (q);

	rltp = tp;			/* remember last Tree Pointer */
	s2[tp++].tb_sym = ERR;		/* stopping point */

	/*
	 *   Descend each alt,next level in turn to
	 *   build the full tree.
	 */
	for (i=rtp; i<rltp; i++)  {

		s2[i].tb_next = tp;
		if (r->n_next)
			ptable(r->n_next);

		r = r->n_alt;
		}

	return;
}

relocate(t, n)
register struct tbl *t;
register int n;
{
	register struct tbl *ep;
	register int rtp;
	register int nxt;
	register int i;


	rtp = tp;
	ep = &parse[rtp];
	tp =+ n;

	while (n-- > 0)  {
		ep->tb_sym = t->tb_sym;
		ep->tb_mem = t->tb_mem;
		nxt = t->tb_next;
		ep->tb_next = (nxt == 0 ? 0 : nxt + rtp);
		ep->tb_act = t->tb_act;
		for (i=0; i<4; i++)
			ep->tb_arg[i] = t->tb_arg[i];

		ep++;
		t++;
		}

	return;
}
