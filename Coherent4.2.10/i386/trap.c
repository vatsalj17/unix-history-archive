/*
 * $Header: /v/src/rcskrnl/i386/RCS/trap.c,v 420.6 1993/12/02 18:05:11 srcadm Exp srcadm $
 */

/*********************************************************************
 *
 * Coherent, Mark Williams Company
 * RCS Header
 * This file contains proprietary information and is considered
 * a trade secret.  Unauthorized use is prohibited.
 *
 *
 * $Id: trap.c,v 420.6 1993/12/02 18:05:11 srcadm Exp srcadm $
 *
 * $Log: trap.c,v $
 * Revision 420.6  1993/12/02  18:05:11  srcadm
 * Attempt to fix bogus call number in lock imbalance printf.
 *
 * Revision 420.5  1993/12/02  18:02:57  srcadm
 * Initial RCS submission.
 *
 *
 */

#ifdef EMBEDDED_VERSION
/* Embedded Version Constant */
char *MWC_TRAP_C_VERSION = "MWC_TRAP_C_VERSION($Revision: 420.6 $)";
#endif

/*
 * Trap entry for miscellaneous i386 traps and faults.
 * Revision 2.5  93/10/29  00:57:27  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.4  93/09/02  18:12:26  nigel
 * Major edits for new flag system, cleanup of dead variables.
 * 
 * Revision 2.3  93/08/19  03:40:17  nigel
 * Nigel's R83
 */

#include <common/_gregset.h>
#include <sys/errno.h>
#include <signal.h>
#include <stddef.h>

#define	_KERNEL		1

#include <kernel/trace.h>
#include <kernel/systab.h>
#include <kernel/reg.h>
#include <sys/proc.h>
#include <sys/uproc.h>
#include <sys/seg.h>
#include <sys/mmu.h>


/*
 * Tunable parameter for controlling whether signal-generating traps cause
 * output on the system console.
 */

extern	unsigned char	CONSOLE_TRAP_DUMP;
extern	unsigned char	NMI_DUMP;


/* opcodes recognized, and partially emulated, in gp fault handler */
#define READ_CR0	1
#define WRITE_CR0	2
#define READ_CR2	3
#define READ_CR3	4
#define WRITE_CR3	5
#define HALT		6
#define IRET		7
#define READ_DR0	8
#define READ_DR1	9
#define READ_DR2	10
#define READ_DR3	11
#define READ_DR6	12
#define READ_DR7	13
#define WRITE_DR0	14
#define WRITE_DR1	15
#define WRITE_DR2	16
#define WRITE_DR3	17
#define WRITE_DR6	18
#define WRITE_DR7	19

#define ENTER_OP	0xC8	/* Opcode for 'enter' instruction.  */
#define IRET_RETRY_LIM	10

extern unsigned char selkcopy();
extern unsigned int DR0,DR1,DR2,DR3,DR7;
static int trap_op();


/*
 * Global symbols from kernel text.
 */

extern unsigned int	__xtrap_break__;
extern unsigned int	__xtrap_off__;
extern unsigned int	__xtrap_on__;
extern unsigned int	signal_386;
extern unsigned int	syscall_386;

/*
 * iret_flt is set when first bad iret is detected.
 */
static int iret_flt;

/*
 * Trap handler.
 * The arguments are the registers,
 * saved on the stack by machine code. This call
 * is different from most C calls in that the registers
 * get copied back; if you change a "trap" parameter then
 * the machine register will be altered when the trap is
 * dismissed.
 *
 * Argument "trapno" is the return eip for the code calling tsave().
 */

void
trap (regset)
gregset_t	regset;
{
	int		sigcode;

	/*
	 * Expect this to never happen!
	 */

	if (SELF->p_flags & PFKERN) {
		panic ("pid%d: kernel process trap: err=%x, ip=%x ax=%d",
			SELF->p_pid, regset._i386._err, __PC_REG (& regset),
			regset._i386._eax);
	}

	T_HAL (0x4000, printf ("T%d ", regset._i386._err));
	sigcode = 0;

	switch (regset._i386._err) {
	case SIOSYS:
		/*
		 * 286 System call.
		 */
		sigcode = oldsys (& regset);
		break;

	case SISYS: {
		struct systab *	stp;
		long		args [MSACOUNT];
		unsigned	callnum;
		struct __menv	sigenv;

		/*
		 * 386 System call.
		 */

		set_user_error (0);
		callnum = regset._i386._eax;

		if (callnum < NMICALL)
			stp = sysitab + callnum;
		else if (callnum == COHCALL)
			stp = & cohcall;
		else if ((callnum & 0xFF) == 0x28 &&
			 (callnum >> 8) <= H28CALL)
			stp = h28itab + (callnum >> 8) - 1;
		else {
			sigcode = SIGSYS;
			goto trapend;
		}

		T_ERRNO (2, cmn_err (CE_CONT, "{%s", stp->s_name));
		stp->s_stat ++;

		ukcopy (regset._i386._uesp + sizeof (long), args,
			stp->s_nargs * sizeof (long));

		if (get_user_error ()) {
			sigcode = SIGSYS;
			goto trapend;
		}

		if (envsave (u.u_sigenvp = & sigenv)) {
			set_user_error (EINTR);
		} else {
			regset._i386._eax = __DOSYSCALL (stp->s_nargs,
							 stp->s_func, args,
							 & regset);
			regset._i386._edx = u.u_rval2;
		}

		/*
		 * LOUIS:
		 * This is after the system call, so we can't be
		 * assured that _eax is the same even though the
		 * environment is recovered.  Well, it's not at
		 * any rate.  So, we just use callnum to place
		 * the system call since this is always good because
		 * it is saved in the stack frame.
		 */

		if (u.u_lock_cnt != 0) {
			cmn_err (CE_WARN,
				 "Lock count imbalance (%d) after syscall %d",
				 u.u_lock_cnt, callnum);
		}

		if (get_user_error ()) {
			regset._i386._eax = get_user_error ();

			__FLAG_REG (& regset) =
				__FLAG_SET_FLAG (__FLAG_REG (& regset),
						 __CARRY);
			T_ERRNO (2, cmn_err (CE_CONT, "-err"));
#ifdef FORJRD
		printf("Call: %d   Errno: %d\n", callnum, get_user_error());
#endif
		} else		/* clear carry flag in return efl */
			__FLAG_REG (& regset) =
				__FLAG_CLEAR_FLAG (__FLAG_REG (& regset),
						   __CARRY);
		LOCK_DISPATCH ();
		u.u_sigenvp = 0;

		T_ERRNO (2, cmn_err (CE_CONT, "=%x}", regset._i386._eax));
		break;
	    } /* end block */

		/*
		 * Trap.
		 */
	case SIDIV:
		if (! __IS_USER_REGS (& regset)) {

			curr_register_dump (& regset, _PRIVILEGE_RING_1);
			panic ("Integer divide by zero");
		}
#if	TRACER
		printf ("Integer divide by zero");
		curr_register_dump (& regset, _PRIVILEGE_RING_1);
#endif
		sigcode = SIGFPE;
		break;

	case SISST:
		sigcode = SIGTRAP;
		break;

	case SIBPT:
		sigcode = SIGTRAP;
		break;

	case SIOVF:
		sigcode = SIGEMT;
		break;

	case SIBND:
		/*
		 * Bound
		 */
		sigcode = SIGIOT;
		break;

	case SIOP: {
		int	      *	ip = (int *) __PC_REG (& regset);

		printf ("(eip)=%x %x %x  ", ip [0], ip [1], ip [2]);

		/*
		 * Invalid opcode
		 */

		if (! __IS_USER_REGS (& regset)) {

			curr_register_dump (& regset, _PRIVILEGE_RING_1);
			panic ("Invalid Opcode");
		}
		
		sigcode = SIGILL;
		break;
	}
#if 0
	case SIXNP:
		/*
		 * Processor extension not available
		 */
		if (int11() & 2)	/* NDP present */
			ndpNewOwner();
		else
			sigcode = SIGFPE;
		break;
#endif

	case SIDBL:
		/*
		 * Double exception
		 */
		panic ("double exception: cs=%x ip=%x", regset._i386._cs,
		       __PC_REG (& regset));
		sigcode = SIGSEGV;
		break;

	case SIXS:
		/*
		 * Processor extension segment overrun
		 */
		sigcode = SIGSEGV;
		break;

	case SITS:
		/*
		 * Invalid task state segment
		 */
		panic ("invalid tss: cs=%x ip=%x", regset._i386._cs,
		       __PC_REG (& regset));
		sigcode = SIGSEGV;
		break;

	case SINP:
		/*
		 * Segment not present
		 */
		sigcode = SIGSEGV;
		break;

	case SISS:
		/*
		 * Stack segment overrun/not present
		 */
		sigcode = SIGKILL;
		break;

	case SINMI:
#define	__SYSCON_B	0x61		/* System control port B */
#define	__PARITY_FAULT	0x80		/* Parity fault flag */
		if ((inb (__SYSCON_B) & __PARITY_FAULT) != 0) {
			printf ("Parity error\n");
			curr_register_dump (& regset, _PRIVILEGE_RING_1);
#if	0
			panic("...");
#endif
		} else if (NMI_DUMP)
			printf ("Unexpected NMI\n");
		return;

	default:
		curr_register_dump (& regset, _PRIVILEGE_RING_1);
		panic("Fatal Trap");
	}

trapend:
	/*
	 * Send user a signal.
	 * If not a breakpoint, do console register dump.
	 */
	if (sigcode) {
		if (sigcode != SIGTRAP && CONSOLE_TRAP_DUMP) {
			curr_register_dump (& regset, _PRIVILEGE_RING_1);
			cmn_err (CE_CONT, "sigcode=#%d  User Trap\n", sigcode);
			user_bt (& regset);
		}
		sendsig (sigcode, SELF);
	}
}


/*
 * trap_op()
 *
 * Look at the trapped instruction.
 * If it's one of a select few, recognize and return the type.
 * Otherwise, return 0.
 *
 * This could be fancier, but all we want to look for is:
 * (for CRx and DRx, second operand is limited to %eax).
 *	0F 20 C0	READ_CR0
 *	0F 22 C0	WRITE_CR0
 *	0F 20 D0	READ_CR2
 *	0F 20 D8	READ_CR3
 *	0F 22 D8	WRITE_CR3
 *	CF		IRET
 *	F4		HALT
 *	0F 23 C0	WRITE_DR0
 *	0F 23 C8	WRITE_DR1
 *	0F 23 D0	WRITE_DR2
 *	0F 23 D8	WRITE_DR3
 *	0F 23 F0	WRITE_DR6
 *	0F 23 F8	WRITE_DR7
 *	0F 21 C0	READ_DR0
 *	0F 21 C8	READ_DR1
 *	0F 21 D0	READ_DR2
 *	0F 21 D8	READ_DR3
 *	0F 21 F0	READ_DR6
 *	0F 21 F8	READ_DR7
 */

static int
trap_op(cs,eip)
unsigned int cs, eip;
{
	int		ret = 0;

	switch (ffbyte (eip, cs)) {

	case 0x0F:
		switch (ffbyte (eip + 1, cs)) {

		case 0x20:
			switch (ffbyte (eip + 2, cs)) {
			case 0xC0:
				ret = READ_CR0;
				break;

			case 0xD0:
				ret = READ_CR2;
				break;

			case 0xD8:
				ret = READ_CR3;
				break;
			}
			break;

		case 0x21:
			switch (ffbyte (eip + 2, cs)) {

			case 0xC0:
				ret = READ_DR0;
				break;

			case 0xC8:
				ret = READ_DR1;
				break;

			case 0xD0:
				ret = READ_DR2;
				break;

			case 0xD8:
				ret = READ_DR3;
				break;

			case 0xF0:
				ret = READ_DR6;
				break;

			case 0xF8:
				ret = READ_DR7;
				break;
			}
			break;

		case 0x22:
			switch (ffbyte (eip + 2, cs)) {

			case 0xC0:
				ret = WRITE_CR0;
				break;

			case 0xD8:
				ret = WRITE_CR3;
				break;
			}
			break;

		case 0x23:
			switch (ffbyte (eip + 2, cs)) {

			case 0xC0:
				ret = WRITE_DR0;
				break;

			case 0xC8:
				ret = WRITE_DR1;
				break;

			case 0xD0:
				ret = WRITE_DR2;
				break;

			case 0xD8:
				ret = WRITE_DR3;
				break;

			case 0xF0:
				ret = WRITE_DR6;
				break;

			case 0xF8:
				ret = WRITE_DR7;
				break;
			}
			break;
		}
		break;

	case 0xF4:
		ret = HALT;
		break;

	case 0xCF:
		ret = IRET;
		break;
	}
	return ret;
}


/*
 * General protection fault handler.
 * Entered via a ring 0 gate.
 */

void
gpfault (regset)
gregset_t	regset;
{
	/*
	 * Switch on CPL of code that trapped.
	 */

	switch (__SELECTOR_PRIVILEGE (regset._i386._cs)) {

	case _PRIVILEGE_RING_0:
		/*
		 * Ring 0 should not gp fault.
		 */
		curr_register_dump (& regset, _PRIVILEGE_RING_0);
		panic ("System GP Fault from Ring 0");
		break;

	case _PRIVILEGE_RING_1:
		/*
		 * If ring 1 faulted on a valid request, emulate the
		 * request while running in ring 0.
		 */
		switch (trap_op (regset._i386._cs, __PC_REG (& regset))) {
		case READ_CR0:
			regset._i386._eax = read_cr0 ();
			__PC_REG (& regset) += 3;
			break;

		case WRITE_CR0:
			write_cr0(regset._i386._eax);
			__PC_REG(& regset) += 3;
			break;

		case READ_CR2:
			regset._i386._eax = read_cr2 ();
			__PC_REG (& regset) += 3;
			break;

		case READ_CR3:
			regset._i386._eax = read_cr3 ();
			__PC_REG (& regset) += 3;
			break;
#if 0
		case WRITE_CR3:
			mmuupdnR0 ();
			__PC_REG (& regset) += 3;
			break;
#endif
		case IRET:
			/*
			 * Some CPU's wrongly generate GP faults on IRET
			 * from inner ring to ring 3.
			 * Fix is to retry the instruction a few times.
			 */
			if (! iret_flt) {
				iret_flt = 1;
				printf ("CPU Bug:  "
				  "Spurious GP Fault on Iret to Ring 3.\n");
			}
			break;

		case READ_DR0:
			regset._i386._eax = read_dr0 ();
			__PC_REG (& regset) += 3;
			break;

		case READ_DR1:
			regset._i386._eax = read_dr1 ();
			__PC_REG (& regset) += 3;
			break;

		case READ_DR2:
			regset._i386._eax = read_dr2 ();
			__PC_REG (& regset) += 3;
			break;

		case READ_DR3:
			regset._i386._eax = read_dr3 ();
			__PC_REG (& regset) += 3;
			break;

		case READ_DR6:
			regset._i386._eax = read_dr6 ();
			__PC_REG (& regset) += 3;
			break;

		case READ_DR7:
			regset._i386._eax = read_dr7 ();
			__PC_REG (& regset) += 3;
			break;

		case WRITE_DR0:
			write_dr0 (regset._i386._eax);
			__PC_REG (& regset) += 3;
			break;

		case WRITE_DR1:
			write_dr1 (regset._i386._eax);
			__PC_REG (& regset) += 3;
			break;

		case WRITE_DR2:
			write_dr2 (regset._i386._eax);
			__PC_REG (& regset) += 3;
			break;

		case WRITE_DR3:
			write_dr3 (regset._i386._eax);
			__PC_REG (& regset) += 3;
			break;

		case WRITE_DR6:
			write_dr6 (regset._i386._eax);
			__PC_REG (& regset) += 3;
			break;

		case WRITE_DR7:
			write_dr7 (regset._i386._eax);
			__PC_REG (& regset) += 3;
			break;

		case HALT:
			halt ();
			break;
#if 0
		case WRITE_DR7:
			/*
			 * Expect breakpoint info in globals DR0-3,DR7.
			 */
printf("Setting DR0=%x  DR1=%x  DR2=%x  DR3=%x  DR7=%x\n",
  DR0, DR1, DR2, DR3, DR7);
			write_dr0 (DR0);
			write_dr1 (DR1);
			write_dr2 (DR2);
			write_dr3 (DR3);
			write_dr7 (DR7);
			__PC_REG (& regset) += 3;
			break;
#endif
		default:
			if (__PC_REG (& regset) >= (__ulong_t) & __xtrap_on__ &&
			    __PC_REG (& regset) < (__ulong_t) & __xtrap_off__) {
#if	TRACER
				curr_register_dump (& regset,
						    _PRIVILEGE_RING_0);
				printf ("copy fault called from %x\n",
					* (int *) regset._i386._edx);
#endif
				SET_U_ERROR (EFAULT, "copy gp");
				__PC_REG (& regset) = (__ulong_t) & __xtrap_break__;
			} else {
				curr_register_dump (& regset,
						    _PRIVILEGE_RING_0);
				panic ("System GP Fault from Ring 1");
			}
		}
		break;

	case _PRIVILEGE_RING_2:
		/*
		 * Nothing should be running in Ring 2.
		 */

	case _PRIVILEGE_RING_3:
		/*
		 * Ring 3 gp fault means errant user process.
		 */
		curr_register_dump (& regset, _PRIVILEGE_RING_0);
		cmn_err (CE_CONT, "User GP Violation\n");
		user_bt (& regset);
		sendsig (SIGSEGV, SELF);
		break;
	}
}


/*
 * Kernel debugger.
 *
 * Runs in ring 0.
 */

void
__debug_ker__ (regset)
gregset_t	regset;
{
	unsigned int	dr6 = read_dr6 ();
	int		do_rdump = 1;

	if (dr6 & 0xf) {	/* report breakpoint exception(s) */
		printf("Breakpoint  ");
		if (dr6 & 1)
			printf("DR0=%x  ", DR0);
		if (dr6 & 2)
			printf("DR1=%x  ", DR1);
		if (dr6 & 4)
			printf("DR2=%x  ", DR2);
		if (dr6 & 8)
			printf("DR3=%x  ", DR3);
		printf("DR7=%x\n", DR7);
	}

	if (dr6 & 0xf000) {	/* report other debug exception(s) */
		if (dr6 & 0x8000)
			printf("Switch to debugged task\n");

		if (dr6 & 0x4000) {
			/* Single Step */
			switch (__SELECTOR_PRIVILEGE (regset._i386._cs)) {

			/*
			 * If user code trapped, send signal
			 * and suppress console register dump.
			 */

			case _PRIVILEGE_RING_1:
				/*
				 * Turn off single-stepping when entering
				 * Ring 1.
				 */
				if (__PC_REG (& regset) == (__ulong_t) & syscall_386 ||
				    __PC_REG (& regset) == (__ulong_t) & signal_386) {
					do_rdump = 0;
				} else {
printf("\nefl=%x  No single stepping the kernel.\n",
	(__flag_arith_t) __FLAG_REG (& regset));
				}
				__FLAG_REG (& regset) = 
					__FLAG_CLEAR_FLAG (__FLAG_REG (& regset),
							   __TRAP);
				break;

			case _USER_PRIVILEGE:
				do_rdump = 0;
T_HAL(0x20000, printf ("Kernel SSTEP eip=%x efl=%x  ", __PC_REG (& regset),
			(__flag_arith_t) __FLAG_REG (& regset)));
				sendsig (SIGTRAP, SELF);
				break;
			}
		}
		if (dr6 & 0x2000) {
			printf("ICE in use\n");
			__PC_REG (& regset) += 3;
		}
	}

	if (do_rdump)
		curr_register_dump (& regset, _PRIVILEGE_RING_0);

	write_dr6 (0);

	__FLAG_REG (& regset) =
		__FLAG_SET_FLAG (__FLAG_REG (& regset), __RESUME);
}


/*
 * User debugger.
 *
 * Runs in ring 1.
 */

void
__debug_usr__ (regset)
gregset_t	regset;
{
	unsigned int	dr6 = read_dr6();
	int		do_rdump = 1;

	if (dr6 & 0xf) {	/* report breakpoint exception(s) */
		printf("Breakpoint  ");
		if (dr6 & 1)
			printf("DR0=%x  ", DR0);
		if (dr6 & 2)
			printf("DR1=%x  ", DR1);
		if (dr6 & 4)
			printf("DR2=%x  ", DR2);
		if (dr6 & 8)
			printf("DR3=%x  ", DR3);
		printf("DR7=%x\n", DR7);
	}

	if (dr6 &  0xf000) {	/* report other debug exception(s) */
		if (dr6 & 0x8000)
			printf("Switch to debugged task\n");

		if (dr6 & 0x4000) {
			/* Single Step */

			switch (__SELECTOR_PRIVILEGE (regset._i386._cs)) {
			/*
			 * If user code trapped, send signal
			 * and suppress console register dump.
			 */

			case _PRIVILEGE_RING_1:
				/*
				 * Turn off single-stepping when entering
				 * Ring 1.
				 */
				if (__PC_REG (& regset) == (__ulong_t) & syscall_386 ||
				    __PC_REG (& regset) == (__ulong_t) & signal_386) {
					do_rdump = 0;
				} else {
printf("/nefl=%x  No single stepping the kernel.\n",
	(__flag_arith_t) __FLAG_REG (& regset));
				}
				__FLAG_REG (& regset) =
					__FLAG_CLEAR_FLAG (__FLAG_REG (& regset),
							   __TRAP);
				break;

			case _USER_PRIVILEGE:
				do_rdump = 0;
T_HAL(0x20000, printf ("User SSTEP eip=%x efl=%x  ", __PC_REG (& regset),
			__FLAG_REG (& regset)));
				sendsig (SIGTRAP, SELF);
				break;
			}
		}
		if (dr6 & 0x2000) {
			printf("ICE in use\n");
			__PC_REG (& regset) += 3;
		}
	}

	if (do_rdump)
		curr_register_dump (& regset, _PRIVILEGE_RING_1);

	write_dr6 (0);


	__FLAG_REG (& regset) =
		__FLAG_SET_FLAG (__FLAG_REG (& regset), __RESUME);
}


/*
 * Ring 0 stack fault handler; this runs at ring 0 so that we can catch kernel
 * stack overflows (or underruns, whatever you want to call them). Note that
 * user code can also cause these.
 */

void
stackfault (regset)
gregset_t	regset;
{
	printf ("Stack fault");
	curr_register_dump (& regset, _PRIVILEGE_RING_0);

	switch (__SELECTOR_PRIVILEGE (regset._i386._cs)) {

	case _USER_PRIVILEGE:
		if (CONSOLE_TRAP_DUMP) {
			curr_register_dump (& regset, _PRIVILEGE_RING_1);
			cmn_err (CE_CONT, "sigcode=#%d  User Stack Fault\n",
				 SIGTRAP);
			user_bt (& regset);
		}
		sendsig (SIGTRAP, SELF);
		break;

	default:
		panic ("The kernel stack has overflowed: we cannot proceed\n");
		break;
	}
}


void
pagefault (regset)
gregset_t	regset;
{
	register int	sigcode;
	register SEG *segp;
	int	splo, datahi;
	unsigned int	txtlo, txthi;
	unsigned long newsp;	/* Anticipated value for stack pointer.  */
	unsigned int cr2 = 0;

	/*
	 * Expect this to never happen!
	 */

	if (SELF->p_flags & PFKERN) {
		panic ("pid%d: kernel process trap: err=%x, ip=%x ax=%d",
			SELF->p_pid, regset._i386._err, __PC_REG (& regset),
			regset._i386._eax);
	}

	T_HAL (0x4000, printf("T%d ", regset._i386._err));
	sigcode = 0;


	/*
	 * Page fault
	 */

	cr2 = read_cr2 ();

	if (! __IS_USER_REGS (& regset)) {
		/*
		 * If page fault during Ring 1 copy service routine,
		 * such as kucopy or ukcopy, set u_error and abort
		 * the copy, but don't send signal to the user.
		 */

		if (__PC_REG (& regset) >= (__ulong_t) & __xtrap_on__ &&
		    __PC_REG (& regset) < (__ulong_t) & __xtrap_off__) {

			T_HAL (0x1000, printf ("copy trapped "));
#if	TRACER
			curr_register_dump (& regset, _PRIVILEGE_RING_1);
			printf ("copy fault called from %x\n",
				* (int *) regset._i386._edx);
#endif
			SET_U_ERROR (EFAULT, "copy service");
			__PC_REG (& regset) = (__ulong_t) & __xtrap_break__;
			goto pf_end;
		} else {
			printf ("cr2=%x", cr2);
			curr_register_dump (& regset, _PRIVILEGE_RING_1);
			panic ("Kernel Page Fault");
		}
	}

	/* Check for stack underflow. */

	/*
	 * I think 'splo' is being calculated in a bass-ackwards way,
	 * and that 'datahi' is just wrong, but I'm not certain,
	 * so the fixes are #if 0'd out. -piggy
	 *
	 * I'll take out the 0 some day and test these changes.
	 */
	segp = SELF->p_segl [SISTACK].sr_segp;
#if 0
	splo = SELF->p_segl [SISTACK].sr_base - segp->s_size;
	datahi = SELF->p_segl [SIPDATA].sr_base + SELF->p_segl [SIPDATA].sr_size;
#else
	splo = __xmode_286 (& regset) ? ISP_286 : ISP_386;
	splo -= segp->s_size;
	datahi = SELF->p_segl [SIPDATA].sr_size;
#endif /* 0 */

	/*
	 * Catch bad function pointer here - don't want to restart
	 * the user instruction and get runaway segv's.
	 *
	 * For 286 executables, eip starts at 0, but cs points to
	 * descriptor SEL_286_UII which adds 0x400000 (UII_BASE).
	 */

	txtlo = (unsigned long) SELF->p_segl [SISTEXT].sr_base;
	if (__xmode_286 (& regset))
		txtlo -= UII_BASE;

	txthi = txtlo + SELF->p_segl [SISTEXT].sr_size;
	if (__PC_REG (& regset) < txtlo ||
	    __PC_REG (& regset) > txthi) {
		T_HAL (0x1000, printf ("Bad eip, txtlo=%x txthi=%x\n",
				       txtlo, txthi));
		goto bad_pf;
	}

	/*
	 * If we trapped on an 'enter' instruction, the stack
	 * pointer (uesp) has not yet been decremented.  In
	 * order to correctly process such a stack overflow,
	 * we must look at the _expected_ value for uesp.
	 * NB: We COPY uesp, because that arg gets loaded back
	 * into the real esp--when we return from the trap the
	 * enter instruction will decrement the esp.
	 */

	newsp = __xmode_286 (& regset) ? regset._i286._usp :
					 regset._i386._uesp;

	if (ffbyte (__PC_REG (& regset), regset._i386._cs) == ENTER_OP) {
		/*
		 * Adjust the sp by the argument of the ENTER
		 * instruction.
		 */

		newsp -= ffword (__PC_REG (& regset) + 1, regset._i386._cs);
	}

	if (cr2 <= splo && newsp <= splo && newsp > datahi &&
	    btocru (datahi) < btocrd (splo)) {
		cseg_t	     *	pp;

		if ((pp = c_extend (segp->s_vmem,
				    btocru (segp->s_size))) == NULL) {
			T_HAL (0x1000, printf ("c_extend(%x,%x)=0 ",
					       segp->s_vmem,
					       btocru (segp->s_size)));
			goto bad_pf;
		}

		segp->s_vmem = pp;
		segp->s_size += NBPC;

		if (sproto (0) == 0) {
			T_HAL(0x1000, printf("sproto(0)=0 "));
			goto bad_pf;
		}

		segload ();
		goto pf_end;
	}

	/*
	 * Catch bad data pointer here - don't want to restart
	 * the user instruction and get runaway segv's.
	 */

	{
		T_HAL(0x1000, printf ("Bad data, splo=%x datahi=%x\n",
				      splo, datahi));
	}

bad_pf:
	/*
	 * User generated unacceptable page fault.
	 */

	sigcode = SIGSEGV;
	printf("\ncr2=%x  ", cr2);

pf_end:
	/*
	 * Send user a signal.
	 * If not segload ();
	 * If not a breakpoint, do console register dump.
	 */

	if (sigcode) {
		if (CONSOLE_TRAP_DUMP) {
			curr_register_dump (& regset, _PRIVILEGE_RING_1);
			cmn_err (CE_CONT, "sigcode=#%d  User Page Fault\n",
				 sigcode);
			user_bt (& regset);
		}
		sendsig (sigcode, SELF);
	}
}
