#include "cplot.h"

/*	enquire as to the current position of the cursor */

senq(x,y)
int *x, *y;
{

	char g_buff[CURSBUFSIZE];
	register int i;

	g_alpha();

	g_ekoff();

	for( i=1; i<g_syncs; i++)
		putc( SYNC, stdout );
	fputs( ENQUIRE, stdout );
	fgets( g_buff, CURSBUFSIZE, stdin );
	g_ekon();

	*x = ((g_buff[0]&MASK)<<5) | (g_buff[1]&MASK);
	*y = ((g_buff[2]&MASK)<<5) | (g_buff[3]&MASK);
}
