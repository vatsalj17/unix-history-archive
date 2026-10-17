/*
 *      Database builder for MMDF address database
 *
 *      Command line:
 *              dbmbuild [-flags] [outfile]
 *
 *              where flags are:
 *              d   -   debugging output
 *              v   -   verbose output
 *		k   -   keep going if a table is missing
 *              n   -   make a new dbm file from scratch
 *
 *      output-file is name of EXISTING dbm file to insert entires.
 *      Note it is two files, an output-file.dir and an output-file.pag.
 *
 *      No arguments defaults to  -n.
 */

#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include "dm.h"
#include "chdbm.h"

extern Table **tb_list;  /* order tables searched         */
extern Domain **dm_list; /* ordr domain tables searched  */
extern Chan  **ch_tbsrch;  /* order chans searched         */

extern char *blt();
extern char *multcat();
extern char *locname;
extern char *tbldfldir;
extern char *tbldbm;
extern char *lckdfldir;

typedef struct
{
    char *dptr;
    int dsize;
} datum;

extern datum fetch ();

typedef struct dbmentry
{
    char dbmvalue[1];
} Entry;

int error;
int Debug;
int Verbose;
int newflag;
int keepgoing;
char dbfile[128];
char *dblock;
int lckfd;

char dbmstr[] = "DBM$$";
			/* string for locking the database */

/*
 * Main procedure.
 * process arguments, set flags and invoke file processing.
 * clean up and exit properly.
 */

main(argc, argv)
char **argv;
{
    int ind;
    char *p;
    char *outfile;
    register Table  *tblptr;

    mmdf_init (argv[0]);
    argv++; argc--;
    outfile = (char *)0;

    if (argc == 0)
	newflag++;

    while(argc-- > 0)
    {
	if (Debug)
	    fprintf (stderr, "arg='%s'\n", *argv);
	p = *argv++;
	if(*p == '-')
	{
	    while(*++p)
		switch(*p)
		{
		    case 'O':	/* Obsolete, ignored */
			    break;

		    case 'd':
			    Debug++;
			    break;

		    case 'k':
			    keepgoing++;
			    break;

		    case 'n':
			    newflag++;
			    break;

		    case 'v':
			    Verbose++;
			    break;

		    default:
			    fprintf (stderr,"Unknown flag %c\n",*p);
			    break;
		}
	}
	else
	{
	     outfile = p;
	     break;
	}
    }

    if (outfile == 0)           /* use default database */
    {
	if (tbldbm == 0 || tbldbm[0] == '\0')
	{
	    fprintf (stderr, "no default data base, in 'tbldbm' variable\n");
	    exit (NOTOK);
	}
	outfile = tbldbm;
    }

    /*
     *  Check for existence first
     */
    error = 0;
    if (argc <= 0) {
	for (ind = 0; dm_list[ind] != (Domain *)0; ind++)
	    error += check(dm_list[ind] -> dm_table);

	for (ind = 0; ch_tbsrch[ind] != (Chan *) 0; ind++)
	{
	    error += check(ch_tbsrch[ind] -> ch_table);

	    if ((tblptr = ch_tbsrch[ind] -> ch_insource) != (Table *) 0)
		error += check(tblptr);
	    if ((tblptr = ch_tbsrch[ind] -> ch_outsource) != (Table *) 0)
		error += check(tblptr);
	    if ((tblptr = ch_tbsrch[ind] -> ch_indest) != (Table *) 0)
		error += check(tblptr);
	    if ((tblptr = ch_tbsrch[ind] -> ch_outdest) != (Table *) 0)
		error += check(tblptr);
	}

	for (ind = 0; (tblptr = tb_list[ind]) != (Table *) 0; ind++)
	    error += check (tblptr);
    } else {
	while (argc-- > 0)
	{
	    if ((tblptr = tb_nm2struct (*argv++)) == (Table *) NOTOK)
	    {
		fprintf (stderr, "Table '%s' is unknown\n", *--argv);
		error++;
	    };
	    error += check(tblptr);
	}
    }

    if (error) {
	fprintf (stderr, "Some tables are missing.  dbmbuild aborted.\n");
    	exit(9);
    }

    getfpath (outfile, tbldfldir, dbfile);
    dblock = multcat (dbfile, ".lck", (char *)0);
    (void) close( creat( dblock, 0444 ));

    if ((lckfd = lk_open(dblock, 0, (char *)0, (char *)0, 10)) < 0) {
	perror (dblock);
	fprintf (stderr, "Data base %s is locked.  Try again later.\n",
		 dbfile);
	exit (NOTOK);
    }

    if (!theinit (newflag))
	cleanup (-1);

    if (argc <= 0)
    {
	/*
	 *  Now process for real!
	 */
	for (ind = 0; dm_list[ind] != (Domain *)0; ind++)
	{
	    if (Debug)
		fprintf (stderr, "Domain '%s'\n", dm_list[ind] -> dm_show);
	    process (dm_list[ind] -> dm_table);
	}

	for (ind = 0; ch_tbsrch[ind] != (Chan *) 0; ind++)
	{
	    if (Debug)
		fprintf (stderr, "Chan '%s'\n", ch_tbsrch[ind] -> ch_show);
	    process (ch_tbsrch[ind] -> ch_table);

	    if ((tblptr = ch_tbsrch[ind] -> ch_insource) != (Table *) 0)
	    {
		if (Debug)
		    fprintf (stderr, "Source inbound filter\n");
		if (Verbose || Debug)
		    putc ('\t', stderr);
		process (tblptr);
	    }
	    if ((tblptr = ch_tbsrch[ind] -> ch_outsource) != (Table *) 0)
	    {
		if (Debug)
		    fprintf (stderr, "Source outbound filter\n");
		if (Verbose || Debug)
		    putc ('\t', stderr);
		process (tblptr);
	    }
	    if ((tblptr = ch_tbsrch[ind] -> ch_indest) != (Table *) 0)
	    {
		if (Debug)
		    fprintf (stderr, "Destination inbound filter\n");
		if (Verbose || Debug)
		    putc ('\t', stderr);
		process (tblptr);
	    }
	    if ((tblptr = ch_tbsrch[ind] -> ch_outdest) != (Table *) 0)
	    {
		if (Debug)
		    fprintf (stderr, "Destination outbound filter\n");
		if (Verbose || Debug)
		    putc ('\t', stderr);
		process (tblptr);
	    }
	}

	for (ind = 0; (tblptr = tb_list[ind]) != (Table *) 0; ind++)
	{                           /* in case table used by chan */
	    if (tblptr -> tb_fp != (FILE *) EOF)
	    {
		if (Debug)
		    fprintf (stderr, "Table '%s'\n", tblptr -> tb_name);
		process (tblptr);
	    }
	}
    }
    else
	while (argc-- > 0)
	{
	    if ((tblptr = tb_nm2struct (*argv++)) == (Table *) NOTOK)
	    {
		 (void) fflush (stdout);
		 fprintf (stderr, "Table '%s' is unknown\n", *--argv);
		 cleanup (-1);
	    };
	    process (tblptr);
	}

    cleanup (theend (newflag));
}

/**/

theinit (new)               /* initialize the dbm files */
int new;
{
    char tmpfile[100];

    if (new)                /* start with fresh files */
    {
	if (Verbose || Debug)
	    fprintf (stderr, "Temporary database:  %s$\n", dbfile);
	(void) sprintf (tmpfile, "%s$.pag", dbfile);
	if(Debug)
	    fprintf (stderr, "creating '%s'\n", tmpfile);

	if (close (creat (tmpfile, 0644)) < 0)
	{                       /* create and/or zero the file */
	    fprintf (stderr, "could not creat '%s':  ", tmpfile);
	    perror ("");
	    cleanup (-1);
	}
	chmod (tmpfile, 0644);  /* in case umask screwed us */

	(void) sprintf (tmpfile, "%s$.dir", dbfile);
	if(Debug)
	    fprintf (stderr, "creating '%s'\n", tmpfile);

	if (close (creat (tmpfile, 0644)) < 0)
	{                       /* create and/or zero the file */
	    fprintf (stderr, "could not creat '%s':  ", tmpfile);
	    perror ("");
	    cleanup (-1);
	}
	chmod (tmpfile, 0644);  /* in case umask screwed us */
	(void) sprintf (tmpfile, "%s$", dbfile);
	return (dbfinit (tmpfile));
    }

    return (dbfinit (dbfile));
}

theend (new)               /* cleanup the dbm files */
int new;
{
    char fromfile[100];
    char tofile[100];

    dbfclose();

    if (new)                /* started with fresh files */
    {
	if (Verbose || Debug)
	    fprintf(stderr, "Moving to database:  %s\n", dbfile);

	(void) sprintf (fromfile, "%s$.pag", dbfile);
	(void) sprintf (tofile, "%s.pag", dbfile);
	if (Debug)
	    fprintf (stderr, "moving '%s'\n", fromfile);

	(void) unlink (tofile);

	if ((link (fromfile, tofile ) < 0) || (unlink (fromfile) < 0))
	{                       /* create and/or zero the file */
	    fprintf (stderr, "could not link to '%s':  ", tofile);
	    perror ("");
	    cleanup (-1);
	}

	(void) sprintf (fromfile, "%s$.dir", dbfile);
	(void) sprintf (tofile, "%s.dir", dbfile);
	if(Debug)
	    fprintf (stderr, "moving '%s'\n", fromfile);

	(void) unlink (tofile);

	if ((link (fromfile, tofile ) < 0) || (unlink (fromfile) < 0))
	{                       /* create and/or zero the file */
	    fprintf (stderr, "could not link to '%s':  ", tofile);
	    perror ("");
	    cleanup (-1);
	}

	return (TRUE);
    }

    return (TRUE);
}

/*
 * Initialize the dbm file.
 * Fetch the local name datum
 * Init to 1 and store it if there is no datum by that name.
 */

dbfinit(filename)
char *filename;
{
	if(dbminit(filename) < 0)
	{
	    fprintf (stderr, "could not initialize data base '%s'", filename);
	    perror("");
	    return (0);
	}
	return (1);
}

/*
 * Close the dbm datafile.
 * We can't close the file because the library does not provide the call...
 */

dbfclose()
{
    return (1);
}

/*
 *      Process a sequential file and insert items into the database
 *      Opens argument assumes database is initialized.
 */

process (tblptr)
    Table *tblptr;
{
    datum key, value;
    char tbkey[LINESIZE],
	 tbvalue[LINESIZE];
    char *cp;

    if (Verbose || Debug)
	fprintf (stderr, tblptr -> tb_fp == (FILE *)EOF ?
		"%s (%s) already done\n" : "%s (%s)\n",
		tblptr -> tb_show, tblptr -> tb_name);
    if (tblptr -> tb_fp == (FILE *)EOF)
	return;

#ifdef NAMESERVER
    if ((tblptr -> tb_flags & TB_SRC) == TB_NS) {
    	if (Debug || Verbose)
	    fprintf (stderr, "   (Nameserver table)\n");
	return;
    }
#endif NAMESERVER
    if (!tb_open (tblptr, TRUE)) /* gain access to a channel table */
    {
	fprintf (stderr, "could not open table \"%s\" (%s, file = '%s'):\n\t",
			tblptr -> tb_show, tblptr -> tb_name,
			tblptr -> tb_file);
	perror("");
	tblptr -> tb_fp = (FILE *) EOF;		/* Don't try again */
    	if (keepgoing)
    	    return;
    	cleanup(-1);
    }

    while (tb_read (tblptr, tbkey, tbvalue))
    {                           /* read a table's record        */
	switch (tbkey[0])
	{
	    case '\0':
	    case '#':
	    case ';':
		continue;       /* empty lines and comments ok */
	}

	if(tbvalue[0] == '\0')
	{                       /* empty value field            */
	    value.dptr = "";
	    value.dsize = 1;
	}
	else
	{
	    value.dptr = tbvalue;
	    value.dsize = strlen (tbvalue) + 1;
	}
    	for (cp = tbkey; *cp != 0; cp++)
    	    *cp = uptolow(*cp);			/* All keys are lower case */
	key.dptr = tbkey;
	key.dsize = strlen (tbkey) + 1;
	if(Debug)
	    fprintf (stderr, "(%d)'%s': (%d)'%s'\n",
		     key.dsize, key.dptr, value.dsize, value.dptr);
	install(key, value, tblptr -> tb_name);
    }

    if (ferror (tblptr -> tb_fp))
    {
	fprintf (stderr, "i/o error with table %s (%s, file = %s):  ",
			    tblptr -> tb_show, tblptr -> tb_name,
			    tblptr -> tb_file);
	perror ("");
    }
    tb_close (tblptr);
    tblptr -> tb_fp = (FILE *) EOF;		/* Don't try again */
}

/*
 * Install a datum into the database..
 * Fetch entry first to see if we have to append
 * name for building entry.
 */

install(key, value, tbname)
datum key, value;
char tbname[];
{
    datum old;
    char newentry[ENTRYSIZE];
    register char *p;

    p = newentry;
    old = fetch (key);
    if (old.dptr != NULL) {
	if (Debug) {
	    fprintf (stderr, "\tFound old entry\n\t");
	    prdatum (old);
	}

	p = blt (old.dptr, p, old.dsize - 1);
	*p++ = FS;			/* end with a record seperator */
    }
    p = blt (tbname, p, strlen (tbname));
    *p++ = ' ';
    p = blt (value.dptr, p, value.dsize);

    old.dptr = newentry;
    old.dsize = p - newentry;
    if (Debug) {
	fprintf (stderr, "\tNew datum\n\t");
	prdatum (old);
    }
    if (store (key, old) < 0) {          /* put the datum back */
	fprintf (stderr, "dbmbuild, install:  store failed; key='%s', value=",
								key.dptr);
	prdatum (old);
	cleanup (NOTOK);
    }
    return (1);
}
/*
 *      Print a datum.
 *      Takes the datum argument and prints it so that we can read
 *      it as either a key or an entry.
 */

prdatum(value)
datum value;
{
	int cnt;
	char data[512];

	blt (value.dptr, data, value.dsize);

	for (cnt = 0; cnt < value.dsize; cnt++) {
		if (value.dptr[cnt] >= ' ' && value.dptr[cnt] <= '~')
			putc (value.dptr[cnt], stderr);
		else
			fprintf (stderr, "<\\%03o>", value.dptr[cnt]);
	}
	putc ('\n', stderr);
}

check (tblptr)
Table *tblptr;
{
#ifdef NAMESERVER
    if ((tblptr -> tb_flags & TB_SRC) == TB_NS)
	return(0);
#endif NAMESERVER
    if (tblptr -> tb_fp == (FILE *)EOF)
	return(1);		/* We already know its bad */
    if (!tb_open (tblptr, TRUE)) /* gain access to a channel table */
    {
	fprintf (stderr, "could not open table \"%s\" (%s, file = '%s'):\n\t",
			tblptr -> tb_show, tblptr -> tb_name,
			tblptr -> tb_file);
	perror("");
	tblptr -> tb_fp = (FILE *) EOF;
	return(1);
    }
    tb_close (tblptr);
    tblptr -> tb_fp = (FILE *) NULL;
    return(0);
}

cleanup (exitval)
int     exitval;
{
	lk_close (lckfd, dblock, (char *)0, (char *)0);
	exit (exitval == TRUE? 0 : 1);	/* TRUE is non-zero */
}
