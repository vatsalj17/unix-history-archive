#include "util.h"
#include "ll_log.h"
#include "cmd.h"

extern int errno;               /* put error code into here, like sys call */
extern char *tai_eptr;          /* pointer to error text  */

/*      tailor other packages   */

tai_pgm (argc, argv, nam, path) /* get name&path of a program */
    int argc;                   /* number of values     */
    char *argv[];               /* list of values       */
    char **nam,                 /* where to put the name of the program */
	 **path;                /* where to put path to program */
{
    register int ind;

    for (ind = 0; ind < argc; ind++)
    {
	if (lexequ ("=", argv[ind]))
	{                       /* key/value pair       */
	    if ((ind += 2) >= argc)
	    {
		errno = EFAULT;
		tai_eptr = argv[ind - 1];       /* point to 'key' */
		argv[ind + 1] = 0;
		return (NOTOK);
	    }
	    if (lexequ ("path", argv[ind - 1]))
		*path = argv[ind];
	    else
	    if (lexequ ("name", argv[ind - 1]))
		*nam = argv[ind];
	    else
	    {
		errno = EINVAL;
		tai_eptr = argv[ind - 1];       /* point to 'key' */
		argv[ind + 1] = 0;
		return (NOTOK);
	    }
	}
	else
	{                               /* same string for both purposes */
	    *path = argv[ind];
	    *nam = argv[ind];
	}
    }
    return (YES);
}
/**/

#define CMDLHDR     1
#define CMDLLEVEL   2
#define CMDLSIZE    3
#define CMDLSTAT    4

LOCVAR Cmd
	    cmdlog[] =
{
    "hdr",      CMDLHDR,    1,
    "level",    CMDLLEVEL,  1,
    "size",     CMDLSIZE,   1,
    "stat",     CMDLSTAT,   1,
    0,          0,          0
};

#define CMDLPFAT    1
#define CMDLPTMP    2
#define CMDLPGEN    3
#define CMDLPBST    4
#define CMDLPFST    5
#define CMDLPPTR    6
#define CMDLPBTR    7
#define CMDLPFTR    8

LOCVAR Cmd
	    cmdlevel[] =
{
    "fat",      CMDLPFAT,   0,
    "tmp",      CMDLPTMP,   0,
    "gen",      CMDLPGEN,   0,
    "bst",      CMDLPBST,   0,
    "fst",      CMDLPFST,   0,
    "ptr",      CMDLPPTR,   0,
    "btr",      CMDLPBTR,   0,
    "ftr",      CMDLPFTR,   0,
#ifdef NVRCOMPIL
	these are commented out in order to save a trivial amount of
	space.  if you really want the synonyms, add them.  (dhc)
    "llogfat",  CMDLPFAT,   0,
    "llogtmp",  CMDLPTMP,   0,
    "lloggen",  CMDLPGEN,   0,
    "llogbst",  CMDLPBST,   0,
    "llogfst",  CMDLPFST,   0,
    "llogptr",  CMDLPPTR,   0,
    "llogbtr",  CMDLPBTR,   0,
    "llogftr",  CMDLPFTR,   0,
#endif
    0,          0,          0
};

#define CMDLSCLS    1
#define CMDLSCYC    2
#define CMDLSWAT    3
#define CMDLSSOME   4

LOCVAR Cmd
	    cmdlstat[] =
{
    "close",    CMDLSCLS,   0,
    "wait",     CMDLSWAT,   0,
    "cycle",    CMDLSCYC,   0,
    "some",     CMDLSSOME,  0,
#ifdef NVRCOMPIL
    "llogcls",  CMDLSCLS,   0,
    "cls",      CMDLSCLS,   0,
    "wat",      CMDLSWAT,   0,
    "llogwat",  CMDLSWAT,   0,
    "cyc",      CMDLSCYC,   0,
    "llogcyc",  CMDLSCYC,   0,
    "llogsome", CMDLSSOME,  0,
#endif
    0,          0,          0
};


tai_log (argc, argv, thelog)    /* get ll_log structure values */
    int argc;                   /* number of values     */
    char *argv[];               /* list of values       */
    LLog *thelog;               /* the ll_log struct to modify */
{
    register int ind;

    for (ind = 0; ind < argc; ind++)
    {
	if (lexequ ("=", argv[ind]))
	{                       /* key/value pair       */
	    ind += 2;
	    switch (cmdsrch (argv[ind - 1], argc - ind + 1, cmdlog))
	    {
		case CMDLHDR:
		    thelog -> ll_hdr = argv[ind];
		    break;

		case CMDLLEVEL:
		    if (isdigit (argv[ind][0]))
			sscanf (argv[ind], "%d", &(thelog -> ll_level));
		    else
			switch (cmdsrch (argv[ind], 0, cmdlevel))
			{
			    case CMDLPFAT:
				thelog -> ll_level = LLOGFAT;
				break;

			    case CMDLPTMP:
				thelog -> ll_level = LLOGTMP;
				break;

			    case CMDLPGEN:
				thelog -> ll_level = LLOGGEN;
				break;

			    case CMDLPBST:
				thelog -> ll_level = LLOGBST;
				break;

			    case CMDLPFST:
				thelog -> ll_level = LLOGFST;
				break;

			    case CMDLPPTR:
				thelog -> ll_level = LLOGPTR;
				break;

			    case CMDLPBTR:
				thelog -> ll_level = LLOGBTR;
				break;

			    case CMDLPFTR:
				thelog -> ll_level = LLOGFTR;
				break;

			    default:
				errno = EINVAL;
				tai_eptr = argv[ind - 1];
				argv[ind + 1] = 0;
				return (NOTOK);
			}
		    break;

		case CMDLSIZE:
		    thelog -> ll_msize = atoi (argv[ind]);
		    break;

		case CMDLSTAT:
		    if (isdigit (argv[ind][0]))
			    /* not all stdio's have a test for octal (dhc) */
			sscanf (argv[ind], "%o", &(thelog -> ll_stat));
		    else
			switch (cmdsrch (argv[ind], 0, cmdlstat))
			{
			    case CMDLSCLS:
				thelog -> ll_stat |= LLOGCLS;
				break;

			    case CMDLSWAT:
				thelog -> ll_stat |= LLOGWAT;
				break;

			    case CMDLSCYC:
				thelog -> ll_stat |= LLOGCYC;
				break;

			    case CMDLSSOME:
				thelog -> ll_stat |= LLOGSOME;
				break;

			    default:
				errno = EINVAL;
				tai_eptr = argv[ind - 1];
				argv[ind + 1] = 0;
				return (NOTOK);
			}
		    break;

		default:
		    errno = EINVAL;
		    tai_eptr = argv[ind - 1];   /* point to 'key' */
		    argv[ind + 1] = 0;
		    return (NOTOK);
	    }
	}
	else
	{
	    if (ind == 0)
		thelog -> ll_file = argv[ind];
	    else
	    {
		errno = EINVAL;
		tai_eptr = argv[ind - 1];   /* point to 'key' */
		argv[ind + 1] = 0;
		return (NOTOK);
	    }
	}
    }
/*****  disabled due to bug in mm_tai.c:tai_llev() that calles this
/*****  function with a garbage log pointer.
/*****    ll_close (thelog);          /* start with a clean slate */
    return (YES);
}
