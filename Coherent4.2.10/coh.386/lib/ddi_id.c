/* $Header: $ */

#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1
#define	_SYSV4		1

/*
 * Definition for the DDI/DKI linkable version of ddi_cpu_id ()
 *
 * $Log: $
 */

/*
 *-IMPORTS:
 *	<common/ccompat.h>
 *		__USE_PROTO__
 *		__ARGS ()
 */

#include <common/ccompat.h>

#include <kernel/ddi_cpu.h>


/*
 *-STATUS:
 *	For the Implementors only.
 *
 *-NAME:
 *	ddi_cpu_id	Determine the current CPU id.
 *
 *-SYNOPSIS:
 *	#include <kernel/ddi_cpu.h>
 *
 *	processorid_t ddi_cpu_id (void);
 *
 *-DESCRIPTION:
 *	ddi_cpu_id () allows DDI/DKI code a way of accessing the current CPU
 *	id for passing to functions such as dtimeout () or defer_int_cpu ().
 *
 *-RETURN VALUE:
 *	The id code of the CPU that the caller is executing on.
 *
 *-LEVEL:
 *	Base or interrupt.
 *
 *-NOTES:
 *	Does not sleep.
 */

#if	__USE_PROTO__
processorid_t (ddi_cpu_id) (void)
#else
processorid_t
ddi_cpu_id __ARGS (())
#endif
{
	return ddi_cpu_data ()->dc_cpuid;
}

