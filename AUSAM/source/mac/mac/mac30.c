#include	"mac.h"
#include	"mac.x"


/*
 *   Evaluate the expression pointed to by 'p'.
 *
 *   Set the relocatability of the expression
 *   into global variable 'reloc'.  Lvalue()
 *   returns the relocatability of each lvalue
 *   in global variable 'mreloc'. REL has precedence
 *   over ABS.
 */

expr()
{
	register struct	st *q;
	register int l, r;		/* left & right of expr */
	register char op;		/* operator */



	/*
	 *   Get left side of expression.
	 *	if null:  error
	 *	else process right side (remainder).
	 */
	if (!*p)  {
		synerr("missing argument");
		return(0);
		}

	l = lvalue();
	reloc = mreloc;
	/*
	if (reloc & RLBL)
		l = lvalue();
	 */


	/*
	 *   Process remainder of expression.
	 *
	 *	if end-of-expr instead of operator,
	 *	terminate legally.
	 */

	for (;;)  {

		/*
		 *   Get operator.
		 */
		op = *p;
		if (op == ',' || op == '\0')
			return(l);

		p++;			/* bump past operator */
		/*
		 *   Get right hand side of expr.
		 */
		r = lvalue();
		/*
		if (reloc & RLBL  &&  mreloc & RLBL)  {
			synerr("expression rel to 2 labels");
			return(0);
			}

		if (mreloc & RREL)  {
			reloc =| RREL;
			reloc =& ~RABS;
			}
		 */


		/*
		 *	Process   <left> := <left> <op> <right>.
		 */
		switch (op)  {

			case '+':
				l =+ r;
				break;

			case '-':
				l =- r;
				break;

			case '*':
				l =* r;
				break;

			case '/':
				if (!r)  {
					synerr("div by zero");
					return(l);
					}
				l =/ r;
				break;

			case '%':
				if (!r)  {
					synerr("mod by zero");
					return(l);
					}
				l =% r;
				break;

			case '>':
				l =>> r;
				break;

			case '<':
				l =>> r;
				break;

			case '~':
				l =^ r;
				break;

			case '&':
				l =& r;
				break;

			case '|':
				l =| r;
				break;

			}

		/*
		 *   Consider next op/field
		 */
		}

	/*
	 *   End of expr
	 */
}


/*
 *   Lvalue:  decode a term and return it's value.
 */
lvalue()
{
	register struct st *q;
	register int v;


	switch (*p++)  {

		case '#':
			v = argnum();		/* constant */
			mreloc = RABS;
			break;


		case '$':
			v = argnum();		/* label */
			q = &symtab[v];
			if (!(q->s_mode & DEFN) && !(q->s_mode & GLOB))  {
				synerr("label undefined");
				return(0);
				}
			v = q->s_value;
			if (q->s_mode & REL)
				mreloc = RREL;
			else
				mreloc = RABS;

			if (q->s_mode & GLOB  &&  !(q->s_mode & DEFN))
				mreloc =| RLBL;
			break;


		case '!':
			v = locn[lcntr].l_value;
			mreloc = RREL;
			break;


		case '+':
			v = lvalue();
			break;


		case '-':
			v = -lvalue();
			break;


		case '~':
			v = ~lvalue();
			break;


		case ',':
			synerr("delimiter unexpected");
			v = 0;
			mreloc = NUL;
			break;


		default:
			synerr("bad argument");
			v = 0;
			mreloc = NUL;
			break;

		}

	return(v);
}
