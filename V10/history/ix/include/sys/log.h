/*
 * commands to log system call
 */
#define LOGON		1
#define LOGOFF		2
#define	LOGGET		3
#define	LOGSET		4
#define	LOGFGET		5
#define	LOGFSET		6
#define	LOGPGET		7
#define	LOGPSET		8


/*
 * security logfile records
 */
#define LOGLEN (BUFSIZE+20) 
struct logbuf {
	short len;		/* total length of whole record */
	short pid;
	long slug;		/* transaction number */
	char code;		/* kind of record, see below */
	char mode;		/* sub-kind */
	char colon;		/* ':', aids sync */
	char body[LOGLEN];	/* big enough for namei arg */
};

#define	LOG_USER		0	/* user write to /dev/log, log.c */
#define	LOG_SYSCALL		2	/* trap.c */
#define	LOG_NAMEI		3	/* nami.c */
#define	LOG_EXEC		5	/* sys1.c */
#define	LOG_PLAB		6	/* process label change, sys5.c */
#define	LOG_SLAB		7	/* jlabel.c */
#define LOG_LOGNAME		8	/* sys4.c */
#define LOG_EXTERN		9	/* this priv or that was exercised: */
#define LOG_NOCHK		10	/* ... */
#define LOG_SETLIC		11
#define LOG_SPRIV		12
#define LOG_UAREA		13
#define LOG_LOG			22
#define LOG_FMOUNT		15	/* mount or unmount call */
#define	LOG_SEEK		16
#define LOG_OPEN		17	/* a process gets a new fd */
#define LOG_CLOSE		18	/* a process looses an fd */
#define	LOG_FLAB		19	/* file label change */
#define	LOG_TRACE		20	/* file or uid being traced */
#define	LOG_EXIT		21	/* sys1.c */

/*
 * bits in p_log
 */
#define loggable(x)	(u.u_procp->p_log & (x))
#define	LABAUDIT (1<<0)		/* this sys call is in midst of logging self */

/*
 * Question logging:
 */
#define	LN	(1<< 8)		/* log refs to file names */
#define	LS	(1<< 9)		/* log refs to seek ptrs */
#define	LU	(1<<10)		/* log refs to u_area */
#define LI	(1<<11)		/* log refs to inode contents */
#define LD	(1<<12)		/* log refs to file contents */
#define LP	(1<<13)		/* log processes creation, etc */

/*
 * Status logging:
 */
#define	LL	(1<<15)		/* log explicit label changes */
#define	LA	(1<<16)		/* log ALL label changes (floating) */
#define	LX	(1<<17)		/* log uses of privilege */
#define	LE	(1<<18)		/* log all ELAB returns */
#define	LT	(1<<19)		/* log all uses of traced file or process */


#ifdef KERNEL

extern int lgcnt;
extern struct inode *lg[];

extern long logmask[5];
extern struct logbuf abuf;
extern struct inode alog;
extern seclog();

#define POISON	3
#define ISLOGFILE 128	/* lend/lease from i_pexflag */
#define	LOG_AC(ip)	((ip)->i_pexflag & POISON) /* file poison level */
#define setpoison(ip,x) (ip)->i_pexflag=((x)&POISON)|((ip)->i_pexflag&~POISON)
#endif KERNEL
