/* $Header: $ */

/*
 * Simple realloc () function for memory allocated with kmem_alloc ()
 * (assuming that it is our kmem_alloc (), that is).
 *
 * $Log: $
 */

#include <common/ccompat.h>
#include <kernel/st_alloc.h>
#include <sys/types.h>
#include <sys/ksynch.h>
#include <sys/kmem.h>

#if	__USE_PROTO__
__VOID__ * (kmem_realloc) (__VOID__ * mem, size_t newsize, size_t oldsize)
#else
__VOID__ *
kmem_realloc __ARGS ((mem, newsize, oldsize))
__VOID__      *	mem;
size_t		newsize;
size_t		oldsize;
#endif
{
	pl_t		prev_pl;

	ASSERT (mem != NULL && newsize > 0 && oldsize > 0);

	prev_pl = LOCK (str_mem->sm_other_lock, str_other_pl);

	if (_ST_CHECK (ddi_global_data ()->dg_kmem_heap,
		       "kmem_realloc ()") != 0)
		mem = NULL;
	else
		mem = st_realloc (str_mem->sm_other_heap, mem, newsize,
				  oldsize);

	OTHER_ALLOCED (newsize - oldsize);

	UNLOCK (str_mem->sm_other_lock, prev_pl);

	return mem;
}


