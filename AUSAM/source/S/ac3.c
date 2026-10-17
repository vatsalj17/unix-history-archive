#
/*
 *	the print routines
 */

#include	<local-system>
#include	"ac.h"

#define	FF	putchar( flg.f ? '\014' : '\n' )



/*
 *	print	the monolithic print routine - all data printed here are zeroed
 *	=====
 */

print( now )
  long now;
{
	char		sa1[PTIMSIZ], sa2[PTIMSIZ], sa3[PTIMSIZ], sa4[PTIMSIZ];
	char		sa5[PTIMSIZ], sa6[PTIMSIZ], sa7[PTIMSIZ], sa8[PTIMSIZ];
	extern char	*ptime(), *itoa();
	static char	**day[]
	{
		"Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
		"Saturday", "Sunday"
	};

# ifdef	DEBUG
  if ( flg.y )
	syswarn( "\nPRINT: %s\n", ctime( now ) );
# endif

  if ( now <= lastprint )
	return;

  /*
   *	time range
   */

  if ( (now - lastprint + DAY-1)/DAY > 1 )
  {
	pday( lastprint );
	printf( " - " );
  }
  else
	printf( "%s ", day[dayp - sennight] );

  pday( now-1 );
  putchar( '\n' );

  if ( w_days_lost )
  {
	printf( "\nNote: %d working days lost this month\n", w_days_lost );
	lastprint =+ w_days_lost * DAY;	/** GLITCHY - also in "wd_elapsed()" **/
  }

  /*
   *	people
   */

  if ( flg.p )
  {
	register struct u_ent	*ep, **hp;
	struct u_ent		**people = ualloc( nu_ents * (sizeof *people) );
	int			persons = 0;
	unsigned		logons = 0;
	long			logtotal = 0;
	long			cputotal = 0;
	long			asynctotal = 0;
	float			f;

	printf( "\n  PEOPLE\nlogons  logged  average    cpu    %%cpu   async   name\n" );

	for ( hp = uhasht ; hp < &uhasht[HASHSIZ] ; hp++ )
		if ( ep = *hp )
			do
				if ( ep->ue_flags & (UPDATE|LOGIN) )
				{
					if ( ep->ue_flags & LOGIN )
					{
						register struct tty	*ttyp;

						for ( ttyp = ttys ; ttyp < LAST_TTY ; ttyp++ )
							if ( ep == ttyp->t_userp )
								ep->ue_logtime =+ now - max( lastprint, ttyp->t_logintime );
					}

					people[persons++] = ep;
				}
			while
				( ep = ep->ue_next );

	qsort( people, persons, (sizeof *people), porder );

	for ( hp = people ; hp < &people[persons] ; hp++ )
	{
		ep = *hp;

		if ( !flg.i || ep->ue_uid == individual )
		{
#			ifdef	USER_NAMES
			printf( "%5l  %s  %s  %s %5.1f  %s  %s%61t%s %s\n"
#			else	USER_NAMES
			printf( "%5l  %s  %s  %s %5.1f  %s  %s\n"
#			endif	USER_NAMES
				,ep->ue_logons
				,ptime( ep->ue_logtime, sa1 )
				,ptime( ep->ue_logons ? ep->ue_logtime/ep->ue_logons : 0L, sa2 )
				,ptime( ep->ue_cputime/HZ, sa3 )
				,ep->ue_logtime ? ( (f = (100. * ep->ue_cputime)/(ep->ue_logtime * HZ.)) > 999.9 ? 0. : f ) : 0.
				,ptime( ep->ue_asynctime/HZ, sa4 )
				,porder == byuid ? itoa( ep->ue_uid ) : ep->ue_namep
#				ifdef	USER_NAMES
				,ep->ue_firstname
				,ep->ue_lastname
#				endif	USER_NAMES
			      );
			logons =+ ep->ue_logons;
			logtotal =+ ep->ue_logtime;
			cputotal =+ ep->ue_cputime;
			asynctotal =+ ep->ue_asynctime;
			ep->ue_logons = 0;
			ep->ue_logtime = 0;
			ep->ue_cputime = 0;
			ep->ue_asynctime = 0;
			ep->ue_flags =& ~UPDATE;
		}
	}

	free( people );

	if ( !flg.i )
		printf( "------------------------------------------------------------\n%5l  %s  %s  %s %5.1f  %s  %d\n"
			,logons
			,ptime( logtotal, sa1 )
			,ptime( logons ? logtotal/logons : 0L, sa2 )
			,ptime( cputotal/HZ, sa3 )
			,logtotal ? ( (f = (100. * cputotal)/(logtotal * HZ.)) > 999.9 ? 0. : f ) : 0.
			,ptime( asynctotal/HZ, sa4 )
			,persons
		      );

	FF;
  }

  /*
   *	ttys
   */

  if ( flg.t )
  {
	register struct tty	*ttyp;
	register struct u_ent	*ep;
	register unsigned	nttys = 0;
	unsigned		logons = 0;
	long			logtotal = 0;
	unsigned		wd_logons = 0;
	long			wd_logtotal = 0;

	printf( "\n  TTYS%30tworking day\ntty logons  logged  average  logons  logged  average\n" );

	for ( ttyp = ttys ; ttyp < LAST_TTY ; ttyp++ )
		if ( (ep = ttyp->t_userp) || ttyp->t_logtime )
		{
			long	logtime = ttyp->t_logtime;

			if ( ep )
				logtime =+ now - max( lastprint, ttyp->t_logintime );

			nttys++;
			printf( " %c  %5l  %s  %s  %5l  %s  %s\n"
				,ttyp - ttys
				,ttyp->t_logons
				,ptime( logtime, sa1 )
				,ptime( ttyp->t_logons ? logtime/ttyp->t_logons : 0L, sa2 )
				,ttyp->t_wd_logons
				,ptime( ttyp->t_wd_time, sa3 )
				,ptime( ttyp->t_wd_logons ? ttyp->t_wd_time/ttyp->t_wd_logons : 0L, sa4 )
			      );
			logons =+ ttyp->t_logons;
			logtotal =+ logtime;
			wd_logons =+ ttyp->t_wd_logons;
			wd_logtotal =+ ttyp->t_wd_time;
			ttyp->t_logons = 0;
			ttyp->t_logtime = 0;
			ttyp->t_wd_logons = 0;
			ttyp->t_wd_time = 0;
		}

	printf( "----------------------------------------------------\n%2d  %5l  %s  %s  %5l  %s  %s\n"
		,nttys
		,logons
		,ptime( logtotal, sa1 )
		,ptime( logons ? logtotal/logons : 0L, sa2 )
		,wd_logons
		,ptime( wd_logtotal, sa3 )
		,ptime( wd_logons ? wd_logtotal/wd_logons : 0L, sa4 )
	      );

	FF;
  }

  /*
  **	tty profile
  */

  if ( flg.u )
  {
	register struct tty_profil	*ttp;
	register			i;
	extern long			wd_elapsed();
	long				wdtotal = wd_elapsed( lastprint, now );
	long				realtotal = now - lastprint;
	long				ex_realtot = 0;
	long				ex_wdtot = 0;

	printf( "\n  TTY PROFILE\n%10ttotal%20tworking day\nttys  incl%% excl%%  incl%% excl%%\n" );

	for ( ttp = &ttyprofil[ttylcount-1] ; ttp >= ttyprofil ; ttp-- )
	{
		long		uptime = now - ttp->tp_ontime;

		ttp->tp_realtime =+ uptime;
		if ( working_day )
			ttp->tp_wd_realtime =+ uptime;
		ttp->tp_ontime = now;
	}

	for ( i = 0 , ttp = ttyprofil ; ttp->tp_realtime ; )
	{
		ttp++;	i++;
	}

	for ( ; --ttp >= ttyprofil ; i-- )
	{
		printf( "%3d   %5.1f %5.1f  %5.1f %5.1f\n"
			,i
			,(100. * ttp->tp_realtime)/realtotal
			,(100. * (ttp->tp_realtime - ex_realtot))/realtotal
			,wdtotal ? (100. * ttp->tp_wd_realtime)/wdtotal : 0.
			,wdtotal ? (100. * (ttp->tp_wd_realtime - ex_wdtot))/wdtotal : 0.
		      );
		ex_realtot = ttp->tp_realtime;
		ttp->tp_realtime = 0;
		ex_wdtot = ttp->tp_wd_realtime;
		ttp->tp_wd_realtime = 0;
	}

	printf( "  0   100.0 %5.1f  100.0 %5.1f\n"
		,(100. * (realtotal - ex_realtotal))/realtotal
		,wdtotal ? (100. * (wdtotal - ex_wdtot))/wdtotal : 0.
	      );

	FF;
  }

  /*
   *	hourly averages
   */

  if ( flg.h )
  {
	register		i;
	register struct Hour	*cp;
	unsigned		days = (now - lastprint + DAY-1)/DAY;
	unsigned		w_days;
	extern long		wd_elapsed();
#	ifdef	CHISTOGRAM
	long			logmax;
	long			scale;
#	endif	CHISTOGRAM
	static char		header[]	"ttys logons logoffs  logged average   cpu     async";

	w_days = (wd_elapsed( lastprint, now ) + (3600L * (W_HOUR_OUT-W_HOUR_IN) - 1))
			/ (3600L * (W_HOUR_OUT-W_HOUR_IN));
	if ( w_days == 0 )
		w_days = 1;	/* because of divide by zero, ho hum */

	if ( now != midnight )
	{
		register struct tty	*ttyp;

		for ( ttyp = ttys ; ttyp < LAST_TTY ; ttyp++ )
			if ( ttyp->t_userp )
				if ( ttyp->t_logintime < lasthour )
				{
					hourp->c_ttys++;
					hourp->c_logtime =+ now - lasthour;
					if ( working_day )
					{
						hourp->c_wd_ttys++;
						hourp->c_wd_lgtime =+ now - lasthour;
					}
				}
				else
				{
					hourp->c_logtime =+ now - ttyp->t_logintime;
					if ( working_day )
						hourp->c_wd_lgtime =+ now - ttyp->t_logintime;
				}
	}

	if ( flg.h & DATE_CHANGE )
	{
		flg.h =& ~DATE_CHANGE;
		printf( "\n*** large date change during this period! ***\n" );
	}

	printf( "\n  HOURLY AVERAGES:%63t(and for working days only)\ntime  : %s   %s\n", header, header );

#	ifdef	CHISTOGRAM
	for ( logmax = CHISTCOLS , cp = clock ; cp < &clock[24] ; cp++ )
		if ( cp->c_logtime > logmax )
			logmax = cp->c_logtime;

	scale = logmax/CHISTCOLS;

#	endif	CHISTOGRAM
	for ( i = 0, cp = clock ; i < 24 ; i++, cp++ )
	{
#		ifdef	CHISTOGRAM
		int	j;

#		endif	CHISTOGRAM
		printf( "%2d-%-2d : %4.1f   %4.1f   %4.1f  %s %s %s  %s  %4.1f   %4.1f   %4.1f  %s %s %s  %s\n"
			,i ,i+1
			,(cp->c_ttys * 1.)/days
			,(cp->c_logons * 1.)/days
			,(cp->c_logoffs * 1.)/days
			,ptime( cp->c_logtime/days, sa1 )
			,ptime( cp->c_logoffs ? cp->c_logtime/cp->c_logoffs : 0L, sa2 )
			,ptime( cp->c_cputime/(days * HZ), sa3 )
			,ptime( cp->c_async/(days * HZ), sa4 )
			,(cp->c_wd_ttys * 1.)/w_days
			,(cp->c_wd_logons * 1.)/w_days
			,(cp->c_wd_lgoffs * 1.)/w_days
			,ptime( cp->c_wd_lgtime/w_days, sa5 )
			,ptime( cp->c_wd_lgoffs ? cp->c_wd_lgtime/cp->c_wd_lgoffs : 0L, sa6 )
			,ptime( cp->c_wd_cputime/(w_days * HZ), sa7 )
			,ptime( cp->c_wd_async/(w_days * HZ), sa8 )
		      );

#		ifdef	CHISTOGRAM
		for ( j = cp->c_logtime/scale + 1 ; --j ; )
			putchar( '-' );
		printf( "*\n" );

#		endif	CHISTOGRAM
		cp->c_ttys = 0;
		cp->c_logons = 0;
		cp->c_logoffs = 0;
		cp->c_logtime = 0;
		cp->c_cputime = 0;
		cp->c_async = 0;
		cp->c_wd_ttys = 0;
		cp->c_wd_logons = 0;
		cp->c_wd_lgoffs = 0;
		cp->c_wd_lgtime = 0;
		cp->c_wd_cputime = 0;
		cp->c_wd_async = 0;
	}

	if ( days > 1 )
		printf( "\nNote: period covers %d days.\n", days );

	FF;
  }

  /*
  **	daily averages
  */

  if ( flg.e )
  {
	register		i;
	register struct Day	*dp;

	if ( now != midnight )
	{
		register struct tty	*ttyp;
		long			last_day = midnight - DAY;

		for ( ttyp = ttys ; ttyp < LAST_TTY ; ttyp++ )
			if ( ttyp->t_userp )
			{
				if ( ttyp->t_logintime < last_day )
					dayp->d_logtime =+ now - last_day;
				else
					dayp->d_logtime =+ now - ttyp->t_logintime;
				dayp->d_sessions++;
			}
	}

	printf( "\n  DAILY AVERAGES:\n%12tsessions  logged average     cpu   async  weeks\n" );

	for ( i = 0 , dp = sennight ; i < 7 ; i++ , dp++ )
	{
		printf( "%s%12t%8l %s %s %s %s%7d\n"
			,day[i]
			,dp->d_sessions/dp->d_weeks
			,ptime( dp->d_weeks ? dp->d_logtime/dp->d_weeks : 0L, sa1 )
			,ptime( dp->d_sessions ? dp->d_logtime/dp->d_sessions : 0L, sa2 )
			,ptime( dp->d_weeks ? dp->d_cputime/(dp->d_weeks*HZ) : 0L, sa3 )
			,ptime( dp->d_weeks ? dp->d_async/(dp->d_weeks*HZ) : 0L, sa4 )
			,dp->d_weeks
		      );

		dp->d_weeks = 0;
		dp->d_sessions = 0;
		dp->d_logtime = 0;
		dp->d_cputime = 0;
		dp->d_async = 0;
	}

	FF;
  }

  /*
   *	system
   */

  if ( flg.s )
  {
	long		wd_time, wd_elapsed();
	static char	*logged_s	"  Access\n%l terminal sessions logged %s (average: %s).\n";
	static char	*cput_s		"  Cpu Usage\n%s cpu time: %s system, %s user - (%.1f%% idle);\n";
	static char	*async_s	" of which %.1f%% was consumed by asynchronous processes,\n and %.1f%% was spent in the system.\n";
	long		uptime = now - sys.st_boottime;

	sys.st_boottime = now;
	sys.st_uptime =+ uptime;
	if ( working_day )
		sys.st_wd_uptime =+ uptime;

	printf( "\n  SYSTEM\nAvailable: %s (%.1f%% uptime"
		,ptime( sys.st_uptime, sa1 )
		,(100. * sys.st_uptime)/(now - lastprint)
	      );
	if ( sys.st_boots || sys.st_crashes )
	{
		printf( " with " );
		if ( sys.st_boots )
		{
			printf( "%d reboo%s"
				,sys.st_boots
				,(sys.st_boots>1) ? "ts" : "t"
			      );
			sys.st_boots = 0;
			if ( sys.st_crashes )
				printf( " and " );
		}
		if ( sys.st_crashes )
		{
			printf( "%d cras%s"
				,sys.st_crashes
				,(sys.st_crashes > 1) ? "hes" : "h"
			      );
			sys.st_crashes = 0;
		}
	}
	printf( ").\n" );

	printf( logged_s
		,sys.st_sessions
		,ptime( sys.st_logtime, sa1 )
		,ptime( sys.st_sessions ? sys.st_logtime/sys.st_sessions : 0L, sa2 )
	      );
	sys.st_sessions = 0;
	sys.st_logtime = 0;

	if ( !flg.o )
	{
		long	totcpu = sys.st_syscpu + sys.st_usercpu;

		printf( cput_s
			,ptime( totcpu/HZ, sa1 )
			,ptime( sys.st_syscpu/HZ, sa2 )
			,ptime( sys.st_usercpu/HZ, sa3 )
			,sys.st_uptime ? 100. - (totcpu*100.)/(sys.st_uptime*HZ) : 0.
		      );
	
		printf( async_s
			,totcpu ? (100. * sys.st_async)/totcpu : 0.
			,totcpu ? (100. * sys.st_syscpu)/totcpu : 0.
		      );
	
		sys.st_syscpu = 0;
		sys.st_usercpu = 0;
		sys.st_async = 0;
	}

	sys.st_uptime = 0;

	if ( wd_time = wd_elapsed( lastprint, now ) )
	{
		printf( "\n  WORKING DAY (%s - %s, %2d:00-%2d:00)\nAvailable: %s ( %.1f%% uptime ).\n"
			,day[W_DAY_IN-1]
			,day[W_DAY_OUT-2]
			,W_HOUR_IN
			,W_HOUR_OUT
			,ptime( sys.st_wd_uptime, sa1 )
			,(100.* sys.st_wd_uptime)/wd_time
		      );

		printf( logged_s
			,sys.st_wd_sessions
			,ptime( sys.st_wd_logtime, sa1 )
			,ptime( sys.st_wd_sessions ? sys.st_wd_logtime/sys.st_wd_sessions : 0L, sa2 )
		      );
		sys.st_wd_sessions = 0;
		sys.st_wd_logtime = 0;

		if ( !flg.o )
		{
			long	totcpu = sys.st_wd_syscpu + sys.st_wd_usercpu;

			printf( cput_s
				,ptime( totcpu/HZ, sa1 )
				,ptime( sys.st_wd_syscpu/HZ, sa2 )
				,ptime( sys.st_wd_usercpu/HZ, sa3 )
				,sys.st_wd_uptime ? 100. - (totcpu*100.)/(sys.st_wd_uptime*HZ) : 0.
			      );
			printf( async_s
				,totcpu ? (100. * sys.st_wd_async)/totcpu : 0.
				,totcpu ? (100. * sys.st_wd_syscpu)/totcpu : 0.
			      );

			sys.st_wd_syscpu = 0;
			sys.st_wd_usercpu = 0;
			sys.st_wd_async = 0;
		}

		sys.st_wd_uptime = 0;
	}

	{
		register struct tty	*ttyp = ttys;
		register		users = 0;

		do
			if ( ttyp->t_userp )
				users++;
		while
			( ++ttyp < LAST_TTY );

		if ( users )
			printf( "\nNote: %d user%scurrently logged on.\n"
				,users
				,(users > 1) ? "s " : " "
			      );
	}

	FF;
  }

  lastprint = now;
}



/*
 *	pday	print day in form "mmm nn"
 *	====
 */

pday( now )
  long now;
{
	register char	*s;
	register	i;
	extern char	*ctime();

  s = ctime( now );
  for ( i = 4 ; i < 10 ; i++ )
	putchar( s[i] );
}




int	zero;		/* TRUE if value printed is zero */

/*
 *	ptime	put time in string in form "nnnXnnY" where X&Y can be any of 'd', 'h', 'm', 's'
 *	=====
 */

char *ptime( time, sp )
  long time;
  char *sp;
{
	struct { int hiword; unsigned loword; };
	char	s[8];

  if ( time < 0 )
	goto negativ;

  {
	register	trac, sepc, divn;

	sepc = 'm';	trac = 's';
	divn = 60;

	if ( time > 3599 )	/* more than 60 mins. */
	{
		trac = sepc;
		sepc = 'h';
		time =/ 60;

#ifdef	TIME_IN_DAYS
		if ( time > 10080 )	/* more than 1 week */
		{
			trac = sepc;
			sepc = 'd';
			divn = 24;
			time =/ 60;

			if ( time > 23999 )	/* more than 1000 days */
				trac = 0;
		}
#else
		if ( time > 59999L )	/* more than 1000 hours */
			trac = 0;
#endif	TIME_IN_DAYS
	}

	zero = 1;

	if ( !trac )
	{
		time =/ divn;
		if ( time.hiword )
			goto overflo;
		ptn( time.loword, s, 6 );
		s[6] = sepc;
	}
	else
	{
		unsigned	t;

		ptn( (t=time/divn), s, 3 );
		s[3] = zero ? ' ' : sepc;
		ptn( (t=time%divn), &s[4], 2 );
		s[6] = zero ? '0' : trac;
	}
	s[7] = 0;
  }
  {
	register char	*cp1, *cp2;

	cp1 = s;
out:
	for ( cp2=sp ; *cp2++ = *cp1++ ; );
	return( sp );

negativ:
	cp1 = "negativ";
	goto out;
overflo:
	cp1 = "overflo";
	goto out;
  }
}



/*
 *	ptn	put a decimal number into string with leading spaces
 *	===
 */

char *ptn( t, s, n )
  register unsigned t,n;
  register char *s;
{
  if ( --n )
	s = ptn( t/10, s, n );
  if ( (t =% 10) || !zero )
  {
	zero = 0;
	*s++ = t + '0';
  }
  else
	*s++ = ' ';
  return( s );
}




/*
**	itoa	convert integer to fixed length ascii string
**	====
*/

char *itoa( i )
{
	static char	s[6];

	if ( i )
	{
		zero = 1;
		ptn( i, s, 5 );
		return( s );
	}
	else
		return( "    0" );
}
