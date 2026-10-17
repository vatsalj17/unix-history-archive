#include "cplot.h"

udrawa(x,y)
float x,y;
{

	/*	Uses same logic as sdrawa but is separate for speed.	*/
	g_sax = g_spx;	/* SAVE THE CURRENT POSITION	*/
	g_say = g_spy;
	g_chtscr(g_px = x,g_py = y);

	if(g_solve(g_sax,g_say,g_spx,g_spy))
	{
		if( !(g_status&ONSCREEN) || (g_status&ALPHA) )
		{
			g_graphics();
			g_putxy(g_sqx1,g_sqy1);
		}
		g_putxy(g_sqx2,g_sqy2);
	}
	g_status = g_last;
}
