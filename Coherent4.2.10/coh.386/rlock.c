/* $Header: /ker/coh.386/RCS/rlock.c,v 2.7 93/10/29 00:55:32 nigel Exp Locker: nigel $ */
/*
 * COHERENT record locking.
 *
 * $Log:	rlock.c,v $
 * Revision 2.7  93/10/29  00:55:32  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.6  93/09/13  07:59:20  nigel
 * Updated to reflect the fact that most driver entry points are 'void' again.
 * 
 * Revision 2.5  93/08/19  10:37:30  nigel
 * r83 ioctl (), corefile, new headers
 * 
 * Revision 2.4  93/08/19  03:26:44  nigel
 * Nigel's r83 (Stylistic cleanup)
 * 
 * Modified 5/92 by steve from tsm's old source.
 * All record locking functions meet the system V standard.
 */

#include <kernel/_sleep.h>
#include <kernel/proc_lib.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>

#define	_KERNEL		1

#include <kernel/alloc.h>
#include <sys/fd.h>
#include <sys/proc.h>
#include <sys/sched.h>
#include <sys/inode.h>

/*
 * Internal record lock.
 * Active locks are attached to inode.
 */
typedef struct	rlock	{
	struct	rlock	*rl_next;	/* link, must be first		*/
	int		rl_type;	/* 0==read, 1==write		*/
	long		rl_start;
	long		rl_end;		/* -1 is EOF			*/
	PROC		*rl_proc;	/* owner's process		*/
} RLOCK;

/* Pending record lock. */
typedef struct	prlock	{
	struct	prlock	*prl_next;	/* link, must be first		*/
	RLOCK		*prl_rls;	/* active locks on desired inode */
	RLOCK		*prl_rl;	/* desired lock			*/
} PRLOCK;

/* Globals. */

RLOCK	*freerl;

__DUMB_GATE	__rlgate = __GATE_DECLARE ("global file-lock list");

#define	__GLOBAL_LOCK_RLOCK_LIST(where)	\
		(__GATE_LOCK (__rlgate, "lock : rlgate " where))
#define	__GLOBAL_UNLOCK_RLOCK_LIST() \
		(__GATE_UNLOCK (__rlgate))


extern unsigned int RLOCKS;		/* tunable */


/* Forward. */

int		rlock();
int		rlinit();
static	int	nextblock();
static	int	nextmeet();
static	RLOCK	*getlock();
static	RLOCK	*freelock();
static	int	extract();
static	int	remlock();
static	int	addlock();
static	int	waitlock();
static	int	waiting();

/*
 * Record locking.
 */
int
rlock (fdp, cmd, flp)
__fd_t	      *	fdp;
int		cmd;
struct flock  *	flp;
{
	RLOCK	*org;
	int	retval;
	RLOCK		srl;
	RLOCK		*rlp;

	/*
	 * NIGEL: There used to be a cast here, but what we are looking at
	 * here is code written by someone who didn't understand the C type
	 * system at all. This needs serious fixing; clearly, people are doing
	 * shit like depending on links magically being at offset 0 and stuff
	 * like that.
	 */

	org = & fdp->f_ip->i_rl;

	retval = -1;
	srl.rl_proc = SELF;
	srl.rl_type = flp->l_type==F_RDLCK ? 0:1;
	srl.rl_start = flp->l_start;
	if (flp->l_whence == SEEK_CUR)
		srl.rl_start += fdp->f_seek;
	else if (flp->l_whence == SEEK_END)
		srl.rl_start += fdp->f_ip->i_size;
	srl.rl_end = srl.rl_start;
	if (flp->l_len > 0)
		srl.rl_end += flp->l_len - 1;
	else if (flp->l_len == 0)
		srl.rl_end = -1;
	else {
		srl.rl_start += flp->l_len;
		srl.rl_end--;
	}
	if (srl.rl_start < 0 || srl.rl_end < -1 ||
	    (srl.rl_end != -1 && srl.rl_start > srl.rl_end)) {
		set_user_error (EINVAL);
		return -1;
	}

	__GLOBAL_LOCK_RLOCK_LIST ("rlock ()");

	rlp = org;
	if (cmd == F_GETLK) {
		if (! nextblock (& rlp, & srl))
			flp->l_type = F_UNLCK;
		else {
			flp->l_type = rlp->rl_type ? F_WRLCK:F_RDLCK;
			flp->l_whence = SEEK_SET;
			flp->l_start = rlp->rl_start;
			if (rlp->rl_end == -1)
				flp->l_len = 0;
			else
				flp->l_len = rlp->rl_end - flp->l_start + 1;
			flp->l_pid = rlp->rl_proc->p_pid;
		}
		retval = 0;
	} else if (flp->l_type == F_UNLCK)
		retval =  remlock (org, & srl);
	else if ((srl.rl_type && (fdp->f_flag & IPW) == 0) ||
		 (! srl.rl_type && (fdp->f_flag & IPR) == 0))
		set_user_error (EINVAL);
	else if (cmd == F_SETLKW)
		retval = waitlock(org, &srl);
	else if (!nextblock(&rlp, &srl))
		retval = addlock(org, &srl);
	else
		set_user_error (EACCES);

	__GLOBAL_UNLOCK_RLOCK_LIST ();

	if (cmd != F_GETLK)
		wakeup(org);
	return retval;
}

/* Allocate and initialize free record lock list. */

int
rlinit()
{
	RLOCK	*rl;

	if (RLOCKS == 0) {
		freerl = NULL;
		return 0;
	}
	if ((freerl = kalloc (RLOCKS * sizeof(RLOCK))) == NULL)
		panic("rlinit: no space for RLOCKs");

	rl = &freerl[RLOCKS-1];
	rl->rl_next = NULL;
	while (--rl >= freerl)
		rl->rl_next = rl+1;
	return 1;
}

/*
 * Record locking subroutines.
 * Most of them assume that rlgate is locked.
 * The user is guaranteed no unwanted unlocks regardless of error.
 * rlgate is locked whenever the lock list contents or free list is used.
 * Be careful if you change anything that these things remain true.
 */

/*
 * In nextblock and nextmeet,
 * list is a ptr to an RLOCK * which is owned by the caller.
 * This allows calls to proceed down an rlock list.
 * These routines should NOT be called with *list == NULL.
 */

/* Set *list = next element in list conflicting with lock. */

static int
nextblock(list, lock)
register RLOCK **list, *lock;
{
	while (nextmeet (list, lock)) {
		if (lock->rl_proc != (* list)->rl_proc &&
		    (lock->rl_type || (* list)->rl_type)) {
			return 1;
		}
	}
	return 0;
}


/* Set *list = next element of list referencing a loc referenced by lock */

static int
nextmeet(list, lock)
RLOCK **list, *lock;
{
	register RLOCK	*block;
	register long top, bot;

	top = lock->rl_start;
	bot = lock->rl_end;
	for (block = (*list)->rl_next; block != NULL; block = block->rl_next) {
		if (bot == -1) {
			if (block->rl_end == -1 || block->rl_end >= top)
				break;
		} else if (bot >= block->rl_start)
			if (block->rl_end == -1 || block->rl_end >= top)
				break;
	}
	if ((*list = block) != NULL)
		return 1;
	return 0;
}


/* Remove and return a lock from the free list. */
static RLOCK *
getlock()
{
	register RLOCK *rl;

	rl = freerl;
	if (rl != NULL)
		freerl = rl->rl_next;
	else
		set_user_error (ENOLCK);
	return rl;
}


/*
 * Remove element from the lock list beginning at org and add it to the free list.
 * Return pointer to (what used to be) its predecessor.
 */

static RLOCK *
freelock(org, elt) register RLOCK *org, *elt;
{
	while (elt != org->rl_next)
		org = org->rl_next;
	org->rl_next = elt->rl_next;
	elt->rl_next = freerl;
	freerl = elt;
	return org;
}

/*
 * Remove new lock from old. 
 * Returns:
 *	1 if caller should free old.
 *	0 if caller should do nothing.
 *	-1 if error (out of locks).
 */

static int
extract(old, new)
register RLOCK *old, *new;
{
	long		tend;
	register RLOCK	*lock;

	if (new->rl_start <= old->rl_start) {
		if (new->rl_end == -1 ||
		    (old->rl_end != -1 && new->rl_end >= old->rl_end))
			return 1;
		old->rl_start = new->rl_end+1;
		return 0;
	}

	tend = old->rl_end;
	old->rl_end = new->rl_start - 1;

	if (new->rl_end == -1 || (tend != -1 && new->rl_end >= tend))
		return 0;

	if ((lock = getlock()) == NULL) {
		old->rl_end = tend;
		return -1;
	}

	lock->rl_type = old->rl_type;
	lock->rl_proc = old->rl_proc;
	lock->rl_start = new->rl_end+1;
	lock->rl_end = tend;
	lock->rl_next = old->rl_next;
	old->rl_next = lock;
	return 0;
}


/* Extract lock from each element of list. */
static int
remlock(list, lock)
RLOCK *list; register RLOCK *lock;
{
	RLOCK *org;

	org = list;
	while (nextmeet (& list, lock)) {
		if (list->rl_proc != lock->rl_proc)
			continue;

		switch (extract (list, lock)) {
			case 1:
				list = freelock (org, list);
				break;

			case 0:
				break;

			default:
				return -1;
		}
	}
	return 0;
}


/*
 * Install lock in list.
 * Combine lock with neighbouring locks of same type and owner.
 * Get space for new lock (best done here - OK since rllock locked).
 * Remove old locks (always different type, same owner) and attach new lock.
 * To avoid removing desired locks, addlock allows redundant locks to appear.
 * This is always rectified by the future unlock and is rare.
 */

static int
addlock (list, lock)
RLOCK *list; register RLOCK *lock;
{
	RLOCK	*new, *org;
	register long top, bot;

	org = list;
	top = lock->rl_start;
	bot = lock->rl_end;
	if (top != 0)
		lock->rl_start --;
	if (bot != -1)
		lock->rl_end ++;

	while (nextmeet (& list, lock)) {
		if (list->rl_proc != lock->rl_proc ||
		    list->rl_type != lock->rl_type)
			continue;
		if (list->rl_start < top)
			top = list->rl_start;

		if (bot != -1 && (list->rl_end == -1 || list->rl_end >= bot))
			bot = list->rl_end;
		list = freelock (org, list);
	}

	if ((new = getlock()) == NULL)
		return -1;

	new->rl_proc = lock->rl_proc;
	new->rl_type = lock->rl_type;
	lock = new;
	lock->rl_start = top;
	lock->rl_end = bot;
	remlock (org, lock);
	lock->rl_next = org->rl_next;
	org->rl_next = lock;
	return 0;
}


/* Sleep until possible to install rl in list then do so. */
#define cleanup()  SELF->p_prl = NULL;  rl->rl_next = freerl;  freerl = rl

static int
waitlock(list, srl)
RLOCK *list, *srl;
{
	register RLOCK *org;

	org = list;

	if (nextblock (& list, srl)) {
		register RLOCK * rl = getlock ();
	
		if (rl == NULL)
			return -1;
		* rl = * srl;
		rl->rl_next = org;
		SELF->p_prl = rl;
		do {
			__sleep_t	sleep;

			do {
				if (waiting (list->rl_proc)) {
					set_user_error (EDEADLK);
					cleanup ();
					return -1;
				}
			} while (nextblock(&list, rl));

			__GLOBAL_UNLOCK_RLOCK_LIST ();

			sleep = x_sleep (org, primed, slpriSigCatch, "rlock");

			__GLOBAL_LOCK_RLOCK_LIST ("waitlock ()");

			if (sleep == PROCESS_SIGNALLED) {
				set_user_error (EINTR);
				cleanup();
				return -1;
			}
			list = org;
		} while (nextblock (& list, rl));
		cleanup ();
	}
	return addlock(org, srl);
}


/* True if pp has requested a lock (even indirectly) pending on SELF. */
static int
waiting(pp)
PROC *pp;
{
	RLOCK		*list;
	register RLOCK	*lock;

	lock = pp->p_prl;
	if (lock == NULL)
		return 0;
	list = lock->rl_next;
	while (nextblock (& list, lock)) {
		if (list->rl_proc == SELF)
			return 1;
		if (waiting (list->rl_proc))
			return 1;
	}
	return 0;
}
