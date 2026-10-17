#
/*
**	generate AUSAM decay constant table for login
**	which satisfies:
*/
char	formula[]	"usage = (usage/decays[hours])*hours*DECAYMULT";


#include	<local-system>
#include	<passwd.h>

#define	HOURS	(24*10)
#define	DECAYMULT 10



main()
{
	register	u, hours;
	double		f, pow();
	extern		fout;

	fout = dup( 1 );
	close( 0 );
	close( 1 );
	close( 2 );

	printf( "%s%s\"\n*/\n\n#define\tDECAYMULT\t%d\n\nunsigned\tdecays[]\n{\n"
		,"/*\n**\tdecay constant table for getty\n**\t which satisfies:-\n**\t\t\""
		,formula
		,DECAYMULT
	      );

	for ( hours = 1 ; hours <= HOURS ; hours++ )
	{
		printf( "\t0%o%c"
			,(u = hours * DECAYMULT * pow( USAGEK, (f = hours) ))
			,hours == HOURS ? ' ' : ','
		      );
		if ( (hours & 7) == 0 )
			putchar( '\n' );
	}

	printf( "};\n\n#define\tDECAYSIZ\t((sizeof decays)/(sizeof decays[0]))\n" );
	flush();
}
