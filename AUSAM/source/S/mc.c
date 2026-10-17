/*
 *	mc [-line width] [+blank count] [:indent count] [filename]
 */

/*
 *	Much of the complications in this program
 *	comes from trying to optimize the I/O.
 */

char proto[]
{
	"/tmp/mcaXXXXX"
};

char iobuf[512];

main( argc,argv )
char *argv[];
{
	extern interrupt();
	register char *bp, *be;
	register lw;
	int line_width, blank_count, indent_count, biggest, n_a_line;
	int ic;
	int input, output, cp, lc, buffer_size;
	char *arg, *blank;

	line_width = 80;
	blank_count = 2;
	indent_count = 0;
	while( argc-- > 1 )
	{
		arg = argv[argc];
		switch( *arg )
		{
		case '-':
			line_width = number( &arg[1] );
			continue;
		case '+':
			blank_count = number( &arg[1] );
			continue;
		case ':':
			indent_count = number( &arg[1] );
			continue;
		}
		close( 0 );
		if( open(arg,0) == -1 )
		{
			perror( arg );
			return 1;
		}
	}
	line_width =- indent_count;
	if( line_width <= 0 )
	{
		prints(2,"indent exceeds line length\n");
		return 1;
	}

	if( (output=creat(mktemp(proto),0400)) == -1 )
	{
		perror("temp file");
		return 1;
	}
	signal(2,&interrupt);
	lw = biggest = 0;

	while( buffer_size = read(0,iobuf,512) )
	{
		be = &iobuf[buffer_size];
		bp = &iobuf[0];
		while( bp<be )
			if( *bp++ == '\n' )
			{
				biggest = max(biggest,lw);
				lw = 0;
			}
			else
				lw++;
		write(output,iobuf,buffer_size);
	}

	if( biggest == 0 )
		goto done;

	n_a_line = ( line_width-1 ) / ( biggest=+blank_count );
	blank = sbrk(biggest);
	be = &blank[biggest];
	bp = &blank[0];
	while( bp<be ) *bp++ = ' ';
	close( 0 );
	close( output );
	input = open(proto,0);
	lw = cp = 0;
	ic = indent_count;

	while( buffer_size = read(input,iobuf,512) )
	{
		be = &iobuf[buffer_size];
		bp = lc = &iobuf[0];
		while( bp < be )
		{
			if( *bp == '\n' )
			{
				for( ; ic-- > 0 ; putchar(' ') );
				write(1,lc,bp-lc);
				if( ++cp >= n_a_line )
				{
					putchar('\n');
					cp = 0;
					ic = indent_count;
				}
				else
				{
					write(1,blank,biggest-(bp-lc+lw));
				}
				lw = 0;
				lc = bp+1;
			}
			bp++;
		}
		if( be - lc )
		{
			write(1,lc,be-lc);
			lw =+ be-lc;
		}
	}
	if( cp )
		putchar('\n');
done:
	unlink(proto);
}

max( a,b )
register int a,b;
{
	if( b < a )
		return( a );
	else
		return( b );
}

copy( in , out )
char *in,*out;
{
	register char *i = in;
	register char *o = out;

	while( *o++ = *i++ );
	return( out );
}

interrupt()
{
	unlink(proto);
	exit();
}

number( s )
char *s;
{
	register c = 0;
	register char *sp = s;

	while( *sp>='0' && *sp<='9' )
		c = c*10 + *sp++ - '0';
	if( *sp!='\0' )
		printf("%s: illegal numeric\n",s);
	return( c );
}
