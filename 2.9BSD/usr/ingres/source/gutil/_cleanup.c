# include	"../fileio.h"

/*
**  _CLEANUP -- flush all files on exit
**
**	Parameters:
**		none
**
**	Returns:
**		none
**
**	Side Effects:
**		All files are flushed.
**
**	Requires:
**		fflush
**		_Iob
**
**	Called By:
**		exit.s
**
**	Diagnostics:
**		none
**
**	Syserrs:
**		none
**
**	History:
**		2/27/78 (eric) -- changed to drop 'flush' call, since
**			flush only flushes stdout and stderr now.
**		2/9/78 [eric] -- written
*/

_cleanup()
{
	register FILE	*iop;

	for (iop = _Iob; iop < &_Iob[_NFILES]; iop++)
		fflush(iop);
}
