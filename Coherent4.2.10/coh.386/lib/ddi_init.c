/* $Header: $ */

#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1
#define	_SYSV4		1

/*
 * Definition for the DDI/DKI initialization code.
 *
 * $Log: $
 */

/*
 *-IMPORTS:
 *	<common/ccompat.h>
 *		__USE_PROTO__
 *		__ARGS ()
 *	<kernel/x86lock.h>
 *		ATOMIC_STORE_UCHAR ()
 *	<kernel/ddi_lock.h>
 *		INTERNAL_LOCK
 *		poll_global_hierarchy
 *		poll_global_priority
 *		defer_hierarchy
 *		defer_priority
 *		proc_global_hierarchy
 *		proc_global_priority
 *	<sys/types.h>
 *		inb ()
 *		outb ()
 *	<sys/ksynch.h>
 *		lkinfo_t
 *		LOCK_ALLOC ()
 *	<sys/kmem.h>
 *		KM_NOSLEEP
 *		kmem_alloc ()
 *	<stddef.h>
 *		NULL
 *	<string.h>
 *		memset ()
 */

#include <common/ccompat.h>
#include <kernel/x86lock.h>
#include <kernel/ddi_lock.h>
#include <sys/types.h>
#include <sys/ksynch.h>
#include <sys/kmem.h>
#include <stddef.h>
#include <string.h>

#include <kernel/ddi_base.h>
#include <kernel/ddi_proc.h>
#include <kernel/ddi_cpu.h>
#include <kernel/ddi_glob.h>


/*
 * Lock information structures.
 */

__LOCAL__ lkinfo_t poll_global_lkinfo = {
	"polling global lock", INTERNAL_LOCK
};

__LOCAL__ lkinfo_t defer_lkinfo = {
	"defer write lock", INTERNAL_LOCK
};

__LOCAL__ lkinfo_t proc_global_lkinfo = {
	"process global lock", INTERNAL_LOCK
};


/*
 * Set up one of the deferred-function tables.
 */

#if	__USE_PROTO__
__LOCAL__ int (DEFER_INIT) (defer_t * deferp, int local, int max)
#else
__LOCAL__ int
DEFER_INIT __ARGS ((deferp, local, max))
defer_t	      *	deferp;
int		local;
int		max;
#endif
{
	deferp->df_tab = (__deffuncp_t *) (local == 0 ?
		kmem_alloc (sizeof (__deffuncp_t) * max, KM_NOSLEEP) :
		ddi_cpu_alloc (sizeof (__deffuncp_t) * max));

	deferp->df_wlock = LOCK_ALLOC (defer_hierarchy, defer_priority,
				       & defer_lkinfo, KM_NOSLEEP);

	if (deferp->df_tab == NULL || deferp->df_wlock == NULL)
		return 1;

	ATOMIC_STORE_UCHAR (deferp->df_max, max);
	return 0;
}


/*
 * Set up the global and per-CPU DDI/DKI data, locks and other whatnot.
 */

__EXTERN_C__
#if	__USE_PROTO__
int (DDI_GLOB_INIT) (void)
#else
int
DDI_GLOB_INIT __ARGS (())
#endif
{
	dgdata_t      *	dgdatap = ddi_global_data ();
	dcdata_t      *	dcdatap = ddi_cpu_data ();

	/*
	 * Set up the global data table. We assume here that our global data
	 * is zeroed by default.
	 */

	if (DEFER_INIT (& dgdatap->dg_defint, 0, 25) != 0 ||
	    DEFER_INIT (& dgdatap->dg_defproc, 0, 25) != 0 ||
	    DEFER_INIT (& dcdatap->dc_defint, 1, 25) != 0 ||
	    DEFER_INIT (& dcdatap->dc_defproc, 1, 25) != 0)
		return 1;

	dgdatap->dg_polllock = LOCK_ALLOC (poll_global_hierarchy,
					   poll_global_priority,
					   & poll_global_lkinfo, KM_NOSLEEP);

	dgdatap->dg_proclock = LOCK_ALLOC (proc_global_hierarchy,
					   proc_global_priority,
					   & proc_global_lkinfo, KM_NOSLEEP);

	if (dgdatap->dg_polllock == NULL || dgdatap->dg_proclock == NULL)
		return 1;

	return 0;
}

