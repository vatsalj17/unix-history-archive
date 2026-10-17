/*
 *	structure for in core limits
 *
 *	used by the system and by programs such as "init" and "login"
 */

struct	lnode
{
	unsigned	l_uid;
	unsigned	l_shares;
	long		l_usage;
	unsigned	l_cmask[CMASKSIZE];
	unsigned	l_flags;
	unsigned	l_dlimit;
	unsigned	l_doverflw;
	char		l_plimit;
	char		l_climit;
	unsigned	l_duse;
	char		l_cusage;
	char		l_refcount;
} ;

/* some useful defines for limits system call */

#define	L_MYLIM		0	/* get own limit structure */
#define	L_OTHLIM	1	/* get other users limits struct */
#define	L_ALLLIM	2	/* get all active limit structures */
#define	L_SETLIM	3	/* set limit4 */

/*	values of limits "flags"	*/

#define	DAWARN		01	/* one warning given during term. sess. */
#define	DLIMIT		02	/* no further disc output allowed */
#define	ASYNCKILL	04	/* kill all async procs at logoff time */
#define	ASYNCNICE	010	/* force async procs to background at log off */
#define CLASSACC 	020	/* regard this entry as a class account */
#define NOEVICT		040	/* this person cannot be evicted */
