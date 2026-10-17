# define	MAXNAME	13	/* one more for the null */
# define	MAXKEYS	6	/* changing this is insuficient */
# define	MAXINDEX	10	/* change as needed */
# define	MAXREL	1000	/* can be increased as needed */
# define	MAXDOM	50
# define	INT	30
# define	FLOAT	31
# define	CHAR	32
## int	User;
struct iobuf
{
	int	fdes;
	int	nleft;
	char	*next;
	char	buf[512];
}	Ifile, Ofile;
struct relation
{
	char	name[MAXNAME];
	char	indx;
	int	spec;
	int	atts;
};

struct attribute
{
	char	aname[MAXNAME];
	char	format;
	char	length;
};

struct index
{
	char	iname[MAXNAME];
	char	ispec;
	char	idom[MAXKEYS];
};


main(argc,argv)
int	argc;
char	**argv;
{

##	char	*db;
##	char 	*ind;
	char	*path;
##	char	name[MAXNAME];
##	int	indx;
##	int	atts;
##	int	spec;
##	char	iname[MAXNAME];
##	int	idomv1, idomv2, idomv3, idomv4, idomv5, idomv6;


	int	n;

	register struct relation	*relptr;
	register struct attribute	*attptr;
	register int			i;
	struct relation			relations[MAXREL];
	struct attribute		attributes[MAXDOM], indatts[MAXKEYS];
	struct index			indexes[MAXINDEX];
	struct index			*indxptr;
	int				attxtra[MAXDOM], indattx[MAXKEYS];
	int				xcount;
	int				j;
	extern				*Exitfn, exitfn;

	if (argc != 3)
		syserr(0,"wrong number of parameters");

	db = argv[1];
	path = argv[2];
	if (path[0] != '/')
		printf("Warning %s is not a full path name!\n",path);


	if (fcreat(ztack(path,"/copy.in"),&Ifile)<0)
		syserr(0,"Cannot creat %s!",ztack(path,"/copy.in"));
	if (fcreat(ztack(path,"/copy.out"),&Ofile)<0)
		syserr(0,"Cannot create %s!",ztack(path,"/copy.out"));

	Exitfn = &exitfn;
	User = getuid() & 0377;
	User =| (getgid() & 0377) << 8;



##	ingres	db


	/* If this is not the dba and the User has created a
	**	new relation relation (or any system relation)
	**	things wount work
	*/

##	range of r is relation
	i = 0;

	/*
	**	Bring the relations into core
	*/
	relptr = relations;
##	retrieve (name = r.relid, spec = r.relspec, indx = r.relindxd,
##		atts = r.relatts)
##		where r.relowner = User
##		and r.relindxd >= 0
##		and r.relid != "relation"	and r.relid != "attribute"
##		and r.relid != "index"		and r.relid != "integrity"
##		and r.relid != "constraint"	and r.relid != "dsptemp"
##		and r.relid != "maprelation"	and r.relid != "statistics"
##		and r.relid != "tupcount"	and r.relid != "lock"
##	{
		if (bequal("_SYS",name,4)) 	/* don't copy temporary relations */
			continue;
		if(i++>=MAXREL)
			syserr("Too many relations!");
		smove(name,&(relptr->name));
		relptr->spec = spec;
		relptr->indx = indx;
		relptr++->atts = atts;
##	}


	/*
	**	For each relation bring its attributes into core
	**	in domain order
	*/
##	range of a is attribute
	ind = "index";
##	range of i is ind
	n = i;
	printf("%d relations found\n",n);
	for (relptr = relations; relptr < &relations[n]; relptr++)
	{
		writein("create ");
		writeout("copy ");
		smove(relptr->name,name);
		writeboth(name);
		writeboth("(\n\t");

		xcount = retatts(name,attributes,attxtra);
		for(i = 0; i < relptr->atts; i++)
		{
			attptr = &attributes[i];
			if(i)
				writeboth(",\n\t");
			writeboth(attptr->aname);
			putboth('=');
			putboth(attptr->format);
			writeboth(iocv(attptr->length));
		}

		writeboth("\n)");
		writeout(" into \"");
		writein("\n\\p\n\\g\n\\r\ncopy ");
		writein(name);
		writein("() from \"");
		writeboth(path);
		putboth('/');
		writeboth(strip(name));
		writeboth("\"\n\\p\n\\g\n\\r\n");


		if (relptr->spec != 5 && relptr->spec != 0)
			modify(name,attributes,attxtra,relptr->spec,xcount);
		if (relptr->indx)
		{
			indxptr = indexes;
			i = 0;

##			retrieve (iname = i.irelidi, spec = i.irelspeci,
##				idomv1 = i.idom1, idomv2 = i.idom2, idomv3 = i.idom3,
##				idomv4 = i.idom4, idomv5 = i.idom5, idomv6 = i.idom6)
##			where i.irelidp = name and i.iownerp = User
##			{
				if (i++ >= MAXREL)
					syserr("Too many indexes on %s!!",name);

				smove(iname,indxptr->iname);
				indxptr->ispec = spec;
				indxptr->idom[0] = idomv1;
				indxptr->idom[1] = idomv2;
				indxptr->idom[2] = idomv3;
				indxptr->idom[3] = idomv4;
				indxptr->idom[4] = idomv5;
				indxptr->idom[5] = idomv6;
				indxptr++;
##			}

			while (--indxptr >= indexes)
			{
	
				writein("\n");
				writein("index on ");
				writein(name);
				writein(" is ");
				writein(indxptr->iname);
				writein("(\n\t");
	
				for (i=0; indxptr->idom[i] && i < MAXKEYS; i++)
				{
					if(i)
						writein(",\n\t");
					writein(attributes[indxptr->idom[i]-1].aname);
				}
				writein("\n)\n\\p\n\\g\n\\r\n");
				if(indxptr->ispec != 10 && indxptr->ispec != 11)
				{
					xcount = retatts(indxptr->iname,indatts,indattx);
					modify(indxptr->iname,indatts,indattx,indxptr->ispec,xcount);
				}
			}
		}
		putboth('\n');
	}
	writein("/* These modifies will cause errors if you are not the DBA */\n");
	writein("modify relation to hash on relid\n\\p\n\\g\n\\r\n");
	writein("modify attribute to hash on attrelid\n\\p\n\\g\n\\r\n");
	writein("modify indexes to hash on irelidp, iownerp\n\\p\n\\g\n\\r\n");
	fflush(&Ifile);
	fflush(&Ofile);
##	exit
	printf("All done\n");
}
exitfn()
{
	fflush(&Ifile);
	fflush(&Ofile);
	exit(-1);
}


modify(name, attributes, attxtra, spec, xcount)
char	*name;
struct attribute	*attributes;
int	*attxtra;
int	spec;
int	xcount;
{
	register	i,j;
	writein("modify ");
	writein(name);
	writein(" to ");
	switch(spec)
	{

	  case -5:
		writein("cheap on ");
		break;

	  case -10:
	  case -11:
		writein("c");
	  case 10:
	  case 11:
		writein("isam on ");
		break;

	  case -20:
	  case -21:
		writein("c");
	  case 21:
	  case 20:
		writein("hash on ");
		break;

	  default:
		syserr("Unimplimented spec: %d on %s",
			spec,name);

	}
	j = 0;
	writein("\n\t");
	for (i = 1; i <= xcount; i++)
	{
		if(j++)
			writein(",\n\t");
		writein(attributes[attxtra[i]].aname);
	}
	if (!j)
		syserr("No keys found for %s!!!",name);
	writein("\\p\n\\g\n\\r\n");
}
retatts(name,attributes,attxtra)
char	*name;
struct attribute	*attributes;
int	attxtra[];
{
	register	xcount, i;
	register struct attribute	*attptr;

##	char	aname[MAXNAME];
##	int	domain, length, extra;
##	char	format[2];
	i = xcount = 0;
##	retrieve (aname = a.attname,domain = a.attid, format = a.attfrmt,
##		 length = a.attfrml, extra = a.attxtra)
##
##	where a.attrelid = name and a.attowner = User
##	{
		if (i++ >= MAXDOM)
			syserr("too many attributes in %s!",name);
		attptr = &attributes[domain-1];
		smove(aname,attptr->aname);
		attptr->format = format[0];
		attptr->length = length;
		if(extra)
		{
			attxtra[extra] = domain - 1;
			xcount++;
		}
##	}
	return(xcount);
}

char	*strip(string)
char	*string;
{

	register char	*sp;

	sp = string;

	while (*sp++);
	--sp;
	while (*--sp == ' ')
		if (sp == string)
			break;

	*++sp = '\0';
	return (string);
}


writeboth(string)
char	*string;
{
	register char	*sp;

	sp = string;

	while(*sp)
		putc(putc(*sp++,&Ifile),&Ofile);
}

putboth(ch)
char	ch;
{
	putc(putc(ch,&Ifile),&Ofile);
}


writeout(string)
char	*string;
{
	register char 	*sp;

	sp = string;
	while (*sp)
		putc(*sp++,&Ofile);
}


writein(string)
char	*string;
{
	register char	*sp;

	sp = string;
	while (*sp)
		putc(*sp++,&Ifile);
}



