/*
 * Random set of variables
 * used by more than one
 * routine.
 */
extern char	canonb[CANBSIZ];	/* buffer for erase and kill (#@) */
extern int	coremap[CMAPSIZ];	/* space for core allocation */
extern int	swapmap[SMAPSIZ];	/* space for swap allocation */
extern int	*rootdir;		/* pointer to inode of root directory */
extern int	cputype;		/* type of cpu =40, 45, or 70 */
extern int	execnt;			/* number of processes in exec */
extern int	lbolt;			/* time of day in HZ not in time */
extern int	time[2];		/* time in sec from 1970 */
extern int	tout[2];		/* time of day of next sleep */
/*
 * The callout structure is for
 * a routine arranging
 * to be called by the clock interrupt
 * (clock.c) with a specified argument,
 * in a specified amount of time.
 * Used, for example, to time tab
 * delays on teletypes.
 */
extern struct	callo
{
	int	c_time;		/* incremental time */
	int	c_arg;		/* argument to routine */
	int	(*c_func)();	/* routine */
} callout[NCALL];
/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */
extern struct	mount
{
	int	m_dev;		/* device mounted */
	int	*m_bufp;	/* pointer to superblock */
	int	*m_inodp;	/* pointer to mounted on inode */
} mount[NMOUNT];
extern int	mpid;			/* generic for unique process id's */
extern char	runin;			/* scheduling flag */
extern char	runout;			/* scheduling flag */
extern char	runrun;			/* scheduling flag */
extern int	curpri;			/*** more scheduling ***/
extern int	maxmem;			/* actual max memory per process */
extern char	*memtop;		/*** highest addr in physical memory */
extern int	rootdev;		/* dev of root see conf.c */
extern int	swapdev;		/* dev of swap see conf.c */
extern int	swplo;			/* block number of swap space */
extern int	nswap;			/* size of swap space */
extern int	updlock;		/* lock for sync */
extern int	rablock;		/* block to be read ahead */
extern int	regloc[];	/***/	/* locs. of saved user registers (trap.c) */
