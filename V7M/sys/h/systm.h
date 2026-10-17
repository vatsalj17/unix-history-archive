/*
 * Random set of variables
 * used by more than one
 * routine.
 */
char	canonb[CANBSIZ];	/* buffer for erase and kill (#@) */
struct inode *rootdir;		/* pointer to inode of root directory */
struct proc *runq;		/* head of linked list of running processes */
/*
 * The following seven parameters describe the hardware
 * features present on the current CPU.
 * They are set up by the startup code.
 */
int	sepid;			/* separate I & D space */
int	ubmaps;			/* unibus map */
int	rh70hs;			/* hs massbus controller type */
int	rh70ml;			/* ml massbus controller type */
int	rh70hp;			/* hp massbus controller type */
int	rh70hm;			/* hm massbus controller type */
int	rh70ht;			/* ht massbus controller type */
int	nmser;			/* number of memory error reg's */
int	cdreg;			/* console display register */
int	lbolt;			/* time of day in 60th not in time */
time_t	time;			/* time in sec from 1970 */

/*
 * Nblkdev is the number of entries
 * (rows) in the block switch. It is
 * set in binit/bio.c by making
 * a pass over the switch.
 * Used in bounds checking on major
 * device numbers.
 */
int	nblkdev;

/*
 * Number of character switch entries.
 * Set by cinit/tty.c
 */
int	nchrdev;

int	mpid;			/* generic for unique process id's */
char	runin;			/* scheduling flag */
char	runout;			/* scheduling flag */
char	runrun;			/* scheduling flag */
char	curpri;			/* more scheduling */
int	maxmem;			/* actual max memory per process */
physadr	lks;			/* pointer to clock device */
daddr_t	swplo;			/* block number of swap space */
int	nswap;			/* size of swap space */
int	updlock;		/* lock for sync */
daddr_t	rablock;		/* block to be read ahead */
extern	char	regloc[];	/* locs. of saved user registers (trap.c) */
char	msgbuf[MSGBUFS];	/* saved "printf" characters */
dev_t	rootdev;		/* device of the root */
dev_t	swapdev;		/* swapping device */
dev_t	pipedev;		/* pipe device */
extern	int	icode[];	/* user init code */
extern	int	szicode;	/* its size */

dev_t getmdev();
daddr_t	bmap();
struct inode *ialloc();
struct inode *iget();
struct inode *owner();
struct inode *maknode();
struct inode *namei();
struct buf *alloc();
struct buf *getblk();
struct buf *geteblk();
struct buf *bread();
struct buf *breada();
struct filsys *getfs();
struct file *getf();
struct file *falloc();
int	uchar();
/*
 * Instrumentation
 */
long	cp_time[5];	/* CPU time states */
			/* user	*/
			/* nice */
			/* system */
			/* idle - total */
			/* idle with I/O active */
/*
 * WARNING !,
 * DK_NC is defined here and in /usr/src/cmd/iostat.c,
 * if one is changed the other must also be changed.
 */
#define	DK_NC	6	/* number of possible disk controllers */
struct	ios		/* I/O statistics, one per drive */
{
	char	dk_tr;		/* indicates transfer rate of drive */
	char	dk_busy;	/* drive activity flag */
	long	dk_numb;	/* number of transfers for drive */
	long	dk_wds;		/* number of 32 word chunks transfered */
	long	dk_time;	/* drive activity time tally */
};

struct	ios	*dk_iop[DK_NC];	/* controller ios structure pointers */
char	dk_nd[DK_NC];		/* number of drives per controller */
long	tk_nin;
long	tk_nout;

/*
 * Structure of the system-entry table
 */
extern struct sysent {
	char	sy_narg;		/* total number of arguments */
	char	sy_nrarg;		/* number of args in registers */
	int	(*sy_call)();		/* handler */
} sysent[];
