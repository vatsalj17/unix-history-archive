# include	<ingres.h>
# include	<aux.h>
# include	<access.h>
# include	<symbol.h>
# include	<catalog.h>
# include	<sccs.h>

SCCSID(@(#)openr.c	8.4	1/22/85)

/*
**  OPENR -- Open a relation into a descriptor
**
**	Openr will open the named relation into the given descriptor
**	according to the mode specified. When searching for a name,
**	a relation owner by the current user will be searched for first.
**	If none is found then one owned by the DBA will be search for.
**
**	There are several available modes for opening a relation. The
**	most common are
**		mode OR_READ    -- open for reading
**		mode OR_WRITE   -- open for writing.
**	Other modes which can be used to optimize performance:
**		mode OR_RELTID  -- get relation-relation tuple and tid only.
**      			Does not open the relation.
**		mode OR_AREAD   -- open relation for reading after a previous
**      			call of mode OR_RELTID.
**		mode OR_AWRITE  -- open relation for writing after a previous
**      			call of mode OR_RELTID.
**		mode OR_REREAD  -- open relation for reading. Assumes that relation
**      			was previously open (eg relation & attributed
**      			have been filled) and file was closed by closer.
**		mode OR_REWRITE -- open relation for writing. Same assumptions as
**      			mode OR_REREAD.
**
**	Parameters:
**		dx - a pointer to a struct descriptor (defined in ingres.h)
**		mode - can be OR_READ -> OR_REWRITE
**		name - a null terminated name (only first 12 chars looked at)
**
**	Returns:
**		1 - relation does not exist
**		0 - ok
**		<0 - error. Refer to the error codes in access.h
**
**	Side Effects:
**		Opens the physical file if required. Fill the
**		descriptor structure. Initializes the access methods
**		if necessary.
**
**	Trace Flags:
**		90
*/


openr(d, mode, name)
register DESC	*d;
int		mode;
char		*name;
{
	int		i;
	register int	retval, filemode;
	char		filename[MAXNAME+3];
	char		btree[MAXNAME];
	char		btreefile[MAXNAME + 3];
#	ifdef xATR1
	if (tTf(21, 0))
		printf("openr:%.12s,%d\n", name, mode);
#	endif

	/* init admin */
	acc_init();

	/* process according to mode */

	filemode = O_RDONLY;

	if (mode >= 0)
		d->relbtree = NULL;

	switch (mode)
	{

	  case OR_RELTID:
		retval = get_reltup(d, name);
		break;

	  case OR_WRITE:
		filemode = O_RDWR;

	  case OR_READ:
		if (retval = get_reltup(d, name))
			break;

	  case OR_AREAD:
	  case OR_AWRITE:
		if (retval = get_attuples(d))
			break;

	  case OR_REWRITE:
		if (mode == OR_AWRITE || mode == OR_REWRITE)
			filemode = O_RDWR;

	  case OR_REREAD:
		clearkeys(d);
		/* descriptor is filled. open file */
		ingresname(d->reldum.relid, d->reldum.relowner, filename);
		/* can't open a view */
		if (d->reldum.relstat & S_VIEW)
		{
			retval = acc_err(AMOPNVIEW_ERR);	/* view */
			break;
		}
		if ((d->relfp = open(filename, filemode)) < 0)
		{
			retval = acc_err(AMNOFILE_ERR);	/* can't open file */
			break;
		}
		d->relopn = (d->relfp + 1) * 5;
		if (filemode == O_RDWR)
			d->relopn = -d->relopn;
		d->reladds = 0;
		retval = 0;
		break;

	  default:
		syserr("openr:bd md=%d", mode);
	}

	if (mode == OR_RELTID && d->reldum.reldim > 0 && !retval)
	{
		/* open btreesec relation */
		capital(d->reldum.relid, btree);
		if ((d->relbtree = (DESC *) calloc(1, sizeof(DESC))) == NULL)
			syserr("calloc error in openr");
		if (i = openr(d->relbtree, OR_RELTID, btree))
			syserr("opening Btreesec %s %d\n", btree, i);
	}

	if (retval == 0 && d->reldum.reldim > 0 && mode != OR_RELTID)
	{
		capital(d->reldum.relid, btree);
		if (d->relbtree == NULL)
		{
			if ((d->relbtree = (DESC *) calloc(1, sizeof(DESC))) == NULL)
				syserr("calloc error in openr");
		}
		if (i = openr(d->relbtree, mode, btree))
			syserr("opening Btreesec %s %d\n", btree, i);
		ingresname(d->reldum.relid, d->reldum.relowner, filename);
		btreename(filename, btreefile);
		if ((d->btree_fd = open(btreefile, O_RDWR)) < 0)
			syserr("openr: can't open %s", btreefile);
	}

	/* return */

#	ifdef xATR1
	if (tTf(21, 4) && mode != OR_RELTID && retval != 1)
		printdesc(d);
	if (tTf(21, 0))
		printf("openr rets %d\n", retval);
#	endif

	return (retval);
}
/*
**  GET_ATTUPLES -- get tuples from attribute relation for this relation
*/

get_attuples(d)
register DESC	*d;
{
	struct attribute	attr, attkey;
	register int		i, dom;
	int			numatts;
	TID			tid1, tid2;

	clearkeys(&Admin.adattd);

	/* zero all format types */
	for (i = 0; i <= d->reldum.relatts; i++)
		d->relfrmt[i] = 0;

	/* prepare to scan attribute relation */
	setkey(&Admin.adattd, (char *) &attkey, d->reldum.relid, ATTRELID);
	setkey(&Admin.adattd, (char *) &attkey, d->reldum.relowner, ATTOWNER);
	if (i = find(&Admin.adattd, EXACTKEY, &tid1, &tid2, &attkey))
		return (i);

	numatts = d->reldum.relatts;

	while (numatts && !get(&Admin.adattd, &tid1, &tid2, &attr, TRUE))
	{

		/* does this attribute belong? */
		if (bequal(&attr, &attkey, MAXNAME + 2))
		{

			/* this attribute belongs */
			dom = attr.attid;	/* get domain number */

			if (d->relfrmt[dom])
				break;	/* duplicate attribute. force error */

			numatts--;
			d->reloff[dom] = attr.attoff;
			d->relfrmt[dom] = attr.attfrmt;
			d->relfrml[dom] = attr.attfrml;
			d->relxtra[dom] = attr.attxtra;
		}
	}

	d->relfrmt[0] = INT;
	d->relfrml[0] = 4;
	/* make sure all the atributes were there */
	for (dom = 1; dom <= d->reldum.relatts; dom++)
		if (d->relfrmt[dom] == 0)
			numatts = 1;	/* force an error */
	if (numatts)
		i = acc_err(AMNOATTS_ERR);

	flush_rel(&Admin.adattd, TRUE);

#	ifdef xATR1
	if (tTf(21, 3))
		printf("get_attr ret %d\n", i);
#	endif

	return (i);
}
