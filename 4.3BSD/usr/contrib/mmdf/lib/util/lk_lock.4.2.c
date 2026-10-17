#include "util.h"
#include <sys/file.h>

/*
 *	Standardized file-locking package  (4.2BSD)
 *
 *  This version assumes the existence of the 4.2BSD flock() system call.
 */
extern int errno;               /* simulate system error problems */

#ifdef DEBUG
#include "ll_log.h"
extern LLog *logptr;
#endif

/**/

lk_open (file, access, lockdir, lockfile, maxtime)
char	*file;			/* file to be locked */
int	access;			/* read-write permissions */
char	*lockdir;		/* --Ignored-- */
char	*lockfile;		/* --Ignored-- */
int	maxtime;		/* maybe break lock after it is this old */
{
    register int fd;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lk_open (%s,%d,%d)",
		file, access, maxtime);
#endif

    if ((fd = open (file, access | O_NDELAY)) < 0
	|| flock (fd, LOCK_EX|LOCK_NB) < 0)
    {
#ifdef DEBUG
	ll_err (logptr, LLOGBTR, "open notok (err %d)", errno);
#endif
    	if (fd >= 0)
    	    (void) close (fd);
	return (NOTOK);
    }
    return (fd);
}

lk_close (fd, file, lockdir, lockfile)
int	fd;
char	*file;			/* -- Ignored -- */
char	*lockdir;		/* -- Ignored -- */
char	*lockfile;		/* -- Ignored -- */
{
    register int retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lk_close (%d)", fd);
#endif
    if (fd < 0)
	return (OK);
    retval = close (fd);
    return (retval);
}
/**/
FILE *
	lk_fopen (file, access, lockdir, lockfile, maxtime)
char	*file;			/* file to be locked */
char	*access;		/* read-write permissions */
char	*lockdir;		/* --Ignored-- */
char	*lockfile;		/* --Ignored-- */
int	maxtime;		/* maybe break lock after it is this old */
{
    register int fd;
    register FILE *fp;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lk_fopen (%s,%s,%d)", file, access, maxtime);
#endif

    if ((fd = open(file, ((access[0]=='r' && access[1]==0) ? 0 : 2)
	| O_NDELAY)) < 0 || flock (fd, LOCK_EX|LOCK_NB) < 0) {
#ifdef DEBUG
	ll_err (logptr, LLOGBTR, "open notok");
#endif
	if (fd >= 0)
	    (void) close (fd);
	return (NULL);
    }

    if ((fp = fdopen (fd, access)) == NULL)
    {
	(void) close (fd);
	return ((FILE *) NULL);
    }
    return (fp);
}

lk_fclose (fp, file, lockdir, lockfile)
FILE	*fp;
char	*file;			/* --Ignored-- */
char	*lockdir;		/* --Ignored-- */
char	*lockfile;		/* --Ignored-- */
{
#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lk_fclose ()");
#endif
    switch ((int)fp) {
    case EOF:
    case NULL:
	    return (OK);
    }
    return (fclose (fp));
}
