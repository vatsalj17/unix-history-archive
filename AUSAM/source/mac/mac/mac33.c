#include	"mac.h"
#include	"mac.x"

/*
 *   Auxilliary routines 3 (for pass 2)
 */


/*
 *   Assemble binary object in a field 'width' bits,
 *   having a value of 'value'.
 *
 *   The value is checked to fit into the space and
 *   truncation errors will result if it overflows.
 *   If the host machine has generated a negative
 *   number as the result of an expression -
 *   check this also by complementing the number,
 *   and then checking it.
 */
assemble(value, width)
register int value;
register int width;
{
	register int mask;
	register int free;
	register int r;
	static	 int	used;
	static	 int	buf;



	/*
	 *   Check for overflow errors.
	 *   (a value too large to fit in
	 *   the desired space.)
	 */
	if (value >= 0)  {
		mask = (-1) << width;
		if (value & mask)
			warning("assemble overflow");
		}
	else  {
		mask = (-1) << (width - 1);
		if (value < mask)
			warning("assemble overflow");
		}


	free = head.h_bu_len - used;			/* free bits in buf */
	while (width && width >= free)  {
		r = width - free;			/* remainder can't use yet */
		mask = BITMASK(free);
		buf = (buf << free) | ((value >> r) & mask);
		mask = BITMASK(r);			/* remainder mask */
		value =& mask;
		width =- free;
		*locn[lcntr].l_next++ = buf;		/* push out to core */
		buf  = 0;
		used = 0;				/* none of buf used */
		free = head.h_bu_len;
		}

	if (!width)
		return;

	/*
	 *   Take care of remainder.
	 */
	used =+ width;
	mask = BITMASK(width);
	buf = (buf << width) | (value & mask);

	if (used >= head.h_bu_len)
		used = 0;

	return;
}


/*
 *   Code formatter.
 *
 *   Decode and assemble binary data in memory
 *   according to the selected format descriptor.
 */
format(fmt, op)
register struct fd *fmt;
register int op;
{
	register int pr;
	register int rf;
	register int n;
	register int mask;
	register int i;

	if (op == head.h_ii)  {
		synerr("illegal instruction");
		return;
		}


	/*
	 *   Format descriptor loop.
	 *
	 *   Get each sub-section, decode it,
	 *   and assemble it's value into the instruction.
	 */
	for (i=0; fmt->f_desc[i] != 0; i++)  {

		pr = fmt->f_desc[i] & (RMODE | PMODE);
		n  = fmt->f_desc[i] & 0xff;

						/* opcode field */
		if (n == 'o')  {
			reloc = RABS;
			rf = fmt->f_width[i];
			mask = BITMASK(rf);
			assemble(op & mask, rf);
			continue;
			}

							/* pc field */
		if (n == '!')  {
			reloc = RREL;
			assemble(locn[lcntr].l_value, fmt->f_width[i]);
			continue;
			}

							/* arg field */
		if (n >= 'a' && n <= 'm')  {
			n = n - 'a' + 1;
			reloc = relstac[n];
			n = oprstac[n];
			if (pr & PMODE)  {
				/* pc relative */
				n =- locn[lcntr].l_value;
				}

			if (pr & RMODE)  {
				/* byte relocation */
				rf = fmt->f_value[i];
				mask = BITMASK(rf);
				n = ((n & mask) << rf) | ((n >> rf) & mask);
				}

			assemble(n, fmt->f_width[i]);
			continue;
			}

							/* constant field */
		if (n == '#')  {
			reloc = RABS;
			assemble(fmt->f_value[i], fmt->f_width[i]);
			continue;
			}

			/* next n bits of opcode */
		if (n == 'n')  {
			reloc = RABS;
			rf = fmt->f_width[i];
			mask = BITMASK(rf);
			assemble(op & mask, rf);
			op =>> rf;
			continue;
			}



		/*  should never happen - but ... */

		printf("corrupted format descriptor %c\n", n);
		exit(1);

		}

	return;
}
