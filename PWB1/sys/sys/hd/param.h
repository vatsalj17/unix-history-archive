/*
 * fundamental variables
 * don't change too often
 */

#define	NOFILE	15		/* max open files per process */
#define	SSIZE	12		/* initial stack size (*64 bytes) */
#define	SINCR	12		/* increment of stack (*64 bytes) */
#define	NCARGS	4096
#define	HZ	60		/* Ticks/second of the clock */
#define	NSWB	3		/* size of swap pool */
#define	CANBSIZ	256		/* max size of typewriter line */
#define	MAXMEM	(64*32)		/* max core per process - first # is Kw */
#define	USIZE	12		/* size of user block (*64) */
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

#define	NSIG	16
/*
 * No more than 16 signals (1-16) because they are
 * stored in bits in a word.
 */
#define	SIGHUP	1	/* hangup */
#define	SIGINT	2	/* interrupt (rubout) */
#define	SIGQIT	3	/* quit (FS) */
#define	SIGINS	4	/* illegal instruction */
#define	SIGTRC	5	/* trace or breakpoint */
#define	SIGIOT	6	/* iot */
#define	SIGEMT	7	/* emt */
#define	SIGFPT	8	/* floating exception */
#define	SIGKIL	9	/* kill, uncatchable termination */
#define	SIGBUS	10	/* bus error */
#define	SIGSEG	11	/* segmentation violation */
#define	SIGSYS	12	/* bad system call */
#define	SIGPIPE	13	/* end of pipe */
#define	SIGCLK	14	/* alarm clock */
#define	SIGTRM	15	/* Catchable termination */

/*
 * fundamental constants
 * cannot be changed
 */

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
 * structure to access a long as integers
 */
struct {
	int	hiword;
	int	loword;
};

/*
 * Certain processor registers
 */
#define PS	0177776
#define KL	0177560
#define SW	0177570

/*
 * Some macros for units conversion
 */
/* Core clicks (64 bytes) to segments */
#define	ctos(x)	((x+127)>>7)

/* Core clicks (64 bytes) to blocks */
#define	ctob(x)	((x+7)>>3)

/* major part of a device */
#define major(x)	(int)((x.hibyte)&0377)

/* minor part of a device */
#define minor(x)	(int)(x&0377)
