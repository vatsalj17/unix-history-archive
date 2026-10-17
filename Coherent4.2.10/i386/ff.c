/* $Header: /ker/i386/RCS/ff.c,v 2.4 93/10/29 00:56:45 nigel Exp Locker: nigel $ */
/*
 * Simulate all the calls for far memory access from COH 286.
 * Far pointers are simulated with virtual addresses.
 *
 * Add __ptov() for debugging.  94/02/19.  hal
 *
 * $Log:	ff.c,v $
 * Revision 2.4  93/10/29  00:56:45  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.3  93/08/19  03:40:03  nigel
 * Nigel's R83
 * 
 */

#include <sys/types.h>

#define	_KERNEL		1

#include <kernel/fakeff.h>
#include <kernel/reg.h>
#include <sys/mmu.h>
#include <sys/seg.h>
#include <sys/cmn_err.h>

MAKESR(allocp, _allocp);

/*
 * P2P used to do the following as a macro but it is feeding a bad
 * physical address to __PTOV() so here is a diagnostic version.
 * Convert system global address to physical address.
 */
#if	__USE_PROTO__
__paddr_t __sg_to_p(__sg_addr_t sys_gl)
#else
__paddr_t
__sg_to_p(sys_gl)
__sg_addr_t	sys_gl;
#endif
{
	__paddr_t	retval;

	retval = ((sysmem.u.pbase[btocrd(sys_gl)]&~(NBPC-1))
	  |(sys_gl&(NBPC-1)));

	/* Range check the result. */
	if (retval >= (__paddr_t) __PHYSICAL_MAP_LEN) {
		cmn_err(CE_WARN, "P2P(%x) ", sys_gl);
		backtrace();
	}

	return retval;
}

/*
 * __PTOV used to do the following as a macro but the -1 return is just
 * a time bomb for the kernel, so the following is a first step at tracking
 * down the problem.
 */
#if	__USE_PROTO__
__caddr_t __ptov(__paddr_t phys)
#else
__caddr_t
__ptov(phys)
__paddr_t	phys;
#endif
{
	if (phys < (__paddr_t) __PHYSICAL_MAP_LEN)
		return (__caddr_t) (phys) + __PHYSICAL_MAP_BASE;
	else {
		cmn_err(CE_WARN, "__PTOV(%x) ", phys);
		backtrace();
		return (__caddr_t) -1;
	}
}

/*
 * Initialize a virtual address to access physical memory at location
 * 'paddr', of size 'len' bytes.  It provides read and write (but not
 * execute) access.  When no longer required, a virtual address should be
 * released by vrelse.
 *
 * 486 CPU's need PCD bit set to 1 since this routine will typically
 * be used by a driver to reserve virtual space for memory mapped i/o.
 */

faddr_t
map_pv(paddr, len)
__paddr_t paddr;
fsize_t len;
{
	int s;			/* Return value of sphi().  */
	int npage;		/* Number of pages we must allocate.  */
	faddr_t	chunk_start;	/* Start of allocated segment in vmem.  */
	faddr_t retval;		/* Address of desired physical memory in vmem.  */
	int base1;		/* Offset into ptable1_v[].  */
	cseg_t	pte;		/* Build page table entries here.  */

	/* Figure out how many clicks we need to map.
	 *	   [		    ]		What we want.
	 *	[	|	|	]	What we get.
	 * Total number of clicks is:
	 *	(click up from (paddr+len)) - (click down from paddr)
	 */
		
	npage = btocru (paddr + len) - btocrd (paddr);

	/* Note that sysmem.vaddre is ALWAYS click aligned.  */
	
	/*
	 * Allocate the required chunk of virtual memory space.
	 * This could be a lot more sophisticated.  For expedience,
	 * there is no way to free this after it has been allocated,
	 * and there are no checks to see if we ran out of virtual space.
	 */

	s = sphi ();
	chunk_start = (faddr_t) sysmem.vaddre;
	sysmem.vaddre += ctob (npage);
	spl (s);

	/*
	 * Figure out where the desired physical address ends up in vmem.
	 */

	retval = chunk_start + (paddr - ctob (btocru (paddr)));
	
	/*
	 * Load the page table.
	 */

	base1 = btocrd (chunk_start);
	pte = ctob (btocrd (paddr));
	do {
		ptable1_v [base1] = pte | SEG_SRW | SEG_PCD;
		base1 ++;
		pte += ctob (1);	/* Bump up to next physical click.  */
	} while (-- npage > 0);
	mmuupd ();	/* Tell the mmu about the new map.  */

	return retval;
}


/*
 * Release a virtual address that we previously obtained with function
 * map_pv().
 */
void
unmap_pv(faddr)
faddr_t faddr;
{
	/* For the moment, this function does nothing.  */
}		


/*
 * Translate virtual address to physical address.
 * Returns the current physical address associated with virtual address 'vaddr'.
 * Returns 0 if that portion of virtual address space is not associated with
 * any physical memory.
 */

__paddr_t
__coh_vtop (vaddr)
__caddr_t vaddr;
{
	paddr_t	retval;
	unsigned int ptable_idx;	/* Index into ptable1_v[].  */

	ptable_idx = btocrd (vaddr);

	/*
	 * There is a 4Mbyte virtual page table ptable1_v[] which is
	 * all the bottom level page tables appended into a big array.
	 * Note that there are huge holes in this data structure, for
	 * unmapped virtual address space.
	 *
	 * We are going to look up 'vaddr' in the virtual page table
	 * ptable1_v[].
	 *
	 * But first, we have to see if the portion of page table we are
	 * going to look at exists.  We do this by looking at the one click
	 * long page table that maps the virtual page table, PPTABLE1_V[].
	 */

	retval = 0;	/* Assume entry not found.  */

	if (ptable0_v [btosrd (vaddr)] & SEG_SRO) {
		/*
		 * ASSERTION:  The portion of ptable1_v[] we want is valid.
		 */
		if (ptable1_v [ptable_idx] & SEG_SRO) {

			/*
			 * ASSERTION:  'vaddr' corresponds to some
			 * physical memory.
			 *
			 * Note that the address of a physical click is
			 * all above bit 11 in the PTE.
			 */
			retval = (ptable1_v[ptable_idx] & ~(NBPC - 1));
			retval += ((long) vaddr & (NBPC - 1));
		}
	}

	return retval;
}


/*
 * Translate system global address 'vpaddr' to physical address.
 *
 * May cause a panic if 'vpaddr' does not correspond to a real physical
 * address.
 */

paddr_t
vptop(vpaddr)
paddr_t vpaddr;
{
	paddr_t	retval;
	cseg_t pte;	/* Page table entry from sysmem.u.pbase[].  */

	pte = sysmem.u.pbase [btocrd (vpaddr)];
	pte &= ~ (NBPC - 1);	/* Strip off the non-address information.  */

	retval = pte | (vpaddr & (NBPC - 1));

	return retval;
}


/*
 * Convert from virtual address to system global address.  Similar to MAPIO(),
 * but does not require separate segment and offset.
 *
 * Only works for Kernel Space virtual addresses.
 */

paddr_t
vtovp(vaddr)
__caddr_t vaddr;
{
	return MAPIO (allocp.sr_segp->s_vmem, vaddr - allocp.sr_base);
}
