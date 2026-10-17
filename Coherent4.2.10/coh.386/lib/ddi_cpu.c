/* $Header: $ */

#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1
#define	_SYSV4		1

/*
 * Definition for the DDI/DKI linkable version of ddi_cpu_data ()
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
 *	Local DDI/DKI extension
 *
 *-NAME:
 *	ddi_cpu_data		Get per-CPU DDI/DKI global data.
 *
 *-SYNOPSIS:
 *	#include <kernel/ddi_cpu.h>
 *
 *	dcdata_t * ddi_cpu_data (void);
 *
 *-DESCRIPTION:
 *	This function returns a base pointer to a table of information that
 *	can be considered per-CPU DDI/DKI static data. The value returned
 *	may be considered "constant" and memoized within a context provided
 *	that no rescheduling could occur.
 *
 *-RETURN VALUE:
 *	The base address of a per-CPU DDI/DKI data table entry.
 *
 *-LEVEL:
 *	Base or interrupt.
 *
 *-NOTES:
 *	This function does not sleep.
 */

#if	__USE_PROTO__
dcdata_t * (ddi_cpu_data) (void)
#else
dcdata_t *
ddi_cpu_data __ARGS (())
#endif
{
	return ddi_cpu_data ();
}

