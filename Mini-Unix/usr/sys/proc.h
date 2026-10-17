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
	char	p_uid;		/* user id, used to direct tty signals */
	char	p_time;		/* resident time for scheduling */
	char	p_cpu;		/* cpu usage for scheduling */
	char	p_nice;		/* nice for cpu usage */
	int	p_pgrp;		/* name of process group leader */
	int	p_pid;		/* unique process id */
	int	p_ppid;		/* process id of parent */
	int	p_wchan;	/* event process is awaiting */
	int	p_clktim;	/* time to alarm clock signal */
	int	p_size;		/* process size (for swap) */
} proc[NPROC];

/* stat codes */
#define	SSLEEP	1		/* awaiting an event */
#define	SWAIT	2		/* (abandoned state) */
#define	SRUN	3		/* running */
#define	SIDL	4		/* intermediate state in process creation */
#define	SZOMB	5		/* intermediate state in process termination */

/* flag codes */
#define	SLOAD	01		/* in core */
#define	SSYS	02		/* scheduling process */
#define	SLOCK	04		/* process cannot be swapped */
#define	SSWAP	010		/* process is being swapped out */
