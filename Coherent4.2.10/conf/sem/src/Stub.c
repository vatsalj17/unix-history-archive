/*
 * System V IPC - semaphores system call stub.
 */

#include <sys/errno.h>

#define	_KERNEL		1

#include <sys/proc.h>

void
semAllAdjust (pp)
PROC	* pp;
{
}


int
usemsys ()
{
	set_user_error (ENOSYS);
}


