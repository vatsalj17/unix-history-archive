# include	"../ingres.h"
# include	"../aux.h"
# include	"../symbol.h"
# include	"../tree.h"
# include	"decomp.h"




writenod(n)
struct querytree	*n;
{
	register int			t;
	register struct querytree	*q;
	register struct symbol		*s;
	char				*cp;
	int				l;

	q = n;
	s = &q->sym;
	t = s->type;
	l = s->len & I1MASK;

	printf("%l/ %l: %l: ", q, q->left, q->right);
	printf("%d, %d, ", t, l);

	switch (t)
	{
	  case VAR:
		printf("%d,%d,%d,%d,%l/", q->varno, q->attno, q->frmt, q->frml&0377, q->valptr);
		if (q->varno < 0) 
			writenod(q->valptr);
		else
			printf("\n");
		return;

	  case AND:
	  case ROOT:
	  case AGHEAD:
		printf("%d,%d,%o,%o", q->tvarc, q->lvarc, q->lvarm, q->rvarm);
		if (t != AND)
			printf(",(%d)", q->rootuser);
		break;

	  case AOP:
	  case RESDOM:
		printf("%d,%d,%d", q->resno, q->frmt, q->frml & 0377);
		if (t == AOP)
			printf("(%d,%d)", q->agfrmt, q->agfrml & 0377);
		break;

	  case UOP:
	  case BOP:
	  case COP:
	  case INT:
		switch (l)
		{
		  case 1:
		  case 2:
			printf("%d", s->value[0]);
			break;

		  case 4:
			printf("%s", locv(s->value->i4type));
		}
		break;

	  case FLOAT:
		printf("%.10f", s->value->f4type);
		break;

	  case CHAR:
		cp = s->value;
		while (l--)
			putchar(*cp++);
		break;

	  case TREE:
	  case OR:
	  case QLEND:
	  case BYHEAD:
		break;

	  default:
		printf("Error in writenod");
	}
	printf("/\n");
}
