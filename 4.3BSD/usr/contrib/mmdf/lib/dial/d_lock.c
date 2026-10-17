#include  "util.h"
#include  <sys/stat.h>
#ifdef V4_2BSD
#include  <sys/file.h>
#endif V4_2BSD
#include  "d_returns.h"

extern	int d_errno;

/*
 *      D_LOCK
 *
 *      routine which attempts to open a file exclusively, and thereby lock
 *      the associated port or direct line.
 *
 *   Feb 84   D. Long modified to stat the lock rather than d_verifying it
 *
 *   Mar 84   D. Rockwell & D. Long -- use flock instead of UUCP convention
 *            D. Rockwell & D. Long -- fixed bug with d_lckfd in d_lock.
 *
 *	Jul 85   D. Rockwell & D. Long -- add UUCP-compatible locking (yuck)
 *	         This assumes that the phone line is being locked with
 *	          "/usr/spool/uucp/LCK..tty*"
 *
 *      lockfile -- lock file name
 */
d_lock (lockfile)
register char  *lockfile;

{
    extern int  d_lckfd;

#ifdef UUCPLOCK
	char lockername[1024];
	int fd;
	int pid;

	d_lckfd = -1;
	pid = getpid();
	sprintf(lockername, "/usr/spool/uucp/MLTMP.%d", pid);
	(void) unlink(lockername);
	if ((fd = creat(lockername, 0644)) < 0) {
#ifdef D_LOG
		d_log ("d_lock", "can't create locker file '%s'", lockername);
#endif D_LOG                                
		(void) unlink(lockername);	/* never needed? */
		d_errno = D_LOCKERR;
		return(D_FATAL);
	}
	write(fd, &pid, sizeof pid);
	(void) close(fd);
	if (link(lockername, lockfile) < 0) {
#define	NEED_D_VERIFY	1
		if (d_verify(lockfile) >= D_OK) {
			(void) unlink(lockername);
			return(D_NO);
		}
		(void) unlink(lockfile);
		if (link(lockername, lockfile) < 0) {
#ifdef D_LOG
			d_log ("d_lock", "can't link lock file '%s'", lockfile);
#endif D_LOG
			(void) unlink(lockername);
			return(D_NO);
		}
	}
	(void) unlink(lockername);

#else UUCPLOCK
#ifndef V4_2BSD
#define	NEED_D_VERIFY	1
    if (d_verify (lockfile) >= D_OK)
	return (D_NO);      /* it's really busy */

    (void) unlink (lockfile);      /* clean it out, if necessary */
    if ((d_lckfd = creat (lockfile, 0644)) < 0)
    {
#else V4_2BSD
    if (d_lckfd < 0 && (d_lckfd = open (lockfile, O_RDONLY|O_CREAT, 0644)) < 0)
    {
#endif V4_2BSD
#ifdef D_LOG
	d_log ("d_lock", "can't create lock file '%s'", lockfile);
#endif D_LOG
	d_errno = D_LOCKERR;
	return (D_FATAL);
    }

#ifdef V4_2BSD
    if (flock(d_lckfd, LOCK_EX|LOCK_NB) < 0)
    {
	(void) close(d_lckfd);
	d_lckfd = -1;
	return (D_NO); 
    }
#endif V4_2BSD
#endif UUCPLOCK
    return (D_OK);
}

#ifdef	NEED_D_VERIFY
d_verify (port)         /* is the port REALLY active? */
register char  *port;
{
    struct stat statbuf;
    time_t curtime;

    if (stat (port, &statbuf) < 0)
	return (D_NO);  /* really is inaccessable */

    time (&curtime);

    if (curtime > (statbuf.st_atime + (time_t) (15 * 60)))
	return (D_NO);  /* inactive for over 15 minutes */

    return (D_OK);
}
#endif NEED_D_VERIFY

d_unlock(lockfile)
{
    if (d_lckfd >= 0)
    {
#ifdef V4_2BSD
	flock(d_lckfd, LOCK_UN);
#endif V4_2BSD
	(void) close(d_lckfd);
	d_lckfd = -1;
    }
    (void) unlink( lockfile );
}
