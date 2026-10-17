/* $Header: $ */

#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1
#define	_SYSV4		1

/*
 * Definition for the DDI/DKI linkable version of ddi_global_data ().
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

#include <kernel/ddi_glob.h>


/*
 *-STATUS:
 *	Local DDI/DKI extension
 *
 *-NAME:
 *	ddi_global_data		Get DDI/DKI global data.
 *
 *-SYNOPSIS:
 *	#include <kernel/ddi_glob.h>
 *
 *	dgdata_t * ddi_global_data (void);
 *
 *-DESCRIPTION:
 *	This function returns a base pointer to a table of information that
 *	represents the global state of the DDI/DKI subsystem, with the
 *	possible exception of the STREAMS global state.
 *
 *-RETURN VALUE:
 *	The base address of the global DDI/DKI data table. The value returned
 *	may be considered "constant" and memoized within a context provided
 *	that no rescheduling may occur.
 *
 *-LEVEL:
 *	Base or interrupt.
 *
 *-NOTES:
 *	This function does not sleep.
 */


#if	__USE_PROTO__
dgdata_t * (ddi_global_data) (void)
#else
dgdata_t *
ddi_global_data __ARGS (())
#endif
{
	return ddi_global_data ();
}

