/*
 * System V IPC - Shared memory system call stub, and stubs for routines and
 * variables referenced from shm0.c
 */

#include <sys/errno.h>

#define	_KERNEL		1

#include <sys/seg.h>

int		SHMMAX = 0;

/*
 * shmSetDs(). Called from shm0.c.
 *
 * Given a pointer to shared memory segment, set shmid_ds.
 */

void
shmSetDs(segp)
SEG	*segp;
{
	set_user_error (EINVAL);
}

int
ushmsys ()
{
	set_user_error (ENOSYS);
}

