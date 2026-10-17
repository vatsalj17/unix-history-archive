/*
   Jonathan Payne at Lincoln-Sudbury Regional High School 5-25-83

   jove_ttout.c

   Deals with output to the terminal, setting up the amount of characters
   to be buffered depending on the output baud rate.  Why it's in a 
   separate file I don't know ... */

#include "jove.h"
#include "termcap.h"

IOBUF	termout;

outc(c)
register int	c;
{
	outchar(c);
}

/* Put a string with padding */

putpad(str, lines)
char	*str;
{
	tputs(str, lines, outc);
}

/* Flush the output, and check for more characters.  If there are
 * some, then return to main, to process them, aborting redisplay.
 */

flushout(x, p)
IOBUF	*p;
{
	register int	n;

	CheckTime = 1;
	if ((n = p->io_ptr - p->io_base) > 0) {
		ignore(write(p->io_file, p->io_base, n));
		if (p == &termout) {
			CheckTime = BufSize;
			p->io_cnt = BufSize;
		} else
			p->io_cnt = BUFSIZ;
		p->io_ptr = p->io_base;
	}
	if (x >= 0)
		Putc(x, p);
}

/* Determinte the number of characters to buffer at each
 * baud rate.  The lower the number, the quicker the
 * response when new input arrives.  Of course the lower
 * the number, the more prone the program is to stop in
 * output.  Decide what matters most to you.
 * This sets the int BufSize to the right number or chars,
 * allocates the buffer, and initiaizes `termout'.
 */

settout()
{
	static int speeds[] = {
		1,	/* 0	*/
		1,	/* 50	*/
		1,	/* 75	*/
		1,	/* 110	*/
		1,	/* 134	*/
		1,	/* 150	*/
		1,	/* 200	*/
		1,	/* 300	*/
		1,	/* 600	*/
		5,	/* 1200 */
		15,	/* 1800	*/
		30,	/* 2400	*/
		60,	/* 4800	*/
		120,	/* 9600	*/
		0,	100,	/* EXTA	*/
		0,	120	/* EXT	*/
	};

	termout.io_cnt = BufSize = CheckTime = speeds[ospeed] * max(LI / 24, 1);
	termout.io_base = termout.io_ptr = emalloc(BufSize);
	termout.io_flag = 0;
	termout.io_file = 1;	/* Standard output */
}
