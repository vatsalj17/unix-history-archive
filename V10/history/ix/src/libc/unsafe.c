#include <sys/types.h>

unsafe(n, r, w)
fd_set *r, *w;
{
	return syscall(64+36, n, r, w);
}
