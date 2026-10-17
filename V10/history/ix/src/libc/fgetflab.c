#include <sys/label.h>

/*
 * the fgetflab system call.
 */

fgetflab(fd, lp)
struct label *lp;
{
	return syscall(64+31, fd, lp);
}
