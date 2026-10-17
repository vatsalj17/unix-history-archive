# include	"../fileio.h"

/*
**  FSEEK -- seek on buffered file
**
**	Equivalent to a system seek, except that it will not take the
**	block modes (3, 4, or 5) and handles the buffering correctly.
**
**	The only interesting thing is the special handling of relative
**	seeks, so that they work correctly knowing about the characters
**	already in the buffer.
**
**	Parameters:
**		iop1 -- io pointer for the file to seek on.
**		offset -- a long (!!!) which gives a byte offset into
**			the file to go to.
**		mode -- 0 -- seek to 'offset'.
**			1 -- seek to current location plus 'offset'.
**			2 -- seek to end of file plus 'offset'.
**
**	Returns:
**		zero -- everything ok.
**		negative -- system error (bad file descriptor).
**
**	Side Effects:
**		activity on file.
**
**	Defines:
**		fseek
**
**	Requires:
**		fflush
**		lseek
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
**		noe
**
**	History:
**		4/5/78 (eric) -- written.
*/

fseek(iop1, offset, mode)
FILE	*iop1;
long	offset;
int	mode;
{
	register FILE	*iop;
	register int	n;

	iop = iop1;

	if ((iop->_flag & _IOWRITE) != 0)
	{
		/* write case -- flush and seek */
		fflush(iop);
		n = lseek(iop->_file, offset, mode);
	}
	else
	{
		/* read case -- get current offset in file if relative */
		if (mode == 1)
		{
			n = iop->_cnt;
			if (n < 0)
				n = 0;
		}
		else
			n = 0;
		n = lseek (iop->_file, offset - n, mode);
		iop->_cnt = 0;
		iop->_ptr = iop->_base;
	}

	return ((n < 0) ? n : 0);
}
