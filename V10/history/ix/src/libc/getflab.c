#include <sys/label.h>

/*
 * the getflab system call.
 */
getflab(name, lp)
char *name;
struct label *lp;
{
	return syscall(64+30, name, lp);
}
