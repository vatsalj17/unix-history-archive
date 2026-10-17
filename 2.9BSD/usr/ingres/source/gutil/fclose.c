# include	"../fileio.h"

/*
**  FCLOSE -- flush and close file
**
**	Parameter:
**		ioptr -- a file pointer (as returned from fopen)
**
**	Return:
**		zero -- successful
**		else -- failure
**
**	Requires:
**		close()
**
**	Defines:
**		fclose()
**
**	History:
**		2/6/78 (eric) -- changed to not clear the _IOBSET bit
**			(so that freopen works right).
**		12/29/77 -- written by eric
*/

fclose(iop1)
FILE	*iop1;
{
	register FILE	*iop;
	register int	i;

	iop = iop1;

	fflush(iop);

	/* close the file */
	i = close(iop->_file);

	/* clear the file status info */
	iop->_flag =& ~(_IOREAD|_IOWRITE);

	return (i);
}
