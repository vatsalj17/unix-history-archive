/*	@(#)swab.c	2.1	SCCS id keyword	*/
/*
 * Swap bytes in 16-bit [half-]words
 */

swab(pf, pt, n)
register short *pf, *pt;
register n;
{

	n /= 2;
	while (--n >= 0) {
		*pt++ = (*pf << 8) + ((*pf >> 8) & 0377);
		pf++;
	}
}
