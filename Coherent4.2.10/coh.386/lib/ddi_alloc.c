/* $Header: $ */

#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1
#define	_SYSV4		1

/*
 * Define a way of allocating some per-CPU data from a small pool (note that
 * one allocated, we cannot free). This is aimed at making the actual per-
 * cpu data structure more resistant to structural alteration.
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
 *	<string.h>
 *		memset ()
 */

#include <common/ccompat.h>
#include <stddef.h>
#include <string.h>

#include <kernel/ddi_cpu.h>


/*
 *-STATUS:
 *	For the Implementors only.
 *
 *-NAME:
 *	ddi_cpu_alloc	Dynamically allocates per-CPU data.
 *
 *-SYNOPSIS:
 *	#include <kernel/ddi_cpu.h>
 *
 *	void * ddi_cpu_alloc (size_t size);
 *
 *-ARGUMENTS:
 *	size		The size in bytes to allocate. This value will always
 *			be rounded up to the size of an integer to ensure the
 *			alignment of the returned space. A value of 0 is not
 *			legal and will result in the function returning NULL.
 *
 *-DESCRIPTION:
 *	ddi_cpu_alloc () is for use in the DDI/DKI implementation to request
 *	per-CPU data in a flexible way. Facilities that require tables of per-
 *	CPU data should request it at startup time to allow the table sizes to
 *	be flexibly administered and also to avoid polluting the "dcdata_t"
 *	structure with information whose size cannot easily be fixed. Using
 *	this facility should allow for greater levels of internal binary
 *	compatibility by restricting the amount of change the "dcdata_t"
 *	structure can undergo between (minor) releases of the operation
 *	system.
 *
 *-RETURN VALUE:
 *	On failure, NULL is returned. On success, a pointer to the requested
 *	data is returned. The pointer returned will be aligned to the size of
 *	an integer on the target machine architecture.
 *
 *-LEVEL:
 *	Base only.
 *
 *-NOTES:
 *	Does not sleep.
 */

#if	__USE_PROTO__
__VOID__ * (ddi_cpu_alloc) (size_t size)
#else
__VOID__ *
ddi_cpu_alloc __ARGS ((size))
size_t		size;
#endif
{
	dcdata_t      *	dcdatap = ddi_cpu_data ();
	__VOID__      *	alloc;

	ASSERT_BASE_LEVEL ();

	size = (size + sizeof (int) - 1) & ~ sizeof (int);

	if (size == 0 || dcdatap->dc_dynalloc + size > dcdatap->dc_dynend)
		return NULL;

	alloc = dcdatap->dc_dynalloc;
	dcdatap->dc_dynalloc += size;

        memset (alloc, 0, size);
	return alloc;
}

