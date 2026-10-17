#include "util.h"

/*
 *	Standardized file-locking package  (RAND exclusive open)
 *
 *  This version assumes the existence of the RAND exclusive open
 *  in the operating system.  (DPK @ BRL, 21 Dec 82)
 */
#define	FEXCLUSV	04	/* OR this in to get an exclusive open */

extern int errno;               /* simulate system error problems */

#ifdef DEBUG
#include "ll_log.h"
extern struct ll_struct *logptr;
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

    if ((fd = open (file, FEXCLUSV | access)) < 0)
    {
#ifdef DEBUG
	ll_err (logptr, LLOGBTR, "open notok (err %d)", errno);
#endif
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

    if ((fd = open(file,
	  FEXCLUSV | (access[0]=='r' && access[1]==0 ? 0 : 2))) < 0) {
#ifdef DEBUG
	ll_err (logptr, LLOGBTR, "open notok");
#endif
	return( NULL );
    }

    if ((fp = fdopen (fd, access)) == NULL)
    {
	(void) close( fd );
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
    ll_log (logptr, LLOGBTR, "lk_fclose (%s,%s,%s)", file,
	    (lockdir ? lockdir : ""), (lockfile ? lockfile : ""));
#endif

    switch ((int)fp) {
	case EOF:
	case NULL:
	    return (OK);
    }
    return(fclose (fp));
}
