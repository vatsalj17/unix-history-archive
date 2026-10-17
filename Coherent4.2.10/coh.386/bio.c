/*
 * $Header: /v/src/rcskrnl/coh.386/RCS/bio.c,v 420.9 1993/12/10 22:04:25 srcadm Exp srcadm $
 */

/*********************************************************************
 *  
 * Coherent, Mark Williams Company, Copyright 1993
 * RCS Header
 * This file contains proprietary information and is considered
 * a trade secret.  Unauthorized use is prohibited.
 *
 *
 * $Id: bio.c,v 420.9 1993/12/10 22:04:25 srcadm Exp srcadm $
 *
 * $Log: bio.c,v $
 * Revision 420.9  1993/12/10  22:04:25  srcadm
 * Fixed problems with character device read/writes (dmareq()).
 * Created functions io_buffer_req() and raw_buf_read() for this
 * purpose.
 * Also, stripped comments and replaced with function headers.
 *
 * Revision 420.8  1993/12/04  00:25:23  srcadm
 * Further mods to io_buffer_req for raw access.
 *
 * Revision 420.7  1993/12/02  18:36:08  srcadm
 * Fixes ps listing data limits so bufneed appears different
 * than buffer_wait, added comments, fixed foobar with u.u_error
 * setting introduced in 420.6 by Louis.
 *
 * Revision 420.6  1993/12/02  00:14:57  srcadm
 * Fixes problem with opening a non-existant block device.
 *
 * Revision 420.5  1993/12/01  23:40:18  srcadm
 * Initial RCS submission.
 *
 *
 */

#ifdef EMBEDDED_VERSION
/* Embedded Version Constant */
char *MWC_BIO_VERSION = "MWC_BIO_VERSION($Revision: 420.9 $)";
#endif

/*
 * Revision 2.8  93/10/29  00:54:55  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.7  93/09/13  07:53:28  nigel
 * Changed so that once again most driver return values are ignored; instead
 * we pass in an extra pointer to an integer where the return value should
 * be placed, along with a (currently NULL) "cred_t *" so that the signature
 * for driver entry points is becoming very much like the DDI/DKI version.
 * 
 * Revision 2.6  93/09/02  18:02:56  nigel
 * Remove spurious globals and prepare for initial DDI/DKI mods
 * 
 * Revision 2.5  93/08/19  10:37:01  nigel
 * r83 ioctl (), corefile, new headers
 * 
 * Revision 2.4  93/08/19  03:26:19  nigel
 * Nigel's r83 (Stylistic cleanup)
 * 
 * Revision 2.2  93/07/26  15:22:22  nigel
 * Nigel's R80
 */

#define	_DDI_DKI	1
#define	_SYSV3		1

#include <common/gregset.h>
#include <common/_canon.h>
#include <kernel/proc_lib.h>
#include <sys/param.h>
#include <sys/confinfo.h>
#include <sys/kmem.h>
#include <sys/cmn_err.h>
#include <sys/debug.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <stddef.h>
#include <limits.h>

#define	_KERNEL		1

#include <kernel/trace.h>
#include <kernel/reg.h>
#include <sys/uproc.h>
#include <sys/inode.h>
#include <sys/mmu.h>
#include <sys/buf.h>
#include <sys/con.h>
#include <sys/io.h>
#include <sys/proc.h>
#include <sys/sched.h>
#include <sys/seg.h>
#include <sys/dmac.h>
#include <sys/types.h>
#include <sys/coherent.h>


/*
 * This is here for the old-style Coherent I/O support hacks.
 */

#include <sgtty.h>

static	buf_t	     **	hasharray;	/* pointer to hash buckets */
static	buf_t	      *	firstbuf;	/* pointer to first in LRU chain */
static	buf_t	      *	lastbuf;	/* pointer to last in LRU chain */

static	int		bufneed;	/* waiting for buffers to become free */
static	volatile int	bufsync;	/* waiting on syncing buffers */
static	buf_t	      *	buf_list;	/* first buffer */
static	buf_t	      *	buf_list_end;	/* points after last buffer */
buf_t *dyn_buf_head = (buf_t *)NULL; /* dynamic buffer head */

MAKESR(blockp, _blockp);

int	bufinit_done;

#undef	__LOCAL__
#define	__LOCAL__

/*
 * The following hacks allow the magic address information used in a block to
 * be saved and restored. This is so that ioreq () and dmareq () can use a
 * single regular buffer for all their work rather than juggling several
 * buffers.
 */

struct addr_save {
	caddr_t		save_virt;
	paddr_t		save_phys;
};

#define	SAVE_ADDR(bp,as)	((as)->save_virt = (bp)->b_vaddr, \
				 (as)->save_phys = (bp)->b_paddr)
#define	REST_ADDR(bp,as)	((bp)->b_vaddr = (as)->save_virt, \
				 (bp)->b_paddr = (as)->save_phys, \
				 (bp)->b_proc = NULL)

/*
 * The following hashing algorithm is used by bclaim ().
 */

#define	HASH(device, blockno)	((device * 257) + blockno)


/*
 * The following space is used as workspace by the radix-sorting algorithm.
 */

typedef struct lock_buf_list {
	buf_t 			*buf;
	struct lock_buf_list 	*next;
} buf_list_t;

__DUMB_GATE	radix_gate = __GATE_DECLARE ("radix");
BUF	      *	radix_space [UCHAR_MAX + 1];
buf_list_t    *u_radix_space[UCHAR_MAX + 1];


/********************************************************************
 * Function: static void HASHinsert(buf_t *bp)
 *
 * Description: Insert the current buffer at the head of the
 *		appropriate hash chain.
 *
 * Returns: Nothing.
 *
 * Affects Globals: hasharray[]
 *
 * Affects U-Area: None.
 *
 * Side effects: Changes b_hash field of passed bp.
 *
 * Comments:
 */

static void
HASHinsert (bp)
buf_t * bp;
{
	ASSERT (bp->b_hash == NULL);

	bp->b_hash = hasharray [bp->b_hashval];
	hasharray [bp->b_hashval] = bp;
}


/*
 * Allocate and initialize buffer headers.
 */

void
bufinit ()
{
	paddr_t p;
	caddr_t v;
	int	i;

	p = MAPIO (blockp.sr_segp->s_vmem, 0);
	v = blockp.sr_base;

	if (NBUF < 32)
		cmn_err (CE_PANIC, "NBUF = %d, must be at least 32", NBUF);
	if (NHASH < 32)
		cmn_err (CE_PANIC, "NHASH = %d, must be at least 32", NHASH);

	buf_list = kmem_alloc (NBUF * sizeof (buf_t), KM_NOSLEEP);
	buf_list_end = buf_list + NBUF;
	hasharray = kmem_alloc (NHASH * sizeof (buf_t *), KM_NOSLEEP);
	if (buf_list == NULL || hasharray == NULL)
		cmn_err (CE_PANIC,
			 "bufinit: insufficient memory for %d buffers", NBUF);

	for (i = 0; i < NHASH; ++ i)
		hasharray [i] = NULL;

	/*
	 * initialize the buffer header array with the physical and
	 * virtual addresses of the buffers, NULL values for the
	 * hash chain pointers, and pointers to the successor and
	 * predecessor of the current node.
	 */

	firstbuf = buf_list;
	lastbuf = buf_list_end - 1;

	for (i = 0 ; i < NBUF ; i ++) {
		buf_t	      *	bp = buf_list + i;

		bp->b_hashval = i % NHASH;
		bp->b_dev = NODEV;
		bp->b_edev = NODEV;
		bp->b_flags = 0;
		bp->b_paddr = p;
		bp->b_vaddr = v;
		bp->b_hash = NULL;
		bp->b_sort = NULL;
		bp->b_LRUf = bp + 1;		/* next entry in chain */
		bp->b_LRUb = bp - 1;		/* prev entry in chain */
		bp->b_bufsiz = BSIZE;

		bp->b_proc = NULL;
		bp->b_iodone = NULL;
		bp->b_private = NULL;

		__INIT_BUFFER_LOCK (bp);

		p += BSIZE;
		v += BSIZE;

		HASHinsert (bp);
	}

	/*
	 * the first and last headers are special cases.
	 */

	buf_list->b_LRUb = NULL;		/* no predecessor */
	(buf_list_end - 1)->b_LRUf = NULL;	/* no successor */

	bufinit_done = 1;
}


#if	0
/*
 * Set this up to be called once a second from clock.c to track down buffer
 * locking problems.
 */

#if	__USE_PROTO__
void check_buffer_cache (void)
#else
void
check_buffer_cache ()
#endif
{
	int		i;

	for (i = 0 ; i < NBUF ; i ++) {
		buf_t	      *	bufp = buf_list + i;
		short		s = sphi ();

		switch (bufp->__b_gate->_lock [0]) {
		case 0:
			break;

		case 16:
			cmn_err (CE_NOTE, "stuck buffer #%x #%x blk %d flags #%x",
				 bufp->b_dev, bufp->b_edev, bufp->b_blkno,
				 bufp->b_flags);
			bufp->__b_gate->_lock [0] = 1;
			break;

		default:
			bufp->__b_gate->_lock [0] ++;
			break;
		}
		spl (s);
	}
}
#endif


/*
 * Set up for radix-sort.
 */

#if	__USE_PROTO__
__LOCAL__ void init_radix_sort (void)
#else
__LOCAL__ void
init_radix_sort ()
#endif
{
	__GATE_LOCK (radix_gate, "buffer sort");
}


/*
 * End radix-sorting.
 */

#if	__USE_PROTO__
__LOCAL__ void end_radix_sort (void)
#else
__LOCAL__ void
end_radix_sort ()
#endif
{
	ASSERT (__GATE_LOCKED (radix_gate));
	__GATE_UNLOCK (radix_gate);
}


/*
 * Perform a radix-sort pass on the buffer list rooted at "bp", with the
 * sort digit being indexed at structure offset "digofs", with the list thread
 * being maintained at index "linkofs".
 */


#define	__RADIX_LINK(bp, ofs)	(* (buf_t **) ((char *) bp + linkofs))
#define	__U_RADIX_LINK(bp, ofs)	(* (buf_list_t **) ((char *) bp + linkofs))


#if	__USE_PROTO__
__LOCAL__ buf_list_t * unlocked_radix_pass (buf_list_t * bp, size_t digofs, size_t linkofs)
#else
__LOCAL__ buf_list_t *
unlocked_radix_pass (bp, digofs, linkofs)
buf_list_t      *bp;
size_t		digofs;
size_t		linkofs;
#endif
{
	int		i;
	buf_list_t      *prev;

	ASSERT (__GATE_LOCKED (radix_gate));

	for (i = 0 ; i < UCHAR_MAX + 1 ; i ++)
		u_radix_space [i] = NULL;

	/*
	 * Walk over the input list, putting items into buckets.
	 */

	while (bp != NULL) {
		buf_list_t	      *	next;

		i = * ((unsigned char *) bp->buf + digofs);
		next = __U_RADIX_LINK (bp, linkofs);
		__U_RADIX_LINK (bp, linkofs) = u_radix_space [i];
		u_radix_space [i] = bp;
		bp = next;
	}

	/*
	 * Now construct the output list by walking over each bucket and
	 * reversing pointers.
	 */

	prev = NULL;
	for (i = UCHAR_MAX + 1 ; i -- > 0 ;) {
		bp = u_radix_space [i];

		while (bp != NULL) {
			buf_list_t      *next = __U_RADIX_LINK (bp, linkofs);
			__U_RADIX_LINK (bp, linkofs) = prev;
			prev = bp;
			bp = next;
		}
	}

	/*
	 * Now we have a partial sort on a buffer list, return the head of the
	 * list.
	 */

	return prev;
}



#if	__USE_PROTO__
__LOCAL__ buf_t * radix_pass (buf_t * bp, size_t digofs, size_t linkofs)
#else
__LOCAL__ buf_t *
radix_pass (bp, digofs, linkofs)
buf_t	      *	bp;
size_t		digofs;
size_t		linkofs;
#endif
{
	int		i;
	buf_t	      *	prev;

	ASSERT (__GATE_LOCKED (radix_gate));

	for (i = 0 ; i < UCHAR_MAX + 1 ; i ++)
		radix_space [i] = NULL;

	/*
	 * Walk over the input list, putting items into buckets.
	 */

	while (bp != NULL) {
		buf_t	      *	next;

		i = * ((unsigned char *) bp + digofs);
		next = __RADIX_LINK (bp, linkofs);
		__RADIX_LINK (bp, linkofs) = radix_space [i];
		radix_space [i] = bp;
		bp = next;
	}

	/*
	 * Now construct the output list by walking over each bucket and
	 * reversing pointers.
	 */

	prev = NULL;
	for (i = UCHAR_MAX + 1 ; i -- > 0 ;) {
		bp = radix_space [i];

		while (bp != NULL) {
			buf_t	      *	next = __RADIX_LINK (bp, linkofs);
			__RADIX_LINK (bp, linkofs) = prev;
			prev = bp;
			bp = next;
		}
	}

	/*
	 * Now we have a partial sort on a buffer list, return the head of the
	 * list.
	 */

	return prev;
}



/********************************************************************
 * Function: CON *drvmap(o_dev_t dev, int flags)
 *
 * Description: Maps a device to its CON entry.
 *
 * Returns: Pointer to the CON entry, or NULL on failure.
 *
 * Affects Globals: None.
 *
 * Affects U-Area: Sets u.u_error to ENXIO if device does not exist.
 *
 * Side Effects: None.
 *
 * Comments:
 *
 * NIGEL: This function is the only code that references drvl [] directly
 * other than the bogus code that manages the load and unload entry points,
 * which we will also need to "enhance". What we add to this code is a range
 * check so that it no longer can index off the end of drvl [], and in the
 * case that we would go off the end of drvl [] we vector instead to the
 * STREAMS system and ask it to return a kludged-up "CON *". The mapping
 * code referred to above is for the i286 and does nothing whatsoever, so
 * all this function really does as it stands is a table lookup.
 */

#if	__USE_PROTO__
__LOCAL__ CON * drvmap (o_dev_t dev, int flags)
#else
__LOCAL__ CON *
drvmap (dev, flags)
o_dev_t 	dev;
int		flags;
#endif
{
	DRV * dp;
	unsigned m;

	if ((m = major (dev)) >= drvn) {
		if (m < _maxmajor && _major [m] != NODEV) {
			return (flags & DFBLK) == 0 ?
				& cdevsw [_major [m]].cdev_con :
				& bdevsw [_major [m]].bdev_con;
		}
	} else if ((dp = drvl + m)->d_conp != NULL)
		return dp->d_conp;

	SET_U_ERROR (ENXIO, "drvmap ()");
	return NULL;
}


/*
 * NIGEL: To avoid accidents with block locking, use this function rather than
 * the primitive old-Coherent sleep locks when locking a block that is
 * supposed to have a particular identity. With this buffer cache
 * implementation, we *must* check that a block ID has not changed *every*
 * time we re-lock it. Using this code ensures that.
 */

#if	__USE_PROTO__
__LOCAL__ buf_t * lock_block (buf_t * bufp, o_dev_t dev, daddr_t bno)
#else
__LOCAL__ buf_t *
lock_block (bufp, dev, bno)
buf_t	      *	bufp;
o_dev_t		dev;
daddr_t		bno;
#endif
{
	__LOCK_BUFFER (bufp, "lock_block ()");

	if (bufp->b_dev != dev || bufp->b_bno != bno) {
		__UNLOCK_BUFFER (bufp);
		bufp = NULL;
	}

	return bufp;
}


/*
 * Unlock a block buffer, and if anyone is waiting for a block on the free
 * list to become available, wake them up. This function doesn't wake anyone
 * who is waiting for I/O completion on the buffer.
 */

#if	__USE_PROTO__
void unlock_block (buf_t * bp)
#else
void
unlock_block (bp)
buf_t	      *	bp;
#endif
{
	__UNLOCK_BUFFER (bp);

	if (bufneed) {
		bufneed = 0;
		wakeup (& bufneed);
	}
}


/*
 * Begin writing a block out; if 'sync' is not set, the buffer is marked as
 * asynchronous and the lock that is on the block is detached from the current
 * process.
 */

#if	__USE_PROTO__
__LOCAL__ void buffer_write_start (buf_t * bp, int sync)
#else
__LOCAL__ void
buffer_write_start (bp, sync)
buf_t	      *	bp;
int		sync;
#endif
{
	ASSERT (__IS_BUFFER_LOCKED (bp));
#if	TRACER
	ASSERT (__GATE_LOCK_OWNER (bp->__b_gate) == (char *) SELF);
#endif

	if (sync == BUF_SYNC)
		bp->b_flag &= ~ BFASY;
	else {
		/*
		 * From here on, the gate does not belong to the calling
		 * process.
		 */

		__MAKE_BUFFER_ASYNC (bp);
	}

	bp->b_flag |= BFNTP;
	bp->b_req = BWRITE;
	bp->b_count = BSIZE;

	dblock (bp->b_dev, bp);
}


/*
 * Wait for a buffer to complete any pending I/O.
 */

#if	__USE_PROTO__
__LOCAL__ void buffer_wait (buf_t * bp)
#else
__LOCAL__ void
buffer_wait (bp)
buf_t	      *	bp;
#endif
{
	unsigned short	s;

	ASSERT (__IS_BUFFER_LOCKED (bp));
#if	TRACER
	ASSERT (__GATE_LOCK_OWNER (bp->__b_gate) == (char *) SELF);
#endif

	/*
	 * LOUIS:
	 * buffer_wait shortened to bfwait because our ps output
	 * doesn't have a long enough field.
	 */
	s = sphi ();
	while ((bp->b_flag & BFNTP) != 0)
		x_sleep (bp, pridisk, slpriNoSig, "bfwait");
	spl (s);
}


/*
 * Attempt to locate a block within the buffer cache.
 */

#if	__USE_PROTO__
__LOCAL__ buf_t * buffer_find (o_dev_t dev, daddr_t bno, int sync)
#else
__LOCAL__ buf_t *
buffer_find (dev, bno, sync)
o_dev_t		dev;
daddr_t		bno;
int		sync;
#endif
{
	buf_t	      *	bp;
	unsigned long	hashval = HASH (dev, bno) % NHASH;

rescan:
	for (bp = hasharray [hashval] ; bp != NULL ; bp = bp->b_hash) {
		if (bp->b_bno != bno || bp->b_dev != dev)
			continue;

		if (sync == BUF_ASYNC ||
		    (bp = lock_block (bp, dev, bno)) != NULL)
			return bp;

		/*
		 * This really can happen; the reason is that
		 * when a *read* has an I/O error, the "b_dev"
		 * field is set to NODEV.
		 */

		goto rescan;
	}

	return NULL;
}


/*
 * Mark the indicated buffer as clean.
 */

#if	__USE_PROTO__
void bclean (o_dev_t dev, daddr_t bno)
#else
void
bclean (dev, bno)
o_dev_t		dev;
daddr_t		bno;
#endif
{
	buf_t	      *	bp;

	if ((bp = buffer_find (dev, bno, BUF_SYNC)) != NULL) {
		bp->b_flag &= ~ BFMOD;
		__UNLOCK_BUFFER (bp);
	}
}


/*
 * Generic iodone function for synced blocks. Specialized iodone functions
 * should chain on to this one.
 */

#if	__USE_PROTO__
__LOCAL__ void sync_iodone (buf_t * bp)
#else
__LOCAL__ void
sync_iodone (bp)
buf_t	      *	bp;
#endif
{
	int s;
	ASSERT (bufsync != 0);

	/*
	 * Unlock the block and maintain the count of blocks which have been
	 * started syncing but are not yet finished. Processor level
	 * changing added by Louis who suspects a race condition.
	 */

	unlock_block (bp);

	s = sphi();

	if (-- bufsync == 0) {
		spl(s);
		wakeup (& bufsync);
	}
	else
		spl(s);
}



#if __USE_PROTO__
void
build_sync_list(buf_list_t **bheadpp, int (*sel) __PROTO((int arg, buf_t *bp)),
		int arg, void (*fin) __PROTO((buf_t *bp)))
							  
#else
void
build_sync_list(bheadpp, sel, arg, fin)
buf_list_t **bheadpp;
int (*sel)();
int arg;
void (*fin)();
#endif /* __USE_PROTO__ */
{
  buf_list_t *bhead = NULL;
  int i;
  buf_list_t *tmp_el = NULL;
  buf_t *cur_bp;

  /* First selection pass on static buffers */
  for (i = 0; i < NBUF; i++) {
    buf_t *cur_bp;

    cur_bp = buf_list + i;

    /*
     * Instead of locking buffer while placing them on the list,
     * we will just build a linked list of them.  We only lock
     * them when necessary.
     */

    __LOCK_BUFFER(cur_bp, "bldsync");
    
    /* Check selection function */
    if (sel != NULL && (*sel)(arg, cur_bp) == 0) {
      __UNLOCK_BUFFER(cur_bp);
      continue;
    }

    /* Make sure buffer is present. */
    if (cur_bp->b_flags & BFNTP) {
      __UNLOCK_BUFFER(cur_bp);
      continue;
    }

    if ((cur_bp->b_flags & BFMOD) == 0) {
      /*
       * NIGEL:
       * We check for this only inside the area where
       * we have the block locked because it may be
       * that we are about to change a buffer that is
       * queued for a read, and we needed to wait
       * for that to complete.
       * A read, when we are closing? Yes, because
       * of the action of readahead!
       *
       * Further to the above, it used to be the case
       * that broken utilities like 'df' worked by
       * accessing the block devices directly, and
       * the buffer cache was flushed when the block
       * device was closed, despite it being
       * in use by the mounted filesystem.
       *
       * LOUIS:
       * In addition, by doing it here before the sort,
       * we save clock cycles sorting and we don't have to
       * allocate a container for it.
       */

      if (fin != NULL)
	(*fin)(cur_bp);

      __UNLOCK_BUFFER(cur_bp);
      continue;
    }

    /*
     * Now, instead of placing into the sort list, we instead
     * shove it into a container and link the container.
     */
    __UNLOCK_BUFFER(cur_bp);

    tmp_el = (buf_list_t *)kmem_alloc((size_t)sizeof(buf_list_t),
				      KM_NOSLEEP);

    if (tmp_el == NULL) {
      cmn_err(CE_WARN,
	      "build_sync_list: kernel heap low; sync may not be complete");
      continue;
    }

    tmp_el->buf = cur_bp;
    tmp_el->next = bhead;
    bhead = tmp_el;
  }

  /* Now, do the same for the dynamically allocated buffers */
  for (cur_bp = dyn_buf_head; cur_bp != NULL; cur_bp = cur_bp->b_dynf) {
    /*
     * Instead of locking buffer while placing them on the list,
     * we will just build a linked list of them.  We only lock
     * them when necessary.
     */

    /* Non-syncable buffer? */
    if (cur_bp->b_flags & BFNSY)
      continue;

    __LOCK_BUFFER(cur_bp, "bldsync");
    
    /* Check selection function */
    if (sel != NULL && (*sel)(arg, cur_bp) == 0) {
      __UNLOCK_BUFFER(cur_bp);
      continue;
    }

    /* Make sure buffer is present. */
    if (cur_bp->b_flags & BFNTP) {
      __UNLOCK_BUFFER(cur_bp);
      continue;
    }

    if ((cur_bp->b_flags & BFMOD) == 0) {
      /*
       * NIGEL:
       * We check for this only inside the area where
       * we have the block locked because it may be
       * that we are about to change a buffer that is
       * queued for a read, and we needed to wait
       * for that to complete.
       * A read, when we are closing? Yes, because
       * of the action of readahead!
       *
       * Further to the above, it used to be the case
       * that broken utilities like 'df' worked by
       * accessing the block devices directly, and
       * the buffer cache was flushed when the block
       * device was closed, despite it being
       * in use by the mounted filesystem.
       *
       * LOUIS:
       * In addition, by doing it here before the sort,
       * we save clock cycles sorting and we don't have to
       * allocate a container for it.
       */

      if (fin != NULL)
	(*fin)(cur_bp);

      __UNLOCK_BUFFER(cur_bp);
      continue;
    }

    /*
     * Now, instead of placing into the sort list, we instead
     * shove it into a container and link the container.
     */
    __UNLOCK_BUFFER(cur_bp);

    tmp_el = (buf_list_t *)kmem_alloc((size_t)sizeof(buf_list_t),
				      KM_NOSLEEP);

    if (tmp_el == NULL) {
      cmn_err(CE_WARN,
	      "build_sync_list: kernel heap low; sync may not be complete");
      continue;
    }

    tmp_el->buf = cur_bp;
    tmp_el->next = bhead;
    bhead = tmp_el;
  }
    
  *bheadpp = bhead;
}

  
/********************************************************************
 * Function: void general_sync(int (*sel)(int arg, buf_t *bp),
 *			int arg, void (*fin)(buf_t *bp),
 *			void (*iodone)(buf_t *bp), int sync)
 *
 * Description: General sync function, pass in selection and
 *		finalization functions.
 *
 * Returns: Nothing.
 *
 * Affects Globals: No.
 *
 * Affects U-Area: Unknown.
 *
 * Side Effects: Locks large regions of buffers.
 *
 * Comments:	(LOUIS) There is always the possibility of deadlock
 *		if you sleep holding multiple resources, especially
 *		if the resources are of the same type. In general,
 *		locking n >> 0 buffers is not a good idea even if
 *		you wish to sort them.  The problem is that you
 *		will be putting a lot of I/O-bound processes to
 *		sleep for a long length of time while the writes
 *		are finished.  Sorting is a good idea, however, so
 *		what we *can* do is build a list of unlocked buffers,
 *		then lock and test each one seperately.  This is
 *		experimental because it will eat large portions of
 *		the kalloc arena while the list exists.
 */
#if	__USE_PROTO__
__LOCAL__ void general_sync (int (* sel) __PROTO ((int arg, buf_t * bp)),
			     int arg,  void (* fin) __PROTO ((buf_t * bp)),
			     void (* iodone) __PROTO ((buf_t * bp)),
			     int sync)
#else
__LOCAL__ void
general_sync (sel, arg, fin, iodone, sync)
int	     (*	sel) ();
int		arg;
void	     (*	fin) ();
void	     (*	iodone) ();
int		sync;
#endif
{
  buf_t	      	*bp;
  buf_list_t	*head = NULL;
  buf_list_t	*tmp_el = NULL;
  int		i;
  short		s;

  /* Build a list of buffers we should sync */
  build_sync_list(&head, sel, arg, fin);
  
  /*
   * NIGEL:
   * Sort passes, one for each digit in a block address, from least
   * to most significant.
   *
   * LOUIS:
   * This sort routine has been modified so that it will work
   * with unlocked buffers.  This is no big deal, really.  In
   * the worst situation, the sort gets screwed up.  So what?
   * The arm will just step over 1024 cylinders for nothing.
   * However, that will slow down I/O time without slowing
   * down CPU turnaround time, and, IMHO, sorting should be
   * done by the drivers and not the kernel.
   */
  
  init_radix_sort();
  
  for(i = 0; i < sizeof(daddr_t); i++) {
    head = unlocked_radix_pass(head, offsetof(buf_t, b_bno) +
			       __OCTET_N_OF_R(__NATIVE_32_MAP, i),
			       offsetof(buf_list_t, next));
  }
  
  end_radix_sort();
  
  tmp_el = head;
  
  while(tmp_el != NULL) {
    buf_list_t *next_el;
    
    /*
     * This is the final lock for writing...
     * before was just some grabass; we mean
     * business now.
     */

    __LOCK_BUFFER((bp = tmp_el->buf), "sync");
    
    next_el = tmp_el->next;

    kmem_free(tmp_el, (size_t)sizeof(buf_list_t)); 

    
    /*
     * Recheck everything.  A couple of extra clock
     * cycles won't kill us.  In fact, this is
     * needed if we are accepting multiple process
     * threads in here.
     */
    
    if (bp->b_flags & BFNTP || bp->b_flags & BFERR) {
      __UNLOCK_BUFFER(bp);
      tmp_el = next_el;
      continue;
    }
    
    if (sel != NULL && (*sel)(arg, bp) == 0) {
      __UNLOCK_BUFFER(bp);
      tmp_el = next_el;
      continue;
    }
    
    /*
     * This is in case we missed one we can cheat on.
     */	
    if (!(bp->b_flags & BFMOD)) {
      if (fin)
	(*fin)(bp);
      
      __UNLOCK_BUFFER(bp);
      tmp_el = next_el;
      continue;
    }
    
    /*
     * OK, now that we burned up lots of CPU time, heh,
     * we can actually try to write this guy out.
     *
     * NOTE: This was taken from the original code,
     * but it seems strange that we can just go and
     * adjust b_iodone....hmm.  Also, sleeping on
     * the same variable that multiple contexts can
     * mess with isn't a good idea (bufsync).
     *
     * For one thing, a second process entering this
     * arena can cause a first one to sleep unnecessarily
     * long.
     *
     * ADDENDUM:  This may be more of a problem than
     * first noted.  I believe that there may be a race
     * on bufsync.  As such, I am declaring it now
     * as volatile and raising the processor level around
     * it. (LOUIS)
     *
     */
    if (sync != BUF_ASYNC) {
      int s;
      
      if (iodone != NULL)
	bp->b_iodone = iodone;
      else
	bp->b_iodone = sync_iodone;
      s = sphi();
      bufsync++;
      spl(s);
    }
    
    buffer_write_start(bp, sync);
    tmp_el = next_el;
  }
  
  if (sync == BUF_ASYNC)
    return;
  
  s = sphi();
  while(bufsync)
    x_sleep(&bufsync, pridisk, slpriNoSig, "syncwait");
  spl(s);
}

/*
 * Synchronise the buffer cache.
 */

#if	__USE_PROTO__
void bsync (void)
#else
void
bsync ()
#endif
{
	general_sync (NULL, 0, NULL, NULL, BUF_SYNC);
}


/*
 * Selection predicate for buffer-flush code.
 */

#if	__USE_PROTO__
__LOCAL__ int bflush_sel (int arg, buf_t * bp)
#else
__LOCAL__ int
bflush_sel (arg, bp)
int		arg;
buf_t	      *	bp;
#endif
{
	return bp->b_dev == arg;
}


/*
 * Finalization function for buffer-flush code.
 */

#if	__USE_PROTO__
__LOCAL__ void bflush_finish (buf_t * bp)
#else
__LOCAL__ void
bflush_finish (bp)
buf_t	      *	bp;
#endif
{
	bp->b_dev = NODEV;
	bp->b_flags = BFNTP;
}

/*
 * Invalidate a buffer.
 */
#if	__USE_PROTO__
void buf_invalidate (buf_t * bp)
#else
void
buf_invalidate (bp)
buf_t	      *	bp;
#endif
{
	bp->b_dev = NODEV;
	bp->b_flags = BFNTP;
}


/*
 * iodone function for buffer-flush code.
 */

#if	__USE_PROTO__
__LOCAL__ void bflush_iodone (buf_t * bp)
#else
__LOCAL__ void
bflush_iodone (bp)
buf_t	      *	bp;
#endif
{
	bflush_finish (bp);
	sync_iodone (bp);
}


/*
 * Synchronise all blocks for a particular device in the buffer cache
 * and invalidate all references.
 */

#if	__USE_PROTO__
void bflush (o_dev_t dev)
#else
void
bflush (dev)
o_dev_t dev;
#endif
{
	general_sync (bflush_sel, dev, bflush_finish, bflush_iodone, BUF_SYNC);
}


/*
 * Release the given buffer.
 */

#if	__USE_PROTO__
void brelease (buf_t * bp)
#else
void
brelease (bp)
buf_t * bp;
#endif
{
	if ((bp->b_flag & BFERR) != 0) {
		bp->b_flag &= ~ BFERR;
		bp->b_errno = 0;
		bp->b_dev = NODEV;
	}
	bp->b_flag &= ~ BFNTP;
	wakeup (bp);

	unlock_block (bp);
}


/********************************************************************
 * Function: buf_t *bread(o_dev_t dev, daddr_t bno, int sync)
 *
 * Description: if sync != 0, return a *locked* buffer filled with
 *		a block from device dev and "sector" bno.
 *		Sync == 0 if asychronous, and NULL is returned.
 *
 * Returns: Pointer to buffer, or NULL if fatal error.
 *
 * Affects Globals: None.
 *
 * Affects U-Area: Sets u.u_error on error condition.
 *
 * Side Effects: Adjusts fields of the buffer returned.
 *
 * Comments:	There exists the fact that this function returns a
 *		NULL if b_resid == b_count, but this is not
 *		necessarily an error condition and needs to be
 *		examined in the near future to determine the
 *		correctness of this return value (LOUIS).
 */

#if	__USE_PROTO__
buf_t * bread (o_dev_t dev, daddr_t bno, int sync)
#else
buf_t *
bread (dev, bno, sync)
o_dev_t dev;
daddr_t bno;
int sync;
#endif
{
	buf_t	      *	bp;
	int		err;

	if (((bp = bclaim (dev, bno, NBPSCTR, sync))->b_flag & BFNTP) == 0)
		return bp;

	if (sync == BUF_SYNC)
		ASSERT ((bp->b_flag & BFASY) == 0);
	else {
		/*
		 * If the BFASY flag is set, then we don't need to
		 * actually initiate a new operation. Whatever is
		 * happening to the buffer now is fine by us...
		 */

		if ((bp->b_flag & BFASY) != 0)
			return NULL;

		/*
		 * Since we are actually going to perform some I/O
		 * on the buffer, we need to lock it first (it used
		 * to be that bclaim () would always do this, but that
		 * prevented useful parallelism).
		 */

		if ((bp = lock_block (bp, dev, bno)) == NULL) {
			/*
			 * Looping at this point would be a Bad Idea;
			 * if the last read of the block had an error,
			 * we will wind up here. Instead, we claim
			 * the buffer synchronously.
			 */

			if (((bp = bclaim (dev, bno, NBPSCTR, 1))->b_flag & BFNTP) == 0)
				return bp;
		}

		__MAKE_BUFFER_ASYNC (bp);
	}

	bp->b_req = BREAD;
	bp->b_flags |= B_READ;
	bp->b_count = BSIZE;
	dblock (dev, bp);
	if (sync == BUF_ASYNC)
		return NULL;

	buffer_wait (bp);

	if ((err = geterror (bp)) != 0) {
		SET_U_ERROR (err, "bread ()");
		brelease (bp);
		return NULL;
	}

	/*
	 * This may need to be adjusted.  This isn't necessarily
	 * an error state.
	 */
	if (bp->b_resid == BSIZE) {
		brelease (bp);
		return NULL;
	}

	return bp;
}


/*
 * Write the given buffer out.  If `sync' is set, the write is synchronous,
 * otherwise asynchronous.  This routine must be called with the buffer
 * gate locked.
 */

#if	__USE_PROTO__
void bwrite (buf_t * bp, int sync)
#else
void
bwrite (bp, sync)
buf_t * bp;
int sync;
#endif
{
	buffer_write_start (bp, sync);
	if (sync == BUF_SYNC)
		buffer_wait (bp);
}


/*
 * Perform an LRU chain update by unlinking the specified buffer
 * from it present location in the LRU chain and inserting it
 * at the head of the chain, as pointed to by "firstbuf".  Handle
 * updating "lastbuf" if current buffer is the last buffer on the chain.
 */

#if	__USE_PROTO__
__LOCAL__ void LRUupdate (buf_t * bp)
#else
__LOCAL__ void
LRUupdate (bp)
buf_t * bp;
#endif
{
	if (bp != firstbuf) {
		if (bp == lastbuf)
			lastbuf = bp->b_LRUb;
		if (bp->b_LRUb != NULL)
			bp->b_LRUb->b_LRUf = bp->b_LRUf;
		if (bp->b_LRUf != NULL)
			bp->b_LRUf->b_LRUb = bp->b_LRUb;
		bp->b_LRUb = NULL;
		bp->b_LRUf = firstbuf;
		firstbuf->b_LRUb = bp;
		firstbuf = bp;
	}
}


/*
 * If the requested buffer header is in the hash chain, delete it.
 */

#if	__USE_PROTO__
__LOCAL__ void HASHdelete (buf_t * bp)
#else
__LOCAL__ void
HASHdelete (bp)
buf_t * bp;
#endif
{
	buf_t	      *	prev;

	/*
	 * We expect the block hash chains to be sufficiently short (i.e.
	 * 1 or 2 entries only) that we can do a linear search for the
	 * previous entry.
	 */

	if ((prev = hasharray [bp->b_hashval]) == bp) {
		/*
		 * We're first in the chain
		 */

		hasharray [bp->b_hashval] = bp->b_hash;
	} else {
		while (prev->b_hash != bp) {
			if ((prev = prev->b_hash) == NULL) {
				cmn_err (CE_WARN, "Can't find buffer %x",
					 (unsigned) bp);
				bp->b_hash = NULL;
				return;
			}
		}
		prev->b_hash = bp->b_hash;
	}

	bp->b_hash = NULL;
}


#if __USE_PROTO__
void
LRUinsert(buf_t *bp)
#else
void
LRUinsert(bp)
buf_t *bp;
#endif /* __USE_PROTO__ */
{
	bp->b_LRUf = firstbuf;
	bp->b_LRUb = NULL;
	firstbuf->b_LRUb = bp;
	firstbuf = bp;
}

#if __USE_PROTO__
void 
LRUremove(buf_t *bp)
#else
void
LRUremove(bp)
buf_t *bp;
#endif /* __USE_PROTO__ */
{
	if (bp == firstbuf) {
		firstbuf->b_LRUb = NULL;
		firstbuf = bp->b_LRUf;
		return;
	}

	if (bp == lastbuf) {
		lastbuf->b_LRUf = NULL;
		lastbuf = bp->b_LRUb;
		return;
	}

	bp->b_LRUb->b_LRUf = bp->b_LRUf;
	bp->b_LRUf->b_LRUb = bp->b_LRUb;
}

#if __USE_PROTO__
buf_t *
getrbuf(long flag)
#else
buf_t *
getrbuf(flag)
long flag;
#endif /* __USE_PROTO__ */
{
	buf_t *bp;

	bp = kmem_alloc(sizeof(buf_t), flag);

	if (bp == NULL)
		return NULL;

	bioerror (bp, 0);
	bp->b_iodone = NULL;
	bp->b_flag = BFNTP | BFDYN | BFNSY;
	bp->b_req = BWRITE;
	bp->b_dev = NODEV;
	bp->b_edev = NODEV;
	bp->b_bno = 0;
	bp->b_dynf = dyn_buf_head;
	dyn_buf_head = bp; 
	bp->b_hashval = HASH(0, 0) % NHASH;
	bp->b_hash = NULL;
	bp->b_sort = NULL;
	bp->b_proc = SELF;
	bp->b_iodone = NULL;
	bp->b_private = NULL;

	__INIT_BUFFER_LOCK(bp);

	__LOCK_BUFFER(bp, "getrbuf");

	HASHinsert(bp);
	LRUinsert(bp);

	return bp;
}

#if __USE_PROTO__
void
freerbuf(buf_t *bp)
#else
void
freerbuf(bp)
buf_t *bp;
#endif /* __USE_PROTO__ */
{
  buf_t *bp_prev;
  buf_t *bp_cur;
  
  /* Note, must be called from a user context where the buffer
   * cannot be taken by an interrupt.  In other words, if anything
   * is sleeping on this buffer, it will sleep forever.  This needs
   * to be worked out sooner or later.
   */
  
  HASHdelete(bp);
  LRUremove(bp);
  
  bp_prev = bp_cur = dyn_buf_head;
  
  while (bp_cur != NULL) {
    if (bp_cur == bp)
      break;

    bp_prev = bp_cur;
    bp_cur = bp_cur->b_dynf;
  }

  if (bp_cur == NULL)
    cmn_err(CE_PANIC,
	    "freerbuf: lost dynamic buffer bno: %d dev: (0x%x, 0x%x)",
	    bp->b_bno, major(bp->b_dev), minor(bp->b_dev));

  if (bp_cur == bp_prev) /* found on head */
    dyn_buf_head = bp_cur->b_dynf;
  else
    bp_prev->b_dynf = bp_cur->b_dynf;

  kmem_free(bp, sizeof(buf_t));
}

/*
 * If the requested buffer is in the buffer cache, return a pointer to
 * it.  If not, pick an empty buffer, set it up and return it.
 */
#if	__USE_PROTO__
buf_t * bclaim (o_dev_t dev, daddr_t bno, long bsize, int sync)
#else
buf_t *
bclaim (dev, bno, bsize, sync)
o_dev_t		dev;
daddr_t		bno;
long		bsize;
int		sync;
#endif
{
	buf_t * bp;

	if ((bp = buffer_find (dev, bno, sync)) != NULL) {
		if (sync != BUF_ASYNC) {
			/*
			 * If the buffer had an I/O error, mark it as
			 * invalid.
			 */

			if (bp->b_flag & BFERR) {
				bp->b_flag |= BFNTP;
				bioerror (bp, 0);
			}
			bp->b_req = BWRITE;
			bp->b_flag &= ~ B_READ;
			bp->b_count = bp->b_bufsiz;
		}
		LRUupdate (bp);
		return bp;
	}


	/*
	 * The requested buffer is not resident in our cache.  Locate the
	 * oldest (least recently used) available buffer.  If it's dirty,
	 * queue up an asynchronous write for it and continue searching
	 * for the next old candidate. Once a candidate is found, move it
	 * to the front of the LRU chain, update the hash pointers, mark
	 * the buffer as invalid, unlock our buffer gate and return the
	 * buffer to the requestor.
	 */

	for (;;) {				/* loop until successful */
		unsigned short	s;

		for (bp = lastbuf ; bp != NULL ; bp = bp->b_LRUb) {
			/*
			 * NIGEL: This code assumes that buffers can be locked
			 * only by other process-level code.
			 */

			if (__IS_BUFFER_LOCKED (bp))
				continue;	/* not available */

			if (bp->b_flag & BFMOD) {
#if	1
				general_sync (NULL, 0, NULL, NULL, BUF_ASYNC);
#else
				__LOCK_BUFFER (bp, "bclaim ()");
				bwrite (bp, BUF_ASYNC);	/* flush dirty buffer */
#endif
				continue;
			}

			if (sync == BUF_SYNC)
				__LOCK_BUFFER (bp, "bclaim () #2");

			/*
			 * Update the hash chain for this old
			 * buffer.  Unlink it from it's old location
			 * fixing up any references. Also, update
			 * the LRU chain to move the buffer to the head.
			 */

			HASHdelete (bp);
			LRUupdate (bp);

			bioerror (bp, 0);
			bp->b_iodone = NULL;
			bp->b_flag = BFNTP;
			bp->b_req = BWRITE;
			bp->b_dev = dev;
			bp->b_edev = dev;
			bp->b_bno = bno;
			bp->b_hashval = HASH (dev, bno) % NHASH;
			bp->b_count = bp->b_bufsiz;
	
			HASHinsert (bp);
			return bp;
		}

		/*
		 * LOUIS:
		 * bufneed shortened to bfneed because our ps output
		 * doesn't have the length to display it.
		 */
		s = sphi ();
		bufneed = 1;
		x_sleep ((char *)& bufneed, pridisk, slpriNoSig, "bfneed");
		/* There are no buffers available.  */
		spl (s);
	}
}


/*
 * This is called by the driver when I/O has completed on a buffer.
 */

#if	__USE_PROTO__
void bdone (buf_t * bp)
#else
void
bdone (bp)
buf_t * bp;
#endif
{
	biodone (bp);
}


/********************************************************************
 * Function: int geterror(struct buf * bp) (DDI/DKI)
 *
 * Description:  Get the error number stored in a buffer header, bp.
 *
 * Returns: Error number in buffer header.
 *
 * Affects Globals: None.
 *
 * Affects U-Area: No.
 *
 * Side Effects: None.
 *
 * Comments:	If the buffer error flag is set but the errno is
 *		0, returns EIO anyhow.
 *
 */
#if	__USE_PROTO__
int geterror (buf_t * bp)
#else
int
geterror (bp)
buf_t	      *	bp;
#endif
{
	if (bp->b_errno != 0)
		return bp->b_errno;
	if (bp->b_flag & BFERR)
		return EIO;
	return 0;
}


/********************************************************************
 * Function: void biodone(buf_t *bp) (DDI/DKI)
 *
 * Description: Releases buffer and wakes up any waiting processes.
 *		Should be called by driver when I/O is done.
 *
 * Returns: Nothing.
 *
 * Affects Globals: None.
 *
 * Affects U-Area: No.
 *
 * Side Effects: Unlocks buffer, wakes processes, changes fields in bp.
 *
 * Comments:	If the driver specified an iodone handler, that function
 *		is called with bp and then we return.
 *	
 *		If not specified, unlock the buffer if ASYNC and return.
 *	
 *		Several important fields are modified, and this function
 *		should be reviewed before using within the kernel.
 */
#if	__USE_PROTO__
void biodone (buf_t * bp)
#else
void
biodone (bp)
buf_t	      *	bp;
#endif
{
	__iodone_t	iodone;

	switch (bp->b_req) {
	case BWRITE:
		bp->b_flag &= ~ BFMOD;
		break;

	case BREAD:
		break;

	default:
		/*
		 * The floppy-disk format request comes through here, too.
		 */
		break;
	}

	if ((bp->b_flag & BFERR) != 0)
		bp->b_dev = NODEV;

	bp->b_flag &= ~ BFNTP;
	wakeup (bp);

	if ((iodone = bp->b_iodone) != NULL) {
		bp->b_iodone = NULL;
		(* iodone) (bp);
		return;
	}

	if ((bp->b_flag & BFASY) != 0) {
		bp->b_flag &= ~ BFASY;

		unlock_block (bp);
	}
}


/********************************************************************
 * Function: void bioerror(buf_t *bp, int errno) (DDI/DKI)
 *
 * Description: Change error status of a buffer.
 *
 * Returns: Nothing.
 *
 * Affects Globals: No.
 *
 * Affects U-Area: No.
 *
 * Side Effects: Changes errnum and BFERR in buffer header.
 *
 * Comments:	Call with errno != 0 to set error, or errno == 0 to
 *		clear error.
 */
#if	__USE_PROTO__
void bioerror (buf_t * bp, int errno)
#else
void
bioerror (bp, errno)
buf_t	      *	bp;
int		errno;
#endif
{
	if ((bp->b_errno = errno) != 0)
		bp->b_flag |= BFERR;
	else
		bp->b_flag &= ~ BFERR;
}



/********************************************************************
 * Function: int biowait(buf_t *bp) (DDI/DKI)
 *
 * Description:  Wait (sleep) until I/O finishes.
 *
 * Returns: Error number generate during I/O.
 *
 * Affects Globals: None.
 *
 * Affects U-Area: No.
 *
 * Side Effects: None.
 *
 * Comments:	Buffer and other fields are affected, and the process
 *		may sleep as a result of a call to this function.
 *		See the functions this calls for more info.
 */
#if	__USE_PROTO__
int biowait (buf_t * bp)
#else
int
biowait (bp)
buf_t	      *	bp;
#endif
{
	buffer_wait (bp);
	return geterror (bp);
}


/********************************************************************
 * Function: void brelse(buf_t *bp) (DDI/DKI)
 *
 * Description: Return a buffer to the system.
 *
 * Returns: Nothing.
 *
 * Affects Globals: None.
 *
 * Affects U-Area: No.
 *
 * Side Effects: Wakes processes sleeping on this buffer.
 *
 * Comments:	WARNING!!! Cannot be used in place of brelease()
 *		This should only be used to release buffers obtained through
 *		geteblk() or ngeteblk()!
 */
#if	__USE_PROTO__
void brelse (buf_t * bp)
#else
void
brelse (bp)
buf_t	      *	bp;
#endif
{
	bp->b_flag &= ~ BFNTP;
	wakeup (bp);

	unlock_block (bp);
}


/********************************************************************
 * Function: void clrbuf(buf_t *bp) (DDI/DKI)
 *
 * Description: Clear out a buffer.
 *
 * Returns: Nothing.
 *
 * Affects Globals: None.
 *
 * Affects U-Area: No.
 *
 * Side Effects: bp->b_resid = 0 and zeros buffer data space.
 *
 * Comments:
 */
#if	__USE_PROTO__
void clrbuf (buf_t * bp)
#else
void
clrbuf (bp)
buf_t	      *	bp;
#endif
{
	memset (bp->b_un.b_addr, 0, bp->b_count);
	bp->b_resid = 0;
}


/********************************************************************
 * Function: buf_t *geteblk() (DDI/DKI)
 *
 * Description: Get a free buffer.
 *
 * Returns: Pointer to a locked buffer.
 *
 * Affects Globals: None.
 *
 * Affects U-Area: No.
 *
 * Side Effects: Changes fields in returned buffer.
 *
 * Comments:	Blocks allocated with this should be freed via
 *		brelse() or biodone().
 */
#if	__USE_PROTO__
buf_t * geteblk (void)
#else
buf_t *
geteblk ()
#endif
{
	buf_t	      *	bufp = bclaim (NODEV, (daddr_t) 0, NBPSCTR, BUF_SYNC);

	bufp->b_flags |= BFRAW;
	return bufp;
}


/*
 * Read data from the I/O segment into kernel space.
 *
 * "v" is the destination virtual address.
 * "n" is the number of bytes to read.
 */

#if	__USE_PROTO__
void ioread (IO * iop, char * v, size_t n)
#else
void
ioread (iop, v, n)
IO * iop;
char * v;
size_t n;
#endif
{
	switch (iop->io_seg) {
	case IOSYS:
		memcpy (v, iop->io.vbase, n);
		iop->io.vbase += n;
		break;

	case IOUSR:
		iop->io.vbase += ukcopy (iop->io.vbase, v, n);
		break;

	case IOPHY:
		dmain (n, iop->io.pbase, v);
		iop->io.pbase += n;
		break;
	}
	iop->io_ioc -= n;
}


/*
 * Clear I/O space.
 */

#if	__USE_PROTO__
void ioclear (IO * iop, size_t size)
#else
void
ioclear (iop, size)
IO	      *	iop;
size_t		size;
#endif
{
	switch (iop->io_seg) {
	case IOSYS:
		(void) memset (iop->io.vbase, 0, size);
		iop->io.vbase += size;
		break;

	case IOUSR:
		(void) umemclear (iop->io.vbase, size);
		iop->io.vbase += size;
		break;

	case IOPHY:
		dmaclear (size, iop->io.pbase);
		iop->io.pbase += size;
		break;
	}
	iop->io_ioc -= size;
}


/*
 * Write data from kernel space to the I/O segment.
 */

#if	__USE_PROTO__
void iowrite (IO * iop, char * v, size_t n)
#else
void
iowrite (iop, v, n)
IO * iop;
char * v;
unsigned n;
#endif
{
	switch (iop->io_seg) {
	case IOSYS:
		memcpy (iop->io.vbase, v, n);
		iop->io.vbase += n;
		break;

	case IOUSR:
		iop->io.vbase += kucopy (v, iop->io.vbase, n);
		break;

	case IOPHY:
		dmaout (n, iop->io.pbase, v);
		iop->io.pbase += n;
		break;
	}
	iop->io_ioc -= n;
}


/*
 * Get a character from the I/O segment.
 */

#if	__USE_PROTO__
int iogetc (IO * iop)
#else
int
iogetc (iop)
IO * iop;
#endif
{
	unsigned char	c;

	if (iop->io_ioc == 0)
		return -1;
	-- iop->io_ioc;
	if (iop->io_seg == IOSYS)
		c = * (unsigned char *) iop->io.vbase ++;
	else {
		c = getubd (iop->io.vbase ++);
		if (get_user_error ())
			return -1;
	}
	return c;
}

/*
 * Put a character using the I/O segment.
 */

#if	__USE_PROTO__
int ioputc (unsigned char c, IO * iop)
#else
int
ioputc (c, iop)
unsigned char	c;
IO	      *	iop;
#endif
{
	if (iop->io_ioc == 0)
		return -1;
	-- iop->io_ioc;
	if (iop->io_seg == IOSYS)
		* (char *) iop->io.vbase ++ = c;
	else {
		putubd (iop->io.vbase ++, c);
		if (get_user_error ())
			return -1;
	}

	/*
	 * Originally, this returned the character value, but sign-extension
	 * problems mean that we need to either return the character cast to
	 * unsigned char, or 0. Hal insists on 0, so that is what we do now.
	 */

	return 0;
}


/*
 * Given an I/O structure and a buffer header, see if the addresses
 * in the I/O structure are valid and set up the buffer header.
 *
 * Search the u area segment table for a data segment containing
 * iop->io.vbase.  If one is found, put the corresponding system
 * global address into bp->b_paddr and return the corresponding
 * SEG pointer, else return NULL.
 */

#if	__USE_PROTO__
__LOCAL__ SEG * iomapvp (IO * iop, buf_t * bp)
#else
__LOCAL__ SEG *
iomapvp (iop, bp)
IO * iop;
buf_t * bp;
#endif
{
	SR * srp;
	SEG * sp;
	caddr_t iobase, base;
	unsigned ioc;

	ASSERT (iop->io_seg == IOUSR);

	iobase = iop->io.vbase;
	ioc = iop->io_ioc;

	bp->b_vaddr = iobase;
	bp->b_proc = SELF;

	for (srp = SELF->p_segl; srp < & SELF->p_segl [NUSEG]; srp ++) {
		if ((sp = srp->sr_segp) == NULL)
			continue;
		if ((srp->sr_flag & SRFDATA) == 0)
			continue;
		/*
		 * The following calculation is because the system represents
		 * the 'base' of a stack as its upper limit (because it is the
		 * upper limit that is fixed).
		 */
		base = srp->sr_base;
		if (srp == & SELF->p_segl [SISTACK])
			base -= srp->sr_size;

		if (iobase < base)
			continue;
		if (iobase + ioc > base + sp->s_size)
			continue;
 		bp->b_paddr = MAPIO (sp->s_vmem, iobase - base);
		return sp;
	}

	/* Is the io area in question contained in a shared memory segment? */
	if ((srp = accShm (iobase, ioc)) != NULL) {
		sp = srp->sr_segp;
		base = srp->sr_base;
 		bp->b_paddr = MAPIO (sp->s_vmem, iobase - base);
		return sp;
	}

	return 0;
}

/********************************************************************
 * Function: buf_t *raw_buf_read(buf_t *bp, o_dev_t dev, daddr_t bno,
 *	long bsize)
 *
 * Description: Read a buffer from a device allowing for reading
 *		past the end of device as a non-error.
 *
 * Returns:  Pointer to buffer (a new buffer if bp was NULL).
 *
 * Affects Globals: None.
 *
 * Affects U-Area: Changes u.u_error on error.
 *
 * Side Effects: Changes fields in bp.
 *
 * Comments:	If bp is null, will also allocate a buffer.  Either way,
 *		this function sets up the buffer itself. Also note that
 *		by calling bclaim(), this may cause the calling context
 *		to sleep.  Read errors should be handled by the caller
 *		and checked for by examining bp->b_flags. This differs
 *		from bread() in that it always returns a buffer, always
 *		reads synchronously, and never handles errors.
 */
#if	__USE_PROTO__
buf_t *raw_buf_read(buf_t *bp, o_dev_t dev, daddr_t bno, long bsize)
#else
buf_t *
raw_buf_read(bp, dev, bno, bsize)
buf_t *bp;
o_dev_t dev;
daddr_t bno;
long bsize;
#endif
{
		/*
		 * If no block sent, allocate one.
		 */
		if(bp == NULL)
			if (((bp = bclaim (dev, bno, bsize, BUF_SYNC))->b_flag
			   & BFNTP) == 0)
				return bp;
			
		/*
		 * Set up the request.
		 */
		bp->b_req = BREAD;
		bp->b_flag |= (B_READ | BFNTP);
		bp->b_dev = dev;	
		bp->b_edev = NODEV;
		bp->b_blkno = bno;
		bp->b_bcount = bp->b_bufsiz;
		bp->b_resid = bp->b_bcount;

		/*
		 * Do the request and return the buffer.
		 */
		dblock(dev, bp);
		buffer_wait(bp);

		return bp;
}

/********************************************************************
 * Function: void io_buffer_req(buf_t *bp, IO *iop, o_dev_t dev,
 *		int req, int f)
 *
 * Description: Do raw I/O on behalf of a block device using
 *		the buffer cache as a side-buffer.  Good for
 *		DMA requests over 16Meg addresses and block straddles.
 *
 * Returns: Nothing.
 *
 * Affects Globals: None.
 *
 * Affects U-Area: u.u_error contains error status, places data
 *		   in user space if a read is being done.
 *
 * Side Effects: changes io_ioc, io_seek in iop
 *
 * Comments:	If used for DMA, this assumes that the buffer
 *	    	cache always lies below the 16M address limit.
 *		This code will also nicely handle block straddles.
 */
#if 0
#if	__USE_PROTO__
void io_buffer_req(buf_t *bp, IO *iop, o_dev_t dev, int req, int f)
#else
void
io_buffer_req(bp, iop, dev, req, f)
buf_t *bp;
IO *iop;
o_dev_t dev;
int req;
int f;
#endif
{
	CON		*cp;
	size_t		request_bytes;
	size_t		amount;
	unsigned int	left_edge;
	unsigned int	right_edge;
	unsigned	original_amount;
	caddr_t barea = 0;

	if((cp = drvmap(dev, DFBLK)) == NULL)
		/* u.u_error set by drvmap() */
		return;

	ASSERT(iop != NULL);

	/*
	 * Special consideration must be give to writing.
	 */
	original_amount = iop->io_ioc;

	/*
	 * Since we will accept block straddles, we need to
	 * read in the first block if we are going to write
	 * to the middle of it.  Although this appears to
	 * duplicate itself later, it helps because the block
	 * will already be buffered upon entry to the loop.
	 */
	left_edge = blocko(iop->io_seek);

	bp = getrbuf(KM_NOSLEEP);
	barea = kmem_alloc(512, KM_NOSLEEP);
	ASSERT(barea);
	bp->b_un.b_addr = barea;
	bp->b_paddr = vtovp(barea);
	bp->b_iodone = NULL;
	bp->b_flags |= (BFRAW | BFNTP);
	bp->b_bufsiz = NBPSCTR;
	

	if(left_edge != 0 && req != BREAD && req != BFLFMT) {
		/*
		 *	i)	Allocate and read from cache.
		 *	ii)	Check for errors.
		 *  
		 * NOTE: We set the error flag in the buffer no matter
		 * what so brelease() sees that it is invalid.  Since
		 * this is a raw write, we don't want to sync the buffer
		 * later on!
		 */
#if 0
		bp = raw_buf_read(NULL, dev, blockn(iop->io_seek), NBPSCTR);
#else
		raw_buf_read(bp, dev, blockn(iop->io_seek), NBPSCTR);
#endif

		if(bp->b_flags & BFERR || bp->b_count == bp->b_resid) {
			SET_U_ERROR(bp->b_errno, "ioreq()");
			buf_invalidate(bp);
			brelease(bp);
			kmem_free(barea, 512);
			freerbuf(bp);
			return;
		}
		
	}
#if 0
	else
		bp = geteblk();
#endif

	/*
	 * The following makes no sense whatsoever, and is
	 * copied from ioreq() in case there is an actual
	 * reason for it.  This code needs to be able to
	 * handle block straddles!!!!  However, it is recognized
	 * that we are holding a crap shoot in assuming that a block
	 * is NBPSCTR bytes on all devices.  Eventually for DDI/DKI
	 * we need to provide a mechanism like physiock() and variable
	 * size buffers.
	 */
#if 0
	if(blocko(iop->io_seek) != 0) {
		SET_U_ERROR(EIO, "io_buffer_req()");
		brelease(bp);
		return;
	}
#endif

	while(iop->io_ioc > 0) {

		/*
	 	 * Since we will accept block straddles, we can decide
	 	 * here whether or not we need to read a block in to
	 	 * update it properly.
		 *
		 *	i)	Release what we held.
		 *	ii)	Allocate and read from cache.
		 *	iii)	Check for errors.
	 	 */
		if(iop->io_ioc < NBPSCTR && req != BREAD && req != BFLFMT) {
			/*
			 * We have to mark the buffer as invalid else
			 * sync() will try and sync it.
			 */

#if 0
			bflush_finish(bp);
			brelease(bp);


			bp = raw_buf_read(NULL, dev, blockn(iop->io_seek), NBPSCTR);
#else
			raw_buf_read(bp, dev, blockn(iop->io_seek), NBPSCTR);
#endif
			

			if(bp->b_flags & BFERR || bp->b_count == bp->b_resid) {
				SET_U_ERROR(bp->b_errno, "ioreq()");
				bflush_finish(bp);
				brelease(bp);
				kmem_free(barea, 512);
				freerbuf(bp);
				return;
			}
		}

		/*
		 * Set up the request.  Some of this may
		 * be redundant and a few cycles could be
		 * saved if cleaned up after the drivers are
		 * made sane.
		 */

		if((bp->b_req = req) == BREAD)
			bp->b_flag |= B_READ;
		else {
			bp->b_flag |= BFMOD;
			bp->b_flag &= ~B_READ;
		}

		bp->b_dev = dev;	
		bp->b_edev = NODEV;
		bp->b_flag |= (f | BFNTP);
		bp->b_bno = blockn(iop->io_seek);
		bp->b_count = NBPSCTR;
		bp->b_resid = bp->b_count;

		/*
		 * Compute the size and offsets of the transfer.
		 */
		right_edge = ((left_edge + iop->io_ioc) > NBPSCTR ? NBPSCTR 
		   : (left_edge + iop->io_ioc));
		amount = (size_t)(right_edge - left_edge);
		request_bytes = NBPSCTR;

		ASSERT(amount > 0);

		/*
		 * If a write, copy data from user space
		 * to the buffer.
		 */
		if(req != BREAD)
			ioread(iop, bp->b_vaddr + left_edge,
			   amount);

		/*
		 * Access device.
		 */
		dblock(dev, bp);
		buffer_wait(bp);

		/*
		 * Compute how many bytes were left in the
		 * request.  This allows partially-served
		 * requests to be recognized. 
		 *
		 * Note also that this allows us to validly update
		 * the io_ioc et al. before the error check because
		 * bp->b_resid should be valid even if there is an error.
		 * Now, this is of course a faint dream at this stage,
		 * but one can hope.  
		 *
		 */
		request_bytes -= bp->b_resid;

#if 0
		printf("req: %lu kamount: %lu, io_ioc: %lu, seek: %lu\n",
		   request_bytes, amount, iop->io_ioc, iop->io_seek);
#endif
		
		/*
		 * Update the seek pointer and we may need to adjust the
		 * count of input. This can happen if we only needed a partial
		 * block to begin with.
		 */
		if(request_bytes > amount)
			request_bytes = amount;
		iop->io_seek += request_bytes;

		/*
		 * If a read, copy data from buffer to user space.
		 */	
		if(req == BREAD)
			iowrite(iop, bp->b_vaddr + left_edge,
			   request_bytes);
		
		/* 
		 * Must check for errors here!  Note
		 * that we can't use geterror() as it stands
		 * because reading past the end of a device
		 * is not an error on raw devices, but geterror()
		 * makes it that way!  To check for the end of
		 * a device on a raw device, the raw device
		 * should return bp->b_count == bp->b_resid (i.e.,
		 * nothing got done).
		 */

		if(bp->b_flags & BFERR || bp->b_count == bp->b_resid) {
			/*
			 * Special handling for writes -- 
			 * If this was the first attempt this call,
			 * then error, else, not an error but return
			 * the correct amount actually read.
			 *
			 * We can tell if this is the first pass or
			 * not by comparing the current amount done with
			 * the original_amount sent to us by the
			 * system call.
			 */
			if((bp->b_flags & B_READ) == 0)
				if((iop->io_ioc + amount) < original_amount) {
					iop->io_ioc += amount;
					SET_U_ERROR(0, "IORQ");
					bflush_finish(bp);
					brelease(bp);
					kmem_free(barea, 512);
					freerbuf(bp);
					return;
				}

			/*
			 * Normal handling for reads.
			 */
			SET_U_ERROR(bp->b_errno, "ioreq()");
			bflush_finish(bp);
			brelease(bp);
			kmem_free(barea, 512);
			freerbuf(bp);
			return;
		}

		/* 
		 * For a write of contiguous bytes, this is
		 * always true after the first straddle.
		 */
		left_edge = 0;

	}

	/* 
	 * Release and unlock buffer -- we're done.
	 */
	bflush_finish(bp);
	brelease(bp);
	kmem_free(barea, 512);
	freerbuf(bp);
}
#endif /* 0 -- io_buffer_req() */

#if __USE_PROTO__
int
physiock(int (*strat)(), buf_t *bp, dev_t dev, int rwflag,
	  daddr_t nblocks, IO *uiop)
#else
int
physiock(strat, bp, dev, rwflag, nblocks, uiop)
int (*strat)();
buf_t *bp;
dev_t dev;
int rwflag;
daddr_t nblocks;
IO *uiop;
#endif /* __USE_PROTO__ */
{

  /* Validate range */
  if (blocko(uiop->io_seek + uiop->io_ioc) > (nblocks - 1)) {
    set_user_error(ENXIO);
    return ENXIO;
  }

  if (blocko(uiop->io_seek + uiop->io_ioc) == (nblocks - 1)) {
    if (rwflag == B_READ)
      return 0;
    else {
      set_user_error(ENXIO);
      return ENXIO;
    }
  }

  return io_user_req(bp, uiop, dev, rwflag == B_READ ? BREAD : BWRITE,
		     BFRAW, strat);
}

 
  
/*
 * Given a buffer pointer, an I/O structure, a device, request type, and
 * a flags word, check the I/O structure and perform the I/O request.
 */
#if	__USE_PROTO__
int
io_user_req (buf_t * bp, IO * iop, o_dev_t dev, int req, int f,
		  int (*strat)(buf_t *bp))
#else
int
io_user_req (bp, iop, dev, req, f, strat)
buf_t * bp;
IO * iop;
o_dev_t dev;
int req;
int f;
int (*strat)();
#endif
{
  size_t amount;
  size_t result_bytes;
  int bp_given = 0;
  int tmp_err;
  
  if (drvmap(dev, DFBLK) == NULL)
    return get_user_error();
  
  ASSERT (iop != NULL);
  
  /*
   * NIGEL: The driver-kit docs said otherwise, but this should always
   * be done. Drivers like the HAI-SCSI that are written to the driver-
   * kit spec don't pass BFBLK even though they need to.
   */
  
  if (/* (f & BFBLK) != 0 && */ blocko(iop->io_seek) != 0
      || (bp && (bp->b_bufsiz % NBPSCTR))) {
    SET_U_ERROR (EIO, "ioreq ()");
    return EIO;
  }
  
  
  if (!(bp_given = (bp != NULL))) {
    /* Allocate a raw buffer header */
    bp = getrbuf(KM_NOSLEEP);
    
    if (bp == NULL) {
      set_user_error(EAGAIN);
      return EAGAIN;
    }
    
    bp->b_iodone = NULL;
  }
  
  while (iop->io_ioc > 0) {
    ASSERT(bp->b_flag & BFNSY);

    /* Must be sector aligned at all times */
    if (blocko(iop->io_seek)) {
#if defined(DEBUG)
      cmn_err(CE_WARN, "io_user_req: unlawful alignment?");
#endif

      set_user_error(EIO);
      buf_invalidate(bp);
      brelease(bp);
      if (!bp_given)
	freerbuf(bp);
      return EIO;
    }

    /* Change from virt addr in IO to system global addr in BUF */
    if (!bp_given && !iomapvp(iop, bp)) {
      SET_U_ERROR (EIO, "ioreq ()");
      goto out;
    }
    
    
    /*
     * How much can we transfer? Several multiples of the block
     * size, up to the next page boundary. Of course, we cannot
     * transfer more than requested. If the resulting request size
     * would be less than a block, shuffle a block's worth of data
     * through the buffer cache.
     *
     * First, get distance to next page boundary. Second, bound by
     * remaining transfer size.
     */
    
    if (!bp_given) {
      if (req == BFLFMT)
	amount = iop->io_ioc;
      else { /* Read or write */
	/* amount is number of bytes to next page bdry. */
	amount = (~ (unsigned) iop->io.vbase & (NBPC - 1)) + 1;

	/* Bound again by requested bytes */
	if (amount > iop->io_ioc)
	  amount = iop->io_ioc;
	
	/* Third, round down to multiple of sector size. */
	if (amount > NBPSCTR)
	  amount &= ~ (NBPSCTR - 1);
	
	bp->b_bufsiz = amount;
      }
    } 
    else
      amount = bp->b_bufsiz < iop->io_ioc ? bp->b_bufsiz : iop->io_ioc;


    if ( amount < NBPSCTR && req != BFLFMT) { /* Use the buffer cache */
      buf_t *tmp_bp;

      /* 
       * This forces sector alignment...be careful if you mess
       * with this.
       */
      if (iop->io_ioc >= NBPSCTR)
	amount = NBPSCTR;

      /* Do a read if a read or a partial write */
      if (req == BREAD || (amount < NBPSCTR && req != BREAD
			   && req != BFLFMT)) {

	/* Read sector */
	tmp_bp = raw_buf_read(NULL, dev, blockn(iop->io_seek), NBPSCTR);

	/* Check error result */
	if (tmp_bp->b_flags & BFERR || tmp_bp->b_count == tmp_bp->b_resid) {
	  set_user_error(tmp_err = tmp_bp->b_errno);
	  buf_invalidate(bp);
	  brelease(bp);
	  buf_invalidate(tmp_bp);
	  brelease(tmp_bp);
      
	  if (!bp_given)
	    freerbuf(bp);
	  return tmp_err;
	}

	/* If a read, we're done */
	if (req == BREAD) {
	  iowrite(iop, tmp_bp->b_vaddr, amount);
	  iop->io_seek += amount;
	  buf_invalidate(tmp_bp);
	  brelease(tmp_bp);
	  continue;
	}

      } else { /* Writing a full sector; just get a buffer */
	tmp_bp = bclaim(dev, blockn(iop->io_seek), NBPSCTR, BUF_SYNC);
      }

      /* Set up for a write */
      tmp_bp->b_flags &= ~B_READ;
      tmp_bp->b_flags |= (BFNTP | f | BFMOD);
      tmp_bp->b_dev = dev;
      tmp_bp->b_edev = NODEV;
      tmp_bp->b_bno = blockn (iop->io_seek);
      tmp_bp->b_resid = tmp_bp->b_count = NBPSCTR;

      ASSERT(amount == NBPSCTR);
      ASSERT(req != BREAD);
      ioread(iop, tmp_bp->b_vaddr, amount);

      if (strat)
	(*strat)(tmp_bp);
      else
	dblock(dev, tmp_bp);
      buffer_wait(tmp_bp);

      /* Error checking */
      if (tmp_bp->b_flags & BFERR || tmp_bp->b_count == tmp_bp->b_resid) {
        set_user_error(tmp_err = tmp_bp->b_errno);
        buf_invalidate(bp);
        brelease(bp);
        buf_invalidate(tmp_bp);
        brelease(tmp_bp);
      
        if (!bp_given)
          freerbuf(bp);
        return tmp_err;
      }

      buf_invalidate(tmp_bp);
      brelease(tmp_bp);

      iop->io_seek += amount;
      continue;
    }
	
    /* ELSE, n >= NBPSCTR || req == BFLFMT */

    ASSERT(amount >= NBPSCTR || req == BFLFMT);
    ASSERT(vtop(bp->b_vaddr + 1) == P2P(bp->b_paddr + 1));

    /* Normal startup before issuing I/O request for bp. */
    bp->b_flag |= f | BFNTP;
    
    if ((bp->b_req = req) == BREAD)
      bp->b_flag |= B_READ;
    else {
      bp->b_flag &= ~B_READ;
      bp->b_flag |= BFMOD;
    }
    
    bp->b_dev = dev;
    bp->b_edev = NODEV;
    bp->b_bno = blockn (iop->io_seek);
    bp->b_resid = bp->b_count = amount;

    if (req != BREAD && bp_given)
      ioread(iop, bp->b_vaddr, amount);
    
    if (strat)
      (*strat)(bp);
    else
      dblock(dev, bp);
    buffer_wait(bp);
    
    result_bytes = amount - bp->b_resid;

    if (req == BREAD && bp_given) {
      iowrite (iop, bp->b_vaddr, result_bytes);
    }
    
    if (bp->b_flags & BFERR || bp->b_count == bp->b_resid) {
      set_user_error(tmp_err = bp->b_errno);
      buf_invalidate(bp);
      brelease(bp);
      
      if (!bp_given)
	freerbuf(bp);
      return tmp_err;
    }
    
    if (!bp_given) {
      /* Offsets not updated; io[read|write] not used. */
      
      /* Success; update offsets */
      iop->io_ioc -= result_bytes;
      iop->io.vbase += result_bytes;
    }

    /* Update seek */
    iop->io_seek += result_bytes;
  }
  
 out:
  buf_invalidate(bp);
  brelease(bp);
  if (!bp_given)
    freerbuf(bp);
  return tmp_err;
}


		
/*
 * Given a buffer pointer, an I/O structure, a device, request type, and
 * a flags word, check the I/O structure and perform the I/O request.
 */

#if	__USE_PROTO__
void ioreq (buf_t * bp, IO * iop, o_dev_t dev, int req, int f)
#else
void
ioreq (bp, iop, dev, req, f)
buf_t * bp;
IO * iop;
o_dev_t dev;
int req;
int f;
#endif
{
  io_user_req (bp, iop, dev, req, f, NULL);
}

/*
 * Like ioreq, but guarantee that no DMA straddle occurs.
 * And assume we are called by fl.c, xt.c, dv.c or someone
 * else who obeys the parameter rules that they do.
 */

#if	__USE_PROTO__
void dmareq (buf_t * bp, IO * iop, o_dev_t dev, int req)
#else
void
dmareq(bp, iop, dev, req)
buf_t *bp;
IO *iop;
dev_t dev;
int req;
#endif
{
  io_user_req(bp, iop, dev, req, BFRAW, NULL);
}


#include <kernel/ddi_cpu.h>


/*
 * Initialise devices.
 * Mark all initialized devices as loaded.
 */

void
devinit ()
{
	DRV * dp;
	int i;

	/*
	 * Set up DDI/DKI-related global data structures. Until this is done,
	 * no DDI/DKI routines can run.
	 */

	while (DDI_GLOB_INIT ())
		cmn_err (CE_PANIC, "Unable to set up DDI/DKI global data");

	/*
	 * After the defer tables have been set up, we can call LOCK_TESTS (),
	 * which we couldn't before because the ..._DEALLOC () calls that the
	 * tests perform at the end require defer-table support.
	 */

	if (LOCK_TESTS (0))
		cmn_err (CE_PANIC, "Lock primitives not functional");

	/*
	 * Call the configured init routines for the system. According to the
	 * specification of init (D2DK), this happens before interrupts are
	 * enabled and before there is any real process context for us to
	 * be able to sleep.
	 */

	for (i = 0 ; i < ninit ; i ++)
		(* inittab [i]) ();

	for (dp = drvl, i = 0 ; i < drvn ; i ++, dp ++) {
		if (dp->d_conp && dp->d_conp->c_load) {
#if	TRACER
			_CHIRP (i + '0', 154);
#endif
			(* dp->d_conp->c_load) ();
#if	0 /* #ifdef	TRACER */
			{
				int		i;
				for (i = 0 ; i < 1000000 ; i ++)
					/* DO NOTHING */ ;
			}
#endif
		}
	}


	/*
	 * Now we can configure the interrupts for the system.
	 */

	for (i = 0 ; i < nintr ; i ++)
		__set_interrupt (inttab + i);

	splbase ();


	/*
	 * And finally, we can call the start routines.
	 */

	for (i = 0 ; i < nstart ; i ++)
		(* starttab [i]) ();
}


#if	0
/*
 * Shut things down, in the right order.
 */

#if	__USE_PROTO__
void (devshutdown) (void)
#else
void
devshutdown __ARGS (())
#endif
{
	int		i;

	spltimeout ();
	ddi_cpu_data ()->dc_int_level = 0;

	for (i = 0 ; i < nexit ; i ++)
		(* exittab [i]) ();

	/*
	 * Turn off interrupts.
	 */

	for (i = 0 ; i < nintr ; i ++)
		clrivec (inttab [i].int_vector);

	for (i = 0 ; i < nhalt ; i ++)
		(* halttab [-- _haltlevel]) ();
}
#endif


/********************************************************************
 * Function: struct inode *dopen(o_dev_t dev, int mode, int flags,
 *				 struct inode *inodep)
 *
 * Description: Given an inode of a driver, open that device.
 *
 * Returns: Inode of opened device or NULL on failure.
 *
 * Affects Globals: None.
 *
 * Affects U-Area: Set u.u_error to error on failure.
 *
 * Side Effects: Unknown (can change argument inodep)
 *
 * Comments:
 *
 * NIGEL: In order to make it at all possible to support the System V DDI / DDK
 * calling conventions for driver entry points, it is necessary for this code
 * to pass the * type * of open being made to the underlying device (which is
 * passed in the 'f' parameter below).
 * 
 * LOUIS: It is possible that the inode returned is not the same one
 * passed by the caller.  For this reason, the caller must keep his
 * own handle on the passed inode and something like ip = dopen(xxxx, ip)
 * should be done with that in mind.
 * 
 */

#if	__USE_PROTO__
struct inode * dopen (o_dev_t dev, int mode, int flags, struct inode * inodep)
#else
struct inode *
dopen (dev, mode, flags, inodep)
o_dev_t		dev;
int		mode;
int		flags;
struct inode  *	inodep;
#endif
{
	CON	      *	cp;

	if (major (dev) == 0xFF && (flags & DFCHR) != 0) {
		/*
		 * Clone open; this only applies to character devices.
		 */

		mode |= IPCLONE;
		dev = minor (dev) << 8;
	}

	if ((cp = drvmap (dev, flags)) == NULL)
		/* drvmap() sets u.u_error on failure */
		return NULL;

	if ((cp->c_flag & flags) == 0) {
		SET_U_ERROR (ENXIO, "dopen ()");
		return NULL;
	}

	if (cp->c_open != NULL)
		(* (kernel_open_t) cp->c_open) (dev, mode, flags,
						SELF->p_credp, & inodep);

	return inodep;
}


/*
 * Close a device.
 *
 * NIGEL: In order to be able to support the System V DDI / DDK calling
 * conventions for driver entry points, this function has to be altered to
 * accept a file-mode and character / block mode parameter. Note that the
 * Coherent 4.0 driver kit documentation says that the driver close entry
 * point is passed the same parameters as the open entry. After this mod,
 * this will be true for the first time.
 */

#if	__USE_PROTO__
int dclose (o_dev_t dev, int mode, int flags, __VOID__ * private)
#else
int
dclose (dev, mode, flags, private)
dev_t		dev;
int		mode;
int		flags;
__VOID__      *	private;
#endif
{
	CON	      *	cp;

	if ((cp = drvmap (dev, flags)) == NULL)
		return -1;

	if (cp->c_close != NULL)
		(* (kernel_close_t) cp->c_close) (dev, mode, flags,
						  SELF->p_credp, private);
	return 0;
}


/*
 * Call the block entry point of a device.
 */

#if	__USE_PROTO__
void dblock (o_dev_t dev, buf_t * bp)
#else
void
dblock (dev, bp)
o_dev_t		dev;
buf_t           *	bp;
#endif
{
	CON	      *	cp;

	if ((cp = drvmap (dev, DFBLK)) == NULL) {
		cmn_err (CE_WARN, "bogus block request, flags=#%x",
			 bp->b_flags);
		backtrace (0);
	} else if (cp->c_block != NULL) {
		(* cp->c_block) (bp);
		return;
	}

	bioerror (bp, ENXIO);
	brelease (bp);
	SET_U_ERROR (ENXIO, "dblock ()");
}


/*
 * Read from a device.
 */

#if	__USE_PROTO__
int dread (o_dev_t dev, IO * iop, __VOID__ * private)
#else
int
dread (dev, iop, private)
o_dev_t		dev;
IO	      *	iop;
__VOID__      *	private;
#endif
{
	CON	      *	cp;

	if ((cp = drvmap (dev, DFCHR)) == NULL)
		return -1;

	if (cp->c_read != NULL) {
		(* (kernel_read_t) cp->c_read) (dev, iop, SELF->p_credp,
						private);
		return 0;
	}

	return -1;
}


/*
 * Write to a device.
 */

#if	__USE_PROTO__
int dwrite (o_dev_t dev, IO * iop, __VOID__ * private)
#else
int
dwrite (dev, iop, private)
o_dev_t		dev;
IO	      *	iop;
__VOID__      *	private;
#endif
{
	CON	      *	cp;

	if ((cp = drvmap (dev, DFCHR)) == NULL)
		return -1;

	if (cp->c_write != NULL) {
		(* (kernel_write_t) cp->c_write) (dev, iop, SELF->p_credp,
						  private);
		return 0;
	}

	return -1;
}


/*
 * Call the ioctl function for a device.
 *
 * NIGEL: In order to support the System V DDI / DDK calling conventions for
 * device driver entry points, this function needs to pass a "mode" parameter
 * indicating the open mode of the file. There are only two calls to this
 * function, for uioctl () and in the / dev / tty driver, "io.386/ ct.c" which is
 * passing its arguments back here (ie, a layered open). The "ct.c" call has
 * not been changed.
 *
 * NIGEL: To support the elimination of u_regl, the current user register set
 * is passed in here (NULL if we are being called from a driver).
 */

#if	__USE_PROTO__
int dioctl (o_dev_t dev, int com, __VOID__ * args, int mode,
	    __VOID__ * private, gregset_t * regsetp)
#else
int
dioctl (dev, com, args, mode, private, regsetp)
dev_t		dev;
int		com;
__VOID__      *	args;
int		mode;
__VOID__      *	private;
gregset_t     *	regsetp;
#endif
{
	CON	      *	cp;
	int		rval;

	if ((cp = drvmap (dev, DFCHR)) == NULL)
		return -1;

	if (cp->c_ioctl == NULL) {
		SET_U_ERROR (ENOTTY, "dioctl ()");
		return -1;
	}

	if (regsetp != NULL) {
		/*
		 * Here we do a bunch of special hacks so that the tty code
		 * can remain ignorant of the myriad variants on the tty
		 * ioctl's.
		 */

		if (__xmode_286 (regsetp)) {
			rval = 0;
			tioc (dev, com, args, mode, SELF->p_credp, & rval,
			      cp->c_ioctl);
			return rval;
		}
		if ((com == TIOCGETP &&
		     ! useracc (args, sizeof (struct sgttyb), 1)) ||
		    ((com == TIOCSETP || com == TIOCSETN) &&
		     ! useracc (args, sizeof (struct sgttyb), 0))) {
			SET_U_ERROR (EFAULT, "dioctl ()");
			return -1;
		}
	}

	rval = 0;
	(* (kernel_ioctl_t) cp->c_ioctl) (dev, com, args, mode, SELF->p_credp,
					  & rval, private);
	return rval;
}


/*
 * Call the powerfail entry point of a device.
 */

int
dpower (dev)
dev_t		dev;
{
	CON	      *	cp;

	if ((cp = drvmap (dev, 0)) != NULL && cp->c_power != NULL) {
		(* cp->c_power) (dev);
		return 0;
	}

	return -1;
}


/*
 * Call the timeout entry point of a device.
 */

int
dtime (dev)
dev_t		dev;
{
	CON	      *	cp;

	if ((cp = drvmap (dev, 0)) != NULL && cp->c_timer != NULL) {
		(* cp->c_timer) (dev);
		return 0;
	}

	return -1;
}


/*
 * Poll a device.
 */

#if	__USE_PROTO__
int dpoll (o_dev_t dev, int events, int msec, __VOID__ * private)
#else
int
dpoll (dev, events, msec, private)
o_dev_t		dev;
int		events;
int		msec;
__VOID__      *	private;
#endif
{
	CON	      *	cp;

	if ((cp = drvmap (dev, DFCHR)) != NULL &&
	    (cp->c_flag & DFPOL) != 0 && cp->c_poll != NULL)
		return (* (kernel_poll_t) cp->c_poll) (dev, events, msec,
							private);

	return POLLNVAL;
}


