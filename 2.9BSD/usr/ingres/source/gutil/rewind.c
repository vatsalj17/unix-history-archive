# include	"../fileio.h"

/*
**  REWIND -- reset file to load point
**
**	Parameter:
**		iop -- file pointer
**
**	Return:
**		zero -- successful
**		else -- failure
**
**	Requires:
**		lseek() -- to reset file pointer
**		fflush() -- to flush old buffer before seeking
**
**	Defines:
**		rewind()
**
**	History:
**		12/29/77 -- initially written by eric
*/

rewind(iop1)
FILE	*iop1;
{
	register FILE	*iop;
	register int	i;
	long		ltemp;

	iop = iop1;

	/* flush existing data */
	fflush(iop);

	/* seek to load point */
	ltemp = 0;
	i = lseek(iop->_file, ltemp, 0);

	/* return */
	return ((i < 0) ? i : 0);
}
