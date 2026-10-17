#include	"mac.h"
#include	"mac.x"

/*
 *   Auxilliary routines 1.
 */


/*
 *   compar:	Compare two strings - return
 *		TRUE if equal, FALSE if not.
 *		terminate with FALSE if > 8 chars.
 */
compar(r, s)
register char *r;
register char *s;
{
	register int n;

	n = 1;
	while (*r == *s)  {
		if (*r == '\0')
			return(1);
		if (n > 8)
			return(0);
		r++;
		s++;
		n++;
		}
	return(0);
}

/*
 *   Get roughly 32 symbol table slots.
 *	set endcore = current break.
 */
cpget()
{
	register char *core;

	if ((core = sbrk(32*ST )) < 0)  {
		synerr("symbol table overflow");
		exit(1);
		}

	endcore = sbrk(0);
	return(core);
}

/*
 *   Define a symbol table entry.
 *
 *   Get next available slot and return a pointer to it.
 */
dslot(name, value, mode)
char	*name;
int	value;
int	mode;
{
	register struct	st *q;
	register char	*r;
	register char	*s;
	register int	ind;

	q = coreptr;
	if ((q+1) > endcore)
		cpget();

	ind = 0;
	r = q->s_name;
	s = name;
	while (ind < 8)  {
		*r++ = *s++;
		ind++;
		}

	q->s_value = value;
	q->s_mode  = mode;
	q->s_next  = NUL;

	nsyms++;
	coreptr =+ ST;
	return(q);
}

/*
 *   Convert an integer to ascii characters and
 *   push chars to core pointed to by 'r'.
 */
num(n, r)
register int n;
register char *r;
{
	register int mask;
	register int neg;
	register int i;
	char	 ch[16];

	i = 0;
	neg = n;
	mask = BITMASK((WORDSIZ - 1));
	if (!n)  {
		*r++ = '0';
		return(r);
		}

	if (neg < 0)  {
		*r++ = '0';
		*r++ = 'x';
		}

	while (n)  {
		if (neg < 0)  {
			ch[i] = hextab[n&0xf];
			n = (n >> 4) & mask;
			}

		else  {
			ch[i] = (n % 10) + '0';
			n =/ 10;
			}
		i++;
		}

	while (--i >= 0)  {
		*r++ = ch[i];
		}

	return(r);
}

/*
 *   Search string 'str' for character 'c' and return its
 *   index in 'str' if found. return ERR otherwise.
 */
any(c, str)
register char c;
char	 str[ ];
{
	register i;

	i = 0;
	while (str[i])  {
		if (c == str[i])
			return(i);
		i++;
		}
	return(ERR);
}
