# include	<ingres.h>
# include	<access.h>
# include	<sccs.h>

SCCSID(@(#)acc_addbuf.c	8.2	2/8/85)

/*
**	ACC_ADDBUF - add access buffers to the usage list
**
**	Parameters:
**		bufs - buffers to add
**		cnt -  number of buffers to add
**
**	Side Effects:
**		Buffers are added to the list pointed
**		to by Acc_tail
**
**	Called by:
**		init_decomp()
**
*/
acc_addbuf(bufs, cnt)
struct accbuf	bufs[];
int		cnt;
{
	register struct accbuf	*b, *end;

	b = bufs;
	end = &b[cnt -1];
	acc_init();

	for ( ; b <= end; b++)
	{
		b->bufstatus = 0;
		resetacc(b);
		Acc_tail->modf = b;
		b->modb = Acc_tail;
		b->modf = NULL;
		Acc_tail = b;
	}
}
