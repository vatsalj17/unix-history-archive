/*	@(#)mount.h 1.1 85/05/30 SMI; from UCB 4.4 82/07/19	*/

/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */
struct	mount
{
	struct vfs	m_vfs;		/* vfs structure for this filesystem */
	dev_t		m_dev;		/* device mounted */
	struct buf	*m_bufp;	/* pointer to superblock */
	struct inode	*m_qinod;	/* QUOTA: pointer to quota file */
};

#ifdef KERNEL
/*
 * mount table
 */
extern struct mount	mount[NMOUNT];

/*
 * Operations
 */
struct mount *getmp();
#endif
