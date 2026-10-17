#include "cplot.h"

sdrawa(x,y)
int x,y;
{
	if( g_solve(g_spx,g_spy,x,y))	/*	Clip the vector with solve.	*/
	{
		/*	If the last thing done was a draw or a point
			dont bother sending out first byte.
		*/
		if( !(g_status & ONSCREEN) || (g_status & ALPHA) )
		{
			g_graphics();
			g_putxy(g_sqx1,g_sqy1);
		}
		g_putxy(g_sqx2,g_sqy2);
	}
	g_spx = x;
	g_spy = y;
	/*	Solve provides g_last which says whether second point was
		in the window.
	*/
	g_status = g_last;
}
