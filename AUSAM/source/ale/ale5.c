#include	"defines.h"

pr()
{
	register	n;

	n = 1;
	printf( "%s", fields[0] );
	while( fields[n][0] )
		printf( ", %s", fields[n++] );
	putc( ' ', stdout );
	putc( '$', stdout );
	if( dear_field[0] )
		printf( "%s", dear_field );
	putc( '\n', stdout );
	putc( '\n', stdout );
}
