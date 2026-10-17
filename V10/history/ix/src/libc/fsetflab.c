#include <sys/label.h>

/*
 * the fsetflab system call.
 */

fsetflab(fd, lp) 
struct label *lp;
{
	return syscall(64+33, fd, lp);
}
