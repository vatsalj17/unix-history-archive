/*
 * Random set of variables used by more than one routine.
 */
struct map {
	int	m_size;
	char	*m_addr;
};
struct map coremap[];	/* core allocation map */
struct map swapmap[];	/* swap allocation map */
int	*rootdir;		/* pointer to inode of root directory */
int	*runq;			/* head of linked list of running processes */
int	cputype;		/* type of cpu =40, 45, or 70 */
int	lbolt;			/* time of day in 60th not in time */
long	time;			/* time in sec from 1970 */
long	tout;			/* time of day of next sleep */
/*
 * The callout structure is for a routine arranging to be
 *  called by the clock interrupt (clock.c) with a specified argument,
 * in a specified amount of time.
 * Used, for example, to time tab delays on typewriters.
 */
struct	callo
{
	int	c_time;		/* incremental time */
	int	c_arg;		/* argument to routine */
	int	(*c_func)();	/* routine */
} callout[];
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
} mount[];

int	mpid;			/* generic for unique process id's */
char	runin;			/* scheduling flag */
char	runout;			/* scheduling flag */
char	runrun;			/* scheduling flag */
char	curpri;			/* more scheduling */
int	maxmem;			/* actual max memory per process */
int	*lks;			/* pointer to clock device */
int	rootdev;		/* dev of root see conf.c */
int	swapdev;		/* dev of swap see conf.c */
int	swplo;			/* block number of swap space */
int	nswap;			/* size of swap space */
int	updlock;		/* lock for sync */
int	rablock;		/* block to be read ahead */
char	regloc[];		/* locs. of saved user registers (trap.c) */
char	pwbname[];
