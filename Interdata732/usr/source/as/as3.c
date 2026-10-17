/*
 *		Assembler pseudo-op processing
 *
 *
 *	Copyright (C) 1978, Richard Miller
 */

#define EXTERN	extern
#include "as.h"

/*
 * Ignored opcode
 */
donull()
{
	nexttoken = EOL;
}

/*
 * Illegal opcode
 */
dobad()
{
	xerror(errc);
}

/*
 * Data definition pseudo-ops
 */

#define DB	0
#define DC	1
#define DAC	2
#define DCF	3
#define DCX	4
#define DCY	5
#define DCZ	6

dodata(op)
{
	register t;

	dcflag = 1;		/* a()-type constants are legal */
	switch (op) {

	case DC:
	case DCX:
	case DCZ:
		align(2);
		break;

	case DAC:
	case DCF:
	case DCY:
		align(4);
		break;

	case DB:
		/*
		 * Stupid special case:
		 *	"DB *" is equivalent to "ALIGN 2"
		 */
		if ((t = token()) == STAR && eol()) {
			label();
			align(2);
			return;
		}
		nexttoken = t;
	}
	label();

	do switch (op) {

		/*
		 * fullword or halfword constant
		 */
		case DC:
		case DAC:
		case DCF:
		case DCZ:
			if ((t = token()) == STRING)
				puts((strlen+01)&~01);
			else {
				nexttoken = t;
				expr();
				/*
				 * relocate
				 */
				switch (exp.rel) {

				case RBSS:
					exp.val += hdr.dsize;
				case RDATA:
					exp.val += hdr.tsize;
				}
				if (exp.half || op == DCZ) {
					if (passg && (exp.val < 0xffff8000
					    || exp.val > 0xffff))
						error(errh);
					puth(exp.val, exp.rel);
				} else
					putw(exp.val, exp.rel);
			}
			continue;

		/*
		 * Hex constant
		 */
		case DCX:
			t = getnum(16);
			if (t < 0xffff8000 || t > 0xffff)
				error(errh);
			puth(t, RABS);
			continue;

		case DCY:
			t = getnum(16);
			putw(t, RABS);
			continue;

		/*
		 * Byte constant
		 */
		case DB:
			if ((t = token()) == STRING && strlen > 1) {
				puts(strlen);
				continue;
			}
			nexttoken = t;
			t = expr();
			if (passg)
				if (t != RABS
				    || exp.val < -127 || exp.val > 255)
					error(errv);
			putb(exp.val);
			if (passl)
				lstb(exp.val);
			continue;

		}
	while ((t = token()) == COMMA);

	nexttoken = t;
}

/*
 * Location counter pseudo-ops
 */

#define IMPURE	0
#define PURE	1
#define ALIGN	2
#define DS	3
#define DSF	4
#define ORG	5
#define COMN	6
#define STRUC	7
#define ENDS	8
#define BSS	9
#define BDATA	10
#define BORG	11
#define DSH	12
#define DLIST	13
#define CNOP	14

#define NOPR	0x0200		/* NO-OP instruction */

doloc(op)
{
	static struct segment *saveseg;		/* previous segment (while in
						   a STRUC or COMN */
	static int saverel;			/* previous relocatability */

	register t;
	register struct symbol *blk;

	switch (op) {

	case IMPURE:
		label();
		curseg = &impure;
		currel = RDATA;
		align(2);
		nexttoken = EOL;
		break;

	case PURE:
		label();
		curseg = &pure;
		currel = RTEXT;
		align(2);
		nexttoken = EOL;
		break;

	case BSS:
		label();
		curseg = &bss;
		currel = RBSS;
		align(2);
		nexttoken = EOL;
		break;

	case ALIGN:
	case CNOP:
		if (expr() != RABS)
			xerror(erra);
		if ((t = exp.val) != 2 && t != 4 && t != 8)
			t = 4;
		if (op == ALIGN) {
			align(t);
			label();
			break;
		}
		align(2);
		label();
		while (curseg->loc & (t-1))
			puth(NOPR, RABS);
		break;

	case DSF:
		align(4);
	case DSH:
		align(2);
	case DS:
		label();
		if (expr() != RABS)
			xerror(erra);
		if ((t = exp.val) < 0) {
			t = 0;
			error(errv);
		}
		if (op == DSF)
			t *= 4;
		else if (op == DSH)
			t *= 2;
		org(curseg->loc + t);
		break;

	case DLIST:
		if (expr() != RABS)
			error(erra);
		if ((t = exp.val) < 0) {
			t = 0;
			error(errv);
		}
		align(4);
		label();
		puth(t, RABS);
		puth(0, RABS);
		putw(0, RABS);
		org(curseg->loc + t*4);
		break;

	case ORG:
		label();
		switch (t = expr()) {
			case RTEXT:
				curseg = &pure;
				break;
			default:
				if (t == currel)
					break;
				error(errr);
				t = RDATA;
			case RDATA:
				curseg = &impure;
				break;
		}
		currel = t;
		org(exp.val);
		break;

	case COMN:
		if (curcomm)
			xerror(erri);
		if (!curlab)
			xerror(errl);
		curcomm = curlab;
		if ((t = curlab->type&SSEG) != SUNDEF &&
		    (t != SDATA || (bdata && pass == 0)))
			error(errm);
		curlab->type = SEXT | (bdata? SUNDEF : t);

		saverel = currel;
		saveseg = curseg;
		currel = REXT | ((curlab-usymtab)<<4);
		curseg = &comm;
		nexttoken = EOL;
		break;

	case STRUC:
		if (curcomm)
			error(erri);
		if (curlab) {
			curcomm = curlab;
			deflab(RABS, 0, 0);
		}
		else
			curcomm = -1;

		saverel = currel;
		saveseg = curseg;
		currel = RABS;
		curseg = &comm;
		nexttoken = EOL;
		break;

	case ENDS:
		if (!curcomm)
			xerror(erri);
		if (comm.loc > comm.maxloc)
			comm.maxloc = comm.loc;
		if ((currel != RABS && curcomm->type == (SEXT|SUNDEF))
		    || (currel == RABS && curcomm != -1))
			curcomm->value = comm.maxloc;
		/*
		 * Block Data program: change common to data
		 */
		if (bdata && currel != RABS) {
			curseg = &impure;
			currel = RDATA;
			curcomm->type = SDATA | SEXT;
			curcomm->value = impure.loc;
			org(impure.loc + comm.maxloc);
		}
		currel = saverel;
		curseg = saveseg;
		curcomm = 0;
		comm.loc = comm.maxloc = 0;
		nexttoken = EOL;
		break;

	case BDATA:
		bdata++;
		mflag++;	/* allow redefinitions in Fortran blkdata */
		nexttoken = EOL;
		break;

	case BORG:
		if (token() != SYMBOL)
			xerror(errs);
		blk = cursym;
		if (token() != LPAREN)
			xerror(errs);
		if (blk->type != (SDATA|SEXT))
			xerror(errr);
		switch (t = expr()&RSEG) {
			case RABS:
			case REXT:
				t = blk->value;
				break;
	
			case RDATA:
				t = 0;
				break;
	
			default:
				xerror(errr);
		}
		if (token() != RPAREN)
			xerror(errx);
		currel = RDATA;
		curseg = &impure;
		org(t + exp.val);
		break;
	}
}

/*
 * Other pseudo-ops
 */

#define EQU	0
#define EXTRN	1
#define ENTRY	2
#define DO	3
#define SQUEZ	4
#define NOSQZ	5
#define NORX3	6

dopseud(op)
{
	register t;
	register t1, t2;

	switch(op) {

	/*
	 * EQUate
	 */
	case EQU:
		if (!curlab) {
			xerror(errl);
		}
		expr();
		deflab(exp.rel, exp.val, mflag);
		if (passl)
			lstaddr(exp.val, exp.rel);
		break;

	case EXTRN:
	case ENTRY:
		do {
			if (!getsym())
				xerror(errs);
			else
				if (symlook(1))
					cursym->type |= SEXT;
		} while ((t = token()) == COMMA);
		nexttoken = t;
		break;

	case DO:
		label();
		docount = 0;
		if (expr() != RABS)
			error(erra);
		if ((t = exp.val) < 0)
			t = 0;
		if (!getline())
			xerror(erre);
		docount = t;
		break;

	case SQUEZ:
		opt = 1;
		break;

	case NOSQZ:
		opt = 0;
		break;

	case NORX3:
		opt = -1;
		break;
	}
}

/*
 * Conditional assembly pseudo-ops
 */

#define IFZ	0
#define IFNZ	1
#define IFP	2
#define IFNP	3
#define IFM	4
#define IFNM	5
#define IFE	6
#define IFO	7
#define IFU	8
#define IFD	9
#define IFCOND	9	/* last compound if */
#define ELSE	10
#define ENDC	11
#define IF	12
#define END	13

docond(op)
{
	register v, cond;

	label();
	/*
	 * Compound IF statements
	 */
	if (op <= IFCOND) {
		/* Evaluate expression -- must be previously defined */
		if (op < IFU) {
			if (expr() == RUNDEF)
				error(erru);
			v = exp.val;
		} else
			if (!getsym())
				error(errs);
			else
				v = symlook(1);

		/* Evaluate condition */
		switch (op) {
			case IFU:
			case IFZ:	cond = (v == 0);	break;
			case IFD:
			case IFNZ:	cond = (v != 0);	break;
			case IFP:	cond = (v > 0);		break;
			case IFNP:	cond = (v <= 0);	break;
			case IFM:	cond = (v < 0);		break;
			case IFNM:	cond = (v >= 0);	break;
			case IFE:	cond = !(v & 01);	break;
			case IFO:	cond = (v & 01);	break;
		}
		/* Adjust conditional assembly & nesting level */
		if (!cond || ifcount)
			ifcount++;
		ifnest++;
		return;
	}

	/*
	 * Other compound conditionals
	 */
	if (op != IF)
		nexttoken = EOL;
	switch (op) {
		case ELSE:
			if (!ifnest)
				error(erri);
			if (ifcount <= 1)
				ifcount = 1-ifcount;
			break;

		case ENDC:
			if (!ifnest)
				error(erri);
			if (ifcount)
				ifcount--;
			ifnest--;
			break;

		/*
		 * Simple IF
		 */
		case IF:
			if (expr() == RUNDEF)
				error(erru);
			noasm = !(exp.val);
			break;

		/*
		 * END statement - set switch for assemble()
		 */
		case END:
			endswitch = 1;
			break;
	}
}
