# include	"../fileio.h"

/*
**  FFLUSH -- flush file buffer
**
**	For a write file, any data in the buffer is flushed to the
**	file.  For a read file, the call is ignored.
**
**	Parameter:
**		iop -- file pointer
**
**	Return:
**		zero -- success
**		EOF -- failure
**
**	Requires:
**		write()
**
**	Defines:
**		fflush()
**
**	History:
**		12/29/77 -- written by eric
*/

fflush(iop1)
FILE	*iop1;
{
	register FILE	*iop;
	register int	i;
	register int	n;

	iop = iop1;

	/* if unbuffered or a read file, ignore this call */
	if (iop->_base == NULL || (iop->_flag & _IOWRITE) == 0 ||
	    iop->_ptr == NULL || (n = iop->_ptr - iop->_base) <= 0)
		return (0);

	i = write(iop->_file, iop->_base, n);
	if (i != n)
	{
		iop->_flag =| _IOERR;
		return (EOF);
	}

	/* reset all the right pointers */
	iop->_cnt = iop->_size;
	iop->_ptr = iop->_base;

	return (0);
}



/*
**  FLUSH -- flush standard output and error
**
**	Parameters:
**		none
**
**	Return:
**		none
**
**	Requires:
**		fflush()
**
**	Defines:
**		flush()
**
**	Called by:
**		general user
**
**	History:
**		2/27/78 (eric) -- changed to flush only standard output
**			and error.
**		1/4/78 -- written by eric
*/

flush()
{
	fflush(stdout);
	fflush(stderr);
}
