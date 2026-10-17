# include	"../ingres.h"
# include	"../fileio.h"

/*
**  SET_SI_BUF -- set standard input buffer unconditionally
**
**	This routine sets the standard input buffer to give
**	buffered input.  The buffer is contained internally.
**
**	Parameters:
**		none
**
**	Returns:
**		TRUE.
**
**	Side Effects:
**		The standard input is left buffered.
**
**	Requires:
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
**		1/17/79 (eric) -- written
*/

set_si_buf()
{
	static char	buffer[IOBUFSIZ];

	setbuf(stdin, buffer, IOBUFSIZ);
	return (TRUE);
}
