#include	"defines.h"

char	buf[MAXBPF], dear[MAXBPF];
int	sir_title,
	the_title;
let()
{
	register char	*p, *p2;
	register	n;

	p = fields[0];
	p2 = buf;
	n_titles = 0;
	sir_title = 0;
	the_title = 0;
	n_names = 0;
	n = 0;

	while( *p )
	{
		names[n_names++] = p2;
		while( *p && ( *p != ' ' ) )
			*p2++ = *p++;
		*p2++ = NULLCHAR;
		if( *p )
			p++;
	}

	while( 1 )
	{
		if( strcmp( names[n_titles], "The" ) == 0 )
		{
			names[n_titles++] = NULLCHAR;
			the_title++;
			continue;
		}
		else
		if( strcmp( names[n_titles], "Sir" ) == 0 )
		{
			names[n_titles++] = NULLCHAR;
			sir_title++;
			continue;
		}
		else
		for( t_ptr = titles; *t_ptr; t_ptr++ )
			if( strcmp( names[n_titles], *t_ptr ) == 0 )
				break;
		if( *t_ptr )
			n_titles++;
		else
			break;
	}

	p = dear;
	n = 0;
	if( !dear_field[0] )
	{
		p2 = "Dear";
		while( *p++ = *p2++ );
		*(p-1) = ' ';
		if( sir_title )
		{
			p2 = "Sir";
			while( *p++ = *p2++ );
			*(p-1) = ' ';
			p2 = names[n_titles];	/* first name of person	*/
		}
		else
			if( n_titles != the_title )
			{
				while( n != n_titles )
				{
					p2 = names[n++];
					if( p2 )
					{
						while( *p++ = *p2++ );
						*(p-1) = ' ';
					}
				}
				n = 0;
				p2 = names[n_names-1];
				while( *p2++ );
				p2--;
				while( ( *--p2 >= 'A' ) && ( *p2 <= 'Z' ) )
					n++;
				if( *p2 == '\~' )
				{
					while( *--p2 == '\~' );
					*++p2 = NULLCHAR;
				}
				else
					if(n & !cflag)
					{
						run_err( "caps in name field" );
						return;
					}
				p2 = names[n_names-1];
			}
			else
			{
				p2 = "Sir/Madam";
				while( *p++ = *p2++ );
				*(p-1) = ' ';
				p2 = "";	/* this is terible	*/
			}
	}
	else
		p2 = dear_field;
	while( *p++ = *p2++ );

	n = 0;
	while( fields[n][0] )
		print( outp_A, fields[n++] );
	putc( '\n', outp_A );
	print( outp_A, dear );
	putc( '\n', outp_A );
}
