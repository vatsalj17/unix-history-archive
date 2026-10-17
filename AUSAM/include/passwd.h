/*
 *	This file contains all the definitions and declarations to be included
 *	with all the password management routines.
 *
 *	It should be noted that these routines leave one file descriptor
 *	open after the initial call. This may need to be closed.
 */

/*
 *	various file names
 */

#define	ETCPASSWD	"/etc/passwd"	/* the passwd file name */

/*
 *	Various things to do with the passwd file
 */

/*	elements of the string array	*/
#define	LNAME		0	/* the login name element in "pw_strings" */
#define	DIRPATH		1	/* the initial directory path name in ditto */
#define	SHELLPATH	2	/* the initial shell pathname in ditto */
#define	FIRSTNAME	3	/* the users firstname element in ditto */
#define	LASTNAME	4	/* the users lastname element in ditto */
#define	OTHER		5	/* the other element in ditto */

/*	various handy values	*/
#define	LOGLEN		100	/* max size of login name checked */
#define	PWTABENTLEN	4	/* the size of a table entry */
#define PWSLENCNT	6	/* the number of var len string ptrs */
#define	PWTABSIZE	2000	/* the total possible number of users */
#define	PWHASHSIZE	511	/* the hashing function range */
#define	PWTABLEN	((PWTABSIZE+PWHASHSIZE)*PWTABENTLEN)
#define	PWENTNULL	0l	/* password null entry signifier */
#define	USAGEK		1.00964	/* decay parameter for usage */
#define	IWEIGHT		2	/* weight factor for inodes */
#define	PASSWDZ		133	/* max size of passwd for routines that do not alloc */
#define	SYSUSERS	20	/* the max uid for system type uids */
#define	COMMONUSERS	99	/* the max uid for inter system validated users */

/*	the actual shape of a passwd entry	*/
struct pwent
{
	long		pw_next;		/* seek address of next entry in list */
	long		pw_last;		/* seek address of previous entry in list */
/*
 *	The following will be part of a limits structure
 */
	unsigned	pw_uid;		/* integer uid */
	unsigned	pw_shares;	/* number of shares held */
	long		pw_usage;	/* accumulated usage figure */
	unsigned	pw_cmask[CMASKSIZE];	/* class mask of this user */
	unsigned	pw_flags;	/* some handy flags */
	unsigned	pw_dlimit;	/* users disc limit in blocks */
	unsigned	pw_doverflw;	/* amount disc limit may be exceeded
					before encountering masonary */
	char		pw_plimit;	/* users process count limit */
	char		pw_climit;	/* users core limit in K bytes */
/*
 *	The rest does not reside in core
 */
	unsigned	pw_tmask;	/* user terminal group mask */
	char		pw_pword[8];	/* encrypted password */
	long		pw_contime;	/* total accumulated connect time */
	long		pw_cputime;	/* total accumulated cpu time */
	long		pw_extime;	/* time at which last limit struct freed */
				/* ie the last time the user did anything */
	char		pw_warn;		/* number of warnings, eg re disc space */
	char		pw_pages;	/* line printer pages limit in 10s */
/*
 *	The super user will need to have limits of -1 (maximum).
 *
 *	In UNSWDCS's case the LA180s run approx. 4 times slower than our larger
 *	printer. The pages limit could therefore be right shifted 2 places
 *	when used as an LA180 limit.
 *
 *
 *	Now some handy terminal booking things
 *
 */
#ifdef	TERMBOOK
	char		pw_tblim; /* usage limit ,sets transient booking limit */
					/* 0..250 */
	char		pw_tbrate; /* rate of penalty for booking */
					/* 0..14 */
#define	TBCREDIT	2	/* credit for 1 period off , thank you ! */
#endif	TERMBOOK

/*	Now follow a set of counts relating to the lengths of character strings.
 *	The strings, all null terminated, follow sequentially immediately
 *	after the last count. "char *"s are used so that they may be converted
 *	to string pointers in core.
 */
	char		*pw_strings[PWSLENCNT];	/* variable length string pointers */
/*
 *	The pointer array elements point to the following:
 *
 *	0	login name
 *	1	initial directory pathname
 *	2	initial shell pathname
 *	3	users first name
 *	4	users last name
 *	5	"other" general data field
 *
 *
 *	A good sized string buffer (say 200 bytes)
 *	should be provided for "getpwuid" and "getpwlog".
 */
#define	SSIZ		200	/* a goodly size */
/*
 *
 *	Data item	Notes and routines which create or modify entry
 *
 *	pw_next		*pwent routines and pwedit
 *	pw_last		*pwent routines and pwedit
 *	pw_pword[8]	pwedit and passwd
 *	pw_uid		*pwent routines and pwedit
 *	pw_shares	pwedit
 *	pw_usage		init and pwedit
 *	pw_flags		init, login and pwedit
 *
 *		flags have uses as follow
 *
 *		01	DAWARN		one warning given during term. sess.
 *		02	DLIMIT		no further disc output allowed
 *		04	ASYNCKILL	kill all async procs at logoff time
 *		010	ASYNCNICE	force async procs to background at log off
 *		020	CLASSACC	this entry regarded as a class account
 *		040	NOEVICT		this bloke can't be evicted off his terminal
 *		.
 *		.			spares
 *		.
 *		0100000
 *
 *	pw_dlimit	pwedit
 *	pw_contime	init and pwedit
 *	pw_cputime	init and pwedit
 *	pw_extime	init and pwedit
 *	pw_climit	pwedit
 *	pw_plimit	pwedit
 *	pw_warn		login and pwedit
 *	pw_pages	pwedit
 *	pw_strings[]	*pwent routines
 *
 *	
 */
};

/*	some useful 'passwd' defines 	*/

#define	NDLIMWARN	3
/*
 * Define and declaration needed to know the size of a pwentry
 */
struct { struct pwent _pw_sizefinder; };
#define PWENTLEN (sizeof 0->_pw_sizefinder)
