#include "cplot.h"

uenq(x,y)
float *x,*y;
{
	char g_buf[6];
	register int i;

	g_alpha();

	g_ekoff();

	for( i=1; i<g_syncs; i++ )
		putc( SYNC, stdout );
	fputs( ENQUIRE, stdout );
	fgets( g_buf, CURSBUFSIZE, stdin );	/* TERMINAL MUST SUPPLY CR AT END OF TRANSMISSION */

	/*  reconstruct x in user co-ords  */
	*x = (((g_buf[0]&MASK)<<5) | (g_buf[1]&MASK)) * (g_xhi - g_xlo) / SXMAX  + g_xlo ;

	/*  reconstruct y in user co-ords  */
	*y = (((g_buf[2]&MASK)<<5) | (g_buf[3]&MASK)) * (g_yhi - g_ylo) / SYMAX + g_ylo;

	g_ekon();
}
