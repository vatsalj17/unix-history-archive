#
/*
 */

#include "param.h"
#include "seg.h"
#include "buf.h"
#include "conf.h"

/*
 * In case console is off,
 * panicstr contains argument to last
 * call to panic.
 */

char	*panicstr;

/*
 * Scaled down version of C Library printf.
 * Only %s %l %d (==%l) %x are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 * Since it is not interrupt driven,
 * all system activities are pretty much
 * suspended.
 * Printf should not be used for chit-chat.
 */
printf(fmt,x1,x2,x3,x4,x5,x6,x7,x8,x9,xa,xb,xc)
char fmt[];
{
	register char *s;
	register *adx, c;

	adx = &x1;
loop:
	while((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(c);
	}
	c = *fmt++;
	if(c == 'd' || c == 'l' || c == 'x')
		printn(*adx, c=='x'? 16: 10);
	if(c == 's') {
		s = *adx;
		while(c = *s++)
			putchar(c);
	}
	adx++;
	goto loop;
}

/*
 * Print an unsigned integer in base b.
 */
printn(n, b)
{
	char pbuff[16];
	register char *p;
	register unsigned a;
	static char hexdigits[] "0123456789ABCDEF";

	a = n;
	p = pbuff;
	do {
		*p++ = hexdigits[a%b];
	} while (a =/ b);
	do {
		putchar(*--p);
	} while (p > pbuff);
}

/*
 * Print a character on console.
 * Attempts to save and restore device
 * status.
 * If the switches are 0, all
 * printing is inhibited.
 */
/*** note:	this routine has been rewritten in assembler ***/

/*
 * Panic is called on unresolvable
 * fatal errors.
 * It syncs, prints "panic: mesg" and
 * then loops.
 */
panic(s)
char *s;
{
	panicstr = s;
	update();
	printf("panic: %s\n", s);
	spl(7);			/***/
	for(;;)
		idle();
}

/*
 * prdev prints a warning message of the
 * form "mesg on dev x/y".
 * x and y are the major and minor parts of
 * the device argument.
 */
prdev(str, dev)
{

	printf("%s on dev %l/%l\n", str, dev.d_major, dev.d_minor);
}

/*
 * deverr prints a diagnostic from
 * a device driver.
 * It prints the device, block number,
 * and a hex word (usually some error
 * status register) passed as argument.
 */
deverror(bp, o1, o2)
int *bp;
{
	register *rbp;

	rbp = bp;
	prdev("err", rbp->b_dev);
	printf("bn %l st %x %x\n", rbp->b_blkno, o1, o2);
}
