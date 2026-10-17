/* $Header: $ */

#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1
#define	_SYSV4		1

/*
 * Definition for the DDI/DKI interrupt subsystem initialization. This is
 * called from KMEM_INIT () at present, but we keep it separate as a matter
 * of course.
 *
 * $Log: $
 */

/*
 *-IMPORTS:
 *	<common/ccompat.h>
 *		__USE_PROTO__
 *		__ARGS ()
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
 *	<stddef.h>
 *		NULL
 */

#include <common/ccompat.h>
#include <kernel/ddi_lock.h>
#include <sys/types.h>
#include <sys/ksynch.h>
#include <stddef.h>

#include <kernel/ddi_cpu.h>


/*
 * Do what we need to do to ensure that the spl... () functions work.
 */

#include <sys/inline.h>

__EXTERN_C__
#if	__USE_PROTO__
int (INTR_INIT) (void)
#else
int
INTR_INIT __ARGS (())
#endif
{
	dcdata_t      *	dcdatap;

	/*
	 * This implementation statically allocates the per-CPU data, since
	 * deep down we really "know" that we are working on a uniprocessor.
	 * This also means we don't have to zero-fill anything.
	 */

	dcdatap = ddi_cpu_data ();
	dcdatap->dc_base_mask = (inb (__SPICM__) << 8) | inb (__PICM__);
	dcdatap->dc_int_level = 0;
	dcdatap->dc_user_level = 1;
	dcdatap->dc_ipl = plbase;


	/*
	 * Set up the "dynamic" part of the structures.
	 */

	dcdatap->dc_dynalloc = __ddi_cpu_dynarea;
	dcdatap->dc_dynend = __ddi_cpu_dynarea + sizeof (__ddi_cpu_dynarea);


	/*
	 * Carve off some of the "dynamic" space for the heirarchy-test stuff.
	 * This is done with the dynamic stuff because it seems like a really
	 * bad idea putting a table in space we want to form the basis of an
	 * internal binary standard (even though this is a very well-defined
	 * array).
	 */

	dcdatap->dc_hierarchy_cnt = (__lkhier_t *)
		ddi_cpu_alloc (sizeof (__lkhier_t) *
			       (__MAX_HIERARCHY__ - __MIN_HIERARCHY__ + 1));

	return dcdatap->dc_hierarchy_cnt == NULL;
}

