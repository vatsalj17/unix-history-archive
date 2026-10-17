/* $Header: /ker/io.386/RCS/shm1.c,v 2.3 93/08/19 04:03:17 nigel Exp Locker: nigel $ */
/*
 * System V Compatible Shared Memory Device Driver
 *
 * $Log:	shm1.c,v $
 * Revision 2.3  93/08/19  04:03:17  nigel
 * Nigel's R83
 * 
 * Revision 2.2  93/07/26  15:32:18  nigel
 * Nigel's R80
 * 
 * Revision 1.3  93/04/14  10:23:10  root
 * r75
 */

#include <kernel/_timers.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/errno.h>
#include <sys/cmn_err.h>

#include <sys/coherent.h>
#include <sys/types.h>
#include <sys/proc.h>
#include <limits.h>


#define	__SHM_READ	__IRUGO
#define	__SHM_WRITE	__IWUGO

#define SHMBASE	0x40000000	/* Base shared memory address */

extern SEG	*shmAlloc();	/* shm0.c */

/* Configurable variables - see /etc/conf/mtune. */

extern int	SHMMNI;
extern int 	SHMMAX;

struct shmid_ds	*shmids = NULL;		/* Array of shared memory segments */
SEG	     **	shmsegs;		/* Array of pointers to segments */

/*
 * Shmctl - Shared Memory Control Operations.
 */

int
ushmctl(shm_id, cmd, ushm_buf)
int 		shm_id,		/* Shared memory id */
		cmd;		/* Command */
struct shmid_ds	*ushm_buf;	/* User shmid_ds buffer */
{
	struct shmid_ds * shm_buf;	

	/* Check if id is in proper range. */
	if (! __IN_RANGE (0, shm_id, SHMMNI - 1)) {
		set_user_error (EINVAL);
		return -1;
	}

	/* Check we did alloc. All allocatable arrays are alloced after
	 * the first ~correct usage of shmget.
	 */

	if (shmids == NULL) {
		set_user_error (EINVAL);
		return -1;
	}
	shm_buf = shmids + shm_id;	/* Requested segment */

	/*
	 * Check if segment is in use. If the segment does not exist, refuse
	 * operations except for IPC_STAT from root.
	 */

	if ((shm_buf->shm_perm.mode & __IPC_ALLOC) == 0 &&
	    (cmd != IPC_STAT || ! super ())) {
		set_user_error (EINVAL);
		return -1;
	}

	switch (cmd) {
	case IPC_STAT:
		/* Check read permission for stat. */
		if (ipc_lack_perm (& shm_buf->shm_perm, __SHM_READ))
			return -1;

		/* Check if user gives a valid buffer */
		if (! useracc (ushm_buf, sizeof (struct shmid_ds), 1)) {
			set_user_error (EFAULT);
			return -1;
		}
		/* kucopy will set u_error if error occurs */
		kucopy (shm_buf, ushm_buf, sizeof(struct shmid_ds));
		break;

	case IPC_SET:
		if (ipc_cred_match (& shm_buf->shm_perm) != _CRED_OWNER) {
			set_user_error (EPERM);
			return -1;
		}

		shm_buf->shm_perm.uid = getuwd (& ushm_buf->shm_perm.uid);
		shm_buf->shm_perm.gid = getuwd (& ushm_buf->shm_perm.gid);
		shm_buf->shm_perm.mode &= ~0777;
		shm_buf->shm_perm.mode |= getuwd (& ushm_buf->shm_perm.mode) &
					 0777;
		break;

	case IPC_RMID:
		if (ipc_cred_match (& shm_buf->shm_perm) != _CRED_OWNER) {
			set_user_error (EPERM);
			return -1;
		}
		
		/* SVR3 allows removing an attached segment. Even worse, the
		 * process that has the segment attached can keep using it. 
		 * Some buggy third party software uses this "feature". 
		 * So, we have to make it available too;-(
		 */
		shm_buf->shm_perm.seq = 0;
		shm_buf->shm_perm.mode = 0;

		/* If segment is attached, set flag to be removed */
		if (shm_buf->shm_nattch > 0)
			shmsegs [shm_id]->s_flags |= SRFBERM;
		else if (shmsegs [shm_id] != NULL)
			shmFree (shmsegs [shm_id]); /* remove it otherwise */

		shmsegs [shm_id] = NULL;
		break;

	/* SHM_LOCK and SHM_UNLOCK: lock/unlock shared memory segement 
	 * in core. Is not a part of iBCS2.
	 * Has no meaning for current 4.0.* release of COHERENT.
	 * Have been done for binary compatibility.
	 */

	case SHM_LOCK:	
		if (! super ())
			return -1;
		break;

	case SHM_UNLOCK:	
		if (! super ())
			return -1;
		break;
	
	default:
		set_user_error (EINVAL);
		return -1;
	}

	return 0;
}


/*
 * Shmget - Get Shared Memory Segment
 * Return shared memory id if succed, -1 and set u_error otheriwse.
 */

int
ushmget(shm_key, shm_size, shm_flag)
key_t	shm_key;	/* Shared memory key */
int	shm_size;	/* Shared memory segment size */
int	shm_flag;	/* Flags */
{
	struct shmid_ds * oldest = 0;/* Oldest free segment */
	unsigned short	i;		/* Loop index */
	SEG	      *	seg;

	/* Check the requested segment size */
	if (! __IN_RANGE (0, shm_size, SHMMAX)) {
		set_user_error (EINVAL);
		return -1;
	}

	/* Init the shared memory on the first shmget. */
	if (shmids == NULL) 
		if (shminit ()) {
			set_user_error (ENOSPC);
			return -1;
		}

	/*
	 * Search for desired shared memory segment.
	 */

	for (i = 0; i < SHMMNI ; i++) {
		struct shmid_ds * shm_buf = shmids + i;

		/*
		 * If segment is not alloced, we will look for the oldest
		 * free segment. We will use it to create a new one.
		 */

		if ((shm_buf->shm_perm.mode & __IPC_ALLOC) == 0) {
			if (oldest == NULL || 
			    oldest->shm_ctime > shm_buf->shm_ctime)
				oldest = shm_buf;
			continue;
		}

		/*
		 * Match the segment key.
		 */

		if (shm_key == IPC_PRIVATE ||
		    shm_key != shm_buf->shm_perm.key)
			continue;

		/*
		 * Request for an exclusive segment should fail.
		 */

		if ((shm_flag & (IPC_CREAT | IPC_EXCL)) ==
		    (IPC_CREAT | IPC_EXCL)) {
			set_user_error (EEXIST);
			return -1;
		}

		/*
		 * Check the requested size and permissions
		 */

		if (shm_buf->shm_segsz < shm_size) {
			set_user_error (EINVAL);
			return -1;
		}

		if (ipc_lack_perm (& shm_buf->shm_perm, shm_flag))
			return -1;
		return i;
	}

	/* We need to create a new segment */
	if (oldest == NULL) {
		set_user_error (ENOSPC);
		return -1;
	}
	if ((shm_flag & IPC_CREAT) == 0) {
		set_user_error (ENOENT);
		return -1;
	}

	/*
	 * Allocate a new shared memory segment
	 */

	if ((seg = shmAlloc (shm_size)) == NULL) {
		set_user_error (ENOSPC);
		return -1;
	}

	/*
	 * Save it in shmsegs
	 */

	i = oldest - shmids;
	shmsegs [i] = seg;

	ipc_perm_init (& oldest->shm_perm, shm_flag);
	oldest->shm_perm.key  = shm_key;
	oldest->shm_perm.seq = i;
	oldest->shm_segsz = shm_size;
	oldest->shm_atime = 0;
	oldest->shm_dtime = 0;
	oldest->shm_ctime = posix_current_time ();
	oldest->shm_cpid  = SELF->p_pid;

	if (shm_key == IPC_PRIVATE)
		oldest->shm_perm.mode |= SHM_DEST;

	return i;
}


/*
 * Allocate space for shared memory data structures.
 */

int
shminit()
{
	shmids = (struct shmid_ds *) kalloc(sizeof(struct shmid_ds) * SHMMNI);
	if (shmids == NULL)
		return -1;

	/* Allocate array of shared memory segments. We do not have to
	 * initalise it
	 */
	shmsegs = (SEG **) kalloc(sizeof(SEG *) * SHMMNI);
	if (shmsegs == NULL) {
		kfree (shmids);
		shmids = NULL;
		return -1;
	}
	return 0;
}


#define	NO_OVERLAP	0
#define	CONTAINS_BASE	1
#define	CONTAINS_LIMIT	2
#define	CONTAINED	(CONTAINS_BASE + CONTAINS_LIMIT)


/*
 * For use below, simple interval calculation.
 */

#if	__USE_PROTO__
__LOCAL__ int seg_overlap (struct sr * srp, caddr_t base, size_t len)
#else
__LOCAL__ int
seg_overlap (srp, base, len)
struct sr     *	srp;
caddr_t		base;
size_t		len;
#endif
{
	size_t		temp;

	if (srp->sr_segp == 0)
		return 0;		/* no overlap */

	temp = base - srp->sr_base;
	return (temp < srp->sr_size ? CONTAINS_BASE : 0) +
		(temp + len <= srp->sr_size ? CONTAINS_LIMIT : 0);
}


/*
 * Check requested address for attach.
 * Just fail for the first release.
 */

#if	__USE_PROTO__
__LOCAL__ caddr_t check_request_address (caddr_t addr, size_t len)
#else
__LOCAL__ caddr_t
check_request_address (addr, len)
caddr_t		addr;	/* Address to atatch */
size_t		len;
#endif
{
	int		i;

	/*
	 * All we really need do is ensure that we are not going to collide
	 * with an existing segment. Clearly, our underlying segment-mangement
	 * system should have a primitive for this, and once demand paging is
	 * implemented it no doubt will.
	 *
	 * Of course, there are also addresses that we cannot use that are
	 * not visible in the segment list. For now, pretend we are on a VAX
	 * and limit ourself to [0, INT_MAX] bytes.
	 */

	if (addr > (caddr_t) INT_MAX)
		return -1;

	for (i = 0 ; i < NUSEG ; i ++)
		if (seg_overlap (& SELF->p_segl [i], addr, len) != NO_OVERLAP) {
			cmn_err (CE_NOTE, "attach [%x, %d] refused, conflict with seg %d",
				 (unsigned) addr, len, i);
			return -1;
		}

	for (i = 0 ; i < NSHMSEG ; i ++)
		if (seg_overlap (& SELF->p_shmsr [i], addr, len) != NO_OVERLAP) {
			cmn_err (CE_NOTE, "attach [%x, %d] refused, conflict with shm %d",
				 (unsigned) addr, len, i);
			return -1;
		}

	return 0;
}


/*
 * Attach shared memory segment.
 */

caddr_t
ushmat (sys_id, shm_addr, shm_flag)
int	sys_id;		/* System segment id */
caddr_t	shm_addr;	/* Address to attach */
int	shm_flag;	/* Flags */
{
	struct sr     *	srp;		/* Shared memory segments */
	SEG	      *	segp;	/* Segment to attach */
	struct shmid_ds	* shm_id;	/* Pointer to a system segment*/
	unsigned int	seg_id;		/* Segment id */
	caddr_t		vAttAddr;	/* Address to attach */
	int		i;		/* Loop index */
	int		read_only = 0;	/* 1 - read only, 0 - rw */
		
	/* Check if sys_id is a valid shared memory id. */
	if (! __IN_RANGE (0, sys_id, SHMMNI)) {
		set_user_error (EINVAL);
		return -1;
	}

	/* Do we really have this segment? */
	shm_id = shmids + sys_id;
	if (shm_id->shm_perm.seq != sys_id) {
		set_user_error (EINVAL);
		return -1;
	}

	/* Check permissions. */
	if (ipc_lack_perm (& shm_id->shm_perm,
			   (shm_flag & SHM_RDONLY) != 0 ? __SHM_READ :
							 __SHM_WRITE))
		return -1;

	/* We will go through all NSHMSEG segments to see if any is free. */
	for (srp = SELF->p_shmsr; srp < SELF->p_shmsr + NSHMSEG;
	     srp ++) {
		if (srp->sr_segp == NULL)
			break;
	}

	/* The segment id is just an element number. */
	seg_id = srp - SELF->p_shmsr;

	/* If segment id is >= NSHMSEG we cannot attach any new segment. */
	if (seg_id >= NSHMSEG) {
		set_user_error (EMFILE);
		return -1;
	}

	/* Get the pointer to the shared memory segment */
	segp = shmsegs [sys_id];

	/* Find an address to attach.
	 * There are two cases: process does not request the address,
	 * process requests the address.
	 * In the first case we have to take the first available free address.
	 * We will try to put a free page between the segments.
	 * In the second case we have to check if address is an available and
	 * attach to this address.
	 */

	/* First case. We have to find a free address. */
	if (shm_addr == NULL) {
		/* Find free space to attach.  */
		for (i = 0; i < NSHMSEG; i ++) {
			if (SELF->p_shmsr [i].sr_base == NULL)
				break;
		}

		/* Check limit of attaches per process */
		if (i >= NSHMSEG) {
			set_user_error (EINVAL);
			return;
		}

		/*
		 * We will use the addresses starting from SHMBASE. 
		 * Each new address can be SHMMAX + NBPC appart.
		 */

		shm_addr = (caddr_t) (SHMBASE + i * (SHMMAX + NBPC));
	} else if (check_request_address (shm_addr, shm_id->shm_segsz)) {
		cmn_err (CE_NOTE, "%s: attempt attach to 0x%x\n", SELF->p_comm,
			 shm_addr);
		set_user_error (EINVAL);
		return;
	}

	if (! shmAtt (seg_id, shm_addr, segp, read_only)) {
		set_user_error (EINVAL);
		return -1;
	}

	shm_id->shm_lpid = SELF->p_pid;
	shm_id->shm_atime = posix_current_time ();
	shm_id->shm_dtime = 0;
	shm_id->shm_nattch = segp->s_urefc - 1;

	/* Keep all attached addresses. We will need them for detach */
	return shm_addr;
}


/*
 * ushmdt() - Detach shared memory segment.
 * Find segment number and call shmDetach() (shm0.c).
 */
int
ushmdt(shm_addr)
char	*shm_addr;	/* Pointer to a segment */
{
	int		i;		/* Loop index */

	/* Go through all segments. */
	for (i = 0; i < NSHMSEG; i++)
		if (SELF->p_shmsr [i].sr_base == (caddr_t) shm_addr) {
 			shmDetach (i);
                        return 0;
		}

	/* We can come here only if we have invalid address */
	set_user_error (EINVAL);
	return -1;
}


/*
 * shmSetDs(). Called from shm0.c.
 *
 * Given a pointer to shared memory segment, set shmid_ds.
 */

void
shmSetDs(segp)
SEG	*segp;
{
	int		j;		/* Loop index */

	for (j = 0; j < SHMMNI; j++)
		if (shmsegs [j] == segp) {

			/* Set proper values */
			shmids [j].shm_lpid = SELF->p_pid;
			shmids [j].shm_dtime = posix_current_time ();
			shmids [j].shm_nattch = segp->s_urefc - 1;
			return;
		}

	/* We should have this segment. */
	set_user_error (EINVAL);
}


/*
 * Dispatch the top-level system call; under iBCS2, all the shm... () calls
 * are funneled through a single kernel entry point.
 */

ushmsys(func, arg1, arg2, arg3)
int func, arg1, arg2, arg3;
{
	switch (func) {
	case 0: return ushmat (arg1, arg2, arg3);
	case 1: return ushmctl (arg1, arg2, arg3);
	case 2: return ushmdt (arg1);
	case 3: return ushmget (arg1, arg2, arg3);
	default: set_user_error (EINVAL);
	}
}

