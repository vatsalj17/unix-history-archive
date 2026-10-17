# include	"../fileio.h"

/*
**  GETC -- get character
**
**	This should really be a macro.
**
**	Parameter:
**		iop1 -- file pointer for file wanted (as returned from
**			fopen).
**
**	Return:
**		EOF -- on end-of-file or error
**		else -- the character (16 bits, no sign extension).
**
**	Requires:
**		_filbuf -- to get next block
**
**	Defines:
**		getc()
**
**	History:
**		12/29/77 -- originally written by eric
**		2/6/78 -- adjusted to use _filbuf by eric
*/

getc(iop1)
FILE	*iop1;
{
	register FILE	*iop;

	iop = iop1;

	if (--iop->_cnt >= 0)
		return (*iop->_ptr++ & 0377);
	else
		return (_filbuf(iop));
}
