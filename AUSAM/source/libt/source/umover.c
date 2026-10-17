#include "cplot.h"

umover(x,y)
float x,y;
{
	if( !(g_status & USER) ) g_chtusr( g_spx, g_spy );
	g_chtscr(g_px =+ x,g_py =+ y);
	g_status =| (USER);
	g_status =& ~ ONSCREEN;
}

