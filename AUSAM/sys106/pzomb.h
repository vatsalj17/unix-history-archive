/*
 *	Structure to reference a process after it
 *	is dead. This must conform exactly to
 *	'proc.h', except for the things explicitly used.
 */


struct	pzomb
{
	char	p_stat;
	char	p_flag;
	char	p_pri;		/* priority, negative is high */
	char	p_sig;		/* signal number sent to this process */
#ifndef	AUSAM16
	int	pz_ur0;		/* returned status */
#endif	AUSAM16
#ifdef	AUSAM16
	char	p_time;
#endif	AUSAM16
	char	p_cpu;		/* cpu usage for scheduling */
#ifndef	AUSAM16
	char	p_nice;		/* nice for scheduling */
#endif	AUSAM16
#ifdef	AUSAM16
	int	pz_ur0;		/* returned status */
	unsigned p_uid;		/* user id, used to direct tty signals */
#endif	AUSAM16
	struct tty *p_ttyp;	/* controlling tty */
	int	p_pid;		/* unique process id */
	int	p_ppid;		/* process id of parent */
	long	pz_utime;					   /* fix000 */
	long	pz_stime;	/* times returned from process */  /* fix000 */
#ifdef	PROCESS_QUEUES
	int	*p_link;	/* pointer to next process */
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
};
