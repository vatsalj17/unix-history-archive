# include	"../fileio.h"

/*
**  SETBUF -- set file buffer
**
**	This routine sets the buffer of the named file.  The buffer
**	must be declared as
**		char	buf[N];
**	where N is passed as the 'size' argument.  IOBUFSIZ is probably
**	a good choice.
**
**	Setbuf() must be called after the file has been opened, but
**	before any I/O has taken place.  It is equivalent to the buffer
**	parameter on fopen().
**
**	Parameters:
**		iop -- file pointer
**		buf -- buffer.  If zero, the file is unbuffered.
**		size -- size of buffer.
**
**	Return:
**		none
**
**	Requires:
**		none
**
**	Defines:
**		setbuf()
**
**	History:
**		2/22/79 (eric) -- _cnt set to _size instead of zero
**			to make writes happen on block boundaries.
**		2/27/78 (eric) -- 'size' parameter added.
**		2/6/78 (eric) -- modified to maintain the _IOBSET flag
**			and the _size field.
**		12/29/77 -- written by eric
*/

setbuf(iop1, buf, size)
FILE	*iop1;
char	buf[];
int	size;
{
	register FILE	*iop;
	register char	*b;

	iop = iop1;
	b = buf;

	iop->_base = iop->_ptr = b;
	if (b == NULL)
		iop->_size = 0;
	else
		iop->_size = size;
	iop->_flag =| _IOBSET;
	if ((iop->_flag & _IOWRITE) == 0)
		iop->_cnt = 0;
	else
		iop->_cnt = iop->_size;
}
