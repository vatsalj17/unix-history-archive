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
	char	p_time;
	char	p_cpu;		/* cpu usage for scheduling */
	int	pz_ur0;		/* returned status */
	unsigned p_uid;		/* user id, used to direct tty signals */
	struct tty *p_ttyp;	/* controlling tty */
	int	p_pid;		/* unique process id */
	int	p_ppid;		/* process id of parent */
	long	pz_utime;					   /* fix000 */
	long	pz_stime;	/* times returned from process */  /* fix000 */
	int	*p_link;	/* pointer to next process */
	unsigned p_stl;		/* count of secs left of sleep time */
	unsigned p_rtl;		/* count of secs left of real time limit */
	struct lnode *p_lnode;	/* pointer to limit structure */
};
