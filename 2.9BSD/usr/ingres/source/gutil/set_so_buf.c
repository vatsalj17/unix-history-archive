# include	"../ingres.h"
# include	"../fileio.h"

/*
**  SET_SO_BUF -- set standard output buffer conditionally
**
**	This routine sets the standard output buffer conditionally,
**	based on whether or not it is a terminal.  If it is, it
**	does not set the output; otherwise, it buffers the output.
**	The buffer is contained internally.
**
**	If trace flag zero is set, it will never buffer.
**
**	Parameters:
**		none
**
**	Returns:
**		TRUE -- if buffer was set
**		FALSE -- otherwise
**
**	Side Effects:
**		The standard output is left buffered or unchanged.
**
**	Requires:
**		gtty -- to determine whether or not stdout is a
**			terminal.
**		tTf -- to decide whether or not to buffer
**
**	Called By:
**		ovqp/main
**			support/printr
**		dbu/print
**		(maybe others?)
**
**	Trace Flags:
**		0 -- but not as trace info.
**
**	Compilation Flags:
**		none, but tTf call should be taken out if all trace
**		flags in the system are taken out so that trace.o will
**		not load.
**
**	Diagnostics:
**		none
**
**	Syserrs:
**		none
**
**	History:
**		2/27/78 (eric) -- written
*/

set_so_buf()
{
	extern int	errno;
	int		gttybuf[3];
	static char	buffer[IOBUFSIZ];

	/* check trace stuff first */
	if (tTf(0, -1))
		return (FALSE);

	/* check for standard output is tty */
	if (gtty(stdout->_file, gttybuf))
	{
		/* no: reset errno and buffer */
		errno = 0;
		setbuf(stdout, buffer, IOBUFSIZ);

		return (TRUE);
	}

	return (FALSE);
}
