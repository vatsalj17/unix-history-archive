#include	"defines.h"
#include	"def.h"

main( ac, av )
int	ac;
char	**av;
{
	register char	p;
	register char	*t, *t2;

	ac--;
	av++;

/*	signal( 3, 1 );*/
	t = illc;
	while( *t )
		illchar[*t++] = ILLEGAL;
	for( p = '\000'; p < '\011'; p++ )
		illchar[p] = ILLEGAL;
	for( p = '\013'; p < '\040'; p++ )
		illchar[p] = ILLEGAL;

	while( **av == '-' )
	{
		if( strcmp( &av[0][1], "pr" ) == 0 )
			prflag++;
		else
		if( strcmp( &av[0][1], "let" ) == 0 )
			letflag++;
		else
		if( strcmp( &av[0][1], "lab" ) == 0 )
			labflag++;
		else
		if( strcmp( &av[0][1], "env" ) == 0 )
			envflag++;
		else
		switch( av[0][1] )
		{
	case 'o':
		oflag++;
		ac--;
		av++;
		t = *av;
		t2 = outfile;
		while( *t2++ = *t++ );
		break;
	case 's':
		sflag++;
		break;
	case 'd':
		switch( av[0][2] )
		{
		case 'q':
			dest = QUME;
			break;
		case 'l':
			dest = LP05;
			break;
		case 'p':
			dest = LA180;
			break;
		default:
			if( !sflag )
				fprintf( stderr, "unknown dest '%c'\n", av[0][2] );
			exit( 1 );
		}
		break;
	case 'w':
		wflag++;
		width = atoi( &av[0][2] );
		break;
	case 'l':
		lflag++;
		length = atoi( &av[0][2] );
		break;
	case 'p':
		pflag++;
		pitch = atoi( &av[0][2] );
		break;
	case 'i':
		iflag++;
		lines = atoi( &av[0][2] );
		break;
	case 'c':
		cflag++;
		break;
	default:
		if( !sflag )
			fprintf( stderr, "unknown option %c\n", av[0][1] );
		exit( 1 );

		}

		av++;
		ac--;
	}

	if( !wflag )
		width = a_width[dest];
	if( !lflag )
		length = a_length[dest];
	if( !pflag )
		pitch = a_pitch[dest];
	if( !iflag )
		lines = a_lines[dest];

	lab_width = pitch * 4;		/* calculate necessary const	*/
	lab_hight = lines * 3/2;

	if( letflag )
		outp_A = ale_open( LETEND );

	if( labflag )
	{
		outp_L = ale_open( LABEND );
		if( dest == LP05 )
			putc( 0375, outp_L );
		dummy_labs();	/* print the dummy labels		*/
	}

	if( envflag )
	{
		outp_E = ale_open( ENVEND );
		fprintf( outp_E, ".pl 4i\n.tr ~ \n.po 35\n.nf\n" );
	}

	if( ac == 0 )
	{
		ioptr = stdin;	/* use standard input			*/
		stdinput = 1;	/* signal standard input is being used	*/
		ac = 1;
	}

	while( ac-- && ( error_flag < MAXERRORS ) )
	{

		if( !stdinput )
		{
			ioptr = fopen( *av, "r" );
			if( ioptr == NULL )
			{
				if( !sflag )
					perror( *av );
				exit( 1 );
			}
			infile = *av;
		}
		lineno = 0;
		l_ptr = line_buf;
		*l_ptr = NULLCHAR;
		while( !eof && ( error_flag < MAXERRORS ) )
		{
			if( envsave( &recover ) == 0 )
			{
				if( status == NORM_FIELD )
					while( !eof && ( nextc() != '$' ) );
				while( !eof && ( nextc() != '\n' ) );
				if( eof )
					break;
			}
			compile = 1;
			getaddr();
			if( eof )
				break;
			if( compile )
			{
				if( prflag )
					pr();
				if( !error_flag )
				{
					if( letflag )
						let();
					if( labflag )
						lab();
					if( envflag )
						env();
				}
			}
		}
		av++;
		eof = 0;
		fclose( ioptr );
	}
	if( error_flag == MAXERRORS )
		if( !sflag )
			fprintf( stderr, "too many errors. program abort\n" );
	if( letflag )
	{
		if( !error_flag )
		{
			fprintf( outp_A, ".ex\n" );
			fflush( outp_A );
			fclose( outp_A );
		}
		else
			ale_unlink( LETEND );
	}
	if( labflag )
	{
		if( !error_flag )
		{
			if( nlabels )
			{
				while( nlabels != 3 )
					blankout( &labs[nlabels++] );
				print_labs();
			}
			fflush( outp_L );
			fclose( outp_L );
		}
		else
			ale_unlink( LABEND );
	}
	if( envflag )
	{
		if( !error_flag )
		{
			fflush( outp_E );
			fclose( outp_E );
		}
		else
			ale_unlink( ENVEND );
	}
}

atoi( s )
register char	*s;
{
	register	i = 0;

	while( *s )
		if( ( *s > '9' ) || ( *s < '0' ) )
		{
			if( !sflag )
				printf( "bad number in flag\n" );
			exit( 1 );
		}
		else
			i =* 10 + *s++ - '0';
}
