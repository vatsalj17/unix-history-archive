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
 *		LOCK ()
 *		UNLOCK ()
 */

#include <common/ccompat.h>
#include <kernel/ddi_lock.h>
#include <kernel/ddi_glob.h>
#include <kernel/st_alloc.h>
#include <sys/types.h>
#include <sys/debug.h>
#include <sys/ksynch.h>

#include <sys/kmem.h>

/*
 *-STATUS:
 *	DDI/DKI
 *
 *-NAME:
 *	kmem_alloc ()	Allocate space from kernel free memory.
 *
 *-SYNOPSIS:
 *	#include <sys/types.h>
 *	#include <sys/kmem.h>
 *
 *	void * kmem_alloc (size_t size, int flag);
 *
 *-ARGUMENTS:
 *	size		Number of bytes to allocate.
 *
 *	flag		Specifies whether the caller is willing to sleep
 *			waiting for memory. If "flag" is set to KM_SLEEP, the
 *			caller will sleep if necessary until the specified
 *			amount of memory is available. If "flag" is set to
 *			KM_NOSLEEP, the caller will not sleep, but
 *			kmem_alloc () will return NULL if the specified amount
 *			of memory is not immediately available.
 *
 *-DESCRIPTION:
 *	kmem_alloc () allocates "size" bytes of kernel memory and returns a
 *	pointer to the allocated memory.
 *
 *-RETURN VALUE:
 *	Upon successful completion, kmem_alloc () returns a pointer to the
 *	allocated memory. If KM_NOSLEEP is specified and sufficient memory is
 *	not immediately available, kmem_alloc () returns a NULL pointer. If
 *	"size" is set to 0, kmem_alloc () always returns NULL regardless of
 *	the value of "flag".
 *
 *-LEVEL:
 *	Base only if "flag" is set to KM_SLEEP. Base or interrupt if "flag" is
 *	set to KM_NOSLEEP.
 *
 *-NOTES:
 *	May sleep if "flag" is set to KM_SLEEP.
 *
 *	Driver-defined basic locks and read/write locks may be held across
 *	calls to this function if "flag" is KM_NOSLEEP but may not be held if
 *	"flag" is KM_SLEEP.
 *
 *	Driver-defined sleep locks may be held across calls to this function
 *	regardless of the value of "flag".
 *
 *	Kernel memory is a limited resource and should be used judiciously.
 *	Memory allocated using kmem_alloc () should be freed as soon as
 *	possible. Drivers should not use local freelists for memory or similar
 *	schemes that cause the memory to be held for longer than necessary.
 *
 *	The address returned by a successful call to kmem_alloc () is word-
 *	aligned.
 *
 *-SEE ALSO:
 *	kmem_free (), kmem_zalloc ()
 */

#if	__USE_PROTO__
_VOID * (kmem_alloc) (size_t size, int flag)
#else
_VOID *
kmem_alloc __ARGS ((size, flag))
size_t		size;
int		flag;
#endif
{
	_VOID	      *	mem;
	pl_t		prev_pl;

	ASSERT (flag == KM_SLEEP || flag == KM_NOSLEEP);

	ASSERT (ATOMIC_FETCH_UCHAR (ddi_global_data ()->dg_init_flag) ||
		ddi_global_data ()->dg_kmem_lock != NULL);

	if (size == 0)
		return NULL;

	for (;;) {
		/*
		 * Lock the basic lock protecting access to the memory pool
		 * and attempt to acquire the memory we desire.
		 */

		if (ddi_global_data ()->dg_kmem_lock != NULL)
			prev_pl = LOCK (ddi_global_data ()->dg_kmem_lock,
					kmem_heap_priority);

		if (_ST_CHECK (ddi_global_data ()->dg_kmem_heap,
			       "kmem_alloc ()") != 0) {
			mem = NULL;
			break;
		}

		if ((mem = st_alloc (ddi_global_data ()->dg_kmem_heap,
				     size)) != NULL ||
		    flag == KM_NOSLEEP)
			break;

		/*
		 * Since we cannot acquire the memory, but the caller is
		 * willing to wait, we wait on a synchronization variable
		 * for sufficient memory to be available. We record the
		 * minimum amount that will satisfy any outstanding wait so
		 * that kmem_free () need not perform broadcasts in a
		 * totally needless fashion.
		 *
		 * We have arbitrarily chosen a low scheduling priority for
		 * SV_WAIT ().
		 */
		/*
		 * RESEARCH NOTE: This policy is a guess, no more. We need to
		 * do some profiling to find out what effect other policies
		 * might have. In particular, the wakeup heuristic could be
		 * altered to broadcast when we can satisfy the largest
		 * request.
		 */

		if (ddi_global_data ()->dg_kmem_required == 0 ||
		    ddi_global_data ()->dg_kmem_required > size)
			ddi_global_data ()->dg_kmem_required = size;

		SV_WAIT (ddi_global_data ()->dg_kmem_sv, prilo,
			 ddi_global_data ()->dg_kmem_lock);
	}

	if (ddi_global_data ()->dg_kmem_lock != NULL)
		UNLOCK (ddi_global_data ()->dg_kmem_lock, prev_pl);

	return mem;
}
