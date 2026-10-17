#
/*
 * C compiler
 *
 *
 */

#include "c0.h"

/*
 * Called from tree, this routine takes the top 1, 2, or 3
 * operands on the expression stack, makes a new node with
 * the operator op, and puts it on the stack.
 * Essentially all the work is in inserting
 * appropriate conversions.
 */
build(op) {
	register int t1;
	int t2, t;
	register struct tnode *p1, *p2;
	struct tnode *p3;
	int dope, leftc, cvn, pcvn;

	/*
	 * a[i] => *(a+i)
	 */
	if (op==LBRACK) {
		build(PLUS);
		op = STAR;
	}
	dope = opdope[op];
	if ((dope&BINARY)!=0) {
		p2 = chkfun(disarray(*--cp));
		t2 = p2->type;
	}
	p1 = *--cp;
	/*
	 * sizeof gets turned into a number here.
	 */
	if (op==SIZEOF) {
		*cp++ = cblock(length(p1));
		return;
	}
	if (op!=AMPER) {
		p1 = disarray(p1);
		if (op!=CALL)
			p1 = chkfun(p1);
	}
	t1 = p1->type;
	pcvn = 0;
	t = INT;
	switch (op) {

	/* end of expression */
	case 0:
		*cp++ = p1;
		return;

	/* no-conversion operators */
	case QUEST:
		if (p2->op!=COLON)
			error("Illegal conditional");
		if (fold(QUEST, p1, p2))
			return;

	case SEQNC:
		t = t2;

	case COMMA:
	case LOGAND:
	case LOGOR:
		*cp++ = block(op, t, NULL, NULL, p1, p2);
		return;

	case CALL:
		if ((t1&XTYPE) != FUNC)
			error("Call of non-function");
		*cp++ = block(CALL,decref(t1),p1->subsp,p1->strp,p1,p2);
		return;

	case STAR:
		if (p1->op==AMPER ) {
			*cp++ = p1->tr1;
			return;
		}
		if ((t1&XTYPE) == FUNC)
			error("Illegal indirection");
		*cp++ = block(STAR, decref(t1), p1->subsp, p1->strp, p1);
		return;

	case AMPER:
		if (p1->op==STAR) {
			*cp++ = p1->tr1;
			return;
		}
		if (p1->op==NAME) {
			*cp++ = block(op,incref(t1),p1->subsp,p1->strp,p1);
			return;
		}
		error("Illegal lvalue");
		break;

	/*
	 * a.b goes to (&a)->b
	 */
	case DOT:
		*cp++ = p1;
		build(AMPER);
		p1 = *--cp;

	/*
	 * In a->b, a is given the type ptr-to-structure element;
	 * then the offset is added in without conversion;
	 * then * is tacked on to access the member.
	 */
	case ARROW:
		if (p2->op!=NAME || p2->tr1->hclass!=MOS) {
			error("Illegal structure ref");
			*cp++ = p1;
			return;
		}
		if (t2==INT && p2->tr1->hflag&FFIELD)
			t2 = UNSIGN;
		t = incref(t2);
		chkw(p1, -1);
		setype(p1, t, p2);
		*cp++ = block(PLUS,t,p2->subsp,p2->strp,p1,cblock(p2->tr1->hoffset));
		build(STAR);
		if (p2->tr1->hflag&FFIELD)
			*cp++ = block(FSEL,UNSIGN,NULL,NULL,*--cp,p2->tr1->hstrp);
		return;
	}
	if ((dope&LVALUE)!=0)
		chklval(p1);
	if ((dope&LWORD)!=0)
		chkw(p1, LONG);
	if ((dope&RWORD)!=0)
		chkw(p2, LONG);
	if ((dope&BINARY)==0) {
		if (op==ITOF)
			t1 = DOUBLE;
		else if (op==FTOI)
			t1 = INT;
		if (!fold(op, p1, 0))
			*cp++ = block(op,t1,p1->subsp,p1->strp,p1);
		return;
	}
	cvn = 0;
	if (t1==STRUCT || t2==STRUCT) {
		error("Unimplemented structure operation");
		t1 = t2 = INT;
	}
	cvn = cvtab[lintyp(t1)][lintyp(t2)];
	leftc = (cvn>>4)&017;
	cvn =& 017;
	t = leftc? t2:t1;
	if ((t==INT||t==CHAR) && (t1==UNSIGN||t2==UNSIGN))
		t = UNSIGN;
	if (dope&ASSGOP) {
		t = t1;
		if (op==ASSIGN) {
			if (cvn==ITP||cvn==PTI)
				cvn = leftc = 0;
			else if (cvn==LTP) {
				if (leftc==0)
					cvn = LTI;
				else {
					cvn = ITL;
					leftc = 0;
				}
			}
		}
		if (leftc)
			cvn = leftc;
		leftc = 0;
	} else if (op==COLON || op==MAX || op==MIN) {
		if (t1>=PTR && t1==t2)
			cvn = 0;
		if (op!=COLON && (t1>=PTR || t2>=PTR))
			op =+ MAXP-MAX;
	} else if (dope&RELAT) {
		if (op>=LESSEQ && (t1>=PTR||t2>=PTR||(t1==UNSIGN||t2==UNSIGN)
		 && (t==INT||t==CHAR||t==UNSIGN)))
			op =+ LESSEQP-LESSEQ;
		if (cvn==PTI)
			cvn = 0;
	}
	if (cvn==PTI) {
		cvn = 0;
		if (op==MINUS) {
			t = INT;
			pcvn++;
		} else {
			if (t1!=t2 || t1!=(PTR+CHAR))
				cvn = XX;
		}
	}
	if (cvn) {
		t1 = plength(p1);
		t2 = plength(p2);
		if (cvn==XX || (cvn==PTI&&t1!=t2))
			error("Illegal conversion");
		else if (leftc)
			p1 = convert(p1, t, cvn, t2);
		else
			p2 = convert(p2, t, cvn, t1);
	}
	if (dope&RELAT)
		t = INT;
	if (t==FLOAT)
		t = DOUBLE;
	if (t==CHAR)
		t = INT;
	if (fold(op, p1, p2)==0) {
		p3 = leftc?p2:p1;
		*cp++ = block(op, t, p3->subsp, p3->strp, p1, p2);
	}
	if (pcvn && t1!=(PTR+CHAR)) {
		p1 = *--cp;
		*cp++ = convert(p1, 0, PTI, plength(p1->tr1));
	}
}

/*
 * Generate the appropriate conversion operator.
 */
struct tnode *
convert(p, t, cvn, len)
struct tnode *p;
{
	register int op;

	op = cvntab[cvn];
	if (opdope[op]&BINARY)
		return(block(op, t, NULL, NULL, p, cblock(len)));
	return(block(op, t, NULL, NULL, p));
}

/*
 * Traverse an expression tree, adjust things
 * so the types of things in it are consistent
 * with the view that its top node has
 * type at.
 * Used with structure references.
 */
setype(ap, at, anewp)
struct tnode *ap, *anewp;
{
	register struct tnode *p, *newp;
	register t;

	p = ap;
	t = at;
	newp = anewp;
	for (;; p = p->tr1) {
		p->subsp = newp->subsp;
		p->strp = newp->strp;
		p->type = t;
		if (p->op==AMPER)
			t = decref(t);
		else if (p->op==STAR)
			t = incref(t);
		else if (p->op!=PLUS)
			break;
	}
}

/*
 * A mention of a function name is turned into
 * a pointer to that function.
 */
struct tnode *
chkfun(ap)
struct tnode *ap;
{
	register struct tnode *p;
	register int t;

	p = ap;
	if (((t = p->type)&XTYPE)==FUNC)
		return(block(AMPER,incref(t),p->subsp,p->strp,p));
	return(p);
}

/*
 * A mention of an array is turned into
 * a pointer to the base of the array.
 */
struct tnode *
disarray(ap)
struct tnode *ap;
{
	register int t;
	register struct tnode *p;

	p = ap;
	/* check array & not MOS */
	if (((t = p->type)&XTYPE)!=ARRAY || p->op==NAME&&p->tr1->hclass==MOS)
		return(p);
	p->subsp++;
	*cp++ = p;
	setype(p, decref(t), p);
	build(AMPER);
	return(*--cp);
}

/*
 * make sure that p is a ptr to a node
 * with type int or char or 'okt.'
 * okt might be nonexistent or 'long'
 * (e.g. for <<).
 */
chkw(p, okt)
struct tnode *p;
{
	register int t;

	if ((t=p->type)!=INT && t<PTR && t!=CHAR && t!=UNSIGN && t!=okt)
		error("Illegal type of operand");
	return;
}

/*
 *'linearize' a type for looking up in the
 * conversion table
 */
lintyp(t)
{
	switch(t) {

	case INT:
	case CHAR:
	case UNSIGN:
		return(0);

	case FLOAT:
	case DOUBLE:
		return(1);

	case LONG:
		return(2);

	default:
		return(3);
	}
}

/*
 * Report an error.
 */
error(s, p1, p2, p3, p4, p5, p6)
{
	nerror++;
	if (filename[0])
		printf("%s:", filename);
	printf("%d: ", line);
	printf(s, p1, p2, p3, p4, p5, p6);
	printf("\n");
}

/*
 * Generate a node in an expression tree,
 * setting the operator, type, dimen/struct table ptrs,
 * and the operands.
 */
struct tnode *
block(op, t, subs, str, p1,p2)
int *subs;
struct str *str;
struct tnode *p1, *p2;
{
	register struct tnode *p;

	p = gblock(sizeof(*p));
	p->op = op;
	p->type = t;
	p->subsp = subs;
	p->strp = str;
	p->tr1 = p1;
	if (opdope[op]&BINARY)
		p->tr2 = p2;
	else
		p->tr2 = NULL;
	return(p);
}

struct tnode *
nblock(ads)
struct hshtab *ads;
{
	register struct hshtab *ds;

	ds = ads;
	return(block(NAME, ds->htype, ds->hsubsp, ds->hstrp, ds));
}

/*
 * Generate a block for a constant
 */
struct cnode *
cblock(v)
{
	register struct cnode *p;

	p = gblock(sizeof(*p));
	p->op = CON;
	p->type = INT;
	p->subsp = NULL;
	p->strp = NULL;
	p->value = v;
	return(p);
}

/*
 * A block for a float or long constant
 */
struct fnode *
fblock(t, string)
char *string;
{
	register struct fnode *p;

	p = gblock(sizeof(*p));
	p->op = FCON;
	p->type = t;
	p->subsp = NULL;
	p->strp = NULL;
	p->cstr = string;
	return(p);
}

/*
 * Assign a block for use in the
 * expression tree.
 */
char *
gblock(n)
{
	register int *p;

	p = curbase;
	if ((curbase =+ n) >= coremax) {
		if (sbrk(1024) == -1) {
			error("Out of space");
			exit(1);
		}
		coremax =+ 1024;
	}
	return(p);
}

/*
 * Check that a tree can be used as an lvalue.
 */
chklval(ap)
struct tnode *ap;
{
	register struct tnode *p;

	p = ap;
	if (p->op==FSEL)
		p = p->tr1;
	if (p->op!=NAME && p->op!=STAR)
		error("Lvalue required");
}

/*
 * reduce some forms of `constant op constant'
 * to a constant.  More of this is done in the next pass
 * but this is used to allow constant expressions
 * to be used in switches and array bounds.
 */
fold(op, ap1, ap2)
struct tnode *ap1, *ap2;
{
	register struct tnode *p1;
	register int v1, v2;

	p1 = ap1;
	if (p1->op!=CON)
		return(0);
	if (op==QUEST) {
		if (ap2->tr1->op==CON && ap2->tr2->op==CON) {
			p1->value = p1->value? ap2->tr1->value: ap2->tr2->value;
			*cp++ = p1;
			return(1);
		}
		return(0);
	}
	if (ap2) {
		if (ap2->op!=CON)
			return(0);
		v2 = ap2->value;
	}
	v1 = p1->value;
	switch (op) {

	case PLUS:
		v1 =+ v2;
		break;

	case MINUS:
		v1 =- v2;
		break;

	case TIMES:
		v1 =* v2;
		break;

	case DIVIDE:
		v1 =/ v2;
		break;

	case MOD:
		v1 =% v2;
		break;

	case AND:
		v1 =& v2;
		break;

	case OR:
		v1 =| v2;
		break;

	case EXOR:
		v1 =^ v2;
		break;

	case NEG:
		v1 = - v1;
		break;

	case COMPL:
		v1 = ~ v1;
		break;

	case LSHIFT:
		v1 =<< v2;
		break;

	case RSHIFT:
		v1 =>> v2;
		break;

	case EQUAL:
		v1 = v1==v2;
		break;

	case NEQUAL:
		v1 = v1!=v2;
		break;

	case LESS:
		v1 = v1<v2;
		break;

	case GREAT:
		v1 = v1>v2;
		break;

	case LESSEQ:
		v1 = v1<=v2;
		break;

	case GREATEQ:
		v1 = v1>=v2;
		break;

	default:
		return(0);
	}
	p1->value = v1;
	*cp++ = p1;
	return(1);
}

/*
 * Compile an expression expected to have constant value,
 * for example an array bound or a case value.
 */
conexp()
{
	register struct tnode *t;

	initflg++;
	if (t = tree())
		if (t->op != CON)
			error("Constant required");
	initflg--;
	curbase = funcbase;
	return(t->value);
}
