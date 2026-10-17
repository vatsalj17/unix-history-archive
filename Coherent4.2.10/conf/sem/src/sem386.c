/* $Header: /ker/io.386/RCS/sem386.c,v 2.3 93/08/19 04:03:14 nigel Exp Locker: nigel $ */
/*
 * This module provides System V compatible semaphore operations.
 *
 * $Log:	sem386.c,v $
 * Revision 2.3  93/08/19  04:03:14  nigel
 * Nigel's R83
 * 
 * Revision 2.2  93/07/26  15:32:16  nigel
 * Nigel's R80
 * 
 * Revision 1.1  93/04/09  08:48:23  bin
 * Initial revision
 */

#include <common/_tricks.h>
#include <kernel/_timers.h>
#include <sys/coherent.h>
#include <sys/sched.h>
#include <sys/types.h>
#include <sys/uproc.h>
#include <sys/proc.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/con.h>

#include <sys/ipc.h>
#include <sys/sem.h>

#define	__SEM_READ	__IRUGO
#define	__SEM_WRITE	__IWUGO

int	iSubAdj ();	/* Subtract value from the sem adjust */

/* Configurable variables */

extern unsigned	SEMMNI;	/* max # of the semaphore sets, systemwide */
extern unsigned	SEMMNS;	/* max # of semaphores, systemwide */
extern unsigned	SEMMSL;	/* max # of semaphores per set */
extern unsigned	SEMVMX;	/* max value of any semaphore */
	
struct semid_ds	* semids = NULL; /* Array of semaphore sets */
int		sem_total = 0;	/* Total number of semaphores, systemwide */

__DUMB_GATE		__sem_gate = __GATE_DECLARE ("semaphores");

#define	__GLOBAL_LOCK_SEMAPHORES(where) \
		(__GATE_LOCK (__sem_gate, "lock : semaphore " where))
#define	__GLOBAL_UNLOCK_SEMAPHORES() \
		(__GATE_UNLOCK (__sem_gate))

/*
 * Clear adjust value for all process.
 */
void	vClearAdj (sem_id, sem_num)
int	sem_id;		/* Semaphore set id */
int	sem_num;	/* Semaphore number */
{
	PROC		* pp;		/* process */
	struct sem_undo	* unNext; 	/* sem_undo structures */

	/* Go through all processes and zero the proper undo entry */
	for (pp = & procq; (pp = pp->p_nforw) != & procq; ) {
		/* Look if adjust value for semaphore was set. */
		for (unNext = pp->p_semu; unNext != NULL; 
		     unNext = unNext->un_np) {
			if (unNext->un_num == sem_num &&
			    unNext->un_id == sem_id)
				unNext->un_aoe = 0;	/* Found */
		}
	}		
}


/*
 * Semget gets set of semaphores. Returns semaphore set id on suuccess,
 * or sets get_user_error () on error.
 */

usemget (skey, nsems, semflg)
key_t 	skey;		/* Semaphore key */
int 	nsems,		/* # of semaphores in the set */
	semflg;		/* Permission flag */
{
	struct semid_ds	* semidp;	/* Semaphore set */
	struct sem    *	semp;		/* Semaphores */
	struct semid_ds	* freeidp = 0;	/* Oldest free set */

	/* Check if we are inside system limits. */
	if (! __IN_RANGE (0, nsems, SEMMSL - 1)) {
		set_user_error (EINVAL);
		return -1;
	}


	/* Allocate memmory on the first semget. This memory (~ 300 bytes)
	 * will stay alloc up to the next reboot. The alloced unused memory
	 * is smaller than code that will allow to manage it more sophisticated.
	 * Allocaton is used to allow patchability of the semaphores. 
	 */
	if (semids == NULL && seminit ()) {
		set_user_error (ENOSPC);
		return;
	}

	__GLOBAL_LOCK_SEMAPHORES ("usemget ()");

	/* Now we will go through all semaphores. */
	for (semidp = semids; semidp < semids + SEMMNI; semidp ++) {
		/* If set is free look for the oldest */
		if ((semidp->sem_perm.mode & __IPC_ALLOC) == 0) {
			if ((freeidp == 0) ||
			    (freeidp->sem_ctime > semidp->sem_ctime))
				freeidp = semidp;
			continue;
		}

		/* Check if a request was for the private set */
		if (skey == IPC_PRIVATE)
			continue;

		if (skey != semidp->sem_perm.key)
			continue; 

		/* Found */

		if ((semflg & (IPC_CREAT | IPC_EXCL)) ==
		    (IPC_CREAT | IPC_EXCL))
			set_user_error (EEXIST);
		else if (ipc_lack_perm (& semidp->sem_perm, semflg))
			/* DO NOTHING */ ;
		else if (semidp->sem_nsems < nsems)
			set_user_error (EINVAL);

		/* Semaphore set id number is the number of an array element */
		__GLOBAL_UNLOCK_SEMAPHORES ();

		if (get_user_error ())
			return -1;
		return semidp - semids;
	}

	/* Set does not exist. So, we have to create it */

	if (nsems < 1) {
		__GLOBAL_UNLOCK_SEMAPHORES ();
		set_user_error (EINVAL);
		return;
	}

	/* Check the total number of semaphores */
	if (sem_total + nsems > SEMMNS) {
		__GLOBAL_UNLOCK_SEMAPHORES ();
		set_user_error (ENOSPC);
		return;
	}

	/* Check if there is the request for creation */
	if ((semflg & IPC_CREAT) == 0) {
		__GLOBAL_UNLOCK_SEMAPHORES ();
		set_user_error (ENOENT);
		return;
	}

	/* Out of system limits */
	if (freeidp == 0) {
		__GLOBAL_UNLOCK_SEMAPHORES ();
		set_user_error (ENOSPC);
		return;
	}

	/* Now we have to creat a new set. freeidp points to the oldest free
	 * set which we will use.
	 */
	semidp = freeidp;
	/* Get space for semaphores */
	semidp->sem_base = kalloc (nsems * sizeof (struct sem));
	if (semidp->sem_base == 0) {
		__GLOBAL_UNLOCK_SEMAPHORES ();
		set_user_error (ENOSPC);
		return;
	}

	/* Initialize created set */
	ipc_perm_init (& semidp->sem_perm, semflg);
	semidp->sem_perm.key = skey;
	semidp->sem_perm.seq = semidp - semids;
	
	semidp->sem_nsems = nsems;
	semidp->sem_otime = 0;
	semidp->sem_ctime = posix_current_time ();

	/* Increment number of semaphores in used */
	sem_total += nsems;

	/* Set values of the semaphores to 0.
	 * SVR3 does not do it and suggests set up sem struct using
	 * semctl () call. SVR4 manual says nothing about it.
	 */
	for (semp = semidp->sem_base; semp < semidp->sem_base + nsems; semp ++) {
		semp->semval = semp->sempid = semp->semncnt = semp->semzcnt = 0;
	}

	__GLOBAL_UNLOCK_SEMAPHORES ();
	return semidp - semids;
}

/*
 * Semctl provides semaphore control operation as specify by cmd.
 * On success return value depends on cmd, sets get_user_error () on error.
 */
union semun {
	int		val;	/* Used for SETVAL only */
	struct semid_ds * buf;	/* Used for IPC_STAT and IPC_SET */
	unsigned short 	* array;/* Used for IPC_GET_ALL and IPC_SETALL */
};

usemctl (semid, semnum, cmd, arg)
int	semid,		/* Semaphore set id */
	cmd;		/* Command */
int	semnum;		/* Semaphore # */
union semun arg;
{
	struct semid_ds	* semidp;	/* Semaphore set */
	struct sem    *	semp;		/* Semaphore */
	int 		val;		/* Semaphore value */
	int		i;		/* Loopindex */
	unsigned short * user_array;	/* User array */
	struct ipc_perm	temp;		/* temp for user data in IPC_SET */

	/* Check if we are inside system limits. */
	if (! __IN_RANGE (0, semnum, SEMMSL - 1)) {
		set_user_error (EINVAL);
		return -1;
	}


	/* Check if there was any successfull semget before.
	 * Problem may be if somebody does semctl () before semids was
	 * alloced.
	 */

	if (semids == NULL) {
		set_user_error (EINVAL);
		return;
	}

	/* semid cannot be < 0 and more than systemwide limit */
	if (! __IN_RANGE (0, semid, SEMMNI - 1)) {
		set_user_error (EINVAL);
		return;
	}
	semidp = semids + semid;
	
	/*
	 * Check if the requested set is alloced, unless this is an IPC_STAT
	 * coming from root.
	 */

	if ((semidp->sem_perm.mode & __IPC_ALLOC) == 0 &&
	    (cmd != IPC_STAT || ! super ())) {
		set_user_error (EINVAL);
		return;
	}

	/* Check if semnum is a correct semaphore number.
	 * SV would do it only when there is request for a
	 * single semaphore value, as GETVAL or SETVAL.
	 */
	if (semnum >= semidp->sem_nsems) {
		set_user_error (EFBIG);
		return;
	}

	semp = semidp->sem_base + semnum;

	switch (cmd) {
	case GETVAL:		/* Return value of semval */
		if (ipc_lack_perm (& semidp->sem_perm, __SEM_READ))
			return;
		return semp->semval;

	case SETVAL:	/* Set semval. Clear all semadj values on success. */
		if (ipc_lack_perm (& semidp->sem_perm, __SEM_WRITE))
			return;

		if (! __IN_RANGE (0, arg.val, SEMVMX)) {
			set_user_error (ERANGE);
			return;
		}

		/* Set semval and wakeup whatever should be */
		if (semp->semval = arg.val) {
			if (semp->semncnt)
				wakeup (& semp->semncnt);
		} else {
			if (semp->semzcnt)
				wakeup (& semp->semzcnt);
		}
		semidp->sem_ctime = posix_current_time ();
		/* Clear corresponding adjust value in all processes */
		vClearAdj (semid, semnum);
		return 0;

	case GETPID:		/* Return value of sempid */
		if (ipc_lack_perm (& semidp->sem_perm, __SEM_READ))
			return;
		return semp->sempid;

	case GETNCNT:		/* Return value of semncnt */
		if (ipc_lack_perm (& semidp->sem_perm, __SEM_READ))
			return;
		return semp->semncnt;

	case GETZCNT:		/* Return value of semzcnt */
		if (ipc_lack_perm (& semidp->sem_perm, __SEM_READ))
			return;
		return semp->semzcnt;

	case GETALL:		/* Return semvals array */
		if (ipc_lack_perm (& semidp->sem_perm, __SEM_READ))
			return;

		/* Copy all values to user array */
		semp  = semidp->sem_base;
		user_array = arg.array;
		for (i = 0; i < semidp->sem_nsems; i ++) {
			putusd (user_array, semp->semval);
			if (get_user_error ())
				return;
			semp ++;
			user_array ++;
		}
		return 0;

	case SETALL:		/* Set semvals array */
		if (ipc_lack_perm (& semidp->sem_perm, __IWRITE))
			return;

		/* Set semvals accoding to the arg.array */
		semp  = semidp->sem_base;
		user_array = arg.array;

		__GLOBAL_LOCK_SEMAPHORES ("usemctl () SETALL");

		for (i = 0; i < semidp->sem_nsems; i ++) {
			val = getusd (user_array ++);
			if (get_user_error ()) {
				__GLOBAL_UNLOCK_SEMAPHORES ();
				return;
			}
			if (! __IN_RANGE (0, val, SEMVMX)) {
				set_user_error (ERANGE);
				__GLOBAL_UNLOCK_SEMAPHORES ();
				return;
			}
			semp->semval = val;
			/* Clear corresponding adjust value in all processes. */
			vClearAdj (semid, i);
			semp ++;
		}
		semidp->sem_ctime = posix_current_time ();
		__GLOBAL_UNLOCK_SEMAPHORES ();
		return 0;

	case IPC_STAT:
		if (ipc_lack_perm (& semidp->sem_perm, __SEM_READ))
			return;

		kucopy (semidp, arg.buf, sizeof (struct semid_ds));
		return 0;

	case IPC_SET:
		if (ipc_lack_perm (& semidp->sem_perm, __IREAD))
			return;

		semidp->sem_perm.uid = getusd (& arg.buf->sem_perm.uid);
		semidp->sem_perm.gid = getusd (& arg.buf->sem_perm.gid);
		semidp->sem_perm.mode  =
			(getusd (& arg.buf->sem_perm.mode) & 0777) | __IPC_ALLOC;
		semidp->sem_ctime = posix_current_time ();
		return 0;

	case IPC_RMID:
		if (ipc_cred_match (& semidp->sem_perm) != _CRED_OWNER) {
			set_user_error (EPERM);
			return;
		}

		/* We have to wake up all waiting proccesses  */
		for (semp = semidp->sem_base; semp < semidp->sem_base +
				semidp->sem_nsems; semp ++) {
			if (semp->semncnt)
				wakeup (& semp->semncnt);
			if (semp->semzcnt)
				wakeup (& semp->semzcnt);
		}

		/* We do not cleane up adjust values here */
		sem_total -= semidp->sem_nsems;
		semidp->sem_perm.mode = 0;
		kfree (semidp->sem_base);
		return 0;

	default:
		set_user_error (EINVAL);
		return;
	}
}

/*
 * Semop - Semaphore Operations.
 */

usemop (sem_id, pstSops, uNsops)
int		sem_id;		/* Semaphore identifier */
struct sembuf	* pstSops;	/* Array of sem. operations */
unsigned 	uNsops;		/* # of elements in the array */
{
	struct semid_ds	* sem_set;	/* Semaphore set */
	struct sem    *	semp;	/* Semaphore */
	struct sembuf *	sem_buf;	/* Operations */
	unsigned short 	sem_num; 	/* Semaphore number */
	short		sem_flag;	/* Semaphore flag */
	short 		sem_oper; 	/* Operation */
	int		i;		/* Loop index */
	short		change = 0;	/* Sem was changed */

	/* Check if semids was alloced */
	if (semids == 0) {
		set_user_error (EINVAL);
		return;
	}

	/* sem_id cannot be < 0 and more than systemwide limit */
	if (! __IN_RANGE (0, sem_id, SEMMNI - 1)) {
		set_user_error (EINVAL);
		return;
	}
	if (! useracc (pstSops, sizeof (struct sembuf) * uNsops, 0) ||
	    uNsops < 1) {
		set_user_error (EFAULT);
		return;
	}

	sem_set = semids + sem_id;	/* Requested set */

	if ((sem_set->sem_perm.mode & __IPC_ALLOC) == 0) {
		set_user_error (EINVAL);
		return -1;
	}

TRY_AGAIN:	/* Repeat the semaphore set */

	__GLOBAL_LOCK_SEMAPHORES ("usemop ()");

	/* do semaphore ops  */
	for (i = 0, sem_buf = pstSops; i < uNsops; i ++, sem_buf ++) {
		sem_num = getusd (& sem_buf->sem_num);
		sem_oper = getusd (& sem_buf->sem_op);
		sem_flag = getusd (& sem_buf->sem_flg);

		if (! __IN_RANGE (- SEMVMX, sem_oper, SEMVMX)) {
			semundo (sem_set, pstSops, i);
			set_user_error (E2BIG);
			__GLOBAL_UNLOCK_SEMAPHORES ();
			return;
		}
		if (get_user_error () != 0 ||
		    sem_num >= sem_set->sem_nsems) {
			/* We have falure here. undo all previous 
			 * operations.
			 */
			semundo (sem_set, pstSops, i);
			/* If u_error was not set it means that sem_number 
			 * is bad. So, set error to EFBIG.
			 */
			if (get_user_error () == 0)
				set_user_error (EFBIG);
			__GLOBAL_UNLOCK_SEMAPHORES ();
			return;
		}

		/* Go to proper semaphore */
		semp = sem_set->sem_base + sem_num;
	
		/* We can have 3 different cases: sem_oper < 0,
		 * sem_oper == 0, & sem_oper > 0.
		 */
		if (sem_oper < 0) {	/* want to decrement semval */
			/* We do not have to undo anything. If we cannot alter
			 * we did not change any value in the set. But we
			 * have to check here because other requests could be
			 * for read (sem_oper = 0)
			 */	
			if (ipc_lack_perm (& sem_set->sem_perm,
					  __IWRITE)) { /* cannot alter */
				__GLOBAL_UNLOCK_SEMAPHORES ();
				return;
			}

			/* If we can decrement semval, do it. If
			 * semval becomes 0 wakeup all processes
			 * waiting for semval == 0.
			 */
			if (semp->semval >= - sem_oper) {
				if ((semp->semval += sem_oper) == 0 &&
		 		    semp->semzcnt != 0)
					wakeup (& semp->semzcnt);
				if ((sem_flag & SEM_UNDO) != 0 &&
				    iSubAdj (sem_id, sem_num, sem_oper)) {
					semundo (sem_set, pstSops, i);
					__GLOBAL_UNLOCK_SEMAPHORES ();
					return;
				}

				change ++;
				continue;
			}

			/* Can't decrement. */
			semundo (sem_set, pstSops, i);
			if (sem_flag & IPC_NOWAIT) {
				if (get_user_error () == 0)
					set_user_error (EAGAIN);
				__GLOBAL_UNLOCK_SEMAPHORES ();
				return;
			}

			__GLOBAL_UNLOCK_SEMAPHORES ();

			if (semwait (sem_id, & semp->semncnt) < 0)
				return;

			/* Now we can retry semaphore set */
			goto TRY_AGAIN;
		} 

		if (sem_oper == 0) {
			if (ipc_lack_perm (& sem_set->sem_perm,
					   __IREAD)) { /* cannot read */
				/* If somebody cannot read it does not
				 * mean that one couldn't alter.
				 */

				if (ipc_lack_perm (& sem_set->sem_perm,
						   __IWRITE))
					semundo (sem_set, pstSops, i);

				__GLOBAL_UNLOCK_SEMAPHORES ();
				return;
			}
			if (semp->semval == 0)
				continue;

			/* Semaphore value isn't 0. Undo all previous
			 * operations.
			 */
			semundo (sem_set, pstSops, i);

			if (sem_flag & IPC_NOWAIT) {
				if (get_user_error () == 0)
					set_user_error (EAGAIN);
				__GLOBAL_UNLOCK_SEMAPHORES ();
				return;
			}

			__GLOBAL_UNLOCK_SEMAPHORES ();
			if (semwait (sem_id, & semp->semzcnt) < 0)
				return;

			goto TRY_AGAIN;
		} 

		if (sem_oper > 0) {
			if (ipc_lack_perm (& sem_set->sem_perm,
					  __IWRITE)) { /* cannot alter */
				__GLOBAL_UNLOCK_SEMAPHORES ();
				return;
			}
			if ((sem_flag & SEM_UNDO) != 0 &&
			    iSubAdj (sem_id, sem_num, sem_oper)) {
				semundo (sem_set, pstSops, i);
				__GLOBAL_UNLOCK_SEMAPHORES ();
				return;
			}

			if (semp->semval > SEMVMX - sem_oper) {
				semundo (sem_set, pstSops, i);
				/* semundo would not adjust current semaphore */
				if (sem_flag & SEM_UNDO) {
					iSubAdj (sem_id, sem_num, -sem_oper);
				}
				set_user_error (ERANGE);
				__GLOBAL_UNLOCK_SEMAPHORES ();
				return;
			}
			semp->semval += sem_oper;

			/* Wake up waiting processes */
			if (semp->semncnt)
				wakeup (& semp->semncnt);
			change ++;
			continue;
		}
	}
	sem_set->sem_otime = posix_current_time ();	/* adjust semop time */
	if (change) /* Semaphore was changed */
		sem_set->sem_ctime = sem_set->sem_otime;
	
	/* Go through all set again and set pid of last semop */
	for (i = 0, sem_buf = pstSops; i < uNsops; i ++, sem_buf ++) {
		sem_num = getusd (& sem_buf->sem_num);
		semp = sem_set->sem_base + sem_num;
		semp->sempid = SELF->p_pid;
	}

	__GLOBAL_UNLOCK_SEMAPHORES ();
	return 0;				/* return last prev semval */
}

/*
 * Wait for an event.
 */
semwait (sem_id, sleep_event)
int		sem_id;		/* Semaphore set id */
unsigned short	* sleep_event;	/* Could be semcnt or semzcnt */
{
	struct semid_ds * sem_set;	/* Semaphore set */
	int		signalled;

	(* sleep_event) ++;
	
	sem_set = semids + sem_id;
	
	signalled = x_sleep (sleep_event, pritty, slpriSigCatch, "semwait") ==
		    PROCESS_SIGNALLED;

	if ((sem_set->sem_perm.mode & __IPC_ALLOC) == 0) {	/* Semaphore gone */
		set_user_error (EIDRM);
		return -1;
	}
	(* sleep_event) --;

	if (signalled) {	/* Signal received */
		set_user_error (EINTR);
		return -1;
	}
	return 0;
}
	
/*
 * Undo a Semaphore Operation. It should undo an adjust values too.
 */
semundo (sem_set, semOp, undo)
struct semid_ds	* sem_set;	/* Pointer to the semaphore set */
struct sembuf	* semOp;	/* Pointer to the undo operation */
int		undo;		/* Number of semaphores to undo */
{
	struct sem    *	semp;		/* */
	struct sembuf *	sem_buf;	/* */
	int		i;		/* Loop index */
	int		sem_id;		/* Semaphore id */
	unsigned short	sem_num;	/* Semaphore number */
	short		sem_oper;	/* Value to undo */
	short		sem_flag;	/* Semaphore flag */

	semp = sem_set->sem_base;
	sem_buf = semOp;
	sem_id = sem_set - semids;

	for (i = 0; i < undo; i ++) {
		sem_num = getusd (& sem_buf->sem_num);
		sem_oper  = getusd (& sem_buf->sem_op);
		sem_flag  = getusd (& sem_buf->sem_flg);
		semp->semval -= sem_oper;

		if (sem_flag & SEM_UNDO)
			iSubAdj (sem_id, sem_num, - sem_oper);

		sem_buf ++;
		semp ++;
	}		
}


/*
 * Allocate and clear space for semapohore sets
 * Return 0 on success, -1 and set errno on error.
 */

seminit ()
{
	unsigned	uSize;		/* Size of the alloc memmory */

	uSize = sizeof (struct semid_ds) * SEMMNI;

	if ((semids = (struct semid_ds *) kalloc (uSize)) == NULL)
		return -1;
	memset ((char *) semids, 0, uSize);
	return 0;
}


/*
 * Subtract iValue from the adjust value for the specified
 * semaphore for the specified process.
 */
int iSubAdj (sem_id, sem_num, sValue)
int		sem_id;		/* Semaphore set id */
unsigned short	sem_num;	/* Semaphore number */
short		sValue;		/* Adjust value */
{
	struct sem_undo	* unPrev, 	/* Previous and next pointers to */
			* unNext;	/* sem_undo structures link list */
	int		newAdjust;	/* New adjust value */

	/* Look if adjust value for semaphore was set */
	for (unNext = SELF->p_semu; unNext != NULL; unNext = unPrev->un_np) {
		if (unNext->un_num == sem_num && unNext->un_id == sem_id) {
			newAdjust = unNext->un_aoe - sValue;
			if (! __IN_RANGE (- SEMVMX, newAdjust, SEMVMX)) {
				set_user_error (ERANGE);
				return -1;
			}
			unNext->un_aoe = newAdjust; /* Found and adjust */
			return 0;
		}
		unPrev = unNext;
	}

	/* There is no adjust value for semaphore.
	 * We have to allocate space and creat a new adjust value.
	 */
	if ((unNext = kalloc (sizeof (struct sem_undo))) == NULL) {
		set_user_error (ENOSPC);
		return -1;
	}
	/* Set values for the next entry */
	unNext->un_np = NULL;
	unNext->un_aoe -= sValue;
	unNext->un_num = sem_num;
	unNext->un_id = sem_id;
	
	/* Put new entry at the end of link list */
	if (SELF->p_semu == NULL)
		SELF->p_semu = unNext;	/* New link list */
	else				/* Add entry to the existing list */
		unPrev->un_np = unNext;	/* unPrev is a last entry in the list */
	return 0;
}

/*
 * Adjust all semaphores and remove sem_undo link list.
 */

void
semAllAdjust (pp)
PROC	* pp;
{
	struct sem_undo	* unPrev, 	/* Previous and next pointers to */
			* unNext;	/* sem_undo structures link list */
	struct semid_ds	* sem_set;	/* Semaphore set */
	struct sem	* sem;	/* Semaphore */

	if ((unNext = pp->p_semu) == NULL)
		return;

	while (unNext != NULL) {
		sem_set = semids + unNext->un_id;	/* Requested set */
		sem = sem_set->sem_base + unNext->un_num;
		sem->semval += unNext->un_aoe;
		sem->sempid = SELF->p_pid;
		sem_set->sem_ctime = posix_current_time ();
		unPrev = unNext;
		unNext = unPrev->un_np;
		kfree (unPrev);
	}
	/* kfree (pp->p_semu);*/
	pp->p_semu = NULL;
}


/*
 * Dispatch the top-level system call; under iBCS2, all the sem... () calls
 * are funneled through a single kernel entry point.
 */

usemsys(func, arg1, arg2, arg3, arg4)
int func, arg1, arg2, arg3, arg4;
{
	switch (func) {
	case 0:	return usemctl (arg1, arg2, arg3, arg4);
	case 1:	return usemget (arg1, arg2, arg3);
	case 2:	return usemop (arg1, arg2, arg3);
	default: set_user_error (EINVAL);
	}
}
