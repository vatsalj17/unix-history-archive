# include	"../ingres.h"
# include	"../aux.h"
# include	"../fileio.h"
# include	"monitor.h"

/*
**  GET CHARACTER
**
**	This routine is just a getchar, except it allows a single
**	backup character ("Peekch").  If Peekch is nonzero it is
**	returned, otherwise a getchar is returned.
*/

char
getch()
{
	register char	c;

	c = Peekch;
	if (c)
		Peekch = 0;
	else
	{
		c = getc(Input);
	}
	if (c < 0)
		c = 0;

	/* deliver EOF if newline in Oneline mode */
	if (c == '\n' && Oneline)
	{
		Peekch = c;
		c = 0;
	}

	return (c);
}




/*
**  GETCHAR -- get character from standard input
**
**	This routine is redefined to do unbuffered reads.
**
**	Parameters:
**		none
**
**	Returns:
**		next character from standard input.
**		EOF if at end of file
**
**	Side Effects:
**		none
**
**	Requires:
**		none
**
**	Called By:
**		{read} primitive of macro processor (macread)
**
**	Trace Flags:
**		none
**
**	Diagnostics:
**		none
**
**	Syserrs:
**		none
**
**	History:
**		5/17/78 (eric) -- added to fix bug with \include files
**			which used {read} calls inside them, when the
**			standard input was redirected from a file
**			(whew!).
*/

getchar()
{
	static char	x;
	register int	i;

	i = read(0, &x, 1);
	if (i <= 0)
		return (EOF);
	else
		return (x);
}
