/*
 * tunable variables
 */

#define	NBUF	6		/* size of buffer cache */
#define	NINODE	50		/* number of in core inodes */
#define	NFILE	50		/* number of in core file structures */
#define	NMOUNT	2		/* number of mountable file systems */
#define	SSIZE	20		/* initial stack size (*64 bytes) */
#define	NOFILE	15		/* max open files per process */
#define	CANBSIZ	256		/* max size of typewriter line */
#define	NCALL	4		/* max simultaneous time callouts */
#define	NPROC	13		/* max number of processes */
#define	NCLIST	100		/* max total clist size */
#define	HZ	60		/* Ticks/second of the clock */

/*
 * priorities
 * probably should not be
 * altered too much
 */

#define	PSWP	-100
#define	PINOD	-90
#define	PRIBIO	-50
#define	PPIPE	1
#define	PWAIT	40
#define	PSLEP	90
#define	PUSER	100

/*
 * signals
 * dont change
 */

#define	NSIG	20
#define		SIGHUP	1	/* hangup */
#define		SIGINT	2	/* interrupt (rubout) */
#define		SIGQIT	3	/* quit (FS) */
#define		SIGINS	4	/* illegal instruction */
#define		SIGTRC	5	/* trace or breakpoint */
#define		SIGIOT	6	/* iot */
#define		SIGEMT	7	/* emt */
#define		SIGFPT	8	/* floating exception */
#define		SIGKIL	9	/* kill */
#define		SIGBUS	10	/* bus error */
#define		SIGSEG	11	/* segmentation violation */
#define		SIGSYS	12	/* sys */
#define		SIGPIPE	13	/* end of pipe */
#define		SIGCLK	14	/* alarm clock */

/*
 * fundamental constants
 * cannot be changed
 */

#define	USIZE	16		/* size of user block (*64) */
#define	NULL	0
#define	NODEV	(-1)
#define	ROOTINO	1		/* i number of all roots */
#define	DIRSIZ	14		/* max characters per directory */

/*
 * structure to access an
 * integer in bytes
 */
struct
{
	char	lobyte;
	char	hibyte;
};

/*
 * structure to access an integer
 */
struct
{
	int	integ;
};

/*
 * Certain processor registers
 */
#define PS	0177776
#define KL	0177560
#define SW	0177570

/*
 * configuration dependent variables
 */

#define UCORE	(16*32)
#define TOPSYS	12*2048
#define TOPUSR	28*2048
#define ROOTDEV 0
#define SWAPDEV 0
#define SWPLO	4000
#define NSWAP	872
#define SWPSIZ	66
