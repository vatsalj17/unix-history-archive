# include	<stdio.h>
# include	<ingres.h>
# include	<pv.h>
# include	<aux.h>
# include	<access.h>
# include	<batch.h>
# include	<lock.h>
# include	<opsys.h>
# include 	<func.h>
# include	<version.h>
# include	<symbol.h>
# include	<catalog.h>
# include	<btree.h>
# include	<sccs.h>
# include	<errors.h>

SCCSID(@(#)modify.c	8.8	5/7/85)

extern	short	tTdbu[];
extern	int	modify();
extern 	int	null_fn();

struct fn_def ModifyFn =
{
	"MODIFY",
	modify,
	null_fn,
	null_fn,
	NULL,
	0,
	tTdbu,
	100,
	'Z',
	0
};

/*
**  MODIFY -- converts any relation to the specified
**		storage structure
**
**	arguments:
**	0 - relation name
**	1 - storage structure ("heap", "cheap", "hash", "chash",
**		"isam", "cisam")
**	2 - "name" for attribute names, or "num" for numbers
**	3 - key1
**	4 - key2
**	    .
**	    .
**	i - null
**	i+1 - option name (e.g., "fillfactor")
**	i+2 - option value
**	    .
**	    .
**
**	If all the options default, parameter i -> pc are omitted.
**	If no keys are provided, parameter 2 is omitted.
*/

int		F_fac, Mn_pages, Mx_pages;
char		Lid[MAXLID][MAXNAME];
int		NLidKeys;
int		LidKey[MAXLID];

struct modtab
{
	char	*type;
	char	newrelspec;
	char	yeskeys;
	char	sortit;
	char	yes_seq;
	int	f_fac;
	int	mn_pages;
	int	mx_pages;
};


struct modtab	Modtab[] =
{
	/* type		spec	keys	sort	seq	ffac	min	max */

	"heap",		M_HEAP,	FALSE,	FALSE,	FALSE,	0,	0,	0,
	"cheap",	-M_HEAP,FALSE,	FALSE,	FALSE,	0,	0,	0,
	"hash",		M_HASH,	TRUE,	TRUE,	FALSE,	50,	10,	-1,
	"chash",	-M_HASH,TRUE,	TRUE,	FALSE,	75,	1,	-1,
	"isam",		M_ISAM,	TRUE,	TRUE,	FALSE,	80,	0,	0,
	"cisam",	-M_ISAM,TRUE,	TRUE,	FALSE,	100,	0,	0,
	"heapsort",	M_HEAP,	TRUE,	TRUE,	TRUE,	0,	0,	0,
	"cheapsort",	-M_HEAP,TRUE,	TRUE,	TRUE,	0,	0,	0,
	"truncated",	M_TRUNC,FALSE,	FALSE,	FALSE,	0,	0,	0,
	"ordered",	M_ORDER,TRUE,	FALSE,	FALSE,	0,	0,	0,
	0
};

struct mod_info
{
	char	outfile[MAXNAME + 4];	/* result file filled by ksort */
	char	formfile[MAXNAME + 4];	/* file with descriptor for ksort */
	char	infile[MAXNAME + 4];	/* input file for ksort (relation itself */
	char	reltemp[MAXNAME + 4];	/* file holding new relation */
	char	spfile[MAXNAME + 4], spflag;	/* isam spool file for overflow */
	char	btree[MAXNAME + 4];	/* file holding temporary btree structure */
	char	temp_sort[MAXNAME + 4];	/* file holding result of special isam
					** required when ordering on a field
					*/
};

struct mod_info	Mod_info;

extern DESC Btreesec;
extern int Btree_fd;



modify(pc, pv)
int	pc;
PARM	*pv;
{
	register int		i, j;
	register char		*rname;
	register struct modtab	*mp;
	struct modtab		*p;
	int			sorted, dim;
	DESC			dold, dnew;
	long			temptid;
	extern int		Noupdt;
	extern DESC		Attdes;
	struct attribute	atttup, attkey;
	TID			tid;
	int			lidkey, numatts;
	extern char		*trim_relname();
	extern char		*iocv();

	
#	ifdef xZTR1
	if (tTf(34, -1))
	{
		printf("enter modify\n");
		prvect(pc, pv);
	}
#	endif

	pv[pc].pv_val.pv_str = NULL;

	/* check for nice parameters */
	if (pc < 2)
		syserr("MODIFY: pc %d", pc);

	/* save relation name for error messages */
	rname = (pv++)->pv_val.pv_str;	/* *pv now pointes to storage spec */

	/* check for good relation */
	i = openr(&dold, OR_READ, rname);
	if (i == AMOPNVIEW_ERR)
		return (error(NOMODVIEW, rname, 0));
	if (i > 0)
		/* reln does not exist */
		return (error(NOREL, rname, 0));	
	else if (i < 0)
		syserr("MODIFY: openr (%.14s) %d", rname, i);
	/* can only modify a relation you own and isn't a sys rel */
	
	if (!bequal(Usercode, dold.reldum.relowner, UCODE_SZ))
	{
		i = NOOWN;
	}
	if ((dold.reldum.relstat & S_CATALOG) && Noupdt)
	{
		i = NOMODSYSREL;
	}
	if (i)
	{
		closer(&dold);
		return (error(i, rname, 0));
	}

	/*
	** Form descriptor for new relation. Here we need to
	** separate the pages from the old and new relations.
	** Since pages are identified by the TID of the relation
	** relation tuple, both old and new have the same identifiers.
	** To avoid this problem, a special TID is hand crafted for
	** the new relation.
	*/
	bmove(&dold, &dnew, sizeof dnew);
	dnew.reltid.s_tupid.line_id = (char) -2;	/* choose impossible reltid */
	/* assume new relation isn't ordered */
	if (dold.reldum.reldim)
	{
		dnew.reldum.relatts -= dold.reldum.reldim;
		dnew.reldum.relwid -= dold.reldum.reldim * LIDSIZE;
		dnew.reldum.reldim = 0;
	}

	/* In case of an interrupt from a previous modify,
	** there might be pages around. Get rid of them.
	*/
	cleanrel(&dnew);

	ingresname(dold.reldum.relid, dold.reldum.relowner, Mod_info.infile);
	dim = 0;
	NLidKeys = 0;

	/* scan for entry in relspec table */
	for (mp = Modtab; mp->type; mp++)
	{
		if (bequal(mp->type, pv->pv_val.pv_str, 7) && bequal("ordered", pv->pv_val.pv_str, 7))
		{
			if ((dim = atoi(pv->pv_val.pv_str + 7)) <= 0 || dim > MAXLID)
			{
				closer(&dold);
				return(error(BADORDDIM, rname, iocv(dim), 0));
			}
			break;
		}
		if (sequal(mp->type, pv->pv_val.pv_str))
			break;
	}

	/* if not found, error */
	if (!mp->type)
	{
		closer(&dold);
		return (error(BADSTORAGE, rname, pv->pv_val.pv_str, 0));	/* bad relspec */
	}

	if (mp->newrelspec == M_ORDER && dold.reldum.relindxd == SECINDEX)
	/* can't order an index relation */
	{
		closer(&dold);
		return(error(NOORDINDX, rname,0));
	}

	if (mp->newrelspec == M_ORDER)
	{
		dnew.reldum.reldim = dim;
		for (i = 0; i < dim; ++i)
		{
			++dnew.reldum.relatts;
			dnew.relxtra[dnew.reldum.relatts] = 0;
			dnew.reloff[dnew.reldum.relatts] = dnew.reldum.relwid;
			dnew.relfrmt[dnew.reldum.relatts] = INT;
			dnew.relfrml[dnew.reldum.relatts] = LIDSIZE;
			dnew.reldum.relwid += LIDSIZE;
		}
		concat(BTREE, Fileset, Mod_info.btree);
		create_btree(Mod_info.btree);
		dnew.btree_fd = Btree_fd;
		/* ok to order ascending/descending */
		mp->yes_seq = TRUE;
	}
	else
	{
		dnew.reldum.relspec = mp->newrelspec;
		if (dold.reldum.reldim)
		{
			dold.reldum.relatts -= dold.reldum.reldim;
			dold.reldum.relwid -= dold.reldum.reldim * LIDSIZE;
			dold.reldum.reldim = 0;
			closer(dold.relbtree);
			close(dold.btree_fd);
		}
	}

	if (dnew.reldum.relspec == M_TRUNC)
		dnew.reldum.relspec = M_HEAP;

	pv++;	/* now points to first parameter */

	/* get the key domains information */
	if ((i = getkeys(&pv, rname, &dnew, mp)) > 0)
	{
		closer(&dold);
		return (i);	/* user error */
	}

	j = 0;
	for (i = 0; i < NLidKeys; ++i)
		if (LidKey[i] > dold.reldum.relatts - dold.reldum.reldim)
		{
			j = 1;
			break;
		}

	if (!j && dold.reldum.reldim)
	/* treat old relation as if not ordered since lid field not needed */
	{
		dold.reldum.relatts -= dold.reldum.reldim;
		dold.reldum.relwid -= dold.reldum.reldim * LIDSIZE;
		dold.reldum.reldim = 0;
		closer(dold.relbtree);
		close(dold.btree_fd);
	}

	if (!dnew.reldum.reldim || !NLidKeys)
	{
		F_fac = mp->f_fac;
		Mn_pages = mp->mn_pages;
		Mx_pages = mp->mx_pages;
	}
	else
	/* set parameters to that of storage type of relation to be ordered */
	{
		for (p = Modtab; p->type; p++)
			if (dnew.reldum.relspec == p->newrelspec)
				break;
		F_fac = p->f_fac;
		Mn_pages = p->mn_pages;
		Mx_pages = p->mx_pages;
	}

	if (mp->newrelspec != M_ORDER)
		for (i = 0; i < dnew.reldum.reldim; ++i)
			Lid[i][0] = NULL;
	else
		for (i = 1; i <= dnew.reldum.reldim; ++i)
			concat("lid", iocv(i), Lid[i-1]);

	/* get fillfactor and other options if any */
	if (i = getfill(&dnew, pv, rname, mp))
	{
		closer(&dold);
		return (i);	/* user error */
	}

	/* check for duplicate attribute name */
	if (mp->newrelspec == M_ORDER)
	{
		opencatalog("attribute", OR_READ);
		setkey(&Attdes, &attkey, dnew.reldum.relid, ATTRELID);
		setkey(&Attdes, &attkey, dnew.reldum.relowner, ATTOWNER);
		numatts = dold.reldum.relatts - dold.reldum.reldim;
		for (i = 0; i < dnew.reldum.reldim; ++i)
		{
			setkey(&Attdes, &attkey, Lid[i], ATTNAME);
			if (getequal(&Attdes, &attkey, &atttup, &tid) == 0)
			{
				if (atttup.attid <= numatts)
				/* ok to duplicate attributes that will be removed */
				{
					closer(&dold);
					return(error(INVALIDATTR, rname, Lid[i], 0));
				}
			}
		}
	}

	/* lock the relation relation */
	if (Lockrel)
	{
		get_p_tid(&dold, &temptid);
		setrll(A_SLP, temptid, M_EXCL);
	}

	if (!dnew.reldum.reldim || NLidKeys > 0)
		/* compute new relation parameters & build descriptor */
		make_newrel(&dnew);

	if (sorted = ((mp->sortit || NLidKeys > 0) && (dold.reldum.reltups != 0)))
	{
		sortrel(&dold, &dnew);
		dold.reldum.relindxd = 0;
	}

	if (!dnew.reldum.reldim || NLidKeys > 0)
		/* physically create the new relation */
		if (formatpg(&dnew, dnew.reldum.relprim) != 0)
			syserr("modify: formatpg");

	/* clear relgiven field; if heap remove any keys */
	clearkeys(&dnew);

	if (abs(dnew.reldum.relspec) == M_HEAP)
		for (i = 1; i <= dnew.reldum.relatts; i++)
			dnew.relxtra[i] = 0;

	if (NLidKeys > 0 && dnew.reldum.relspec == M_ISAM)
		sort_isam(&dold, &dnew);

	if (mp->newrelspec != M_TRUNC)
		fill_rel(&dold, &dnew, sorted);

	closer(&dold);	/* error return is impossible */
	if (abs(dnew.reldum.relspec) == M_ISAM && (!dnew.reldum.reldim || NLidKeys > 0))
	{
		j = dnew.reldum.reldim;
		dnew.reldum.reldim = 0;
		if (i = bldindex(&dnew))
			syserr("bldindex: %.14s %d", dnew.reldum.relid, i);
		dnew.reldum.reldim = j;
		unspool(&dold, &dnew);
	}

	/*
	** New relation is now complete. The system relations need to
	** be updated. First destroy all buffers with pages from the
	** new relation.
	*/
	if (i = cleanrel(&dnew))
		syserr("modify:clean new %d,%.14s", i, dnew.reldum.relid);

	fill_batch(&dold, &dnew);

	/*
	** Close the file for the new relation. This must be
	** done after the fill_batch in case we are modifing
	** the attribute relation.
	*/
	if (!dnew.reldum.reldim || NLidKeys > 0)
		close(dnew.relfp);
	dnew.relopn = 0;
	ruboff("modify");
	modupdate();
	if (mp->newrelspec == M_ORDER)
	{
		close(dnew.btree_fd);
		make_bsec(dnew.reldum.relid, dim);
	}
	rubon();

	if (Lockrel)
		unlrl(temptid);

	return (0);
}


/*
**	GETKEYS - get key domains information
**
**	Parameters:
**		ppv - parameter vector with info about keys
**		relname - relation name
**		d - new descriptor for the relation
**		mp - mod table
**
**	Return Codes:
**		0 - ok
**		>0 - error from modseqkey		
*/
getkeys(ppv, relname, d, mp)
PARM		**ppv;
char		*relname;
register DESC	*d;
struct modtab	*mp;
{
	register PARM		*pv;
	register char		*cp;
	int			namemode, sort_only, as_ds;
	int			i, j, keyno, keywid;
	struct attribute	attkey, atttup;
	struct index		ikey, itup;
	TID			tid;
	extern DESC		Attdes, Inddes;
	extern char		*iocv();

	pv = *ppv;	/* copy list of params */

	if (mp->newrelspec != M_ORDER)
		/* zero key info (ordering does not change keyed fields) */
		for (i = 0; i <= d->reldum.relatts; i++)
			d->relxtra[i] = 0;
	for (i = 0; i <= d->reldum.relatts; ++i)
		d->relgiven[i] = 0;

	/* determine whether there are any keys at all */
	keywid = 0;
	keyno = 0;
	sort_only = FALSE;
	cp = pv->pv_val.pv_str;

	if (cp == NULL || *cp == NULL)
	{
		/* no key information. default as needed */
		if (mp->yeskeys && mp->newrelspec != M_ORDER)
		{
			cp = "\1";	/* default to first key */
			namemode = FALSE;
		}
		else
			pv++;	/* point one to far */
	}
	else
	{
		/* check for name mode */
		if (namemode = sequal(cp, "name"))
		{

			/* check attribute names, and convert them to numbers */
			opencatalog("attribute", OR_READ);
			setkey(&Attdes, &attkey, Mod_info.infile, ATTRELID);
			setkey(&Attdes, &attkey, Usercode, ATTOWNER);
		}
		pv++;	/* inc to next key */
		cp = (pv++)->pv_val.pv_str;
	}

	/* scan for attribute names */
	for (; cp != NULL; cp = (pv++)->pv_val.pv_str)
	{
		/* check for separator between keys & options */
		if (*cp == NULL)
		{
			pv++;	/* point two past NULL */
			break;
		}

		if (NLidKeys >= d->reldum.reldim && mp->newrelspec == M_ORDER)
		{
			/* more than one field specified as ordering key */
			closeall(0l, 0l);
			return(error(TOOMANYORDKEYS, relname, 0));
		}

		if (namemode)
		{
			/* check for "sort only" attribute */
			if (*cp == '#')
			{
				cp++;	/* inc to start of name */
				sort_only = TRUE;
			}

			/* check for ascending/descending modifier */
			if ((as_ds = modseqkey(cp, relname, mp->yes_seq)) > 0)
				return (as_ds);	/* error */

			setkey(&Attdes, &attkey, cp, ATTNAME);
			i = getequal(&Attdes, &attkey, &atttup, &tid);
			if (i < 0)
				syserr("MODIFY: geteq(att) %d", i);
			if (i > 0)
			{
				closeall(0l, 0l);
				return (error(INVALIDATTR, relname, cp, 0));	/* bad att name */
			}
			i = atttup.attid;
			if (i > d->reldum.relatts)
			{
				/* attempting to key on lid field which will be
				** removed
				*/
				closeall(0l,0l);
				return(error(ATTRREMV, relname, cp, 0));
			}
		}
		else
		{
			i = *cp;
			as_ds = 0;
		}

		keyno++;
		/* add new key to descriptor */
		if (mp->newrelspec == M_ORDER)
			LidKey[NLidKeys++] = i;
		if (!sort_only && mp->newrelspec != M_ORDER)
		{
			d->relxtra[i] = keyno;
			keywid += (d->relfrml[i] & I1MASK);
		}
		if (d->relgiven[i])
		{
			closeall(0l, 0l);
			return (error(DUPKEY, relname, cp, 0));	/* duplicate attribute */
		}
		d->relgiven[i] = as_ds == 0 ? keyno : -keyno;
	}
	pv--;	/* back up one to point to "-1" terminator */


	if (abs(d->reldum.relspec) == M_ISAM && keywid > (MAXTUP / 2 - 4))
	{
		closeall(0l, 0l);
		return (error(TOOWIDEISAM, relname, iocv(keywid), 0));
	}

	/* if a heap, there can be no keys */
	if (!mp->yeskeys && keyno != 0)
	{
		closeall(0l, 0l);
		return (error(NOKEYSHEAP, relname, mp->type, 0));	/* no keys allowed on heap */
	}
	/* fill out default sort on remainder of keys */
	if (mp->yeskeys)
		for (i = 1; i <= d->reldum.relatts; i++)
			if (d->relgiven[i] == 0)
				d->relgiven[i] = ++keyno;
	*ppv = pv;
	return (0);
}


/*
**	MODSEQKEY - verify that sequence specified is valid
**
**	Parameters:
**		domain - list of domains
**		relname - relation name
**		seq_ok - whether it is ok to specify the sequence
**				ascending or descending
**
**	Return Codes:
**		0 - ok
**		> 0 - error in sequence specified
**
**	Called by:
**		getkeys
*/
modseqkey(domain, relname, seq_ok)
char	*domain;
char	*relname;
int	seq_ok;
{
	register char	*cp, c;
	register int	ret;

	ret = 0;

	for (cp = domain; c = *cp++; )
		if (c == ':')
			break;

	if (c != '\0')
	{
		/* replace ":" with null */
		*(cp - 1) = '\0';

		/* verify sequence is valid */
		if (!seq_ok)
		{
			closeall(0l, 0l);
			ret = error(BADSEQSPEC, relname, cp, domain, 0);
		}
		else if (sequal("descending", cp) || sequal("d", cp))
			ret = -1;
		else if (!(sequal("ascending", cp) || sequal("a", cp)))
		{
			closeall(0l, 0l);
			ret = error(INVALIDSEQ, relname, cp, domain, 0);
		}
	}

	return (ret);
}
/*
**	GETFILL -- Get fill factor and minimum pages parameters
**		from argument list, convert them from ascii to integer
**		and store them in global variables.  If the global
**		variable for the corresponding parameter is zero,
**		it means that that parameter is not allowed and an
**		error is generated.
*/

/*ARGSUSED*/
getfill(d, pv, rel, mp)
DESC		*d;
register PARM	*pv;
char		*rel;
struct modtab	*mp;
{
	register char		*p1;
	register int		err;
	char			*p2;
	int			i, j;
	int			fill_flag, min_flag, max_flag, lid_flag[MAXLID];

	err = 0;
	fill_flag = min_flag = max_flag = FALSE;
	for (i = 0; i < d->reldum.reldim; ++i)
		lid_flag[i] = FALSE;

	while ((p1 = (pv++)->pv_val.pv_str) != NULL)
	{
		p2 = (pv++)->pv_val.pv_str;
		if (sequal(p1, "fillfactor"))
		{
			if (F_fac == 0 || fill_flag)
			{
				err = NOTALLOWED;
				break;
			}
			p1 = p2;
			F_fac = atoi(p1);
			if (F_fac > 100 || F_fac < 1)
			{
				err = FILLBOUND;
				break;
			}
			fill_flag = TRUE;
			continue;
		}
		if (sequal(p1, "minpages"))
		{
			if (Mn_pages == 0 || min_flag)
			{
				err = NOTALLOWED;
				break;
			}
			p1 = p2;
			Mn_pages = atoi(p1);
			if (Mn_pages < 1)
			{
				err = MINPGBOUND;
				break;
			}
			if (max_flag && (Mn_pages > Mx_pages))
			{
				err = MINGTMAX;
				break;
			}
			min_flag = TRUE;
			continue;
		}
		if (sequal(p1, "maxpages"))
		{
			if (Mx_pages == 0 || max_flag)
			{
				err = NOTALLOWED;
				break;
			}
			p1 = p2;
			Mx_pages = atoi(p1);
			if (Mx_pages < 1)
			{
				err = MAXPGBOUND;
				break;
			}
			if (min_flag && (Mn_pages > Mx_pages))
			{
				err = MINGTMAX;
				break;
			}
			max_flag = TRUE;
			continue;
		}
		for ( i  = 1; i <= d->reldum.reldim && !err; ++i)
			if (sequal(p1, ztack("lid", iocv(i))))
			{
				if (lid_flag[i-1] || *Lid[i-1] == NULL)
				{
					err = NOTALLOWED;
					break;
				}
				for (j = 0; j < d->reldum.reldim; ++j)
					if (i - 1 != j && sequal(p2, Lid[j]) && lid_flag[j])
					{
						err = NOTALLOWED;
						break;
					}
				p1 = p2;
				smove(p1, Lid[i - 1]);
				lid_flag[i - 1] = TRUE;
				break;
			}
			if (err)
				break;
		if (i <= d->reldum.reldim)
			continue;
		err = NEEDFILL;
		break;
	}
	if (err)
	{
		closeall(0l, 0l);
		return (error(err, rel, p1, 0));
	}
	return (0);
}
/*
**  MAKE_NEWREL -- Create a file for the modified relation
**	and build one or more primary pages for the
**	relation based on its storage structure and the
**	number of tuples it must hold.
*/

make_newrel(desc)
register DESC	*desc;
{
	register int	tups_p_page;
	int		width;

	concat(MODTEMP, Fileset, Mod_info.reltemp);
	close(creat(Mod_info.reltemp, FILEMODE));
	if ((desc->relfp = open(Mod_info.reltemp, O_RDWR)) < 0)
		syserr("MAKE_NEWREL: open %.14s %d", Mod_info.reltemp, desc->relfp);
	desc->relopn = (desc->relfp + 1) * -5;
	desc->reldum.relprim = 1;
	if (abs(desc->reldum.relspec) == M_HASH && F_fac > 0 && Mn_pages > 0)
	{
		/*
		** Determine the number of primary pages. The following
		** first determines the number of tuples/page which the
		** relation should have in order to get the requested
		** fillfactor. Then that number is divided into the
		** number of tuples to get the number of primary pages.
		** To avoid round off, it must guaranteed that the
		** number of tuples per page must be at least 1.
		**
		** primary_pages = #tuples / (#tuples/page * fillfactor)
		*/
		width = desc->reldum.relwid + 2 - LIDSIZE * desc->reldum.reldim;
		tups_p_page = (((MAXTUP+2) / width) * F_fac) / 100;
		if (tups_p_page == 0)
			tups_p_page = 1;
		 /* we add one to simulate a ceiling function */
		desc->reldum.relprim = desc->reldum.reltups / tups_p_page + 1;
		if (desc->reldum.relprim < Mn_pages)
			desc->reldum.relprim = Mn_pages;
		if (Mx_pages > 0 && desc->reldum.relprim > Mx_pages)
			desc->reldum.relprim = Mx_pages;
#		ifdef xZTR1
		if (tTf(36, 0))
			printf("using %ld prim pages\n", desc->reldum.relprim);
#		endif
	}
	desc->reldum.reltups = 0;
	return (0);
}
/*
**	SORTREL - Call KSORT to sort the given relation.  SORTREL
**		sets up the descriptor struct specifying the sort
**		keys and tells KSORT whether or not the hash key should
**		be included as a sort key.
*/

sortrel(odesc, desc)
DESC		*odesc;
register DESC	*desc;
{
	extern char	*Pathname;
	register int	i;
	char		buf[50];
	DESC		tempdesc;
	char		*temp;
	int		len;
	short		smalli;

	concat(ISAM_SORTED, Fileset, Mod_info.outfile);
	if (close(creat(Mod_info.outfile, FILEMODE)))
		syserr("SORTREL: creat %.14s", Mod_info.outfile);
	bmove(odesc, &tempdesc, sizeof *odesc);
	for (i = 1; i <= desc->reldum.relatts; ++i)
	/* set up temporary descriptor for ksort with new keyed fields */
	{
		tempdesc.relxtra[i] = desc->relxtra[i];
		tempdesc.relgiven[i] = desc->relgiven[i];
	}

	if (abs(desc->reldum.relspec) == M_HASH && !desc->reldum.reldim)
	{
		/* sort on hash bucket first, (if ordering sort on ordering key, not bucket) */
		tempdesc.relgiven[0] = 1;
		for (i = 1; i <= desc->reldum.relatts; i++)
			tempdesc.relgiven[i]++;
	}

# ifdef xZTR2
	if (tTf(36, 4))
	{
		printf("sortrel: ");
		printdesc(&tempdesc);
	}
# endif

	/* flush buffers used by modify so that ksort can't look at them */
	flush_rel(desc, TRUE);
	resetacc(NULL);

	/* copy Fileset so it can't get trashed */

	len = length(Fileset) + 1;
	temp = (char *) need(Qbuf, len);
	bmove(Fileset, temp, len);

	initp();
	setp(PV_STR, temp);
	setp(PV_STR, Mod_info.infile);
	setp(PV_STR, Mod_info.outfile);

	/* Descriptor for new relation */
	setp(PV_STR, tempdesc.reldum.relid);
	setp(PV_STR, tempdesc.reldum.relowner);
	setp(PV_INT, tempdesc.reldum.relspec);
	setp(PV_INT, tempdesc.reldum.relindxd);
	setp(PV_INT, tempdesc.reldum.relstat2);
	setp(PV_INT, tempdesc.reldum.relstat);
	setp(PV_INT, (short) tempdesc.reldum.relsave);
	setp(PV_INT, (short) tempdesc.reldum.reltups);
	setp(PV_INT, tempdesc.reldum.relatts);
	setp(PV_INT, tempdesc.reldum.relwid);
	setp(PV_INT, (short) tempdesc.reldum.relprim);
	setp(PV_INT, (short) tempdesc.reldum.relfree);
	setp(PV_INT, (short) tempdesc.reldum.relstamp);
	setp(PV_INT, tempdesc.reldum.reldim);

	setp(PV_STR, tempdesc.relvname);
	setp(PV_INT, tempdesc.relfp);
	setp(PV_INT, tempdesc.relopn);
	setp(PV_INT, (short) tempdesc.reladds);
	setp(PV_INT, tempdesc.reltid.ltid);
	for (i = 0; i <= tempdesc.reldum.relatts; ++i)
	{
		smalli = (short) tempdesc.reloff[i];
		setp(PV_INT, smalli);
		smalli = (short) tempdesc.relfrmt[i];
		setp(PV_INT, smalli);
		smalli = (short) tempdesc.relfrml[i];
		setp(PV_INT, smalli);
		smalli = (short) tempdesc.relxtra[i];
		setp(PV_INT, smalli);
		smalli = (short) tempdesc.relgiven[i];
		setp(PV_INT, smalli);
	}

	if (tempdesc.reldum.reldim > 0)
	{
		setp(PV_STR, odesc->relbtree->reldum.relid);
		setp(PV_STR, odesc->relbtree->reldum.relowner);
		setp(PV_INT, odesc->relbtree->reldum.relspec);
		setp(PV_INT, odesc->relbtree->reldum.relindxd);
		setp(PV_INT, odesc->relbtree->reldum.relstat2);
		setp(PV_INT, odesc->relbtree->reldum.relstat);
		setp(PV_INT, (short) odesc->relbtree->reldum.relsave);
		setp(PV_INT, (short) odesc->relbtree->reldum.reltups);
		setp(PV_INT, odesc->relbtree->reldum.relatts);
		setp(PV_INT, odesc->relbtree->reldum.relwid);
		setp(PV_INT, (short) odesc->relbtree->reldum.relprim);
		setp(PV_INT, (short) odesc->relbtree->reldum.relfree);
		setp(PV_INT, (short) odesc->relbtree->reldum.relstamp);
		setp(PV_INT, odesc->relbtree->reldum.reldim);

		setp(PV_STR, odesc->relbtree->relvname);
		setp(PV_INT, odesc->relbtree->relfp);
		setp(PV_INT, odesc->relbtree->relopn);
		setp(PV_INT, (short) odesc->relbtree->reladds);
		setp(PV_INT, odesc->relbtree->reltid.ltid);

		for (i = 0; i <= odesc->relbtree->reldum.relatts; ++i)
		{
			smalli = (short) odesc->relbtree->reloff[i];
			setp(PV_INT, smalli);
			smalli = (short) odesc->relbtree->relfrmt[i];
			setp(PV_INT, smalli);
			smalli = (short) odesc->relbtree->relfrml[i];
			setp(PV_INT, smalli);
			smalli = (short) odesc->relbtree->relxtra[i];
			setp(PV_INT, smalli);
			smalli = (short) odesc->relbtree->relgiven[i];
			setp(PV_INT, smalli);
		}
	}

	call(mdKSORT, NULL);

	/* flush buffers used by ksort so that modify can't look at them */
	flush_rel(desc, TRUE);
	resetacc(NULL);

# ifdef xZTR1
	if (tTf(36,9))
		printf("SORTREL: done calling ksort\n");
#endif
	return (0);
}
/*
**	SORT_ISAM -- Sorts an isam relation back to its original order
**	so that it will be inserted into the relation in the proper order.
**	It is presently not in order because it has been sorted according
**	to a specified field for ordering.
*/
sort_isam(sdesc, desc)
DESC *sdesc;
DESC *desc;
{
	long		lid[MAXLID];
	register int	i, j, k;
	char		tup_buf[MAXTUP], last_tup[MAXTUP], *dp, *sp;
	FILE		*sfp, *fp;
	TID		tid, tidpos;
	DESC		tempdesc;
	int		w;

	if (desc->reldum.reldim > 0)
		Btree_fd = desc->btree_fd;
	concat(STEMP, Fileset, Mod_info.temp_sort);
	if ((sfp = fopen(Mod_info.temp_sort, "w")) == NULL)
		syserr("sort_isam: can't open %s", Mod_info.temp_sort);
	if ((fp = fopen(Mod_info.outfile, "r")) == NULL)
		syserr("sort_isam: can't open %s", Mod_info.outfile);
	for (i = 0; i < desc->reldum.reldim; lid[i++] = 0);
	/* create input file for sort with proper lid attached to each tuple */
	w = sdesc->reldum.relwid - LIDSIZE * sdesc->reldum.reldim;
	for ( ; ; )
	{
		i = fread(tup_buf, 1, sdesc->reldum.relwid, fp);
		if (i == 0)
			break;
		if (i != sdesc->reldum.relwid)
			syserr("sort_isam: read error in %s", Mod_info.outfile);
		for (j = 0; j < desc->reldum.reldim; ++j)
			if (j < NLidKeys && j < desc->reldum.reldim - 1)
			{
				dp = tup_buf + (sdesc->reloff[LidKey[j]] & I1MASK);
				sp = last_tup + (sdesc->reloff[LidKey[j]] & I1MASK);
				if (!bequal(dp, sp, sdesc->relfrml[LidKey[j]]) || !lid[j])
				{
					++lid[j];
					for (k = j + 1; k < desc->reldum.reldim; ++k)
						lid[k] = 0;
					break;
				}
			}
			else
			{
				if (!lid[0])
				{
					lid[0] = 1;
					if (!(desc->reldum.reldim - 1))
						break;
				}
				++lid[desc->reldum.reldim - 1];
				break;
			}
		bmove(tup_buf, last_tup, sdesc->reldum.relwid);
		/* reserve a slot in btree for tuple */
		insert_mbtree(desc, Mod_info.btree, lid, &tid, &tidpos);
		bmove(lid, tup_buf + w, LIDSIZE * desc->reldum.reldim);
		if (fwrite(tup_buf, 1, sdesc->reldum.relwid + LIDSIZE * desc->reldum.reldim, sfp) != sdesc->reldum.relwid + LIDSIZE * desc->reldum.reldim)
			syserr("sort_isam: write error in %s", Mod_info.temp_sort);
	}
	fclose(fp);
	fclose(sfp);
	/* set up new descriptor accounting for lid field */
	bmove(sdesc, &tempdesc, sizeof *sdesc);
	tempdesc.reldum.relspec = M_ORDER;
	for (i = 0; i < desc->reldum.reldim; ++i)
	{
		tempdesc.reldum.relwid += LIDSIZE;
		++tempdesc.reldum.relatts;
		tempdesc.reloff[tempdesc.reldum.relatts] = tempdesc.reldum.relwid - LIDSIZE;
		tempdesc.relfrmt[tempdesc.reldum.relatts] = INT;
		tempdesc.relfrml[tempdesc.reldum.relatts] = LIDSIZE;
	}
	j = 0;
	/* use old keying attributes for specifying sort order */
	clearkeys(&tempdesc);
	for (i = 1; i <= sdesc->reldum.relatts; ++i)
		if (sdesc->relxtra[i])
		{
			tempdesc.relgiven[i] = sdesc->relxtra[i];
			++j;
		}
	for (i = 1; i <= tempdesc.reldum.relatts; ++i)
		if (!tempdesc.relgiven[i])
			tempdesc.relgiven[i] = ++j;
	sortfile(Mod_info.temp_sort, &tempdesc, FALSE);
	if (unlink(Mod_info.outfile) < 0)
		syserr("can't unlink %s", Mod_info.outfile);
	if (link(ztack(REPL_OUT, Fileset), Mod_info.outfile) == -1)
		syserr("can't link %s to %s", ztack(REPL_OUT, Fileset), Mod_info.outfile);
	if (unlink(Mod_info.temp_sort) < 0)
		syserr("sort_isam: can't unlink %s", Mod_info.temp_sort);
	if (unlink(ztack(REPL_OUT, Fileset)) < 0)
		syserr("sort_isam: can't unlink replout file");
}
/*
**	FILL_REL -- Fill the new relation with tuples from either
**		the old relation or the output file of KSORT.
*/

fill_rel(sdesc, desc, sortit)
register DESC	*sdesc, *desc;
char			sortit;
{
	register int	i;
	char		tup_buf[MAXTUP], last_tup[MAXTUP], tup[2 * LIDSIZE]; 
	char		junk[4], newreltype, anytups, chkdups;
	int		need, j, k;
	long		lnum, lid[MAXLID], l, page, t;
	TID		tid, stid, stidlim, ntid, tidpos, btid;
	FILE		*fp, *spfp;
	char		*dp, *sp;
	int		w, temp;
	struct locator	tidloc;

	newreltype = abs(desc->reldum.relspec);
	if (sortit)
	{
		if ((fp = fopen(Mod_info.outfile, "r")) == NULL)
			syserr("FILL_REL: fopen %.14s", Mod_info.outfile);
	}
	else
	{
		cleanrel(sdesc);	/* make sure each page is read fresh */
		find(sdesc, NOKEY, &stid, &stidlim);
	}
	if (newreltype == M_ISAM && (NLidKeys > 0 || !desc->reldum.reldim))
	{
		lnum = 0;
		stuff_page(&tid, &lnum);
		tid.line_id = 0;
		get_page(desc, &tid);
		concat(ISAM_SPOOL, Fileset, Mod_info.spfile);
		/* assume that spool file is not needed */
		spfp = NULL;
		Mod_info.spflag = FALSE;
		if (F_fac == 0)
			F_fac = 100;
		/* setup relgiven field for kcompare later on */
		for (i = 1; i <= desc->reldum.relatts; i++)
			desc->relgiven[i] = desc->relxtra[i];
		if (desc->reldum.reldim)
			Btree_fd = desc->btree_fd;
	}
	desc->reladds = 0;
	for (i = 0; i < desc->reldum.reldim; lid[i++] = 0)
		continue;
	anytups = FALSE;
	chkdups = !sortit && (newreltype != M_ORDER);
# ifdef xZTR2
	if (tTf(36, 3))
	{
		printf("  FILLREL: ");
		printdesc(sdesc);
		printdesc(desc);
	}
# endif
	for (;;)
	{
		w = (newreltype == M_ISAM) ? sdesc->reldum.relwid + desc->reldum.reldim * LIDSIZE : sdesc->reldum.relwid;
		if (sortit)
		{
			i = fread(tup_buf, 1, w, fp);
			if (i == 0)
				break;
			if (i != w)
				syserr("FILL_REL: fread A %d", i);
			if (newreltype == M_HASH && !desc->reldum.reldim)
				if (fread(junk, 1, 4, fp) != 4)
					syserr("FILL_REL: fread B");
		}
		else
		{
#			ifdef xZTR2
			if (tTf(36, 1))
			{
				printf("FILL_REL: stid ");
				dumptid(&stid);
				printf("FILL_REL: stidlim ");
				dumptid(&stidlim);
			}
#			endif
			i = get(sdesc, &stid, &stidlim, tup_buf, TRUE);
#			ifdef xZTR2
			if (tTf(36, 1))
			{
				printf("FILLREL: get %d ", i);
				printup(sdesc, tup_buf);
			}
#			endif
			if (i < 0)
				syserr("FILL_REL: get %d", i);
			if (i == 1)
				break;
		}
		if (newreltype != M_ISAM || (newreltype == M_ISAM && NLidKeys == 0 && desc->reldum.reldim > 0))
		{
			for (j = 0; j < desc->reldum.reldim; ++j)
				if (j < NLidKeys && j < desc->reldum.reldim - 1)
				{
					dp = tup_buf + (sdesc->reloff[LidKey[j]] & I1MASK);
					sp = last_tup + (sdesc->reloff[LidKey[j]] & I1MASK);
					if (!bequal(dp, sp, sdesc->relfrml[LidKey[j]]) || !lid[j])
					{
						++lid[j];
						for (k = j + 1; k < desc->reldum.reldim; ++k)
							lid[k] = 0;
						break;
					}
				}
				else
				{
					if (!lid[0])
					{
						lid[0] = 1;
						if (!(desc->reldum.reldim -1))
							break;
					}
					++lid[desc->reldum.reldim - 1];
					break;
				}
			Btree_fd = desc->btree_fd;
			if (!desc->reldum.reldim || NLidKeys > 0)
			{
				/* assume unordered so btree inserts done
				** separately */
				temp = 0;
				if (desc->reldum.reldim > 0)
				{
					temp = desc->reldum.reldim;
					desc->reldum.reldim = 0;
					desc->reldum.relwid -= temp * LIDSIZE;
				}
				if ((i = insert(desc, &tid, tup_buf, chkdups)) < 0)
					syserr("FILL_REL: insert %d", i);
				if (NLidKeys > 0)
				{
					bmove(&tid, &stid, LIDSIZE);
					desc->reldum.reldim = temp;
					desc->reldum.relwid += temp * LIDSIZE;
					insert_mbtree(desc, Mod_info.btree, lid, &tid, &tidpos);
				}
			}
			if (desc->reldum.reldim && !NLidKeys)
			{
				/* new relation not changed, only lids added */
				page = RT;
				for (j = 0; j < desc->reldum.reldim - 1; ++j)
				{
					if (!lid[j])
						lid[j] = 1;
					if ((t = get_tid(page, lid[j], &tidloc)) < 0)
					{
						insert_btree(Mod_info.btree, page, lid[j], &ntid, &tidpos, j + 2);
						bmove(&ntid, &page, LIDSIZE);
					}
					else
						bmove(&t, &page, LIDSIZE);
				}
				insert_btree(Mod_info.btree, page, lid[abs(desc->reldum.reldim) - 1], &stid, &tidpos, FALSE);
			}
			bmove(tup_buf, last_tup, sdesc->reldum.relwid);
			if (desc->reldum.reldim > 0)
			{
				dp = tup_buf + desc->reldum.relwid - desc->reldum.reldim * LIDSIZE;
				bmove(lid, dp, LIDSIZE * desc->reldum.reldim);
			}
#			ifdef xZTR2
			if (tTf(36, 2))
			{
				printf("FILL_REL: insert ");
				printup(desc, tup_buf);
				printf("FILL_REL: insert ret %d at", i);
				dumptid(&tid);
			}
#			endif
			continue;
		}
		if (anytups)
			i = kcompare(desc, tup_buf, last_tup);
		else
		{
			anytups = TRUE;
			i = 1;
		}
		bmove(tup_buf, last_tup, desc->reldum.relwid);
		need = canonical(desc, tup_buf);
		if (i == 0 && need > space_left(Acc_head))
		{
			/* spool out this tuple. will go on overflow page later */
			if (spfp == NULL)
			{
				if ((spfp = fopen(Mod_info.spfile, "w")) == NULL)
					syserr("FILL_REL: fopen %.14s", Mod_info.spfile);
				Mod_info.spflag = TRUE;
			}
			if (fwrite(tup_buf, 1, desc->reldum.relwid, spfp) != desc->reldum.relwid)
				syserr("FILL_REL: putb spool");
			continue;
		}
		j = (100 - F_fac) * MAXTUP / 100;
		if (j < need)
			j = need;
		if (i != 0 && j > space_left(Acc_head))
		{
			if (i = add_prim(desc, &tid))
				syserr("FILL_REL: force ovflo %d", i);
		}
		tid.line_id = newlino(need);
		put_tuple(&tid, Acctuple, need);
		if (NLidKeys > 0)
		{
			bmove(tup_buf + desc->reldum.relwid - LIDSIZE * desc->reldum.reldim, lid, LIDSIZE * desc->reldum.reldim);
			page = RT;
			for (j = 0; j < desc->reldum.reldim; ++j)
			{
				if ((t = get_tid(page, lid[j], &tidloc)) < 0)
					syserr("get_tid error in modify isam ordered");
				page = t;
			}
			stuff_page(&btid, &tidloc.pageno);
			btid.line_id = tidloc.page.node.leafnode.tid_loc[tidloc.offset];
			/* place proper tid in tree */
			replace_btree(tid, &btid);
		}
		desc->reladds++;
	}
	if (sortit)
	{
		fclose(fp);
		unlink(Mod_info.outfile);
	}
	if (newreltype == M_ISAM && desc->reldum.reldim <= 0)
	{
		if (i = pageflush(Acc_head))
			syserr("fill_rel:pg clean %d", i);
		if (spfp != NULL)
			fclose(spfp);
	}
	if (!desc->reldum.reldim || NLidKeys > 0)
		desc->reldum.reltups = desc->reladds;
	desc->reladds = 0;
	return (0);
}


/*
**	BLDINDEX -	
**
**	Parameters:
**		d - descriptor for relation
**
**	Return Codes:
**		0 - ok
**		<0 - error
**
**	Trace Flags:
**		Z38.7, Z38.8
**
**	Called by:
**		modify
**
*/
bldindex(d)
register DESC	*d;
{
	register TID	*tid;
	register int	tmp;
	TID		tidx;
	struct accbuf	dirbuf;
	int		keywid, level, savespec, keyx[MAXDOM];
	int		offset, len;
	char		tuple[MAXTUP], temptup[MAXTUP], *key;
	long		pageid, start, stop, newstart, newstop;

	tid = &tidx;
	keywid = 0;
	for (tmp = 0; tmp < MAXDOM; tmp++)
		keyx[tmp] = 0;
	for (tmp = 1; tmp <= d->reldum.relatts; tmp++)
		if (d->relxtra[tmp] > 0)
		{
			keyx[d->relxtra[tmp] - 1] = tmp;
			keywid += d->relfrml[tmp] & I1MASK;
		}

	/* Determine the last page of the relation. This will
	** only work if all pages have been written out. Fill_rel
	** must guarantee that all pages have been written
	*/
	level = 0;
	last_page(d, tid, 0);
	pluck_page(tid, &stop);
	start = 0;
	dirbuf.filedesc = d->relfp;
	dirbuf.rel_tupid = d->reltid.ltid;
	savespec = d->reldum.relspec;
	for (;;)
	{
#		ifdef xZTR2
		if (tTf(38, 7))
			printf("isam: level %d\n", level);
#		endif
		dirbuf.ovflopg = start;
		dirbuf.mainpg = level;
		dirbuf.thispage = stop + 1;
		dirbuf.linetab[0] = (short) (dirbuf.firstup - (char *) &dirbuf);
		offset = dirbuf.linetab[0];
		dirbuf.bufstatus = BUF_DIRTY | BUF_DIRECT;

		dirbuf.nxtlino = 0;
		newstart = stop + 1;
		newstop = newstart;
		for (pageid = start; pageid <= stop; pageid++)
		{
#			ifdef xZTR2
			if (tTf(38, 8))
				printf("isam:get key from %ld\n", pageid);
#			endif
			stuff_page(tid, &pageid);
			tid->line_id = 0;
			if (tmp = get(d, tid, tid, tuple, FALSE))
			{
				/*
				** If the relation is empty, then page 0 will
				** return AMINVL_ERR on a get(). Form a blank tuple
				** and use it to create a one tuple directory
				*/
				if (pageid == 0 && tmp == AMINVL_ERR)
				{
					clr_tuple(d, tuple);
				}
				else
				{
					return (-2);
				}
			}

			/*
			** If this is the first level then form the tuple
			** from the mainpage of the relation. Otherwise
			** the tuple is the first tuple of a directory page
			** and it is already correctly formed.
			*/
			if (level == 0)
			{
				key = temptup;
				for (tmp = 0; keyx[tmp] != 0; tmp++)
				{
					len = d->relfrml[keyx[tmp]] & I1MASK;
					bmove(&tuple[d->reloff[keyx[tmp]]], key, len);
					key += len;
				}
				key = temptup;
			}
			else
				key = tuple;

			if (keywid > space_left(&dirbuf))
			{
				if (pageflush(&dirbuf))
					return (-3);
				dirbuf.thispage++;
				newstop = dirbuf.thispage;
				dirbuf.ovflopg = pageid;
				dirbuf.linetab[0] = (short) (dirbuf.firstup - (char *) &dirbuf);
				offset = dirbuf.linetab[0];
				dirbuf.bufstatus = BUF_DIRTY;
				dirbuf.nxtlino = 0;
			}
			/* copy key to directory page */
			bmove(key, (char *) &dirbuf + offset, keywid);

			/* update next line number */
			offset += keywid;
			dirbuf.nxtlino++;
			dirbuf.linetab[-dirbuf.nxtlino] = offset;
		}
		if (pageflush(&dirbuf))
			return (-4);
		if (newstart == newstop)
			break;
		d->reldum.relspec = abs(d->reldum.relspec);
		level++;
		start = newstart;
		stop = newstop;
	}
	d->reldum.relspec = savespec;
	d->reldum.relprim = newstart;
	return (0);
}
/*
**	UNSPOOL -- Take tuples saved in spool file and insert them
**		in new relation.  This is only for ISAM relations.
*/

unspool(sdesc, desc)
register DESC	*sdesc, *desc;
{
	register int	i, j;
	TID		tid, btid;
	char		tup_buf[MAXTUP], tup[2 * LIDSIZE];
	FILE		*spfp;
	long		lid[MAXLID], page, t;
	int		w;
	struct locator  tidpos;

	w = sdesc->reldum.relwid + desc->reldum.reldim * LIDSIZE;
	if (Mod_info.spflag)
	{
		if ((spfp = fopen(Mod_info.spfile, "r")) == NULL)
			syserr("UNSPOOL: fopen spool");
		while ((i = fread(tup_buf, 1, w, spfp)) == w)
		{
			if ((i = insert(desc, &tid, tup_buf, FALSE)) < 0)
				syserr("UNSPOOL: insert %.14s %d", desc->reldum.relid, i);
			if (NLidKeys > 0)
			{
				bmove(tup_buf + desc->reldum.relwid - LIDSIZE * desc->reldum.reldim, lid, LIDSIZE * desc->reldum.reldim);
				page = RT;
				for (j = 0; j < desc->reldum.reldim; ++j)
				{
					if ((t = get_tid(page, lid[j], &tidpos)) < 0)
						syserr("get_tid error in unspool");
					page = t;
				}
				stuff_page(&btid, &tidpos.pageno);
				btid.line_id = tidpos.page.node.leafnode.tid_loc[tidpos.offset];
				replace_btree(tid, &btid);
			}
		}
		if (i != 0)
			syserr("UNSPOOL: read %d", i);
		fclose(spfp);
		unlink(Mod_info.spfile);
	}
	desc->reldum.reltups += desc->reladds;
	desc->reladds = 0;
	return (0);
}
/*
**	FILL_BATCH -- Create and fill a batch file containing the
**		updates for the system catalog so that MODIFY will
**		be recoverable if the system crashes.
*/

fill_batch(odesc, desc)
DESC		*odesc;
register DESC	*desc;
{
	register DESC		*dessys;
	register int		i, k;
	struct relation		reltup, rkey;
	TID			tid, lotid, hitid;
	struct attribute	atttup, akey;
	int			numatts, j;
	char			prebatch[MAXNAME + 4], modbatch[MAXNAME + 4];

	if (bequal(desc->reldum.relid, "relation    ", 12))
	{
		clearkeys(desc);
		setkey(desc, &rkey, desc->reldum.relid, RELID);
		setkey(desc, &rkey, desc->reldum.relowner, RELOWNER);
		if (i = getequal(desc, &rkey, &reltup, &tid))
			syserr("FILL_BATCH: geteq rel rel %d", i);
		bmove(&tid, &desc->reltid, sizeof desc->reltid);
	}
	else
		bmove(&odesc->reltid, &desc->reltid, sizeof desc->reltid);
	resetacc(Acc_head);
	concat(MOD_PREBATCH, Fileset, prebatch);
	close(creat(prebatch, FILEMODE));
	if ((Batch_fp = open(prebatch, O_RDWR)) < 0)
		syserr("FILL_BATCH: open %.14s %d", prebatch, Batch_fp);
	smove(Fileset, Batchbuf.file_id);
	Batch_cnt = 0;
	wrbatch(desc, sizeof *desc);
	if (bequal(desc->reldum.relid, "attribute   ", 12))
		dessys = desc;
	else
		dessys = &Admin.adattd;
	clearkeys(dessys);
	setkey(dessys, &akey, desc->reldum.relid, ATTRELID);
	setkey(dessys, &akey, desc->reldum.relowner, ATTOWNER);
	if (i = find(dessys, EXACTKEY, &lotid, &hitid, &akey))
		syserr("FILL_BATCH: find %d", i);

	/* if ordered relation, one of attributes is LID field */
	numatts = j = desc->reldum.relatts - desc->reldum.reldim;

	while(!(i = get(dessys, &lotid, &hitid, &atttup, TRUE)) && j > 0)
		if (!kcompare(dessys, &akey, &atttup))
			if (atttup.attid <= numatts)
			{
				j--;
				atttup.attxtra = desc->relxtra[atttup.attid];
				wrbatch(&lotid, sizeof lotid);
				wrbatch(&atttup, sizeof atttup);
			}
	for (k = 1; k <= desc->reldum.reldim; ++k)
	/* create new tuple corresponding to LID field; LID field is the
	** last field of relation, a 4-byte integer 
	*/
	{
		smove(desc->reldum.relid, atttup.attrelid);
		bmove(desc->reldum.relowner, atttup.attowner, 2);
		atttup.attid = numatts + k;
		smove(Lid[k - 1], atttup.attname);
		pad(atttup.attname, MAXNAME);
		atttup.attoff = desc->reldum.relwid - (desc->reldum.reldim - k + 1) * LIDSIZE;
		atttup.attfrmt = INT;
		atttup.attfrml = LIDSIZE;
		atttup.attxtra = 0;
		wrbatch(&atttup, sizeof atttup);
	}
	if (i < 0 || j > 0)
		syserr("FILL_BATCH: get att %d count %d", i, j);
	/* get rid of attribute pages */
	cleanrel(dessys);
	flushbatch();
	close(Batch_fp);
	concat(MODBATCH, Fileset, modbatch);
	if (link(prebatch, modbatch) == -1)
		syserr("FILL_BATCH: can't link %.14s %.14s",
			prebatch, modbatch);
	unlink(prebatch);
	return (0);

}

/*
**	MAKE_BSEC -- Creates the seecondary btree relation by first creating
**		a heaped relation.  The main relation tids are found by
**		scanning the leaves of the btree.  The relation is then 
**		modified to an isam relation.
*/	

make_bsec(relname, dim)
char *relname;
int dim;
{
	PARM		pv[8];
	register int	i;
	DESC		bdesc;
	TID		tid, btid;
	long		mtid, page, t, next;
	char		tuple[2 * LIDSIZE], btree[MAXNAME], btreefile[MAXNAME + 4];
	struct locator	tidpos;
	extern char	*iocv();
	extern DESC	Reldes;

	pv[0].pv_val.pv_str = "0000002";
	capital(trim_relname(relname), btree);
	pv[1].pv_val.pv_str = btree;
	pv[2].pv_val.pv_str = "mtid";
	pv[3].pv_val.pv_str = "i4";
	pv[4].pv_val.pv_str = "btid";
	pv[5].pv_val.pv_str = "i4";
	pv[6].pv_type = PV_EOF;
	if (create(6, pv))
		syserr("can't create btreesec %s", pv[1].pv_val.pv_str);
	
	if (noclose(&Reldes))
		syserr("noclose in make_bsec");

	if (i = openr(&bdesc, OR_WRITE, btree))
		syserr("opening bsec relation %d", i);
	btreename(relname, btreefile);
	if ((Btree_fd = open(btreefile, O_RDWR)) < 0)
		syserr("make_bsec: can't open %s", btreefile);
	page = RT;
	for (i = 0; i < dim - 1; ++i)
	{
		t = get_tid(page, 1, &tidpos);
		if (t < 0)
			break;	/* lid value doesn't exist */
		bmove(&t, &page, LIDSIZE);
	}
	if (t >= 0)	/* only do inserts if there are lids! */
	{
		do
		{
			get_node(page, &tidpos.page);
			next = tidpos.page.nexttree;
			get_tid(page, 1, &tidpos);
			page = tidpos.pageno;
			for (;;)
			/* scan through leaves of btree */
			{
				stuff_page(&btid, &page);
				for (i = 0; i < tidpos.page.nelmts; ++i)
				{
					btid.line_id = tidpos.page.node.leafnode.tid_loc[i];
					mtid = tidpos.page.node.leafnode.tid_pos[btid.line_id];
					/* form tuple */
					bmove(&mtid, tuple, LIDSIZE);
					bmove(&btid, tuple + LIDSIZE, LIDSIZE);
					if (insert(&bdesc, &tid, tuple, TRUE) < 0)
						syserr("insert error in btreesec");
				}
				page = tidpos.page.node.leafnode.nextleaf;
				if (page == NULL)
					break;
				else 
					get_node(page, &tidpos.page);
			}
		} while (page = next);
	}
	close(Btree_fd);
	closer(&bdesc);

	/* modify to isam on mtid */
	pv[0].pv_val.pv_str = btree;
	pv[1].pv_val.pv_str = "isam";
	pv[2].pv_val.pv_str = "name";
	pv[3].pv_val.pv_str = "mtid";
	pv[4].pv_val.pv_str = "\0";
	pv[5].pv_val.pv_str = "fillfactor";
	/* use fillfactor provided for main relation */
	if (F_fac == 0)
		pv[6].pv_val.pv_str = iocv(80);
	else
		pv[6].pv_val.pv_str = iocv(F_fac);
	pv[7].pv_type = PV_EOF;
	if (modify(7, pv))
		syserr("can't modify btreesec to isam");
}
