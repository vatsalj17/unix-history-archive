/*
 * Various interpreter data tables.
 */

#include "../h/rt.h"

/*
 * External declarations for blocks of built-in procedures.
 */
extern struct b_proc
#define FncDef(p) Cat(B,p),
#include "../h/fdef.h"
   Xnoproc; /* Hack to avoid ,; at end */
#undef FncDef
struct b_proc Xnoproc;

/*
 * Array of addresses of blocks for built-in procedures.  It is important
 *  that this table and the one in link/builtin.c agree; the linker
 *  supplies iconx with indices into this array.
 */
struct b_proc *functab[] = {
#define FncDef(p) Cat(&B,p),
#include "../h/fdef.h"
#undef FncDef
   0
   };
 
asgn(), bang(), bscan(), cat(), compl(), create(),
diff(), div(), eqv(), err(), escan(), field(), inter(), lconcat(),
lexeq(), lexge(), lexgt(), lexle(), lexlt(), lexne(),
minus(), mod(), mult(), neg(), neqv(), nonnull(), null(), number(),
numeq(), numge(), numgt(), numle(), numlt(), numne(), plus(), power(),
random(), rasgn(), refresh(), rswap(), sect(), size(),
subsc(), swap(), tabmat(), toby(), unions(), value();
/*
 * When an opcode n has a subroutine call associated with it, the
 *  nth word here is the routine to call.
 */
int (*optab[])() = {
	err,		asgn,		bang,		cat,
	compl,		diff,		div,		eqv,
	inter,		lconcat,	lexeq,		lexge,
	lexgt,		lexle,		lexlt,		lexne,
	minus,		mod,		mult,		neg,
	neqv,		nonnull,	null,		number,
	numeq,		numge,		numgt,		numle,
	numlt,		numne,		plus,		power,
	random,		rasgn,		refresh,	rswap,
	sect,		size,		subsc,		swap,
	tabmat,		toby,		unions,		value,
	bscan,		err,		err,		err,
	err,		err,		create,		err,
	err,		err,		err,		escan,
	err,		field,		err,		err
	};
