/*
 * Structure of the super-block
 */
struct	filsys {
	unsigned short s_isize;	/* size in blocks of i-list */
	daddr_t	s_fsize;   	/* size in blocks of entire volume */
	short  	s_nfree;   	/* number of addresses in s_free */
	daddr_t	s_free[NICFREE];/* free block list */
	short  	s_ninode;  	/* number of i-nodes in s_inode */
	ino_t  	s_inode[NICINOD];/* free i-node list */
	char   	s_flock;   	/* lock during free list manipulation */
	char   	s_ilock;   	/* lock during i-list manipulation */
	char   	s_fmod;    	/* super block modified flag */
	char   	s_ronly;   	/* mounted read-only flag */
	time_t 	s_time;    	/* last super block update */
	/* initialized by mkfs, updated by fsck, but not used by system */
	daddr_t	s_tfree;   	/* total free blocks*/
	ino_t  	s_tinode;  	/* total free inodes */
	/* initialized by mkfs, used by fsck for rebuilding free lists */
	short  	s_m;       	/* interleave factor - free list spacing */
	short	s_n;		/* interleave factor - blocks per cylinder */
	/* remainder not maintained by Unix/v7m operating system */
	char   	s_fname[6];	/* file system name */
	char   	s_fpack[6];	/* file system pack name */
};
