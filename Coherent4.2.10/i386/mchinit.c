/*
 * mchinit.c - protected, nonpaged startup for COHERENT.
 *
 */

#define	_KERNEL		1

#include <common/ccompat.h>
#include <kernel/reg.h>
#include <kernel/trace.h>
#include <sys/mmu.h>
#include <sys/proc.h>
#include <limits.h>


#define	PTABLE0_P	0x00001	/* Page directory physical address.	*/


unsigned total_mem;		/* Total physical memory in bytes.  */

#define	SPLASH	3
#define	NDATA	4	/* process data segments			*/
#define	BLKSZ	2	/* log2 sizeof(BLOCKLIST)/sizeof(cseg_t)	*/

/* These defines belong somewhere else:  */
#define LOMEM	0x15	/* CMOS address of size in K of memory below 1MB.  */
#define EXTMEM	0x17	/* CMOS address of size in K of memory above 1MB.  */
#define ONE_K	1024
#define ONE_MEG	1048576
#define USE_NDATA	1

typedef struct {
	unsigned short	off_lo;
	unsigned short	seg;
	unsigned short	flags;
	unsigned short	off_hi;
} IDT;

/*
 * Used to print hex numbers, and to verify data relocation.
 */

extern char digtab [];

/*
 * "__end" is end of kernel (i.e., end of BSS).
 */

extern char __end [];

/*
 * stext
 *
 *   When mchinit runs, bootstrap code assures us that stext corresponds
 *   to physical address 0x2000.
 *
 *   The kernel is generated so that symbol stext has value 0xFFC0_0000.
 *   When mchinit runs, the data segment selector (SEG_386_ID) has an
 *   offset of 0x0040_2000.  So, reference to stext in mchinit () code
 *   also refers to physical address 0x2000.
 */

extern char stext [];

extern char __end_data [], __end_text [], sdata [];
extern int RAM0, RAMSIZE;


/*
 * DMA will not work to memory above 16M, so limit the amount of memory
 * above 1M to 15M.  A much cleverer scheme should be implemented.
 */

int HACK_LIMIT = 15 * ONE_MEG;

/*
 * This is a bitmap of 4mb regions for which we should create 1-1 logical-
 * physical mappings in the page tables.
 */

extern	unsigned long	physical_mapping_map;

/*
 * pageDir
 *
 *  pageDir is the virtual address where the page directory is kept,
 *  in virtual space immediately below kernel text.
 *  As of 94/06/15, pagedir is 0xFFBF_F000.
 * 
 *  Assembly-language startup points CR3 (page directory register)
 *  at *physical* address 0x1000, the second page from the start of
 *  low RAM.  (The lowest page of physical RAM is left unchanged from
 *  boot-up, and contains the BIOS RAM area.)
 */

#define	pageDir		((long *) (stext - NBPC))

int total_pages;	/* How many pages did we start with?  */


/****************************************************************
 * init_phy_seg ()
 *
 ***************************************************************/

#if __USE_PROTO__
void init_phy_seg (long * ptab1_v, int addr, int base)
#else
void
init_phy_seg (ptab1_v, addr, base)
long	* ptab1_v;
#endif
{
	register int i;

	for (i = 0; i < btocru (0x10000); i ++) {
		ptab1_v [addr + i] = base | SEG_SRW; 
		base += NBPC;
	}
}

/****************************************************************
 * idtinit ()
 *
 * Fix up descriptors which are hard to create properly at compile / link time.
 * Apply to idt and ldt.
 *
 * Swap 16-bit words at descriptor + 2, descriptor + 6.
 ***************************************************************/

#if __USE_PROTO__
void idtinit (void)
#else
void
idtinit ()
#endif
{
	extern IDT	idt [], idtend [];
	extern IDT	ldt [], ldtend [];
	extern IDT	gdtFixBegin [], gdtFixEnd [];

	register IDT * ip;
	register unsigned short tmp;

	for (ip = idt; ip < idtend; ip ++) {
		tmp = ip->off_hi;
		ip->off_hi = ip->seg;
		ip->seg = tmp;
	}

	for (ip = ldt; ip < ldtend; ip ++) {
		tmp = ip->off_hi;
		ip->off_hi = ip->seg;
		ip->seg = tmp;
	}

	for (ip = gdtFixBegin; ip < gdtFixEnd; ip ++) {
		tmp = ip->off_hi;
		ip->off_hi = ip->seg;
		ip->seg = tmp;
	}
}

/****************************************************************
 * mchinit ()
 *
 * Set up page tables just before entering paged mode.
 *
 * mchinit() is the last thing to be done before paging is turned on.
 * It executes in protected mode, and has a valid data segment.
 *
 * At compile-time, data segment symbols are based at 0xFFC0_0000.
 *
 * At run time for mchinit (), the data segment has an offset of
 * 0x0040_2000 in the CPU descriptor tables - selector SEG_386_ID.
 ***************************************************************/

#if __USE_PROTO__
void mchinit (void)
#else
void
mchinit ()
#endif
{
	int lo;		/* Number of bytes of physical memory below 640K.  */
	int hi;		/* Number of bytes of physical memory above 1M.  */
	char * pe; 
	int	i;
	long * ptab1_v;
	unsigned short	base;
	int	codeseg, sysseg, ptable1;
	int	ptoff;	/* An offset into pageDir []  */
	int	nalloc;
	static	SEG	uinit;
	int	budArenaBytes;	/* number of bytes in buddy pool */
	int	kerBytes;	/* number of bytes in kernel text and data */

	__paddr_t	ua_paddr;

	/*
	 * 1.
	 *   a. Relocate the data on a page boundary (4K bytes) the
	 *      bootstrap relocates it on a paragraph boundary (16 bytes)
	 *
	 *   b. Verify that the data has been relocated correctly 
	 */

	pe = __end_data;					/* 1.a */
	i = (((unsigned) __end_text + 15) & ~ 15) - (unsigned) sdata;
	do {
		pe --;
		pe [0] = pe [i];
	} while (pe != sdata);					/* 1.b */


	/*
	 * Can now access the .data segment from C.
	 * If not, next loop will hang the kernel.
	 */

	CHIRP ('A');
	while (digtab [0] != '0')
		/* DO NOTHING */ ;
	CHIRP ('*');

	/* Zero the bss. */
	memset (__end_data, 0, __end - __end_data);

	/*
	 * Zero the page directory, which occupies the page
	 * of virtual space immediately below kernel text.
	 */

	memset (pageDir, 0, NBPC);


	CHIRP ('2');

	/*
	 * 3. Calculate total system memory.
	 *    Count the space used by the system and the page
	 *    descriptors, the interrupt stack, and the refresh work area
	 *
	 * a. initialize allocation area and adjust system size
	 *    to take allocation area and free page area into account
	 */

	/*
	 * btocru (__end) - SBASE is the number of pages in kernel text
	 * plus data, rounded up.
	 * PBASE is the starting physical page number of the kernel.
	 *
	 * Set sysmem.lo to the physical page address just past the kernel.
	 */

	kerBytes = (int)(__end - ((SBASE - PBASE) << BPCSHIFT));
	sysmem.lo = btocru (kerBytes);


	/*
	 * lo is the size in bytes of memory between the end of the kernel
	 *	and the end of memory below 640K.
	 * hi is the size in bytes of memory over 1 Megabyte (Extended memory).
	 *
	 * Round the sizes from the CMOS down to the next page.  This
	 * compensates for systems where the CMOS reports sizes that are
	 * not multiples of 4K.
	 */

	lo = ctob (read16_cmos (LOMEM) >> 2) - ctob (sysmem.lo);
	hi = ctob (read16_cmos (EXTMEM) >> 2);


	/*
	 * Sometimes, we die horribly if there is too much memory.
	 * Artificially limit hi to HACK_LIMIT.
	 */

	if (hi > HACK_LIMIT)
		hi = HACK_LIMIT;
	
	/* Record total memory for later use.  */
	total_mem = ctob (sysmem.lo) + lo + hi;

#if 0
	/*
	 * Clear base memory above the kernel.  93/12/09 - hal
	 *
	 * This is in hopes of eradicating some compatibility issues
	 * that arose at startup with 4.2.05, which did not clear memory
	 * above the kernel.
	 * For example, is the U area, including u.u_rdir, initially NULL
	 * as pfork() expects it to be?
	 */
	CHIRP('z');
	memset (ctob (sysmem.lo + SBASE - PBASE), 0, lo);
	CHIRP('Z');

	/* clear extended memory */
	memset (ONE_MEG + ctob (SBASE - PBASE), 0, hi);
	CHIRP('Y');
#endif

	/*
	 * sysmem.pfree and relatives will keep track of a pool of 4k pages
	 * assigned to processes, hereinafter known as the sysmem pool.
	 * How many pages can go into this pool?  nalloc.
	 * Allow NBPC for the page itself, a short for the sysmem pointer,
	 * and SPLASH * sizeof (long) for buddy system overhead.
	 */

	nalloc = (lo + hi) / (sizeof (short) + SPLASH * sizeof (long) + NBPC);


	/*
	 * ASSERT:
	 * For the moment we want only to assure that the
	 * BUDDY arena and the stack of free pages will fit below
	 * 640K.
	 */

	budArenaBytes = SPLASH * nalloc * sizeof (long);

	/*
	 * Initialize the buddy system arena.  This memory is used
	 * for the compressed page tables.
	 */

	areainit (budArenaBytes);

	/*
	 * Initialize the stack of free pages.
	 * __end is the virtual address just past kernel data
	 * Point sysmem.tfree to the lowest virtual address just above
	 * the buddy pool, and initialize sysmem.pfree there.
	 */

	sysmem.tfree = sysmem.pfree = 
			(unsigned short *) (__end + budArenaBytes);


	/* sysmem.hi is the physical page number just past high RAM */
	sysmem.hi = btocru (hi + ONE_MEG);


	/* base is the physical page number just past base RAM */
	base = sysmem.lo + (lo >> BPCSHIFT);


	/*
	 * Adjust sysmem.lo to be the physical page number just above
	 * not just the kernel, but above sysmem overhead as well.
	 */

	sysmem.lo = btocru (kerBytes + budArenaBytes + nalloc * sizeof (short));


	/*
	 * sysmem.vaddre is the virtual address of the next page after the
	 * kernel.
	 */

	sysmem.vaddre = (caddr_t)(ctob (sysmem.lo + SBASE-PBASE));


	/* include in system area pages for arena, free area */

	CHIRP ('3');

	/*
	 * 4.
	 *  Free the memory from [end, 640) kilobytes
	 *  Free the memory from [1024, 16 * 1024) kilobytes
	 *
	 *  We are building a stack of free pages bounded below
	 *  by sysmem.tfree and above by sysmem.efree.  sysmem.pfree
	 *  is the top of the stack.  The stack grows upwards.
	 */

	total_pages = 0;

	/*
	 * Initialize the sysmem table (phase 1 - base RAM).
	 * Put base RAM above the kernel and sysmem overhead area into
	 * sysmem pool.
	 */

	while (base > sysmem.lo) {
		* sysmem.pfree ++ = -- base;
		++ total_pages;
	}

	/*
	 * Initialize the sysmem table (phase 2 - extended RAM).
	 * Put all extended RAM into the sysmem pool.
	 */

	base = btocru (ONE_MEG);
	while (base < sysmem.hi && total_pages < nalloc) {
		* sysmem.pfree ++ = base ++;
		++ total_pages;
	}


	/*
	 * Roundoff error may have made nalloc smaller than necessary.
	 */

	while (base < sysmem.hi) {
		if ((unsigned int)sysmem.pfree + 1
		  >= (unsigned int)sysmem.vaddre)
			break;
		* sysmem.pfree ++ = base ++;
		++ total_pages;
		nalloc ++;
	}


	/*
	 * sysmem.efree points just past the last pointer in the sysmem
	 * table.
	 */

	sysmem.efree = sysmem.pfree;

	/*
	 * Grab a free page of physical memory.
	 *
	 * It will hold the page table entries for the second-from-top
	 * 4 MByte segment of virtual space.
	 * Point the second-to-last page directory entry at this
	 * physical page.
	 *
	 * ptable1 is the physical address of page just grabbed.
	 *
	 * ptab1_v holds the physical address of this page, relocated
	 * to compensate for the current data segment offset.
	 * So, ptab1_v [i] will contain the physical address (and
	 * some mode bits) for the 4KByte page of memory mapped to
	 * virtual address 0xFF80_0000 + (i * NBPC).
	 *
	 * Each nonzero entry in ptab1_v [] points to a 4 KByte
	 * page table.
	 *
	 * The corresponding 4 MByte region of virtual space,
	 * 0xFF80_0000 - (0xFFC0_0000-1), will hold page tables for
	 * various parts of the 4 GByte address space.
	 *
	 * Number 4 MByte segments of virtual space 0,1,..,3FF.
	 *
	 * Suppose we want to access memory in segment i.
	 * Then a 4 KByte page table must be allocated for segment i,
	 * say with physical address p.
	 *
	 * And, page directory entry i must point to the physical
	 * address of the page table entry.
	 *	pageDir [i] = p with mode bits
	 *
	 * If we expect to change which pages of physical memory will
	 * be mapped to segment i, the page table must itself be
	 * accessible, so we give it a virtual address in ptab1_v []:
	 *	ptab1_v [i] = p with mode bits
	 */

	ptable1 = ctob (* -- sysmem.pfree);		/* 5.d */
	pageDir [0x3FE] = ptable1 | SEG_SRW; 

	ptab1_v = (long *) (ptable1 + ctob (SBASE - PBASE));

	CHIRP ('4');

	/*
	 * 5. allocate page entries and initialize level 0 ^'s
	 * a. [ 00000000 .. 003FFFFF)		user code segment
	 * b. [ 00400000 .. 007FFFFF)		user data & bss
	 * c. [ 7FC00000 .. 7FFFFFFF)		user stack
	 * c.i.[ 80000000 .. 80FFFFFF)		ram disk
	 * d. [ FF800000 .. FFBFFFFF)		pointers to level 1 page table
	 * e. [ FFC00000 .. FFFFFFFF)		system process addresses
	 *
	 * F0000000 -> F7FFFFFF		128Mb of 1-1 mapped physical memory.
	 */

	/* 5.a */
	codeseg = ctob (* -- sysmem.pfree);
	ptab1_v [0] = pageDir [0] = codeseg | SEG_RW;

	/* 5.b */
	for (i = 0; i < NDATA; i ++)
		ptab1_v [i + 1] = pageDir [i + 1] =
				ctob (* -- sysmem.pfree) | SEG_RW;

	/* 5.c */
	ptab1_v [0x1FF] = pageDir [0x1FF] =
			ctob (* -- sysmem.pfree) | SEG_RW;

	/* 5.e */
	sysseg = ctob (* -- sysmem.pfree);
	ptab1_v [0x3FF] = pageDir [0x3FF] = sysseg | SEG_SRW;


	/*
	 * Create page directory and page table entries for the space that
	 * will hold the space mapped 1-1 with physical memory.
	 *
	 * The initial mapping includes whatever areas we are told are needed
	 * by the configuration process, as well as enough space for all the
	 * RAM we are told this machine has.
	 */

	physical_mapping_map |= (1 << btosru (hi + 0x100000UL)) - 1;

	for (i = 0 ; i < sizeof (physical_mapping_map) * CHAR_BIT ; i ++) {
	     	unsigned long	physpage;
	     	unsigned long *	phystable;
	     	int		j;

	     	/*
	     	 * Set up page directory to point to table table entries for
	     	 * this space, and also the part of the page-table area that
	     	 * maps itself.
	     	 */

	     	if ((physical_mapping_map & (1 << i)) == 0)
	     		continue;

	     	physpage = ctob (* -- sysmem.pfree);
		ptab1_v [i + btosrd (__PHYSICAL_MAP_BASE)] =
			pageDir [i + btosrd (__PHYSICAL_MAP_BASE)] =
				physpage | SEG_SRW;

		phystable = (unsigned long *)(physpage + ctob (SBASE - PBASE));
		physpage = stob (i);

		for (j = 0 ; j < ctob (1) / sizeof (cseg_t) ; j ++) {
			phystable [j] = physpage | SEG_SRW;
			physpage += NBPC;
		}
	}

	CHIRP ('5');


	/*
	 * 6. initialize  level 2 ^'s to [5.d]
	 *
	 * This ram disk stuff should go away once the scheme
	 * for allocating pieces of virtual memory space is in place.
	 */

	/* 5.c.i */
	for (ptoff = btosrd (RAM0) & 0x3ff;
	     ptoff < (btosrd (RAM0 + 2 * RAMSIZE) & 0x3ff); ++ ptoff) {
		ptab1_v [ptoff] = pageDir [ptoff] =
				ctob (* -- sysmem.pfree) | SEG_SRW; 
	}

	CHIRP ('6');

	/*
	 * 7.
	 * b. map kernel code and data
	 * 	map ^ to:
	 * c. 	level 0 page table
	 * d. 	level 1 page table
	 * e. 	I / O segments (video RAM, ...) 
	 */ 

	ptab1_v  = (long *) (sysseg + ctob (SBASE - PBASE));	/* 7.b */
	for (i = PBASE; i < sysmem.lo; i ++)
		ptab1_v [i - PBASE] = ctob (i) | SEG_SRW;

	ptab1_v [0x3FE] = ctob (PTABLE0_P) | SEG_SRW;		/* 7.c */
	ptab1_v [0x3FD] = ptable1 | SEG_SRW;			/* 7.d */

	init_phy_seg (ptab1_v, ROM - SBASE,   0x0000F0000);	/* 7.e. */
	init_phy_seg (ptab1_v, VIDEOa - SBASE, 0x0000B0000);
	init_phy_seg (ptab1_v, VIDEOb - SBASE, 0x0000B8000);

	CHIRP ('7');

	/*
	 * 8. allocate and map U area
	 */

	uinit.s_flags = SFSYST | SFCORE;
	uinit.s_size = UPASIZE;
	uinit.s_vmem = c_alloc (btocru (UPASIZE));
	ptab1_v [0x3FF] = * uinit.s_vmem | SEG_SRW;
	procq.p_segl [SIUSERP].sr_segp = & uinit;

	/* Clear the U area. */
	ua_paddr = ptab1_v[0x3ff] &~(NBPC - 1);
	memset((caddr_t)(ua_paddr + ctob(SBASE - PBASE)), 0, NBPC);

	CHIRP ('8');

	/*
	 * 9. make FFC00000 and 00002000 map to the same address
	 * to prevent the prefetch after the instruction turning on
	 * paging from causing a page fault
	 */

	((long *) (codeseg + ctob (SBASE - PBASE))) [PBASE] =
			ctob (PBASE) | SEG_SRW;

	CHIRP ('9');

	/*
	 * 10. load page table base address into MMU
	 *	fix up the interrupt vectors
	 */

	mmuupdnR0();
	CHIRP ('U');
	idtinit ();
	CHIRP ('I');
}
