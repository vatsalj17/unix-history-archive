#
/*
 *	ac [ [[-]f...]... ] [name]
 *
 *		where 'f' is any of 'defhilnopstuwCILNPSU'
 *		and "name" is any extra string required by a flag 'f'.
 *
 *	Piers Lauder	Mar '78
 */

#include	<local-system>

#ifdef	AUSAM

#include	"ac.h"		/* for definitions of flags, data and include files */
#include	<passwd.h>
#ifdef	WTMP_AUTOLOCK
#include	<stat16.h>
#endif	WTMP_AUTOLOCK

char	*wtmpf	"/usr/adm/wtmp";

int	wf;			/* file descriptor for wtmp file */

static char	noname[]	"%s: no name for '%c' flag.\n";

#ifdef	DIAGNOSTIC_REDIRECT
char	*diagf	"ac.diagnostics";
#endif



/*
 *	the main program - deal with parameters, set up files, call ac()
 */

main( argc, argv )
  register argc;
  register char **argv;
{
	register	c;
	extern		fout;
	extern char	*etcpasswd;

  fout = 2;			/* for errors */
  name = *argv++;		/* invoked name */

  while ( --argc )
  {
	while ( c = *(*argv)++ )
		switch ( c )
		{
		 case '-':	continue;

		 case 'I':	flg.I++;	continue;
		 case 'P':	flg.P++;	continue;
		 case 'd':	flg.d++;	continue;
		 case 'e':	flg.e++;	continue;
		 case 'f':	flg.f++;	continue;
		 case 'h':	flg.h++;	continue;
		 case 'i':	flg.z++;	continue;
		 case 'l':	flg.l++;	continue;
		 case 'n':	flg.n++;	continue;
		 case 'o':	flg.o++;	continue;
		 case 'p':	flg.p++;	continue;
		 case 's':	flg.s++;	continue;
		 case 't':	flg.t++;	continue;
		 case 'u':	flg.u++;	continue;
		 case 'w':	flg.w++;	continue;
#		ifdef	DIAGNOSTIC_REDIRECT
		 case 'x':	flg.x++;	continue;
#		endif
#		ifdef	DEBUG|TIME_TRACE
		 case 'y':	flg.y++;	continue;
#		endif

		 case 'C':	porder = bycpu;		continue;
		 case 'L':	porder = bylog;		continue;
		 case 'N':	porder = byname;	continue;
		 case 'S':	porder = bylogons;	continue;
		 case 'U':	porder = byuid;		continue;

		 default:
			printf( "%s: bad flag '%c',\n", name, c );
			continue;
		}

	argv++;

	if ( flg.w )
		if ( --argc )
		{
			wtmpf = *argv++;
			flg.w = 0;
		}
		else
		{
			printf( noname, name, 'w' );
			return( 1 );
		}

	if ( flg.P )
		if ( --argc )
		{
			if ( flg.i )
			{
				printf( "%s: 'P' should precede 'i'\n", name );
				return( 1 );
			}

			pwfile( *argv++ );
			flg.P = 0;
		}
		else
		{
			printf( noname, name, 'P' );
			return( 1 );
		}

	if ( flg.z )
		if ( --argc )
		{
			struct pwent	pe;

			pe.pw_strings[LNAME] = *argv++;
			if ( getpwuid( &pe, 0, 0 ) < 0 )
			{
				printf( "%s: who is \"%s\"?\n", name, pe.pw_strings[LNAME] );
				return( 1 );
			}

			if ( !flg.i )
			{
				individual = pe.pw_uid;

				flg.i++;	flg.p++;	flg.n++;
			}
			else
				printf( "%s: only one individual allowed, %s ignored\n", name, pe.pw_strings[LNAME] );

			flg.z = 0;
		}
		else
		{
			printf( noname, name, 'i' );
			return( 1 );
		}

	if ( flg.l )
		if ( --argc )
		{
			w_days_lost = atoi( *argv++ );
			flg.l = 0;
		}
		else
		{
			printf( noname, name, 'l' );
			return( 1 );
		}
  }

  if ( !flg.I && (wf = open( wtmpf, 0 )) < 0 )
  {
	printf( "%s: cannot open %s!\n", name, wtmpf );
	return( 1 );
  }

# ifdef	WTMP_AUTOLOCK
  if ( !flg.I )
  {
	struct statbuf	buf;

	fstat( wf, &buf );

	if ( buf.sb_flags & IFLOCK )
	{
		unlock();
		flg.locking++;
	}
  }

# endif	WTMP_AUTOLOCK
  if ( flg.d && w_days_lost )
  {
	printf( "%s: working days lost ignored!\n" );
	w_days_lost = 0;
  }

  fout = dup( 1 );

# ifdef	DIAGNOSTIC_REDIRECT
  if ( flg.x )
  {
	close( 2 );
	if ( open( diagf, 1 ) < 0 )
	{
		if ( creat( diagf, 0600 ) < 0 )
		{
			printf( "%s: cannot create %s!\n", name, diagf );
			return( -1 );
		}
	}
	else
		seek( 2, 0, 2 );
  }
# endif

  if ( !flg.I )
	close( 0 );
  close( 1 );

  if ( !flg.e && !flg.h && !flg.i && !flg.p && !flg.s && !flg.t && !flg.u )
  {
	if ( individual = getreal() )
	{
		flg.i++;	/* default individual if not root */
		flg.p++;
	}
	else
		flg.s++;	/* default statistics */
#	ifdef	DIAGNOSTIC_REDIRECT
	if ( !flg.x )
#	endif	DIAGNOSTIC_REDIRECT

	flg.n++;		/* with no warnings */
  }

  if ( flg.p && !porder )
	porder = byname;	/* default people sort order */

  if ( flg.o )
	printf( "\nNote: cpu times not available in pre-AUSAM accounts.\n" );

  ac();

  flush();

  return( 0 );
}




struct	wtmp	wtmp[512/WTMPSIZ];

/*
 *	ac	main loop to read accounting file and call appropriate accounting routines
 *	==
 */

ac()
{
	register struct wtmp	*wp;
	register		n;
	extern			login(), logout(), async();

  for ( ;; )
  {
#	ifdef	WTMP_AUTOLOCK
	if ( flg.locking )
		readlock( wf );

#	endif	WTMP_AUTOLOCK
	n = read( wf, (wp=wtmp), (sizeof wtmp) );

#	ifdef	WTMP_AUTOLOCK
	if ( flg.locking )
		unlock();

#	endif	WTMP_AUTOLOCK
	if ( (n <= 0) || ((n =/ WTMPSIZ) == 0) )
		break;

	do
	{
		switch ( wp->w_type )
		{
		 case U_TYPE:	time( login, wp );	break;
		 case O_TYPE:	time( logout, wp );	break;
		 case W_TYPE:	time( async, wp );	break;
		 case S_TYPE:	boot( wp );		break;
		 case D_TYPE:	update( wp );		break;
		 case SU_TYPE:	continue;

		 default:
			warn( "unrecognised type ignored", wp );
			continue;
		}
	}
	while
		( wp++ , --n );

  }

  if ( flg.z )
	print( timenow );
}



/*
 *	time	initialises and keeps trace of time
 *	====
 */

time( f, wp )
  int			(*f)();
  register struct wtmp	*wp;
{
  if ( wp->w_finishtime >= hour )
	if ( flg.z )
		chime( wp, 0 );
	else
		init( wp );

  if ( wp->w_finishtime < timenow )
  {
	static long	sometime;

	if ( sometime != timenow && wp->w_finishtime < (timenow - MINUTE) )
	{
		warn( "date error", wp );
		sometime = timenow;
	}

	if ( wp->w_finishtime == 0 )
	{
		syswarn( " -- zero entry ignored\n" );
		return;
	}
  }
  else
	timenow = wp->w_finishtime;

  if ( !flg.i || wp->w_uid == individual )
	(*f)( wp );
}



/*
 *	init	initialise all times
 *	====
 */

init( dp )
  register struct dtmp *dp;
{
# ifdef	TIME_TRACE
  print_times( "init", TT_ALL );
# endif	TIME_TRACE
#ifdef	DEBUG
  if ( flg.y )
	syswarn( "init at %s", ctime( dp->d_newtime ) );
#endif

  do
	year =+ YEAR;
  while
	( year <= dp->d_newtime );

  midnight = year - YEAR;
  fixuptime( dp );

  lastprint = midnight - DAY;
  sys.st_boottime = lastprint;

  flg.z++;
#ifdef	DEBUG
  if ( flg.y )
	syswarn( "init: hour is %s", ctime( hour ) );
#endif
# ifdef	TIME_TRACE
  print_times( "init", TT_ALL );
# endif	TIME_TRACE
}

#endif	AUSAM
