#define	_DDI_DKI	1
#define	_SYSV4		1

/*
 * STREAMS memory management code.
 *
 * This is layered on top of the fast first-fit heap allocator whose
 * implementation is described in <sys/st_alloc.h>. The particulars of how
 * STREAMS memory is allocated (including synchronisation and watermarks)
 * is kept here so that the generic allocator is just that, generic.
 */

/*
 *-IMPORTS:
 *	<common/ccompat.h>
 *		__USE_PROTO__
 *		__ARGS ()
 *		__LOCAL__
 *	<kernel/ddi_lock.h>
 *		stream_heap_hierarchy
 *		stream_dir_hierarchy
 *		stream_proc_hierarchy
 *	<sys/debug.h>
 *		ASSERT ()
 *	<sys/types.h>
 *		_VOID
 *		size_t
 *	<sys/ksynch.h>
 *		lock_t
 *		LOCK_ALLOC ()
 *		LOCK ()
 *		UNLOCK ()
 *	<sys/cmn_err.h>
 *		CE_WARN
 *		cmn_err ()
 */

#include <common/ccompat.h>
#include <kernel/ddi_lock.h>
#include <sys/types.h>
#include <sys/debug.h>
#include <sys/ksynch.h>
#include <sys/cmn_err.h>

#include <sys/kmem.h>
#include <kernel/strmlib.h>
#include <string.h>


/*
 * Here we'll define the actual instance of the streams memory control
 * structure.
 */

struct streams_mem str_mem [1];


/*
 * We need to define information structures for the various locks and
 * synchronization variables used in the above.
 */

__LOCAL__ lkinfo_t _stream_heap_lkinfo = {
	"STREAMS message memory lock", INTERNAL_LOCK
};

__LOCAL__ lkinfo_t _stream_seq_lkinfo = {
	"STREAMS log sequence-number lock", INTERNAL_LOCK
};

__LOCAL__ lkinfo_t _stream_proc_lkinfo = {
	"STREAMS qprocsoff () lock", INTERNAL_LOCK
};

__LOCAL__ lkinfo_t _stream_dir_lkinfo = {
	"STREAM directory read/write lock", INTERNAL_LOCK
};


/*
 * This private function gathers some of the aspects of streams message memory
 * allocation into a single place (message blocks are allocated in allocb (),
 * dupb (), and esballoc ()). We leave the initialization of the newly
 * allocated memory up to the caller.
 */

#if	__USE_PROTO__
mblk_t * (STRMEM_ALLOC) (size_t size, int pri, int flag)
#else
mblk_t *
STRMEM_ALLOC __ARGS ((size, pri, flag))
size_t		size;
int		pri;
int		flag;
#endif
{
	pl_t		prev_pl;
	mblk_t	      *	mblkp;

	ASSERT (size > 0);
	ASSERT (flag == KM_SLEEP || flag == KM_NOSLEEP);

	/*
	 * Note that if the size is one such that it cannot possibly ever be
	 * satisfied given the allocation watermarks we have set, then we just
	 * return failure now.
	 */

	pri = MAP_PRI_LEVEL (pri);

	if (size > str_mem->sm_max [pri])
		return NULL;


	for (;;) {
		/*
		 * Lock the basic lock protecting access to the memory pool
		 * and attempt to acquire the memory we desire.
		 */

		prev_pl = LOCK (str_mem->sm_msg_lock, str_msg_pl);


		/*
		 * Before allocating any memory, we check to see that it makes
		 * sense to give out that memory to the given priority level.
		 */

		if (str_mem->sm_used + size <= str_mem->sm_max [pri]) {
			/*
			 * Try to get the memory, and if we do update the
			 * priority bookkeeping information to record the
			 * amount of memory that we have allowed out.
			 */

			mblkp = (mblk_t *) st_alloc (str_mem->sm_msg_heap,
						     size);

			if (mblkp != NULL) {

				str_mem->sm_used += size;
				break;
			}
		}


		/*
		 * Depending on the caller, we may sleep waiting for memory
		 * to become available.
		 */

		if (flag == KM_NOSLEEP) {

			mblkp = NULL;
			break;
		}


		/*
		 * RESEARCH NOTE: This policy is a guess, no more. We need to
		 * do some profiling to find out what effect other policies
		 * might have. In particular, the wakeup heuristic could be
		 * altered to broadcast when we can satisfy the largest
		 * request.
		 */

		if (str_mem->sm_msg_needed == 0 ||
		    str_mem->sm_msg_needed > size)
			str_mem->sm_msg_needed = size;

		SV_WAIT (str_mem->sm_msg_sv, prilo, str_mem->sm_msg_lock);
	}

	UNLOCK (str_mem->sm_msg_lock, prev_pl);

	return mblkp;
}


/*
 * This simple function factors out some common code from different calls to
 * st_free () inside freeb (). This just reduces some of the cost of all the
 * error checking that is my custom, and consolidates the interface to the
 * bookkeeping for callback events and so forth.
 *
 * The streams heap must be locked on entry to this function.
 */

#if	__USE_PROTO__
void (STRMEM_FREE) (mblk_t * bp, size_t size)
#else
void
STRMEM_FREE __ARGS ((bp, size))
mblk_t	      *	bp;
size_t		size;
#endif
{
	int		free_ok;

	ASSERT (TRYLOCK (str_mem->sm_msg_lock, str_msg_pl) == invpl);

	free_ok = st_free (str_mem->sm_msg_heap, bp, size);

	if (free_ok != 0) {
		/*
		 * The heap manager has a problem with freeing the block that
		 * was passed to it, display a console diagnostic. For
		 * simplicity we display addresses as longs.
		 */

		cmn_err (CE_WARN,
			 "MSGB_FREE : st_free () complained with %d freeing %d bytes at %lx",
			 free_ok, size, (long) bp);
	} else {
		/*
		 * Update the allocation bookkeeping, and request that the
		 * routine that processes bufcall () events be run. This other
		 * procedure also has responsibility for waking up message and
		 * possibly "other" allocations if there is sufficient memory
		 * available.
		 */

		str_mem->sm_used -= size;

		SCHEDULE_BUFCALLS ();
	}
}


__EXTERN_C__
#if	__USE_PROTO__
int (STRMEM_INIT) (__VOID__ * addr, size_t size)
#else
int
STRMEM_INIT __ARGS ((addr, size))
__VOID__      *	addr;
size_t		size;
#endif
{
	int		i;

	/*
	 * Now initialize the fast-first-fit heap manager.
	 */

	str_mem->sm_msg_heap = st_heap_init (addr, size);

	str_mem->sm_msg_lock =
			LOCK_ALLOC (stream_heap_hierarchy, str_msg_pl,
				    & _stream_heap_lkinfo, KM_NOSLEEP);

	str_mem->sm_msg_sv = SV_ALLOC (KM_NOSLEEP);


	/*
	 * If either of the above allocations failed, we have some kind of
	 * major problem, so we exit without unlocking the initialization flag
	 * with an error indication.
	 */

	if (str_mem->sm_msg_lock == NULL || str_mem->sm_msg_sv == NULL) {

init_error:
		cmn_err (CE_PANIC, "Could not initialize STREAMS subsystem");
		return -1;
	}


	/*
	 * Now we can calculate the watermarks... start at the
	 * top and make each lower one some percentage of the
	 * next higher one (say, 15/16 or 93%, so that it's
	 * easy to calculate).
	 */

	for (i = N_PRI_LEVELS ; i -- > 0 ;) {

		str_mem->sm_max [i] = size;
		size -= size >> 4;	/* - 1/16 */
	}


	/*
	 * Do other kinds of initialization for the "str_mem" structure.
	 */

	for (i = N_PRI_LEVELS ; i -- > 0 ; ) {

		if (SELIST_INIT (& str_mem->sm_bcevents [i],
				 KM_SLEEP) == NULL)
			goto init_error;
	}


	str_mem->sm_seq_lock = LOCK_ALLOC (stream_seq_hierarchy, plstr,
					   & _stream_seq_lkinfo, KM_SLEEP);

	str_mem->sm_head_lock = RW_ALLOC (stream_dir_hierarchy, plstr,
					  & _stream_dir_lkinfo, KM_SLEEP);

	str_mem->sm_proc_lock = LOCK_ALLOC (stream_proc_hierarchy, plstr,
					    & _stream_proc_lkinfo, KM_SLEEP);

	str_mem->sm_proc_sv = SV_ALLOC (KM_SLEEP);

	if (SCHED_INIT (str_mem->sm_sched, KM_SLEEP) == NULL ||
	    str_mem->sm_seq_lock == NULL || str_mem->sm_head_lock == NULL ||
	    str_mem->sm_proc_lock == NULL || str_mem->sm_proc_sv == NULL)
		goto init_error;

	return 0;		/* all OK */
}
