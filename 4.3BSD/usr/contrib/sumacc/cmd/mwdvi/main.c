#include	<stdio.h>
#include	<signal.h>

#include	"site.h"
#include	"gen.h"
#include	"option.h"
#include	"md.h"

LOCAL	char	* program;
LOCAL	char	* outfile	= NULL;
LOCAL	char	tmpfile[] = "/tmp/mwXXXXXX";
LOCAL	bool	temp = FALSE;

#ifdef	BSD_42
LOCAL	char	pr_opts[BUFSIZ]	= "";
#endif	BSD_42
LOCAL	void	sig();


/*
 *	global variables
 */
FILE	* infd;
FILE	* outfd = NULL;



/*
 *	options
 */
bool	verbose	= FALSE;
bool	spool = FALSE;
bool	debflg	= FALSE;
char	* font_sub_file	= NULL;


extern	char	* getenv();


main( argc, argv )
int	argc;
char	* argv[];
	{

	bool	first_time = TRUE;

	program = argv[0];

	/*
	 *	signals, signals, signals!
	 */
	signal( SIGINT, sig );
	signal( SIGQUIT, sig );
	signal( SIGHUP, sig );

	/*
	 *	take a shot at some environment variables
	 */
	font_sub_file = getenv( "MACFONTS" );


	/*
	 *	process command line options
	 */
	while( (++argv, --argc) && (*argv)[0] == '-' )
	switch( (*argv)[1] )
		{

		/*	be noisy	*/
		case 'v':
			verbose = TRUE;
			break;

		/*	spool resultant output	*/
		case 'p':
			spool = TRUE;
			break;
		
#ifdef	BSD_42
		case 'P':
			strcat( pr_opts, " " );
			strcat( pr_opts, *argv );
			spool = TRUE;
			break;
#endif	BSD_42
		

		/*	debug output	*/
		case 'D':
			debflg = TRUE;
			break;

		/*	font substitution file	*/
		case 's':
			if( --argc == 0 )
				fatal( "must specify output file" );

			font_sub_file = *(++argv);
			break;
			

		/*	put mah output here	*/
		case 'o':
			if( --argc == 0 )
				fatal( "must specify output file" );

			outfile = *(++argv);
			break;
		
		default:
			fatal( "unknown option %c", (*argv)[1] );

		}
	

	/*
	 *	do it
	 */
	if( argc == 0 )
		fatal( "no input file" );
	

	/*	all output goes to a file 	*/
	if( outfile == NULL )
		{
		if( !spool )
			fatal( "no output file" );
		else
			{
			mktemp( tmpfile );
			temp = TRUE;
			outfile = tmpfile;
			}
		}
	

	if( (outfd = fopen( outfile, "w" )) == NULL )
		fatal( "couldn't open %s", outfile );


	

	while( argc-- )
		{

		if( (infd = fopen( *(argv++), "r" )) == NULL )
			fatal( "couldn't open macwrite file %s", *argv );
		
		if( verbose && argc > 0 )
			fprintf( stderr, "%s:\n", *argv );

		if( first_time )
			{
			init();
			first_time = FALSE;
			}

		read_globals();
		body();

		fclose( infd );

		}
	
	finish();

	fclose( outfd );
	if( verbose )
		fprintf( stderr, "%d page(s) produced\n", page_cnt );

	if( spool )
		spool_file();

	Exit( 0 );

	}


Exit( rc )
int	rc;
	{

	if( temp && outfd )
		unlink( tmpfile );
	
	exit( rc );

	}



/*
 *	SYSTEM DEPENDENT!
 */


/*	write dvi output directly to imagen	*/
spool_file()
	{
	char	buf[300];
	bool	temp = FALSE;

	if( verbose )
		fprintf( stderr, "spooling dvi output\n" );

	/*
	 *	must use less precise -a setting because of common bug in
	 *  dviimp  ->  fixptrsuconv calculated using 2E20 fix/pt, 
	 *  not 2^20 fix/pt
	 */

#ifdef	BSD_41
	sprintf( buf, "dviimp -a %s | ipr -Limpress", outfile );
#endif	BSD_41

#ifdef	BSD_42
	sprintf( buf, "TERMCAP=/etc/printcap; export TERMCAP; dviimp -a -p %s %s", pr_opts, outfile );
#endif	BSD_42

	if( system( buf ) != 0 )
		fatal( "error in spooling" );


	}


LOCAL	void
sig()
	{
	fprintf( stderr, "%s aborted\n", program );
	Exit( 1 );
	}


fatal( fmt, a1, a2, a3, a4 )
char	* fmt;
int	a1, a2, a3, a4;
	{

	fprintf( stderr, "%s: fatal - ", program );
	fprintf( stderr, fmt, a1, a2, a3, a4 );
	putc( '\n', stderr );

	Exit( 1 );

	}

warning( fmt, a1, a2, a3, a4 )
char	* fmt;
int	a1, a2, a3, a4;
	{

	fprintf( stderr, "%s: warning - ", program );
	fprintf( stderr, fmt, a1, a2, a3, a4 );
	putc( '\n', stderr );

	}

