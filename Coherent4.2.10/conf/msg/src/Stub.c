/*
 * System V IPC - message system call stub.
 */

#include <sys/errno.h>

int
umsgsys ()
{
	set_user_error (ENOSYS);
}

