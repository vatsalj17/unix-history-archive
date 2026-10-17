/*
 *	/usr/include/utmp.h
 */

/*
 *	utmp structure for logins
 *
 *	written into	/etc/utmp
 *		and	/usr/adm/wtmp
 *		by	"login"
 *			( utmp entry cleared by "init" )
 */
#define	U_TYPE	'\n'
struct	utmp
{
  char		u_type;
  char		u_ttyid;
  unsigned	u_u_id;
  long		u_logintime;
  char		u_u_name[8];
};
#define	UTMPSIZ	16

/*
 *	otmp structure for logoffs
 *
 *	written into	/usr/adm/wtmp
 *		by	"init"
 */
#define	O_TYPE	0
struct	otmp
{
  char		o_type;
  char		o_ttyid;
  unsigned	o_uid;
  long		o_logofftime;
  long		o_usertime;
  long		o_systime;
};
#define	OTMPSIZ	16

/*
 *	wtmp structure for process termination
 *
 *	written into	/usr/adm/wtmp
 *		by	"init"
 */
#define	W_TYPE	1
struct	wtmp
{
  char		w_type;
  char		w_ttyid;
  unsigned	w_uid;
  long		w_finishtime;
  long		w_usertime;
  long		w_systime;
};
#define	WTMPSIZ	16

/*
 *	stmp structure for system boot
 *
 *	written into	/usr/adm/wtmp
 *		by	"init"
 */
#define	S_TYPE	2
struct	stmp
{
  char		s_type;
  char		s_fill1[3];
  long		s_boottime;
  char		s_fill2[8];
};
#define	STMPSIZ	16

/*
 *	dtmp structure for date changes
 *
 *	written into	/usr/adm/wtmp
 *		by	"date"
 */
#define	D_TYPE	3
struct	dtmp
{
  char		d_type;
  char		d_fill1[3];
  long		d_newtime;
  long		d_oldtime;
  char		d_fill2[4];
};
#define	DTMPSIZ	16

/*
 *	sutmp structure for "su" command
 *
 *	written into	/usr/adm/wtmp
 *		by	"su"
 */
#define	SU_TYPE	4
struct	sutmp
{
  char		su_type;
  char		su_fill1;
  unsigned	su_newuid;
  long		su_usetime;
  char		su_fill2[6];
  unsigned	su_olduid;
};
#define	SUTMPSIZ	16
