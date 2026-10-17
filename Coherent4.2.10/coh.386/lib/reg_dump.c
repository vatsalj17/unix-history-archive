/* $Header: /ker/coh.386/RCS/ker_data.c,v 2.4 93/08/19 03:26:31 nigel Exp Locker: nigel $ */
/*
 * This file contains definitions for the functions which support the Coherent
 * internal binary-compatibility scheme. We select _SYSV3 to get some old
 * definitions like makedev () visible.
 *
 * $Log:	ker_data.c,v $
 * Revision 2.4  93/08/19  03:26:31  nigel
 * Nigel's r83 (Stylistic cleanup)
 * 
 * Revision 2.2  93/07/26  14:55:28  nigel
 * Nigel's R80
 */

#define	_SYSV3		1

#include <common/ccompat.h>
#include <common/_gregset.h>
#include <sys/cmn_err.h>


/*
 * These two pull in all the old garbage.
 */

#define	_KERNEL		1

#include <sys/uproc.h>
#include <sys/proc.h>


/*
 * Perform a register dump from a gregset_t, plus any useful data from the
 * current process.
 */

#if	__USE_PROTO__
void curr_register_dump (gregset_t * regsetp, __sel_arith_t curr_priv)
#else
void
curr_register_dump (regsetp, curr_priv)
gregset_t     *	regsetp;
__sel_arith_t	curr_priv;
#endif
{
	cmn_err (CE_CONT, "\neax=%x  ebx=%x  ecx=%x  edx=%x",
		 regsetp->_i386._eax, regsetp->_i386._ebx,
		 regsetp->_i386._ecx, regsetp->_i386._edx);
	cmn_err (CE_CONT, "\nesi=%x  edi=%x  ebp=%x  esp=%x",
		 regsetp->_i386._esi, regsetp->_i386._edi,
		 regsetp->_i386._ebp, regsetp->_i386._esp);
	cmn_err (CE_CONT, "\ncs=%x  ds=%x  es=%x  fs=%x  gs=%x",
		 __SELECTOR_ARITH (regsetp->_i386._cs),
		 __SELECTOR_ARITH (regsetp->_i386._ds),
		 __SELECTOR_ARITH (regsetp->_i386._es),
		 __SELECTOR_ARITH (regsetp->_i386._fs),
		 __SELECTOR_ARITH (regsetp->_i386._gs));
	cmn_err (CE_CONT, "\nerr #%d eip=%x ", regsetp->_i386._err,
		 regsetp->_i386._eip);

	/*
	 * The very top part of the context is missing if it is from the same
	 * privelege level as us.
	 */

	if (__SELECTOR_PRIVILEGE (regsetp->_i386._cs) != curr_priv)
		cmn_err (CE_CONT, " ss=%x uesp=%x ",
			 __SELECTOR_ARITH (regsetp->_i386._ss),
			 regsetp->_i386._uesp);
	cmn_err (CE_CONT, "cmd=%s\nefl=%x  ", SELF->p_comm,
		 (__flag_arith_t) __FLAG_REG (regsetp));

	if (! __IS_USER_REGS (regsetp)) {
		/*
		 * This register set is indicating something that happened in
		 * the kernel, so do a call backtrace. To make this a little
		 * more robust, we only go upwards in the stack from the frame
		 * where we are, so that a smashed %ebp or frame causes no
		 * problems.
		 */

		__ulong_t     *	ebp = (__ulong_t *) regsetp->_i386._ebp;
		__ulong_t     *	prev = & regsetp->_i386._ebp;
		while (ebp > prev) {
			cmn_err (CE_CONT, " -> %x", * (ebp + 1));
			ebp = (__ulong_t *) * (prev = ebp);
		}
	}
	cmn_err (CE_CONT, "\n");
}


