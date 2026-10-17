# include	<ingres.h>
# include	<access.h>
# include	<sccs.h>

SCCSID(@(#)formatpg.c	8.2	2/8/85)

/*
**	FORMATPG - write n pages to a buffer
**
**
**	Parameters:
**		d - descriptor
**		n - number of pages
**
**
**	Return Codes:
**		0 - successful
**		-2 - failure when setting file ptr
**		-3, -4 - failure when writing page to buffer
**			-4 is for last page
**
**	Called by:
**		create()
**		modify()
**		resetrel()
**
*/
formatpg(d, n)
DESC	*d;
long	n;
{
	struct accbuf	buf;
	register char	*p;
	extern long	lseek();

	if (Acc_head == 0)
		acc_init();
	if (lseek(d->relfp, 0l, 0) == -1)
		return (-2);
	buf.rel_tupid = d->reltid.ltid;
	buf.filedesc = d->relfp;
	for (p = (char *) &buf; p <= (char *) buf.linetab; p++)
		*p = NULL;
	buf.nxtlino = 0;
	buf.linetab[0] = (int) buf.firstup - (int) &buf;
	buf.ovflopg = 0;
	for (buf.mainpg = 1; buf.mainpg < n; (buf.mainpg)++)
	{
		if (write(buf.filedesc, (char *) &buf, PGSIZE) != PGSIZE)
			return (-3);
	}
	buf.mainpg = 0;
	if (write(buf.filedesc, (char *) &buf, PGSIZE) != PGSIZE)
		return (-4);
	Accuwrite += n;
	return (0);
}
