#include	"defines.h"

env()
{
	register	n;

	n = 0;

	while( fields[n][0] )
		print( outp_E, fields[n++] );
	fprintf( outp_E, ".bp\n" );
}
