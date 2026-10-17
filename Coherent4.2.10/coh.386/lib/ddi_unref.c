/* $Header: $ */

#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1
#define	_SYSV4		1

/*
 * Definition for the DDI/DKI linkable version of ddi_proc ()
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
 *		memset ()
 */

#include <common/ccompat.h>
#include <stddef.h>

#include <kernel/ddi_cpu.h>


/*
 *-STATUS:
 *	For the Implementors only.
 *
 *-NAME:
 *	ddi_cpu_unref		Return per-CPU data for other CPUs
 *
 *-SYNOPSIS:
 *	#include <kernel/ddi_cpu.h>
 *
 *	void ddi_cpu_unref (dcdata_t * data);
 *
 *-ARGUMENTS:
 *	data		A pointer to per-CPU data obtained via the
 *			ddi_cpu_ref () function.
 *
 *-DESCRIPTION:
 *	This function is used to release a reference to per-CPU data for some
 *	CPU that was obtained by ddi_cpu_ref (). This function may perform
 *	various actions to undo any work necessary to ensure that the memory
 *	in question can be accessed by the calling CPU, such as unmapping it
 *	from the calling CPU's address space to provide maximum protection
 *	against accidental damage.
 *
 *	There must be exactly one call to ddi_cpu_unref () for each call to
 *	ddi_cpu_ref ().
 *
 *-RETURN VALUE:
 *	None.
 *
 *-LEVEL:
 *	Base or interrupt.
 *
 *-NOTES:
 *	Does not sleep.
 */

#if	__USE_PROTO__
void (ddi_cpu_unref) (dcdata_t * data)
#else
void
ddi_cpu_unref __ARGS ((data))
dcdata_t      *	data;
#endif
{
	ASSERT (data != NULL && data == ddi_cpu_data ());
}

