#
#include "param.h"
#include "systm.h"
#include "user.h"
#include "proc.h"
#include "reg.h"
#include "seg.h"

#define	EBIT	010	/*** user err bit in psw: C-bit ***/
#define	UMODE	01<<8	/*** user-mode bit in tsw (prot bit) ***/
#define	SYS	0341	/*** svc (trap) instruction ***/
#define	USER	020	/* user-mode flag added to dev */

/*
 * structure of the system entry table (sysent.c)
 */
extern struct sysent	{
	int	count;		/* argument count */
	int	(*call)();	/* name of handler */
} sysent[64];

/*
 * Offsets of the user's registers relative to
 * the saved r0. See reg.h
 */
int	regloc[18]
{
	R0,  R1,  R2,  R3,  R4,  R5,  R6,  R7,		/***/
	R8,  R9,  R10, R11, R12, R13, R14, R15,		/***/
	RPS, RPC
};

/*
 * Called from low.s when a processor trap occurs.
 * The arguments are the words saved on the system stack
 * by the software during the trap processing.
 * Their order is dictated by the details
 * of C's calling sequence. They are peculiar in that
 * this call is not 'by value' and changed user registers
 * get copied back on return.
 * dev is the kind of trap that occurred.
 */
trap(dev, stat, r0, r1, r2, r3, r4, r5, r6, asp, rf, ps, pc, nps) /***/
{
	register i, a;
	register struct sysent *callp;

	trace(02, "trap", dev);
	trace(02, "psw", pc);

#ifdef FPREGS
	savfp();
#endif

	if ((ps&UMODE) == UMODE)
		dev =| USER;
	u.u_ar0 = &r0;
	switch(dev) {

	/*
	 * Trap not expected.
	 * Usually a kernel mode address error.
	 */
	default:
		printf("Useg = %x\n", *ka6);
		printf("PSW = %x %x\n", ps, pc);
		printf("SP = %x\n", asp);
		printf("trap type %d\n", dev);
		panic("trap");

	case 0+USER: /* address error */
		i = SIGBUS;
		break;

	case 1+USER: /* illegal instruction */

#ifdef FPTRAP
	/****	if illegal instruction assume it to be a floating
	point one and call fptrap.
		passing fptrap a pointer to the user's saved r0
	( see reg.h ) and a pointer to his fp regs in the ppda  ****/
		if ( i=fptrap(&r0,u.u_fsav))
			break;			/***  error returned by fptrap ***/
		goto out;
#else
		i = SIGINS;
		break;
#endif

	case 2+USER: /* bpt or trace */
		i = SIGTRC;
		break;

	case 3+USER: /* iot */
		i = SIGIOT;
		break;

	case 5+USER: /* emt */
		i = SIGEMT;
		break;

	case 6+USER: /* sys call */
		u.u_error = 0;
		ps =& ~EBIT;
		callp = &sysent[stat&077];	/*** stat is svc no. ***/
		if (callp == sysent) { /* indirect */
			a = fuiword(pc);
			pc =+ 4;		/***/
			i = fuword(a);
			if ((i >> 24) != SYS)
				i = 077;	/* illegal */
			callp = &sysent[i&077];
			for(i=0; i<callp->count; i++)
				u.u_arg[i] = fuword(a =+ 4);	/***/
		} else {
			for(i=0; i<callp->count; i++) {
				u.u_arg[i] = fuiword(pc);
				pc =+ 4;		/***/
			}
		}
		trace(04, "svc", callp-sysent);
		u.u_dirp = u.u_arg[0];
		trap1(callp->call);
		if(u.u_intflg)
			u.u_error = EINTR;
		if(u.u_error < 100) {
			if(u.u_error) {
				ps =| EBIT;
				r0 = u.u_error;
			}
			goto out;
		}
		i = SIGSYS;
		break;

	/*
	 * Since the floating exception is an
	 * imprecise trap, a user generated
	 * trap may actually come from kernel
	 * mode. In this case, a signal is sent
	 * to the current process to be picked
	 * up later.
	 */
	case 8: /* floating exception */
		psignal(u.u_procp, SIGFPT);
		return;

	case 8+USER:
		i = SIGFPT;
		break;

	/*
	 * If the user SP is below the stack segment,
	 * grow the stack automatically.
	 * This relies on the ability of the hardware
	 * to restart a half executed instruction.
	 * At the moment this is unimplemented (and possibly
	 * unimplementable) on the Interdata.
	 */
	case 9+USER: /* segmentation exception */

/***
		a = asp;
		if(backup(u.u_ar0) == 0)
		if(grow(a))
			goto out;
***/

		i = SIGSEG;
		break;

	case 10+USER: /* illegal svc number */
		i = SIGSYS;
		break;
	}
	psignal(u.u_procp, i);

out:
	if(issig())
		psig();
	setpri(u.u_procp);
}

/*
 * Call the system-entry routine f (out of the
 * sysent table). This is a subroutine for trap, and
 * not in-line, because if a signal occurs
 * during processing, an (abnormal) return is simulated from
 * the last caller to savu(qsav); if this took place
 * inside of trap, it wouldn't have a chance to clean up.
 *
 * If this occurs, the return takes place without
 * clearing u_intflg; if it's still set, trap
 * marks an error which means that a system
 * call (like read on a typewriter) got interrupted
 * by a signal.
 */
trap1(f)
int (*f)();
{

	u.u_intflg = 1;
	savu(u.u_qsav);
	(*f)();
	u.u_intflg = 0;
}

/*
 * nonexistent system call-- set fatal error code.
 */
nosys()
{
	u.u_error = 100;
}

/*
 * Ignored system call
 */
nullsys()
{
}
