# include	"../fileio.h"

/*
**  PUTC -- put character, buffered
**
**	This should really be a macro.
**
**	Parameters:
**		ch -- character to put
**		iop -- file pointer
**
**	Return:
**		EOF -- on output error
**		ch -- on no error
**
**	Requires:
**		write() -- to flush buffer
**
**	Defines:
**		putc()
**
**	History:
**		12/29/77 -- originally written by eric
*/

putc(ch, iop1)
char	ch;
FILE	*iop1;
{
	register FILE	*iop;

	iop = iop1;

	if (--iop->_cnt > 0)
		return ((*iop->_ptr++) = ch);
	else
		return (_flsbuf(ch & 0377, iop));
}
