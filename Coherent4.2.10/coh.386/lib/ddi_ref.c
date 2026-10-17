/* $Header: $ */

#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1
#define	_SYSV4		1

/*
 * Definition for the DDI/DKI linkable version of ddi_cpu_ref ()
 *
 * $Log: $
 */

/*
 *-IMPORTS:
 *	<common/ccompat.h>
 *		__USE_PROTO__
 *		__ARGS ()
 *	<stddef.h>
 *		NULL
 */

#include <common/ccompat.h>
#include <stddef.h>

#include <kernel/ddi_cpu.h>


/*
 *-STATUS:
 *	For the Implementors only.
 *
 *-NAME:
 *	ddi_cpu_ref		Return per-CPU data for other CPUs
 *
 *-SYNOPSIS:
 *	#include <kernel/ddi_cpu.h>
 *
 *	dcdata_t * ddi_cpu_ref (processorid_t cpu);
 *
 *-ARGUMENTS:
 *	cpu		ID code for processor whose data needs to be accessed.
 *
 *-DESCRIPTION:
 *	The implementations of DDI/DKI facilities may wish to write into the
 *	private data areas of other CPUs. This function provides a way of
 *	accessing that information.
 *
 *	Note that the per-CPU data area provides many conveniences to the
 *	implementor, not the least of which is latitude with synchronization,
 *	which has a data space cost if not a run-time cost, and also makes
 *	many operations considerably simpler. Access by other CPUs to per-CPU
 *	data bypasses most of the assumptions that can be conveniently made
 *	about access to this area, so users of this function are especially
 *	cautioned to use all the appropriate interlock mechanisms when writing
 *	data via the pointer this function provides.
 *
 *	There must be exactly one call to ddi_cpu_unref () for each call to
 *	ddi_cpu_ref (). This should be done as soon as possible after the
 *	call to ddi_cpu_ref () since on some architectures making this memory
 *	shared may have a serious impact on performance.
 *
 *-RETURN VALUE:
 *	NULL is returned if the cpu ID is not valid for the machine
 *	configuration. Otherwise, a pointer to the per-process data for the
 *	indicated CPU is returned.
 *
 *-LEVEL:
 *	Base or interrupt.
 *
 *-NOTES:
 *	Does not sleep.
 *
 *	It is as yet uspecified whether recursive calls to ddi_cpu_ref () are
 *	permitted.
 */

#if	__USE_PROTO__
dcdata_t * (ddi_cpu_ref) (processorid_t cpu)
#else
dcdata_t *
ddi_cpu_ref __ARGS ((cpu))
processorid_t	cpu;
#endif
{
	/*
	 * Only one processor, as yet.
	 */

	return cpu == 0 ? ddi_cpu_data () : NULL;
}

