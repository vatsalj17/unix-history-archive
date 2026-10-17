/* $Header: $ */

#define	_DDI_DKI	1
#define	_DDI_DKI_IMPL	1
#define	_SYSV4		1

/*
 * Definition for the actual definitions of DDI/DKI globals.
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
#include <kernel/ddi_glob.h>


/*
 * Here we have the actal static data declarations for the uniprocessor
 * implementation of the DDI/DKI per-CPU data. Note that under a "real"
 * system, the dynamic area would be the space left over by the fixed data up
 * to the size of a page, because that seems the best (read fastest and most
 * secure) way of doing it (by mapping each processor's private data into the
 * same address across all processors). Of course, it's questionable whether
 * any drivers *will* actually use the binary-compatible function-versions of
 * the accessors, but that's not our problem.
 *
 * Don't forget that if we *do* use the paging system for per-CPU data that
 * those pages will have to be mapped somewhere else as well, because there is
 * data like the deferred function table that the world may want controlled
 * access to.
 */

dcdata_t	__ddi_cpu_data [1];
dgdata_t	__ddi_global_data;
char		__ddi_cpu_dynarea [512];

