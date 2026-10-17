/*
 *		Assembler output & location counter routines
 *
 *
 *	Copyright (C) 1978, Richard Miller
 */

#define EXTERN	extern
#include "as.h"

/*
 * Initialize segments (at the beginning of each pass)
 */
seginit()
{
	impure.loc = impure.maxloc = pure.loc = pure.maxloc =
		bss.loc = bss.maxloc = 0;
	curseg = &impure;
	currel = RDATA;
}

/*
 * Write out the a.out header and initialize output buffers
 *	(at the beginning of code-generation pass(es))
 */
outhdr()
{
	register n;

	if (pure.maxloc < pure.loc)
		pure.maxloc = pure.loc;
	if (impure.maxloc < impure.loc)
		impure.maxloc = impure.loc;
	if (bss.maxloc < bss.loc)
		bss.maxloc = bss.loc;

	hdr.mword = 0407;
	hdr.tsize = (pure.maxloc+03)&~03;
	hdr.dsize = (impure.maxloc+03)&~03;
	hdr.bsize = (bss.maxloc+03)&~03;
	hdr.ssize = 16*(nextsym - usymtab);
	write(ofile, &hdr, sizeof hdr);

	pure.tseek = n = sizeof hdr;
	impure.tseek = (n += hdr.tsize);
	pure.rseek = (n += hdr.dsize);
	impure.rseek = (n += hdr.tsize);
	symseek = (n += hdr.dsize);

	pure.nchar = impure.nchar = 0;
}

/*
 * Emit a string of <len> characters
 */
puts(len)
register len;
{
	register struct segment *sp;
	register i, n;

	if ((sp = curseg) != &impure && sp != &pure)
		xerror(errd);
	sp->loc += len;

	if (passg) {
		n = sp->nchar;
		for (i = 0; len--; i++) {
			if (n >= OBSIZE) {
				sp->nchar = n;
				oflush(sp);
				n = 0;
			}
			sp->tbuf[n] = strbuf[i];
			sp->rbuf[n++] = RABS;
		}
		sp->nchar = n;
	} else if (passl)
		for (i = 0; len--; i++)
			lstb(strbuf[i]);
}

/*
 * Emit a word
 */
putw(val, rel)
struct { short halfw[2]; } val;
{
	puth(val.halfw[0], rel==RABS? rel : rel|RHI);
	puth(val.halfw[1], rel);
}

/*
 * Emit a halfword
 *	- current location counter must be even
 */
puth(val, rel)
short val;
{
	register struct segment *sp;
	register n;

	if ((sp = curseg) != &pure && sp != &impure)
		xerror(errd);
	sp->loc += 2;

	if (passg) {
		if ((n = sp->nchar) >= OBSIZE) {
			oflush(sp);
			n = 0;
		}
		sp->nchar = n+2;
		n >>= 1;
		((short *)sp->tbuf)[n] = val;
		((short *)sp->rbuf)[n] = rel;
	}
	if (passl)
		lsth(val);
}

/*
 * Emit a byte
 */
putb(val)
{
	register struct segment *sp;
	register n;

	if ((sp = curseg) != &impure && sp != &pure)
		xerror(errd);
	sp->loc++;

	if (passg) {
		if ((n = sp->nchar) >= OBSIZE) {
			oflush(sp);
			n = 0;
		}
		sp->tbuf[n] = val;
		sp->rbuf[n] = RABS;
		sp->nchar = ++n;
	}
}

/*
 * Define a label at the current location
 */
label()
{
	if (passl)
		lstloc();
	if (curlab)
		deflab(currel, curseg->loc, 0);
}

/*
 * Align generated code to the specified boundary by padding with zeroes
 */
align(boundary)
{
	register struct segment *sp;
	register n;

	n = boundary - 1;
	if ((sp = curseg) != &pure && sp != &impure) {
		sp->loc = (sp->loc + n) & ~n;
		return;
	}
	while (sp->loc&n)
		putb(0);
}

/*
 * Move the current location counter to the specified location
 *	- done by repositioning seek pointers, not padding with zeroes,
 *	  to avoid overwriting previously generated code
 */
org(loc)
register loc;
{
	register struct segment *sp;
	register off;

	sp = curseg;
	if ((off = loc-curseg->loc) == 0)
		return;
	if (loc & 0xff000000)
		xerror(erro);
	if (sp->maxloc < sp->loc)
		sp->maxloc = sp->loc;
	sp->loc = loc;

	if (passg && (sp == &impure || sp == &pure)) {
		oflush(sp);
		sp->tseek += off;
		sp->rseek += off;
		sp->nchar = sp->tseek & 01;	/* keep proper alignment */
	}
}

/*
 * Flush the output buffer for the specified segment
 */
oflush(sp)
register struct segment *sp;
{
	register n;
	register off;

	/*
	 * If text in buffer begins on an odd byte boundary, adjust offset
	 *	and length for writes
	 */
	off = sp->tseek & 01;
	if ((n = sp->nchar-off) == 0)
		return;

	seek(ofile, sp->tseek, 0);
	write(ofile, sp->tbuf+off, n);
	seek(ofile, sp->rseek, 0);
	write(ofile, sp->rbuf+off, n);
	sp->tseek += n;
	sp->rseek += n;
}

/*
 * Define the symbol in curlab to have the given relocatability and value
 *	- converts relocation bits to symbol type
 *	- checks for illegal redefinitions
 *	- if flag==1, redefinition is permitted (used mainly to compensate
 *	  for bug in Interdata Fortran V
 *	- redefinition of absolute symbols is always permitted
 */
deflab(rel, val, flag)
{
	register type, otype;

	/*
	 * convert relocation bits to symbol type
	 */
	if (rel&REXT)
		type = SCOMN | ((rel & ~RSEG)<<4);
	else
		type = (rel>>1)+1;

	/*
	 * check for illegal redefinitions
	 */
	if (!flag) {
		if ((otype = curlab->type&SSEG) == SCOMN)
			otype = curlab->type;
		if (pass == 0) {
			if (otype!=SUNDEF && (otype!=SABS || type!=SABS))
				error(errm);
		}
		else if (otype != type)
			error(errm);
		else if (passg && type != SABS && curlab->value != val)
			error(errp);
	}

	curlab->type = (curlab->type & SEXT) | type;
	curlab->value = val;
}
