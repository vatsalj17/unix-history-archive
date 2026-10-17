#include <sys/label.h>

/*
 * The getplab system call.
 */

getplab(lp, cp)
struct label *lp, *cp;
{
	return syscall(64+34, lp, cp);
}
