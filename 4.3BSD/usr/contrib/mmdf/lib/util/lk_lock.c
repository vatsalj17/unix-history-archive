#include "util.h"
#include <sys/stat.h>

/*
 *	Standardized file-locking package  (using links)
 *
 *  This version presumes no o/s support of locking, with
 *  link(2) being the only available atomic o/s operation.
 */

extern int errno;               /* simulate system error problems */
#ifdef DEBUG
#include "ll_log.h"
extern LLog *logptr;
#endif

extern	char *lckdfldir;

LOCVAR	int breaktime = 120;   /* amount to sleep after breaking lock */
LOCVAR	char *NIL = "NIL";

/**/

LOCFUN
lk_lock (file, lockdir, lockfile, maxtime)
char	*file;			/* file to be locked */
char	*lockdir;		/* directory to put parallel file into */
char	*lockfile;		/* file to lock against */
int	maxtime;		/* maybe break lock after it is this old */
{
    int sleepval;
    int retval;
    int tries;
    char lkname[100];           /* full name of locking file */
    char tmpname[100];          /* name of tmp file to link from */

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lk_lock (%s,%s,%s,%d)", file,
	    lockdir ? lockdir : NIL, lockfile ? lockfile : NIL, maxtime);
#endif

    if (lk_name (lkname, file, lockdir, lockfile) < OK)
	return (NOTOK);

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "lkname '%s'", lkname);
#endif

    for (tries = 0; tries < 2; tries++)
	switch (lk_test (file, lkname, maxtime))
	{
	    case OK:                /* lock exists & is ok */
#ifdef DEBUG
		ll_log (logptr, LLOGFTR, "already locked & ok");
#endif
		errno = ETXTBSY;
		return (NOTOK);

	    case NOTOK:             /* lock exists and is old */
#ifdef DEBUG
		ll_log (logptr, LLOGFTR, "old lock");
#endif
		if (lk_unlock (file, lockdir, lockfile) == NOTOK)
		    return (NOTOK); /* hmmmmmm */

/*
 *  the amount of time to sleep is at least breaktime.
 *  the random number generation is used to try to separate
 *  two processes that may be close to syncrony in checking the lock.
 */
		srand (getpid ());
		sleepval = breaktime + (rand ()%100);
#ifdef DEBUG
		ll_log (logptr, LLOGFTR, "sleeping (%d)", sleepval);
#endif
		sleep (sleepval);
		continue;

	    default:
		goto lockit;
	}

/* below here, try to lock the guy */

lockit:
    getfpath (",tmp.XXXXXX", lockdir ? lockdir : lckdfldir, tmpname);
    mktemp (tmpname);           /* have to have something to link from */

    if (close (creat (tmpname, 0666)) < OK)
    {
#ifdef DEBUG
	ll_err (logptr, LLOGFTR, "problem w/lock base file '%s'", tmpname);
#endif
	(void) unlink (tmpname);
	return (NOTOK);
    }

    retval = link (tmpname, lkname);

#ifdef DEBUG
    if (retval < 0)
	ll_err (logptr, LLOGFTR,
		    "problem linking '%s' to '%s'", tmpname, lkname);
    else
	ll_log (logptr, LLOGFTR,
		    "linked '%s' to '%s'", tmpname, lkname);
#endif

    (void) unlink (tmpname);

    return (retval);
}
/**/

LOCFUN
lk_unlock (file, lockdir, lockfile)
char	*file;		/* file to be locked */
char	*lockdir;	/* directory to put parallel file into */
char	*lockfile;	/* file to lock against */
{
    int retval;
    char lkname[100];           /* full name of locking file */

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lk_unlock (%s,%s,%s)", file,
	    lockdir ? lockdir : NIL, lockfile ? lockfile : NIL);
#endif

    if (lk_name (lkname, file, lockdir, lockfile) < OK)
	return (NOTOK);

    retval = unlink (lkname);

#ifdef DEBUG
    if (retval < 0)
	ll_err (logptr, LLOGFTR, "problem unlinking '%s'", lkname);
#endif

    return (retval);
}

/**/

LOCFUN
	lk_name (lkname, file, lockdir, lockfile)
char	*lkname;		/* put full name of file to use as lock */
char	*file;			/* resource to be locked */
char	*lockdir;		/* dir containing locking file, if any */
char	*lockfile;		/* file to use ask lock */
{
    struct stat statbuf;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lk_name (%s,%s,%s)", file,
	    lockdir ? lockdir : NIL, lockfile ? lockfile : NIL);
#endif

    if (lockdir == (char *) 0)
	lockdir = lckdfldir;

    if (lockfile && *lockfile )
	getfpath (lockfile, lockdir, lkname);
    else {                      /* make out own file name */
	if (file == (char *) 0) /* no way to create a fixed name */
	{
	    errno = EINVAL;
	    return (NOTOK);
	}
	else
	{
	    if (stat (file, &statbuf) < OK)
		return (NOTOK);
	    sprintf (lkname, "%s/,LK%05.5o%o",
			lockdir, statbuf.st_dev, statbuf.st_ino);
	}
    }

    return (OK);
}
/**/

LOCFUN
	lk_test (file, lkfile, maxtime)
char	*file;
char	*lkfile;
int	maxtime;
{
    time_t curtime;
    struct stat statbuf;

    time (&curtime);

    if (stat (lkfile, &statbuf) < OK)
    {
#ifdef DEBUG
	ll_err (logptr, LLOGBTR, "lk_test couldn't stat lockfile '%s'", lkfile);
#endif
	return (DONE);          /* assume that it does not exist */
    }
    if (maxtime <= 0)
	return (OK);            /* no time limit */

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lockfile (%ld:%ld) minutes",
		(((curtime - statbuf.st_mtime)+59L)/60L), (long) (maxtime));
#endif
    if ((((curtime - statbuf.st_mtime)+59L)/60L) > (long) (maxtime))
    {                           /* well, the lock is too old */
	if (file != (char *) 0) /* is the resource inactive, also? */
	    if (stat (file, &statbuf) >= OK)
	    {
#ifdef DEBUG
		ll_log (logptr, LLOGBTR, "resource access (%ld:%ld) minutes",
		    (((curtime - statbuf.st_mtime)+59L)/60L), (long) (maxtime));
#endif
		if ((((curtime - statbuf.st_atime)+59L)/60L) <= (long) (maxtime))
		    return (OK); /* let them go */
	    }
	return (NOTOK);
    }

    return (OK);        /* lock still has time */
}
/**/

lk_open (file, access, lockdir, lockfile, maxtime)
char	*file;			/* file to be locked */
int	access;                 /* read-write permissions */
char	*lockdir;		/* directory to put parallel file into */
char	*lockfile;		/* file to lock against */
int	maxtime;		/* maybe break lock after it is this old */
{
    register int fd;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lk_open (%s,%d,%s,%s,%d)", file, access,
	    lockdir ? lockdir : NIL, lockfile ? lockfile : NIL, maxtime);
#endif

    if (lk_lock (file, lockdir, lockfile, maxtime) < OK)
    {
#ifdef DEBUG
	ll_err (logptr, LLOGBTR, "open lock notok");
#endif
	return (NOTOK);
    }

    if ((fd = open (file, access)) < 0)
    {
#ifdef DEBUG
	ll_err (logptr, LLOGBTR, "open notok");
#endif
	lk_unlock (file, lockdir, lockfile);
	return (NOTOK);
    }
    return (fd);
}

lk_close (fd, file, lockdir, lockfile)
int fd;
char	*file;			/* file to be locked */
char	*lockdir;		/* directory to put parallel file into */
char	*lockfile;		/* file to lock against */
{
    register int retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lk_close (%d,%s,%s,%s)", fd, file,
	    lockdir ? lockdir : NIL, lockfile ? lockfile : NIL);
#endif
    if (fd < 0)
	return (OK);
    retval = close (fd);
    lk_unlock (file, lockdir, lockfile);
    return (retval);
}
/**/
FILE *
	lk_fopen (file, access, lockdir, lockfile, maxtime)
char	*file;			/* file to be locked */
char	*access;		/* read-write permissions */
char	*lockdir;		/* directory to put parallel file into */
char	*lockfile;		/* file to lock against */
int	maxtime;		/* maybe break lock after it is this old */
{
    register FILE *fp;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lk_fopen (%s,%s,%s,%s)", file, access,
	    lockdir ? lockdir : NIL, lockfile ? lockfile : NIL);
#endif

    if (lk_lock (file, lockdir, lockfile, maxtime)  < OK)
    {
#ifdef DEBUG
	ll_err (logptr, LLOGBTR, "fopen lock notok");
#endif
	return ((FILE *) NULL);
    }

    if ((fp = fopen (file, access)) == NULL)
    {
#ifdef DEBUG
	ll_err (logptr, LLOGBTR, "fopen notok");
#endif
	lk_unlock (file, lockdir, lockfile);
	return ((FILE *) NULL);
    }
    return (fp);
}


lk_fclose (fp, file, lockdir, lockfile)
FILE	*fp;
char	*file;			/* file to be locked */
char	*lockdir;		/* directory to put parallel file into */
char	*lockfile;		/* file to lock against */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lk_fclose (%s,%s,%s)", file,
	    lockdir ? lockdir : NIL, lockfile ? lockfile : NIL);
#endif
    lk_unlock (file, lockdir, lockfile);
    switch ((int)fp) {
	case EOF:
	case NULL:
	    return (OK);
    }
    return (fclose (fp));
}
