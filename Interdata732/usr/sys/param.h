/*
 * UNIX Operating System Kernel - Level 6
 *
 *	Copyright (C) 1975, Bell Telephone Laboratories, Inc.
 *
 * Interdata 7/32, 8/32 Version - Release 1.0
 *
 *	Copyright (C) 1977, 1978, University of Wollongong
 *
 */

/*
 * conditional compilation switches
 */
#define TRACE		/* include debugging trace calls */
#define FPTRAP		/* include simulated floating-point trap
			- must also define SPFPT and/or DPFPT in fptrap.s */
/* #define FPREGS		/* hardware floating-point registers exist
			- must also define FPREGS and/or DPREGS in low.s */

/*
 * tunable variables
 */

#define	NBUF	20		/* size of buffer cache */
#define	NINODE	80		/* number of in core inodes */
#define	NFILE	100		/* number of in core file structures */
#define	NMOUNT	8		/* number of mountable file systems */
#define	NEXEC	5		/* number of simultaneous exec's */
#define	MAXMEM	(128*4)		/* max core per process - first # is Kb */
#define	SSIZE	16		/*** initial stack size (*256 bytes) ***/
#define	SINCR	4		/*** increment of stack (*256 bytes) ***/
#define	NOFILE	15		/* max open files per process */
#define	CANBSIZ	256		/* max size of typewriter line */
#define	CMAPSIZ	100		/* size of core allocation area */
#define	SMAPSIZ	100		/* size of swap allocation area */
#define	NCALL	5		/* max simultaneous time callouts */
#define	NPROC	50		/* max number of processes */
#define	NTEXT	24		/* max number of pure texts */
#define	NCLIST	100		/* max total clist size */
#define	HZ	100		/* Ticks/second of the clock */

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

/*
 * fundamental constants
 * cannot be changed
 */

#define	USIZE	6		/* size of user block (*256) */
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
	char	pad0;		/***/
	char	pad1;		/***/
	char	hibyte;
	char	lobyte;
};

/*
 * structure to access an integer
 */
struct
{
	int	integ;
};

#ifdef	TRACE
#define trace(mask, label, value)	dtrace(mask, label, value)
#else
#define trace(mask, label, value)	/***/
#endif
