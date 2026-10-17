#
/*
 *	routines associated with wtmp records
 */

#include	"ac.h"


/*
 *	login
 *	=====
 */

login( up )
  register struct utmp *up;
{
	register struct u_ent	*ep;
	register struct tty	*ttyp;
	extern struct u_ent	*getuser();

  ep = getuser( up );

  if ( ep->ue_flags & LOGIN )
	ep->ue_multilogins++;
  else
	ep->ue_flags =| LOGIN;
  ep->ue_logins++;
  ep->ue_logons++;

  ttyp = gtty( up );
  if ( ttyp->t_userp )
  {
	if ( ttyp != BADTTY )
		warn( "tty multi-logon", up );
	logoff( ttyp, up );
  }
  ttyp->t_userp = ep;
  ttyp->t_logintime = up->u_logintime;
  ttyp->t_logons++;

  hourp->c_logons++;
  if ( working_day )
  {
	hourp->c_wd_logons++;
	ttyp->t_wd_logons++;
  }
  ttycount( ttyp );
  ttyprofil[ttylcount++].tp_ontime = up->u_logintime;
}



/*
 *	logout
 *	======
 */

logout( op )
  register struct otmp *op;
{
	register struct u_ent	*ep;
	register struct tty	*ttyp;
	extern long		cputimes();

  ttyp = gtty( op );
  if ( !(ep = ttyp->t_userp) )
  {
	if ( ttyp != BADTTY && op->o_uid )
		warn( "unexpected logoff", op );
	ep = getuser( op );
	ttycount( ttyp );
  }
  else
  {
	logoff( ttyp, op );
	if ( ep->ue_uid != op->o_uid && flg.p )
	{
		unsigned	uid = op->o_uid;

		warn( "wrong user logging off", op );
		op->o_uid = ep->ue_uid;
		warn( "should be", op );
		op->o_uid = uid;
		ep = getuser( op );
	}
  }

  ep->ue_cputime =+ cputimes( op );
  ep->ue_flags =| UPDATE;
}



/*
 *	logoff	 log user off a particular tty
 *	======
 */

logoff( ttyp, op )
  register struct tty *ttyp;
  register struct otmp *op;
{
	register struct u_ent	*ep = ttyp->t_userp;
	long			logtime;

  logtime = op->o_logofftime - ttyp->t_logintime;
  ttyp->t_logtime =+ logtime;
  ttyp->t_userp = 0;

  if ( ttyp->t_logintime < lasthour )
  {
  	hourp->c_logtime =+ op->o_logofftime - lasthour;
	if ( working_day )
	{
		hourp->c_wd_lgtime =+ op->o_logofftime - lasthour;
		ttyp->t_wd_time =+ op->o_logofftime - lasthour;
	}
  }
  else
  {
  	hourp->c_logtime =+ logtime;
	if ( working_day )
	{
		hourp->c_wd_lgtime =+ logtime;
		ttyp->t_wd_time =+ logtime;
	}
  }
  hourp->c_logoffs++;
  if ( working_day )
	hourp->c_wd_lgoffs++;

  if ( ttyp->t_logintime < (midnight - DAY) )
	dayp->d_logtime =+ op->o_logofftime - (midnight - DAY);
  else
	dayp->d_logtime =+ logtime;
  dayp->d_sessions++;

  sys.st_logtime =+ logtime;
  sys.st_sessions++;
  if ( working_day )
  {
	sys.st_wd_logtime =+ logtime;
	sys.st_wd_sessions++;
  }

  ttycount( ttyp );

  if ( --ttylcount < 0 )
  {
	warn( "-ve ttys ?", op );
	ttylcount = 0;
  }
  ttyprofil[ttylcount].tp_realtime =+ op->o_logofftime - ttyprofil[ttylcount].tp_ontime;
  if ( working_day )
	ttyprofil[ttylcount].tp_wd_realtime =+ op->o_logofftime - ttyprofil[ttylcount].tp_ontime;

  if ( --ep->ue_logins <= 0 )
  {
	if ( ep->ue_logins < 0 )	/* program bug ? */
	{
		warn( "user multi-logoff - program bug?", op );
		ep->ue_logins = 0;
	}
	ep->ue_flags =& ~LOGIN;
  }
  ep->ue_logtime =+ logtime;
  ep->ue_flags =| UPDATE;
}



/*
 *	async
 *	=====
 */

async( wp )
  register struct wtmp *wp;
{
	register struct u_ent	*ep;
	extern struct u_ent	*getuser();
	long			cput;
	extern long		cputimes();

  ep = getuser( wp );
  cput = cputimes( wp );
  sys.st_async =+ cput;
  if ( working_day )
  {
	sys.st_wd_async =+ cput;
	hourp->c_wd_async =+ cput;
  }
  hourp->c_async =+ cput;
  dayp->d_async =+ cput;
  ep->ue_asynctime =+ cput;
  ep->ue_cputime =+ cput;
  ep->ue_flags =| UPDATE;
}



/*
 *	update
 *	======
 */

update( dp )
  register struct dtmp *dp;
{
	long			change;

# ifdef	DEBUG
  if ( flg.y )
	warn( "update trace", dp );

# endif	DEBUG
# ifdef	TIME_TRACE
  print_times( "update", TT_ALL );

# endif	TIME_TRACE
  if ( !flg.z )
  {
	init( dp );
	return;
  }

  change = dp->d_newtime - dp->d_oldtime;

# ifdef	DEBUG
  if ( flg.y )
  {
	char	s[PTIMSIZ];

	syswarn( "update: boottime %s\t%screased by %s\n"
		,ctime( sys.st_boottime )
		,change < 0 ? "de" : "in"
		,ptime( change < 0 ? -change : change, s )
	);
  }
# endif

# ifdef	DEBUG
  if ( flg.y )
	syswarn( "update: boottime now %s", ctime( sys.st_boottime ) );
# endif

  if ( change < 0 )
  {
	do
		midnight =- DAY;
	while
		( midnight > dp->d_newtime );
	midnight =+ DAY;
	hour = midnight;

	do
		hour =- HOUR;
	while
		( hour > dp->d_newtime );

	if ( lasthour > hour )	/* hourly statistics ruined ! */
	{
		if ( flg.h )
			flg.h =| DATE_CHANGE;
		warn( "large negative date change", dp );
		set_week();
		nextmonth();
	}

	lasthour = hour;
	hour =+ HOUR;

	hourp = &clock[23 - (midnight - hour)/HOUR];
  }
  else
	if ( change >= DAY )
	{
		if ( dp->d_oldtime > lastprint && ( flg.d || dp->d_newtime > month ) )
		{
			print( dp->d_oldtime );
			lastprint =+ change;
		}
		if ( flg.h )
			flg.h =| DATE_CHANGE;
		fixuptime( dp );
	}
	else
		if ( hour <= dp->d_newtime )
			chime( dp, 1 );

  {
	register struct tty	*ttyp = ttys;

#	ifdef	DEBUG
	if ( flg.y )
		syswarn( "update in: hour %s", ctime( hour ) );
#	endif
	do
		if ( ttyp->t_userp )
			ttyp->t_logintime =+ change;
	while
		( ++ttyp < LAST_TTY );
  }
  {
	register struct tty_profil	*ttypp = ttyprofil;
	register			i = ttylcount;

	while ( i-- )
		(ttypp++)->tp_ontime =+ change;
  }

  sys.st_boottime =+ change;

#ifdef	DEBUG
  if ( flg.y )
	syswarn( "update out: hour %s\n", ctime( hour ) );

#endif
  timenow = dp->d_newtime;
# ifdef	TIME_TRACE

  print_times( "update", TT_ALL );
# endif	TIME_TRACE
}



/*
 *	boot
 *	====
 */

boot( sp )
  register struct stmp *sp;
{
	register struct tty	*ttyp = ttys;
	register		users = 0;
	unsigned		uid = 0;
	long			boottime = sp->s_boottime;

# ifdef	TIME_TRACE
  print_times( "boot", TT_ALL );

# endif	TIME_TRACE
  if ( !flg.z )
	init( sp );
  else
	if ( hour <= sp->s_boottime )
		chime( sp, 0 );

  sp->s_boottime = timenow;	/* if there has been a disaster, do the best we can,
				   and wait for "update" to fix the world */
# ifdef	DEBUG
  if ( flg.y )
	warn( "boot trace", sp );
# endif

  do
	if ( ttyp->t_userp )
	{
		if ( !uid )
			uid = ttyp->t_userp->ue_uid;
		users++;
		logoff( ttyp, sp );
	}
  while
	( ++ttyp < LAST_TTY );

  /*
  **	if only root logged on when system crashed - assume intentional !
  */
  if ( uid || (!flg.p && (users > 1)) )
  {
	syswarn( "CRASH: %d user%slogged on when system crashed on %s"
		,users
		,users > 1 ? "s " : " "
		,ctime( sp->s_boottime )
	       );
	sys.st_crashes++;
  }
  else
	sys.st_boots++;

  if ( working_day )
	sys.st_wd_uptime =+ sp->s_boottime - sys.st_boottime;
  sys.st_uptime =+ sp->s_boottime - sys.st_boottime;

  if ( boottime < timenow )
  {
	sp->d_newtime = boottime;
	sp->d_oldtime = timenow;
	update( sp );
  }

  sys.st_boottime = boottime;

  timenow = boottime;
# ifdef	DEBUG
  if ( flg.y )
  {
	char	s[PTIMSIZ];

	syswarn( "boot: uptime %s\n\n", ptime( sys.st_uptime, s ) );
  }
# endif
# ifdef	TIME_TRACE

  print_times( "boot", TT_ALL );
# endif	TIME_TRACE
}
