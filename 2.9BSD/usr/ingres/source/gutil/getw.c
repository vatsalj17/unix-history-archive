# include	"../fileio.h"

/*
**  GETW -- get word
**
**	This routine matches getc except that it returns words instead
**	of bytes.  There are no word boundary problems.
**
**	There is probably a more efficient implementation.
**
**	Parameters:
**		iop -- I/O pointer of input file
**
**	Returns:
**		The next word from the file.
**		EOF on end of file or error.  Note that this is a legal
**			return value also.
**
**	Side Effects:
**		file activity
**
**	Defines:
**		getw
**
**	Requires:
**		getc
**
**	Called By:
**		USER
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
**		5/30/78 (eric) -- written
*/


getw(iop1)
FILE	*iop1;
{
	register FILE	*iop;
	register int	rval;
	register int	i;

	iop = iop1;

	/* get the first (low-order) character, test for EOF */
	rval = getc(iop);
	if (rval == EOF)
		return (rval);

	/* get the second (high-order) character, also test EOF */
	i = getc(iop);
	if (i == EOF)
		return (i);

	/* combine the two and return */
	rval =| i << 8;
	return (rval);
}
