# include	"../fileio.h"

/*
**  PUTCHAR -- put character to standard output
**
**	The character is put directly to standard output, unless it is
**	a control character, in which case it is translated to a back-
**	slash escape so that it can be read.  If this translation is
**	not wanted, use putc().
**
**	The determination of printability is done by looking at the
**	bit vector '_Printable' -- if the corresponding bit is set,
**	the character is printed directly.
**
**	Parameter:
**		c -- character to output
**
**	Return:
**		same as putc
**
**	Requires:
**		putc() -- to actually output the character
**
**	Defines:
**		putchar()
**		_Printable
**
**	History:
**		2/26/78 (eric) -- '_Printable' added.
**		12/30/77 -- written by eric
*/

int	_Printable[16]
{
	0033600, 0000000, 0177777, 0177777,
	0177777, 0177777, 0177777, 0077777,
	0000000, 0000000, 0000000, 0000000,
	0000000, 0000000, 0000000, 0000000,
};

putchar(c1)
char	c1;
{
	register char	c;
	register int	i;
	register int	mask;

	c = c1;
	i = (c >> 4) & 017;
	mask = 1 << (c & 017);

	if ((mask & _Printable[i]) == 0)
	{
		/* output the octal */
		putc('\\', stdout);
		i = c;
		putc(((i & 0300) >> 6) + '0', stdout);
		putc(((i & 0070) >> 3) + '0', stdout);
		return (putc((i & 0007) + '0', stdout));
	}
	else
		return(putc(c, stdout));
}
