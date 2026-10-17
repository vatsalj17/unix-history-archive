/* $Header: /ker/i386/RCS/fakedma.c,v 2.5 93/10/29 00:56:44 nigel Exp Locker: nigel $ */
/*
 * these routines are written in C until the 386 compiler/assembler
 * are available
 *
 * Copyright (c) Ciaran O'Donnell, Bievres (FRANCE), 1991
 *
 * $Log:	fakedma.c,v $
 * Revision 2.5  93/10/29  00:56:44  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.4  93/09/02  18:11:13  nigel
 * Remove unreferenced globals
 * 
 * Revision 2.3  93/08/19  03:40:02  nigel
 * Nigel's R83
 */

#include <sys/debug.h>
#include <sys/inline.h>
#include <sys/errno.h>
#include <sys/cmn_err.h>
#include <signal.h>

#define	_KERNEL		1

#include <kernel/reg.h>
#include <sys/mmu.h>
#include <sys/inode.h>
#include <sys/proc.h>
#include <sys/seg.h>

#include <coh/fakedma.h>

#define	min(a, b)	((a) < (b) ? (a) : (b))

#define	OLD_DMA		0


/*
 * dmacopy()
 *
 * Copy "npage" 4 kbyte pages from phys addr "from" to phys addr "to".
 */

void
dmacopy(npage, from, to) 
long	npage;
cseg_t	*from, *to;
{
	ASSERT (npage > 0);

	for (;;) {
		cseg_t		src = * from ++ & ~ (NBPC - 1);
		cseg_t		dest = * to ++ & ~ (NBPC - 1);

		memcpy (__PTOV (dest), __PTOV (src), NBPC);

		if (-- npage == 0)
			break;
	}
}


/*
 * dmaclear()
 *
 * Given a byte count, and a system global address, zero-fill the memory.
 */

void
dmaclear (nbytes, to)
int	nbytes;
paddr_t	to;
{
	while (nbytes > 0) {

		unsigned int start_offset, x;

		/* Compute offset within page of starting s.g. address. */
		start_offset = to & (NBPC - 1);

		/* Compute number of bytes from "to" through end of page. */
		x = NBPC - start_offset;

		/* Bytes to clear is minimum of "nbytes" and "x". */
		if (x > nbytes)
			x = nbytes;

		/* Clear bytes.  Stay within a single system global page. */
		memset (__PTOV (P2P (to)), 0, x);

		/* Update start address and byte count. */
		to += x;
		nbytes -= x;
	}
}


/*
 * dmain()
 * 
 * Copy in "nbytes" from system global address "to" to kernel address
 * "vaddr".
 */

void
dmain(nbytes, to, vaddr)
long	nbytes;
paddr_t	to;
caddr_t	vaddr;
{
	while (nbytes > 0) {

		unsigned int start_offset, x;

		/* Compute offset within page of starting s.g. address. */
		start_offset = to & (NBPC - 1);

		/* Compute number of bytes from "to" through end of page. */
		x = NBPC - start_offset;

		/* Bytes to copy is minimum of "nbytes" and "x". */
		if (x > nbytes)
			x = nbytes;

		/* Copy bytes.  Stay within a single system global page. */
		memcpy (vaddr, __PTOV (P2P (to)), x);

		/* Update start addresses and byte count. */
		to += x;
		vaddr += x;
		nbytes -= x;
	}
}


/*
 * dmaout()
 * 
 * Copy out "nbytes" from kernel address "vaddr" to system global address
 * "to".
 */

void
dmaout(nbytes, to, vaddr)
long	nbytes;
paddr_t	to;
caddr_t	vaddr;
{
	while (nbytes > 0) {

		unsigned int start_offset, x;

		/* Compute offset within page of starting s.g. address. */
		start_offset = to & (NBPC - 1);

		/* Compute number of bytes from "to" through end of page. */
		x = NBPC - start_offset;

		/* Bytes to copy is minimum of "nbytes" and "x". */
		if (x > nbytes)
			x = nbytes;

		/* Copy bytes.  Stay within a single system global page. */
		memcpy (__PTOV (P2P (to)), vaddr, x);

		/* Update start addresses and byte count. */
		to += x;
		vaddr += x;
		nbytes -= x;
	}
}


/*
 * pxcopy()
 *
 * copy "n" bytes of data at kernel address "v" from address "uo" in:
 * 	system global address space		(space & SEG_VIRT)
 * 	physical memory				! (space & SEG_VIRT)
 * Rights are determined by (space & ~ SEG_VIRT):
 * 	"v" can be anywhere in kernel address space	SEL_386_KD
 * Up to one click of data can be copied. No alignment restrictions
 * on "uo" apply.
 */

#if __USE_PROTO__
int pxcopy(__paddr_t uo, __caddr_t v, int n, int space)
#else
int
pxcopy(uo, v, n, space)
__paddr_t uo;
char	*v;
int n;
int space;
#endif
{
	cseg_t	      *	base;
	int		save;
	int		err;
	int		work;

	if (n > NBPC || n == 0)
		return 0;

	work = workAlloc ();

	if (space & SEG_VIRT) {
		space &= ~ SEG_VIRT;
		base = & sysmem.u.pbase [btocrd (uo)];
		ptable1_v [work] = * base ++ | SEG_SRW;
		ptable1_v [work + 1] = * base ++ | SEG_SRW;
	} else {
		ptable1_v [work] = (uo & ~ (NBPC - 1)) + SEG_SRW;
		ptable1_v [work + 1] = (uo & ~ (NBPC - 1)) + NBPC + SEG_SRW;
	}

	save = setspace (space);
	err = kucopy (ctob (work) + (uo & (NBPC - 1)), v, n);
	setspace (save);

	workFree (work);
	return err;
}


/*
 * xpcopy()
 * 
 * copy "n" bytes of data from kernel address "v" to address "uo" in:
 *      system global address space                      (space&SEG_VIRT)
 *      physical memory                                 !(space&SEG_VIRT)
 * Rights are determined by (space&~SEG_VIRT):
 *      "v" can be anywhere in kernel address space      SEL_386_KD
 *      "v" must be an address accessible to the user    SEL_386_UD
 * Up to one click of data can be copied. No alignment restrictions on "uo"
 * apply.
 */

#if __USE_PROTO__
int xpcopy(__caddr_t v, __paddr_t uo, int n, int space)
#else
int
xpcopy(v, uo, n, space)
__caddr_t *v;
__paddr_t uo;
int n;
int space;
#endif
{
	cseg_t	      *	base;
	int		save;
	int		err;
	int		work;

	if (n > NBPC || n == 0)
		return 0;

	work = workAlloc ();

	if (space & SEG_VIRT) {
		space &= ~ SEG_VIRT;
		base = & sysmem.u.pbase [btocrd (uo)];
		ptable1_v [work] = * base ++ | SEG_SRW;
		ptable1_v [work + 1] = * base ++ | SEG_SRW;
	} else {
		ptable1_v [work] = (uo & ~ (NBPC - 1)) + SEG_SRW;
		ptable1_v [work + 1] = (uo & ~ (NBPC - 1)) + NBPC + SEG_SRW;
	}

	save = setspace (space);
	err = ukcopy (v, ctob (work) + (uo & (NBPC - 1)), n);
	setspace (save);

	workFree (work);
	return err;
}
