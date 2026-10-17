/* $Header: $ */

#define	_DDI_DKI	1
#define	_SYSV3		1

/*
 * Code for executing the functions stored in the DDI/DKI version of the
 * old Coherent deferred-function table.
 *
 * $Log: $
 */

#include <kernel/ddi_cpu.h>
#include <kernel/ddi_glob.h>
#include <sys/confinfo.h>
#include <sys/types.h>
#include <sys/cmn_err.h>
#include <stdlib.h>

/*
 * Since we don't have a real trap-handler, I have factored out the deferred-
 * function check to here.
 *
 * Entered with interrupts disabled. This routine must be able to enable
 * interrupts without causing excess stack growth. Note that if we have
 * interrupt prologue and epilogue code in assembly language, a rather
 * different loop structure might work out simpler.
 */

#if	__USE_PROTO__
int (__RUN_INT_DEFER) (defer_t * deferp)
#else
int
__RUN_INT_DEFER __ARGS ((deferp))
defer_t	      *	deferp;
#endif
{
	int		recheck = 0;
	int		idx;

	/*
	 * If we detected a non-empty global defer table, try and lock the
	 * table before processing the deferred routines. Only try once; if
	 * the table is busy, then someone else must be dealing with it.
	 *
	 * Note that we return 0 rather than one if we encounter the table
	 * locked; if we are on a uniprocessor system, if a nested interrupt
	 * context finds this situation it should not loop! Of course, it may
	 * be desirable to prevent this on a uniprocessor at a higher level
	 * anyway, but we can't assume that to be the case. Thanks to Louis
	 * Giliberto for pointing this out.
	 */

	if (ATOMIC_FETCH_AND_STORE_UCHAR (deferp->df_rlock, 1) != 0)
		return 0;

	while ((idx = ATOMIC_FETCH_UCHAR (deferp->df_read)) !=
			ATOMIC_FETCH_UCHAR (deferp->df_write)) {
		__CHEAP_ENABLE_INTS ();

		(* deferp->df_tab [idx ++]) ();

		if (idx == ATOMIC_FETCH_UCHAR (deferp->df_max))
			idx = 0;

		ATOMIC_STORE_UCHAR (deferp->df_read, idx);

		recheck = 1;

		__CHEAP_DISABLE_INTS ();
	}

	/*
	 * Release the lock on the global defer table.
	 */

	ATOMIC_STORE_UCHAR (deferp->df_rlock, 0);

	return recheck;
}

