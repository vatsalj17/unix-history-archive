#include	"mac.h"
#include	"mac.x"

/*
 *   Standard linear search for pseudo op.
 */
pscan()
{
	register int i;
	register char *t;

	t = clabel;
	if (*t == 'd' && *(t+1) == 'c' && *(t+2) && !*(t+3))  {

		/*
		 *   Special define constant (dc?)
		 */
		for (i=0; i<4; i++)
			if (*(t+2) == head.dctype[i].f_class)
				return(i);
		return(ERR);
		}

	/*
	 *   Scan remaining pseudo ops.
	 */
	for (i=4; plabel[i]; i++)  {
		if (compar(clabel, plabel[i]))
			return(i);
		}

	return(ERR);
}


/*
 *   Binary search of opcode table.
 */
oscan()
{
	register char *r;			/* pointer to opcodes */
	register int i;
	register int j;
	register int k;

	i = 0;
	j = head.h_ops - 1;
	while (i <= j)  {
		k = (i + j) >> 1;
		r = &opcode[0] + (k * head.h_o_len);
		switch (cmp(clabel, r))  {

			case  1:
				i = k + 1;
				break;

			case  0:
				/*
				 *   found symbolic.
				 */
				return(k);

			case -1:
				j = k - 1;
				break;

			}
		}
	/*
	 *   Not found.
	 */
	return(ERR);
}

/*
 *   Hashed list search for a label in the symbol
 *   table. options exist to define a label, look
 *   up a label (referenced) or check for label in
 *   the table currently.
 */
lscan(name, mode, c1, value, type)
register char	*name;
int	 mode;
register int	c1;
register int	value;
register int	type;
{
	register struct st *q;

	switch (mode)  {

		default:
			printf("internal error mode %d\n", mode);
			return(ERR);

		/*
		 *   Scan symbol table:
		 *	if label is present, return index,
		 *	if not - define a slot in the table,
		 *	mark the symbol as referenced but
		 *	undefined, and return the index.
		 */
		case LKP:
			q = fchar[c1];
			if (!q)  {
				q = dslot(name, value, REFR);
				fchar[c1] = q;
				return(q - symtab);
				}

			/*
			 *   Search list.
			 */
			while (q)  {
				if (compar(name, q->s_name))  {
					q->s_mode =| REFR;
					return(q - symtab);
					}
				q = q->s_next;
				}

			/*
			 *   Not in list.
			 */
			q = dslot(name, value, REFR);
			q->s_next = fchar[c1];
			fchar[c1] = q;
			return(q - symtab);

		/*
		 *   Scan symbol table:
		 *	if symbol is not in table, define
		 *	an entry for it, and return the index.
		 *	if it is present, mark it defined,
		 *	and enter its value (error if previously
		 *	defined) and return its index.
		 */
		case DEF:
			q = fchar[c1];
			if (!q)  {
				q = dslot(name, value, type|DEFN);
				fchar[c1] = q;
				return(q - symtab);
				}
			/*
			 *   Search list - error if dup. found.
			 */
			while (q)  {
				if (compar(name, q->s_name))  {
					if (q->s_mode & DEFN)  {
						synerr("multi def. label");
						return(ERR);
						}
					q->s_mode =| type|DEFN;
					q->s_value =  locn[lcntr].l_value;
					return(q - symtab);
					}
				q = q->s_next;
				}
			/*
			 *   Add label definition.
			 */
			q = dslot(name, value, type|DEFN);
			q->s_next = fchar[c1];
			fchar[c1] = q;
			return(q - symtab);

		}


	/*
	 *   End of scanner.
	 */
}
