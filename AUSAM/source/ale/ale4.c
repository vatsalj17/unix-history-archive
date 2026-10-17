#include	"defines.h"

/*
 *	the getch() routine returns the next character
 *	in the input stream. it also performs the following
 *		flags illegal characters
 *		condences [ \t]+ to ' '.
 *		sets the global variable 'eof' to 1 on eof
 */

char	getch()
{
	register char	c;	/* the character just read		*/
	char		nextc();
	char		*mess	"illegal character '?'";

	c = nextc();
	if( eof )
		return( c );
	if( illchar[c] == ILLEGAL )
	{
		mess[19] = c;
		errlog( mess );
	}
	if( ( c == ' ' ) || ( c == '\t' ) )
	{
		while( ( ( c = nextc() ) == ' ' ) || ( c == '\t' ) );
		unextc( c );		/* put char back		*/
		return( ' ' );		/* return a blank		*/
	}
	return( c );
}

char	nextc()
{
	if( *l_ptr == NULLCHAR )
	{
		readln();	/* input a new line			*/
		lineno++;
	}

	if( *l_ptr == EOF )
		eof = 1;
	return( *l_ptr++ );
}

unextc( c )
char	c;
{
	*--l_ptr = c;
}

readln()			/* reads a line in and strips comments	*/
{
	register char	*p;	/* pointer for temporary buffer		*/

	p = line_buf;
	while( ( ( *p++ = getc( ioptr ) ) != '\n' ) && ( *(p-1) != EOF ) );
	*p-- = NULLCHAR;	/* null terminate the string		*/

	if( p > ( line_buf + MAXLINE ) )
		errlog( "line to long" );

	while( ( *p == ' ' ) || ( *p == '\t' ) )
		*p-- = NULLCHAR;/* remove trailing blanks		*/
	if( new_address )
	{
		while( ( *p != ':' ) && ( p != line_buf ) )
			p--;
		if( *p == ':' )
			p++;	/* have now removed comment		*/
		new_address = 0;
	}
	else
		p = line_buf;
	while( ( *p == ' ' ) || ( *p == '\t' ) )
		p++;		/* remove leading blanks		*/

	l_ptr = p;
}

/*
 *	the getfield() routine enteres the next
 *	field into its argument.
 *	it returns 4 values which are
 *		END	on $\n		end of address
 *		DEAR	on $[^\n]	dear field following
 *		NL	on \n		new line
 *		SEP	on ,[\n]	valid field seperator
 */

getfield( p )
register char	*p;		/* where to enter the field		*/
{
	register char	c;	/* temperary char			*/
	register	val;	/* the value to return			*/
	int		init_p;	/* initial value of p			*/

	init_p = p;
	if( ( *p = getch() ) == ' ' )	/* remove leading blanks	*/
		*p = getch();

	while( ( *p != ',' ) && ( *p != '$' ) &&  /* copy the field	*/
	       ( *p != '\n' ) && ( *p != EOF ) )  /* across		*/
			*++p = getch();
	if( p > ( init_p + MAXBPF ) )
		errlog( "field to large" );

	switch( *p )		/* test for the appropriate ending cond	*/
	{

case EOF:
	val = END;
	break;
case '\n':
	val = NL;		/* return this value			*/
	break;

case ',':
	c = getch();
	if( c == ' ' )
		c = getch();
	if( c != '\n' )
		unextc( c );
	val = SEP;
	break;

case '$':
	c = getch();
	if( c == ' ' )
		c = getch();
	if( c == '\n' )
		val = END;
	else
		val = DEAR;
	if( c != '\n' )
		unextc( c );
	break;

	}

	if( *(p-1) == ' ' )
		p--;		/* remove trailing blanks		*/
	if( ( p == init_p ) && !eof )
		errlog( "null field" );
	size = p - init_p;
	*p = NULLCHAR;		/* get rid of the last character	*/
	return( val );
}

/*
 *	the getaddr() routine enters all the
 *	fields returned via getfield() in
 *	the global variable fields[][].
 */

getaddr()
{
	register	val,	/* value returned from getfield()	*/
			n,	/* index to fields[][] array		*/
			fin;	/* whether to finish or not		*/
	char		c;	/* temporary character			*/

	n = 0;
	fin = 0;
	new_address = 1;
	status = NORM_FIELD;
	dear_field[0] = NULLCHAR;

	while( ( c = getch() ) == '\n' );
		unextc( c );

	while( ( fin != 1 ) && ( ( val = getfield( fields[n] ) ) != END ) )
	{
		size_field[n] = size;
/*
		if( size > label_hz )
			errlog( "field to long for label" );
		if( size > width )
			errlog( "field to long for line" );
		if( n > label_vt )
			errlog( "to many fields for label" );
*/
		if( n == NFIELDS )
			errlog( "to many fields for core allocation" );
		switch( val )
		{

	case SEP:
		n++;
		break;		/* get more fields			*/

	case DEAR:
		status = DEAR_FIELD;
		switch( getfield( dear_field ) )
		{
		case NL:
			break;	/* finish now that all fields found	*/
		case END:
		case DEAR:
			errlog( "$ token in dear field" );
		case SEP:
			errlog( "',' separator in dear field" );
		}
		fin = 1;
		if( ( dear_field[0] != 'D' ) || ( dear_field[1] != 'e' ) ||
		    ( dear_field[2] != 'a' ) || ( dear_field[3] != 'r' ) ||
		    ( dear_field[4] != ' ' ) )
			run_err( "'Dear' token not in 'dear' field" );
		break;

	case NL:
		errlog( "missing ',' or '$' token" );

		}

	}

	if( eof && ( n != 0 ) )
		errlog( "eof in field" );
	size_field[n++] = size;
	fields[n][0] = NULLCHAR;/* null terminate the fields		*/
	size_field[n] = 0;	/* zero length last element		*/
}

/*
 *	the errlog() routine reports the error
 *	by printing the file name (if input not
 *	standard input), the line number it occured
 *	on, and the string given to it.
 *	it then performs an envrest() to effect
 *	the error recovery
 */

errlog( s )
char	*s;
{
	error_flag++;
	if( !sflag )
	{
		if( stdinput == 0 )	/* standard input not used	*/
			fprintf( stderr, "%s: ", infile );
		fprintf( stderr, "%d: %s\n", lineno, s );
	}
	envrest( &recover );	/* enter error recovery			*/
}

/*
 *	this routine prints the given
 *	error message as a runtime error
 */

run_err( s )
char	*s;
{
	if( !sflag )
	{
		fprintf( stderr, "runtime error: " );
		if( infile )
			fprintf( stderr, "%s: ", infile );
		fprintf( stderr, "%d: %s\n", lineno, s );
	}
	error_flag++;
	compile = 0;
}

/*
 *	this routine prints the character
 *	string on the given output.
 *	it checks for wrap around stuff
 */

print( outport, s )
char	*s;
int	outport;
{
	register char	*p, *base, *last_blank;
	int		nchars;

	nchars = strlen( s );
	if( nchars <= WRAP_WIDTH )
		fprintf( outport, "%s\n", s );
	else
	{
		base = p = s;
		while( *p )
		{
			nchars = -1;
			while( *p && ( nchars < WRAP_WIDTH ) )
			{
				last_blank = p;
				nchars++;
				for( p = last_blank+1; *p && ( *p != ' ' ); p++ )
					nchars++;
			}
			if( nchars >= WRAP_WIDTH )
			{
				*last_blank = NULLCHAR;
				p = last_blank + 1;
			}
			fprintf( outport, "%s\n", base );
			base = last_blank + 1;
		}
	}
}

/*
 *	this routine opens the output file
 *	with the given character appended
 *	to the end. it returns the ioptr
 *	returned by fopen()
 */

ale_open( end_char )
char	end_char;
{
	register	n;
	register char	*t;

	t = outfile;
	while( *++t );
	*t++ = end_char;
	*t = NULLCHAR;

	n = fopen( outfile, "w" );
	if( n == NULL )
	{
		if( !sflag )
			perror( outfile );
		exit( 1 );
	}

	*--t = NULLCHAR;
	return( n );
}

/*
 *	This routine unlinks the output file with
 *	the character passed to it appended on the end
 */

ale_unlink( end_char )
char	end_char;
{
	register char	*t;

	t = outfile;
	while( *++t );
	*t++ = end_char;
	*t = NULLCHAR;
	unlink( outfile );
	*--t = NULLCHAR;
}

/*
 *	this routine is used by print_labs() to
 *	optimize the output of blanks
 */

putc_labs( c )
char	c;
{
	static int	n;

	switch( c )
	{

	case ' ':
		n++;
		break;

	case '\n':
		n = 0;
		putc( '\n', outp_L );
		break;

	default:
		while( n-- )
			putc( ' ', outp_L );
		putc( c, outp_L );
		n = 0;
		break;

	}
}
