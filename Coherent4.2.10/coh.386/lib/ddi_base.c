/* $Header: $ */

#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1
#define	_SYSV4		1

/*
 * Definition for the DDI/DKI linkable version of ddi_base_data ().
 *
 * $Log: $
 */

/*
 *-IMPORTS:
 *	<common/ccompat.h>
 *		__USE_PROTO__
 *		__ARGS ()
 *	<kernel/ddi_cpu.h>
 *		ASSERT_BASE_LEVEL ()
 */

#include <common/ccompat.h>
#include <kernel/ddi_cpu.h>

#include <kernel/ddi_base.h>


#if	__COHERENT__

/*
 *-IMPORTS:
 *	<sys/io.h>
 *		IO
 *	<sys/uproc.h>
 *		UPROC
 *		u.u_uid
 *		u.u_gid
 */

#include <sys/io.h>


/*
 * Get the Coherent grunge.
 */

#define	_KERNEL		1

#include <sys/uproc.h>
#define	U_AREA()		(& u)

#elif	defined (__MSDOS__)

# include <kernel/dosproc.h>

#endif


#define	ddi_base_data()		((dbdata_t *) U_AREA ()->u_nigel)


/*
 *-STATUS:
 *	Local DDI/DKI extension
 *
 *-NAME:
 *	ddi_base_data		Get per-process DDI/DKI base data.
 *
 *-SYNOPSIS:
 *	#include <kernel/ddi_base.h>
 *
 *	dbdata_t * ddi_base_data (void);
 *
 *-DESCRIPTION:
 *	This function returns a base pointer to a table of information that
 *	the DDI/DKI needs to associate with a process but will not need to
 * 	access outside that process context.
 *
 *-RETURN VALUE:
 *	The base address of a per-process DDI/DKI data table entry. The
 *	value returned may be considered "constant" and memoized within a
 *	context provided that no rescheduling may occur.
 *
 *-LEVEL:
 *	Base only.
 *
 *-NOTES:
 *	This function does not sleep.
 */


#if	__USE_PROTO__
dbdata_t * (ddi_base_data) (void)
#else
dbdata_t *
ddi_base_data __ARGS (())
#endif
{
	ASSERT_BASE_LEVEL ();

	return ddi_base_data ();
}

