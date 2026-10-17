# include	"../ingres.h"
# include	"../access.h"

main(argc, argv)
int	argc;
char	*argv[];
{
	struct admin		ad;
	register int		i, fp;
	register char		*db;
	extern char		*Dbpath;
	extern char		*Parmvect[], *Flagvect[];

#	ifdef xSTR1
	tTrace(&argc, argv, 'T');
#	endif

	i = initucode(argc, argv, TRUE, NULL, -1);
	db = Parmvect[0];
	switch (i)
	{
	  case 0:
	  case 5:
		break;

	  case 1:
	  case 6:
		printf("Database %s does not exist\n", db);
		exit(-1);

	  case 2:
		printf("You are not authorized to access this database\n");
		exit(-1);

	  case 3:
		printf("You are not a valid INGRES user\n");
		exit(-1);

	  case 4:
		printf("No database name specified\n");
	usage:
		printf("usage: printadmin database\n");
		exit(-1);
	  default:
		syserr("initucode %d", i);
	}

	if (Flagvect[0] != NULL)
	{
		printf("No flags are allowed for this command\n");
		goto usage;
	}

	if (Parmvect[1] != NULL)
		goto usage;

	if (chdir(Dbpath) < 0)
		syserr("cannot access database %s", db);
#	ifdef xTTR2
	if (tTf(1, 0))
		printf("entered database %s\n", Dbpath);
#	endif



	if ((fp = open("admin", 0)) < 0)
		syserr(0, "cannot open admin file on %s", Dbpath);
	if (read(fp, &ad, sizeof ad) != sizeof ad)
		syserr("can't read admin struct");

	printf("Database %s, Dba %.2s, Adflags %o\n",
		db, ad.adhdr.adowner, ad.adhdr.adflags);

	printdesc(&ad.adreld);

	printf("\n\n");
	printdesc(&ad.adattd);
}
rubproc()
{
	exit(1);
}
