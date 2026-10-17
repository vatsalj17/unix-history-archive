# include	"../fileio.h"

/*
**  FWRITE -- buffered binary write
**
**	Parameters:
**		iop1 -- file pointer
**		area -- data area
**		count -- byte count
**
**	Return:
**		number of characters actually written.  Note: because
**		of the buffering, it will return EOF if there is any
**		io error.
**
**	Requires:
**		_flsbuf()
**
**	Defines:
**		fwrite()
**
**	History:
**		12/30/77 -- written by eric
**		2/9/78 [eric] -- modified to use _flsbuf and do large
**			sets of buffered moves to save CPU time.
*/

fwrite(iop1, area, count)
FILE	*iop1;
char	*area;
int	count;
{
	register int	cnt;
	char		*p;
	register int	i;
	register FILE	*iop;

	cnt = count;
	p = area;
	iop = iop1;

	/* buffer out data */
	while (cnt > 0)
	{
		/* find min(bytes_to_write, space_in_buffer) */
		i = iop->_cnt;
		if (cnt < i)
			i = cnt;

		if (i > 0)
		{
			/* move the data into the buffer & update pointers */
			bmove(p, iop->_ptr, i);
			iop->_ptr =+ i;
			iop->_cnt =- i;
			p =+ i;
			cnt =- i;
		}

		/* check for buffer full */
		if (iop->_cnt <= 0)
		{
			/* no, flush the buffer */
			if (_flsbuf(EOF, iop) == EOF)
			{
				/* error on write */
				return (count - cnt);
			}
		}
	}

	return (count);
}
