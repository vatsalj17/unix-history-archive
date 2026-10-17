/* $Header: $ */

#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1
#define	_SYSV4		1

/*
 * Definition for the DDI/DKI linkable version of ddi_proc_data ()
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

#include <kernel/ddi_proc.h>

/*
 * Pull in the bogus Coherent junk.
 */

#define	_KERNEL		1

#if	__COHERENT__

#include <sys/proc.h>
#define	CURRENT_PROCESS()	cprocp

#elif	defined (__MSDOS__)

# include <sys/dosproc.h>

#endif

#define	ddi_proc_data()	((dpdata_t *) & CURRENT_PROCESS ()->p_ddi_space)


/*
 *-STATUS:
 *	Local DDI/DKI extension
 *
 *-NAME:
 *	ddi_proc_data		Get per-process DDI/DKI global data.
 *
 *-SYNOPSIS:
 *	#include <kernel/ddi_proc.h>
 *
 *	dpdata_t * ddi_proc_data (void);
 *
 *-DESCRIPTION:
 *	This function returns a base pointer to a table of information that
 *	the DDI/DKI needs to associate with a process but may need to access
 * 	outside the process context.
 *
 *-RETURN VALUE:
 *	The base address of a per-process DDI/DKI data table entry. The
 *	value returned may be considered "constant" and memoized within a
 *	context provided that no rescheduling may occur.
 *
 *-LEVEL:
 *	Base or interrupt.
 *
 *-NOTES:
 *	This function does not sleep.
 */


#if	__USE_PROTO__
dpdata_t * (ddi_proc_data) (void)
#else
dpdata_t *
ddi_proc_data __ARGS (())
#endif
{
	return ddi_proc_data ();
}

