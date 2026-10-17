/*
 *		Assembler expression evaluation
 *
 *
 *	Copyright (C) 1978, Richard Miller
 */

#define EXTERN	extern
#include "as.h"

struct exp	trm;		/* accumulator for expression terms */

/*
 * Evaluate an expression from the input line, leaving its value in exp
 *	- checks for undefined symbols
 *	- returns the relocatability of the expression
 */
expr()
{
	register op, val, rel, half;

	/*
	 * first term
	 */
	term();
	rel = trm.rel;
	val = trm.val;
	half = trm.half;
	/*
	 * check for further binary operators
	 */
	while ((op = token()) <= BINOP) {
		term();
		/*
		 * find relocatability of result
		 */
		rel = combine(rel, trm.rel, op);
		half &= trm.half;
		/*
		 * compute fullword value of result
		 */
		switch (op) {

		case PLUS:
			val += trm.val;
			break;
		case MINUS:
			val -= trm.val;
			break;
		case STAR:
			val *= trm.val;
			break;
		case SLASH:
			if (trm.val)
				val /= trm.val;
			else
				error(errz);
			break;
		case AND:
			val &= trm.val;
			break;
		case OR:
			val |= trm.val;
		}
	}
	nexttoken = op;
	exp.val = val;
	exp.half = half;
	if (rel == RUNDEF && pass > 0)
		error(erru);
	return(exp.rel = rel);
}

/*
 * Evaluate a single expression term from the input line, leaving its
 * value in trm
 */
term()
{
	register val, sign, half;

	half = 0;
	sign = 1;
    for (;;) {		/* loop through leading + or - signs */
	switch (token()) {

	/*
	 * leading sign(s)
	 */
	case MINUS:
		sign = -sign;
	case PLUS:
		continue;

	/*
	 * address constant
	 */
	case ZPAR:
		half = 1;
	case APAR:
		expr();
		trm.rel = exp.rel;
		val = exp.val;
		if (token() != RPAREN)
			xerror(errx);
		break;

	/*
	 * numeric constant
	 */
	case HCON:
		half = 1;
	case CON:
		trm.rel = RABS;
		val = conbuf;
		break;
	/*
	 * character constant -- right-justify into a word
	 */
	case STRING:
		{
			register i, n;

			if (strlen > 4)
				xerror(errq);
			n = 0;
			for (i = 0; i < strlen; i++) {
				n <<= 8;
				n |= strbuf[i];
			}
			trm.rel = RABS;
			val = n;
		}
		break;

	/*
	 * symbolic name -- get its value & relocation bits
	 */
	case SYMBOL:
	{
		register type, symno;

		val = cursym->value;
		switch (type = (cursym->type&SSEG)) {

		case SUNDEF:
			trm.rel = cursym->type&SEXT ?
				/* external reference */
				REXT | ((cursym-usymtab)<<4) :
				/* undefined symbol */
				RUNDEF;
			val = 0;
			break;

		case SCOMN:
			symno = cursym->type>>8;
			if ((type=usymtab[symno].type&SSEG)==SUNDEF)
				/* offset in common block */
				trm.rel = REXT | (symno<<4);
			else {
				/* offset from local symbol */
				trm.rel = (type-1)<<1;
				val += usymtab[symno].value;
			}
			break;

		default:
			/* translate symbol type to relocation bits */
			trm.rel = ((type&SSEG)-1)<<1;
		}
	}
	break;

	/*
	 * location counter
	 */
	case STAR:
		val = curseg->loc;
		trm.rel = currel;
		break;

	default:
		xerror(errx);
	}

	if (sign < 0) {
		if (trm.rel != RABS && trm.rel != RUNDEF)
			error(errr);
		val = -val;
	}

	trm.val = val;
	trm.half = half;
	return;
    }
}

/*
 * Determine relocatability of result of operation op performed on
 *	operands with relocatability rel1 and rel2
 *
 * Permitted combinations are:
 *	undefined with anything		->	undefined
 *	absolute with absolute		->	absolute
 *	relocatable + absolute		->	relocatable
 *	relocatable - absolute		->	relocatable
 *	absolute + relocatable		->	relocatable
 *	relocatable - relocatable	->	absolute (provided both are in
 *					same segment or in same common block)
 *
 * Note that external references are permitted to take part in expressions
 */
combine(rel1, rel2, op)
register rel1, rel2;
{
	register ret;

	if (rel1 == RUNDEF || rel2 == RUNDEF)
		ret = RUNDEF;
	else if (pass == 0 && (rel1 & REXT || rel2 & REXT))
		/* forward references in ENTRY statements may change types */
		ret = RUNDEF;

	else if (rel2 == RABS)
		if (rel1 == RABS)
			ret = RABS;
		else if (op <= MINUS)
			ret = rel1;
		else
			error(errr);

	else if (rel1 == RABS)
		if (op == PLUS)
			ret = rel2;
		else
			error(errr);

	else if (rel1 == rel2 && op == MINUS)
		ret = RABS;

	else
		error(errr);

	return(ret);
}
