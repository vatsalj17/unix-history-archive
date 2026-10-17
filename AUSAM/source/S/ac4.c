#
/*
 *	various subroutines in alphabetical order
 */

#include	<local-system>
#include	"ac.h"
#include	<passwd.h>



/*
 *	bycpu		return reverse order of two user cpu times
 *	=====
 */

int bycpu( p1, p2 )
  struct u_ent **p1, **p2;
{
  if ( (*p1)->ue_cputime < (*p2)->ue_cputime )
	return( 1 );
  return( -1 );
}



/*
 *	bylog		return reverse order of two user logged times
 *	=====
 */

int bylog( p1, p2 )
  struct u_ent **p1, **p2;
{
  if ( (*p1)->ue_logtime < (*p2)->ue_logtime )
	return( 1 );
  return( -1 );
}



/*
 *	bylogons	return reverse order of two user logon counts
 *	========
 */

int bylogons( p1, p2 )
  struct u_ent **p1, **p2;
{
  if ( (*p1)->ue_logons < (*p2)->ue_logons )
	return( 1 );
  return( -1 );
}



/*
 *	byname		return order of two user names
 *	======
 */

int byname( p1, p2 )
  struct u_ent **p1, **p2;
{
	register char	*s1 = (*p1)->ue_namep;
	register char	*s2 = (*p2)->ue_namep;
	register	c;

  while ( c = (*s2++|040) )
	if ( c != (*s1++|040) )
		if ( c > (*--s1|040) )
			return( -1 );	/* 1 < 2 */
		else
			return( c );	/* 1 > 2 */

  return( *s1 );	/* 1 >= 2 */
}



/*
 *	byuid		return order of two user ids
 *	=====
 */

int byuid( p1, p2 )
  struct u_ent **p1, **p2;
{
  if ( (*p1)->ue_uid > (*p2)->ue_uid )
	return( 1 );
  return( -1 );
}



/*
 *	chime	keep trace of time changes - hour-day-month
 *	=====
 */

chime( dp, date_change )
  register struct dtmp *dp;
  register date_change;
{
  do
  {
	{
		register struct tty *ttyp = ttys;

		do
		{
			ttyp->t_flags =& ~NEWHR;

			if ( !date_change && ttyp->t_userp )
				if ( ttyp->t_logintime < lasthour )
				{
					hourp->c_ttys++;
					hourp->c_logtime =+ HOUR;
					if ( working_day )
					{
						hourp->c_wd_ttys++;
						hourp->c_wd_lgtime =+ HOUR;
						ttyp->t_wd_time =+ HOUR;
					}
				}
				else
				{
					hourp->c_logtime =+ hour - ttyp->t_logintime;
					if ( working_day )
					{
						hourp->c_wd_lgtime =+ hour - ttyp->t_logintime;
						ttyp->t_wd_time =+ hour - ttyp->t_logintime;
					}
				}
		}
		while
			( ++ttyp < LAST_TTY );
	}

	if ( !date_change )
	{
		register struct tty_profil	*ttpp;

		for ( ttpp = &ttyprofil[ttylcount-1] ; ttpp >= ttyprofil ; ttpp-- )
		{
			long		uptime = hour - ttpp->tp_ontime;

			ttpp->tp_realtime =+ uptime;
			if ( working_day )
				ttpp->tp_wd_realtime =+ uptime;
			ttpp->tp_ontime = hour;
		}
	}

	if ( !date_change )	/* && sys.st_boottime >= lasthour && sys.st_boottime < hour ) */
	{
		sys.st_uptime =+ hour - sys.st_boottime;
		if ( working_day )
			sys.st_wd_uptime =+ hour - sys.st_boottime;
		sys.st_boottime = hour;
#		ifdef	DEBUG
		if ( flg.y )
		{
			char	s[PTIMSIZ];

			syswarn( "chime: uptime %s\n", ptime( sys.st_uptime, s ) );
		}
#		endif
	}

	lasthour = hour;
	hour =+ HOUR;
	if ( hour > midnight )
	{
		if ( !date_change )
		{
			register struct tty	*ttyp = ttys;
			long			last_day = midnight - DAY;

			do
				if ( ttyp->t_userp )
				{
					if ( ttyp->t_logintime < last_day )
						dayp->d_logtime =+ DAY;
					else
						dayp->d_logtime =+ midnight - ttyp->t_logintime;
					dayp->d_sessions++;
				}
			while
				( ++ttyp < LAST_TTY );
		}

		if ( flg.d )
			print( midnight );
		if ( hour > month )
		{
			if ( !flg.d )
				print( month );
			nextmonth();
#ifdef	DEBUG
			if ( flg.y )
				syswarn( "chime - midnight: %s", ctime( midnight ) );
#endif
		}
		midnight =+ DAY;
		hourp = clock;
		if ( hour > week )
		{
			week =+ WEEK;
			dayp = sennight;
		}
		else
			dayp++;

		dayp->d_weeks++;
#		ifdef	DAY_TRACE

		printf( "DAY %d = %d\n", dayp-sennight, dayp->d_weeks );
#		endif	DAY_TRACE
#		ifdef	TIME_TRACE

		print_times( "chime", TT_HOUR );
#		endif	TIME_TRACE
	}
	else
		hourp++;

	if ( hourp >= &clock[W_HOUR_IN] && hourp < &clock[W_HOUR_OUT]
	    && dayp >= &sennight[W_DAY_IN-1] && dayp < &sennight[W_DAY_OUT-1]
	   )
		working_day = 1;
	else
		working_day = 0;
  }
  while
	( hour <= dp->d_newtime );
}



/*
 *	copystring	allocate new string area and copy string into it
 *	==========
 */

char *copystring( string )
  char *string;
{
	register char	*s, *ns;
	register	i;
	char		*new;
	extern char	*ualloc();

  for ( i = 1 , s = string ; *s++ && (!flg.o || i <= 8) ; i++ );

  new = ns = ualloc( i );

  for ( s = string ; i-- ; )
	*ns++ = *s++;
  if ( flg.o )
	*--ns = 0;

  return( new );
}



/*
 *	cputimes	update system cputimes and return total
 *	========
 */

long cputimes( op )
  register struct otmp *op;
{
	long	t;

  if ( !flg.o )
  {
	sys.st_syscpu =+ op->o_systime;
	sys.st_usercpu =+ op->o_usertime;
	t = op->o_systime + op->o_usertime;
	if ( working_day )
	{
		sys.st_wd_syscpu =+ op->o_systime;
		sys.st_wd_usercpu =+ op->o_usertime;
		hourp->c_wd_cputime =+ t;
	}
	hourp->c_cputime =+ t;
	dayp->d_cputime =+ t;
	return( t );
  }
  else
	return( 0 );
}



/*
 *	fixuptime	bring time variables up to date
 *	=========
 */

fixuptime( dp )
  register struct dtmp *dp;
{
# ifdef	TIME_TRACE
  print_times( "fixuptime", TT_ALL );

# endif	TIME_TRACE
  while ( midnight <= dp->d_newtime )
	midnight =+ DAY;

  hour = midnight - DAY;
  lasthour = hour - HOUR;
  hourp = &clock[-1];

  set_week();
  chime( dp, 1 );
  nextmonth();
# ifdef	TIME_TRACE

  print_times( "fixuptime", TT_ALL );
# endif	TIME_TRACE
}



/*
 *	getuser		if user can't be found in hash table,
 *	=======		allocate a new user structure, and enter it in the table
 */

struct u_ent *getuser( up )
  register struct utmp *up;
{
	register struct u_ent	*ep;
	register		hashval = uhash( up->u_u_id );
	struct pwent		pen;
	char			sbuf[SSIZ];
	extern char		*ualloc(), *copystring();

  if ( !flg.p )
  {
	static struct u_ent	everyman;

	return( &everyman );
  }

  if ( ep = uhasht[hashval] )
  do
	if ( up->u_u_id == ep->ue_uid )
		return( ep );
  while
	( ep = ep->ue_next );

  nu_ents++;
  ep = ualloc( sizeof *ep );

  /*
   *	initialise structure elements in order of declaration
   */

  ep->ue_next = uhasht[hashval];
  uhasht[hashval] = ep;

  pen.pw_uid = ep->ue_uid = up->u_u_id;
  ep->ue_flags = 0;
  ep->ue_logins = 0;
  ep->ue_multilogins = 0;
  ep->ue_logons = 0;

  if ( ( !flg.o && getpwlog( &pen, sbuf, SSIZ ) < 0 )
      || ( flg.o && ( !up->u_u_name[0] || up->u_type != U_TYPE ) )
     )
  {
	warn( "unrecognised user", up );
	ep->ue_namep = copystring( "????????" );
  }
  else
  if ( flg.p )
  {
	ep->ue_namep = copystring( flg.o ? up->u_u_name : pen.pw_strings[LNAME] );
#	ifdef	USER_NAMES
	if ( !flg.o )
	{
		ep->ue_firstname = copystring( pen.pw_strings[FIRSTNAME] );
		ep->ue_lastname = copystring( pen.pw_strings[LASTNAME] );
	}
#	endif	USER_NAMES
  }

  ep->ue_asynctime = 0;
  ep->ue_cputime = 0;
  ep->ue_logtime = 0;

  return( ep );
}



/*
 *	nextmonth	calculate end of current month
 *	=========
 */

nextmonth()
{
	register	i,j;
	register	day = (midnight - 1)/DAY;
	int		f = 0;
	int		mon = 0;
	extern int	dysize(), dmsize[];

# ifdef	TIME_TRACE
  print_times( "nextmonth", TT_MONTH );

# endif	TIME_TRACE
#ifdef	DEBUG
  if ( flg.y )
	syswarn( "nextmonth in hour: %s", ctime( hour ) );

#endif
  for ( i = 1970 ; day >= 0 ; i++ )
  {
	j = dysize( i );
	day =- j;	mon =+ j;
  }
  day =+ j;	mon =- j;

  if ( j == 366 )	/* leap year */
	f++;

  for ( i = 0 ; day >= 0 ; i++ )
  {
	j = dmsize[i];
	if ( f && i == 1 )	/* february */
		j++;
	day =- j;	mon =+ j;
  }

  month = mon*DAY;
#ifdef	DEBUG

  if ( flg.y )
	syswarn( "nextmonth out month: %s", ctime( month ) );
#endif
# ifdef	TIME_TRACE

  print_times( "nextmonth", TT_MONTH );
# endif	TIME_TRACE
}
#ifdef	TIME_TRACE




struct timetable time_table[]
{
  {	"year",		&year		}
 ,{	"month",	&month		}
 ,{	"week",		&week		}
 ,{	"midnight",	&midnight	}
 ,{	"hour",		&hour		}
 ,{	"lasthour",	&lasthour	}
 ,{	"timenow",	&timenow	}
 ,{	"lastprint",	&lastprint	}
 ,{	"boot time",	&sys.st_boottime}
};

#define	NTIMES	((sizeof time_table)/(sizeof time_table[0]))




/*
 *	print_times	print out trace of time values
 *	===========
 */

print_times( s, only )
  char		*s;
  register	only;
{
	register struct timetable	*p;

	if ( flg.y )
	{
		printf( "TIME TRACE in %s:\n", s );

		for ( p = &time_table[only >= NTIMES ? 0 : only] ; p < &time_table[NTIMES] ; p++ )
		{
			printf( "\t%s:%20t%s", p->t_name, ctime( *p->t_timep ) );
			if ( only )
				break;
		}
	}
}
#endif	TIME_TRACE



/*
 *	set_week	calculate end of current week and set "dayp"
 *	========
 */

set_week()
{
	extern int	*localtime();
	register	weekday = localtime( midnight-1 )[6] - 1;

#	ifdef	TIME_TRACE
	print_times( "set_week", TT_WEEK );

#	endif	TIME_TRACE
	if ( weekday < 0 )
		weekday = 6;	/* Sunday */

	dayp = &sennight[weekday];
	dayp->d_weeks++;

	week = midnight + DAY * (6 - weekday);
#	ifdef	TIME_TRACE

	print_times( "set_week", TT_WEEK );
#	endif	TIME_TRACE
}




/*
 *	syswarn		general warning on file 2
 *	=======
 */

syswarn( s, a1, a2, a3, a4, a5 )
{
	register	of;
	extern		fout;

  if ( !flg.n )
  {
	flush();
	of = fout;
	fout = 2;
	printf( s, a1, a2, a3, a4, a5 );
	flush();
	fout = of;
  }
}



/*
 *	ttycount	count a tty if first time it has been used in current hour
 *	========
 */

ttycount( ttyp )
  register struct tty *ttyp;
{
  if ( !(ttyp->t_flags & NEWHR) )
  {
	hourp->c_ttys++;
	if ( working_day )
		hourp->c_wd_ttys++;
	ttyp->t_flags =| NEWHR;
  }
}



/*
 *	ualloc		allocate new area, if none left, attempt garbage collection from hash table
 *	======
 */

char *ualloc( n )
{
	char			*s;
	extern char		*alloc();

loop:
  if ( (s = alloc(n)) == -1 )
  {
#	ifdef	GARBAGE_COLLECTION
	register struct u_ent	*ep, **bp, **hp;

	for ( hp = &uhasht[HASHSIZ-1] ; hp >= uhasht ; --hp )
		if ( ep = *hp )
		{
			bp = hp;
			do
				if ( (ep->ue_flags & (LOGIN|UPDATE)) == 0 )
				{
					nu_ents--;
					*bp = ep->ue_next;
					free( ep->ue_namep );
					free( ep );
					goto loop;
				}
			while
				( bp = &ep->ue_next , ep = ep->ue_next );
		}
#	endif	GARBAGE_COLLECTION

	flg.n = 0;
	syswarn( "%s: not enough memory!\n", name );
	exit( -1 );
  }

  return( s );
}



/*
 *	warn		accounting record related warning
 *	====
 */

warn( s, up )
  char *s;
  register struct utmp *up;
{
	register	c;
	register	type;
	extern char	*ctime();

  if ( flg.o && up->u_u_id == 0xffff )
	return;

  switch ( type = up->u_type )
  {
   case U_TYPE:	type = 'U';	break;
   case O_TYPE:	type = 'O';	break;
   case W_TYPE:	type = 'W';	break;
   case S_TYPE:	type = 'S';	break;
   case D_TYPE:	type = 'D';	break;

   default:	switch ( type =+ ' ' )
		{
		 case 'U': case 'O': case 'W': case 'S': case 'D':
			type =| 040;
		}
  }

  syswarn( "Warning: %s;\n\ttype:'%c' tty:'%c' uid:%-5l  %s"
	   ,s
	   ,type
	   ,(c = up->u_ttyid) ? c : '?'
	   ,up->u_u_id
	   ,ctime( up->u_logintime )
	 );
}




/*
 *	wd_elapsed	return working day time during period
 *	==========
 */

long wd_elapsed( time1, time2 )
  long time1, time2;
{
	register	hour;
	register	wd;
	int		timeout;
	int		day;

  time1 =- w_days_lost * DAY;

  {
	register	*tp;
	extern int	*localtime();

	tp = localtime( time1 );
	if ( tp[6] >= W_DAY_IN && tp[6] < W_DAY_OUT )
	{
		wd = 1;
		if ( tp[2] >= W_HOUR_IN && tp[2] < W_HOUR_OUT )
		{
			timeout = tp[1] * 60 + tp[0];
			time1 =- timeout;
		}
		else
			timeout = 0;
	}
	else
	{
		wd = 0;
		timeout = 0;
	}

	timeout =+ w_days_lost * ((W_HOUR_OUT-W_HOUR_IN)*HOUR);

	hour = tp[2];
	day = tp[6];

	tp = localtime( time2-1 );
	if ( tp[6] >= W_DAY_IN && tp[6] < W_DAY_OUT && tp[2] >= W_HOUR_IN && tp[2] < W_HOUR_OUT )
		timeout =+ HOUR - 1 - (tp[1]*60 + tp[0]);
  }
  {
	register	wh = 0;

	for ( ; time1 < time2 ; time1 =+ HOUR )
	{
		if ( hour >= W_HOUR_IN && hour < W_HOUR_OUT && wd )
			wh++;

		if ( hour++ == 23 )
		{
			hour = 0;

			if ( day++ == 7 )	/* Sunday */
				day = 1;	/* Monday */

			if ( day >= W_DAY_IN && day < W_DAY_OUT )
				wd = 1;
			else
				wd = 0;
		}
	}

	return( wh * 3600L - timeout );
  }
}
