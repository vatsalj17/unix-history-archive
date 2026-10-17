/*
 *		Assembler listing routines
 *
 *
 *	Copyright (C) 1978, Richard Miller
 */

#define EXTERN	extern
#include "as.h"

/*
 * Buffer for listing
 */
struct {
	char	l_loc[5];
	char	l_locr;
	char	l_fill1[1];
	char	l_data[16];
	char	l_add[8];
	char	l_addr;
	char	l_fill3[1];
} lstbuf;
char	*listp;		/* current output position in l_data */

/*
 * Initialize listing buffer
 */
lstinit()
{
	register char	*p;
	register	n;

	p = &lstbuf;
	for (n = sizeof(lstbuf); n--; p++)
		*p = ' ';

	listp = &lstbuf.l_data[0];
}

/*
 * List location counter value
 */
lstloc()
{
	lstx(curseg->loc, 5, lstbuf.l_loc);
	lstbuf.l_locr = lstreloc(currel);
}

/*
 * List the operand address
 */
lstaddr(val, rel)
{
	lstx(val, 8, lstbuf.l_add);
	lstbuf.l_addr = lstreloc(rel);
}

/*
 * List a halfword of code/data
 */
lsth(val)
{
	if (listp < &lstbuf.l_data[19])
		lstx(val, 4, listp);
	listp += 5;
}

/*
 * List a byte of data
 */
lstb(val)
{
	register	n;

	if (listp < &lstbuf.l_data[19]) {
		for (n = listp-lstbuf.l_data; (n -= 5) > 0; )
			;
		if (n == -1)
			listp++;
		lstx(val, 2, listp);
	}
	listp += 2;
}

/*
 * Print out listing line
 */
putline()
{
	extern char	linebuf[];

	if (passl)
		printf("%.33s %5d %s", &lstbuf, line, linebuf);
}

/*
 * List number in hex with leading zeroes
 */
lstx(value, nibbles, buf)
char		*buf;
register	value;
{
	register char	*p;

	for (p = buf+nibbles; --p >= buf; ) {
		*p = "0123456789ABCDEF"[value&0xf];
		value >>= 4;
	}
}

/*
 * Convert relocation type to readable value
 */
lstreloc(rel)
{
	switch (rel&RSEG) {

		case RABS:
			return('a');

		case RTEXT:
			return('t');

		case RDATA:
			return('d');

		case RBSS:
			return('b');

		case REXT:
			return('U');

		default:
			return('?');
	}
}
