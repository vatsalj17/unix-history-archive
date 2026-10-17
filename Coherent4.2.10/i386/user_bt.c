/*
 * user_bt.c - console display of user eip stack backtrace.
 */

#include <common/_gregset.h>
#include <sys/types.h>
#include <sys/cmn_err.h>

#define _KERNEL		1

#include <kernel/reg.h>
#include <sys/seg.h>
#include <sys/proc.h>
#include <sys/mmu.h>

extern	unsigned char	do_user_bt;

/* User backtrace. */
void
user_bt (regsetp)
gregset_t	* regsetp;
{
	unsigned	bp, next_bp;
	unsigned	text_addr;
	register SEG *	segp;
	unsigned	splo;
	unsigned	bt_limit;

	/* Only do the user backtrace stuff if the feature is enabled. */

	if (do_user_bt == 0)
		return;

	/* Only bother with COFF's - no l.out's for now. */

	if (__xmode_286 (regsetp)) {
		return;
	}

	/* Get user stack limits, to make sure we stay inside them. */

	segp = SELF->p_segl [SISTACK].sr_segp;
	splo = (unsigned) (__xmode_286 (regsetp) ? ISP_286 : ISP_386);
	splo -= segp->s_size;

	/* Print user eip. */

	cmn_err (CE_CONT, "User backtrace (COFF): %x ", regsetp->_i386._eip);

	/*
	 * Loop as long as it looks as if we're tracking user stack frame.
	 *
	 * User stack frames look like this:
	 *	...
	 *	arguments for function foo
	 *	return address on return from foo
	 * ebp->previous stack frame
	 *	...
	 */

	bp = (unsigned) (regsetp->_i386._ebp);

	for (bt_limit = 0 ; bt_limit < 50 ; bt_limit ++) {
		/* Stop backtrace if frame pointer aims outside stack. */
		if (bp < splo || bp >= 0x80000000)
			break;

		/* Fetch and print a return address. */
		if (copyin((caddr_t)(bp + 4), & text_addr, 4))
			break;
		cmn_err (CE_CONT, " -> %x", text_addr);
		
		/* Stop backtrace if text pointer goes out of range. */
		/* Sorry about the magic constants - hal */
		if (text_addr < 0xA8 || text_addr >= 0x400000)
			break;

		/* Advance to the next older stack frame. */
		if (copyin((caddr_t) bp, & next_bp, 4))
			break;

		/* Stop backtrace if frame pointer goes down instead of up. */
		if (next_bp <= bp)
			break;

		bp = next_bp;
	}
	if (bt_limit == 50)
		cmn_err (CE_CONT, " ... (there's more)");

	cmn_err (CE_CONT, "\n");
}
