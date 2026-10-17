/*
 * One structure allocated per active
 * process. It contains all data needed
 * about the process while the
 * process may be swapped out.
 * Other per process data (user.h)
 * is swapped with the process.
 *
 * IN ADDITION TO THE IFDEF NEWSCHED CHANGES, SOME V7 FIELDS HAVE
 * BEEN MOVED AROUND TO MAKE THE PROC TABLE INFORMATION FOR ZOMBIES A
 * LITTLE MORE USEFUL (see ps.c for more clues).
 */
struct	proc {
	struct proc *p_link;	/* linked list of running processes */
	char	p_stat;
	char	p_flag;
#ifdef NEWSCHED
	char	p_level;	/* queue level for cpu scheduling */
	char	p_baslev;	/* "base level" for scheduling */
	char	p_quant;	/* incomplete quantum leftover part */
	char	p_sflag;	/* scheduler information flags */
#else
	char	p_pri;		/* priority, negative is high */
	char	p_time;		/* resident time for scheduling */
	char	p_cpu;		/* cpu usage for scheduling */
	char	p_nice;		/* nice for cpu usage */
#endif
	short	p_sig;		/* signals pending to this process */
	short	p_uid;		/* user id, used to direct tty signals */
	short	p_pid;		/* unique process id */
	short	p_ppid;		/* process id of parent */
	short	p_size;		/* size of swappable image (clicks) */
	caddr_t p_wchan;	/* event process is awaiting */
	struct text *p_textp;	/* pointer to text structure */
	short	p_addr;		/* address of swappable image */
	short	p_pgrp;		/* name of process group leader */
	int	p_clktim;	/* time to alarm clock signal */
};

extern struct proc proc[];	/* the proc table itself */

/* stat codes */
#define SEMPTY	0		/* free proc table slot */
#define	SSLEEP	1		/* awaiting an short-term event */
#define	SWAIT	2		/* awaiting a long-term event */
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
#define	SULOCK	0100		/* user settable lock in core */

/* sflag codes */

#define STTYIN	0001		/* indicates tty input since last SETRQ */
#define STTYOUT	0002		/* indicates tty output since last SETRQ */
#define SEXEC	0004		/* indicates process is doing EXEC call */

/*
 * parallel proc structure
 * to replace part with times
 * to be passed to parent process
 * in ZOMBIE state.
 */
struct	xproc {
	struct proc *xp_link;
	char	xp_stat;
	char	xp_flag;
#ifdef NEWSCHED
	char	xp_level;
	char	xp_sflag;
	short	xp_quant;
#else
	char	xp_pri;
	char	xp_time;
	char	xp_cpu;
	char	xp_nice;
#endif
	short	xp_sig;		/* signals pending to this process */
	short	xp_uid;		/* user id, used to direct tty signals */
	short	xp_pid;		/* unique process id */
	short	xp_ppid;	/* process id of parent */
	short	xp_size;
	/* beginning of overlayed fields */
	short	xp_xstat;	/* Exit status for wait */
	time_t	xp_utime;	/* user time, this proc */
	time_t	xp_stime;	/* system time, this proc */
};
