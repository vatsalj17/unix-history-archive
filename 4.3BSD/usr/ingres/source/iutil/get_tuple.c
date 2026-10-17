# include	<ingres.h>
# include	<access.h>
# include	<symbol.h>
# include	<catalog.h>
# include	<btree.h>
# include	<sccs.h>

SCCSID(@(#)get_tuple.c	8.2	2/8/85)

/*
**	Routine associated with getting a tuple out of
**	the current buffer.
*/


/*
**	Get_tuple - take the tuple specified
**	by tid and move it to "tuple"
*/

get_tuple(d, tid, tuple)
register DESC	*d;
TID		*tid;
char		*tuple;
{
	register char	*cp;
	char		*get_addr();
	TID		tidloc;
	char		lid[MAXLID * LIDSIZE];
	extern DESC	Btreesec;
	extern int	Btree_fd;

	cp = get_addr(tid);

	if (d->reldum.relspec < 0)
		uncomp_tup(d, cp, tuple);	/* compressed tuple */
	else
	{
		if (d->reldum.reldim == 0)
			bmove(cp, tuple, d->reldum.relwid);	/* uncompressed tuple */
		else
			/* ignore lid field for now */
			bmove(cp, tuple, d->reldum.relwid - d->reldum.reldim * LIDSIZE);
	}

	if (d->reldum.reldim > 0)
	/* find corresponding lid value given main relation tid */
	{
		bmove(d->relbtree, &Btreesec, sizeof(Btreesec));
		Btree_fd = d->btree_fd;
		search_btree(*tid, &tidloc);
		get_lid(&tidloc, lid);
		/* attach lid value to end of tuple */
		cp = tuple + d->reldum.relwid - d->reldum.reldim * LIDSIZE;
		bmove(lid, cp, d->reldum.reldim * LIDSIZE);
	}
}
/*
**	Getint_tuple - get the tuple specified by
**	tid. If possible avoid moving the tuple.
**	Return value is address of tuple.
*/

char *
getint_tuple(d, tid, tuple)
register DESC	*d;
TID		*tid;
char		*tuple;
{
	register char	*cp, *ret;
	extern char	*get_addr();
	TID		tidloc;
	char		lid[MAXLID * LIDSIZE];
	extern DESC	Btreesec;
	extern int	Btree_fd;

	cp = get_addr(tid);

	if (d->reldum.relspec < 0)
	{
		ret = tuple;
		uncomp_tup(d, cp, ret);	/* compressed tuple */
	}
	else if (d->reldum.reldim == 0)
		ret = cp;			/* uncompressed tuple */
	else
	{
		/* ignore lid field for now */
		ret = tuple;
		bmove(cp, tuple, d->reldum.relwid - d->reldum.reldim * LIDSIZE);
	}
	if (d->reldum.reldim > 0)
	{
		bmove(d->relbtree, &Btreesec, sizeof(Btreesec));
		Btree_fd = d->btree_fd;
		/* find corresponding lid value */
		search_btree(*tid, &tidloc);
		get_lid(&tidloc, lid);
		/* attach lid value to end of tuple */
		cp = ret + d->reldum.relwid - d->reldum.reldim * LIDSIZE;
		bmove(lid, cp, d->reldum.reldim * LIDSIZE);
	}
	return (ret);
}
/*
**	Routine to compute the address of a tuple
**	within the current buffer.
**	Syserr if specified tuple deleted.
*/

char *
get_addr(tid)
register TID	*tid;
{
	register int	offset;

	offset = Acc_head->linetab[-(tid->line_id & I1MASK)];
	if (offset == 0)
	{
		syserr("get_addr rel=%ld tid=%ld", Acc_head->rel_tupid, *(long *)tid);
	}
#	ifdef xATR3
	if (offset < 0 || offset > PGSIZE)
		syserr("get_addr: offset=%d\n");
#	endif
	return (((char *) Acc_head) + offset);
}
/*
**	Uncompress - decompress the tuple at address cp
**	according to descriptor.
*/

uncomp_tup(d, cp, tuple)
register DESC	*d;
char		*cp;
char		*tuple;
{
	register char	*src, *dst;
	int		i, j, numatts;

	dst = tuple;
	src = cp;

	/* for each domain, copy and blank pad if char */
	/* ignore lid field */
	numatts = d->reldum.relatts - d->reldum.reldim;
	for (i = 1; i <= numatts; i++)
	{
		j = d->relfrml[i] & I1MASK;
		if (d->relfrmt[i] == CHAR)
		{
			while (j--)
			{
				if ((*dst++ = *src++) == NULL)
				{
					/* back up one */
					dst--;
					j++;
					break;
				}
			}

			/* blank pad if necessary */
			while (j-- > 0)
				*dst++ = ' ';
		}
		else
		{
			while (j--)
				*dst++ = *src++;
		}
	}
}
/*
**	Check if a line number is valid.
**	If linenumber is illegal return AMINVL_ERR
**	if Line has been deleted return DELTUP
**	else return 0
*/

invalid(tid)
register TID	*tid;
{
	register int	i;

	i = tid->line_id & I1MASK;

	if (i >= Acc_head->nxtlino)
		return (acc_err(AMINVL_ERR));

	if (Acc_head->linetab[-i] == 0)
		return (DELTUP);

	return (0);
}
