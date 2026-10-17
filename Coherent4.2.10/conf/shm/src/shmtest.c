#include <sys/compat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <stddef.h>

#define	TEST_KEY	((key_t) 1)

#if	USE_PROTO
void showperm (struct ipc_perm * perm)
#else
void
showperm (perm)
struct ipc_perm * perm;
#endif
{
	printf ("creator uid %d gid %d owner uid %d gid %d mode #%x"
		" seq %d key %d\n", perm->cuid, perm->cgid, perm->uid,
		perm->gid, perm->mode, perm->seq, perm->key);
}


#if	USE_PROTO
void shmstat (int shmid)
#else
void
shmstat (shmid)
int		shmid;
#endif
{
	struct shmid_ds	buf;

	if (shmctl (shmid, IPC_STAT, & buf) == -1) {
		perror ("shmtest: cannot stat");
		return;
	}

	printf ("ID %d  size %d  last pid %d  creator %d  attach count %d\n",
		shmid, buf.shm_segsz, buf.shm_lpid, buf.shm_cpid,
		buf.shm_nattch);

	showperm (& buf.shm_perm);
}


#if	USE_PROTO
int shmserver (void)
#else
int
shmserver ()
#endif
{
	int		shmseg;
	char	      *	addr;

	if ((shmseg = shmget (TEST_KEY, 1024, 0666 | IPC_CREAT)) == -1) {
		perror ("shmtest: cannot create shared memory segment");
		return -1;
	}

	printf ("Segment = %d\n", shmseg);

	if ((addr = shmat (shmseg, 0x64000000UL, 0)) == (VOID *) -1) {
		perror ("shmtest: could not attach segment");
		return -1;
	}

	shmstat (shmseg);

	* (char *) addr = 0;

	for (;;) {
		poll (NULL, 0, 1000);

		switch (* addr) {
		case 0:
			continue;

		case 1:
			write (STDOUT_FILENO, addr + 2, addr [1]);
			addr [0] = 0;
			continue;

		default:
			break;
		}
		break;
	}

	if (shmctl (shmseg, IPC_RMID) == -1) {
		perror ("shmtest: cannot remove id");
		return -1;
	}

	return 0;
}


#if	USE_PROTO
int shmclient (void)
#else
int
shmclient ()
#endif
{
	int		shmseg;
	char	      *	addr;

	if ((shmseg = shmget (TEST_KEY, 1024, 0666)) == -1) {
		perror ("shmtest: cannot access shared memory segment");
		return -1;
	}

	printf ("Segment = %d\n", shmseg);

	if ((addr = shmat (shmseg, 0x60000000UL, 0)) == NULL) {
		perror ("shmtest: count not attach segment");
		return -1;
	}

	while (* addr != 0)
		poll (NULL, 0, 500);

	strcpy (addr + 2, "Hi there");
	addr [1] = strlen (addr + 2);
	* addr = 1;

	while (* addr != 0)
		poll (NULL, 0, 500);

	* addr = 2;
	return 0;
}


#if	USE_PROTO
int main (int argc, char * CONST * argv)
#else
int
main (argc, argv)
int		argc;
char  * CONST *	argv;
#endif
{
	if (argc == 1)
		shmserver ();
	else
		shmclient ();
	return 0;
}

