#define _KERNEL 1
#include <sys/types.h>
#include <common/_gregset.h>
#include <common/ccompat.h>

__EXTERN_C_BEGIN__

void __idle __PROTO(());
void __idle_end __PROTO(());	/* PURE ADDRESS!! DO **NOT** CALL THIS! */

__EXTERN_C_END__

/*
 * trap_level() returns non-zero if the thread described by regset
 * is a kernel context based on the stored code segment selector
 * and the stored program counter.  The exception to this rule is
 * the idle process since it counts as a user context in kernel
 * text and data.  3 is the magic number for user code selectors.
 */
#if __USE_PROTO__
int trap_level(gregset_t regset)
#else
int
trap_level(regset)
gregset_t regset;
#endif
{
  if ((regset._i386._cs.__selector & 3) != 3
      && (((unsigned long)regset._i386._eip < (unsigned long)__idle)
	  || ((unsigned long)regset._i386._eip
	      >= (unsigned long)(__idle_end))))
    return 1;
  else
    return 0;
}
