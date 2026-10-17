/*
 * Each buffer in the pool is usually doubly linked into 2 lists:
 * the device with which it is currently associated (always)
 * and also on a list of blocks available for allocation
 * for other use (usually).
 * The latter list is kept in last-used order, and the two
 * lists are doubly linked to make it easy to remove
 * a buffer from one list when it was found by
 * looking through the other.
 * A buffer is on the available list, and is liable
 * to be reassigned to another disk block, if and only
 * if it is not marked BUSY.  When a buffer is busy, the
 * available-list pointers can be used for other purposes.
 * Most drivers use the forward ptr as a link in their I/O
 * active queue.
 * A buffer header contains all the information required
 * to perform I/O.
 * Most of the routines which manipulate these things
 * are in bio.c.
 */
struct buf
{
	int	b_flags;		/* see defines below */
	struct	buf *b_forw;		/* headed by devtab of b_dev */
	struct	buf *b_back;		/*  "  */
	struct	buf *av_forw;		/* position on free list, */
	struct	buf *av_back;		/*     if not BUSY*/
	int	b_dev;			/* major+minor device name */
	int	b_wcount;		/* transfer count (usu. words) */
#ifdef	MAPPED_BUFFERS
 /* b_addr MUST have displacement == 016 for 'bswtch' in mch.s to work */
 /* b_xmem MUST follow b_addr and MUST be an int for above reason      */
#endif	MAPPED_BUFFERS
	char	*b_addr;		/* low order core address */
	int	b_xmem;			/* high order core address */
	unsigned b_blkno;		/* block # on device */ /* fix000 */
	char	b_error;		/* returned after I/O */
	char	b_sector;		/* used by optimised rk driver */
	unsigned b_scratch;		/* used by optimised rk driver */
	int	b_resid;		/* words not transferred after error */
} buf[NBUF];

#ifdef	MAPPED_BUFFERS
/* 
 * The buffers are all at the same virtual address, namely 120000 (octal).
 * The following declaration is merely for convenience.
 * The real declaration is in "m70.s".
 * (See also the declaration of "u".)
 */
struct {
	char	buff[512];
	} b;
#endif	MAPPED_BUFFERS
/*
 * Each block device has a devtab, which contains private state stuff
 * and 2 list heads: the b_forw/b_back list, which is doubly linked
 * and has all the buffers currently associated with that major
 * device; and the d_actf/d_actl list, which is private to the
 * device but in fact is always used for the head and tail
 * of the I/O queue for the device.
 * Various routines in bio.c look at b_forw/b_back
 * (notice they are the same as in the buf structure)
 * but the rest is private to each device driver.
 */
struct devtab
{
	char	d_active;		/* busy flag */
	char	d_errcnt;		/* error count (for recovery) */
#ifndef	HASHED_BUFFERS
	struct	buf *b_forw;		/* first buffer for this dev */
	struct	buf *b_back;		/* last buffer for this dev */
#endif	HASHED_BUFFERS
	struct	buf *d_actf;		/* head of I/O queue */
	struct 	buf *d_actl;		/* tail of I/O queue */
};

/*
 * This is the head of the queue of available
 * buffers-- all unused except for the 2 list heads.
 * The 'av' list is the free list.
 * The 'b' list contains all blocks associated with 'nodev'.
 */
struct	buf bfreelist;

/*
 * These flags are kept in b_flags.
 */
#define	B_WRITE	0	/* non-read pseudo-flag */
#define	B_READ	01	/* read when I/O occurs */
#define	B_DONE	02	/* transaction finished */
#define	B_ERROR	04	/* transaction aborted */
#define	B_BUSY	010	/* not on av_forw/back list */
#define	B_PHYS	020	/* Physical IO potentially using UNIBUS map */
#define	B_MAP	040	/* This block has the UNIBUS map allocated */
#define	B_WANTED 0100	/* issue wakeup when BUSY goes off */
#ifdef	QMOUNT
#define	B_MOUNT	0200	/* a mounted filesystem */
#endif	QMOUNT
#define	B_ASYNC	0400	/* don't wait for I/O completion */
#define	B_DELWRI 01000	/* don't write till block leaves available list */
#define	B_NOTIO	02000	/* this buffer contains non i/o info */
#ifdef	BUFFER_AGING
#define	B_AGE	04000	/* for proper ageing on write behind */
			/* ie place at head of free queue on brelsing */
#endif
#define	B_INVAL	010000	/* umount called this device ==> block invalid  fix030 */



#ifdef	HASHED_BUFFERS
/*
 * The buffer hash table.
 */
struct bhashelt {
	int	b_crap;
	struct	buf	*b_forw;		/* as above */
	struct	buf	*b_back;		/*     "    */
}
	btab[BHASHSZ];
#endif	HASHED_BUFFERS
