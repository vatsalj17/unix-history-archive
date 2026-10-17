/* $Header: /ker/i386/RCS/mmu.c,v 2.5 93/10/29 00:57:12 nigel Exp Locker: nigel $ */
/*
 * MMU dependent code for Coherent 386
 *
 * Copyright (c) Ciaran O'Donnell, Bievres (FRANCE), 1991
 *
 * $Log:	mmu.c,v $
 * Revision 2.5  93/10/29  00:57:12  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.4  93/09/02  18:11:38  nigel
 * Minor edits to prepare for DDI/DKI integration
 * 
 * Revision 2.3  93/08/19  03:40:05  nigel
 * Nigel's R83
 */

#include <common/ccompat.h>
#include <common/_tricks.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <coh/misc.h>
#include <limits.h>

#define	_KERNEL		1

#include <kernel/trace.h>
#include <kernel/alloc.h>
#include <kernel/param.h>
#include <kernel/reg.h>
#include <sys/mmu.h>
#include <sys/proc.h>
#include <sys/clist.h>
#include <sys/inode.h>
#include <sys/seg.h>
#include <sys/buf.h>
#include <sys/filsys.h>
#include <l.out.h>
#include <ieeefp.h>


#define	PTABLE0_P	0x00001	/* Page directory physical address.	*/

#define	PPTABLE1_V	0xFFFFC	/* Virtual address of the page table
				 * for the virtual page table.
				 */

#define	INSERT2(p, pp) ((void) ((p)->forw = (pp), (p)->back = (pp)->back, \
			 	(pp)->back->forw = (pp)->back = (p)))

#define	DELETE2(p)	((p)->forw->back = (p)->back, \
			 (p)->back->forw = (p)->forw, \
			 (p)->forw = (p)->back = (p))

#define	INIT2(lp)	((lp)->forw = (lp)->back = (lp))


extern unsigned int total_mem;	/* Total physical memory in bytes.  */

#define BUDDY(addr,size)	((addr) ^ (1 << (size)))
#define	SPLASH	3
#define	NDATA	4	/* process data segments			*/
#define	BLKSZ	2	/* log2 sizeof(BLOCKLIST)/sizeof(cseg_t)	*/

/* These defines belong somewhere else:  */
#define LOMEM	0x15	/* CMOS address of size in K of memory below 1MB.  */
#define EXTMEM	0x17	/* CMOS address of size in K of memory above 1MB.  */
#define ONE_K	1024
#define ONE_MEG	1048576
#define USE_NDATA	1

/*
 * End of kernel (i.e., end of BSS).
 */

extern char __end [];


/*
 * For 0 < i < 64, buddysize [i] is log (base 2) of nearest power of two
 * which is greater than or equal to i.
 */

char	buddysize [64] = {
	-1, 0, 1, 2, 2, 3, 3, 3,
	3, 4, 4, 4, 4, 4, 4, 4,
	4, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6 };

#define	min(a, b)	((a) < (b) ? (a) : (b))


/*
 * Is 'p' a valid physical page address?
 */

#define	pvalid(p)	((p) >= sysmem.lo && (p) < sysmem.hi)


/*
 * Functions.
 *	Import.
 *	Export.
 *	Local.
 */
void		areacheck ();
void		areafree ();
void		areainit ();
struct __blocklist *	arealloc ();
int		areasize ();
cseg_t *	c_extend (); 
int		c_grow ();
int		countsize ();
char *		getPhysMem ();
void		physMemInit ();
SR		* loaded ();
void		sunload ();
void		valloc ();


/* Read a 16 byte number from the CMOS.  */

#if	__USE_PROTO__
unsigned int read16_cmos (unsigned int addr)
#else
unsigned int
read16_cmos (addr)
unsigned int addr;
#endif
{
        unsigned char read_cmos ();
	
	return (read_cmos (addr + 1) << 8) + read_cmos (addr);
}


/*
 * Load a segment's page entries into the page table.
 */

#if	__USE_PROTO__
void load_seg_pages (SR * srp)
#else
void
load_seg_pages (srp)
SR	      *	srp;
#endif
{
	int		n;
	cseg_t	      *	pp;
	int		base1;
	int		akey;

	pp = srp->sr_segp->s_vmem;
	base1 = btocrd (srp->sr_base);
	n = btocru (srp->sr_size);

	/*
	 * we load all pages
	 */

	 /* a shm segment ref may be Read-Write or Read-Only */
	if (srp->sr_flag & SRFRODT)
		akey = SEG_RO;
	else {
		switch (srp->sr_segp->s_flags & (SFSYST | SFTEXT)) {
		case SFTEXT:
			akey = SEG_RO;
			break;

		case SFSYST:
			akey = SEG_SRW;
			break;

		default:
			akey = SEG_RW;
			break;
		}
	}

	do
		ptable1_v [base1 ++] = (* pp ++ & ~ SEG_NPL) | akey;
	while (-- n);
}


/*
 * Mark all the pages that are occupied by a segment as being invalid in the
 * active page table.
 */

#if	__USE_PROTO__
void remove_seg_pages (SR * srp)
#else
void
remove_seg_pages (srp)
SR	      *	srp;
#endif
{
	int		n;
	int		base1;

	base1 = btocrd (srp->sr_base);
	
	n = btocru (srp->sr_size);
	do {
		ptable1_v [base1 ++] = SEG_ILL;
	} while (-- n);
}


/*
 * Load pages from a segment into the page table, and request a paging TLB
 * flush.
 */

#if	__USE_PROTO__
void doload (SR * srp)
#else
void
doload (srp)
SR	      *	srp;
#endif
{
	load_seg_pages (srp);
	mmuupd ();
}


/*
 * Remove the pages that are occupied by a segment from the active page table,
 * and request a paging TLB flush.
 */

#if	__USE_PROTO__
void unload (SR * srp)
#else
void
unload (srp)
SR	      *	srp;
#endif
{
	remove_seg_pages (srp);
	mmuupd ();
}


/*
 * Allocate 'pages_wanted' pages of core space.
 * Returns physical segment descriptor if success, else NULL.
 * The physical segment descriptor is a table of page table entries
 * suitable for insertion into a page table.
 */

cseg_t *
c_alloc (pages_wanted)
unsigned	pages_wanted;
{
	unsigned	pno;
	cseg_t * pp;
	register cseg_t * qp;

	/* Do we have enough free physical pages for this request?  */
	if (pages_wanted > allocno ())
		return NULL;

	/* Allocate some space for the table to return.  */
	if ((pp = (cseg_t *) arealloc (pages_wanted)) == NULL)
		return NULL;
	qp = pp;

	/* fill in entries in the requested table */
	do {
		pno = * -- sysmem.pfree;
		if (! pvalid (pno))
			panic ("c_alloc");
		* qp ++ = (ctob (pno) & ~ SEG_BITS) | SEG_PRE;
	} while (-- pages_wanted);
	return pp;
}


/*
 * Given an array "pp" containing "numPages" page descriptors,
 *   if "pp" is the page list for a user segment currently loaded
 *     invalidate page entries for "pp" in the current page table
 *   return each page in "pp" to the sysmem pool, if it came from there.
 *   return the array "pp" to the buddy pool.
 */

void
c_free (pp, numPages)
cseg_t	* pp;
unsigned	numPages;
{
	unsigned	pno;
	register cseg_t * qp;
	register int	sz;
	SR		* srp;

	if ((srp = loaded (pp)) != NULL) {
		unload (srp);
		srp->sr_segp = 0;
	}
	sz = numPages;
	if (& sysmem.pfree [sz] > sysmem.efree)
		panic ("c_free - nalloc");
	qp = pp;
	do {
		if ((* qp & SEG_NPL) == 0) {
			pno = btocrd (* qp);
			if (! pvalid (pno))
				panic ("c_free");
			* sysmem.pfree ++ = pno;
		} else {
			T_HAL (0x40000, printf ("c_free NPL %x ", * qp));
		}
		qp ++;
	} while (-- sz);
	areafree ((struct __blocklist *) pp, numPages);
}

MAKESR (physMem, _physMem);
extern int	PHYS_MEM;	/* Number of bytes of contiguous RAM needed */


/*
 * A block of contiguous physical memory has been allocated for special
 * i / o devices.
 * Problem: pages of physical memory are in reverse order in the
 * page table.
 * This routine reverses the page table entries for the pages
 * involved.  It relies * heavily * on all pages having virtual addresses
 * in the FFCx xxxx segment.
 *
 * If all goes well, assign physAvailStart to the virtual address of
 * the beginning of the region, and physAvailBytes to the number of bytes
 * in the region.  Otherwise, leave physAvailStart and physAvailBytes at 0.
 *
 * As memory is allocated, physAvailStart advances to point to the next
 * available byte of contiguous memory, physAvailBytes is decremented,
 * and physPoolStart remains set to the virtual address of the start of
 * the contiguous pool.
 */

static int	physPoolStart;	/* start of contiguous memory area */
static int	physAvailStart;	/* next free byte in contiguous memory area */
static int	physAvailBytes;	/* number of bytes in contiguous memory area */



void
physMemInit ()
{
	int m;
	int err = 0, num_pages = btocru (PHYS_MEM);
	int prevPaddr, paddr;

	/*
	 * Going half way into page table for physMem
	 *   If entry and its complementary entry aren't both in top segment
	 *     Error exit (no phys mem will be available).
	 *   Get page table entries and swap them.
	 */
	for (m = 0; m < num_pages / 2; m ++) {
		int m2 = num_pages - 1 - m;	/* complementary index */

		/* compute virtual addresses */
		int lo_addr = physMem.sr_base + ctob (m);
		int hi_addr = physMem.sr_base + ctob (m2);

		/* compute indices into page table (ptable1_v) */
		int lo_p1ix = btocrd (lo_addr);
		int hi_p1ix = btocrd (hi_addr);

		/* fetch physical addresses from page table */
		int lo_paddr = ptable1_v [lo_p1ix];
		int hi_paddr = ptable1_v [hi_p1ix];

		/* abort if either address is not in top segment */
		if (btosrd (lo_addr) != 0x3FF) {
			err = 1;
			break;
		}
		if (btosrd (hi_addr) != 0x3FF) {
			err = 1;
			break;
		}

		/* exchange page table entries */
		ptable1_v [lo_p1ix] = hi_paddr;
		ptable1_v [hi_p1ix] = lo_paddr;
	}

	/*
	 * Final sanity check.
	 * In case someone gets creative with startup code, check
	 * again here that the memory is actually contiguous.
	 */
	prevPaddr = __coh_vtop (physMem.sr_base);
	for (m = 0; m < num_pages - 1; m ++) {
		paddr = __coh_vtop (physMem.sr_base + ctob (m + 1));
		if (paddr - prevPaddr != NBPC) {
			err = 1;
			break;
		}
		prevPaddr = paddr;
	}

	if (! err) {
		physPoolStart = physAvailStart = physMem.sr_base;
		physAvailBytes = PHYS_MEM;
	}
}


/*
 * Return virtual address of block of contiguous physical memory.
 * If request cannot be granted, return 0.
 *
 * Expect physMem resource to be granted during load routine of device
 * drivers.  Once allocated, memory is not returned to the physMem pool.
 */

char *
getPhysMem (numBytes)
unsigned	numBytes;
{
	char * ret;

	if (numBytes > physAvailBytes) {
		printf ("getPhysMem failed - %d additional bytes "
			 "PHYS_MEM needed\n", physAvailBytes - numBytes);
		return NULL;
	}

	ret = (char *) physAvailStart;
	physAvailStart += numBytes;
	physAvailBytes -= numBytes;

	return ret;
}


/*
 * Return virtual address of aligned block of contiguous physical memory.
 * Mainly for devices using the stupid Intel DMA hardware without
 * scatter/gather.
 * If request cannot be granted, return 0.
 *
 * Argument "align" says what physical boundary we need alignment on.
 * It must be a power of 2.
 * For 4k alignment, align = 4k, etc.
 * Sorry, but will throw away memory to get to the next acceptable address.
 *
 * Once allocated, memory is not returned to the physMem pool.
 */

char *
getDmaMem (numBytes, align)
unsigned	numBytes;
unsigned	align;
{
	int		wastedBytes;

	if (align == 0) {
		printf ("getDmaMem (0) (?)\n");
		return NULL;
	}

	if (! __IS_POWER_OF_TWO (align)) {
		printf ("getDmaMem (%x) (?)\n", align);
		return NULL;
	}

	/*
	 * Waste RAM from bottom of pool up to physical
	 * address with desired alignment.
	 */

	wastedBytes = align - (__coh_vtop (physAvailStart) % align);

	if (getPhysMem (wastedBytes) == NULL)
		return NULL;

	return getPhysMem (numBytes);
}

/*
 * Check whether a range of physical addresses lies within the
 * pool of contiguous physical memory.
 */

#if	__USE_PROTO__
int physValid (unsigned int base, unsigned int numBytes)
#else
int
physValid (base, numBytes)
unsigned int base, numBytes;
#endif
{
	int vpool;
	int ret = 0;

	if (PHYS_MEM) {
		vpool = __coh_vtop (physPoolStart);
		T_HAL (0x40000, printf ("PHYS_MEM phys addrs %x to %x  ",
		  vpool, vpool + PHYS_MEM));
		if (base >= vpool && (base + numBytes) <= (vpool + PHYS_MEM))
			ret = 1;
	} else {
		T_HAL (0x40000, printf ("No PHYS_MEM "));
	}

	T_HAL (0x40000, printf ("physValid (%x, %x) = %d ", base, numBytes, ret));
	return ret;
}


/*
 * Given a user virtual address, a physical address, and a byte
 * count, map the specified virtual address into the user data
 * page table for the current process.
 *
 * This is meant to be called from the console ioctl, KDMAPDISP.
 * The user virtual address must be page aligned.
 * The range of physical addresses must lie outside installed RAM
 * or within the "PHYS_MEM" pool.
 *
 *
 * Return 1 on success, else 0.
 */

#if	__USE_PROTO__
int mapPhysUser (__caddr_t virtAddr, int physAddr, int numBytes)
#else
int
mapPhysUser (virtAddr, physAddr, numBytes)
__caddr_t virtAddr;
int physAddr;
int numBytes;
#endif
{
	int ret = 0;
	SR * srp = SELF->p_segl + SIPDATA;
	SEG * sp = srp->sr_segp;
	cseg_t * pp = sp->s_vmem, * qp;
	int pno, pageOffset, numPages, i;

	/* Check alignment. */
	if (((int)virtAddr & (NBPC - 1)) || (physAddr & (NBPC - 1))) {
		T_HAL (0x40000, printf ("mPU: failed alignment "));
		goto mPUdone;
	}

	/*
	 * If "numBytes" is not a multiple of page size,
	 * round it up before proceeding.
	 */
	numBytes = __ROUND_UP_TO_MULTIPLE(numBytes, NBPC);

	/* Check validity of range of virtual addresses. */
	if (virtAddr < srp->sr_base ||
	    virtAddr + numBytes >= srp->sr_base + srp->sr_size) {
		T_HAL (0x40000, printf ("mPU: bad vaddr "));
		goto mPUdone;
	}

	/* Check validity of range of physical addresses. */
	/* if not in PHYS_MEM pool... */
	if (! physValid (physAddr, numBytes)) {

		/* get installed RAM physical addresses */
		unsigned int physLow = ctob ((read16_cmos (LOMEM) + 3) >> 2);
		unsigned int physHigh = ctob ((read16_cmos (EXTMEM) + 3) >> 2)
		  + ONE_MEG;

		T_HAL (0x40000, printf ("physLow =%x physHigh =%x ",
		  physLow, physHigh));

		/* Fail if physical range overlaps installed base RAM. */
		if (physAddr < physLow) {
			T_HAL (0x40000, printf ("mPU: overlap base RAM "));
			goto mPUdone;
		}

		/* Fail if physical range overlaps installed extended RAM. */
		if (physAddr < physHigh && physAddr + numBytes >= ONE_MEG) {
			T_HAL (0x40000, printf ("mPU: overlap extended RAM "));
			goto mPUdone;
		}
	}

	/*
	 * For each page in user data segment which is to be remapped
	 *   if current page was taken from sysmem pool
	 *     return current page to sysmem pool
	 *   write new physical address into current page entry
	 *   mark current page as not coming from sysmem pool
	 *   map current page into page table
	 */
	/* NIGEL: cast to long to make GCC compile this */
	pageOffset = btocrd (virtAddr - (long) srp->sr_base);
	numPages = numBytes >> BPCSHIFT;
	for (qp = pp + pageOffset, i = 0; i < numPages; i ++, qp ++) {
		if ((* qp & SEG_NPL) == 0) {
			pno = btocrd (* qp);
			if (! pvalid (pno)) {
				T_HAL (0x40000, printf ("mPU: bad release "));
			} else {
				* sysmem.pfree ++ = pno;
				T_HAL (0x40000,
				  printf ("mPU: freeing virtual page %x ",
				  virtAddr + ctob (i)));
			}
		} else {
			T_HAL (0x40000,
			  printf ("mPU: rewriting virtual NPL page %x ",
			  virtAddr + ctob (i)));
		}
		* qp = (physAddr + ctob (i)) | (SEG_RW | SEG_NPL);
		ptable1_v [btocrd (virtAddr) + i] = * qp;
	}
	mmuupd ();
	ret = 1;

mPUdone:
	return ret;
}


/*
 * Add a page to a segment.
 * Enlarge buddy table for segment, if needed.
 *
 * Arguments:
 *	pp points to segment reference table (segp->s_vmem, e.g.)
 *	osz is old segment size, in pages
 *
 * Return pointer to enlarged segment reference table, or NULL if failed.
 */

cseg_t *
c_extend (pp, osz) 
register cseg_t * pp;
int osz;
{
	register	cseg_t * pp1;
	register unsigned	pno;
	register int	i;
	SR		* srp;

	/* Fail if no more free pages available. */
	if (sysmem.pfree < & sysmem.tfree [1])
		goto no_c_extend;

	/* Don't grow segment beyond hardware segment size (4 megabytes). */
	if (osz >= (NBPS / NBPC))
		goto no_c_extend;

	if (srp = loaded (pp)) {
		unload (srp);
		srp->sr_segp = 0;
	}

	/*
	 * If the old size was a power of 2, it has used up an entire
	 * buddy area, so we will need to allocate more space.
	 */

	if (__IS_POWER_OF_TWO (osz)) {
		if ((pp1 = (cseg_t *) arealloc (osz + 1))== 0)
			goto no_c_extend;
		for (i = 0; i < osz; i ++)
			pp1 [i] = pp [i];
		areafree (pp, osz);
		pp = pp1;
	}

	for (i = osz; -- i >= 0;)
		pp [i + 1] = pp [i];

	pno = * -- sysmem.pfree;
	if (! pvalid (pno))
		panic ("c_extend");
	pp [0] = ctob (pno) | SEG_RW;
	return pp;

no_c_extend:
	return 0;
}


/*
 * Given segment size in bytes, estimate total space needed
 * to keep track of the segment (I think - hws).
 *
 * return value is num_bytes plus some overhead...
 */

int
countsize (num_bytes)
int num_bytes;
{
	int ret;

	if (num_bytes <= NBPC / sizeof (long))
		ret = num_bytes + 1;
	else
		ret = num_bytes
		  + ((num_bytes + NBPC / sizeof (long) - 1) >> BPC1SHIFT) + 1;
	return ret;
}


/*
 * buddy allocation 
 */

/*
 * Deallocate a segment descriptor area.
 * "sp" is not really a struct __blocklist *, rather a cseg_t *.
 * "numPages" is the number of pages referenced in the area.
 */

void
areafree (sp, numPages)
struct __blocklist * sp;
int numPages;
{
	int	n;	/* adresse du buddy, taille du reste */
	int	ix, nx;
	struct __blocklist * buddy;

	areacheck (2, sp);

	/*
	 * Pointer "sp" points to an element in the sysmem table of
	 * free pages.
	 * Integer "ix" is the index of "sp" into that table.
	 * Will use "ix" to index into one or more buddy tables.
	 */
	ix = sp - sysmem.u.budtab;
	n = areasize (numPages);
	do {
		/* "nx" is index of buddy element to the one at "ix". */
		nx = BUDDY (ix, n);
		if (sysmem.budfree [nx >> WSHIFT] & 1 <<(nx &(WCOUNT-1))) {
			/* coalesce two buddies */
			buddy = sysmem.u.budtab + nx; 
			if (buddy->kval != n)
				break;
			sysmem.budfree [nx >> WSHIFT] &= ~ (1 <<(nx & (WCOUNT-1)));
			DELETE2(buddy);
			if (nx < ix) 
				ix = nx;
		} else
			break;
	} while (++ n < NBUDDY);
	sysmem.budfree [ix >> WSHIFT] |= 1 << (ix & (WCOUNT-1));
	buddy = sysmem.u.budtab + ix;
	INSERT2 (buddy, & sysmem.bfree [n]);
	buddy->kval = n;
	areacheck (3, buddy);
}


/*
 * arealloc ()
 *
 * Given size in "pages" of a segment to manage,
 * return pointer to an array of enough descriptors.
 * If not enough free descriptors available, return 0.
 */

struct __blocklist *
arealloc (pages)
int pages;
{
	struct __blocklist * sp;
	struct __blocklist * p, * q;
	int	size;
	struct __blocklist * rsp;
	int	nx;

	areacheck (0, 0);
	size = areasize (pages);
	/*
	 * 1. Find little end, bloc p, free >= size
	 */
	for (q = p = sysmem.bfree + size;p->forw == p; size ++, p ++)
		if (p >= sysmem.bfree + NBUDDY - 1) {
			return 0;	/* y en a pas */
		}

	rsp = p->forw;
	DELETE2(rsp);
	nx = rsp - sysmem.u.budtab;
	sysmem.budfree [nx >> WSHIFT] &= ~(1 << (nx & (WCOUNT-1)));
	size = 1 << size;
	sp = rsp + size; /* buddy address */
	while (p-- != q) {
		/*
		 * 2.1 The block is too big, uncouple & free buddy
		 */
		sp -= (size >>= 1);
		nx = sp - sysmem.u.budtab;
		sysmem.budfree [nx >> WSHIFT] |= 1 << (nx & (WCOUNT-1));
		INSERT2 (sp, p);
		sp->kval = p - sysmem.bfree;
	}
	areacheck (1, rsp);
	return rsp;
}


void
areainit (n)
{
	register int i;

	for (i = 0; i < (1 <<(NBUDDY-WSHIFT)); i ++)
		sysmem.budfree [i] = 0;
	for (i = 0; i < NBUDDY; i ++)
		INIT2(& sysmem.bfree [i]);
	sysmem.u.budtab = (struct __blocklist *) __end;
	n /= sizeof (struct __blocklist);
	if (n > (1 << NBUDDY))
		panic ("areainit");
	for (i = 0; i < n; i ++)
		areafree (& sysmem.u.budtab [i],
			  sizeof (struct __blocklist) / sizeof (long));
}


/*
 * areasize ()
 *
 * Do a log (base 2) calculation on n.
 * If n is zero, return -1.
 *
 * Else, consider the nearest power of two which is greater than or
 * equal to n
 *	p/2 < n <= p
 * Then set p = 4 * (2 ** x).  Note BLKSZ is 2.
 * Return max (x, 0).
 *
 * If n is too large (more than 3F00), we will go beyond the limits of
 * table buddysize [].
 *
 * In practice, n is the total number of pages needed in a segment,
 * and the return value will be used to access a buddy system list.
 *
 * The buddy system tracks memory in 4-page chunks.
 * areasize(pagecount) returns the log base 2 of the number of chunks,
 * which gives an index into the buddy list tracking regions just large
 * enough to accommodate pagecount.
 */

int
areasize (n)
register unsigned int	n;
{
	register int m;
#ifdef FROTZ
	int ret, oldn = n;
#endif

	if (n > 0x3F00)
		panic ("areasize");

	n = (n + (1 << BLKSZ) - 1) >> BLKSZ;
	m = n & 0x3F;
#ifdef FROTZ
	if ((n >>= 6) == 0)
		ret = buddysize [m];
	else {
		int index;

		index = n;
		if (m)
			index ++;
		ret = buddysize [index] + 6;
	}
	return ret;
#else
	if ((n >>= 6) == 0)
		return buddysize [m];
	return buddysize [n + ((m!= 0)?1:0)] + 6;
#endif
}


#define	MAXBUDDY	2048
#define	CHECK(p) ((p>=& sysmem.bfree [0] && p <& sysmem.bfree [NBUDDY]) || \
		(p >= sysmem.u.budtab && p <& sysmem.u.budtab [1 << NBUDDY]))
void
areacheck (flag, sp)
struct __blocklist * sp;
{
	struct __blocklist * next, * start;
	int i, nx;

	if (sp) {
		if (& sysmem.u.budtab [sp-sysmem.u.budtab] != sp)
		  printf ("* check * %d %x %x\n", flag, sp, sysmem.u.budtab);
	}
		
	for (i = 0; i < NBUDDY; i ++) {
		start = next = & sysmem.bfree [i];
		do {
			next = next->forw;
			if (! CHECK (next))
				printf ("next = %x (%d)\n", next, flag);
			if (next->back != start)
				printf ("%x->forw->back != %x\n", next, start);
			if (next != & sysmem.bfree [i]) {
				if (next->kval != i)
					printf ("bad kval %x, %d (%d)\n",
						next, next->kval, flag);
				nx = next - sysmem.u.budtab;
				if ((sysmem.budfree [nx >> WSHIFT] & (1 << (nx & (WCOUNT-1)))) == 0)
					printf ("in bfree but not budfree %x (%d)\n", next, flag);
			}
			start = next;
		} while (next != & sysmem.bfree [i]);
	}
}

/*
 * Load up segmentation registers.
 */

SR	ugmtab [NUSEG];

void
segload ()
{
	int i;
	SR * start;

	/*
	 * unprogram the currently active UGM user segments
	 */

	for (start = ugmtab + 1; start < ugmtab + NUSEG ; start ++) {
		if (start->sr_segp != NULL)
			remove_seg_pages (start);
		start->sr_segp = NULL;
	}


	/*
	 * Load each segment in the p->p_region list into the MMU
	 * Remember values in ugmtab.
	 */

	start = ugmtab + 1;
	for (i = 1; i < NUSEG; i ++) {
		if (SELF->p_segl [i].sr_segp == NULL)
			continue;

		* start = SELF->p_segl [i];
		switch (i) {
		case SIPDATA:
			if (SELF->p_segl [SISTACK].sr_base == 0)
				break;

			start->sr_size = min (start->sr_size,
				(long) SELF->p_segl [SISTACK].sr_base -
				  SELF->p_segl [SISTACK].sr_size);
			break;

		case SISTACK:
			start->sr_base -= start->sr_size;
			break;
		}

		load_seg_pages (start);
		start ++;
	}

	/*
	 * Update shm segment information, then flush the paging TLB.
	 */

	shmLoad ();
	mmuupd ();
}


SR *
loaded (pp)
cseg_t * pp;
{
	SR	* start;

	for (start = ugmtab; start < ugmtab + NUSEG; start ++)
		if (start->sr_segp && start->sr_segp->s_vmem == pp)
			return start;
	return NULL;
}

MAKESR (r0stk, _r0stk);
extern int tss_sp0;


/*
 * General initialization
 */

void
i8086()
{
	unsigned	csize, isize, allsize;
	caddr_t	base;
	unsigned int	calc_mem, boost;
	extern	caddr_t		clistp;
	extern	SR		blockp;
	extern	SR		allocp;

	/* This is the first C code executed after paging is turned on. */

	workPoolInit ();

	/*
	 * Allocate contiguous physical memory if PHYS_MEM is patched
	 * to a nonzero value.
	 */

	if (PHYS_MEM) {
		physMem.sr_size = (PHYS_MEM + NBPC - 1) & ~ (NBPC - 1);
		valloc (& physMem);
		physMemInit ();
	}

	/*
	 * Allocate a page for ring 0 stack.
	 */

	r0stk.sr_size = NBPC;
	valloc (& r0stk);
	tss_sp0 = r0stk.sr_base + NBPC;

	/*
	 * calc_mem is used for autosizing buffer cache and kalloc pool.
	 * It is total_mem, limited below by 1 meg and above by 12 meg.
	 * The upper limit is a temporary move to allow booting on 16 Meg
	 * systems.
	 *
	 * "boost" is used in autosizing buffer cache and kalloc pool.
	 * It is the number of megabytes of calc_mem above 1 meg, i.e.,
	 * a number between 0 and 11.
	 */

	if (total_mem < ONE_MEG)
		calc_mem = ONE_MEG;
	else if (total_mem > 12 * ONE_MEG)
		calc_mem = 12 * ONE_MEG;
	else
		calc_mem = total_mem;

	boost = (calc_mem - ONE_MEG) / ONE_MEG;

	/*
	 * If the number of cache buffers was not explicitly set (i.e., ! 0)
	 * then calculate the number of buffers using the simple heuristic:
	 *     128 minimum + 400 per MB of available RAM (i.e., after 1MB)
	 */

	if (NBUF == 0)
		NBUF = 128 + (400 * boost);

	/*
	 * Calculate NHASH as the next lower prime number from NBUF.
	 */

	NHASH = nlp (NBUF);


	/*
	 * If the amount of kalloc () space was not explicitly set (i.e., ! 0)
	 * then calculate using the simple heuristic:
	 *     64k minimum + 32k per MB of available RAM (i.e., after 1MB)
	 */
	if (ALLSIZE == 0)
		ALLSIZE = 65536 + (32768 * boost);

	blockp.sr_size = NBUF * BSIZE;
	valloc (& blockp);

	allocp.sr_size = allsize = NBUF * sizeof (BUF) + ALLSIZE;
	allocp.sr_size += isize = NINODE * sizeof (INODE);
	allocp.sr_size += csize = NCLIST * sizeof (CLIST);
	valloc (& allocp);
	base = allocp.sr_base;

	KMEM_INIT (base, allsize);

	base += allsize;
	inode_table = (struct inode *) base;
	base += isize;
	clistp = base;
}


/*
 * Allocate srp->sr_size bytes of physical memory, and map it into
 * virtual memory space.  At the end, the struct at srp will describe
 * the new segment.
 */

void
valloc (srp)
SR	* srp;
{
	register int npage;

	/*
	 * If we've run out of virtual memory space, panic ().
	 *
	 * A more graceful solution is needed, but valloc () does
	 * not provide a return value.
	 */
	if (sysmem.vaddre + srp->sr_size > MAX_VADDR) {
		panic ("valloc: out of virtual memory space");
	}

	npage = btocru (srp->sr_size);

	srp->sr_base = sysmem.vaddre;
	srp->sr_segp->s_size = srp->sr_size;
	srp->sr_segp->s_vmem = c_alloc (npage);
	srp->sr_segp->s_flags = SFSYST | SFCORE;
	doload (srp);

	sysmem.vaddre += ctob (npage);
}


/*
 * See if the given process may fit in core.
 */

int
testcore (pp)
PROC * pp;
{
	return 1;
}

/*
 * Calculate segmentation for a
 * new program. If there is a stack segment
 * present merge it into the data segment and
 * relocate the argument list.
 * Make sure that the changes are reflected in the SELF->p_segl array
 * which sproto sets up.
 */

int
mproto ()
{
	return 1;
}


#if __USE_PROTO__
int accdata (__caddr_t base, size_t count)
#else
int
accdata (base, count)
__caddr_t	base;
size_t		count;
#endif
{
	SR * srp;

	srp = & SELF->p_segl [SIPDATA];
	return base >= srp->sr_base
	  && base + count <= srp->sr_base + srp->sr_size;
}

#if __USE_PROTO__
int accstack (__caddr_t base, size_t count)
#else
int
accstack (base, count)
__caddr_t	base;
size_t		count;
#endif
{
	SR * srp;

	srp = & SELF->p_segl [SISTACK];
	return base >= srp->sr_base - srp->sr_size
	  && base + count <= srp->sr_base;
}

#if __USE_PROTO__
int acctext (__caddr_t base, size_t count)
#else
int
acctext (base, count)
__caddr_t	base;
size_t		count;
#endif
{
	SR * srp;

	srp = & SELF->p_segl [SISTEXT];
	return base >= srp->sr_base
	  && base + count <= srp->sr_base + srp->sr_size;
}


/*
 * Grow a segment - increase its size to the desired new length in bytes.
 * Return 0 on success, -1 on failure.
 * Possible failures:
 *   attempt to grow a segment to smaller than its present size
 *   not enough pages available in free pool
 *   can't allocate a new descriptor vector
 */
int
c_grow (sp, new_bytes)
SEG * sp;
int new_bytes;
{
	register int	i;
	register cseg_t * pp;
	int		new_pages, pno, nsize, old_pages;
	SR		* srp;

	T_PIGGY (0x8000000, printf ("c_grow (sp: %x, new: %x)", sp, new_bytes));

	new_pages = btocru (new_bytes);
	old_pages = btocru (sp->s_size);

	if (new_pages == old_pages)
		return 0;

	if (new_pages < old_pages) {
		printf ("%s:can't contract segment\n", SELF->p_comm);
		return -1;
	}

	if (new_pages - old_pages > allocno ())
		return -1;

	T_PIGGY (0x8000000, printf ("nc: %x, oc: %x,", new_pages, old_pages));

	/*
	 * Allocate a new descriptor vector if necessary.
	 * pp is the element corresponding to the virtual address
	 * "0"(sr_base)
	 */

	pp = sp->s_vmem;
	nsize = areasize (new_pages);
	if (nsize != areasize (old_pages) &&
	    (pp = (cseg_t *) arealloc (new_pages)) == NULL) {
		T_PIGGY (0x8000000,
			 printf ("Can not allocate new descriptor."));
		return -1;
	}

	T_PIGGY (0x8000000, printf ("new pp: %x", pp));

	if ((srp = loaded (sp->s_vmem)) != NULL) {
		T_PIGGY (0x8000000, printf ("unloading srp: %x, ", srp));
		unload (srp);
		srp->sr_segp = 0;
	}

	/*
	 * Allocate new descriptors.
	 */

	T_PIGGY (0x8000000, printf ("new desc: ["));
	for (i = old_pages; i < new_pages; i ++) {
		pno = * -- sysmem.pfree;
		pp [i] = ctob (pno) | SEG_RW;
		T_PIGGY (0x8000000, printf ("%x, ", pp [i]));
	}
	T_PIGGY (0x8000000, printf ("]"));

	/*
	 * Copy unchanged descriptors and free old vector if necessary.
	 */

	if (pp != sp->s_vmem) {
		T_PIGGY (0x8000000, printf ("old desc: ["));
		for (i = 0; i < old_pages; i ++) {
			pp [i] = sp->s_vmem [i];
			T_PIGGY (0x8000000, printf ("%x, ", pp [i]));
		}
		T_PIGGY (0x8000000, printf ("]"));
		areafree ((struct __blocklist *) sp->s_vmem, old_pages);
	}

	sp->s_vmem = pp;

	/*
	 * clear the added pages
	 *
	 * MAPIO macro - convert array of page descriptors, offset
	 *   into system global address.
	 */

	T_PIGGY (0x8000000, printf ("dmaclear (%x, %x, 0)", 
				ctob (new_pages - old_pages),
				MAPIO (sp->s_vmem, ctob (old_pages))
			   )); /* T_PIGGY () */

	dmaclear (ctob (new_pages - old_pages),
		  MAPIO (sp->s_vmem, ctob (old_pages)));
	return 0;
}
