#
/*

	    	C compiler, part 2


*/

#include "c1.h"

#define	dbprint(op)	/* */
#ifdef	DEBUG
#define	dbprint(op)	printf("\t. %s", opntab[op])	/***/
#endif

/*
 * If using OS/MT CAL assembler, read string file first, since
 * it contains ENTRY definitions for functions
 */
#ifdef unix
#define	FILE1	argv[1]
#define FILE2	argv[2]
#else
#define FILE1	argv[2]
#define FILE2	argv[1]
#endif
char	maprel[] {	EQUAL, NEQUAL, GREATEQ, GREAT, LESSEQ,
			LESS, GREATQP, GREATP, LESSEQP, LESSP
};

char	notrel[] {	NEQUAL, EQUAL, GREAT, GREATEQ, LESS,
			LESSEQ, GREATP, GREATQP, LESSP, LESSEQP
};

struct tconst czero { CON, INT, 0};
struct tconst cone  { CON, INT, 1};

struct	table	cregtab[1], lsptab[1];

int	nreg	6;		/***/
int	isn	10000;

main(argc, argv)
char *argv[];
{
/***/	register i;
	extern fout;

	if (argc<4) {
		error("Arg count");
		exit(1);
	}
	if(fopen(FILE1, ascbuf)<0) {
		error("Missing temp file");
		exit(1);
	}
	if ((fout = creat(argv[3], 0666)) < 0) {
		error("Can't create %s", argv[3]);
		exit(1);
	}
/***
 *** Write register equates
 ***/
/***/	for (i = 0; i < 16; i++)
/***/		printf("r%d equ %d\n", i, i);
/***/	printf("sp equ 7\nrf equ 15\n");

	funcbase = curbase = coremax = sbrk(0);
	getree();
/***/	printf("  align adc\n");
/***/	sdata();
	/*
	 * If any floating-point instructions
	 * were used, generate a reference which
	 * pulls in the floating-point part of printf.
	 */
	if (nfloat)
/***/		printf("\textrn fltused\n\tdc\tfltused\n");
	/*
	 * tack on the string file.
	 */
	close(ascbuf[0]);
	if (fopen(FILE2, ascbuf)<0) {
		error("Missing temp file");
		exit(1);
	}
	getree();
/***/	printf("  end\n");
	flush();
	exit(nerror!=0);
}

/*
 * Given a tree, a code table, and a
 * count of available registers, find the code table
 * for the appropriate operator such that the operands
 * are of the right type and the number of registers
 * required is not too large.
 * Return a ptr to the table entry or 0 if none found.
 */
char *match(atree, table, nrleft)
struct tnode *atree;
struct table *table;
{
	int op, d1, d2, dope;
	register struct tnode *p2;
	register struct tnode *p1, *tree;
	register struct optab *opt;

	if ((tree=atree)==0)
		return(0);
	if ((op = tree->op)==0)
		return(0);
	if (table == lsptab)
		table = sptab;
	dope = opdope[op];
	if ((dope&LEAF) == 0)
		p1 = tree->tr1;
	else
		p1 = tree;
	d1 = dcalc(p1, nrleft);
	if ((dope&BINARY)!=0) {
		p2 = tree->tr2;
		/*
		 * If a subtree starts off with a conversion operator,
		 * try for a match with the conversion eliminated.
		 * E.g. int = double can be done without generating
		 * the converted int in a register by
		 * movf double,fr0; movfi fr0,int .
		 */
		if (opdope[p1->op]&CNVRT && (opdope[p2->op]&CNVRT)==0
		 && (opdope[p1->tr1->op]&CNVRT)==0) {
			tree->tr1 = p1->tr1;
			if (opt = match(tree, table, nrleft))
				return(opt);
			tree->tr1 = p1;
		} else if (opdope[p2->op]&CNVRT && (opdope[p1->op]&CNVRT)==0
			 && (opdope[p2->tr1->op]&CNVRT)==0) {
			tree->tr2 = p2->tr1;
			if (opt = match(tree, table, nrleft))
				return(opt);
			tree->tr2 = p2;
		}
		d2 = dcalc(p2, nrleft);
	}
	for (; table->op!=op; table++)
		if (table->op==0)
			return(0);
	for (opt = table->tabp; opt->tabdeg1!=0; opt++) {
		if (d1 > (opt->tabdeg1&077)
		 || (opt->tabdeg1 >= 0100 && (p1->op != STAR)))
			continue;
/***/		if (op != AMPER && notcompat(p1, opt->tabtyp1, op)) {
			continue;
		}
		if ((opdope[op]&BINARY)!=0 && p2!=0) {
			if (d2 > (opt->tabdeg2&077)
			 || (opt->tabdeg2 >= 0100) && (p2->op != STAR) )
				continue;
			if (notcompat(p2,opt->tabtyp2, 0))
				continue;
			/***
			 *** Kludge for SHORT - mh and dh don't mean what
			 *** you think, so special case is required.
			 *** This should really be handled by a new
			 *** code table category.
			 ***/
/***/			if (p2->type==SHORT && opt->tabtyp2==1
/***/			  && (op==TIMES || op==ASTIMES || op==DIVIDE
/***/			    || op==ASDIV || op==MOD || op==ASMOD))
/***/				continue;
		}
		return(opt);
	}
	return(0);
}

/*
 * Given a tree, a code table, and a register,
 * produce code to evaluate the tree with the appropriate table.
 * Registers reg and upcan be used.
 * If there is a value, it is desired that it appear in reg.
 * The routine returns the register in which the value actually appears.
 * This routine must work or there is an error.
 * If the table called for is cctab, sptab, or efftab,
 * and tree can't be done using the called-for table,
 * another try is made.
 * If the tree can't be compiled using cctab, regtab is
 * used and a "tst" instruction is produced.
 * If the tree can't be compiled using sptab,
 * regtab is used and the register is pushed on the stack.
 * If the tree can't be compiled using efftab,
 * just use regtab.
 * Regtab must succeed or an "op not found" error results.
 *
 * A number of special cases are recognized, and
 * there is an interaction with the optimizer routines.
 */
rcexpr(atree, atable, reg)
struct tnode *atree;
struct table *atable;
{
	register r;
	int modf, nargs, recurf;
	register struct tnode *tree;
	register struct table *table;

	table = atable;
	recurf = 0;
	if (reg<0) {
		recurf++;
		reg = ~reg;
		if (reg>=020) {
			reg =- 020;
			recurf++;
		}
	}
again:
	if((tree=atree)==0)
		return(0);
	if (opdope[tree->op]&RELAT && tree->tr2->op==CON && tree->tr2->value==0
	 && table==cctab)
		tree = atree = tree->tr1;
	/*
	 * fieldselect(...) : in efftab mode,
	 * ignore the select, otherwise
	 * do the shift and mask.
	 */
	if (tree->op == FSELT) {
		if (table==efftab)
			atree = tree = tree->tr1;
		else {
			tree->op = FSEL;
			atree = tree = optim(tree);
		}
	}
	switch (tree->op)  {
	/*
	 * An initializing expression
	 */
	case INIT:
		tree = optim(tree);
		doinit(tree->type, tree->tr1);
		return(0);

	/*
	 * Put the value of an expression in r0,
	 * for a switch or a return
	 */
	case RFORCE:
		if((r=rcexpr(tree->tr1, regtab, reg)) != 0)
			movreg(r, 0, tree->tr1);
		return(0);

	/*
	 * sequential execution
	 */
	case SEQNC:
		rcexpr(tree->tr1, efftab, reg);
		atree = tree = tree->tr2;
		goto again;

	/*
	 * Handle a subroutine call. It has to be done
	 * here because if cexpr got called twice, the
	 * arguments might be compiled twice.
	 * There is also some fiddling so the
	 * first argument, in favorable circumstances,
	 * goes to (sp) instead of -(sp), reducing
	 * the amount of stack-popping.
	 */
	case CALL:
/***
		r = 0;
		nargs = 0;
		modf = 0;
		if (tree->tr1->op!=NAME || tree->tr1->class!=EXTERN) {
			nargs++;
			nstack++;
		}
		tree = tree->tr2;
		if(tree->op) {
			while (tree->op==COMMA) {
				r =+ comarg(tree->tr2, &modf);
				tree = tree->tr1;
				nargs++;
			}
			r =+ comarg(tree, &modf);
			nargs++;
		}
		tree = atree;
		tree->op = CALL2;
		if (modf && tree->tr1->op==NAME && tree->tr1->class==EXTERN)
			tree->op = CALL1;
		cexpr(tree, regtab, reg);
		popstk(r);
		nstack =- nargs;
***/
/***/		/* Find length of arguments and adjust stack */
/***/		nargs = pstack;
/***/		r = 0;
/***/		tree = tree->tr2;
/***/		if (tree->op) {
/***/			while (tree->op == COMMA) {
/***/				r =+ arlength(tree->tr2->type);
/***/				tree = tree->tr1;
/***/			}
/***/			r =+ arlength(tree->type);
/***/			r =/ NCPW;
/***/			if ((modf = nstack+r) > pstack)
/***/				setstk(modf);
/***/			if (modf < pstack) {
/***/				nstack =+ pstack-modf;
/***/				r =+ pstack-modf;
/***/			}
/***/			/* Compile arguments onto stack */
/***/			tree = atree->tr2;
/***/			while (tree->op == COMMA) {
/***/				comarg(tree->tr2);
/***/				tree = tree->tr1;
/***/			}
/***/			comarg(tree);
/***/		}
/***/		tree = atree;
/***/		tree->op = CALL1;
/***/		cexpr(tree, regtab, reg);
/***/		if (pstack != nargs)
/***/			setstk(nargs);
/***/		nstack =- r;

		if (table==efftab || table==regtab)
			return(0);
		r = 0;
		goto fixup;

	/*
	 * Longs need special treatment.
	 */
	case ASLSH:
	case LSHIFT:
		if (tree->type==LONG) {
			if (tree->tr2->op==ITOL)
				tree->tr2 = tree->tr2->tr1;
			else
				tree->tr2 = optim(tnode(LTOI,INT,tree->tr2));
			if (tree->op==ASLSH)
				tree->op = ASLSHL;
			else
				tree->op = LLSHIFT;
		}
		break;

	/*
	 * Try to change * to shift.
	 */
	case TIMES:
	case ASTIMES:
		tree = pow2(tree);
		break;

	/***
	 *** Interdata special: change --, -= to ++, += to use AM instruction
	 ***/
/***/	case ASMINUS:
/***/		if (tree->tr2->op != CON)
/***/			break;
/***/	case DECBEF:
/***/	case DECAFT:
/***/		if (table==efftab || table==cctab) {
/***/			tree->op =+ (ASPLUS-ASMINUS);
/***/			tree->tr2->value = -tree->tr2->value;
/***/		}

	}
	/*
	 * Try to find postfix ++ and -- operators that can be
	 * pulled out and done after the rest of the expression
	 */
	if (table!=cctab && table!=cregtab && recurf<2
	 && (opdope[tree->op]&LEAF)==0) {
		if (r=delay(&atree, table, reg)) {
			tree = atree;
			table = efftab;
			reg = r-1;
		}
	}
	/*
	 * Basically, try to reorder the computation
	 * so  reg = x+y  is done as  reg = x; reg =+ y
	 */
	if (recurf==0 && reorder(&atree, table, reg)) {
		if (table==cctab && atree->op==NAME)
			return(reg);
	}
	tree = atree;
	if (table==efftab && tree->op==NAME)
		return(reg);
	if ((r=cexpr(tree, table, reg))>=0)
		return(r);
	if (table!=regtab && (table!=cctab||(opdope[tree->op]&RELAT)==0)) {
		if((r=cexpr(tree, regtab, reg))>=0) {
	fixup:
/***/			modf = ishfloat(tree);
			if (table==sptab) {
/***/				if(tree->type==LONG)
/***/					printf("\tsta\tr%d,%d(sp)\n",
/***/					  r+1, (pstack-nstack+1)*NCPW);
/***/				printf("\tst%c\tr%d,%d(sp)\n", modf,
/***/				  modf=='e'? r*2 : r, (pstack-nstack)*NCPW);
			}
/***/			if (table==lsptab) {
/***/				printf("\tst%c\tr%d,%d(r14)\n", modf,
/***/				  modf=='e'? r*2 : r, NCPW*(ntemp-1));
/***/			}
			if (table==cctab)
/***/				movreg(r, r, tree);
			return(r);
		}
	}
	/*
	 * There's a last chance for this operator
	 */
	if (tree->op==LTOI) {
		r = rcexpr(tree->tr1, regtab, reg);
		if (r >= 0) {
			r++;
			goto fixup;
		}
	}
	if (tree->type == STRUCT)
		error("Illegal operation on structure");
	else if (tree->op>0 && tree->op<RFORCE && opntab[tree->op])
		error("No code table for op: %s", opntab[tree->op]);
	else
		error("No code table for op %d", tree->op);
	return(reg);
}

/*
 * Try to compile the tree with the code table using
 * registers areg and up.  If successful,
 * return the register where the value actually ended up.
 * If unsuccessful, return -1.
 *
 * Most of the work is the macro-expansion of the
 * code table.
 */
cexpr(atree, table, areg)
struct tnode *atree;
struct table *table;
{
	int c, r;
	register struct tnode *p, *p1, *tree;
	struct table *ctable;
	register struct tnode *p2;
	char *string;
	int reg, reg1, rreg, flag, opd;
	char *opt;

	tree = atree;
	reg = areg;
	p1 = tree->tr2;
	c = tree->op;
	opd = opdope[c];
	/*
	 * When the value of a relational or a logical expression is
	 * desired, more work must be done.
	 */
	if ((opd&RELAT||c==LOGAND||c==LOGOR||c==EXCLA) && table!=cctab) {
		cbranch(tree, c=isn++, 1, reg);
		rcexpr(&czero, table, reg);
		branch(isn, 0);
		label(c);
		rcexpr(&cone, table, reg);
		label(isn++);
		return(reg);
	}
	if(c==QUEST) {
		if (table==cctab)
			return(-1);
		cbranch(tree->tr1, c=isn++, 0, reg);
		flag = nstack;
		rreg = rcexpr(p1->tr1, table, reg);
		nstack = flag;
		branch(r=isn++, 0);
		label(c);
		reg = rcexpr(p1->tr2, table, rreg);
		if (rreg!=reg)
			movreg(reg, rreg, tree->tr2);
		label(r);
		return(rreg);
	}
	reg = oddreg(tree, reg);
	reg1 = reg+1;
	/*
	 * long values take 2 registers.
	 */
	if ((tree->type==LONG||opd&RELAT&&tree->tr1->type==LONG) && tree->op!=ITOL)
		reg1++;
	/*
	 * Leaves of the expression tree
	 */
	if ((r = chkleaf(tree, table, reg)) >= 0)
		return(r);
	/*
	 * Because of a peculiarity of the PDP11 table
	 * char = *intreg++ and *--intreg cannot go through.
 	 */
/***
	if (tree->tr1->type==CHAR && tree->tr2->type!=CHAR
	 && (tree->tr2->op==AUTOI||tree->tr2->op==AUTOD))
		tree->tr2 = tnode(LOAD, tree->tr2->type, tree->tr2);
***/
	if (table==cregtab)
		table = regtab;
	/*
	 * The following peculiar code depends on the fact that
	 * if you just want the codition codes set, efftab
	 * will generate the right code unless the operator is
	 *** ASSIGN or
	 * postfix ++ or --. Unravelled, if the table is
	 * cctab and the operator is not special, try first
	 * for efftab;  if the table isn't, if the operator is,
	 * or the first match fails, try to match
	 * with the table actually asked for.
	 */
	r = nreg - reg;
/***/	if (table!=cctab || c==ASSIGN
/***/	 || c==INCAFT || c==DECAFT || tree->type==LONG
	 || (opt = match(tree, efftab, r)) == 0)
		if ((opt=match(tree, table, r))==0)
			return(-1);
	string = opt->tabstring;
	p1 = tree->tr1;
	if (p1->op==FCON && p1->value>0) {
/***
 *** Warning -- lark's vomit
 *** (won't work on the 7/16)
 ***/
/***/		printf(" impur\nL%d\tdc\ty'%x'\n  pure\n", p1->value,  p1->fvalue);
		p1->value = -p1->value;
	}
	p2 = 0;
	if (opdope[tree->op]&BINARY) {
		p2 = tree->tr2;
		if (p2->op==FCON && p2->value>0) {
/***/			printf(" impur\nL%d\tdc\ty'%x'\n  pure\n",
/***/				p2->value, p2->fvalue);
			p2->value = -p2->value;
		}
	}
loop:
	/*
	 * The 0200 bit asks for a tab.
	 */
	if ((c = *string++) & 0200) {
		c =& 0177;
		putchar('\t');
	}
	switch (c) {

	case '\n':
		dbprint(tree->op);
		break;

	case '\0':
		if (!isfloat(tree))
			if (tree->op==MOD||tree->op==ASMOD)
				reg--;
		return(reg);

	/* A1 */
	case 'A':
		p = p1;
		goto adr;

	/* A2 */
	case 'B':
		p = p2;
		goto adr;

	adr:
		c = 0;
		while (*string=='\'') {
			c++;
			string++;
		}
		if (*string=='+') {
			c = 100;
			string++;
		}
		pname(p, c);
		goto loop;

	/* I */
	case 'M':
		if ((c = *string)=='\'')
			string++;
		else
			c = 0;
		prins(tree->op, c, instab);
		goto loop;

	/* B1 */
	case 'C':
		if ((opd&LEAF) != 0)
			p = tree;
		else
			p = p1;
		goto pbyte;

	/* BF */
	case 'P':
		p = tree;
		goto pb1;

	/* B2 */
	case 'D':
		p = p2;
	pbyte:
/***/		if (p->op==CON || p->op==AMPER) {
#ifdef HALFW
/***/			putchar('h');
#endif
/***/			putchar('i');
/***/			goto loop;
/***/		}
/***/		if (p->op==NAME && p->class==REG) {
#ifdef HALFW
/***/			putchar('h');
#endif
/***/			putchar('r');
/***/			goto loop;
/***/		}
/***/		if (p->type==CHAR) {
/***/			putchar('b');
/***/			goto loop;
/***/		}
/***/		if (p->type==SHORT) {
/***/			putchar('h');
/***/			goto loop;
/***/		}
/***/	pb1:
/***/		if (c=ishfloat(p))
/***/			putchar(c);
		goto loop;

	/* BE */
/***
	case 'L':
		if (p1->type==CHAR || p2->type==CHAR)
			putchar('b');
		p = tree;
		goto pb1;
***/

	/* F */
	case 'G':
		p = p1;
		flag = 01;
		goto subtre;

	/* S */
	case 'K':
		p = p2;
		flag = 02;
		goto subtre;

	/* H */
	case 'H':
		p = tree;
		flag = 04;

	subtre:
		ctable = regtab;
		if (flag&04)
			ctable = cregtab;
		c = *string++ - 'A';
/***/		if ((c&020) && !reg) {	/* nonzero register required */
/***/			reg = 1;
/***/			reg1 = 2;
/***/		}
		if (*string=='!') {
			string++;
			c =| 020;	/* force right register */
		}
/***/		if ((c&02)!=0) {
/***/			ctable = lsptab;
/***/			if (++ntemp > maxtemp)
/***/				maxtemp = ntemp;
/***/		}
		if ((c&04)!=0)
			ctable = cctab;
		if ((flag&01) && ctable==regtab && (c&01)==0
		  && (tree->op==DIVIDE||tree->op==MOD
		   || tree->op==ASDIV||tree->op==ASMOD))
			ctable = cregtab;
		if ((c&01)!=0) {
			p = p->tr1;
/***/			if(collcon(p) && ctable!=lsptab) {
/***
				if (p->op==STAR)
					p = p->tr1;
***/
				p = p->tr1;
			}
		}
		if (c&010)
			r = reg1;
		else
			if (opdope[p->op]&LEAF || p->degree < 2)
				r = reg;
			else
				r = areg;
		rreg = rcexpr(p, ctable, r);
		if (ctable!=regtab && ctable!=cregtab)
			goto loop;
		if (c&010) {
			if (c&020 && rreg!=reg1)
				movreg(rreg, reg1, p);
			else
				reg1 = rreg;
		} else if (rreg!=reg)
			if ((c&020)==0 && oddreg(tree, 0)==0 && tree->type!=LONG
			&& (flag&04
			  || flag&01&&xdcalc(p2,nreg-rreg-1)<=(opt->tabdeg2&077)
			  || flag&02&&xdcalc(p1,nreg-rreg-1)<=(opt->tabdeg1&077))) {
				reg = rreg;
				reg1 = rreg+1;
			} else
				movreg(rreg, reg, p);
		goto loop;

	/* R */
	case 'I':
		r = reg;
		if (*string=='-') {
			string++;
			r--;
		}
		goto preg;

	/* R1 */
	case 'J':
		r = reg1;
	preg:
		if (*string=='+') {
			string++;
			r++;
		}
/***/		else if (*string==':') {
/***/			string++;
/***/			if (tree->op==MOD || tree->op==ASMOD)
/***/				r--;
/***/		}
/***/		else if (*string=='*') {
/***/			string++;
/***/			if (isfloat(tree) || tree->op==FTOI) {
/***/				r =* 2;
/***/				goto pregf;
/***/			}
/***/		}
		if (r>nreg || r>=4 && tree->type==DOUBLE)
			error("Register overflow: simplify expression");
/***/ pregf:
		printf("r%d", r);
		goto loop;

/***/	/* P */
/***/	case 'Q':
/***/		if (*string == '-') {		/* push onto stack */
/***/			string++;
/***/			if (++ntemp > maxtemp)
/***/				maxtemp = ntemp;
/***/			goto loop;
/***/		}
/***/		/* reference to top word on stack */
/***/		if (table==sptab)
/***/			printf("%d(sp)", (pstack - nstack)*NCPW);
/***/		else
/***/			printf("%d(r14)", NCPW*(ntemp-1));
/***/		if (*string == '+') {		/* pop stack */
/***/			string++;
/***/			ntemp--;
/***/		}
/***/		goto loop;

	/* #1 */
	case '#':
		p = p1->tr1;
		goto nmbr;

	/* #2 */
	case '"':
		p = p2->tr1;

	nmbr:
		if(collcon(p)) {
/***
			if (p->op==STAR) {
				printf("*");
				p = p->tr1;
			}
***/
			if ((p = p->tr2)->op == CON) {
				if (p->value)
					psoct(p->value);
/***/				else
/***/					putchar('0');
			} else if (p->op==AMPER)
				pname(p->tr1, 0);
		}
/***/		else
/***/			putchar('0');
		goto loop;

	/*
	 * Certain adjustments for / % and PTOI
	 */
	case 'T':
		c = reg-1;
/***
		if (tree->op == PTOI) {
			printf("bic	r%d,r%d\nsbc	r%d\n", c,c,c);
			goto loop;
		}
		if (p1->type==UNSIGN || p1->type&XTYPE) {
			printf("clr	r%d\n", c);
			goto loop;
		}
		if (dcalc(p1, 5)>12 && !match(p1, cctab, 10))
			printf("tst	r%d\n", reg);
		printf("sxt	r%d\n", c);
***/

/***/		if (tree->op==PTOI || p1->type==UNSIGN || p1->type&XTYPE) {
/***/			printf("\tlis\tr%d,0\n", c);
/***/			goto loop;
/***/		}
/***/		printf("\tldar\tr%d,r%d\n\tsraa\tr%d,31\n", c, reg, c);
		goto loop;

	/* adc sbc, clr, or sxt as required for longs */
/***
	case 'V':
		switch(tree->op) {
		case PLUS:
		case ASPLUS:
		case INCBEF:
		case INCAFT:
			printf("adc");
			break;

		case MINUS:
		case ASMINUS:
		case NEG:
		case DECBEF:
		case DECAFT:
			printf("sbc");
			break;

		case ASSIGN:
			p = tree->tr2;
			goto lcasev;

		case ASDIV:
		case ASMOD:
		case ASULSH:
			p = tree->tr1;
		lcasev:
			if (p->type!=LONG) {
				if (p->type==UNSIGN || p->type&XTYPE)
					printf("clr");
				else
					printf("sxt");
				goto loop;
			}
		default:
			while ((c = *string++)!='\n' && c!='\0');
			break;
		}
		goto loop;
***/

	/*
	 * Mask used in field assignments
	 */
	case 'Z':
/***/		psoct(~tree->mask);
		goto loop;

	/*
	 * Relational on long values.
	 * Might bug out early. E.g.,
	 * (long<0) can be determined with only 1 test.
	 */
	case 'X':
		if (xlongrel(*string++ - '0'))
			return(reg);
		goto loop;
	}
	putchar(c);
	goto loop;
}

/*
 * This routine just calls sreorder (below)
 * on the subtrees and then on the tree itself.
 * It returns non-zero if anything changed.
 */
reorder(treep, table, reg)
struct tnode **treep;
struct table *table;
{
	register r, o;
	register struct tnode *p;

	p = *treep;
	o = p->op;
	if (opdope[o]&LEAF || o==LOGOR || o==LOGAND)
		return(0);
	while(sreorder(&p->tr1, regtab, reg))
		;
	if (opdope[o]&BINARY) 
		while(sreorder(&p->tr2, regtab, reg))
			;
	r = 0;
/***/	if (table!=cctab && table!=cregtab)
	while (sreorder(treep, table, reg))
		r++;
	*treep = optim(*treep);
	return(r);
}

/*
 * Basically this routine carries out two kinds of optimization.
 * First, it observes that "x + (reg = y)" where actually
 * the = is any assignment op is better done as "reg=y; x+reg".
 * In this case rcexpr is called to do the first part and the
 * tree is modified so the name of the register
 * replaces the assignment.
 * Moreover, expressions like "reg = x+y" are best done as
 * "reg = x; reg =+ y" (so long as "reg" and "y" are not the same!).
 */
sreorder(treep, table, reg)
struct tnode **treep;
struct table *table;
{
	register struct tnode *p, *p1;

	p = *treep;
	if (opdope[p->op]&LEAF)
		return(0);
	if (p->op==PLUS)
		if (reorder(&p->tr2, table, reg))
			*treep = p = optim(p);
	p1 = p->tr1;
	if (p->op==STAR || p->op==PLUS) {
		if (reorder(&p->tr1, table, reg))
			*treep = p = optim(p);
		p1 = p->tr1;
	}
/***/	if (opdope[p->op]&LEAF)
/***/		return(0);
/***/	if (p1->op==NAME && p1->class==REG) switch(p->op) {
		case ASLSH:
		case ASRSH:
		case ASSIGN:
/***/			if (p1->type==CHAR||isfloat(p->tr2))
				return(0);
			if (p->op==ASSIGN) switch (p->tr2->op) {
			case TIMES:
				if (!ispow2(p->tr2))
					break;
				p->tr2 = pow2(p->tr2);
			case PLUS:
			case MINUS:
			case AND:
			case ANDN:
			case OR:
			case EXOR:
			case LSHIFT:
			case RSHIFT:
				p1 = p->tr2->tr2;
				if (xdcalc(p1, 16) > 12
				 || p1->op==NAME
				 &&(p1->nloc==p->tr1->nloc
				  || p1->regno==p->tr1->nloc))
					return(0);
				p1 = p->tr2;
				p->tr2 = p1->tr1;
				if (p1->tr1->op!=NAME
				 || p1->tr1->class!=REG
				 || p1->tr1->nloc!=p->tr1->nloc)
					rcexpr(p, efftab, reg);
				p->tr2 = p1->tr2;
				p->op = p1->op + ASPLUS - PLUS;
				*treep = p;
				return(1);
			}
			goto OK;

		case ASTIMES:
			if (!ispow2(p))
				return(0);
		case ASPLUS:
		case ASMINUS:
		case ASAND:
		case ASANDN:
		case ASOR:
		case ASXOR:
		case INCBEF:
		case DECBEF:
		OK:
			if (table==cctab||table==cregtab)
				reg =+ 020;
			rcexpr(optim(p), efftab, ~reg);
			*treep = p1;
			return(1);
	}
	return(0);
}

/*
 * Delay handles postfix ++ and -- 
 *** It observes that "x + reg++" is better
 *** treated as "x + reg; reg++".
 * If the operator is ++ or -- itself,
 * it calls rcexpr to load the operand, letting
 * the calling instance of rcexpr to do the
 * ++ using efftab.
 * Otherwise it uses sdelay to search for inc/dec
 * among the operands.
 */
delay(treep, table, reg)
struct tnode **treep;
{
	register struct tnode *p, *p1;
	register r;

	p = *treep;
	if ((p->op==INCAFT||p->op==DECAFT)
/***/	 && p->tr1->op==NAME && p->tr1->class==REG) {
		return(1+rcexpr(p->tr1, table, reg));
	}
	p1 = 0;
	if (opdope[p->op]&BINARY)
		p1 = sdelay(&p->tr2);
	if (p1==0)
		p1 = sdelay(&p->tr1);
	if (p1) {
		r = rcexpr(optim(p), table, reg);
		*treep = p1;
		return(r+1);
	}
	return(0);
}

sdelay(ap)
struct tnode **ap;
{
	register struct tnode *p, *p1;

	p = *ap;
/***/	if ((p->op==INCAFT||p->op==DECAFT) && p->tr1->op==NAME
/***/		&& p->tr1->class==REG) {
		*ap = ncopy(p->tr1);
		return(p);
	}
	if (p->op==STAR || p->op==PLUS)
		if (p1=sdelay(&p->tr1))
			return(p1);
	if (p->op==PLUS)
		return(sdelay(&p->tr2));
	return(0);
}

/*
 * Copy a tree node for a register variable.
 * Used by sdelay because if *reg-- is turned
 * into *reg; reg-- the *reg will in turn
 * be changed to some offset class, accidentally
 * modifying the reg--.
 */
ncopy(ap)
struct tname *ap;
{
	register struct tname *p, *q;

	p = ap;
	if (p->class!=REG)
		return(p);
	q = getblk(sizeof(*p));
	q->op = p->op;
	q->type = p->type;
	q->class = p->class;
	q->offset = p->offset;
	q->nloc = p->nloc;
	return(q);
}

/*
 * If the tree can be immediately loaded into a register,
 * produce code to do so and return success.
 */
chkleaf(atree, table, reg)
struct tnode *atree;
{
	struct tnode lbuf;
	register struct tnode *tree;

	tree = atree;
	if (tree->op!=STAR && dcalc(tree, nreg-reg) > 12)
		return(-1);
/***/	if (tree->op==AMPER)
/***/		return(-1);
	lbuf.op = LOAD;
	lbuf.type = tree->type;
	lbuf.degree = tree->degree;
	lbuf.tr1 = tree;
	return(rcexpr(&lbuf, table, reg));
}

/*
 * Compile a function argument.
 * If the stack is currently empty, put it in (sp)
 * rather than -(sp); this will save a pop.
 * Return the number of bytes pushed,
 * for future popping.
 */
comarg(atree)
{
	register struct tnode *tree;
	register retval;

	tree = atree;
/***/	if (tree->type == LONG)
/***/		nstack++;
/***/	nstack++;
/***/	rcexpr(tree, sptab, 0);
}

/*
 * Compile an initializing expression
 */
doinit(atype, atree)
struct tnode *atree;
{
	register struct tnode *tree;
	register int type;
	float sfval;
	double fval;
	long lval;
/***/	register val;

	tree = atree;
	type = atype;
	if (type==CHAR) {
/***/		printf("\tdb\t");
		if (tree->type&XTYPE)
			goto illinit;
	}
/***/	else
/***/		printf("\tdc\t");
	if (type&XTYPE)
		type = INT;
	switch (type) {
	case INT:
	case UNSIGN:
/***/	case SHORT:
/***/	case CHAR:
		if (tree->op==FTOI) {
			if (tree->tr1->op!=FCON && tree->tr1->op!=SFCON)
				goto illinit;
			tree = tree->tr1;
			tree->value = tree->fvalue;
			tree->op = CON;
		} else if (tree->op==LTOI) {
			if (tree->tr1->op!=LCON)
				goto illinit;
			tree = tree->tr1;
			lval = tree->lvalue;
			tree->op = CON;
			tree->value = lval;
		}
/***/		if (tree->op == CON) {
/***/			val = tree->value;
/***/			if (type==SHORT) {
/***/				if (val < -0x8000 || val > 0xffff)
/***/					goto illinit;
/***/				printf("x'%x'\n", val&0xffff);
/***/			} else {
/***/				if (type==CHAR) {
/***/					if (val < -0x80 || val > 0xff)
/***/						goto illinit;
/***/					val =& 0xff;
/***/				}
/***/				psoct(val);
/***/				putchar('\n');
/***/			}
/***/		}
/***/		else if (tree->op==AMPER && type!=SHORT && type!=CHAR) {
			pname(tree->tr1, 0);
			putchar('\n');
		} else
			goto illinit;
		return;

	case DOUBLE:
	case FLOAT:
		if (tree->op==ITOF) {
			if (tree->tr1->op==CON) {
				fval = tree->tr1->value;
			} else
				goto illinit;
		} else if (tree->op==FCON || tree->op==SFCON)
			fval = tree->fvalue;
		else if (tree->op==LTOF) {
			if (tree->tr1->op!=LCON)
				goto illinit;
			fval = tree->tr1->lvalue;
		} else
			goto illinit;
/***
		if (type==FLOAT) {
***/
			sfval = fval;
/***/			psoct(sfval);
/***/			putchar('\n');
/***
		} else
			printf("%o; %o; %o; %o\n", fval);
***/
		return;

/***
	case LONG:
		if (tree->op==FTOL) {
			tree = tree->tr1;
			if (tree->op==SFCON)
				tree->op = FCON;
			if (tree->op!= FCON)
				goto illinit;
			lval = tree->fvalue;
		} else if (tree->op==ITOL) {
			if (tree->tr1->op != CON)
				goto illinit;
			lval = tree->tr1->value;
		} else if (tree->op==LCON)
			lval = tree->lvalue;
		else
			goto illinit;
		printf("%o; %o\n", lval);
		return;
***/
	}
illinit:
	error("Illegal initialization");
}

/***
movreg(r0, r1, tree)
struct tnode *tree;
{
	register char *s;

	if (r0==r1)
		return;
	if (tree->type==LONG) {
		s = "mov	r%d,r%d\nmov	r%d,r%d\n";
		if (r0 < r1)
			printf(s, r0+1,r1+1,r0,r1);
		else
			printf(s, r0,r1,r0+1,r1+1);
		return;
	}
	printf("mov%c	r%d,r%d\n", isfloat(tree), r0, r1);
}
***/
movreg(rsource, rdest, tree)
struct tnode *tree;
{
	register r1, r2, modf;

	r1 = rdest;
	r2 = rsource;
	if ((modf = ishfloat(tree)) == 'e') {
		r1 =* 2;
		r2 =* 2;
	}
	printf("\tl%cr\tr%d,r%d\n", modf, r1, r2);
}

/***
 *** Adjust stack pointer up or down to ns
 ***/
setstk(ns)
{
	register op, offset;
	register char *s;

	op = 's';
	if ((offset = (ns-pstack)*NCPW) < 0) {
		op = 'a';
		offset = -offset;
	}
	s = (offset<16? "is" : (offset>>15? "ai" : "hi"));
	printf("\t%c%s\tsp,%d\n", op, s, offset);

	pstack = ns;
}
