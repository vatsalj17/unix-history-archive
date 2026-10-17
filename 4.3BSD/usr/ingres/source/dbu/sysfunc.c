# include	<ingres.h>
# include	<aux.h>
# include	<pv.h>
# include	<sccs.h>
# include 	<symbol.h>
# include 	<func.h>


SCCSID (@(#)sysfunc.c	8.2 	1/15/85)

short		tTdbu[100];
extern int	sysfunc();
extern int 	null_fn();

int		Noupdt = FALSE;

struct  fn_def	SysFuncFn = 
{	
	"SYSMOD",
	sysfunc,
	null_fn,
	null_fn,
	NULL,
	0,
	tTdbu,
	100,
	'S',
	0
};


extern int	Status;

struct modtabl
{
	char	*rname;
	char	**parvec;
	int	goahead;
	int	normgo;
	int	optn;
};

char	*Relpar[] =
{
	"relation",	"hash",		"name",
	"relid",	NULL
};

char	*Attpar[] =
{
	"attribute",	"hash",		"name",
	"attrelid",	"attowner",	"#attid",
	NULL
};

char	*Indpar[] =
{
	"indexes",	"hash",		"name",
	"irelidp",	"iownerp",	"",
	"minpages",	"5",		NULL
};

char	*Trepar[] =
{
	"tree",		"hash",		"name",
	"treerelid",	"treeowner",	"treetype",
	NULL
};

char	*Propar[] =
{
	"protect",	"hash",		"name",
	"prorelid",	"prorelown",	NULL
};

char	*Intpar[] =
{
	"integrities",	"hash",		"name",
	"intrelid",	"intrelowner",	NULL
};

char	*Rdelpar[] =
{
	"rdelim",	"isam",		"name",
	"order",	"group",	"delim",
	NULL
};

struct modtabl  Modtabl[] =
{
	"relation",	&Relpar[0],	FALSE,	TRUE,	FALSE,
	"attribute",	&Attpar[0],	FALSE,	TRUE,	FALSE,
	"indexes",	&Indpar[0],	FALSE,	TRUE,	FALSE,
	"tree",		&Trepar[0],	FALSE,	TRUE,	TRUE,
	"protect",	&Propar[0],	FALSE,	TRUE,	TRUE,
	"integrities",	&Intpar[0],	FALSE,	TRUE,	TRUE,
	"rdelim",	&Rdelpar[0],	FALSE,	TRUE,	TRUE,
	0
};


/*
**	SYSMOD -- Modify system catalogs to a predetermined
**		storage structure with predetermined keys.
**
**		Flags:
**			'-Rsystem relation name' will modify
**			only the named relations
**
**		Trace flags:
**			-S99
**
*/

sysfunc (pc, pv)
int 	pc;
PARM 	*pv;
{
	register int	i;
	register int	j;
	register char	**av;
	char		*p;
	char		*argptr;


# ifdef xSTR1
	if (tTf(99,0))
	{
		printf("SYSFUNC: starting. \n");
		prvect(pc, pv);
	}
# endif

/*
**	if there are any arguments, verify that they are valid
**	names of relations which can be modified by this program.
**	if there are no arguments, assume all system relations are to be
**	modified.
*/

	if (pv[0].pv_val.pv_str != NULL)
		if ((pv[1].pv_val.pv_str == NULL) && sequal(argptr = &pv[0].pv_val.pv_str[2], "all"))
			for (i = 0; Modtabl[i].rname; i++)
				Modtabl[i].goahead = TRUE;
		else
			for ( i = 0; i <= pc; i++)
			{
				av = &pv[i].pv_val.pv_str;
				if (( p = *av) == NULL)
					break;
				
				argptr = &p[2];
				for (j = 0; Modtabl[j].rname; j++)
				{
					if (sequal(argptr, Modtabl[j].rname))
					{
						if (Modtabl[j].goahead)
						{
							printf("%s duplicate relation name\n", p);
							return(1);
						}
						Modtabl[j].goahead = TRUE;
						break;
					}
				}
				if ((!Modtabl[j].rname) && !(p[1] == 'Z'))
				{
					printf("%s is not a system relation\n", p);
					return(1);
				}
			}
	else
		for (i = 0; Modtabl[i].rname; i++)
			Modtabl[i].goahead = Modtabl[i].normgo;

	for (i = 0; Modtabl[i].rname; i++)
	{
		if (Modtabl[i].goahead == 0 || optn_rel(&Modtabl[i]))
			continue;
		argptr = Modtabl[i].rname;
		printf("modifying %s\n", argptr); 
		av = Modtabl[i].parvec;
		j = 0;
		initp();
		while (*av != NULL)
		{
			setp(PV_STR, *av);
			av++;
		}

		call( mdMODIFY, NULL);
	}
	printf("sysmod done\n");
	return(0);
}


optn_rel(mx)
struct modtabl	*mx;
{
	register struct modtabl	*m;
	register int		ret;
	char			*temp;

	struct descriptor	des;

	m = mx;
	ret = FALSE;

	if (m->optn)
	{
		if (openr(&des, OR_RELTID, m->rname))
		{
			ret = TRUE;
		}
	}
	return (ret);
}

/* rubproc()
{
	printf("sysmod interrupted\n");
	return(1);
} */
