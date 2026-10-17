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
 *-STATUS:
 *	DDI/DKI
 *
 *-NAME:
 *	kmem_free ()	Free previously allocated kernel memory.
 *
 *-SYNOPSIS:
 *	#include <sys/types.h>
 *	#include <sys/kmem.h>
 *
 *	void kmem_free (void * addr, size_t size);
 *
 *-ARGUMENTS:
 *	addr		Address of the allocated memory to be returned. "addr"
 *			must specify the same address that was returned by the
 *			corresponding call to kmem_alloc () or kmem_zalloc ()
 *			which allocated the memory.
 *
 *	size		Number of bytes to free. The "size" parameter must
 *			specify the same number of bytes as was allocated by
 *			the corresponding call to kmem_alloc () or\
 *			kmem_zalloc ().
 *
 *-DESCRIPTION:
 *	kmem_free () returns "size" bytes of previously allocated kernel
 *	memory to the free pool. The "addr" and "size" arguments must specify
 *	exactly one complete area of memory that was allocated by a call to
 *	kmem_alloc () or kmem_zalloc () (that is, the memory cannot be freed
 *	piecemeal).
 *
 *-RETURN VALUE:
 *	None.
 *
 *-LEVEL:
 *	Base or Interrupt.
 *
 *-NOTES:
 *	Does not sleep.
 *
 *	Driver-defined basic locks, read/write locks and sleep locks may be
 *	held across calls to this function.
 *
 *-SEE ALSO:
 *	kmem_alloc (), kmem_zalloc ()
 */

#if	__USE_PROTO__
void (kmem_free) (_VOID * addr, size_t size)
#else
void
kmem_free __ARGS ((addr, size))
_VOID	      *	addr;
size_t		size;
#endif
{
	pl_t		prev_pl;
	int		free_ok;

	ASSERT (addr != NULL);
	ASSERT (size > 0);

	ASSERT (ddi_global_data ()->dg_kmem_lock != NULL);

	/*
	 * Acquire the basic lock protecting access to the memory and free
	 * the caller's area. If there are processes waiting on memory
	 * becoming available, wake them up via a synchronization variable
	 * broadcast.
	 */

	if (ddi_global_data ()->dg_kmem_lock != NULL)
		prev_pl = LOCK (ddi_global_data ()->dg_kmem_lock,
				kmem_heap_priority);


	if (_ST_CHECK (ddi_global_data ()->dg_kmem_heap, "kmem_free ()") != 0)
		free_ok = -3;
	else
		free_ok = st_free (ddi_global_data ()->dg_kmem_heap, addr, size);

	if (ddi_global_data ()->dg_kmem_required > 0 &&
	    ddi_global_data ()->dg_kmem_required <=
			st_maxavail (ddi_global_data ()->dg_kmem_heap)) {
		/*
		 * Wake up *all* the waiting processes and clear the marker
		 * to indicate that there are no waiting processes.
		 */

		SV_BROADCAST (ddi_global_data ()->dg_kmem_sv, 0);
		ddi_global_data ()->dg_kmem_required = 0;
	}

	if (ddi_global_data ()->dg_kmem_lock != NULL)
		UNLOCK (ddi_global_data ()->dg_kmem_lock, prev_pl);

	if (free_ok != 0) {
		/*
		 * The heap manager has a problem with freeing the block that
		 * was passed to it, display a console diagnostic. For
		 * simplicity we display addresses as longs.
		 */

		cmn_err (CE_WARN,
			 "kmem_free : st_free () complained with %d freeing %d bytes at %lx",
			 free_ok, size, (long) addr);
		backtrace (0);
	}
}
