/* $Header: $ */

#define	_DDI_DKI	1
#define	_SYSV4		1

/*
 * This is layered on top of the fast first-fit heap allocator. Since we want
 * to keep the allocator portable, this layer deals with synchronization
 * issues. Note that we have to deal with a special chicken-and-egg problem
 * with synchronization, in that we want to use the allocation functions for
 * allocating the locks that will be used in the allocation functions...
 *
 * $Log: $
 */

/*
 *-IMPORTS:
 *	<common/ccompat.h>
 *		__USE_PROTO__
 *		__ARGS ()
 *	<kernel/ddi_lock.h>
 *		kmem_heap_hierarchy
 *		kmem_heap_priority
 *	<kernel/ddi_glob.h>
 *		dgdata_t
 *		ddi_global_data ()
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
#include <kernel/ddi_glob.h>
#include <kernel/st_alloc.h>
#include <sys/types.h>
#include <sys/debug.h>
#include <sys/ksynch.h>
#include <sys/cmn_err.h>

#include <sys/kmem.h>


/*
 * In order for the kmem_... () system to work, we need the locking system
 * to work, and that needs the spl... () functions. So, we ensure that that
 * is set up.
 */

__EXTERN_C_BEGIN__

int		INTR_INIT		__PROTO ((void));

__EXTERN_C_END__


/*
 * We need to define information structures for the various locks and
 * synchronization variables used in the above.
 */

__LOCAL__ lkinfo_t _kmem_heap_lkinfo = {
	"kmem_... () heap memory lock", INTERNAL_LOCK
};


#define	INIT_FLAG	ddi_global_data ()->dg_init_flag

/*
 *-STATUS:
 *	Initialisation
 *
 *-DESCRIPTION:
 *	This function initializes the memory subsystem given a region of
 *	kernel virtual memory space to manage.
 */

__EXTERN_C__
#if	__USE_PROTO__
int (KMEM_INIT) (__VOID__ * addr, size_t size)
#else
int
KMEM_INIT __ARGS ((addr, size))
__VOID__      *	addr;
size_t		size;
#endif
{
	/*
	 * We use a test-and-set lock operation on the streams memory
	 * structure so that the initialisation process is multiprocessor-
	 * safe. We don't use a basic lock since we don't know whether basic
	 * locks exist yet.
	 */

	if (ATOMIC_TEST_AND_SET_UCHAR (INIT_FLAG) != 0) {
		/*
		 * Presumably we are on a separate processor waiting for the
		 * initialization to be completed by someone else. To make
		 * this processor's call to STRMEM_INIT () behave with the
		 * right semantics, we wait for the other instance to complete
		 * the setup process.
		 */

		while (ATOMIC_FETCH_UCHAR (INIT_FLAG) != 0) {
#ifdef	__UNIPROCESSOR__
			cmn_err (CE_PANIC, "Init startup deadlock???");
#endif
		}
		return 0;
	}

	if (ddi_global_data ()->dg_kmem_lock != NULL) {
		/*
		 * The init has already been done, thanks!
		 */

		ATOMIC_CLEAR_UCHAR (INIT_FLAG);
		return 0;
	}

	INTR_INIT ();

	/*
	 * Now initialize the fast-first-fit heap manager; then, allocate the
	 * locks for the code above to use in protecting heap access.
	 */

	ddi_global_data ()->dg_kmem_heap = st_heap_init (addr, size);

	ddi_global_data ()->dg_kmem_lock =
			LOCK_ALLOC (kmem_heap_hierarchy, kmem_heap_priority,
				    & _kmem_heap_lkinfo, KM_NOSLEEP);

	ddi_global_data ()->dg_kmem_sv = SV_ALLOC (KM_NOSLEEP);


	/*
	 * If either of the above allocations failed, we have some kind of
	 * major problem, so we exit without unlocking the initialization flag
	 * with an error indication.
	 */

	if (ddi_global_data ()->dg_kmem_lock == NULL ||
	    ddi_global_data ()->dg_kmem_sv == NULL) {

		cmn_err (CE_PANIC,
			 "Could not initialize kmem_... () subsystem");
		return -1;
	}


	/*
	 * All OK, let other CPUs proceed and return success to the caller.
	 */

	ATOMIC_CLEAR_UCHAR (INIT_FLAG);
	return 0;		/* all OK */
}
