/*
 *	data and structure definitions
 */

#include	<wtmp.h>

#define	USER_NAMES			/* put user's name on people printout */
/*
#define	CHISTOGRAM			/* put histogram on hourly averages */
/*
#define	DIAGNOSTIC_REDIRECT		/* allows flag 'x' to redirect diagnostics to file "diagf" */
/*
#define	DEBUG				/* allows flag 'y' to turn on debugging */
/*
#define	GARBAGE_COLLECTION		/* allows for garbage collection in "ualloc" if "sys brk" fails */
/*
#define	TIME_IN_DAYS			/* allows conversion of time printout to days if >300 hours */
/*
#define	TIME_TRACE			/* allows flag 'y' to trace time changes */
/*
#define	DAY_TRACE			/* trace day updates */


/*
 *	FLAGS -- program parameters
 */

struct	flags
{
	char	I;			/* read wtmp file from standard input */
	char	P;			/* specify alternative password file */
	char	d;			/* details by day (default month) */
	char	e;			/* give statistis over 7 days */
	char	f;			/* use form-feeds */
	char	h;			/* give statistics over 24 hours */
	char	i;			/* "individual" contains uid of person */
	char	l;			/* set working days lost */
	char	locking;		/* set if "wtmp" file is locked */
	char	n;			/* no warnings */
	char	o;			/* assume input produced by "cnvtwtmp" */
	char	p;			/* details of people */
	char	s;			/* details of system (default) */
	char	t;			/* details of ttys */
	char	u;			/* tty usage */
	char	w;			/* specify alternative wtmp file */
#	ifdef	DIAGNOSTIC_REDIRECT
	char	x;			/* diagnostic redirection requested */
#	endif
#	ifdef	DEBUG|TIME_TRACE
	char	y;			/* DEBUG */
#	endif
	char	z;			/* (initialised flag) */
}
	flg;

#define	DATE_CHANGE	0200		/* bit set in flag if large -ve date change has occurred */

/*
 *	sort orders for people
 */

int	bycpu();			/* flag 'C' */
int	bylog();			/* flag 'L' */
int	byname();			/* flag 'N' */
int	bylogons();			/* flag 'S' */
int	byuid();			/* flag 'U' */

int	(*porder)();			/*  pointer to chosen order */



/*
 *	times
 */

#define	YEAR		31536000L	/* 365 days ! */
#define	WEEK		604800L
#define	DAY		86400L
#define	HOUR		3600
#define	MINUTE		60

long	year;				/* end of current year */	/* N.B. NOT EXACT AND NOT USED */
long	month;				/* end of current month */
long	week;				/* end of current week */
long	midnight;			/* end of current day */
long	hour;				/* end of hour */
long	lasthour;			/* end of last hour */
long	timenow;			/* current time */
long	lastprint;			/* time of last print out */

#define	W_DAY_IN		1	/* Monday (incl.)	*/
#define	W_DAY_OUT		6	/* Saturday (excl.)	*/
#define	W_HOUR_IN		9	/* 9a.m. (incl.)	*/
#define	W_HOUR_OUT		17	/* 5p.m. (excl.)	*/


/*
 *	U_ENT -- the user structure -- initialised in getuser()
 */

#define	HASHSIZ		251
#define	uhash(A)	((A)%HASHSIZ)

struct	u_ent
{
	struct u_ent	*ue_next;	/* next structure in chain */
	unsigned	ue_uid;
	char		ue_flags;
	char		ue_logins;	/* count of current logins */
	unsigned	ue_multilogins;	/* count of multiple logins */
	unsigned	ue_logons;	/* count of all logons */
	char		*ue_namep;
#	ifdef	USER_NAMES
	char		*ue_firstname;
	char		*ue_lastname;
#	endif	USER_NAMES
	long		ue_asynctime;
	long		ue_cputime;
	long		ue_logtime;
}
	*uhasht[HASHSIZ];		/* the hash table */

/*	ue_flags	*/
#define	LOGIN		01		/* person is currently logged in */
#define	UPDATE		02		/* person has done something in last period */
#define	NEWHR		04		/* data has been collected for this hour */

int	nu_ents;			/* count of user structures */


/*
 *	TTYS -- the ttys
 */

#define	NACTTYS		0177

struct	tty
{
	struct u_ent	*t_userp;	/* non-zero if user logged in */
	unsigned	t_flags;
	unsigned	t_logons;	/* count of all logons */
	long		t_logintime;	/* time user logged in */
	long		t_logtime;	/* total logged */
	unsigned	t_wd_logons;	/* working day logons */
	long		t_wd_time;	/* working day logged */
}
	ttys[NACTTYS+1];			/* the ttys */

#define	BADTTY		(&ttys[NACTTYS])
#define	LAST_TTY	(&ttys[NACTTYS+1])
#define	gtty(A)		(&ttys[((A)->w_ttyid&0177)])	/* expand to check tty */


/*
 *	TTYPROFIL -- ttyprofil[i].tp_realtime = time (i+1) ttys were logged on [ 0 <= i <= NACTTYS-1 ]
 */

struct	tty_profil
{
	long		tp_ontime;	/* start time with this number ot ttys */
	long		tp_realtime;	/* real time with this number of ttys */
	long		tp_wd_realtime;	/* real time with this number of ttys during working day */
}
	ttyprofil[NACTTYS];

int	ttylcount;			/* count of currently logged on ttys */


/*
 *	CLOCK -- hourly clock
 */

struct	Hour
{
	unsigned	c_ttys;		/* ttys used during this hour */
	unsigned	c_logons;	/* logons during this hour */
	unsigned	c_logoffs;	/* logoffs during this hour */
	long		c_logtime;	/* time logged during this hour */
	long		c_cputime;	/* cpu time used this hour */
	long		c_async;	/* async time used this hour */
	unsigned	c_wd_ttys;	/* ttys used during this hour */
	unsigned	c_wd_logons;	/* logons during this hour */
	unsigned	c_wd_lgoffs;	/* logoffs during this hour */
	long		c_wd_lgtime;	/* time logged during this hour */
	long		c_wd_cputime;	/* cpu time used this hour */
	long		c_wd_async;	/* async time used this hour */
}
	clock[24],			/* the clock */
	*hourp;				/* the hour hand */

#ifdef	CHISTOGRAM
#define	CHISTCOLS	10		/* number of columns to be used by histogram */
#endif	CHISTOGRAM


/*
 *	SENNIGHT -- 7 days of the week
 */

struct	Day
{
	unsigned	d_weeks;	/* number of weeks at this day */
	unsigned	d_sessions;	/* terminal sessions this day */
	long		d_logtime;	/* time logged this day */
	long		d_cputime;	/* cpu time used this day */
	long		d_async;	/* async time used this day */
}
	sennight[7],			/* the week ( Monday = 0 ) */
	*dayp;				/* day of the week */


/*
 *	SYS -- system totals
 */

struct	sys_ent
{
	long	st_boottime, st_uptime; /* uptime figures */
	long	st_syscpu, st_usercpu;	/* total cputime figures */
	long	st_async;
	unsigned st_sessions;	
	long	st_logtime;
	long	st_wd_uptime;		/* working day uptime */
	long	st_wd_syscpu;
	long	st_wd_usercpu;
	long	st_wd_async;
	unsigned st_wd_sessions;
	long	st_wd_logtime;
	int	st_boots;		/* count of reboots */
	int	st_crashes;
}
	sys;


/*
 *	miscellany
 */

#define	PTIMSIZ		8		/* string size produced by ptime() */

char	*name;				/* program invoked name */

#define	max(A,B)	(((A)>(B))?A:B)

int	working_day;			/* TRUE if 9-5, Monday to Friday */

int	w_days_lost;			/* days supposedly lost from wtmp file */

unsigned individual;			/* uid of person if flg.i */


#ifdef	TIME_TRACE
struct	timetable
{
	char		*t_name;
	long		*t_timep;
}
	time_table[];

#define	TT_ALL		0
#define	TT_YEAR		0
#define	TT_MONTH	1
#define	TT_WEEK		2
#define	TT_MIDNIGHT	3
#define	TT_HOUR		4
#define	TT_LASTHOUR	5
#define	TT_TIMENOW	6
#define	TT_LASTPRINT	7

#endif	TIME_TRACE
