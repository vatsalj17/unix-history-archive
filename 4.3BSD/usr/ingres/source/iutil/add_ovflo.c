# include	<ingres.h>
# include	<access.h>
# include	<aux.h>
# include	<lock.h>
# include	<sccs.h>

SCCSID(@(#)add_ovflo.c	8.2	2/8/85)

/*
**	ADD_OVFLO -- Allocates an overflow page which will be
**		attached to current page.  TID must be for current page.
**		TID will be updated to id of new overflow page.
**
**	Parameters:
**		dx - descriptor for relation
**		tid - tid for current page
**
**	Side Effects:
**		tid is updated to id of new overflow page
**
**	Trace Flags:
**		26.3
**
**	Returns:
**		0 - success
**		-1 (NOBUFFER) can't get buffer for overflow page
**		-2 (NOSETUP) can't set up overflow page
**		-3 (NOGETCURRENT) can't get the current page
**		-4 (NORMVMAIN) can't remove main page
**		-5 (NOGETOVFLO) can't get the overflow page
**
*/

add_ovflo(dx, tid)
DESC	*dx;
TID	*tid;
{
	register DESC		*d;
	register struct accbuf	*b;
	extern struct accbuf	*choose_buf();
	register int		lk;
	int			i;
	long			mpage, newpage;
	TID			tidx;

	d = dx;
#	ifdef xATR2
	if (tTf(26, 3))
		printf("ADD_OVFLO:\n");
#	endif

	/*
	**	save main page pointer so that it may be used when
	**	setting up new overflow page.
	*/
	mpage = Acc_head->mainpg;

	if (lk = (Acclock && (d->reldum.relstat & S_CONCUR) && (d->relopn < 0 )))
	{
		setcsl(Acc_head->rel_tupid);
		Acclock = FALSE;
	}

	/*
	**	Determine last page of the relation
	*/
	last_page(d, &tidx, Acc_head);
	pluck_page(&tidx, &newpage);
	newpage++;

	/*
	**	choose an available buffer as victim for setting up
	**	overflow page.
	*/
	if ((b = choose_buf(d, newpage)) == NULL)
	{
		if (lk)
		{
			Acclock = TRUE;
			unlcs(Acc_head->rel_tupid);
		}
		return(NOBUFFER);
	}

	/*
	**	setup overflow page
	*/

	b->mainpg = mpage;
	b->ovflopg = 0;
	b->thispage = newpage;
	b->linetab[0] = (int) b->firstup - (int) b;
	b->nxtlino = 0;
	b->bufstatus |= BUF_DIRTY;
	if (pageflush(b))
		return (NOSETUP);

	/*
	**	now that overflow page has successfully been written,
	**	get the old current page back and link the new overflow page
	**	to it.
	**	If the relation is a heap then don't leave the old main
	**	page around in the buffers. This is done on the belief
	**	that it will never be accessed again.
	*/

	if (get_page(d, tid))
		return (NOGETCURRENT);
	Acc_head->ovflopg = newpage;
	Acc_head->bufstatus |= BUF_DIRTY;
	i = pageflush(Acc_head);
	if (lk)
	{
		Acclock = TRUE;
		unlcs(Acc_head->rel_tupid);
	}
	if (i)
		return (NORMVMAIN);
	if (abs(d->reldum.relspec) == M_HEAP)
		resetacc(Acc_head);	/* no error is possible */

	/*
	**	now bring the overflow page back and make it current.
	**	if the overflow page is still in AM cache, then this will not
	**	cause any disk activity.
	*/

	stuff_page(tid, &newpage);
	if (get_page(d, tid))
		return (NOGETOVFLO);
	return (0);
}
