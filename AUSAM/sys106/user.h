/*
 * The user structure.
 * One allocated per process.
 * Contains all per process data
 * that doesn't need to be referenced
 * while the process is swapped.
 * The user block is USIZE*64 bytes
 * long; resides at virtual kernel
 * loc 140000; contains the system
 * stack per user; is cross referenced
 * with the proc structure for the
 * same process.
 */
struct user
{
	int	u_rsav[2];		/* save r5,r6 when exchanging stacks */
#ifdef	BIG_UNIX | MAPPED_BUFFERS
	int	u_ksave;		/* savearea of kisa5 */
#endif	BIG_UNIX | MAPPED_BUFFERS
		/* rsav, ksave must be in this order */
#ifdef	FPU
	int	u_fec;			/* saved fec */
	int	u_fea;			/* saved fea */
	int	u_fsav[25];		/* save fp registers */
#endif	FPU
	char	u_segflg;		/* flag for io:	*/
#define	SEG_USD	0			/* user data segment */
#define	SEG_KER	1			/* kernal data segment */
#define	SEG_UST	2			/* user text segment */
#define	SEG_SSD	3			/* supervisor segment */
	char	u_error;		/* return error code */
#ifndef	AUSAM16
	char	u_uid;			/* effective user id */
	char	u_gid;			/* effective group id */
	char	u_ruid;			/* real user id */
	char	u_rgid;			/* real group id */
#endif	AUSAM16
#ifdef	AUSAM16
	unsigned u_uid;			/* effective 16bit uid */
	unsigned u_ruid;		/* real 16bit uid */
#endif	AUSAM16
	int	u_procp;		/* pointer to proc structure */
	char	*u_base;		/* base address for IO */
	char	*u_count;		/* bytes remaining for IO */
	long	u_offset;		/* offset in file for IO */ /* fix000 */
	int	*u_cdir;		/* pointer to inode of current directory */
	char	u_dbuf[DIRSIZ];		/* current pathname component */
	char	*u_dirp;		/* current pointer to inode */
	struct	{			/* current directory entry */
		int	u_ino;
		char	u_name[DIRSIZ];
	} u_dent;
	struct inode *u_pdir;		/* inode of parent directory of dirp */ /* fix000 */
#ifndef	_1140
	int	u_uisa[16];		/* prototype of segmentation addresses */
	int	u_uisd[16];		/* prototype of segmentation descriptors */
#endif
#ifdef	_1140
	int	u_uisa[8];		/* prototype of segmentation addresses */
	int	u_uisd[8];		/* prototype of segmentation descriptors */
#endif
	struct file *u_ofile[NOFILE];	/* pointers to file structures of open files */ /* fix000 */
#ifndef	SHARED_DATA | EP_ADDRESS
	int	u_arg[5];		/* arguments to current system call */
#endif
#ifdef	SHARED_DATA | EP_ADDRESS
	int	u_arg[8];		/* arguments to current system call */
#endif
	int	u_tsize;		/* text size (*64) */
	int	u_dsize;		/* data size (*64) */
	int	u_ssize;		/* stack size (*64) */
	int	u_sep;			/* flag for I and D separation */
	int	u_qsav[2];		/* label variable for quits and interrupts */
	int	u_ssav[2];		/* label variable for swapping */
	int	u_signal[NSIG];		/* disposition of signals */
	long	u_utime;		/* this process user time */  /* fix000 */
	long	u_stime;		/* this process system time */  /* fix000 */
	long	u_cutime;		/* sum of childs' utimes */  /* fix000 */
	long	u_cstime;		/* sum of childs' stimes */  /* fix000 */
	int	*u_ar0;			/* address of users saved R0 */
	int	u_prof[4];		/* profile arguments */
#ifdef	TIME_LIMITS
	int	u_cpusec;		/* seconds left to abort */
	char	u_tix;			/* ticks left to dec of cpusec */
#endif
#ifndef	SHARED_DATA
	char	u_intflg;		/* catch intr from sys */
#endif
#ifdef	SHARED_DATA
	char u_flags;
#define	USHRDATA 0200		/* set if this process using shared data */
#define	UINTFLG	0001		/* used by ptrace for catching interrupt from sys */
#endif
#ifdef LOCKING
	struct inode *u_locki;	/* currently locked file if any. */
	char	u_lockf;	/* flags ... defined in inode.h */
#endif
#ifdef	VICAR
	char	u_heap[0];		/* must be last entry */
#endif


		/* kernel stack per user
		 * extends from u + USIZE*64
		 * backward not to reach here
		 */
} u;

/* u_error codes */
#define	EPERM	1
#define	ENOENT	2
#define	ESRCH	3
#define	EINTR	4
#define	EIO	5
#define	ENXIO	6
#define	E2BIG	7
#define	ENOEXEC	8
#define	EBADF	9
#define	ECHILD	10
#define	EAGAIN	11
#define	ENOMEM	12
#define	EACCES	13
#define	EFAULT	14		/* fix025 */
#define	ENOTBLK	15
#define	EBUSY	16
#define	EEXIST	17
#define	EXDEV	18
#define	ENODEV	19
#define	ENOTDIR	20
#define	EISDIR	21
#define	EINVAL	22
#define	ENFILE	23
#define	EMFILE	24
#define	ENOTTY	25
#define	ETXTBSY	26
#define	EFBIG	27
#define	ENOSPC	28
#define	ESPIPE	29
#define	EROFS	30
#define	EMLINK	31
#define	EPIPE	32
#ifdef	LARGE_FILE_REFERENCES
#define	ENREF	33	/* too many references to file */
#endif
#ifdef	TTY_CONNECT
#define	EREDIRECT	34	/* tty re-direction error */
#define	ENOREDIRECT	35	/* tty not capable of re-direction */
#endif	TTY_CONNECT
#ifdef	AUSAML
#define	EPROCLIM	36	/* too many process this user */
#define	ETOOMANYU	37	/* no limit structure left */
#define	ENOLIM		38	/* no limit structure exists */
#define	ENOROOTLIM	39	/* no limits for root */
#define	ELIMEXIST	40	/* limit already exists */
#define EDISKLIM	41	/* brick wall disk limit reached */
#endif	AUSAML

#define	EOPENFAIL	42	/* fix032 - open failure on single open device */
#define	EDBIG		43	/* fix040 - directory with > 4095 files */
