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
#define	L_SETLIM	3	/* set limits */

/*	values of limits "flags"	*/

#define	DAWARN		01	/* one warning given during term. sess. */
#define	DLIMIT		02	/* no further disc output allowed */
#define	ONCELOGIN	04	/* one login, nice async procs to background at log off */
#define	LOGGEDIN	010	/* once only login account is logged in */
#define	CLASSACC	020	/* regard this entry as a class account */
#define	NOEVICT		040	/* this person cannot be evicted */
#define	NOLOGIN		0100	/* this person may not log on */
#define	USENET		0200	/* can use the network stuff */
#define	ADMIN		0400	/* system administrator priviledges */
#define	TUTOR		01000	/* tutor priviledges */
#define	GROUPACC	02000	/* regard this entry as a group account */
#define	CURCLASS	04000	/* this account is in current active class */
#define	ASYNCKILL	010000	/* kill all async procs at logoff time */
#define	NEWPASSWD	020000	/* force a password change before login */
#define	CONNECTLIM	040000	/* connect time limits in force */
#define	TTYOPTS		0100000	/* allow people to set high baudrates etc */

/*	disk usage calculations		*/

#define	IWEIGHT		2		/* disk units weight factor for inodes */

/*	background nice level		*/

#define	BACKGROUND	5
