int __bolt;
/* $Header: /ker/coh.386/RCS/clock.c,v 2.6 93/10/29 00:55:00 nigel Exp Locker: nigel $ */
/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or persuant to the license agreement is unlawful.
 *
 *	COHERENT Version 5.0
 *	Copyright (c) 1982, 1993.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
/*
 * The clock comes in two parts.  There is the routine `clock' which
 * gets called every tick at high priority.  It does the minimum it
 * can and returns as soon as possible.  The second routine, `stand',
 * gets called whenever we are about to return from an interrupt to
 * user mode at low priority.  It can look at flags that the clock set
 * and do the things the clock really wanted to do but didn't have time.
 * Stand is truly the kernel of the system.
 *
 * $Log:	clock.c,v $
 * Revision 2.6  93/10/29  00:55:00  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.5  93/09/02  18:03:30  nigel
 * Remove spurious globals and unusable #ifdef'd code
 * 
 * Revision 2.4  93/08/19  03:26:22  nigel
 * Nigel's r83 (Stylistic cleanup)
 *
 * 90/08/13	Hal Snyder		/usr/src/sys/coh/clock.c
 * Add external altclk to allow polled device drivers.
 * (extern'ed in coherent.h)
 * 
 * 87/10/26	Allan cornish		/usr/src/sys/coh/clock.c
 * Timed functions are now invoked with TIM * tp as second argument.
 * This facilitates the use of timed functions within loadable drivers.
 *
 * 87/07/07	Allan Cornish		/usr/src/sys/coh/clock.c
 * Clocks static variable added - incremented by clock, decremented by stand().
 * Lbolt variable added - clock ticks since startup - incremented by stand().
 * Support for multiple timing queues ported from RTX.
 *
 * 87/01/05	Allan Cornish		/usr/src/sys/coh/clock.c
 * stand() now only wakes &stimer if swap timer is active.
 *
 * 86/11/24	Allan Cornish		/usr/src/sys/coh/clock.c
 * Added support for new t_last field in tim struct.
 *
 * 86/11/19	Allan Cornish		/usr/src/sys/coh/clock.c
 * Stand() calls defend() to execute functions deferred from interrupt level.
 */

#include <common/_gregset.h>
#include <common/_tricks.h>
#include <kernel/ddi_glob.h>
#include <kernel/ddi_cpu.h>
#include <sys/cmn_err.h>
#include <sys/stat.h>
#include <stddef.h>

#define	_KERNEL		1

#include <kernel/_timers.h>
#include <kernel/sig_lib.h>
#include <kernel/timeout.h>
#include <sys/con.h>
#include <sys/uproc.h>
#include <sys/proc.h>
#include <sys/sched.h>
#include <sys/mmu.h>


__EXTERN_C_BEGIN__

int		__RUN_INT_DEFER	__PROTO ((defer_t * deferp));

__EXTERN_C_END__


int (*altclk)();	/* pointer to higher-speed clock function */

static	int 	clocks;
static	int	broken_clocks;

#if	TRACER

extern	char	stext;
extern	char	__end_text;

#endif

#if	_USE_IDLE_PROCESS
extern	__proc_t      *	iprocp;
#endif


/*
 * This routine is the raw clock-interrupt handler, which is normally the
 * highest-priority interrupt task in the system to avoid lost clock ticks.
 */

#if	__USE_PROTO__
void clock_intr (int __NOTUSED (arg), __VOID__ * __NOTUSED (intstuff),
		 gregset_t regset)
#else
void
clock_intr (arg, intstuff, regset)
int		arg;
__VOID__      *	intstuff;
gregset_t	regset;
#endif
{
	/*
	 * Hook for alternate clock interrupt;
	 * Call polling function ("altclk") if there is one.
	 *
	 * For near function, "altsel" is 0 and "altclk" is offset.
	 * For far function, "altsel" is the CS selector and "altclk"
	 * is the offset.
	 *
	 * Since the polling function ends with a near rather than
	 * far return, far invocation is via ld_call() (ldas.s) which uses
	 * the despatch routine at CS:4 (ld.s) in any loadable driver.
	 */
	if (altclk && (* altclk) ())
		return;

__bolt++;

	/*
	 * Update timers.  Decrement time slice.
	 */

	clocks += 1;
	timer.t_tick += 1;

	/* Ensure we don't bill the wrong process.
	 *
	 * The global variable "quantum" is how many clock ticks remain
	 * for the current running process.
	 */
	if (ddi_cpu_data()->dc_int_level <= 1)
	  quantum -= 1;

	/*
	 * Tax current process and update his times.
	 */

	if (__IS_USER_REGS (& regset)) {
		SELF->p_utime ++;
		u.u_ppc = (caddr_t) __PC_REG (& regset);
	} else if (! __IS_CPU_IDLE ())
		SELF->p_stime ++;
	else
		ddi_cpu_data ()->dc_idle_ticks ++;

#if 0
	/* Watch size of free clist pool. */
	{
		int		i;
		unsigned long	pc;
		extern int	nclfree;

		pc = nclfree;

		for (i = 0 ; i < 8 ; i ++) {
			(* __PTOV (0xB8020UL - i * 2)) =
				"0123456789ABCDEF" [pc & 0x0F];
			pc >>= 4;
		}
	}
#endif

#if	0
	/*
	 * Put a blob on the screen showing the interrupt nesting level and
	 * the caller's PC.
	 */

	(* __PTOV (0xB8000UL)) = ddi_cpu_data ()->dc_int_level + '0';

	if ((timer.t_tick % 50U) != 0)
		return;

	{
		int		i;
		unsigned long	pc;

		pc = st_maxavail (ddi_global_data ()->dg_kmem_heap);

		for (i = 0 ; i < 8 ; i ++) {
			(* __PTOV (0xB8020UL - i * 2)) =
				"0123456789ABCDEF" [pc & 0x0F];
			pc >>= 4;
		}

		if (st_assert (ddi_global_data ()->dg_kmem_heap) < 0) {
			char	      *	dest = __PTOV (0xB8050UL);

			memcpy (dest, "B A D   H E A P ", 16);
		}
	}
#endif
}


/*
 * return_from_interrupt ()
 *
 * Called when there has been an interrupt and the system is about to return
 * from the topmost interrupt context. There may not be any valid process
 * context at this point.
 */

#if	__USE_PROTO__
void return_from_interrupt (void)
#else
void
return_from_interrupt ()
#endif
{
	/*
	 * Check the timed function queue if necessary. Note that we don't
	 * manipulate the interrupt mask because there should only be one
	 * context active here at once. Code in the interrupt-dispatch routine
	 * and below in return_to_user () deals with ensuring this.
	 */

	while (clocks) {
		broken_clocks ++;
		clocks --;

		/*
		 * Check the sane DDI/DKI timeout system.
		 */

		CHECK_TIMEOUT ();
	}

	/*
	 * Run the DDI/DKI version of the deferred function system.
	 */

	__CHEAP_DISABLE_INTS ();

	while (__RUN_INT_DEFER (& ddi_global_data ()->dg_defint) ||
	       __RUN_INT_DEFER (& ddi_cpu_data ()->dc_defint))
		;	/* DO NOTHING */

	__CHEAP_ENABLE_INTS ();
}


/*
 * check_clock ()
 *
 * This routine deals with the stuff normally done before return to user level
 * that doesn't relate to the current process. This routine could be called
 * within an idle look in dispatch () if we don't want to be running a special
 * idle process.
 */

void
check_clock ()
{
	int		outflag = 0;

	LOCK_DISPATCH ();

	/*
	 * Update the clock. This ought to be in the return_from_interrupt ()
	 * section; needless to say, the stupidity of those who came before
	 * prevents this...
	 */

	while (timer.t_tick >= HZ) {
		timer.t_time ++;
		timer.t_tick -= HZ;
		outflag = 1;
	}


	/*
	 * Timeout any devices.
	 */

	if (outflag) {
		int		n;

		for (n = 0 ; n < drvn ; n ++) {
			short		s;

			if (drvl [n].d_time == 0)
				continue;

			s = sphi ();
			dtime ((dev_t) makedev (n, 0));
			spl (s);
		}
	}


	/*
	 * As is typical for Coherent, the users of this garbage don't
	 * perform any form of synchronization at all.
	 */

	while (broken_clocks) {
		short		s;
		TIM	      *	np;
		TIM	      *	tp;

		/*
		 * Update [serviced] clock ticks since startup.
		 */

		lbolt ++;
		broken_clocks --;

		/*
		 * Remove timing list from queue, creating new temporary queue.
		 */

		s = sphi ();

		/*
		 * Scan timing list.
		 */

		for (np = timq [lbolt % __ARRAY_LENGTH (timq)];
		     (tp = np) != NULL ;) {
			/*
			 * Remember next function in timing list.
			 * NOTE: Must be done before function is invoked,
			 *	 since it may start a new timer.
			 */

			np = tp->t_next;

			/*
			 * Function has not timed out: leave it on timing list.
			 */

			if (tp->t_lbolt != lbolt)
				continue;

			/*
			 * Remove function from timing list.
			 */

			if ((tp->t_last->t_next = tp->t_next) != NULL)
				tp->t_next->t_last = tp->t_last;
			tp->t_last = NULL;

			/*
			 * Invoke function.
			 */
#if	TRACER
			if ((char *) tp->t_func < (char *) & stext ||
			    (char *) tp->t_func > (char *) & __end_text)
				cmn_err (CE_PANIC,
					 "Bad timeout function %x in timer %x",
					 (unsigned) tp->t_func,
					 (unsigned) tp);
#endif

			spl (s);
			(* tp->t_func) (tp->t_farg, tp);
			sphi ();
		}

		spl (s);
	}


	/*
	 * Execute deferred functions.
	 */

	defend ();
}


/*
 * return_to_user ()
 *
 * Called when there is an interrupt or trap, and the system is about to
 * return to user mode (or to the idle process, if there is one).
 */

void
return_to_user (regset)
gregset_t	regset;
{
	check_clock ();

	/*
	 * Check expiration of quantum.
	 */

	if (quantum <= 0) {
		quantum = 0;
		disflag = 1;
	}


	/*
	 * Do profiling.
	 */

	if (u.u_pscale & ~ 1) {	/* if scale is not zero or one */
		/*
		 * Treat u.u_pscale as fixed-point fraction 0xXXXX.YYYY.
		 * Increment the (short) profiling entry at
		 *	base + (pc - offset) * scale
		 */

		caddr_t a;

		a = (caddr_t) ((long) (u.u_pbase + pscale (u.u_ppc - u.u_pofft,
							   u.u_pscale)) & ~ 1);
		if (a < u.u_pbend)
			putusd (a, getusd (a) + 1);
	}


	/*
	 * Check for signals and execute them; we pass in the address of the
	 * user's process context rather than depend on the state of the
	 * "u.u_regl" global idiocy.
	 */

	curr_check_signals (& regset, NULL);


	/*
	 * Should we dispatch? Note that we don't have to check for whether
	 * there is an active process because the fact we are executing is
	 * sufficient.
	 */

	if ((SELF->p_flags & PFDISP) != 0) {
		SELF->p_flags &= ~ PFDISP;
		disflag = 1;
	}


	/*
	 * Redispatch.
	 */

	if (disflag) {
		short		s = sphi ();
#if	_USE_IDLE_PROCESS
		/*
		 * If we are not running the idle process, save the context of
		 * the current process.
		 */

		if (SELF != iprocp)
			process_set_runnable (SELF);
#endif
		dispatch ();
		spl (s);
	}


	/*
	 * We call the return-from-interrupt code to give early service to
	 * deferred functions queued while we were running at base level.
	 *
	 * We maintain the CPU state variables to prevent an interrupt context
	 * from entering the return_from_interrupt () code. This is required
	 * if we want to assume that return_to_interrupt () is only active in
	 * one context.
	 */

	ASSERT_BASE_LEVEL ();

	ddi_cpu_data ()->dc_int_level ++;
	return_from_interrupt ();
	ddi_cpu_data ()->dc_int_level --;
}
