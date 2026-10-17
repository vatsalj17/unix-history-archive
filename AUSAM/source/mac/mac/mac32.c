#include	"mac.h"
#include	"mac.x"

/*
 *   Auxilliary routines #2
 */


/*
 *   Convert ascii number to binary pointed to by 'p'.
 */
argnum()
{
	register int n;
	register int j;

	n = 0;

	if (*p == '0')  {
		p++;

		if (*p == 'x')  {
			p++;
			while ((j = any(*p, hextab)) >= 0)  {
				n = (n<<4) | j;
				p++;
				}
			return(n);
			}

		if (*p == 'b')  {
			p++;
			while (*p == '0' || *p == '1')
				n = (n<<1) | (*p++ - '0');
			return(n);
			}

		while ((j = any(*p, octtab)) >= 0)  {
			n = (n<<3) | j;
			p++;
			}
		return(n);
		}

	while ((j = any(*p, dectab)) >= 0)  {
		n = n * 10 + j;
		p++;
		}

	return(n);
}

/*
 *   Comparison for searching.
 *
 *	return  1:  *r > *s,
 *		0:  *r = *s,
 *	       -1:  *r < *s.
 */
cmp(r, s)
register char *r;
register char *s;
{
	register int i;


	/*
	 *   fast compare
	 */
	if (*r < *s)
		return(-1);
	if (*s < *r)
		return(1);

	i = 0;
	while (i < 8)  {
		r++;
		s++;

		if (*r < *s)
			return(-1);
		if (*s < *r)
			return(1);
		if (!*r)
			return(0);		/* equal on nul */
		i++;
		}
	return(0);
}


/*
 *   printl:  like prf1 but with leading zero's.
 */
printl(width, data)
register int width;
register int data;
{
	register char *r;
	register int i;
	register int fmt;
	register int mask;

	if (!data)  {
		for (i=0; i<width; i++)
			putchar('0');
		return;
		}

	r = buf;
	if (OPTION('h'))  {
		fmt = 'h';		/* hex */
		mask = BITMASK(WORDSIZ - 4);
		}
	if (OPTION('o'))  {
		fmt = 'o';		/* oct */
		mask = BITMASK(WORDSIZ - 3);
		}
	if (OPTION('b'))  {
		fmt = 'b';		/* bin */
		mask = BITMASK(WORDSIZ - 1);
		}

	i = width;
	while (i--)  switch (fmt)  {

		case 'd':
		case 'h':
			*r++ = hextab[data & 0x0f];
			data = (data >> 4) & mask;
			break;

		case 'o':
			*r++ = (data & 07) + '0';
			data = (data >> 3) & mask;
			break;

		case 'b':
			*r++ = (data & 01) + '0';
			data = (data >> 1) & mask;
			break;

		}

	r = &buf[width-1];
	for (i=0; i<width; i++)
		putchar(*r--);

	return;
}
