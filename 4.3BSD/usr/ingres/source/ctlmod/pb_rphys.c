# include	"pipes.h"
# include	<sccs.h>

SCCSID(@(#)pb_rphys.c	8.1	12/31/84)

/*
**  PB_RPHYS -- physical read on pipe
**
**	Parameters:
**		ppb -- a pointer to the data area.
**		fd -- the file descriptor.
**
**	Returns:
**		none
**
**	Side Effects:
**		none
**
**	Called By:
**		pb_read
**
**	Trace Flags:
**		none
*/

pb_rphys(ppb, fd)
register pb_t	*ppb;
register int	fd;
{
	return (read(fd, (char *) ppb, PB_IOSIZE));
}
