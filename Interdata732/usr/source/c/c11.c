#
/*
 *  C compiler
 */

#include "c1.h"

max(a, b)
{
	if (a>b)
		return(a);
	return(b);
}

degree(at)
struct tnode *at;
{
	register struct tnode *t, *t1;

	if ((t=at)==0 || t->op==0)
		return(0);
	if (t->op == CON)
		return(-3);
	if (t->op == AMPER)
		return(-2);
	if (t->op==ITOL) {
		if ((t1 = isconstant(t)) && t1->value>=0)
			return(-2);
		if ((t1=t->tr1)->type==UNSIGN && opdope[t1->op]&LEAF)
			return(-1);
	}
	if ((opdope[t->op] & LEAF) != 0) {
		if (t->type==CHAR || t->type==FLOAT)
			return(1);
		return(0);
	}
	return(t->degree);
}

pname(ap, flag)
struct tnode *ap;
{
	register i;
	register struct tnode *p;
	struct { int intx[2]; };

	p = ap;
loop:
	switch(p->op) {

/***
	case LCON:
		printf("$%o", flag>10? p->lvalue.intx[1]:p->lvalue.intx[0]);
		return;
***/

	case SFCON:
	case CON:
		psoct(p->value);
		return;

	case FCON:
		printf("L%d", (p->value>0? p->value: -p->value));
		return;

	case NAME:
		i = p->offset;
		if (flag>10)
/***/			i =+ NCPW;
/***/		if (p->class==OFFS && p->regno==14) {
/***/			if (i<0)
/***/				printf("L.%d%d(r14)", fautolen, i);
/***/			else
/***/				printf("L.%d+%d(r14)", fautolen, i);
/***/			return;
/***/		}
		if (i) {
/***/			printf("%d", i);
			if (p->class!=OFFS)
				putchar('+');
			if (p->class==REG)
				regerr();
		}
/***/		else
/***/			if (p->class==OFFS)
/***/				putchar('0');
		switch(p->class) {

		case SOFFS:
		case XOFFS:
			pbase(p);

		case OFFS:
			printf("(r%d)", p->regno);
			return;

		case EXTERN:
		case STATIC:
			pbase(p);
			return;

		case REG:
			printf("r%d", p->nloc);
			return;

		}
		error("Compiler error: pname");
		return;

	case AMPER:
/***
		putchar('$');
***/
		p = p->tr1;
		if (p->op==NAME && p->class==REG)
			regerr();
		goto loop;

/***
	case AUTOI:
		printf("(r%d)%c", p->nloc, flag==1?0:'+');
		return;

	case AUTOD:
		printf("%c(r%d)", flag==2?0:'-', p->nloc);
		return;

	case STAR:
		p = p->tr1;
		putchar('*');
		goto loop;
***/

	}
	error("pname called illegally");
}

regerr()
{
	error("Illegal use of register");
}

pbase(ap)
struct tnode *ap;
{
	register struct tnode *p;

	p = ap;
	if (p->class==SOFFS || p->class==STATIC)
		printf("L%d", p->nloc);
	else
/***/		printf("%.8s", &(p->nloc));
}

xdcalc(ap, nrleft)
struct tnode *ap;
{
	register struct tnode *p;
	register d;

	p = ap;
	d = dcalc(p, nrleft);
	if (d<20 && p->type==CHAR) {
		if (nrleft>=1)
			d = 20;
		else
			d = 24;
	}
	return(d);
}

dcalc(ap, nrleft)
struct tnode *ap;
{
	register struct tnode *p, *p1;

	if ((p=ap)==0)
		return(0);
	switch (p->op) {

	case NAME:
		if (p->class==REG)
			return(9);

	case AMPER:
	case FCON:
	case LCON:
	case AUTOI:
	case AUTOD:
		return(12);

	case CON:
	case SFCON:
		if (p->value==0)
			return(4);
		if (p->value==1)
			return(5);
/***/		return(8);

/***
	case STAR:
		p1 = p->tr1;
		if (p1->op==NAME||p1->op==CON||p1->op==AUTOI||p1->op==AUTOD)
			if (p->type!=LONG)
				return(12);
***/
	}
	if (p->type==LONG)
		nrleft--;
	return(p->degree <= nrleft? 20: 24);
}

notcompat(ap, ast, op)
struct tnode *ap;
{
	register at, st;
	register struct tnode *p;

	p = ap;
	at = p->type;
	st = ast;
	if (st==0)		/* word, byte */
/***/		return(at!=CHAR && at!=INT && at!=UNSIGN && at!=SHORT && at<PTR);
	if (st==1)		/* word */
/***/		return(at!=INT && at!=UNSIGN && at!=SHORT && at<PTR);
	st =- 2;
	if ((at&(~(TYPE+XTYPE))) != 0)
		at = 020;
	if ((at&(~TYPE)) != 0)
		at = at&TYPE | 020;
	if (st==FLOAT && at==DOUBLE)
		at = FLOAT;
	if (p->op==NAME && p->class==REG && op==ASSIGN && st==CHAR)
		return(0);
	return(st != at);
}

prins(op, c, itable)
struct instab *itable;
{
	register struct instab *insp;
	register char *ip;

	for (insp=itable; insp->op != 0; insp++) {
		if (insp->op == op) {
			ip = c? insp->str2: insp->str1;
			if (ip==0)
				break;
			printf("%s", ip);
			return;
		}
	}
	error("No match' for op %d", op);
}

collcon(ap)
struct tnode *ap;
{
	register op;
	register struct tnode *p;

	p = ap;
/***
	if (p->op==STAR) {
		if (p->type==LONG+PTR) /* avoid *x(r); *x+2(r) */
/***
			return(0);
		p = p->tr1;
	}
***/
	if (p->op==PLUS) {
		op = p->tr2->op;
/***/		if (op==CON || (op==AMPER
/***/		    && p->tr2->tr1->class!=OFFS))
			return(1);
	}
	return(0);
}

isfloat(at)
struct tnode *at;
{
	register struct tnode *t;

	t = at;
	if ((opdope[t->op]&RELAT)!=0)
		t = t->tr1;
	if (t->type==FLOAT || t->type==DOUBLE) {
		nfloat = 1;
/***/		return('e');
	}
	return(0);
}

ishfloat(t)				/***/
struct tnode *t;			/***/
{					/***/
/***/	register c;

/***/	if (c = isfloat(t))
/***/		return(c);
#ifdef HALFW
/***/	return('h');
#endif
#ifndef HALFW
/***/	return(0);
#endif
}					/***/

oddreg(t, areg)
struct tnode *t;
{
	register reg;

	reg = areg;
	if (!isfloat(t))
		switch(t->op) {
		case LLSHIFT:
		case ASLSHL:
			return((reg+1)&~01);

		case DIVIDE:
		case MOD:
		case ASDIV:
		case ASMOD:
		case PTOI:
		case ULSH:
		case ASULSH:
/***/		case TIMES:
/***/		case ASTIMES:
			reg++;

/***
		case TIMES:
		case ASTIMES:
***/
			return(reg|1);
		}
	return(reg);
}

arlength(t)
{
	if (t>=PTR)
/***/		return(NCPW);
	switch(t) {

	case INT:
	case CHAR:
	case UNSIGN:
/***/	case SHORT:
/***/		return(NCPW);


	case LONG:
/***/		return(2*NCPW);

	case FLOAT:
	case DOUBLE:
/***/		return(4);
	}
	return(1024);
}

/*
 * Strings for switch code.
 */

/***
char	dirsw[] {"\
cmp	r0,$%o\n\
jhi	L%d\n\
asl	r0\n\
jmp	*L%d(r0)\n\
.data\n\
L%d:\
" };

char	simpsw[] {"\
mov	$L%d,r1\n\
mov	r0,L%d\n\
L%d:cmp	r0,(r1)+\n\
jne	L%d\n\
jmp	*L%d-L%d(r1)\n\
.data\n\
L%d:\
"};

char	hashsw[] {"\
mov	r0,r1\n\
clr	r0\n\
div	$%o,r0\n\
asl	r1\n\
add	$L%d,r1\n\
mov	r0,*(r1)+\n\
mov	(r1)+,r1\n\
L%d:cmp	r0,-(r1)\n\
jne	L%d\n\
jmp	*L%d-L%d(r1)\n\
.data\n\
L%d:\
"};
***/

char dirsw[] {"\n\
	bp	L%d\n\
	ldar	r1,r0\n\
	bm	L%d\n\
	slaa	r1,ladc\n\
	lda	r1,L%d(r1)\n\
	br	r1\n\
" };

char simpsw[] {"\
	ldai	r1,L%d-adc\n\
	sta	r0,L%d-adc\n\
L%d	ais	r1,adc\n\
	ca	r0,0(r1)\n\
	bne	L%d\n\
	lda	r1,L%d-L%d(r1)\n\
	br	r1\n\
" };

char hashsw[] {"*hash switch??\n"};

pswitch(afp, alp, deflab)
struct swtab *afp, *alp;
{
	int ncase, i, j, tabs, worst, best, range;
	register struct swtab *swp, *fp, *lp;
	int *poctab;

	fp = afp;
	lp = alp;
	if (fp==lp) {
/***/		printf("	b	L%d\n", deflab);
		return;
	}
	isn++;
	if (sort(fp, lp))
		return;
	ncase = lp-fp;
	lp--;
	range = lp->swval - fp->swval;
	/* direct switch */
	if (range>0 && range <= 3*ncase) {
/***/		printf("* dirsw\n");
/***/		if (fp->swval) {
/***/			printf("	sai	r0,");
/***/			psoct(fp->swval);
/***/			putchar('\n');
/***/		}
/***/		printf("	cai	r0,");
/***/		psoct(range);
/***/		printf(dirsw, deflab, deflab, isn);
/***/		sdata();
/***/		label(isn);
		isn++;
		for (i=fp->swval; i<=lp->swval; i++) {
			if (i==fp->swval) {
/***/				printf("	dc	L%d\n", fp->swlab);
				fp++;
			} else
/***/				printf("	dc	L%d\n", deflab);
		}
		goto esw;
	}
	/* simple switch */
/***
	if (ncase<8) {
***/	{
		i = isn++;
		j = isn++;
/***/		printf(simpsw, i, j, isn, isn, j, i);
/***/		sdata();
/***/		label(i);
		isn++;
/***/		for (; fp<=lp; fp++) {
/***/			printf("	dc	");
/***/			psoct(fp->swval);
/***/			putchar('\n');
/***/		}
/***/		printf("	ds	adc\nL%d	equ	*\n", j);
		for (fp = afp; fp<=lp; fp++)
/***/			printf("	dc	L%d\n", fp->swlab);
/***/		printf("	dc	L%d\n", deflab);
		goto esw;
	}
	/* hash switch */
/*** Hash switch temporarily removed
	best = 077777;
	poctab = getblk(((ncase+2)/2) * sizeof(*poctab));
	for (i=ncase/4; i<=ncase/2; i++) {
		for (j=0; j<i; j++)
			poctab[j] = 0;
		for (swp=fp; swp<=lp; swp++)
			poctab[lrem(0, swp->swval, i)]++;
		worst = 0;
		for (j=0; j<i; j++)
			if (poctab[j]>worst)
				worst = poctab[j];
		if (i*worst < best) {
			tabs = i;
			best = i*worst;
		}
	}
	i = isn++;
	printf(hashsw, tabs, isn, i, i, isn+tabs+1, isn+1, isn);
	isn++;
	for (i=0; i<=tabs; i++)
		printf("L%d\n", isn+i);
	for (i=0; i<tabs; i++) {
		printf("L%d:..\n", isn++);
		for (swp=fp; swp<=lp; swp++)
			if (lrem(0, swp->swval, tabs) == i)
				printf("%o\n", ldiv(0, swp->swval, tabs));
	}
	printf("L%d:", isn++);
	for (i=0; i<tabs; i++) {
		printf("L%d\n", deflab);
		for (swp=fp; swp<=lp; swp++)
			if (lrem(0, swp->swval, tabs) == i)
				printf("L%d\n", swp->swlab);
	}
***/
esw:
/***/	stext();
}

sort(afp, alp)
struct swtab *afp, *alp;
{
	register struct swtab *cp, *fp, *lp;
	int intch, t;

	fp = afp;
	lp = alp;
	while (fp < --lp) {
		intch = 0;
		for (cp=fp; cp<lp; cp++) {
			if (cp->swval == cp[1].swval) {
				error("Duplicate case (%d)", cp->swval);
				return(1);
			}
			if (cp->swval > cp[1].swval) {
				intch++;
				t = cp->swval;
				cp->swval = cp[1].swval;
				cp[1].swval = t;
				t = cp->swlab;
				cp->swlab = cp[1].swlab;
				cp[1].swlab = t;
			}
		}
		if (intch==0)
			break;
	}
	return(0);
}

ispow2(atree)
{
	register int d;
	register struct tnode *tree;

	tree = atree;
	if (!isfloat(tree) && tree->tr2->op==CON) {
		d = tree->tr2->value;
		if (d>1 && (d&(d-1))==0)
			return(d);
	}
	return(0);
}

pow2(atree)
struct tnode *atree;
{
	register int d, i;
	register struct tnode *tree;

	tree = atree;
	if (d = ispow2(tree)) {
		for (i=0; (d=>>1)!=0; i++);
		tree->tr2->value = i;
		switch (tree->op) {

		case TIMES:
			tree->op = LSHIFT;
			break;

		case ASTIMES:
			tree->op = ASLSH;
			break;

		case DIVIDE:
/***/		case PTOI:
/***
			tree->op = ULSH;
			tree->tr2->value = -i;
***/			tree->op = RSHIFT;
			break;

		case ASDIV:
/***
			tree->op = ASULSH;
			tree->tr2->value = -i;
***/			tree->op = ASRSH;
			break;

		case MOD:
			tree->op = AND;
			tree->tr2->value = (1<<i)-1;
			break;

		case ASMOD:
			tree->op = ASAND;
			tree->tr2->value = (1<<i)-1;
			break;

		default:
			error("pow2 botch");
		}
		tree = optim(tree);
	}
	return(tree);
}

cbranch(atree, albl, cond, areg)
struct tnode *atree;
{
	int l1, op;
	register lbl, reg;
	register struct tnode *tree;

	lbl = albl;
	reg = areg;
again:
	if ((tree=atree)==0)
		return;
	switch(tree->op) {

	case LOGAND:
		if (cond) {
			cbranch(tree->tr1, l1=isn++, 0, reg);
			cbranch(tree->tr2, lbl, 1, reg);
			label(l1);
		} else {
			cbranch(tree->tr1, lbl, 0, reg);
			cbranch(tree->tr2, lbl, 0, reg);
		}
		return;

	case LOGOR:
		if (cond) {
			cbranch(tree->tr1, lbl, 1, reg);
			cbranch(tree->tr2, lbl, 1, reg);
		} else {
			cbranch(tree->tr1, l1=isn++, 1, reg);
			cbranch(tree->tr2, lbl, 0, reg);
			label(l1);
		}
		return;

	case EXCLA:
		cbranch(tree->tr1, lbl, !cond, reg);
		return;

	case SEQNC:
		rcexpr(tree->tr1, efftab, reg);
		atree = tree->tr2;
		goto again;

	case ITOL:
		tree = tree->tr1;
		break;
	}
	op = tree->op;
	if (opdope[op]&RELAT
	 && tree->tr1->op==ITOL && tree->tr2->op==ITOL) {
		tree->tr1 = tree->tr1->tr1;
		tree->tr2 = tree->tr2->tr1;
	}
	if (tree->type==LONG
	  || opdope[op]&RELAT&&tree->tr1->type==LONG) {
		longrel(tree, lbl, cond, reg);
		return;
	}
	rcexpr(tree, cctab, reg);
	op = tree->op;
	if ((opdope[op]&RELAT)==0)
		op = NEQUAL;
	else {
		l1 = tree->tr2->op;
	 	if ((l1==CON || l1==SFCON) && tree->tr2->value==0)
			op =+ 200;		/* special for ptr tests */
/***
		else
			op = maprel[op-EQUAL];
***/
	}
/***
	if (isfloat(tree))
		printf("cfcc\n");
***/
	branch(lbl, op, !cond);
}

branch(lbl, aop, c)
{
	register op;

/***/	putchar('\t');
	if(op=aop)
/***/		prins(op, c, brtab);
	else
/***/		putchar('b');
/***/	printf("\tL%d\n", lbl);
}

longrel(atree, lbl, cond, reg)
struct tnode *atree;
{
	int xl1, xl2, xo, xz;
	register int op, isrel;
	register struct tnode *tree;

	reorder(&atree, cctab, reg);
	tree = atree;
	isrel = 0;
	if (opdope[tree->op]&RELAT) {
		isrel++;
		op = tree->op;
	} else
		op = NEQUAL;
	if (!cond)
		op = notrel[op-EQUAL];
	xl1 = xlab1;
	xl2 = xlab2;
	xo = xop;
	xlab1 = lbl;
	xlab2 = 0;
	xop = op;
	xz = xzero;
	xzero = !isrel || tree->tr2->op==ITOL && tree->tr2->tr1->op==CON
		&& tree->tr2->tr1->value==0;
	if (tree->op==ANDN) {
		tree->op = TAND;
		tree->tr2 = optim(tnode(COMPL, LONG, tree->tr2));
	}
    doitover:
	if (cexpr(tree, cctab, reg) < 0) {
		if (tree->op==TAND) {
			tree->op = ANDN;
			tree->tr2 = optim(tnode(COMPL, LONG, tree->tr2));
			goto doitover;
		}
		if (isrel) {
			tree->op = MINUS;
			tree->type = LONG;
			tree = optim(tree);
		}
/***
		printf("ashc	$0,r%d\n", rcexpr(tree, regtab, reg));
***/
		branch(xlab1, op, 0);
	}
	xlab1 = xl1;
	xlab2 = xl2;
	xop = xo;
	xzero = xz;
}

/*
 * Tables for finding out how best to do long comparisons.
 * First dimen is whether or not the comparison is with 0.
 * Second is which test: e.g. a>b->
 *	cmp	a,b
 *	bgt	YES		(first)
 *	blt	NO		(second)
 *	cmp	a+2,b+2
 *	bhi	YES		(third)
 *  NO:	...
 * Note some tests may not be needed.
 */
char	lrtab[2][3][6] {
	0,	NEQUAL,	LESS,	LESS,	GREAT,	GREAT,
	NEQUAL,	0,	GREAT,	GREAT,	LESS,	LESS,
	EQUAL,	NEQUAL,	LESSEQP,LESSP,	GREATQP,GREATP,

	0,	NEQUAL,	LESS,	LESS,	GREATEQ,GREAT,
	NEQUAL,	0,	GREAT,	0,	0,	LESS,
	EQUAL,	NEQUAL,	EQUAL,	0,	0,	NEQUAL,
};

xlongrel(f)
{
	register int op, bno;

	op = xop;
	if (f==0) {
		if (bno = lrtab[xzero][0][op-EQUAL])
			branch(xlab1, bno, 0);
		if (bno = lrtab[xzero][1][op-EQUAL]) {
			xlab2 = isn++;
			branch(xlab2, bno, 0);
		}
		if (lrtab[xzero][2][op-EQUAL]==0)
			return(1);
	} else {
		branch(xlab1, lrtab[xzero][2][op-EQUAL], 0);
		if (xlab2)
			label(xlab2);
	}
	return(0);
}

label(l)
{
/***/	printf("L%d	equ	*\n", l);
}

/***
popstk(a)
{
	if (a)
		printf("	aai	sp,%d\n", a);
}
***/

error(s, p1, p2, p3, p4, p5, p6)
{
	register f;
	extern fout;

	nerror++;
	flush();
	f = fout;
	fout = 1;
	printf("%d: ", line);
	printf(s, p1, p2, p3, p4, p5, p6);
	putchar('\n');
	flush();
	fout = f;
}

psoct(an)
{
	register int n, sign;

	sign = 0;
/***/	if ((n = an) < 0 && n >= -0177777) {	/*** Interdata CAL bug ***/
		n = -n;
		sign = '-';
	}
#ifdef HALFW
/***/	printf("%cx'%x'", sign, n);
#endif
#ifndef HALFW
/***/	printf("%cy'%x'", sign, n);
#endif
}

/*
 * Read in an intermediate file.
 */
getree()
{
	LTYPE itol();
/***/	static struct tnode *expstack[20];
	register struct tnode **sp;
	register t, op;
/***/	register t1;
	static char s[9];
	register struct swtab *swp;
	double atof();
/***/	static char numbuf[64];
	struct tname *np;
	struct xtname *xnp;
	struct ftconst *fp;
	struct lconst *lp;
	int lbl, cond;

	curbase = funcbase;
	sp = expstack;
	for (;;) {
		if (sp >= &expstack[20])
			error("Stack botch");
		op = getw(ascbuf);
		if ((op&0177400) != 0177000) {
			error("Intermediate file error");
			exit(1);
		}
		lbl = 0;
		switch(op =& 0377) {

	case SINIT:
/***/		printf("	dc	");
/***/		psoct(getw(ascbuf));
/***/		putchar('\n');
		break;

	case EOF:
		return;

	case BDATA:
		if (getw(ascbuf) == 1) {
/***/			printf("	db	");
/***/			for (t=1; ; t++) {
/***/				psoct(getw(ascbuf));
				if (getw(ascbuf) != 1)
					break;
/***/				if ((t&07) == 0)
/***/					printf("\n	db	");
/***/				else
/***/					putchar(',');
			}
			printf("\n");
		}
		break;

	case PROG:
/***/		stext();
		break;

	case DATA:
/***/		sdata();
		break;

	case BSS:
#ifdef unix
/***/		printf("  bss\n");
#else
/***/		sdata();
#endif
		break;

	case SYMDEF:
		outname(s);
/***
		printf(".globl%s%s\n", s[0]?"	_":"", s);
***/
/***/		if (s[0])
/***/			printf("  entry %s\n", s);
		break;

/***/	case GLOBAL:
/***/		outname(s);
/***/		printf("  extrn %s\n", s);
/***/		break;

	case RETRN:
/***/		printf("\tlm\tr8,L.%d-%d(r14)\n\taai\tsp,L.%d\n",
			fautolen, 8*NCPW, fautolen);
/***/		printf("	br	rf\n");
		break;

	case CSPACE:
/***/		outname(s);
/***/		t = getw(ascbuf);
#ifdef unix
/***/		printf("%s\tcomn\n\tds\t%d\n\tends\n", s, t);
#else
/***/		sdata();
/***/		printf("  entry %s\n%s\tequ\t*", s, s);
/***/		printf("	%d bytes\n", t);
/***/		if (t1 = t/(8*NCPW))
/***/			printf("	do %d\n	dc	0,0,0,0,0,0,0,0\n", t1);
/***/		t =% 8*NCPW;
/***/		if (t1 = t/NCPW) {
/***/			for (printf("	dc	0"); --t1; )
/***/				printf(",0");
/***/			putchar('\n');
/***/		}
/***/		if (t =% NCPW) {
/***/			for (printf("	db	0"); --t; )
/***/				printf(",0");
/***/			putchar('\n');
/***/		}
#endif
		break;

	case SSPACE:
#ifdef unix
/***/		printf("\tds\t%d\n", getw(ascbuf));
#else
/***/		printf("\tdo\t%d\n\tdb\t0\n", getw(ascbuf));
#endif
		break;

	case EVEN:
/***/		printf("  align adc\n");
		break;

	case SAVE:
/***/		fautolen++;
/***/		pstack = 2;
/***/		ntemp = maxtemp = 2;
/***/		printf("\tsai\tsp,L.%d\n\tstm\tr8,L.%d-%d(sp)\n",
/***/			fautolen, fautolen, 8*NCPW);
/***/		printf("\tldar\tr14,sp\n");
		break;

	case SETSTK:
/***/		t = getw(ascbuf)+NCPW*maxtemp;
/***/		printf("L.%d equ %d\n", fautolen, t);
		break;

	case PROFIL:
/***/		t = getw(ascbuf);
/***/		printf("\tla\tr1,L%d\n\tbal\trf,mcount\n", t);
#ifdef unix
/***/		printf("  bss\nL%d\tdas\t1\n  pure\n", t);
#else
/***/		sdata();
/***/		printf("L%d\tdas\t1\n", t);
/***/		stext();
#endif
		break;

	case SNAME:
		t = outname(s);
/***/		printf("* %s = L%d\n", t, getw(ascbuf));
		break;

	case ANAME:
		t = outname(s);
/***/		printf("* %s = %d\n", t, getw(ascbuf));
		break;

	case RNAME:
		t = outname(s);
/***/		printf("* %s = r%d\n", t, getw(ascbuf));
		break;

	case SWIT:
		t = getw(ascbuf);
		line = getw(ascbuf);
		curbase = funcbase;
		while(swp=getblk(sizeof(*swp)), swp->swlab = getw(ascbuf))
			swp->swval = getw(ascbuf);
		pswitch(funcbase, swp, t);
		break;

	case CBRANCH:
		lbl = getw(ascbuf);
		cond = getw(ascbuf);
	case EXPR:
		line = getw(ascbuf);
		if (sp != &expstack[1]) {
			error("Expression input botch");
			exit(1);
		}
		nstack = 0;
/***/		printf("*@%d\n", line);
		*sp = optim(*--sp);
		if (lbl)
			cbranch(*sp, lbl, cond, 0);
		else
			rcexpr(*sp, efftab, 0);
		curbase = funcbase;
		break;

	case NAME:
		t = getw(ascbuf);
		if (t==EXTERN) {
			np = getblk(sizeof(*xnp));
			np->type = getw(ascbuf);
			outname(np->name);
		} else {
			np = getblk(sizeof(*np));
			np->type = getw(ascbuf);
			np->nloc = getw(ascbuf);
		}
		np->op = NAME;
		np->class = t;
		np->regno = 0;
		np->offset = 0;
		*sp++ = np;
		break;

	case CON:
		getw(ascbuf);	/* ignore type, assume int */
		*sp++ = tconst(getw(ascbuf));
		break;

	case LCON:
		getw(ascbuf);	/* ignore type, assume long */
		t = getw(ascbuf);
		op = getw(ascbuf);
		if (t==0 && op>=0 || t == -1 && op<0) {
			*sp++ = tnode(ITOL, LONG, tconst(op));
			break;
		}
		lp = getblk(sizeof(*lp));
		lp->op = LCON;
		lp->type = LONG;
		lp->lvalue = itol(t, op);
		*sp++ = lp;
		break;

	case FCON:
		t = getw(ascbuf);
		outname(numbuf);
		fp = getblk(sizeof(*fp));
		fp->op = FCON;
		fp->type = t;
		fp->value = isn++;
		fp->fvalue = atof(numbuf);
		*sp++ = fp;
		break;

	case FSEL:
		*sp = tnode(FSEL, getw(ascbuf), *--sp, NULL);
		t = getw(ascbuf);
		(*sp++)->tr2 = tnode(COMMA, INT, tconst(getw(ascbuf)), tconst(t));
		break;

	case NULLOP:
		*sp++ = tnode(0, 0, NULL, NULL);
		break;

	case LABEL:
		label(getw(ascbuf));
		break;

	case NLABEL:
		t = outname(s);
/***/		printf("%s	equ	*\n", t);
		break;

	case RLABEL:
		t = outname(s);
/***/		printf("  title %s\n%s\tequ\t*\n", t, t);
		break;

	case BRANCH:
		branch(getw(ascbuf), 0);
		break;

	case SETREG:
/***
		nreg = getw(ascbuf)-1;
***/		getw(ascbuf);
		break;

	default:
		if (opdope[op]&BINARY) {
			if (sp < &expstack[1]) {
				error("Binary expression botch");
				exit(1);
			}
			t = *--sp;
			*sp++ = tnode(op, getw(ascbuf), *--sp, t);
		} else {
			sp[-1] = tnode(op, getw(ascbuf), sp[-1]);
		}
		break;
	}
	}
}

outname(s)
{
	register char *p, c;
	register n;

	p = s;
	n = 0;
	while (c = getc(ascbuf)) {
#ifndef unix
		/***
		 *** Change '_' within names to '.' to make legal Interdata
		 *** CAL identifiers
		 ***/
/***/		if (c == '_')
/***/			c = '.';
#endif
		*p++ = c;
		n++;
	}
	do {
		*p++ = 0;
	} while (n++ < 8);
	return(s);
}

stext()
{
#ifndef HALFW
/***/	printf("  pure\n");
#endif
}

sdata()
{
#ifndef HALFW
/***/	printf("  impur\n");
#endif
}
