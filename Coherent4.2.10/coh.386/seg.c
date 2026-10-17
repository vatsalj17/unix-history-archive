/* $Header: /ker/coh.386/RCS/seg.c,v 2.6 93/10/29 00:55:34 nigel Exp Locker: nigel $ */
/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or persuant to the license agreement is unlawful.
 *
 *	COHERENT Version 2.3.37
 *	Copyright (c) 1982, 1983, 1984.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
/*
 * Coherent.
 * Segment manipulation.
 *
 * $Log:	seg.c,v $
 * Revision 2.6  93/10/29  00:55:34  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.5  93/09/02  18:08:01  nigel
 * Prepare for DDI/DKI merge
 * 
 * Revision 2.4  93/08/19  03:26:45  nigel
 * Nigel's r83 (Stylistic cleanup)
 */

#include <sys/errno.h>
#include <stddef.h>

#define	_KERNEL		1

#include <kernel/alloc.h>
#include <kernel/trace.h>
#include <kernel/reg.h>
#include <sys/uproc.h>
#include <sys/proc.h>
#include <sys/mmu.h>
#include <sys/buf.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/proc.h>
#include <sys/sched.h>
#include <sys/seg.h>
#include <a.out.h>


/*
 * NIGEL: Wrap up the garbage before we take it out.
 */
 
__DUMB_GATE	__seglink = __GATE_DECLARE ("segment list");

#define	__LOCK_SEGMENT_LIST(where) \
		(__GATE_LOCK (__seglink, "lock : seglink " where))
#define	__UNLOCK_SEGMENT_LIST() \
		(__GATE_UNLOCK (__seglink))

SEG	segmq;				/* seg.h */

#define	min(a, b)	((a) < (b) ? (a) : (b))

/*
 * Initialisation code.
 */

void
seginit()
{
	/*
	 * Create empty circular-list of memory segments.
	 */

	segmq.s_forw = & segmq;
	segmq.s_back = & segmq;
}


/*
 * Given an inode, `ip', and flags, `ff', describing a segment associated
 * with the inode, see if the segment already exists and if so, return a
 * copy.  If the segment does not exist, allocate the segment having size
 * `ss', and read the segment using the inode at seek offset `dq' with a
 * size of `ds'.
 */

SEG *
ssalloc(ip, ff, ss)
INODE *ip;
int ff;
int ss;
{
	SEG *sp;
	int f;

	__LOCK_SEGMENT_LIST ("ssalloc ()");

	f = ff & (SFSHRX | SFTEXT);

	/*
	 * Look for the segment in the memory queue.
	 */

	for (sp = segmq.s_forw ; sp != & segmq ; sp = sp->s_forw) {

		if (sp->s_ip == ip &&
		    (sp->s_flags & (SFSHRX | SFTEXT)) == f) {

			__UNLOCK_SEGMENT_LIST ();
			return segdupl (sp);
		}
	}
	__UNLOCK_SEGMENT_LIST ();

	/*
	 * Allocate and create the segment.
	 */

	return salloc (__ROUND_UP_TO_MULTIPLE (ss, NBPC), ff);
}


/*
 * Free the given segment pointer.
 */

void
sfree (sp)
SEG *sp;
{
	INODE *ip;

	if (sp->s_urefc != 1) {
		sp->s_urefc --;
		sp->s_lrefc --;
		return;
	}

	__LOCK_SEGMENT_LIST ("sfree ()");

	-- sp->s_lrefc;

	sp->s_back->s_forw = sp->s_forw;
	sp->s_forw->s_back = sp->s_back;

	c_free (sp->s_vmem, btocru (sp->s_size));

	__UNLOCK_SEGMENT_LIST ();

	if (sp->s_lrefc)
		panic ("Bad segment count");

	/*
	 * Check if inode is ilocked, in order to allow the process
	 * to exec itself (file with the same inode as parent). Vlad.
	 */

	if ((ip = sp->s_ip) != NULL && ! ilocked (ip))
		ldetach (ip);

	kfree (sp);
}


/*
 * Given a pointer to a newly created process, copy all of our segments
 * into the given process.
 *
 * Return nonzero if successful.
 */

int
segadup (cpp)
PROC *cpp;
{
	SEG * sp;
	int n;

	cpp->p_flags |= PFSWIO;

	for (n = 0 ; n < NUSEG ; n ++) {
		cpp->p_segl [n] = SELF->p_segl [n];

		if ((sp = SELF->p_segl [n].sr_segp) == NULL)
			continue;

		if ((sp = segdupl (sp)) == NULL)
			break;

		cpp->p_segl [n].sr_segp = sp;
		if ((sp->s_flags & SFCORE) == 0)
			cpp->p_flags &= ~ PFCORE;
	}

	/*
	 * One of the calls to segdupl() failed.
	 * Undo any that succeeded.
	 */

	if (n < NUSEG) {


		/*
		 * If segdupl () fails on first segment, sr_segp needs
		 * to be cleared or relproc () would cause parent to
		 * nuke itself.
		 */
		if (n == 0)
			cpp->p_segl [0].sr_segp = NULL;
			
		while (n > 0) {
			if ((sp = cpp->p_segl [-- n].sr_segp) != NULL) {
				cpp->p_segl [n].sr_segp = NULL;
				sfree (sp);
			}
		}
	}

	cpp->p_flags &= ~ PFSWIO;
	return n;
}


/*
 * Duplicate a segment.
 */

SEG *
segdupl(sp)
SEG *sp;
{
	SEG *sp1;

	if (sp->s_flags & SFSHRX) {
		sp->s_urefc ++;
		sp->s_lrefc ++;
		return sp;
	}
	if ((sp->s_flags & SFCORE) == 0)
		panic("Cannot duplicate non shared swapped segment");

	if ((sp1 = salloc (sp->s_size,
			   sp->s_flags | SFNSWP | SFNCLR)) == NULL)
		return NULL;

	sp1->s_flags = sp->s_flags;
	dmacopy (btocru (sp->s_size), sp->s_vmem, sp1->s_vmem);

	return sp1;
}


/*
 * Allocate a segment `bytes_wanted' bytes long.
 * `flags' contains some pseudo flags.
 */

SEG *
salloc (bytes_wanted, flags)
int bytes_wanted, flags;
{
	SEG *sp;

	if ((sp = smalloc (bytes_wanted)) == NULL)
		return NULL;

	sp->s_flags = (flags & (SFSYST | SFTEXT | SFSHRX | SFDOWN)) | SFCORE;

	if ((flags & SFNCLR) == 0)
		dmaclear (sp->s_size, MAPIO (sp->s_vmem, 0));

	return sp;
}


/*
 * Make the segment descriptor pointed to by `sp1' have the attributes
 * of `sp2' including it's position in the segment queue and release
 * `sp2'.  `seglink' must be locked when this routine is called.
 */

void
satcopy(sp1, sp2)
SEG *sp1;
SEG *sp2;
{
	sp1->s_back->s_forw = sp1->s_forw;
	sp1->s_forw->s_back = sp1->s_back;
	sp2->s_back->s_forw = sp1;
	sp1->s_back = sp2->s_back;
	sp2->s_forw->s_back = sp1;
	sp1->s_forw  = sp2->s_forw;
	sp1->s_daddr = sp2->s_daddr;
	sp1->s_size = sp2->s_size;
	sp1->s_vmem = sp2->s_vmem;
	kfree (sp2);
}


/*
 * Grow or shrink the segment `sp' so that it has size `new_bytes' bytes.
 * Return 1 on success, 0 on failure.
 * 
 * WARNING: Downward growing segments (like user stack) not done yet!
 */

int
seggrow(sp, new_bytes)
SEG *sp;
unsigned int new_bytes;
{
	SEG *sp1;
	int dowflag;
	unsigned int	old_bytes, common_pages;

	dowflag = sp->s_flags & SFDOWN;
	old_bytes = sp->s_size;

	/* Get rid of degenerate case. */
	if (new_bytes == old_bytes)
		return 1;

	/*
	 * If we want a larger segment AND c_grow() succeeds
	 *	boost segment size to new_bytes
	 */

	if (new_bytes >= old_bytes && c_grow (sp, new_bytes) == 0) {

		T_HAL(0x100, printf("c_grow(%d) ", new_bytes));

		sp->s_size = new_bytes;
		dmaclear (new_bytes - old_bytes,
			  MAPIO (sp->s_vmem, old_bytes));
		return 1;
	}

	if ((sp1 = salloc (new_bytes,
			   sp->s_flags | SFNSWP | SFNCLR)) != NULL) {

		T_HAL(0x100, printf("salloc(%d) ", new_bytes));
		if (dowflag == 0) {
			common_pages = btocru (min (new_bytes, old_bytes));
			dmacopy (common_pages, sp->s_vmem, sp1->s_vmem);
			if (new_bytes > old_bytes)
				dmaclear (new_bytes - old_bytes,
					  MAPIO (sp1->s_vmem, old_bytes));
		} else
			panic ("downflag");

		__LOCK_SEGMENT_LIST ("seggrow ()");

		c_free (sp->s_vmem, btocru (old_bytes));
		satcopy (sp, sp1);

		__UNLOCK_SEGMENT_LIST ();

		return 1;
	}

	return 0;
}


/*
 * Given a segment pointer, `sp' and a segment size, grow the given segment
 * to the given size.
 */

void
segsize(sp, s2)
SEG *sp;
caddr_t s2;
{
	caddr_t s1;

	s1 = (caddr_t) sp->s_size;
	if (s2 == 0 || seggrow (sp, (off_t) s2) == 0) {
		SET_U_ERROR (ENOMEM, "can not grow segment");
		return;
	}

	if (sproto (0) == 0) {
		if (seggrow (sp, (off_t) s1) == 0 || sproto (0) == 0) {

			T_PIGGY (0x2000000, printf("auto SEGV\n"));
			sendsig (SIGSEGV, SELF);
		}
	}
	segload ();
}


/*
 * Allocate a segment in memory that is `bytes_wanted' bytes long.
 * The `seglink' gate should be locked before this routine is called.
 *
 * if successful, return allocated SEG * else, return 0
 *
 * NIGEL: This routine is actually only called from salloc (), whose callers
 * expect a completely initialized structure (or so it seems). Let's do that
 * initialization rather than expecting kalloc () to have accidentally done
 * the job. Furthermore, this routine is specially set up to only work for the
 * _I386 version of the data structures.
 */

SEG *
smalloc (bytes_wanted)
off_t bytes_wanted;
{
	SEG *sp1;
	SEG *new_seg;
	unsigned	pages_wanted;

	pages_wanted = btocru (bytes_wanted);

	/*
	 * Estimate space needed for new segment and its overhead.
	 * Fail if not enough free RAM available.
	 */

	if (countsize (pages_wanted) > allocno ())
		return 0;

	/*
	 * Allocate a new SEG struct to keep track of the segment, if possible.
	 */

	if ((new_seg = kalloc (sizeof (SEG))) == NULL)
		return 0;

	if ((new_seg->s_vmem = c_alloc (pages_wanted)) == NULL) {
		kfree (new_seg);
		return 0;
	}

	/* link new_seg in at start of segmq */

	__LOCK_SEGMENT_LIST ("smalloc ()");

	sp1 = segmq.s_forw;
	sp1->s_back->s_forw = new_seg;
	new_seg->s_back = sp1->s_back;
	sp1->s_back = new_seg;
	new_seg->s_forw = sp1;

	new_seg->s_urefc = 1;
	new_seg->s_lrefc = 1;
	new_seg->s_size  = bytes_wanted;

	new_seg->s_ip = NULL;
	new_seg->s_daddr = 0;

	__UNLOCK_SEGMENT_LIST ();

	return new_seg;
}


/*
 * Set up `SR' structure in user area from segments descriptors in
 * process structure.  Also set up the user segmentation registers.
 */

int
sproto (xhp)
struct xechdr *xhp;
{
	int n;
	SEG *sp;

	for (n = 0 ; n < NUSEG ; n ++) {
		SELF->p_segl [n].sr_flag = SELF->p_segl [n].sr_size = 0;
		if ((sp = SELF->p_segl [n].sr_segp) == NULL)
			continue;

		if (n == SIUSERP)
			SELF->p_segl [n].sr_base = (caddr_t) & u;
		else {
			if (xhp)
				SELF->p_segl [n].sr_base = (caddr_t) xhp->segs [n].mbase;
			SELF->p_segl [n].sr_flag |= SRFPMAP;
		}
		if (n != SIUSERP && n != SISTEXT)
			SELF->p_segl [n].sr_flag |= (SRFDATA | SRFDUMP);
		SELF->p_segl [n].sr_size = sp->s_size;
	}
	return mproto ();
}


/*
 * Search for a busy text inode.
 */

int
sbusy(ip)
struct inode *ip;
{
	SEG *sp;

	__LOCK_SEGMENT_LIST ("sbusy ()");

	/*
	 * Look for the segment in the memory queue.
	 */

	for (sp = segmq.s_forw ; sp != & segmq ; sp = sp->s_forw) {
		if (sp->s_ip == ip &&
		    (sp->s_flags & (SFSHRX | SFTEXT)) == (SFSHRX | SFTEXT)) {
			__UNLOCK_SEGMENT_LIST ();
			return 1;
		}
	}

	__UNLOCK_SEGMENT_LIST ();
	return 0;
}
