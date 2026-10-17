# include	"../fileio.h"

/*
**  _FLSBUF -- internal buffer flush routine
**
**	This routine flushes an I/O buffer.  It is called when there
**	does not appear to be enough space in the buffer to put the
**	next character.  NOTE that this is not necessarily the case;
**	_flsbuf will be called on the first putc to get the buffer.
**	Also, note that when _flsbuf is called, there must always be
**	room for one more character in the buffer (unless c1 == EOF);
**	this will always be the case when it is first called, since
**	an empty buffer will be assigned.
**
**	What this all boils down to is that _flsbuf will not always
**	flush the buffer; it will put the character in and then flush
**	if the buffer is now full.  If c1 == EOF, a flush is forced.
**
**	Parameters:
**		c1 -- the character to put in the buffer.  If EOF,
**			the buffer is merely flushed.
**		iop1 -- the file to flush.
**
**	Returns:
**		EOF -- if an I/O error
**		zero -- if c1 is EOF and there is no error
**		c1 -- otherwise
**
**	Side Effects:
**		The buffer is emptied to the named file.
**		The struct pointed to by iop1 is adjusted to reflect
**			the new (empty) state of the buffer.
**
**	Requires:
**		write
**		_getbuf
**
**	Called By:
**		putc
**		fflush
**
**	History:
**		2/6/78 -- adapted from <stdio> by eric
*/

_flsbuf(c1, iop1)
int	c1;
FILE	*iop1;
{
	register FILE	*iop;
	register char	*base;
	register int	rtval;
	int		n;

	iop = iop1;

	/* check for a write file */
	if ((iop->_flag & _IOWRITE) == 0)
		return (EOF);

	/* get a buffer (if one does not already exist) */
	_getbuf(iop);

	base = iop->_base;
	if ((rtval = c1) == EOF)
		rtval = 0;
	else
	{
		/* guaranteed one more space in the buffer */
		*iop->_ptr++ = rtval;
		iop->_cnt--;
		if (iop->_cnt > 0)
		{
			/* this was the first call (buffer initialized) */
			return (rtval);
		}
	}

	/* if we have anything to write, write it */
	n = iop->_ptr - base;
	if (n > 0)
	{
		if (write(iop->_file, base, n) != n)
		{
			/* then an I/O error */
			iop->_flag =| _IOERR;
			rtval = EOF;
		}
	}

	/* reset buffer pointers and all */
	iop->_ptr = base;
	iop->_cnt = iop->_size;

	/* return with error/non-error value */
	return (rtval);
}
