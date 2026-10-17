/*
 * MASSBUS SMD disk data structures
 * one hpdisk per drive
 */

struct hpdisk {
	struct hpdevice *addr;
	char flags;
	char type;	/* index into hpst */
	char errcnt;
	char recal;
	short badsec;
	struct buf *actf, *actl;	/* queue of buffers waiting for xfer */
};

/*
 * device size info
 * here so hpdump can get it
 */
struct	hptype {
	short	type;	/* hpdt drive type */
	char	nsect;	/* sectors per track */
	char	ntrak;	/* tracks per cylinder */
	char	sdist;	/* sectors twixt search and io */
	char	rdist;	/* sectors close enough not to search */
	short	nspc;	/* sectors per cylinder */
	short	ncyl;	/* cylinders per pack */
	struct	size *sizes;
};
