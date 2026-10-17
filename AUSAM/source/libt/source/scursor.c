#include "cplot.h"

char scursor(x,y)
int *x, *y;
{
	char g_buf[CURSBUFSIZE];
	register int i;

	g_alpha();

	g_ekoff();	/* TURN OFF ERASE AND KILL */

	for( i=1; i< g_syncs; i++ )
		putc( SYNC, stdout );
	fputs( CURSREAD, stdout );
	fgets( g_buf, CURSBUFSIZE, stdin );
	/* TERMINAL MUST SUPPLY CR AT END OF TRANSMISSION */
	if( feof(stdin) )
	{
		fgets( &g_buf[1], CURSBUFSIZE - 1, stdin );
		g_buf[0] = EOF;
	}

       /*  reconstruct x in screen co-ords  */
	*x = ((g_buf[1]&MASK)<<5) | (g_buf[2]&MASK);

       /*  reconstruct y in screen co-ords  */
	*y = ((g_buf[3]&MASK)<<5) | (g_buf[4]&MASK);

	g_ekon();

	return(g_buf[0]);
}
