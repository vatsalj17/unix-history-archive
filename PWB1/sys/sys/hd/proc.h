/*
 * One structure allocated per active process. It contains all data needed
 * about the process while the process may be swapped out.
 * Other per process data (user.h) is swapped with the process.
 */
struct	proc {
	char	p_stat;
	char	p_flag;
	char	p_pri;		/* priority, negative is high */
	char	p_time;		/* resident time for scheduling */
	char	p_cpu;		/* cpu usage for scheduling */
	char	p_nice;		/* nice for cpu usage */
	int	p_sig;		/* signals pending to this process */
	int	p_uid;		/* user id, used to direct tty signals */
	int	p_pgrp;		/* name of process group leader */
	int	p_pid;		/* unique process id */
	int	p_ppid;		/* process id of parent */
	int	p_addr;		/* address of swappable image */
	int	p_size;		/* size of swappable image (*64 bytes) */
	int	p_wchan;	/* event process is awaiting */
	int	*p_textp;	/* pointer to text structure */
	int	*p_link;	/* linked list of running processes */
	int	p_clktim;	/* time to alarm clock signal */
};
extern struct proc proc[];

/* stat codes */
#define	SSLEEP	1		/* awaiting an event */
#define	SWAIT	2		/* (abandoned state) */
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

/*
 * Structure used to access saved times and status
 * of a dead processs, by the parent.
 * Overlays the proc structure.
 */
struct	xproc {
	char	p_stat;
	char	p_flag;
	char	p_pri;		/* priority, negative is high */
	char	p_time;		/* resident time for scheduling */
	char	p_cpu;		/* cpu usage for scheduling */
	char	p_nice;		/* nice for cpu usage */
	int	p_sig;		/* signals pending to this process */
	int	p_uid;		/* user id, used to direct tty signals */
	int	p_pgrp;		/* name of process group leader */
	int	p_pid;		/* unique process id */
	int	p_ppid;		/* process id of parent */
	int	xp_xstat;	/* Exit status for wait */
	long	xp_utime;	/* user time, this proc */
	long	xp_stime;	/* system time, this proc */
	int	p_clktim;	/* time to alarm clock signal */
};
