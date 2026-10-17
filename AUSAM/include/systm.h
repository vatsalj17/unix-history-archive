/*
 * Random set of variables
 * used by more than one
 * routine.
 */
#define	canonb	u.u_heap	/* buffer for erase and kill (#@) */
int	coremap[CMAPSIZ];	/* space for core allocation */
int	swapmap[SMAPSIZ];	/* space for swap allocation */
int	umap[UMAPSIZ];		/* space for UNIBUS-MAP allocation */
struct inode *rootdir;		/* pointer to inode of root directory */
int	cputype;		/* type of cpu =40, 45, or 70 */
int	execnt;			/* number of processes in exec */
int	lbolt;			/* time of day in 60th not in time */
long	time;			/* time in sec from 1970 */  /* fix000 */
unsigned bufarea;


/*
 * The timout structure is a redefinition
 * of a clist block used to store timeout
 * information as for a callout above.
 */
struct	timout	{
	struct	timout	*t_next;
	int		t_arg;
	int		(*t_func)();
	int		t_time;
}
	timouts;

#define tfreelist	timouts.t_func.t_next	/* the timeout reserve freelist */
#define tfreecount	timouts.t_time		/* number of blocks in reserve freelist */
#define toverloadcount	timouts.t_arg		/* number of times the freelists were empty */

/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */
struct	mount
{
	int	m_dev;		/* device mounted */
	int	*m_bufp;	/* pointer to superblock */
	int	*m_inodp;	/* pointer to mounted on inode */
} mount[NMOUNT];

struct lnode lnode [ MAXUSERS ];

int	mpid;			/* generic for unique process id's */
char	runin;			/* scheduling flag */
char	runout;			/* scheduling flag */
char	runrun;			/* scheduling flag */
char	curpri;			/* more scheduling */
unsigned maxmem;		/* actual max memory per process */ /* fix000 */
int	*lks;			/* pointer to clock device */
unsigned rootdev;		/* dev of root see conf.c */ /* fix000 */
unsigned swapdev;		/* dev of swap see conf.c */ /* fix000 */
int	swplo;			/* block number of swap space */
int	nswap;			/* size of swap space */
int	updlock;		/* lock for sync */
int	rablock;		/* block to be read ahead */
char	regloc[];		/* locs. of saved user registers (trap.c) */

int	bufscr[101];
