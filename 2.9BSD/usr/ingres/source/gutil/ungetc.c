# include	"../fileio.h"

/*
**  UNGETC -- return character to file
**
**	This function is used to return a single character to a buffer
**	being read by getc or getchar.  There are several restrictions
**	on its use.
**
**	Only one character may be ungot with no intervening 'get'.
**	(In fact, this may work in some instances, but is not
**	dependable).
**
**	At least one character must have been gotten before any charac-
**	ters may be ungotten.
**
**	Parameters:
**		ch -- the character to unget.
**		iop1 -- the file pointer to unget to.
**
**	Returns:
**		ch -- normally
**		EOF -- if one of the constraints was not satisified.
**
**	Side Effects:
**		none
**
**	Defines:
**		ungetc()
**
**	Requires:
**		none
**
**	Called By:
**		USER
**
**	Files:
**		none
**
**	Compilation Flags:
**		none
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
**		7/17/78 (eric) -- written.
*/

ungetc(ch, iop1)
char	ch;
FILE	*iop1;
{
	register int	c;
	register FILE	*iop;

	iop = iop1;

	/* test for room in the buffer */
	if (iop->_ptr <= iop->_base)
		return (EOF);

	/* yes, put the character in the buffer */
	*--iop->_ptr = c = ch & 0377;
	iop->_cnt++;

	/* return the good value */
	return (c);
}
