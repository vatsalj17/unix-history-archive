# include	"../fileio.h"

/*
**  FREAD -- read a binary block
**
**	Parameters:
**		iop1 -- a file pointer
**		area -- the data area
**		count -- the byte count
**
**	Return:
**		The actual number of characters read.
**
**	Requires:
**		_filbuf()
**
**	Defines:
**		fread()
**
**	History:
**		12/29/77 -- written by eric
**		2/9/78 [eric] -- modified to use _filbuf instead of
**			getc.
*/

fread(iop1, area, count)
FILE	*iop1;
char	*area;
int	count;
{
	register int	cnt;	/* total bytes still needed */
	char		*p;	/* pointer into 'area' */
	register int	i;
	register FILE	*iop;

	cnt = count;
	p = area;
	iop = iop1;

	/* buffer in data */
	while (cnt > 0)
	{
		if (iop->_cnt <= 0)
		{
			/* buffer is empty, fill it from file */
			i = _filbuf(iop);
			if (i == EOF)
			{
				/* premature EOF */
				return (count - cnt);
			}
			cnt--;
			*p++ = i;
			continue;
		}

		/* there in info in the buffer -- do a block move */
		i = iop->_cnt;
		if (i > cnt)
			i = cnt;
		bmove(iop->_ptr, p, i);
		iop->_ptr =+ i;
		iop->_cnt =- i;
		p =+ i;
		cnt =- i;
	}

	return (count);
}
