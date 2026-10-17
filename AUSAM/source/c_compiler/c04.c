#
/*
 * C compiler
 *
 *
 */

#include "c0.h"

/*
 * Reduce the degree-of-reference by one.
 * e.g. turn "ptr-to-int" into "int".
 */
decref(at)
{
	register t;

	t = at;
	if ((t & ~TYPE) == 0) {
		error("Illegal indirection");
		return(t);
	}
	return((t>>TYLEN) & ~TYPE | t&TYPE);
}

/*
 * Increase the degree of reference by
 * one; e.g. turn "int" to "ptr-to-int".
 */
incref(t)
{
	return(((t&~TYPE)<<TYLEN) | (t&TYPE) | PTR);
}

max(a, b)
char *a, *b;
{
	if(a > b)
		return(a);
	return(b);
}

/*
 * Make a tree that causes a branch to lbl
 * if the tree's value is non-zero together with the cond.
 */
cbranch(t, lbl, cond)
struct tnode *t;
{
	treeout(t);
	outcode("BNNN", CBRANCH, lbl, cond, line);
}

/*
 * Write out a tree.
 */
rcexpr(atp)
struct tnode *atp;
{
	register struct tnode *tp;

	/*
	 * Special optimization
	 */
	if ((tp=atp)->op==INIT && tp->tr1->op==CON) {
		if (tp->type==CHAR) {
			outcode("B1N0", BDATA, tp->tr1->value);
			return;
		} else if (tp->type==INT || tp->type==UNSIGN) {
			outcode("BN", SINIT, tp->tr1->value);
			return;
		}
	}
	treeout(tp);
	outcode("BN", EXPR, line);
}

treeout(atp)
struct tnode *atp;
{
	register struct tnode *tp;
	register struct hshtab *hp;

	if ((tp = atp) == 0) {
		outcode("B", NULLOP);
		return;
	}
	switch(tp->op) {

	case NAME:
		hp = tp->tr1;
		if (hp->hclass==TYPEDEF)
			error("Illegal use of type name");
		outcode("BNN", NAME, hp->hclass==0?STATIC:hp->hclass, tp->type);
		if (hp->hclass==EXTERN)
			outcode("S", hp->name);
		else
			outcode("N", hp->hoffset);
		return;

	case LCON:
		outcode("BNNN", tp->op, tp->type, tp->lvalue);
		return;

	case CON:
		outcode("BNN", tp->op, tp->type, tp->value);
		return;

	case FCON:
		outcode("BNF", tp->op, tp->type, tp->cstr);
		return;

	case STRING:
		outcode("BNNN", NAME, STATIC, tp->type, tp->tr1);
		return;

	case FSEL:
		treeout(tp->tr1);
		outcode("BNNN",tp->op,tp->type,tp->tr2->bitoffs,tp->tr2->flen);
		return;


	default:
		treeout(tp->tr1);
		if (opdope[tp->op]&BINARY)
			treeout(tp->tr2);
		outcode("BN", tp->op, tp->type);
		return;
	}
}

/*
 * Generate a branch
 */
branch(lab)
{
	outcode("BN", BRANCH, lab);
}

/*
 * Generate a label
 */
label(l)
{
	outcode("BN", LABEL, l);
}

/*
 * ap is a tree node whose type
 * is some kind of pointer; return the size of the object
 * to which the pointer points.
 */
plength(ap)
struct tname *ap;
{
	register t, l;
	register struct tnode *p;

	p = ap;
	if (p==0 || ((t=p->type)&~TYPE) == 0)		/* not a reference */
		return(1);
	p->type = decref(t);
	l = length(p);
	p->type = t;
	return(l);
}

/*
 * return the number of bytes in the object
 * whose tree node is acs.
 */
length(acs)
struct tnode *acs;
{
	register t, n;
	register struct tnode *cs;

	cs = acs;
	t = cs->type;
	n = 1;
	while ((t&XTYPE) == ARRAY) {
		t = decref(t);
		n = *cs->subsp;
	}
	if ((t&~TYPE)==FUNC)
		return(0);
	if (t>=PTR)
		return(SZPTR*n);
	switch(t&TYPE) {

	case INT:
	case UNSIGN:
		return(SZINT*n);

	case CHAR:
		return(n);

	case FLOAT:
		return(SZFLOAT*n);

	case LONG:
		return(SZLONG*n);

	case DOUBLE:
		return(SZDOUB*n);

	case STRUCT:
		if ((t = cs->strp->ssize) == 0)
			error("Undefined structure");
		return(n * t);
	}
	error("Compiler error (length)");
	return(0);
}

/*
 * The number of bytes in an object, rounded up to a word.
 */
rlength(cs)
struct tnode *cs;
{
	return((length(cs)+ALIGN) & ~ALIGN);
}

/*
 * After an "if (...) goto", look to see if the transfer
 * is to a simple label.
 */
simplegoto()
{
	register struct hshtab *csp;

	if ((peeksym=symbol())==NAME && nextchar()==';') {
		csp = csym;
		if (csp->hblklev == 0)
			pushdecl(csp);
		if (csp->hclass==0 && csp->htype==0) {
			csp->htype = ARRAY;
			csp->hflag =| FLABL;
			if (csp->hoffset==0)
				csp->hoffset = isn++;
		}
		if ((csp->hclass==0||csp->hclass==STATIC)
		 &&  csp->htype==ARRAY) {
			peeksym = -1;
			return(csp->hoffset);
		}
	}
	return(0);
}

/*
 * Return the next non-white-space character
 */
nextchar()
{
	while (spnextchar()==' ')
		peekc = 0;
	return(peekc);
}

/*
 * Return the next character, translating all white space
 * to blank and handling line-ends.
 */
spnextchar()
{
	register c;

	if ((c = peekc)==0)
		c = getchar();
	if (c=='\t' || c=='\014')	/* FF */
		c = ' ';
	else if (c=='\n') {
		c = ' ';
		if (inhdr==0)
			line++;
		inhdr = 0;
	} else if (c=='\001') {	/* SOH, insert marker */
		inhdr++;
		c = ' ';
	}
	peekc = c;
	return(c);
}

/*
 * is a break or continue legal?
 */
chconbrk(l)
{
	if (l==0)
		error("Break/continue error");
}

/*
 * The goto statement.
 */
dogoto()
{
	register struct tnode *np;

	*cp++ = tree();
	build(STAR);
	chkw(np = *--cp, -1);
	rcexpr(block(JUMP,0,NULL,NULL,np));
}

/*
 * The return statement, which has to convert
 * the returned object to the function's type.
 */
doret()
{
	register struct tnode *t;

	if (nextchar() != ';') {
		t = tree();
		*cp++ = &funcblk;
		*cp++ = t;
		build(ASSIGN);
		cp[-1] = cp[-1]->tr2;
		build(RFORCE);
		rcexpr(*--cp);
	}
	branch(retlab);
}

/*
 * Write a character on the error output.
 */
putchar(c)
{
	int sc;

	sc = c&0177;
	write(1, &sc, 1);
}

/*
 * Coded output:
 *   B: beginning of line; an operator
 *   N: a number
 *   S: a symbol (external)
 *   1: number 1
 *   0: number 0
 */
outcode(s, a)
char *s;
{
	extern errno;
	register *ap, *bufp;
	int n, anyerr;
	register char *np;

	bufp = obuf;
	if (strflg)
		bufp = sbuf;
	ap = &a;
	errno = 0;
	anyerr = 0;
	for (;;) switch(*s++) {
	case 'B':
		putw(*ap++ | (0376<<8), bufp);
		anyerr =| errno;
		continue;

	case 'N':
		putw(*ap++, bufp);
		anyerr =| errno;
		continue;

	case 'F':
		n = 1000;
		goto str;

	case 'S':
		n = NCPS;
	str:
		np = *ap++;
		while (n-- && *np) {
			putc(*np++&0177, bufp);
		}
		putc(0, bufp);
		anyerr =| errno;
		continue;

	case '1':
		putw(1, bufp);
		anyerr =| errno;
		continue;

	case '0':
		putw(0, bufp);
		anyerr =| errno;
		continue;

	case '\0':
		if (anyerr) {
			error("Write error on temp");
			exit(1);
		}
		return;

	default:
		error("Botch in outcode");
	}
}
