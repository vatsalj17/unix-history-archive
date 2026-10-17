#include "util.h"
#include "mmdf.h"

/*
 *      Print contents of MMDF address database
 *
 */

#include "ch.h"
#include "chdbm.h"

extern char *blt();
extern char *locname;
extern char *tbldfldir;
extern char *tbldbm;

typedef struct
{
    char *dptr;
    int dsize;
} datum;

extern datum fetch (),
	     firstkey (),
	     nextkey ();

datum key,
	value;

char dbfile[128];

/*
 * Main procedure.
 * process arguments, set flags and invoke file processing.
 * clean up and exit properly.
 */

main(argc, argv)
char **argv;
{
    char tmp[100];
    char *path;

    mmdf_init (argv[0]);
    for (path = 0, argv++; argc-- > 1; argv++) 
    {
	if (argv[0][0] == '-')
	    switch (argv[0][1])
	    {
		default: ;
	    }
	else
	    path = argv[0];
    }
    getfpath ((path == 0) ? tbldbm : path, tbldfldir, dbfile);

    if(dbminit(dbfile) < 0)
    {
	    perror("tblprint, fileinit");
	    exit (-1);
    }

    for (key = firstkey (); key.dptr != NULL; key = nextkey (key))
    {
	blt (key.dptr, tmp, key.dsize);
	tmp[key.dsize] = '\0';
	printf ("%-15s:  ", tmp);      /* key string */
	value = fetch (key);
	prdatum (value);
    }
    exit (0);
}

prdatum(val)
datum val;
{
    register int cnt;
    register char *dbptr;

    for (dbptr = val.dptr, cnt = val.dsize - 1; cnt-- > 0; dbptr++)
    {
	if (*dbptr >= ' ' && *dbptr <= '~')
	    fputc (*dbptr, stdout);
	else
	    fprintf (stdout, "<\\%03o>", *dbptr);
    }
    fputs ("\n",stdout);
}
