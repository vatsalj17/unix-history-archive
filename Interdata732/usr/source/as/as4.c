/*
 *		Assembler input & lexical scan routines
 *
 *
 *	Copyright (C) 1978, Richard Miller
 */

#define EXTERN	extern
#include "as.h"

/*
 * Character types
 */
#define	C_ALPHA		0x80	/* Alphabetic (i.e. legal within symbol) */
#define	C_UPPER		0x40	/* Upper-case alpha */
#define	C_HEXDIGIT	0x20	/* Hexadecimal digit [0-9 a-f A-F] */
#define	C_SPECIAL	0x00	/* Other special characters */
#define	C_VALUE		0x0f	/* C_HEXDIGIT - numerical value of digit
				   C_SPECIAL - lexical token for character */

/*
 * Character type table
 */
char ctab[128] {
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	SPACE,	EOL,	0,	EOL,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	SPACE,	OR,	0,	0,	0x80,	0,	AND,	0,
	LPAREN,	RPAREN,	STAR,	PLUS,	COMMA,	MINUS,	0x80,	SLASH,
	0x20,	0x21,	0x22,	0x23,	0x24,	0x25,	0x26,	0x27,
	0x28,	0x29,	0,	0,	0,	0,	0,	0,
	0x80,	0xea,	0xeb,	0xec,	0xed,	0xee,	0xef,	0xc0,
	0xc0,	0xc0,	0xc0,	0xc0,	0xc0,	0xc0,	0xc0,	0xc0,
	0xc0,	0xc0,	0xc0,	0xc0,	0xc0,	0xc0,	0xc0,	0xc0,
	0xc0,	0xc0,	0xc0,	0,	0,	0,	0,	0x80,
	0,	0xaa,	0xab,	0xac,	0xad,	0xae,	0xaf,	0x80,
	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,
	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,	0x80,
	0x80,	0x80,	0x80,	0,	0,	0,	0,	0
};

/*
 * Read next input line, truncating to 71 chars
 *	- resets scan pointer to start of line
 *	- initializes listing buffer
 *	- returns 0 at end-of-file, '\n' otherwise
 */

char	linebuf[74];		/* input line buffer */
char	*scanp;			/* input scan cursor */

getline()
{
	static char ibuf[512], *next;
	static int nleft;
	register char *nx; register nl;
	register char *p;
	register c, ncol;

	scanp = p = linebuf;
	lstinit();

	/*
	 * If previous statement was DO, decrement the count and re-use
	 *	the same buffer
	 */
	if (docount-- > 0)
		return('\n');
	else
		docount = 0;

	ncol = 71;
	for (nx = next, nl = nleft; ;) {
		if (--nl < 0) {
			while ((nl = read(0, ibuf, sizeof ibuf)-1) < 0)
				if (!nextfile())
					return(0);
			nx = ibuf;
		}
		if ((*p = *nx++) == '\n') {
			p[1] = '\0';
			next = nx;
			nleft = nl;
			line++;
			return('\n');
		}
		if (--ncol > 0)
			p++;
	}
}

/*
 * Return next lexical token from input line
 *	- if constant, value is in conbuf
 *	- if symbol, symbol pointer is in cursym
 */
token()
{
	register ret, c;
	register ctype;
	register char *p;

	if (ret = nexttoken) {
		nexttoken = 0;
		return(ret);
	}

	/*
	 * first character "alphabetic"
	 */
	p = scanp;
	if ((ctype = ctab[c = *p])&C_ALPHA) {
		/*
		 * quoted numerical constants
		 */
		if (p[1] == '\'') {
			scanp = (p += 2);
			ret = CON;
			switch (c) {

			case 'x':
			case 'X':
				ret = HCON;
			case 'y':
			case 'Y':
				conbuf = getnum(16);
				break;

			case 'h':
			case 'H':
				ret = HCON;
			case 'f':
			case 'F':
				conbuf = getnum(10);
				break;

			case 'c':
			case 'C':
				ret = STRING;
				conbuf = getstr();
				break;

			default:
				xerror(errx);
			}
			if (*scanp++ != '\'')
				xerror(errq);
			return(ret);
		}
		/*
		 * address constants
		 */
		if (dcflag && p[1] == '(') {
			scanp = (p += 2);
			switch (c) {

			case 'a':
			case 'A':
				return(APAR);
			case 'z':
			case 'Z':
				return(ZPAR);
			default:
				scanp -= 2;
			}
		}
		/*
		 * symbol
		 */
		getsym();
		symlook(2);
		return(SYMBOL);
	}
	/*
	 * first character numeric -- decimal constant
	 */
	if (ctype&C_HEXDIGIT) {
		conbuf = getnum(10);
		return(CON);
	}
	/*
	 * special characters
	 */
	scanp = ++p;
	switch (ctype &= C_VALUE) {

	/*
	 * garbage character
	 */
	case 0:
		xerror(errg);

	/*
	 * white space
	 */
	case SPACE:
		while ((c = *p) == ' ' || c == '\t')
			p++;
		scanp = p;
		return(SPACE);
	/*
	 * don't scan past end of line
	 */
	case EOL:
		scanp = --p;
		return(EOL);
	/*
	 * stupid special case: "//" is a legal label in a COMN statement
	 */
	case SLASH:
		if (*p == '/' && p == &linebuf[1]) {
			enter("//");
			scanp = ++p;
			return(SYMBOL);
		}
		return(SLASH);
	/*
	 * single-character tokens
	 */
	default:
		return(ctype);

	}
}

/*
 * Scan a decimal or hexadecimal number (with optional sign)
 *	 from input line
 */
getnum(base)
register base;
{
	register char *p;
	register c, n, sign;

	p = scanp;
	sign = 1;
	if ((c = *p) == '-') {
		sign = -1;
		c = *++p;
	}
	else if (c == '+')
		c = *++p;
	if (((c = ctab[c])&C_HEXDIGIT) == 0 || (n = c&C_VALUE) >= base)
		xerror(errn);
	while ((c = ctab[*++p])&C_HEXDIGIT && (c &= C_VALUE) < base) {
		n *= base;
		n += c;
	}
	scanp = p;
	return(n*sign);
}

/*
 * Scan a character string from input line until ending quote
 *	- translate pairs of quotes to single quote
 *	- pad string with extra blank
 *	- leave string in strbuf, and length in strlen
 */

getstr()
{
	register char *s, *p;
	register c;

	s = strbuf;
	for (p = scanp; ; p++) {
		if ((c = *p) == '\'') {
			if (p[1] == '\'')
				p++;
			else
				break;
		}
		else if (c == '\n')
			xerror(errq);
		*s++ = c;
	}

	*s = ' ';
	strlen = s - strbuf;
	scanp = p;
	return(strbuf);
}

/*
 * Scan a symbol from the input line into symbuf
 *	if -u mode, translate to lowercase
 *	truncate long symbols to 8 characters (without warning)
 *
 *	returns	0:	no legal symbol found
 *		1:	symbol found
 */
getsym()
{
	register c, ctype;
	register char *p, *s;

	p = scanp;
	if (!((ctype = ctab[c = *p]) & C_ALPHA))
		return(0);
	s = symbuf;
	((int *)s)[0] = ((int *)s)[1] = 0;
	do {
		if (s < &symbuf[sizeof symbuf]) {
			if (ucase && (ctype&C_UPPER))
				c += 'a'-'A';
			*s++ = c;
		}
	} while ((ctype = ctab[c = *++p])&(C_ALPHA|C_HEXDIGIT));

	scanp = p;
	return(1);
}

/*
 * Determine whether scan pointer is at end of line
 */
eol()
{
	register c;

	return((c = ctab[*scanp]) == EOL || c== SPACE);
}

/*
 * Save / restore scan pointer (for backtracking)
 */

static char	*scansave;

sscan()
{
	scansave = scanp;
}

rscan()
{
	scanp = scansave;
}
