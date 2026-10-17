#
#include "../defines.h"
#include "../param.h"
#ifdef	AUSAML
#include	"../lnode.h"
#endif	AUSAML
#include "../systm.h"
#include "../file.h"
#include "../user.h"
#include "../proc.h"
#include "../reg.h"
#include "../seg.h"

#define	EBIT	1		/* user error bit in PS: C-bit */
#ifdef	SETPSW
#define	UMODE	0140000		/* user-mode bits in PS word */
#endif
#ifndef	SETPSW
#define	UMODE	0170000		/* user-mode bits in PS word */
#endif
#define	SETD	0170011		/* SETD instruction */
#define	SYS	0104400		/* sys (trap) instruction */
#define	USER	020		/* user-mode flag added to dev */

char *panicstr;		/* fix016 */
/*
 * structure of the system entry table (sysent.c)
 */
struct sysent	{
	int	count;		/* argument count */
	int	(*call)();	/* name of handler */
} sysent[64];

#ifdef SYS_CALL_PROFILE
long	traptable[0100 + 1];	/* N_SYS_CALLS + 1 for illegal */
int	trapword;
#endif SYS_CALL_PROFILE

/*
 * Offsets of the user's registers relative to
 * the saved r0. See reg.h
 */
char	regloc[9]
{
	R0, R1, R2, R3, R4, R5, R6, R7, RPS
};

/*
 * Called from l40.s or l45.s when a processor trap occurs.
 * The arguments are the words saved on the system stack
 * by the hardware and software during the trap processing.
 * Their order is dictated by the hardware and the details
 * of C's calling sequence. They are peculiar in that
 * this call is not 'by value' and changed user registers
 * get copied back on return.
 * dev is the kind of trap that occurred.
 */
#ifndef	_1170 | _1145
trap(dev, sp, r1, nps, r0, pc, ps)
{
	register i, a;
	register struct sysent *callp;

	savfp();
	if ((ps&UMODE) == UMODE) {
		dev =| USER;
/*		if( issig() ) goto out1;	/* fix005 */
	}
	u.u_ar0 = &r0;
	switch(dev) {

#ifdef	MEM_PAR_INTR
	/*
	 * memory parity
	 */
	case 10+USER:
	case 10:
		printf( "\n\nmemory parity ");
		i = PARMEMCSR;
		while ( (a = *i.integptr++) >= 0 ) ;
		/* if we run out of registers before a parity error
		   we may get a bus error */
		if( (dev&USER)==0 ) panicstr++;
		printf( "address = %o000\n", (a>>3)&0774 );
		if( dev&USER ) {
			i = SIGMEMPAR;
			break;
		}
#endif	MEM_PAR_INTR
	/*
	 * Trap not expected.
	 * Usually a kernel mode bus error.
	 * The numbers printed are used to
	 * find the hardware PS/PC as follows.
	 * (all numbers in octal 18 bits)
	 *	address_of_saved_ps =
	 *		(ka6*0100) + aps - 0140000;
	 *	address_of_saved_pc =
	 *		address_of_saved_ps - 2;
	 */
	default:
		panicstr++;	/* fix016  - indicate about to panic
				   and what follows is vital !! */
		printf("\n\nka6 = %o\n", *ka6);
#ifdef	BIG_UNIX
	/*
	 * Print Ka5 as well for big 11/40 systems
	 * You will need a system map to understand it.
	 */
		printf("ka5 = %o\n", *(ka6-1));
#endif
		printf("aps = %o\n", &ps);
		printf("trap type %o\n", dev);
		panic("trap");

	case 0+USER: /* bus error */
		i = SIGBUS;
		break;

	/*
	 * If illegal instructions are not
	 * being caught and the offending instruction
	 * is a SETD, the trap is ignored.
	 * This is because C produces a SETD at
	 * the beginning of every program which
	 * will trap on CPUs without 11/45 FPU.
	 */
	case 1+USER: /* illegal instruction */
		if(fuiword(pc-2) == SETD && u.u_signal[SIGINS] == 0)
			goto out;
		i = SIGINS;
		break;

	case 2+USER: /* bpt or trace */
		i = SIGTRC;
		ps =| 03400;		/* fix015 */
					/*
					 * unused bits...
					 * there is code in the
					 * assembler support routines
					 * to detect this and rtt instead of
					 * rti
					 */
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
#ifndef SYS_CALL_PROFILE
		callp = &sysent[fuiword(pc-2)&077];
#else SYS_CALL_PROFILE
		callp = &sysent[(trapword = fuiword(pc-2))&077];
		trapword =& 0377;
		traptable[trapword >= 0100 ? 0100 : trapword] ++;
#endif SYS_CALL_PROFILE
		if (callp == sysent) { /* indirect */
			a = fuiword(pc);
			pc =+ 2;
			i = fuword(a);
			if ((i & ~077) != SYS)
#ifndef SYS_CALL_PROFILE
				i = 0;	/* illegal  fix028 */
#else SYS_CALL_PROFILE
			{
				traptable[0100] ++;
				i = 0;	/* illegal  fix028 */
			}
			else
				traptable[i & 077] ++;
#endif SYS_CALL_PROFILE
			callp = &sysent[i&077];
			for(i=0; i<callp->count; i++)
				u.u_arg[i] = fuword(a =+ 2);
		} else {
			for(i=0; i<callp->count; i++) {
				u.u_arg[i] = fuiword(pc);
				pc =+ 2;
			}
		}
		if(u.u_error)
			goto badsys;	/* fix037 */
		u.u_dirp = u.u_arg[0];
		trap1(callp->call);
#ifndef	SHARED_DATA
		if(u.u_intflg)
#else
		if(u.u_flags&UINTFLG)
#endif	SHARED_DATA
			u.u_error = EINTR;
		if(u.u_error < 100) {
			if(u.u_error) {
				ps =| EBIT;
				r0 = u.u_error;
			}
			goto out;
		}
badsys:
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
	 * On the 11/40 this is not the case and
	 * the routine backup/l40.s may fail.
	 * The classic example is on the instruction
	 *	cmp	-(sp),-(sp)
	 */
	case 9+USER: /* segmentation exception */
		a = sp;
		if(backup(u.u_ar0) == 0)
		if(grow(a))
			goto out;
		i = SIGSEG;
		break;
	}
	psignal(u.u_procp, i);

out:
	if(issig())
out1:
		psig();
	curpri = setpri(u.u_procp);	/* fix025 */
}
#endif

#ifdef _1170 | _1145
trap(dev, sp, r1, nofault, r0, pc, ps)
{
	register i, a;
	register struct sysent *callp;
#ifdef	MEM_PAR_INTR | CRASH_TRACE
	extern	cpuereg, memereg, memhigha, memlowa;
#endif	MEM_PAR_INTR | CRASH_TRACE
#ifdef	SYS_TIME
	char hhmm[6];
#endif	SYS_TIME

	savfp();
	u.u_ar0 = &r0;
	switch(dev) {

	/*
	 * Trap not expected.
	 * Usually a kernel mode bus error.
	 * The numbers printed are used to
	 * find the hardware PS/PC as follows.
	 * (all numbers in octal 18 bits)
	 *	address_of_saved_ps =
	 *		(ka6*0100) + aps - 0140000;
	 *	address_of_saved_pc =
	 *		address_of_saved_ps - 2;
	 */
	default:
		panicstr++;	/* fix016  - indicate about to panic
				   and what follows is vital !! */
		printf("\n\nka6 = %o\n", *ka6);
		printf("aps = %o\n", &ps);
		printf("trap type %o\n", dev);
#ifdef	CRASH_TRACE
		printf("cpu-err = %o m-hi-a = %o m-lo-a = %o m-e-reg = %o\n",
				cpuereg, memhigha, memlowa, memereg);
#endif	CRASH_TRACE
		panic("trap");

	case 015:	/* Trap through zero */
		printf("Interrupt/0\n");
		return;

#ifdef	POWER_FAIL
	case 016:	/* power fail recovery complete */
#ifdef	SYS_TIME
		printf("power fail at %s\nrecovery complete\n", systime(hhmm) );
#else
		printf("power fail\nrecovery complete\n");
#endif	SYS_TIME
		return;

#endif	POWER_FAIL
	case 7:		/* user bus error */
		i = SIGBUS;
		break;

	/*
	 * If illegal instructions are not
	 * being caught and the offending instruction
	 * is a SETD, the trap is ignored.
	 * This is because C produces a SETD at
	 * the beginning of every program which
	 * will trap on CPUs without 11/45 FPU.
	 */
	case 2:		/* illegal instruction */
		if(fuiword(pc-2) == SETD && u.u_signal[SIGINS] == 0)
			goto out;
		i = SIGINS;
		break;

	case 3:		/* bpt or trace trap */
		i = SIGTRC;
		ps =| 03400;		/* fix015 */
					/*
					 * unused bits...
					 * there is code in the
					 * assembler support routines
					 * to detect this and rtt instead of
					 * rti
					 */
		break;

	case 5:		/* iot */
		i = SIGIOT;
		break;

	case 6:		/* emt */
		i = SIGEMT;
		break;

	case 4:		/* sys call */
		u.u_error = 0;
		ps =& ~EBIT;
#ifndef SYS_CALL_PROFILE
		callp = &sysent[fuiword(pc-2)&077];
#else SYS_CALL_PROFILE
		callp = &sysent[(trapword = fuiword(pc-2))&077];
		trapword =& 0377;
		traptable[trapword >= 0100 ? 0100 : trapword] ++;
#endif SYS_CALL_PROFILE
		if (callp == sysent) { /* indirect */
			a = fuiword(pc);
			pc =+ 2;
			i = fuword(a);
			if ((i & ~077) != SYS)
#ifndef SYS_CALL_PROFILE
				i = 0;	/* illegal  fix028 */
#else SYS_CALL_PROFILE
			{
				traptable[0100] ++;
				i = 0;	/* illegal  fix028 */
			}
			else
				traptable[i & 077] ++;
#endif SYS_CALL_PROFILE
			callp = &sysent[i&077];
			for(i=0; i<callp->count; i++)
				u.u_arg[i] = fuword(a =+ 2);
		} else {
			for(i=0; i<callp->count; i++) {
				u.u_arg[i] = fuiword(pc);
				pc =+ 2;
			}
		}
		if(u.u_error)		/* fix037 */
			goto badsys;
		u.u_dirp = u.u_arg[0];
		trap1(callp->call);
#ifndef	SHARED_DATA
		if(u.u_intflg)
#else
		if(u.u_flags&UINTFLG)
#endif	SHARED_DATA
			u.u_error = EINTR;
		if(u.u_error < 100) {
			if(u.u_error) {
				ps =| EBIT;
				r0 = u.u_error;
			}
			goto out;
		}
badsys:
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
	case 1:		/* kernal mode floating exception */
		psignal(u.u_procp, SIGFPT);
		return;

	case 0:		/* user mode floating exception */
		i = SIGFPT;
		break;

	/*
	 * If the user SP is below the stack segment,
	 * grow the stack automatically.
	 * This relies on the ability of the hardware
	 * to restart a half executed instruction.
	 * On the 11/40 this is not the case and
	 * the routine backup/l40.s may fail.
	 * The classic example is on the instruction
	 *	cmp	-(sp),-(sp)
	 */
	case 010:	/* segmentation violation */
		a = sp;
		if(backup(u.u_ar0) == 0)
		if(grow(a))
			goto out;
		i = SIGSEG;
		break;
#ifdef	MEM_PAR_INTR
	/*
	 *	memory parity errors
	 *	either hard/soft USER mode traps,
	 *	or soft KERNEL mode traps.
	 *	enter with parity traps disabled.
	 */
	case 012:
#ifdef	SYS_TIME
		printf("\n%s: ",systime(hhmm));
#endif	SYS_TIME
		printf(" memory parity (%o,%o), m-e-reg = %o\n",
				memhigha, memlowa, memereg);
		MEMCNTRL->integ =& 0177774;	/* enable parity traps */
		if( memereg & 0101000 ) {
			/* 'HARD' error */
			i = SIGMEMPAR;	/* clobber the user */
			break;
		}
		return;
#endif	MEM_PAR_INTR
	}
	psignal(u.u_procp, i);

out:
	if(issig())
		psig();
	curpri = setpri(u.u_procp);
}
#endif

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

#ifdef	SHARED_DATA
	u.u_flags =| UINTFLG;
#endif
#ifndef	SHARED_DATA
	u.u_intflg = 1;
#endif
	savu(u.u_qsav);
	(*f)();
#ifdef	SHARED_DATA
	u.u_flags =& ~UINTFLG;
#endif
#ifndef	SHARED_DATA
	u.u_intflg = 0;
#endif
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
