#include "util.h"
#include "mmdf.h"
#include "chdbm.h"

/*
 * Edit the mmdf dbm database - Phil Cockcroft UCL
 * From an Idea by Peter Collinson
 *      Should really use the mmdf routines but it is easier to write
 *      your own than to find the right ones.
 */

extern char *tbldbm;
extern char *tbldfldir;
extern char *dupfpath();
extern char *multcat();
extern char *gets();

typedef struct  {
	char    *dptr;
	int     dsize;
} datum;

extern  datum   fetch();
datum   dcons();

char    ibuf[LINESIZE];
#define MARGS   100
char    *argv[MARGS];                /* max number of args on a line */
int     argc;

int     help(), add(), del(), change(), print(), quit();

struct  cmds    {
	char    *cm_name;
	int     (*cm_func)();
	char    *cm_help;
} cmds[] = {
	"help", help,   "Print out help text",
	"add",  add,    "Add an entry",
	"delete", del,  "Delete an entry",
	"change", change, "Change an entry",
	"print", print, "Print an entry",
	"quit", quit,   "Quit the program",
	"h",    help,   0,              /* single character aliases */
	"a",    add,    0,
	"d",    del,    0,
	"c",    change, 0,
	"p",    print,  0,
	"q",    quit,   0,
	"?",    help,   0,
}, *curcmd;

int	verbose;

/*
 * dbm expanded structure
 */

#define MDB     50

char	*dblock;
int     ndbents;

struct  db      {
	char    *d_table;
	char    *d_value;
} dbs[MDB], *lastdb;

main(ac, av)
char **av;
{
	mmdf_init(av[0]);

	dblock = multcat (dupfpath (tbldbm, tbldfldir), ".lck", (char *)0);
	(void) close( creat( dblock, 0444 ));

	while (ac > 1 && av[1][0] == '-') {
		if (lexequ(av[1], "-v")) {
			verbose++;
			ac--;
			av++;
		} else if (lexequ(av[1], "-d")) {
			if (ac < 3) {
				fprintf(stderr, "dbmedit: missing database path\n");
				exit(NOTOK);
			}
			tbldbm = av[2];
			ac -= 2;
			av += 2;
			dblock = multcat (tbldbm, ".lck", (char *)0);
			(void) close( creat( dblock, 0444 ));
		} else {
			fprintf(stderr, "dbmedit: unknown argument '%s'\n", av[1]);
			exit(NOTOK);
		}
	}
	if(!isstr(tbldbm)) {
		fprintf(stderr, "dbmedit: cannot find database path\n");
		exit(NOTOK);
	}
	dbfinit(tbldbm);

	/* if got command line arguments process these instead */
	if(ac > 1){
		avfix(ac, av);
		if(findcmd() < 0)
			exit(1);
		exit((*curcmd->cm_func)());
	}

	for(;;){
		printf("dbmedit> ");
		(void) fflush(stdout);
		if (gets(ibuf) == NULL)
			exit(0);
		if (!isstr(ibuf))
			continue;
		if ((argc = sstr2arg(ibuf, MARGS, argv, " \t")) == 0)
			continue;
		if(findcmd() < 0)
			continue;
		(*curcmd->cm_func)();
	}
}

avfix(ac, av)
register char   **av;
{
	register char   **ap;

	for(av++, ac--, ap = argv; ac ; ac--, argc++)
		*ap++ = *av++;
}

findcmd()
{
	register struct cmds *cp;

	for(cp = cmds ; cp < &cmds[sizeof(cmds)/sizeof(cmds[0])] ; cp++)
		if(lexequ(cp->cm_name, argv[0])){
			curcmd = cp;
			return(0);
		}
	fprintf(stderr, "dbmedit: unknown command '%s'\n", argv[0]);
	return(-1);
}

help()
{
	register struct cmds *cp;

	for(cp = cmds ; cp < &cmds[sizeof(cmds)/sizeof(cmds[0])] ; cp++)
		if(cp->cm_help != 0)
			printf("%s\t\t%s\n", cp->cm_name, cp->cm_help);
	return(0);
}

datum
dcons(value)
char    *value;
{
	datum d;

	d.dptr = value;
	d.dsize = strlen(value)+1;
	return(d);
}

dssplit(str)
register char   *str;
{
	register struct db      *dp;
	static  char    ti[512];
	char    *strcpy();

	ndbents = 0;
	str = strcpy(ti, str);
	for(dp = dbs ; dp < &dbs[sizeof(dbs)/sizeof(dbs[0])]; dp++){
		if(*str == 0)
			break;
		dp->d_table = str;
		while(*str && *str != ' ')
			str++;
		*str++ = 0;
		dp->d_value = str;
		while(*str && *str != FS)
			str++;
		ndbents++;
		if(*str == FS)
			*str++ = 0;
	}
	lastdb = dbs + ndbents;
}

add()
{
	datum   d;
	register struct db      *dp;

	if(argc != 4){
		fprintf(stderr, "Usage: add key table value\n");
		return(9);
	}
	d = fetch(dcons(argv[1]));
	if(d.dptr == 0){
		if (verbose)
			printf("Creating new key: \"%s\" table: \"%s\" value: \"%s\"\n",
						argv[1], argv[2], argv[3]);
		lastdb = dbs;
		goto newalias;
	}
	dssplit(d.dptr);
	if (verbose)
	    for(dp = dbs ; dp < lastdb ; dp++){
		if(lexequ(dp->d_table, argv[2])){
		    fprintf(stderr,
	     "Warning: key \"%s\" has existing value \"%s\" in table \"%s\"\n",
					argv[1], dp->d_value, argv[2]);
		}
	    }

newalias:;
	dp = lastdb++;
	dp->d_table = argv[2];
	dp->d_value = argv[3];
	return(dscons());
}

del()
{
	datum   d;
	register struct db      *dp;
	int     changed = 0;
	int	lckfd;

	if (argc < 2 || argc > 4) {
		fprintf(stderr, "Usage: delete key [table [value] ]\n");
		return(9);
	}

	d = fetch(dcons(argv[1]));
	if (d.dptr == 0) {
		fprintf(stderr, "Key \"%s\" not found in database\n", argv[1]);
		return(99);
	}
	if (argc == 2) {	/* all entries */
		if (verbose)
			printf("Deleting key \"%s\"\n", argv[1]);
		goto delall;
	}
	dssplit(d.dptr);
	for(dp = dbs ; dp < lastdb ; dp++) {
		if(dp->d_table && lexequ(dp->d_table, argv[2]) 
		   && ( (argc == 3) || lexequ(dp->d_value, argv[3]) )) {
			if (verbose)
			    printf("Deleting \"%s\" from table \"%s\" (value: \"%s\")\n",
					argv[1], argv[2], dp->d_value);
			changed++;
			dp->d_table = 0;
		}
	}
	if(!changed){
	    if (argc == 3)
		fprintf(stderr,
			"\"%s\" does not have a value in table \"%s\"\n",
						argv[1], argv[2]);
	    else
		fprintf(stderr,
		    "\"%s\" does not have a value of \"%s\" in table \"%s\"\n",
						argv[1], argv[3], argv[2]);
	    return(99);
	}
	if(lastdb - changed <= dbs) {
		if (verbose)
			printf("All values deleted for \"%s\" - deleting key\n",
								argv[1]);
delall:		if ((lckfd = lk_open(dblock, 0, (char *)0, (char *)0, 10)) < 0) {
			perror(dblock);
			printf("Database %s is locked.  Try again later.\n",
				 tbldbm);
			return(1);
		}
		if(delete(dcons(argv[1])) < 0)
			fprintf(stderr, "Delete failed\n");
		lk_close (lckfd, dblock, (char *)0, (char *)0);
		return(0);
	}
	return(dscons());
}

change()
{
	datum   d;
	register struct db      *dp;
	int	newvalpos;

	if(argc < 4 || argc > 5) {
		fprintf(stderr,
			"Usage: change key table [oldvalue] newvalue\n");
		return(9);
	}

	newvalpos = (argc == 4) ? 3 : 4;
	d = fetch(dcons(argv[1]));
	if(d.dptr == 0) {
		if (verbose)
			fprintf(stderr, "key \"%s\" not found in database\n",
				argv[1]);
		if (argc == 4)
			return(add());
		return(0);
	}
	dssplit(d.dptr);
	for(dp = dbs ; dp < lastdb ; dp++)
		if(lexequ(dp->d_table, argv[2])){
			if ((argc == 5) && !lexequ(dp->d_value, argv[3]))
				continue;
			if (verbose)
				printf("changing \"%s\" to \"%s\"\n",
						dp->d_value, argv[newvalpos]);
			dp->d_value = argv[newvalpos];
			break;
		}

	if(dp >= lastdb){  /* no change made */
		if (argc == 4)
			return(add());
		else {
			fprintf(stderr, 
				"No entry found for key \"%s\" in table \"%s\" with value \"%s\"\n",
				argv[1], argv[2], argv[3]);
			return(0);
		}
	}
	return(dscons());
}

print()
{
	datum   d;
	register struct db      *dp;

	if(argc < 2){
		fprintf(stderr, "Usage: print key [table]\n");
		return(9);
	}

	d = fetch(dcons(argv[1]));
	if(d.dptr == 0){
		fprintf(stderr, "key \"%s\" not found in database\n", argv[1]);
		return(99);
	}
	dssplit(d.dptr);
	for(dp = dbs ; dp < lastdb ; dp++)
		if (argc == 2 || lexequ(dp->d_table, argv[2]) ||
		    lexequ("*", argv[2]))
			printf("key \"%s\": table \"%s\": value \"%s\"\n",
				argv[1], dp->d_table, dp->d_value);
	return(0);
}

dscons()
{
	char    tbuf[ENTRYSIZE];
	datum   d;
	register struct db      *dp;
	register char   *p, *q;
	int     changed = 0;
	int	lckfd;

	for(p = tbuf, dp = dbs ; dp <lastdb ; dp++){
		if((q = dp->d_table) == 0)
			continue;
		if(p != tbuf)
			*(p-1) = FS;
		changed++;
		while(*p++ = *q++);
		*(p-1) = ' ';
		for(q = dp->d_value ; *p++ = *q++;);
	}
	if (!changed) {
		fprintf(stderr, "dbmedit: internal error 'no change'\n");
		return(99);
	}
	d = dcons(tbuf);

	if ((lckfd = lk_open(dblock, 0, (char *)0, (char *)0, 10)) < 0) {
		perror (dblock);
		printf("Database %s is locked.  Try again later.\n", tbldbm);
		return(1);
	}
	if (store(dcons(argv[1]), d) < 0) {
		fprintf(stderr, "dbmedit: cannot store new value for '%s'\n", argv[1]);
	}
	lk_close (lckfd, dblock, (char *) 0, (char *)0);
	return(0);
}

/*
 * Initialize the dbm file.
 */

dbfinit(filename)
char *filename;
{
	if(dbminit(filename) < 0) {
	    fprintf (stderr, "could not initialize data base '%s'", filename);
	    perror("");
	    exit(99);
	}
	return (0);
}

quit()
{
	exit(0);
}
