# include	<pv.h>
# include	<ingres.h>
# include	<aux.h>
# include	<catalog.h>
# include	<access.h>
# include	<func.h>
# include	<signal.h>
# include	<sccs.h>
# include	<errors.h>

SCCSID(@(#)help.c	8.5	2/8/85)

extern short 	tTdbu[100];
extern int	help();
extern int	null_fn();

struct fn_def HelpFn =
{
	"HELP",
	help,
	null_fn,		/* initialization function */
	null_fn,
	NULL,
	0,
	tTdbu,
	100,
	'Z',
	0
};


/*
**  HELP - Provide Information to User
**
**	Arguments:
**		pv[i] - code
**			RELINFO - print relation information
**			MANSEC - print manual section
**			DELIMLIST - print delim information
**			RELIST - print relation list
**			ALLRELINFO - print relation info for all accessible 
**						relations
**
**		pv[i+1] - name of entity for modes 0 or 1
**
**	Trace Flags:
**		44
*/

help(parmc, parmv)
int	parmc;
PARM	parmv[];
{
	DESC		des;
	int		mode;
	register PARM	*pv;
	register int	ret;
	int		i;

# ifdef	xZTR1
	if (tTf(44, -1))
	{
		printf(">>help\n");
		if (tTf(44, 0))
			prvect(parmc, parmv);
	}
# endif

	ret = 0;
	pv = parmv;
	getuser(-1);	/* init getuser for modes RELINFO & MANSEC */
	while (pv->pv_type != PV_EOF)
	{
		mode = (pv++)->pv_val.pv_int;
		if (mode < RELIST && pv->pv_type == PV_EOF)
			syserr("help: mode %d no val", mode);

#		ifdef xZTR1
		if (tTf(44, -1))
		{
			printf("help %d", mode);
			if (mode != RELIST)
				printf(" %s", pv->pv_val.pv_str);
			putchar('\n');
		}
#		endif
		switch (mode)
		{

		  case RELINFO:	/* help relation */
			if (!openr(&des, OR_RELTID, pv->pv_val.pv_str))
			{
				rel_fmt(&des);
				pv->pv_val.pv_str = NULL;
			}
			pv++;
			break;

		  case MANSEC:	/* help manual section */
			if (man(pv->pv_val.pv_str))
				pv->pv_val.pv_str = NULL;
			pv++;
			break;

		  case DELLIST:	/* help delim */
			if (!openr(&des, OR_READ, "rdelim"))
			{
				if ((i = helpdelim(pv->pv_val.pv_str,&des)) == -1)
					return(error(RDELIMERR, pv->pv_val.pv_str, 0));
				closer(&des);
				pv->pv_val.pv_str = NULL;
			}
			pv++;
			break;

		  case RELIST:
		  case ALLRELINFO:
			relpr(mode);
			break;

		  case ALLDELLIST:
			if (!openr(&des, OR_READ, "rdelim"))
			{
				if ((i = alldelims(&des)) == -1)
					return(error(RDELIMERR, pv->pv_val.pv_str, 0));
				closer(&des);
				pv->pv_val.pv_str = NULL;
			}
			pv++;
			break;

		  default:
			syserr("HELP: mode %d", mode);
		}
	}
	getuser(0);	/* close getuser in case mode RELINFO or MANSEC */

	/* now rescan for error messages */
	pv = parmv;
	while (pv->pv_type != PV_EOF)
	{
		mode = (pv++)->pv_val.pv_int;

		if (mode < RELIST)
		{
			if (pv->pv_val.pv_str != NULL)
				ret = nferror(NORELEXIST + mode, pv->pv_val.pv_str, 0);
			pv++;
		}
	}
	return (ret);
}


/*
**  Nroff Manual Section
**
**	The manual section given by 'name' is nroff'ed.  Returns one
**	on success, zero if the manual section is not found.
**
**	Uses trace flag 11
*/

int
man(name)
char	*name;
{
	char		manual[100];
	register int	i;
	int		stat;
	char		name_nr[18];
	register char	*naa;
	extern char	*ztack();

	if (length(name) > 14)
		return (0);

	/* a null manual name gives table of contents */
	if (name[0] == 0)
		smove("../toc.nr", name_nr);
	else
		concat(name, ".nr", name_nr);

	concat(ztack(Pathname, "/doc/quel/"), name_nr, manual);
	if ((i = open(manual, O_RDONLY)) < 0)
	{
		/* try a unix command instead */
		concat(ztack(Pathname, "/doc/unix/"), name_nr, manual);
		if ((i = open(manual, O_RDONLY)) < 0)
			return (0);
	}
	if (close(i))
		syserr("cannot close %s", manual);
	ruboff(0);	/* wait for child's death if rubout occures */
	i = fork();
	if (i == 0)
	{
		signal(SIGINT, SIG_DFL);	/* die on rubout */
		setuid(getuid());
#		ifndef xB_UNIX
		setgid(getgid());
#		endif
		naa = ztack(Pathname, "/doc/iaa");
		execl("/bin/nroff", "nroff", naa, manual, 0);
		execl("/usr/bin/nroff", "nroff", naa, manual, 0);
		syserr("help: exec: nroff");
	}
	/* wait for nroff if fork succeeded */
	if (i > 0)
		fullwait(i, "help: nroff");
	rubon();
	return (1);
}


/*
**  PRINT DATABASE INFORMATION
**
**	Prints a list of all the relations in the database, together
**	with their owner.
**
**	Uses trace flag 12
*/

relpr(mode)
int	mode;
{
	extern DESC	Reldes;
	register DESC	*d;
	register int			i;
	register char			*cp;
	struct tup_id			limtid, tid;
	char				buf[MAXLINE + 1];
	char				lastuser[2];
	struct relation			rel;

	opencatalog("relation", OR_READ);
	d = &Reldes;
	if (i = find(d, NOKEY, &tid, &limtid))
		syserr("help: relpr: find %d", i);

	lastuser[0] = '\0';

	if (mode == RELIST)
		printf("\n relation name     relation owner\n\n");

	while ((i = get(d, &tid, &limtid, &rel, 1)) == 0)
	{
		if (mode == RELIST)
		{
			if (!bequal(lastuser, rel.relowner, 2))
			{
				if (getuser(rel.relowner, buf))
				{
					/* cant find user code */
					bmove("  ", buf, 2);
					cp = &buf[2];
					bmove(rel.relowner, cp, 2);
					cp = &cp[2];
					*cp = '\0';
				}
				else
				{
					for (cp = buf; *cp != ':'; cp++)
						;
					*cp = '\0';
				}
				bmove(rel.relowner, lastuser, 2);
			}
			printf(" %.12s      %s\n", rel.relid, buf);
		}
		else
		{
			if ((rel.relstat & S_CATALOG) || bequal("_SYS", rel.relid, 4))
				continue;
			if (bequal(Usercode, rel.relowner, UCODE_SZ) || bequal(Admin.adhdr.adowner, rel.relowner, 2))
				rel_fmt(&rel);
		}
	}

	if (i < 0)
		syserr("help: relpr: get %d", i);
	if (mode == RELIST)
		printf("\n");
	return (0);
}



/*
**  Print Relation Information
**
**	Prints detailed information regarding the relation.
**
**	Uses trace flag 13
*/

rel_fmt(r)
register struct relation	*r;
{
	struct tup_id		limtid, tid;
	char			buf[MAXLINE + 1];
	struct attribute	att;
	struct index		indkey, ind;
	register int		i;
	int			j;
	extern DESC		Attdes, Inddes;
	char			*trim_relname();

	printf("\nRelation:\t\t%s\n", trim_relname(r->relid));
	i = getuser(r->relowner, buf);
	if (i)
	{
		smove("(xx)", buf);
		bmove(r->relowner, &buf[1], 2);
	}
	else
	{
		for (i = 0; buf[i] != ':'; i++)
			continue;
		buf[i] = 0;
	}
	printf("Owner:\t\t\t%s\n", buf);
	printf("Tuple width:\t\t%d\n", r->relwid);
	if (r->relsave != 0)
	{
		printf("Saved until:\t\t%s", ctime(&r->relsave));
	}
	if ((r->relstat & S_VIEW) == 0)
	{
		printf("Number of tuples:\t%ld\n", r->reltups);
		printf("Storage structure:\t");
		i = r->relspec;
		if (i < 0)
		{
			printf("compressed ");
			i = -i;
		}
		switch (i)
		{
	
		  case M_HEAP:
			printf("paged heap\n");
			break;
	
		  case M_ISAM:
			printf("ISAM file\n");
			break;
	
		  case M_HASH:
			printf("random hash\n");
			break;
	
		  default:
			printf("unknown structure %d\n", i);
			break;
	
		}
	}

	printf("Relation type:\t\t");
	if (r->relstat & S_CATALOG)
		printf("system catalog\n");
	else if (r->relstat & S_VIEW)
		printf("view\n");
	else
		if (r->reldim > 0)
			printf("ordered relation\n");
		else if (r->relindxd < 0)
		{
			printf("secondary index on ");
			opencatalog("indexes", OR_READ);
			setkey(&Inddes, &indkey, r->relowner, IOWNERP);
			setkey(&Inddes, &indkey, r->relid, IRELIDI);
			if (!getequal(&Inddes, &indkey, &ind, &tid))
				printf("%s\n", trim_relname(ind.irelidp));
			else
				printf("unknown relation\n");
		}
		else
		{
			if (r->relstat & S_DISTRIBUTED)
				printf("distributed ");
			printf("user relation\n");
		}
	if (r->relindxd > 0)
	{
		printf("Secondary Indices:\t");
		opencatalog("indexes", OR_READ);
		setkey(&Inddes, &indkey, r->relid, IRELIDP);
		setkey(&Inddes, &indkey, r->relowner, IOWNERP);
		if (i = find(&Inddes, EXACTKEY, &tid, &limtid, &indkey))
			syserr("help: find %d indexes", i);
		j = FALSE;
		while ((i = get(&Inddes, &tid, &limtid, &ind, 1)) == 0)
		{
			if (!bequal(&indkey, &ind, MAXNAME + 2))
				continue;
			if (j)
				printf(", ");
			j =TRUE;
			printf("%s", trim_relname(ind.irelidi));
		}
		if (i < 0)
			syserr("help:get indexes %d", i);
		if (!j)
			printf("unknown");
	}
	printf("\n");

	opencatalog("attribute", OR_READ);
	printf("\n attribute name    type  length  keyno.\n\n");
	seq_init(&Attdes, r);
	while (seq_attributes(&Attdes, r, &att))
	{
		printf(" %.12s	    %c%8d",
			att.attname, att.attfrmt, att.attfrml & I1MASK);
		if (att.attxtra)
			printf("%7d", att.attxtra);
		printf("\n");
	}

	printf("\n");
	return (0);
}

/*
**	HELPDELIM - print all delims presently defined.
**
**		Parameters:
**			group - the group of delims to print
**
**		Returns:
**			0 - if successful
**			-1 - if relation not found
*/
helpdelim(group,des)
char	*group;
DESC	*des;
{
	DELIM_TUP	tuple;
	TID		lotid,hitid;
	int		notfound = 1;
	char		delim[12];
	int		start = 0;
	int		begin = 1;
	char		dstring[1024];
	char		*dptr;
	int		found=0;

	if (find(des,LRANGEKEY, &lotid, &hitid, group) < 0)
		return(-1);
	find(des,HRANGEKEY, &lotid, &hitid, group);
	printf("\n \t>>>>  %s  <<<<\n", group);

	while (!get(des, &lotid, &hitid, &tuple, 1))
	{

		if (strcmp(tuple.group, group))
			continue;

		if (strcmp(tuple.delim, delim))
			start = 1;

		if (start)
		{
			found = 1;
			if (begin)
				begin = 0;
			else
				print_delim(dstring);

			/*start a new string*/
			strcpy(delim, tuple.delim);
			start = 0;
			*dstring = NULL;
			strcat(dstring, tuple.delim);
			strcat(dstring, ":		");
			convert_bitmap(dstring, &tuple);
		}
		else
		{
			/*add to old string*/
			convert_bitmap(dstring, &tuple);
		}
	}
	if (!found)
		printf("group %s does not exist\n", group);
	else
		print_delim(dstring);
	return(0);
}

/*
**	CONVERT_BITMAP - convert a bitmap back to a BNF expression
**
**		Parameters:
**			dstring - string to stuff the BNF expression info
**			tuple - tuple containing the bitmap
**
*/
convert_bitmap(dstring, tuple)
char		*dstring;
DELIM_TUP	*tuple;
{
	int	i,j;
	char	*pntr;

	pntr = dstring + strlen(dstring);
	*pntr++ = (tuple->type == ONE ? LBRACKET : LBRACE);
	i = 0;
	while (i < 128 )
	{
		if (test(tuple->bitmap, i))
		{
			*pntr++ = i;
			j = ++i;
			while ((j < 128) && test(tuple->bitmap, j))
				j++;
			if ( (j - i) >= 5)
			{
				j--;
				*pntr++ = '-';
				*pntr++ = j;
				i = j + 1;
			}
		}
		else
			i++;
	}
	*pntr++ = (tuple->type == ONE ? RBRACKET : RBRACE);
	*pntr = NULL;
}

/*
**	PRINT_DELIM - print one delim of the rdelim relation
**
**		Parameters:
**
*/
print_delim(delim)
char	*delim;
{
	printf("%s\n", delim);
}

/*
**	ALLDELIMS - print all the delims currently defined
**
*/
alldelims(des)
DESC	*des;
{
	DELIM_TUP	tuple;
	int		start=1;
	char		group[12];
	TID		lotid;
	TID		hitid;

	printf("Delimitor groups:\n");
	if (find(des,LRANGEKEY, &lotid, &hitid, group) < 0)
		return(-1);
	find(des,HRANGEKEY, &lotid, &hitid, group);

	while (!get(des, &lotid, &hitid, &tuple, 1))
	{

		if (strcmp(tuple.group, group))
			start = 1;

		if (start)
		{
			strcpy(group, tuple.group);
			printf("\t\t %s\n", group);
			/*helpdelim(group, des); */
			start = 0;
		}
	}
}
