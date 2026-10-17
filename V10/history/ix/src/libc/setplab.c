#include <sys/label.h>

/*
 * The setplab system call.
 */

setplab(lp, cp)
struct label *lp, *cp;
{
	return syscall(64+35, lp, cp);
}
