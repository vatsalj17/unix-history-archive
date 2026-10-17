# include	<pipes.h>
# include	<sccs.h>

SCCSID(@(#)IIpb_wphys.c	8.1	12/31/84)


/*
**  PB_WPHYS -- physical write on pipe
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
*/

IIpb_wphys(ppb, fd)
register pb_t	*ppb;
register int	fd;
{
	register int	i;

	i = write(fd, ppb, PB_IOSIZE);
	if (i != PB_IOSIZE)
		IIsyserr("pb_wphys: write error");
}
