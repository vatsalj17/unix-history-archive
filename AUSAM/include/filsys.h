/*
 * Definition of the unix super block.
 * The root super block is allocated and
 * read in iinit/alloc.c. Subsequently
 * a super block is allocated and read
 * with each mount (smount/sys3.c) and
 * released with unmount (sumount/sys3.c).
 * A disk block is ripped off for storage.
 * See alloc.c for general alloc/free
 * routines for free list and I list.
 */
struct	filsys
{
	unsigned s_isize;	/* size in blocks of I list */ /* fix000 */
	unsigned s_fsize;	/* size in blocks of entire volume */ /* fix000 */
	int	s_nfree;	/* number of in core free blocks (0-100) */
	unsigned s_free[100];	/* in core free blocks */ /* fix000 */
	int	s_ninode;	/* number of in core I nodes (0-100) */
	int	s_inode[100];	/* in core free I nodes */
	char	s_flock;	/* lock during free list manipulation */
	char	s_ilock;	/* lock during I list manipulation */
	char	s_fmod;		/* super block modified flag */
	char	s_ronly;	/* mounted read-only flag */
	long	s_time;		/* current date of last update */  /* fix000 */
	unsigned s_ilowa;	/* lowest block with a free inode  fix026 */
	int	pad[47];
};

/*
 * ilock codes			fix026
 */
#define	S_BUSY		01	/* ialloc in progress */
#define S_WANTED	02	/* want to ialloc */
