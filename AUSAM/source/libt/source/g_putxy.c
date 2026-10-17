#include "cplot.h"

/*	Does its best to put the minimum number of bytes out.	*/

g_putxy( x, y )
int x, y;
{
	int register l, h, ly;
	l = y;
	h = ((l>>5)&MASK) | HIY;
	ly = l = (l&MASK) | LOY;
	if( h != g_b1 ) g_put(g_b1=h);
	if( l != g_b2 )
	{
		g_put(g_b2=l);
		ly=0;
	}
	l = x;
	h = ((l>>5)&MASK) | HIX;
	l = (l&MASK) | LOX;
	if( h != g_b3 )
	{
		if( ly ) g_put(g_b2=ly);
		g_put(g_b3=h);
	}
	g_put(g_b4=l);

	for( l=0; l<g_syncs; l++ ) g_put( SYNC );
}

