# include	"../fileio.h"

/*
**  _FILBUF -- fill I/O buffer
**
**	The I/O buffer is filled and the next character returned.
**
**	Parameters:
**		iop1 -- the file.
**
**	Returns:
**		EOF -- on I/O error or EOF
**		else -- next character
**
**	Side Effects:
**		The file structure will be modified to reflect
**			the new information.
**		File activity will occur.
**
**	Requires:
**		read
**		_getbuf -- to get a buffer
**
**	Called By:
**		getc
**		fread
**
**	Diagnostics:
**		EOF return on I/O error or bad pointer.
**
**	History:
**		2/1/78 -- adapted from <stdio> by eric
*/

_filbuf(iop1)
FILE	*iop1;
{
	register FILE	*iop;
	register char	*base;
	register int	i;

	iop = iop1;

	/* check for a valid read file */
	if ((iop->_flag & _IOREAD) == 0)
		return (EOF);

	/* get a buffer if one does not already exist */
	_getbuf(iop);

	/* get info about next block */
	base = iop->_base;

	/* try reading in the block */
	i = read(iop->_file, base, iop->_size);

	/* check for EOF or error */
	if (i <= 0)
	{
		if (i < 0)
		{
			/* read error */
			iop->_flag =| _IOERR;
		}
		else
		{
			/* end of file */
			iop->_flag =| _IOEOF;
		}
		return (EOF);
	}

	/* set up the information in file header */
	iop->_cnt = i;
	iop->_ptr = base;

	/* return first character in buffer */
	iop->_cnt--;
	return (*iop->_ptr++ & 0377);
}
