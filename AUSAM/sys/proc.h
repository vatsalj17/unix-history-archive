/*
 * One structure allocated per active
 * process. It contains all data needed
 * about the process while the
 * process may be swapped out.
 * Other per process data (user.h)
 * is swapped with the process.
 */
struct	proc
{
	char	p_stat;
	char	p_flag;
	char	p_pri;		/* priority, negative is high */
	char	p_sig;		/* signal number sent to this process */
#ifndef	AUSAM16
	char	p_uid;		/* user id, used to direct tty signals */
#endif	AUSAM16
	char	p_time;		/* resident time for scheduling */
	char	p_cpu;		/* cpu usage for scheduling */
	char	p_nice;		/* nice for scheduling */
#ifdef	AUSAM16
	char	p_dummy;	/* this would be there anyway */
	unsigned p_uid;		/* user id, used to direct tty signals */
#endif	AUSAM16
	struct tty *p_ttyp;	/* controlling tty */	/* fix000 */
	int	p_pid;		/* unique process id */
	int	p_ppid;		/* process id of parent */
	int	p_addr;		/* address of swappable image */
	int	p_size;		/* size of swappable image (*64 bytes) */
	int	p_wchan;	/* event process is awaiting */
	int	*p_textp;	/* pointer to text structure */
#ifdef	PROCESS_QUEUES
	int	*p_link;	/* pointer to next proc */
#endif	PROCESS_QUEUES
#ifdef	NEW_SLEEP
	unsigned p_stl;		/* count of secs left of sleep time */
#endif
#ifdef	TIME_LIMITS
	unsigned p_rtl;		/* count of secs left of real time limit */
#endif
#ifdef	IGNORE_SIGNALS
	unsigned p_ignsig;	/* boolean array of 1st 16 signals if ignored */
#endif	IGNORE_SIGNALS
#ifdef	AUSAML
	struct lnode *p_lnode;	/* pointer to limit structure */
#endif	AUSAML
} proc[NPROC];

#ifdef	MAX_PROC
struct	proc	*max_proc;	/* address of maximum entry used in proc table */
#endif	MAX_PROC
#ifdef	HIGH_PROC
struct	proc	*high_proc;	/* address of maximum entry used in proc table */
unsigned high_nproc;		/* maximum highwater mark in proc table in range 0 -> NPROC */
#endif	HIGH_PROC

/* stat codes */
#define	SSLEEP	1		/* sleeping on high priority */
#ifndef	PROCESS_QUEUES
#define	SWAIT	2		/* sleeping on low priority */
				/* not needed if willing to test p_pri */
#endif	PROCESS_QUEUES
#define	SRUN	3		/* running */
#define	SIDL	4		/* intermediate state in process creation */
#define	SZOMB	5		/* intermediate state in process termination */
#define	SSTOP	6		/* process being traced */

/* flag codes */
#define	SLOAD	01		/* in core */
#define	SSYS	02		/* scheduling process */
#define	SLOCK	04		/* process cannot be swapped */
#define	SSWAP	010		/* process is being swapped out */
#define	STRC	020		/* process is being traced */
#define	SWTED	040		/* another tracing flag */
