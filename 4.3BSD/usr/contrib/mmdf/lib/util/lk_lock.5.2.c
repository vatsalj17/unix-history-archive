#include "util.h"
	/* <lockf.h> may have to be replaced with <unistd.h>    */
	/* on some systems such as the 3B2                      */
#include <lockf.h>
#include <fcntl.h>

/*
 *	Standardized file-locking package  (System 5 Rel 2)
 *
 *  This version assumes the existence of the System 5 lockf() system call
 *  Adapted by Mark Vasoll <vasoll@a.cs.okstate.edu> from the 4.2 BSD version
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
    ll_log (logptr, LLOGBTR, "lk_open (%s,%d,%s,%s,%d)",
		file, access, lockdir, lockfile, maxtime);
#endif

    if ((fd = open (file, access | O_CREAT)) < 0
	|| lockf (fd, F_TLOCK, 0) < 0)
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
char	*file;			/* file to be locked */
char	*lockdir;		/* directory to put parallel file into */
char	*lockfile;		/* file to lock against */
{
    register int retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lk_close (%d,%s,%s,%s)",
		fd, file, lockdir, lockfile);
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
    ll_log (logptr, LLOGBTR, "lk_fopen (%s,%s,%s,%s,%d)",
		file, access, lockdir, lockfile, maxtime);
#endif

    if ((fd = open(file, (access[0]=='r' && access[1]==0 ? 0 : 2)
	| O_CREAT)) < 0 || lockf (fd, F_TLOCK, 0) < 0) {
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
    register int retval;

#ifdef DEBUG
    ll_log (logptr, LLOGBTR, "lk_fclose (%s,%s,%s)", file, lockdir, lockfile);
#endif
    switch (fp)
    {
	case EOF:
	case NULL:
	    return (OK);
    }
    retval = fclose (fp);
    return (retval);
}
