#include <sys/label.h>

/*
 * the setflab system call.
 */
setflab(name, lp)
char *name;
struct label *lp;
{
	return syscall(64+32, name, lp);
}
