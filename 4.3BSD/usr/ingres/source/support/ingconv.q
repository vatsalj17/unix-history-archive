# include	<ingres.h>
# include	<access.h>
# include	<sys/file.h>
# include	<pwd.h>
# include	<errno.h>
# include	<signal.h>
# include	<sccs.h>

SCCSID(@(#)ingconv.q	8.5 1/16/85)

/*
**  INGRES.H -- basic header file for ingres.
**
**	See also aux.h for definitions used by some but not all.
**
**	Version:
**		@(#)ingres.h	7.1	2/5/81
*/


/*
**	RELATION relation struct
**
**	The RELATION relation contains one tuple for each relation
**	in the database.  This relation contains information which
**	describes how each relation is actually stored in the
**	database, who the owner is, information about its size,
**	assorted operation information, etc.
*/

struct orelation
{
	c_12	relid[MAXNAME];	/* relation name	*/
	c_2	relowner[2];	/* code of relation owner */
	i_1	relspec;	/* storage mode of relation	*/
				/* M_HEAP  unsorted paged heap	*/
				/* -M_HEAP compressed heap	*/
				/* M_ISAM  isam			*/
				/* -M_ISAM compressed isam	*/
				/* M_HASH  hashed		*/
				/* -M_HASH compressed hash	*/
	i_1	relindxd;	/* -1 rel is an index, 0 not indexed, 1 indexed */
	i_2	relstat2;	/* more status bits */
	i_2	relstat;	/* relation status bits */
	i_4	relsave;	/*unix time until which relation is saved*/
	i_4	reltups;	/*number of tuples in relation	*/
	i_2	relatts;	/*number of attributes in relation	*/
	i_2	relwid;		/*width (in bytes) of relation	*/
	i_4	relprim;	/*no. of primary pages in relation*/
	i_4	relfree;	/* head of freelist (b-trees only) */
	i_4	relstamp;	/*time of last mod*/
};


/*
**  Definitions for the range table.
**
**	Version:
**		@(#)range.h	7.1	2/5/81
*/



/*
**	DESCRIPTOR struct
**
**	The DESCRIPTOR struct is initialized by OPENR to describe any
**	open relation.  The first part of the descriptor is the tuple
**	from the RELATION relation.  The remainder contains some magic
**	numbers and a template initialized from the ATTRIBUTE relation.
**
**	This structure also defines the range table.
*/

struct odescriptor
{
	struct orelation	reldum;
		/*the above part of the descriptor struct is identical
		  to the relation struct and the inormation in this
		  part of the struct is read directly from the
		  relation tuple by openr.  the rest of the descriptor
		  struct is calculated by openr.  */
	char	relvname[MAXNAME];	/* range variable name */
	i_2	relfp;		/*filep for relation , if open	*/
	i_2	relopn;		/*indicates if relation is really open*/
	tid_type reltid;	/*when relation is open, this indicates
				  the tid in the relation relation for
				  this relation */
	i_4	reladds;	/*no. of additions of tuples during this open*/
	i_2	reloff[MAXDOM];	/*reloff[i] is offset to domain i 	*/
	c_1	relfrmt[MAXDOM]; /* format of domain i
				 ** INT, FLOAT, or CHAR  */
	c_1	relfrml[MAXDOM]; /* relfrml[i] is an unsigned integer
				  which indicates length
				  in bytes of domain */
	c_1	relxtra[MAXDOM]; /*relxtra[i] is non-zero if domain i is
				 ** a key domain for the relation */
	c_1	relgiven[MAXDOM]; /*cleared by openr and set before
				  call to find to indicate value of this
				  domain has been supplied in the key*/
};

#
/*
**  ACCESS.H -- definitions relating to the access methods.
**
**	Version:
**		@(#)access.h	7.1	2/5/81
*/


/*
**  ADMIN file struct
**
**	The ADMIN struct describes the initial part of the ADMIN file
**	which exists in each database.  This file is used to initially
**	create the database, to maintain some information about the
**	database, and to access the RELATION and ATTRIBUTE relations
**	on OPENR calls.
*/


struct oadmin
{
	struct adminhdr		adhdr;
	struct odescriptor	adreld;
	struct odescriptor	adattd;
};
/*
**  VERSION.H -- system version definition file.
**
**	NOTICE:
**		Version numbers stored in files are SCCS id's
**		and may not correspond to the external distribution
**		version number.  The distribution number applies to
**		the entire system and not to any particular file.
**		This file defines a "release" number, used for
**		creating file names.  The entire system version
**		number (including mod number) is defined by
**		conf/version.c.
**
**	Version:
**		@(#)version.h	8.1	12/31/84
*/


/*
**	VERSION is the version number of this incarnation of INGRES
**		for purposes of creating file names.
**	DBVERCODE is the code for this database version stored in
**		the admin file.
**	PATHEXT is an extension for the path as derived from the
**		"ingres" entry in the password file to determine
**		the "Pathname" variable.  If it is not defined,
**		no extension is made.
*/

# define	VERSION		"8"		/* version number */
# define	DBVERCODE	1		/* database version code */
/* # define	PATHEXT		"/x"		/* the root path extension */
extern	struct	oadmin	OAdmin;
struct	admin		Admin;
char			Ingcode[3];	/* Ingres code of ingres */
char			User[3];	/* Ingres code for DBA of this database */


/*
** Ingconv
**	Convert version 7 databases into version 8 databases.
**	We assume that there is a $P/bin7 to fake everything with.
*/
main(argc,argv)
int	argc;
char	**argv;
{

	struct	passwd	*pwd;
	int		fd;
	int		i;
	int		oldsigint;
	int		oldsigquit;
	char		relname[15];
	char		attname[15];
	char		orelname[15];
	char		oattname[15];
	char		buffer[1000];
	char		*path;
	int		sflag = 0;
	extern	int	errno;
	extern	int	goodbye();
	extern	int	(*ExitFn)();
	extern	char	*getenv();

	argc--;
	argv++;

	ExitFn = goodbye;

	if ( argc != 1 )
	{
		if ( argc == 2 && strcmp(*argv,"-s") == 0 )
		{
				sflag = 1;
				argc--;
				argv++;
		}
		else
			syserr("ingconv: wrong number of arguments, useage 'ingconv [-s] database'");
	}

	if ( (pwd = getpwnam("ingres")) == 0 )
		syserr("ingconv: can't find ingres in password file");

	/*
	** Wander into ~ingres/bin7, and then make sure
	** that the Ingres we get is the one here. This will
	** be a version 7 ingres.
	*/
	if ( chdir(pwd->pw_dir) == -1 )
		syserr("can't chdir to %s",pwd->pw_dir);
	if ( chdir("bin7") == -1 )
		syserr("can't chdir to bin7");
	if ( (path = getenv("PATH")) == 0 )
		syserr("No PATH environment");
	strcpy(path,":.:");

	printf("converting database '%s'\n",*argv);
	/*
	** Make new versions of the relation relation, and
	** attribute relations. Store these new things as
	** "_rtempv8" and "_atempv8".
	*/
	changerels(*argv);


	/*
	** Now we go into the data/base/database, and do some magic
	** disgusting stuff.
	*/
	if ( chdir(pwd->pw_dir) == -1 )
		syserr("can't chdir to %s",pwd->pw_dir);
	if ( chdir("data/base") == -1 )
		syserr("can't chdir to data/base");

	errno = 0;
	if ( chdir(*argv) == -1 )
	{
		if ( errno == ENOTDIR )
			syserr("can't handle indirect files, change it into a symbolic link first");
		syserr("no database '%s'",*argv);
	}

	bzero(&Admin, sizeof (Admin));
	bzero(&OAdmin, sizeof (OAdmin));

	if ( (fd = open("admin",O_RDONLY)) == -1 )
		syserr("can't open admin file");

	/*
	** Read in the old admin relation, then write out a new
	** version into _admin.
	*/
	startadmin(fd);
	close(fd);
	Admin.adhdr = OAdmin.adhdr;
	descasign(&OAdmin.adreld, &Admin.adreld);
	descasign(&OAdmin.adattd, &Admin.adattd);
        Admin.adhdr.adreldsz = Admin.adhdr.adattdsz = sizeof Admin.adreld;

	if ( (fd = open("_admin",O_WRONLY|O_CREAT,0600)) == -1 )
		syserr("can't create '_admin' temp file");
	if ( write(fd,&Admin, sizeof (Admin)) != sizeof (Admin) )
		syserr("can't write _admin file");
	close(fd);



	strcpy(orelname,"relation    ");
	strcat(orelname,User);
	strcpy(relname,"_rtempv8    ");
	strcat(relname,Ingcode);

	strcpy(oattname,"attribute   ");
	strcat(oattname,User);
	strcpy(attname,"_atempv8    ");
	strcat(attname,Ingcode);
	/*
	** Up to now, we can be interrupted, now we come to the
	** magic stuff that might leave us undefended...
	*/
	oldsigint = (int) signal(SIGINT,SIG_IGN);
	oldsigquit = (int) signal(SIGQUIT,SIG_IGN);
	for ( i = 1 ; i < NSIG ; i++ )
		signal(i,SIG_IGN);

	if ( rename(attname,oattname) == -1 )
		syserr("Could not rename '%s' to '%s'",attname, oattname);
	if ( rename(relname,orelname) == -1 )
		syserr("Could not rename '%s' to '%s', database is now corrupted!!!!, restore the attribute relation from tape.",relname, orelname);
	if ( rename("_admin","admin") == -1 )
		syserr("Could not rename '_admin' to 'admin'. Database is now corrupted, read the attribute and relation relations in from tape.");

	/*
	** Turn the interrupts back on, now that the
	** critical section is over.
	*/
	for ( i = 1 ; i < NSIG ; i++ )
		signal(i,SIG_DFL);
	signal(SIGINT,oldsigint);
	signal(SIGQUIT,oldsigquit);
	if ( sflag )
		sprintf(buffer,"%s/bin/sysmod -s %s",pwd->pw_dir,*argv);
	else
		sprintf(buffer,"%s/bin/sysmod %s",pwd->pw_dir,*argv);
	printf("%s\n",buffer);
	system(buffer);
	printf("Database '%s' is now a version 8 database.\n",*argv);
}

/*
** Assign an old descriptor to a new descriptor.
*/
descasign(a,b)
struct	odescriptor	*a;
struct	descriptor	*b;
{
	struct	orelation	*orel;
	struct	relation	*rel;

	strcpy(b->relvname,a->relvname);
	b->relfp = a->relfp;
	b->relopn = a->relopn;
	b->reltid = a->reltid;
	b->reladds = a->reladds;
	bcopy(a->reloff,b->reloff,(sizeof (i_2)) * MAXDOM);
	bcopy(a->relfrmt,b->relfrmt,(sizeof (c_1)) * MAXDOM);
	bcopy(a->relfrml,b->relfrml,(sizeof (c_1)) * MAXDOM);
	bcopy(a->relxtra,b->relxtra,(sizeof (c_1)) * MAXDOM);
	bcopy(a->relgiven,b->relgiven,(sizeof (c_1)) * MAXDOM);

	orel = &a->reldum;
	rel = &b->reldum;

	bcopy(orel->relid,rel->relid,MAXNAME);
	bcopy(orel->relowner,rel->relowner,2);
	rel->relspec = orel->relspec;
	rel->relindxd = orel->relindxd;
	rel->relstat2 = orel->relstat2;
	rel->relstat = orel->relstat;
	rel->relsave = orel->relsave;
	rel->reltups = orel->reltups;
	rel->relatts = orel->relatts;
	rel->relwid = orel->relwid;
	rel->relprim = orel->relprim;
	rel->relfree = orel->relfree;
	rel->relstamp = orel->relstamp;
}

goodbye()
{
	exit(1);
}



struct	oadmin	OAdmin;

/*
**  STARTADMIN -- starts admin file version, etc.
**
**	The checks for database version code and whatnot are
**	factored out into this routine.  When this routine returns,
**	the admin file should be legible to this program.
**	If the admin file is not legible, it will syserr.
**
**	Parameters:
**		fd -- open file descriptor for admin file.  Only
**			read access is required.
**
**	Returns:
**		nothing if ok.
**		not at all (or via syserr) if not ok.
**
**	Side Effects:
**		The OAdmin.adhdr struct will be filled in.
*/

startadmin(fd)
register int	fd;
{
	register int	i;
	register int	k;

	i = ((char *) &OAdmin.adhdr.adversion) - ((char *) &OAdmin.adhdr);
	if (read(fd, (char *) &OAdmin.adhdr, i) != i)
		syserr("readadmin: admin read err 1");
	if (!bitset(A_NEWFMT, OAdmin.adhdr.adflags))
		syserr("readadmin: cannot use old databases");

	/* read in remainder of admin header */
	i = sizeof OAdmin.adhdr;
	if (OAdmin.adhdr.adlength < i)
		i = OAdmin.adhdr.adlength;
	i -= ((char *) &OAdmin.adhdr.adversion) - ((char *) &OAdmin.adhdr);
	if (i <= 0)
		syserr("readadmin: adlen=%d, hdrsz=%d, ct=%d", OAdmin.adhdr.adlength, sizeof OAdmin.adhdr, i);
	if ((k = read(fd, (char *) &OAdmin.adhdr.adversion, i)) != i)
		syserr("readadmin: admin read err 2, i=%d k=%d", i, k);

	/* check versions here */
	if (OAdmin.adhdr.adversion != DBVERCODE)
		syserr("cannot handle code %d databases (current code is %d)",
			OAdmin.adhdr.adversion, DBVERCODE);
	if (OAdmin.adhdr.adreldsz != sizeof OAdmin.adreld)
		syserr("checkadmin: descriptor size mismatch, dec=%d, actual=%d",
			OAdmin.adhdr.adreldsz, sizeof OAdmin.adreld);

	/* get to beginning of descriptors */
	if (lseek(fd, (long) OAdmin.adhdr.adlength, 0) < 0)
		syserr("checkadmin: seek");

	/* read the descriptors */
	if (read(fd, (char *) &OAdmin.adreld, OAdmin.adhdr.adreldsz) != OAdmin.adhdr.adreldsz)
		syserr("checkadmin: reld read sz=%d", OAdmin.adhdr.adreldsz);
	if (read(fd, (char *) &OAdmin.adattd, OAdmin.adhdr.adattdsz) != OAdmin.adhdr.adattdsz)
		syserr("checkadmin: attd read sz=%d", OAdmin.adhdr.adattdsz);
}

changerels(database)
##	char	*database;
{
##	char	*user;
##	int	sysmod;
##	char	*delname;
	char	data[3];
	extern	char	Ingcode[];
	extern	char	User[];

	user = data;
	user[0] = user[1] = user[2] = '\0';
	sysmod = -1;
	delname = "delim";


##	ingres database "-rheap"
##	range of r is relation
##	range of a is attribute

##	retrieve ( sysmod = r.relspec ) where r.relid = "_rtempv8"

	if ( sysmod != -1 )
		syserr("The data base '%s' has a the relation '_rtempv8'. Please remove, or change this relation before using this program");

##	retrieve ( sysmod = r.relspec ) where r.relid = "_rtempv8"

	if ( sysmod != -1 )
		syserr("The data base '%s' has a the relation '_atempv8'. Please remove, or change this name before using this program");

##	create _rtempv8(relid=c12, relowner=c2, relspec=i1, relindxd=i1, 
##			relstat2=i2, relstat=i2, relsave=i4, reltups=i4, 
##			relatts=i2, relwid=i2, relprim=i4, relfree=i4, 
##			relstamp=i4, reldim=i2)


##	retrieve ( user = r.relowner ) where r.relid = "relation"
	User[0] = user[0];
	User[1] = user[1];
	User[2] = '\0';

##	create rdelim (order=i4, group=c12, delname=c12, type=i4, bitmap=c16)
##	modify rdelim to isam on order,group,delname

##	retrieve into _rtempv81 (r.all, reldim = 0)
##	range of t is _rtempv81
##	append _rtempv8 (t.all)
##	destroy _rtempv81
##	range of t is _rtempv8
##	delete t where t.relid = "_rtempv8" or t.relid = "_rtempv81"
##	replace t ( relsave = 0, relstat = 275, relspec = 11) where t.relid = "rdelim"
##	replace t ( reltups = t.reltups -1, relatts = t.relatts + 1, relwid = t.relwid + 2) where
##		t.relid = "relation"


##	retrieve into _atempv8 (a.all)
##	append _atempv8(attrelid="relation",attowner=user, attid=14, attname="reldim", attoff=44, attfrmt="i", attfrml=2, attxtra=0)

##	range of t is _atempv8
##	delete t where t.attrelid = "_rtempv8" or t.attrelid = "_rtempv81" or t.attrelid = "_atempv8"

	sysmod = 0;

##	retrieve ( sysmod = r.relspec ) where r.relid = "relation" and r.relowner = user

	if ( sysmod != 5 )
	{
##		modify _rtempv8 to hash on relid
##		modify _atempv8 to hash on attrelid,attowner
	}

	user[0] = user[1] = '\0';
##	retrieve ( user = r.relowner ) where r.relid = "_atempv8"

	if ( (Ingcode[0] = user[0]) == '\0' )
		syserr("can't find my own name");
	Ingcode[1] = user[1];
	Ingcode[2] = '\0';
##	exit
	sync();
}
