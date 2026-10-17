/*
 * file descriptor driver
 * open(minor n) -> dup of minor n
 */

#include "sys/param.h"
#include "sys/file.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/conf.h"
#include "sys/inode.h"

int fdopen();

daddr_t devfd0dev = makedev(40, 0);
struct cdevsw fdcdev = cdinit(fdopen, nodev, nodev, nodev, nodev);

fdopen(dev, flag)
{
	register unsigned fd;
	register struct fildes *fr;

	varused(flag);
	fd = minor(dev);
	if(fd >= NOFILE || (fr = u.u_ofile[fd]) == NULL) {
		u.u_error = EBADF;
		return;
	}
	/*
	 * Now for the slimy part: do what dup() does.
	 * This routine is ultimately called from openi()
	 * via fsopen(), where ERANGE is recognized & cleared.
	 * The error value prevents open1() from making a file
	 * descriptor for the ORIGINAL inode, as well.
	 */
	u.u_r.r_val1 = f_dup(-1, fr, SAFETOWRITE|SAFETOREAD);
	if(u.u_error == 0)
		u.u_error = ERANGE;
}
