/*
 *		Assembler Machine Instruction Processing
 *
 *
 *	Copyright (C) 1978, Richard Miller
 */

#define EXTERN	extern
#include "as.h"

/*
 * disassembled instruction parts
 */
int	reg1;
int	reg2;
int	reg3;
int	dval;
int	drel;

/*
 * Table of immediate instructions optimizable to SF format
 */
struct sftab {
	char op_ri1;	/* RI1-format opcode */
	char op_sf;	/* SF-format opcode */
	char op_csf;	/* complement SF-format opcode */
} sftab[] {
	0xca, 0x26, 0x27,	/* ahi - ais - sis */
	0xcb, 0x27, 0x26,	/* shi - sis - ais */
	0xc8, 0x24, 0x25,	/* lhi - lis - lcs */
	0xec, 0x10, 0,		/* srl - srls */
	0xed, 0x11, 0,		/* sll - slls */
	0xcc, 0x90, 0,		/* srhl - srhls */
	0xcd, 0x91, 0,		/* slhl - slhls */
	0
};

int	nsquez;			/* no. of bytes squeezed out in pass 1 */

/*
 * RR- or SF-type instruction
 */
dorr(op)
{
	align(2);
	label();
	getr1(op);
	reg2 = regexpr();
	puth(((op|reg1)<<4) | reg2, RABS);
}

/*
 * RI1-type instruction
 */
dori1(op)
{
	register nindex, sqz;

	sqz = (pass <= 1? 0 : getsqz());
	align(2);
	label();
	getr1(op);
	if ((nindex = geta()) > 1)
		error(errx);
	if (passg && (dval < 0xffff8000 || dval > 0xffff))
		error(errh);
	genri1(op, nindex, sqz);
}

/*
 * RI2-type instruction
 */
dori2(op)
{
	register nindex, sqz;

	sqz = (pass <= 1? 0 : getsqz());
	align(2);
	label();
	getr1(op);
	if ((nindex = geta()) > 1)
		error(errx);
	/*
	 * Shrink RI2 to RI1
	 */
	if (pass && drel==RABS && dval>=0xffff8000 && dval<=0x7fff)
		if (sqz <= 2 && opt > 0) {
			genri1(op - 0x300, nindex, sqz);
			if (pass == 1)
				nsquez += 2;
			return;
		}
	/*
	 * Use full RI2 form
	 */
	if (pass)
		setsqz(3);
	puth(((op|reg1)<<4) | reg2, RABS);
	putw(dval, drel);
}

/*
 * RX-type instruction
 */
dorx(op)
{
	register nindex, sqz;

	sqz = (pass <= 1? 0 : getsqz());
	align(2);
	label();
	getr1(op);
	nindex = geta();
	puth(((op|reg1)<<4) | reg2, RABS);
	genrx(nindex, sqz);
}

/*
 * RR-type branch
 */
dobrr(op)
{
	align(2);
	label();
	reg2 = regexpr();
	puth((op<<4) | reg2, RABS);
}

/*
 * SF-type branch
 */
dobsf(op)
{
	register off;

	align(2);
	label();
	if (geta() != 0)
		error(errx);

	off = dval - curseg->loc;
	if (currel == RDATA)
		off -= hdr.tsize;
	if (passg)
		if (drel != currel || off < -30 || off > 30)
			error(errb);
	genbsf(op, off);
}

/*
 * RX-type branch
 */
dobrx(op)
{
	register nindex, off, sqz;

	sqz = (pass <= 1? 0 : getsqz());
	align(2);
	label();
	nindex = geta();

	/*
	 * Shrink BRX to BSF
	 */
	off = dval - curseg->loc;
	if (currel == RDATA)
		off -= hdr.tsize;
	if (pass && nindex == 0 && drel == currel
	    && off >= -30 && off <= 30+nsquez)
		if (sqz <= 1 && opt > 0) {
			genbsf(op&0x10 ? op-0x210 : op-0x220, off);
			if (pass==1) {
				setsqz(1);
				nsquez += 4;
			}
			return;
		}
	puth((op<<4) | reg2, RABS);
	genrx(nindex, sqz);
}

/*
 * Stupid special case: NOP and NOPR instructions
 *	address operand may be left out
 */

#define NOP	0x42
#define NOPR	0x02

donop(op)
{
	align(2);
	label();

	switch (op) {

	case NOPR<<4:
		puth((NOPR<<8) | (eol()? 0 : regexpr()), RABS);
		break;

	case NOP<<4:
		if (eol())
			putw(NOP<<24, RABS);
		else {
			register sqz, nindex;

			sqz = (pass <= 1? 0 : getsqz());
			nindex = geta();
			puth((NOP<<8) | reg2, RABS);
			genrx(nindex, sqz);
		}
	}
}

/*
 * Generate destination field of RX instruction
 */
genrx(nindex, sqz)
{
	register off;

	/*
	 * Try to shrink instruction
	 */
	if (pass && nindex < 2 && sqz <= 2) {
		/*
		 * Shrink RX3 to RX1
		 */
		if (drel==RABS && dval>=0 && dval<=0x3fff && opt != 0) {
			puth(dval, drel);
			setsqz(2);
			if (pass == 1)
				nsquez += 2;
			return;
		}
		/*
		 * Shrink RX3 to RX2
		 */
		off = dval - (curseg->loc+2);
		if (currel == RDATA)
			off -= hdr.tsize;
		if (drel == currel && opt != 0
		  && off>=0xffffc000 && off<=0x3fff) {
			puth((off&0x7fff)|0x8000, RABS);
			if (passl)
				lstaddr(dval, drel);
			setsqz(2);
			if (pass == 1)
				nsquez += 2;
			return;
		}
	}
	/*
	 * Use full RX3 format
	 */
	if (pass)
		setsqz(3);
	puth(((0x40|reg3)<<8) | ((dval>>16)&0xff), drel|RHI);
	puth(dval&0xffff, 0);	/* second reloc=0 means 24-bit address */
}

/*
 * Generate destination field of RI2 instruction
 */
genri1(op, nindex, sqz)
{
	register rop, pop;
	register struct sftab *p;

	/*
	 * Shrink RI1 to SF
	 */
	if (pass && drel==RABS && nindex==0 && sqz <= 1 && opt > 0) {
		rop = op>>4;
		for (p = sftab; pop = p->op_ri1; p++)
			if (pop == rop) {
				if (dval>=0 && dval<=15) {
					puth((p->op_sf<<8)|(reg1<<4)|dval, RABS);
					if (pass == 1) {
						setsqz(1);
						nsquez += 2;
					}
					return;
				}
				if ((pop = p->op_csf) && dval<0 && dval >= -15) {
					puth((pop<<8)|(reg1<<4)|(-dval), RABS);
					if (pass == 1) {
						setsqz(1);
						nsquez += 2;
					}
					return;
				}
				break;
			}
	}
	/*
	 * Use full RI1 format
	 */
	if (pass)
		setsqz(2);
	puth(((op|reg1)<<4) | reg2, RABS);
	puth(dval, drel);
}

/*
 * Generate a short branch instruction
 */
genbsf(op, off)
{
	if (off < 0)
		off = -off;
	else
		op += 0x10;

	puth((op<<4) | (off>>1), RABS);
	if (passl)
		lstaddr(dval, drel);
}

/*
 * Get first operand from instruction
 */
getr1(op)
{
	reg1 = 0;
	sscan();		/* save scan pointer for backtracking */
	expr();
	if (token() != COMMA) {
		/*
		 * Stupid special case instructions
		 *	reg1 operand is optional
		 */
		switch (op) {
			case 0xd50:	/* al	*/
			case 0xc20:	/* lpsw	*/
			case 0x180:	/* lpswr*/
			case 0xe20:	/* sint	*/
			case 0xe00:	/* ts	*/
				/*
				 * Reset scan pointer to parse expression again
				 *	as second operand, and use default reg 0
				 */
				rscan();
				return;

			default:
				xerror(errx);
		}
	}
	if (pass && (exp.rel != RABS || (reg1 = exp.val) & ~0xf))
		error(errv);
}

/*
 * Get second operand from instruction
 *	- returns number of index registers
 */
geta()
{
	register t, nreg;

	reg2 = reg3 = 0;
	drel = expr();
	dval = exp.val;

	/*
	 * relocate
	 */
	switch (drel & RSEG) {
	case RBSS:
		dval += hdr.dsize;
	case RDATA:
		dval += hdr.tsize;
	}
	if ((t = token()) != LPAREN) {
		nexttoken = t;
		return(0);
	}
	reg2 = regexpr();
	if ((t = token()) != COMMA)
		nreg = 1;
	else {
		reg3 = regexpr();
		nreg = 2;
		t = token();
	}
	if (t != RPAREN)
		xerror(errx);
	return(nreg);
}

/*
 * Evaluate an expression which is expected to return
 *	a 4-bit absolute value
 */
regexpr()
{
	register ret;
	register r;

	ret = 0;
	r = expr();
	if (pass && (r != RABS || (ret = exp.val) & ~0xf))
		error(errv);
	return(ret);
}

/*
 * SQUEZ 'optimization' routines
 *
 *	During pass 1, while final values of labels are being calculated, an
 * attempt is made to shrink instructions to shorter equivalent forms:
 *		RI2 -> RI1 -> SF
 *		RX branch -> SF branch
 *		RX3 -> RX2 or RX1
 *	Since labels must not move during the code generation pass, the lengths
 * of all squeezable instructions are recorded in a bitmap (or, more precisely,
 * a two-bit map: each pair of bits gives the length in halfwords of the
 * corresponding instruction).  In pass 2, instructions are squeezed only if
 * the bitmap shows that they were squeezed during pass 1.
 *	In pathological cases, the operand of a squeezable instruction may
 * move out of range during subsequent squeezing.  If pass 2 finds that an
 * instruction has grown longer, the 'passg' flag is reset, causing assemble()
 * to repeat the code generation pass.
 *
 */

#define SQSIZE	512			/* allocation increment for bitmap */
char	*squezmap;		/* bitmap of squeezable instructions */
char	*squeztop;		/* end of bitmap */
char	*pbits;			/* current byte position in bitmap */
int	sbits;			/* current bit position in *pbits */
int	lastsq;			/* last squeeze bits returned */
char	*nocore = "Out of squeeze space\n";

/*
 * Allocate squeeze bitmap at end of symbol table
 *	- called at the beginning of pass 1
 */
sqzalloc()
{

	squezmap = (char *)nextsym;
	squeztop = (char *)symtop;
	if (squeztop <= squezmap && brk(squeztop += SQSIZE) < 0) {
		printf(nocore);
		exit(1);
	}
}

/*
 * Initialize for squeeze pass
 */
sqzinit()
{
	pbits = squezmap;
	sbits = 6;
	nsquez = 0;
}

/*
 * Return next sequential pair of squeeze bits
 */
getsqz()
{
	register b;

	b = (*pbits>>sbits)&03;
	if ((sbits -= 2) < 0) {
		pbits++;
		sbits = 6;
	}
	return(lastsq = b);
}

/*
 * Save the next sequential pair of squeeze bits
 */
setsqz(b)
{
	register p;

	if (pass > 1) {
		if (b == lastsq)
			return;
		/*
		 * instruction length has changed - need another pass 2
		 */
		passg = 0;
		if ((sbits += 2) > 6) {
			pbits--;
			sbits = 0;
		}
	}
	p = *pbits & ~(03<<sbits);
	*pbits = p | (b<<sbits);
	if ((sbits -= 2) < 0) {
		sbits = 6;
		if (++pbits >= squeztop && brk(squeztop += SQSIZE) < 0) {
			printf(nocore);
			exit(1);
		}
	}
}
