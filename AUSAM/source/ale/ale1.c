#include	"defines.h"

lab()
{
	format( &labs[nlabels++] );
	if( nlabels == 3 )
	{
		print_labs();
		nlabels = 0;
	}
}

format( lab )
struct label	*lab;
{
	register char	*p, *s, *lp;
	char		*b, *e, buf[1024];
	int		n, i, j, maxlen, maxhite;
	int		xoffset, yoffset;

	blankout( lab );

	maxlen = 0;
	maxhite = 0;			/* for the centering		*/
	p = buf;			/* buffer to store the fields	*/
					/* as they will be printed	*/

	for( i = 0; fields[i][0]; i++ )
	{
		for( s = fields[i]; *s; maxlen = ( maxlen < n ? n : maxlen ) )
		{
			maxhite++;
			n = 0;
			b = p;
			e = b + WRAP_WIDTH;
			if( *s == ' ' )
				s++;
			while( ( p < e ) && *s )
				*p++ = *s++, n++;
			if( !*s )
			{
				*p++ = 0;
				continue;
			}
			while( ( *s != ' ' ) && ( p > b ) )
				p--, s--, n--;
			if( p != b )
				*p++ = 0;
			else
			{
				run_err( "field to large for label" );
				break;
			}
		}
	}

	/* now to copy the fields across into the label			*/

	if( maxhite > ( MAXLINES - 1 ) )
	{
		run_err( "to many lines for label" );
		return;
	}
	xoffset = ( lab_width - 1 - maxlen ) / 2;
	yoffset = ( lab_hight - 1 - maxhite ) / 2;
	p = buf;
	for( i = yoffset; i < ( yoffset + maxhite ); i++ )
	{
		lp = &lab->lab_lines[i][xoffset];
		while( *p )
			*lp++ = *p++;
		p++;
	}
}

print_labs()
{
	register	i, j;
	register char	*p;
	int		n;

	for( i = 0; i < lab_hight; i++ )
	{
		for( j = 0; j < 3; j++ )
		{
			p = &labs[j].lab_lines[i];
			while( *p )
				putc_labs( *p++ );
			putc_labs( ' ', outp_L );
		}
		putc_labs( '\n' );
	}
}

blankout( lab )
struct	label	*lab;
{
	register	i, j;

	for( i = 0; i < lab_hight; i++ )
	{
		for( j = 0; j < lab_width; j++ )
			lab->lab_lines[i][j] = ' ';
		lab->lab_lines[i][j] = 0;/* null terminate the string	*/
	}
}

dummy_labs()
{
	register	i, j;
	static char	stars[]	"**************************************************";
	static char	sblks[]	"*                                                 ";

	stars[lab_width+1] = sblks[lab_width+1] = 0;
	stars[lab_width] = ' ';
	sblks[lab_width-1] = '*';
	
	j = a_dlabs[dest];
	while( j-- )
	{
		fprintf( outp_L, "%s%s%s\n", stars, stars, stars );
		for( i = 0; i < ( lab_hight - 3 ); i++ )
			fprintf( outp_L, "%s%s%s\n", sblks, sblks, sblks );
		fprintf( outp_L, "%s%s%s\n", stars, stars, stars );
		putc( '\n', outp_L );
	}
}
