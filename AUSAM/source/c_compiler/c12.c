#
/*
 *		C compiler part 2 -- expression optimizer
 *
 */

#include "c1.h"

optim(atree)
struct tnode *atree;
{
	struct { int intx[4]; };
	register op, dope;
	int d1, d2;
	struct tnode *t;
	register struct tnode *tree;

	if ((tree=atree)==0)
		return(0);
	if ((op = tree->op)==0)
		return(tree);
	if (op==NAME && tree->class==AUTO) {
		tree->class = OFFS;
		tree->regno = 5;
		tree->offset = tree->nloc;
	}
	dope = opdope[op];
	if ((dope&LEAF) != 0) {
		if (op==FCON
		 && tree->fvalue.intx[1]==0
		 && tree->fvalue.intx[2]==0
		 && tree->fvalue.intx[3]==0) {
			tree->op = SFCON;
			tree->value = tree->fvalue.intx[0];
		}
		return(tree);
	}
	if ((dope&BINARY) == 0)
		return(unoptim(tree));
	/* is known to be binary */
	if (tree->type==CHAR)
		tree->type = INT;
	switch(op) {
	/*
	 * PDP-11 special:
	 * generate new =&~ operator out of =&
	 * by complementing the RHS.
	 */
	case ASAND:
		tree->op = ASANDN;
		tree->tr2 = tnode(COMPL, tree->tr2->type, tree->tr2);
		break;

	/*
	 * On the PDP-11, int->ptr via multiplication
	 * Longs are just truncated.
	 */
	case LTOP:
		tree->op = ITOP;
		tree->tr1 = unoptim(tnode(LTOI,INT,tree->tr1));
	case ITOP:
		tree->op = TIMES;
		break;

	case MINUS:
		if (t = isconstant(tree->tr2)) {
			tree->op = PLUS;
			if (t->type==DOUBLE)
				/* PDP-11 FP representation */
				t->value =^ 0100000;
			else
				t->value = -t->value;
		}
		break;
	}
	op = tree->op;
	dope = opdope[op];
	if (dope&LVALUE && tree->tr1->op==FSEL)
		return(lvfield(tree));
	if ((dope&COMMUTE)!=0) {
		d1 = tree->type;
		tree = acommute(tree);
		if (tree->op == op)
			tree->type = d1;
		/*
		 * PDP-11 special:
		 * replace a&b by a ANDN ~ b.
		 * This will be undone when in
		 * truth-value context.
		 */
		if (tree->op!=AND)
			return(tree);
		/*
		 * long & pos-int is simpler
		 */
		if (tree->type==LONG && tree->tr2->op==ITOL
		 && (tree->tr2->tr1->op==CON && tree->tr2->tr1->value>=0
		   || tree->tr2->tr1->type==UNSIGN)) {
			tree->type = UNSIGN;
			t = tree->tr2;
			tree->tr2 = tree->tr2->tr1;
			t->tr1 = tree;
			tree->tr1 = tnode(LTOI, UNSIGN, tree->tr1);
			return(optim(t));
		}
		tree->op = ANDN;
		tree->tr2 = tnode(COMPL, tree->tr2->type, tree->tr2);
	}
    again:
	tree->tr1 = optim(tree->tr1);
	tree->tr2 = optim(tree->tr2);
	if ((dope&RELAT) != 0) {
		if ((d1=degree(tree->tr1)) < (d2=degree(tree->tr2))
		 || d1==d2 && tree->tr1->op==NAME && tree->tr2->op!=NAME) {
			t = tree->tr1;
			tree->tr1 = tree->tr2;
			tree->tr2 = t;
			tree->op = maprel[op-EQUAL];
		}
		if (tree->tr1->type==CHAR && tree->tr2->op==CON
		 && (dcalc(tree->tr1) <= 12 || tree->tr1->op==STAR)
		 && tree->tr2->value <= 127 && tree->tr2->value >= 0)
			tree->tr2->type = CHAR;
	}
	d1 = max(degree(tree->tr1), islong(tree->type));
	d2 = max(degree(tree->tr2), 0);
	switch (op) {

	/*
	 * In assignment to fields, treat all-zero and all-1 specially.
	 */
	case FSELA:
		if (tree->tr2->op==CON && tree->tr2->value==0) {
			tree->op = ASAND;
			tree->tr2->value = ~tree->mask;
			return(optim(tree));
		}
		if (tree->tr2->op==CON && tree->mask==tree->tr2->value) {
			tree->op = ASOR;
			return(optim(tree));
		}

	case LTIMES:
	case LDIV:
	case LMOD:
	case LASTIMES:
	case LASDIV:
	case LASMOD:
		tree->degree = 10;
		break;

	case ANDN:
		if (isconstant(tree->tr2) && tree->tr2->value==0) {
			return(tree->tr1);
		}
		goto def;

	case CALL:
		tree->degree = 10;
		break;

	case QUEST:
	case COLON:
		tree->degree = max(d1, d2);
		break;

	case DIVIDE:
	case ASDIV:
	case ASTIMES:
	case PTOI:
		if (tree->tr2->op==CON && tree->tr2->value==1)
			return(tree->tr1);
	case MOD:
	case ASMOD:
		if (tree->tr1->type==UNSIGN && ispow2(tree))
			return(pow2(tree));
	case ULSH:
	case ASULSH:
		d1 =+ 2;
		d2 =+ 2;
		if (tree->type==LONG)
			return(hardlongs(tree));
		goto constant;

	case LSHIFT:
	case RSHIFT:
	case ASRSH:
	case ASLSH:
		if (tree->tr2->op==CON && tree->tr2->value==0) {
			return(tree->tr1);
		}
		/*
		 * PDP-11 special: turn right shifts into negative
		 * left shifts
		 */
		if (tree->type == LONG) {
			d1++;
			d2++;
		}
		if (op==LSHIFT||op==ASLSH)
			goto constant;
		if (tree->tr2->op==CON && tree->tr2->value==1
		 && tree->tr1->type!=UNSIGN)
			goto constant;
		op =+ (LSHIFT-RSHIFT);
		tree->op = op;
		tree->tr2 = tnode(NEG, tree->type, tree->tr2);
		if (tree->tr1->type==UNSIGN) {
			if (tree->op==LSHIFT)
				tree->op = ULSH;
			else if (tree->op==ASLSH)
				tree->op = ASULSH;
		}
		goto again;

	constant:
		if (tree->tr1->op==CON && tree->tr2->op==CON) {
			const(op, &tree->tr1->value, tree->tr2->value);
			return(tree->tr1);
		}


	def:
	default:
		tree->degree = d1==d2? d1+islong(tree->type): max(d1, d2);
		break;
	}
	return(tree);
}

unoptim(atree)
struct tnode *atree;
{
	struct { int intx[4]; };
	register struct tnode *subtre, *tree;
	register int *p;
	double static fv;
	struct { int integer; };
	struct ftconst *fp;

	if ((tree=atree)==0)
		return(0);
    again:
	subtre = tree->tr1 = optim(tree->tr1);
	switch (tree->op) {

	case ITOF:
		if (tree->tr1->type==UNSIGN) {
			tree->tr1 = unoptim(tnode(ITOL, LONG, tree->tr1));
			tree->op = LTOF;
		}
		break;

	case LTOI:
		p = tree->tr1;
		switch (p->op) {
		case NAME:
			p->offset =+ 2;
			p->type = tree->type;
			return(p);

		case STAR:
			p->type = tree->type;
			p->tr1->type = tree->type+PTR;
			p->tr1 = tnode(PLUS, tree->type, p->tr1, tconst(2));
			return(optim(p));

		case ITOL:
			return(p->tr1);

		case PLUS:
		case MINUS:
		case AND:
		case ANDN:
		case OR:
		case EXOR:
			p->tr2 = tnode(LTOI, tree->type, p->tr2);
		case NEG:
		case COMPL:
			p->tr1 = tnode(LTOI, tree->type, p->tr1);
			p->type = tree->type;
			return(optim(p));
		}
		break;

	case FSEL:
		tree->op = AND;
		tree->tr1 = tree->tr2->tr1;
		tree->tr2->tr1 = subtre;
		tree->tr2->op = RSHIFT;
		tree->tr1->value = (1 << tree->tr1->value) - 1;
		return(optim(tree));

	case FSELR:
		tree->op = LSHIFT;
		tree->type = UNSIGN;
		tree->tr1 = tree->tr2;
		tree->tr1->op = AND;
		tree->tr2 = tree->tr2->tr2;
		tree->tr1->tr2 = subtre;
		tree->tr1->tr1->value = (1 << tree->tr1->tr1->value) -1;
		return(optim(tree));

	case AMPER:
		if (subtre->op==STAR)
			return(subtre->tr1);
		if (subtre->op==NAME && subtre->class == OFFS) {
			p = tnode(PLUS, tree->type, subtre, tree);
			subtre->type = tree->type;
			tree->op = CON;
			tree->type = INT;
			tree->degree = 0;
			tree->value = subtre->offset;
			subtre->class = REG;
			subtre->nloc = subtre->regno;
			subtre->offset = 0;
			return(optim(p));
		}
		break;

	case STAR:
		if (subtre->op==AMPER)
			return(subtre->tr1);
		if (subtre->op==NAME && subtre->class==REG) {
			subtre->type = tree->type;
			subtre->class = OFFS;
			subtre->regno = subtre->nloc;
			return(subtre);
		}
		p = subtre->tr1;
		if ((subtre->op==INCAFT||subtre->op==DECBEF)&&tree->type!=LONG
		 && p->op==NAME && p->class==REG && p->type==subtre->type) {
			p->type = tree->type;
			p->op = subtre->op==INCAFT? AUTOI: AUTOD;
			return(p);
		}
		if (subtre->op==PLUS && p->op==NAME && p->class==REG) {
			if (subtre->tr2->op==CON) {
				p->offset =+ subtre->tr2->value;
				p->class = OFFS;
				p->type = tree->type;
				p->regno = p->nloc;
				return(p);
			}
			if (subtre->tr2->op==AMPER) {
				subtre = subtre->tr2->tr1;
				subtre->class =+ XOFFS-EXTERN;
				subtre->regno = p->nloc;
				subtre->type = tree->type;
				return(subtre);
			}
		}
		break;
	case EXCLA:
		if ((opdope[subtre->op]&RELAT)==0)
			break;
		tree = subtre;
		tree->op = notrel[tree->op-EQUAL];
		break;

	case NEG:
	case COMPL:
		if (tree->type==CHAR)
			tree->type = INT;
		if (tree->op == subtre->op)
			return(subtre->tr1);
		if (subtre->op==ITOL) {
			subtre->op = tree->op;
			subtre->type = subtre->tr1->type;
			tree->op = ITOL;
			tree->type = LONG;
			goto again;
		}
		if (tree->op!=NEG)
			break;
		/*
		 * PDP-11 FP negation
		 */
		if (subtre->op==SFCON) {
			subtre->value =^ 0100000;
			subtre->fvalue.intx[0] =^ 0100000;
			return(subtre);
		}
		if (subtre->op==FCON) {
			subtre->fvalue.intx[0] =^ 0100000;
			return(subtre);
		}
		if (subtre->op == LCON) {
			subtre->lvalue = -subtre->lvalue;
			return(subtre);
		}
	}
	if (subtre->op == CON) switch(tree->op) {

	case NEG:
		subtre->value = -subtre->value;
		return(subtre);

	case COMPL:
		subtre->value = ~subtre->value;
		return(subtre);

	case ITOF:
		fv = subtre->value;
		p = &fv;
		p++;
		if (*p++==0 && *p++==0 && *p++==0) {
			tree = getblk(sizeof(*fp));
			tree->op = SFCON;
			tree->type = DOUBLE;
			tree->value = fv.integer;
			tree->fvalue = fv;
			return(tree);
		}
		break;
	}
	tree->degree = max(islong(tree->type), degree(subtre));
	return(tree);
}

/*
 * Deal with assignments to partial-word fields.
 * The game is that select(x) =+ y turns into
 * select(x =+ select(y)) where the shifts and masks
 * are chosen properly.  The outer select
 * is discarded where the value doesn't matter.
 * Sadly, overflow is undetected on =+ and the like.
 * Pure assignment is handled specially.
 */

lvfield(at)
struct tnode *at;
{
	register struct tnode *t, *t1;
	register struct fasgn *t2;

	t = at;
	switch (t->op) {

	case ASSIGN:
		t2 = getblk(sizeof(*t2));
		t2->op = FSELA;
		t2->type = UNSIGN;
		t1 = t->tr1->tr2;
		t2->mask = ((1<<t1->tr1->value)-1)<<t1->tr2->value;
		t2->tr1 = t->tr1;
		t2->tr2 = t->tr2;
		t = t2;

	case ASANDN:
	case ASPLUS:
	case ASMINUS:
	case ASOR:
	case ASXOR:
	case INCBEF:
	case INCAFT:
	case DECBEF:
	case DECAFT:
		t1 = t->tr1;
		t1->op = FSELR;
		t->tr1 = t1->tr1;
		t1->tr1 = t->tr2;
		t->tr2 = t1;
		t1 = t1->tr2;
		t1 = tnode(COMMA, INT, tconst(t1->tr1->value),
			tconst(t1->tr2->value));
		return(optim(tnode(FSELT, UNSIGN, t, t1)));

	}
	error("Unimplemented field operator");
	return(t);
}

struct acl {
	int nextl;
	int nextn;
	struct tnode *nlist[20];
	struct tnode *llist[21];
};

acommute(atree)
{
	struct acl acl;
	int d, i, op, flt, d1;
	register struct tnode *t1, **t2, *tree;
	struct tnode *t;

	acl.nextl = 0;
	acl.nextn = 0;
	tree = atree;
	op = tree->op;
	flt = isfloat(tree);
	insert(op, tree, &acl);
	acl.nextl--;
	t2 = &acl.llist[acl.nextl];
	if (!flt) {
		/* put constants together */
		for (i=acl.nextl;i>0&&t2[0]->op==CON&&t2[-1]->op==CON;i--) {
			acl.nextl--;
			t2--;
			const(op, &t2[0]->value, t2[1]->value);
		}
	}
	if (op==PLUS || op==OR) {
		/* toss out "+0" */
		if (acl.nextl>0 && (t1 = isconstant(*t2)) && t1->value==0) {
			acl.nextl--;
			t2--;
		}
		if (acl.nextl <= 0)
			return(*t2);
		/* subsume constant in "&x+c" */
		if (op==PLUS && t2[0]->op==CON && t2[-1]->op==AMPER) {
			t2--;
			t2[0]->tr1->offset =+ t2[1]->value;
			acl.nextl--;
		}
	} else if (op==TIMES || op==AND) {
		t1 = acl.llist[acl.nextl];
		if (t1->op==CON) {
			if (t1->value==0)
				return(t1);
			if (op==TIMES && t1->value==1 && acl.nextl>0)
				if (--acl.nextl <= 0)
					return(acl.llist[0]);
		}
	}
	if (op==PLUS && !flt)
		distrib(&acl);
	tree = *(t2 = &acl.llist[0]);
	d = max(degree(tree), islong(tree->type));
	if (op==TIMES && !flt)
		d++;
	for (i=0; i<acl.nextl; i++) {
		t1 = acl.nlist[i];
		t1->tr2 = t = *++t2;
		d1 = degree(t);
		/*
		 * PDP-11 strangeness:
		 * rt. op of ^ must be in a register.
		 */
		if (op==EXOR && dcalc(t, 0)<=12) {
			t1->tr2 = t = optim(tnode(LOAD, t->type, t));
			d1 = t->degree;
		}
		t1->degree = d = d==d1? d+islong(t1->type): max(d, d1);
		t1->tr1 = tree;
		tree = t1;
		if (tree->type==LONG) {
			if (tree->op==TIMES)
				tree = hardlongs(tree);
			else if (tree->op==PLUS && (t = isconstant(tree->tr1))
			       && t->value < 0) {
				tree->op = MINUS;
				t->value = - t->value;
				t = tree->tr1;
				tree->tr1 = tree->tr2;
				tree->tr2 = t;
			}
		}
	}
	if (tree->op==TIMES && ispow2(tree))
		tree->degree = max(degree(tree->tr1), islong(tree->type));
	return(tree);
}

distrib(list)
struct acl *list;
{
/*
 * Find a list member of the form c1c2*x such
 * that c1c2 divides no other such constant, is divided by
 * at least one other (say in the form c1*y), and which has
 * fewest divisors. Reduce this pair to c1*(y+c2*x)
 * and iterate until no reductions occur.
 */
	register struct tnode **p1, **p2;
	struct tnode *t;
	int ndmaj, ndmin;
	struct tnode **dividend, **divisor;
	struct tnode **maxnod, **mindiv;

    loop:
	maxnod = &list->llist[list->nextl];
	ndmaj = 1000;
	dividend = 0;
	for (p1 = list->llist; p1 <= maxnod; p1++) {
		if ((*p1)->op!=TIMES || (*p1)->tr2->op!=CON)
			continue;
		ndmin = 0;
		for (p2 = list->llist; p2 <= maxnod; p2++) {
			if (p1==p2 || (*p2)->op!=TIMES || (*p2)->tr2->op!=CON)
				continue;
			if ((*p1)->tr2->value == (*p2)->tr2->value) {
				(*p2)->tr2 = (*p1)->tr1;
				(*p2)->op = PLUS;
				(*p1)->tr1 = (*p2);
				*p1 = optim(*p1);
				squash(p2, maxnod);
				list->nextl--;
				goto loop;
			}
			if (((*p2)->tr2->value % (*p1)->tr2->value) == 0)
				goto contmaj;
			if (((*p1)->tr2->value % (*p2)->tr2->value) == 0) {
				ndmin++;
				mindiv = p2;
			}
		}
		if (ndmin > 0 && ndmin < ndmaj) {
			ndmaj = ndmin;
			dividend = p1;
			divisor = mindiv;
		}
    contmaj:;
	}
	if (dividend==0)
		return;
	t = list->nlist[--list->nextn];
	p1 = dividend;
	p2 = divisor;
	t->op = PLUS;
	t->type = (*p1)->type;
	t->tr1 = (*p1);
	t->tr2 = (*p2)->tr1;
	(*p1)->tr2->value =/ (*p2)->tr2->value;
	(*p2)->tr1 = t;
	t = optim(*p2);
	if (p1 < p2) {
		*p1 = t;
		squash(p2, maxnod);
	} else {
		*p2 = t;
		squash(p1, maxnod);
	}
	list->nextl--;
	goto loop;
}

squash(p, maxp)
struct tnode **p, **maxp;
{
	register struct tnode **np;

	for (np = p; np < maxp; np++)
		*np = *(np+1);
}

const(op, vp, av)
int *vp;
{
	register int v;

	v = av;
	switch (op) {

	case PLUS:
		*vp =+ v;
		return;

	case TIMES:
		*vp =* v;
		return;

	case AND:
		*vp =& v;
		return;

	case OR:
		*vp =| v;
		return;

	case EXOR:
		*vp =^ v;
		return;

	case DIVIDE:
	case MOD:
		if (v==0)
			error("Divide check");
		else
			if (op==DIVIDE)
				*vp =/ v;
			else
				*vp =% v;
		return;

	case RSHIFT:
		*vp =>> v;
		return;

	case LSHIFT:
		*vp =<< v;
		return;

	case ANDN:
		*vp =& ~ v;
		return;
	}
	error("C error: const");
}

insert(op, atree, alist)
struct acl *alist;
{
	register d;
	register struct acl *list;
	register struct tnode *tree;
	int d1, i;
	struct tnode *t;

	tree = atree;
	list = alist;
	if (tree->op == op) {
	ins:	list->nlist[list->nextn++] = tree;
		insert(op, tree->tr1, list);
		insert(op, tree->tr2, list);
		return;
	}
	tree = optim(tree);
	if (tree->op == op)
		goto ins;
	if (!isfloat(tree)) {
		/* c1*(x+c2) -> c1*x+c1*c2 */
		if ((tree->op==TIMES||tree->op==LSHIFT)
		  && tree->tr2->op==CON && tree->tr2->value>0
		  && tree->tr1->op==PLUS && tree->tr1->tr2->op==CON) {
			d = tree->tr2->value;
			if (tree->op==TIMES)
				tree->tr2->value =* tree->tr1->tr2->value;
			else
				tree->tr2->value = tree->tr1->tr2->value << d;
			tree->tr1->tr2->value = d;
			tree->tr1->op = tree->op;
			tree->op = PLUS;
			if (op==PLUS)
				goto ins;
		}
	}
	d = degree(tree);
	for (i=0; i<list->nextl; i++) {
		if ((d1=degree(list->llist[i]))<d) {
			t = list->llist[i];
			list->llist[i] = tree;
			tree = t;
			d = d1;
		}
	}
	list->llist[list->nextl++] = tree;
}

tnode(op, type, tr1, tr2)
struct tnode *tr1, *tr2;
{
	register struct tnode *p;

	p = getblk(sizeof(*p));
	p->op = op;
	p->type = type;
	p->degree = 0;
	p->tr1 = tr1;
	if (opdope[op]&BINARY)
		p->tr2 = tr2;
	else
		p->tr2 = NULL;
	return(p);
}

tconst(val)
{
	register struct tconst *p;

	p = getblk(sizeof(*p));
	p->op = CON;
	p->type = INT;
	p->value = val;
	return(p);
}

getblk(size)
{
	register *p;

	if (size&01)
		abort();
	p = curbase;
	if ((curbase =+ size) >= coremax) {
		if (sbrk(1024) == -1) {
			error("Out of space-- pass 2");
			exit(1);
		}
		coremax =+ 1024;
	}
	return(p);
}

islong(t)
{
	if (t==LONG)
		return(2);
	return(1);
}

isconstant(at)
struct tnode *at;
{
	register struct tnode *t;

	t = at;
	if (t->op==CON || t->op==SFCON)
		return(t);
	if (t->op==ITOL && t->tr1->op==CON)
		return(t->tr1);
	return(0);
}

hardlongs(at)
struct tnode *at;
{
	register struct tnode *t;

	t = at;
	switch(t->op) {

	case TIMES:
	case DIVIDE:
	case MOD:
		t->op =+ LTIMES-TIMES;
		break;

	case ASTIMES:
	case ASDIV:
	case ASMOD:
		t->op =+ LASTIMES-ASTIMES;
		t->tr1 = tnode(AMPER, LONG+PTR, t->tr1);
		break;

	default:
		return(t);
	}
	return(optim(t));
}
