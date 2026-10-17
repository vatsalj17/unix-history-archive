/* $Header: $ */
/*
 * Iteration functions for walking over the process table in common useful
 * ways.
 *
 * $Log: $
 */

#include <common/ccompat.h>
#include <kernel/proc_lib.h>
#include <sys/types.h>
#include <stddef.h>
#include <limits.h>


#define	_KERNEL		1

#include <sys/proc.h>

/*
 * Iterator function for causing a return from the iteration after finding a
 * single candidate (i.e., making the iterator functions do a simple search).
 */

#if	__USE_PROTO__
int iter_find_any (__proc_t * __NOTUSED (procp), __VOID__ * __NOTUSED (arg))
#else
int
iter_find_any (procp, arg)
__proc_t      *	procp;
__VOID__      *	arg;
#endif
{
	return 1;	/* terminate iteration */
}


/*
 * Locate the process with the process ID "pid", and return a pointer to it,
 * or NULL if no process with the given ID can be found.
 */

#if	__USE_PROTO__
__proc_t * process_find_pid (__pid_t pid)
#else
__proc_t *
process_find_pid (pid)
__pid_t		pid;
#endif
{
	__proc_t      *	procp;

	/*
	 * The Coherent 4.x (and presumably 3.x) process table is kept in a
	 * doubly-linked list sorted by process ID. Since this has no value
	 * whatsoever, we don't rely on it staying that way.
	 */

	__GLOBAL_LOCK_PROCESS_TABLE ("process_find_pid ()");

	for (procp = procq.p_nforw ; procp != & procq ;
	     procp = procp->p_nforw) {

		if (procp->p_pid == pid) {
			__GLOBAL_UNLOCK_PROCESS_TABLE ();
			return procp;
		}
	}

	__GLOBAL_UNLOCK_PROCESS_TABLE ();
	return NULL;
}


/*
 * Locate every process with the given parent process ID, and if the user-
 * supplied callback function is not NULL, call the user with the address of
 * each found entry. If the callback function returns 0, the search continues,
 * otherwise the search will terminate at that point. The callback function is
 * called with the process table globally locked.
 *
 * This function specifically ignores zombie processes. There is a matching
 * search process which specifically only includes zombies.
 *
 * The return value is the number of matches located.
 */

#if	__USE_PROTO__
int process_find_ppid (__pid_t ppid, __proc_iter_t iter, __VOID__ * arg)
#else
int
process_find_ppid (ppid, iter, arg)
__pid_t		ppid;
__proc_iter_t	iter;
__VOID__      *	arg;
#endif
{
	int		match_count = 0;
	__proc_t      *	procp;

	/*
	 * The Coherent 4.x (and presumably 3.x) process table is kept in a
	 * doubly-linked list sorted by process ID. Since this has no value
	 * whatsoever, we don't rely on it staying that way.
	 */

	__GLOBAL_LOCK_PROCESS_TABLE ("process_find_ppid ()");

	for (procp = procq.p_nforw ; procp != & procq ;
	     procp = procp->p_nforw) {

		if (procp->p_ppid == ppid && procp->p_state != PSDEAD) {
			match_count ++;

			if (iter != NULL && (* iter) (procp, arg))
				break;
		}
	}

	__GLOBAL_UNLOCK_PROCESS_TABLE ();
	return match_count;
}


/*
 * Locate every process with the given parent process ID, and if the user-
 * supplied callback function is not NULL, call the user with the address of
 * each found entry. If the callback function returns 0, the search continues,
 * otherwise the search will terminate at that point. The callback function is
 * called with the process table globally locked.
 *
 * This function will only match zombie processes. There is a corresponding
 * function that only matches non-zombie processes.
 *
 * The return value is the number of matches located.
 */

#if	__USE_PROTO__
int process_find_zombies (__pid_t ppid, __proc_iter_t iter, __VOID__ * arg)
#else
int
process_find_zombies (ppid, iter, arg)
__pid_t		ppid;
__proc_iter_t	iter;
__VOID__      *	arg;
#endif
{
	int		match_count = 0;
	__proc_t      *	procp;

	/*
	 * The Coherent 4.x (and presumably 3.x) process table is kept in a
	 * doubly-linked list sorted by process ID. Since this has no value
	 * whatsoever, we don't rely on it staying that way.
	 */

	__GLOBAL_LOCK_PROCESS_TABLE ("process_find_zombies ()");

	for (procp = procq.p_nforw ; procp != & procq ;
	     procp = procp->p_nforw) {

		if (procp->p_ppid == ppid && procp->p_state == PSDEAD) {
			match_count ++;

			if (iter != NULL && (* iter) (procp, arg))
				break;
		}
	}

	__GLOBAL_UNLOCK_PROCESS_TABLE ();
	return match_count;
}


/*
 * Locate every process with the given process group ID, and if the user-
 * supplied callback function is not NULL, call the user with the address of
 * each found entry. If the callback function returns 0, the search continues,
 * otherwise the search will terminate at that point. The callback function is
 * called with the process table globally locked.
 *
 * The return value is the number of matches located.
 */

#if	__USE_PROTO__
int process_find_pgid (__pid_t pgid, __proc_iter_t iter, __VOID__ * arg)
#else
int
process_find_pgid (pgid, iter, arg)
__pid_t		pgid;
__proc_iter_t	iter;
__VOID__      *	arg;
#endif
{
	int		match_count = 0;
	__proc_t      *	procp;

	/*
	 * The Coherent 4.x (and presumably 3.x) process table is kept in a
	 * doubly-linked list sorted by process ID. Since this has no value
	 * whatsoever, we don't rely on it staying that way.
	 */

	__GLOBAL_LOCK_PROCESS_TABLE ("process_find_pgid ()");

	for (procp = procq.p_nforw ; procp != & procq ;
	     procp = procp->p_nforw) {

		if (procp->p_group == pgid && procp->p_state != PSDEAD) {
			match_count ++;

			if (iter != NULL && (* iter) (procp, arg))
				break;
		}
	}

	__GLOBAL_UNLOCK_PROCESS_TABLE ();
	return match_count;
}


/*
 * Locate every process with the given session ID, and if the user-supplied
 * callback function is not NULL, call the user with the address of each found
 * entry. If the callback function returns 0, the search continues, otherwise
 * the search will terminate at that point. The callback function is called
 * with the process table globally locked.
 *
 * The return value is the number of matches located.
 */

#if	__USE_PROTO__
int process_find_sid (__pid_t sid, __proc_iter_t iter, __VOID__ * arg)
#else
int
process_find_sid (sid, iter, arg)
__pid_t		sid;
__proc_iter_t	iter;
__VOID__      *	arg;
#endif
{
	int		match_count = 0;
	__proc_t      *	procp;

	/*
	 * The Coherent 4.x (and presumably 3.x) process table is kept in a
	 * doubly-linked list sorted by process ID. Since this has no value
	 * whatsoever, we don't rely on it staying that way.
	 */

	__GLOBAL_LOCK_PROCESS_TABLE ("process_find_sid ()");

	for (procp = procq.p_nforw ; procp != & procq ;
	     procp = procp->p_nforw) {

		if (procp->p_sid == sid && procp->p_state != PSDEAD) {
			match_count ++;

			if (iter != NULL && (* iter) (procp, arg))
				break;
		}
	}

	__GLOBAL_UNLOCK_PROCESS_TABLE ();
	return match_count;
}


/*
 * Locate every process that can be signalled (i.e., every process except for
 * the idle process, init, and any kernel processes), and if the user-supplied
 * callback function is not NULL, call the user with the address of each found
 * entry. If the callback function returns 0, the search continues, otherwise
 * the search will terminate at that point. The callback function is called
 * with the process table globally locked.
 *
 * The return value is the number of matches located.
 */

#if	__USE_PROTO__
int process_find_all (__proc_iter_t iter, __VOID__ * arg)
#else
int
process_find_all (iter, arg)
__proc_iter_t	iter;
__VOID__      *	arg;
#endif
{
	int		match_count = 0;
	__proc_t      *	procp;

	/*
	 * The Coherent 4.x (and presumably 3.x) process table is kept in a
	 * doubly-linked list sorted by process ID. Since this has no value
	 * whatsoever, we don't rely on it staying that way.
	 */

	__GLOBAL_LOCK_PROCESS_TABLE ("process_find_all ()");

	for (procp = procq.p_nforw ; procp != & procq ;
	     procp = procp->p_nforw) {

		if (procp->p_state != PSDEAD && procp->p_pid > SYSPID_MAX &&
		    (procp->p_flags & PFKERN) == 0) {
			match_count ++;

			if (iter != NULL && (* iter) (procp, arg))
				break;
		}
	}

	__GLOBAL_UNLOCK_PROCESS_TABLE ();
	return match_count;
}


/*
 * Locate every process that can be signalled (i.e., every process except for
 * the idle process, init, and any kernel processes), and if the user-supplied
 * callback function is not NULL, call the user with the address of each found
 * entry. If the callback function returns 0, the search continues, otherwise
 * the search will terminate at that point. The callback function is called
 * with the process table globally locked.
 *
 * The return value is the number of matches located.
 */

#if	__USE_PROTO__
int session_find_pgid (__proc_t * sesp, __pid_t pgid, __proc_iter_t iter,
		       __VOID__ * arg)
#else
int
session_find_pgid (sesp, pgid, iter, arg)
__proc_t      *	sesp;
__pid_t		pgid;
__proc_iter_t	iter;
__VOID__      *	arg;
#endif
{
	int		match_count = 0;
	__proc_t      *	procp;

	/*
	 * The Coherent 4.x (and presumably 3.x) process table is kept in a
	 * doubly-linked list sorted by process ID. Since this has no value
	 * whatsoever, we don't rely on it staying that way.
	 */

	__GLOBAL_LOCK_PROCESS_TABLE ("session_find_pgid ()");

	for (procp = procq.p_nforw ; procp != & procq ;
	     procp = procp->p_nforw) {

		if (procp->p_sid == sesp->p_sid && procp->p_group == pgid &&
		    procp->p_state != PSDEAD) {
			match_count ++;

			if (iter != NULL && (* iter) (procp, arg))
				break;
		}
	}

	__GLOBAL_UNLOCK_PROCESS_TABLE ();
	return match_count;
}
